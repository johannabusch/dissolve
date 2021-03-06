// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "classes/atomtype.h"
#include "classes/box.h"
#include "classes/configuration.h"
#include "classes/neutronweights.h"
#include "classes/species.h"
#include "genericitems/listhelper.h"
#include "io/export/data1d.h"
#include "main/dissolve.h"
#include "math/filters.h"
#include "math/ft.h"
#include "modules/import/import.h"
#include "modules/neutronsq/neutronsq.h"
#include "modules/rdf/rdf.h"
#include "modules/sq/sq.h"

// Run set-up stage
bool NeutronSQModule::setUp(Dissolve &dissolve, ProcessPool &procPool)
{
    /*
     * Load and set up reference data (if a file/format was given)
     */
    if (referenceFQ_.hasValidFileAndFormat())
    {
        // Load the data
        Data1D referenceData;
        if (!referenceFQ_.importData(referenceData, &procPool))
        {
            Messenger::error("Failed to load reference data '{}'.\n", referenceFQ_.filename());
            return false;
        }

        //         // Truncate data beyond QMax
        //         const auto qMax = keywords_.asDouble("QMax") < 0.0 ? 30.0 : keywords_.asDouble("QMax");
        //         if (referenceData.constXAxis().lastValue() < qMax)
        //             Messenger::warn("Qmax limit of {:e} Angstroms**-1 for calculated NeutronSQ ({}) is beyond limit of "
        //                             "reference data (Qmax = {:e} Angstroms**-1).\n",
        //                             qMax, uniqueName(), referenceData.constXAxis().lastValue());
        //         else
        //             while (referenceData.constXAxis().lastValue() > qMax)
        //                 referenceData.removeLastPoint();

        // Remove first point?
        if (keywords_.asBool("ReferenceIgnoreFirst"))
        {
            referenceData.removeFirstPoint();
            Messenger::print("Removed first point from supplied reference data - new Qmin = {:e} Angstroms**-1.\n",
                             referenceData.constXAxis().firstValue());
        }

        // Get dependent modules
        const SQModule *sqModule = keywords_.retrieve<const SQModule *>("SourceSQs", nullptr);
        if (!sqModule)
            return Messenger::error("A source SQ module must be provided.\n");
        const RDFModule *rdfModule = sqModule->keywords().retrieve<const RDFModule *>("SourceRDFs", nullptr);
        if (!rdfModule)
            return Messenger::error("A source RDF module (in the SQ module) must be provided.\n");

        // Remove normalisation factor from data
        auto normType = keywords_.enumeration<NeutronSQModule::NormalisationType>("ReferenceNormalisation");
        if (normType != NeutronSQModule::NoNormalisation)
        {
            // We need the neutron weights in order to do the normalisation
            NeutronWeights weights;
            if (!calculateWeights(rdfModule, weights))
                return Messenger::error("Couldn't get summed Weights for reference data in NeutronSQ module '{}', and "
                                        "so can't perform requested normalisation.\n",
                                        uniqueName());

            // Remove normalisation of data
            if (normType == NeutronSQModule::AverageOfSquaresNormalisation)
            {
                referenceData.values() *= weights.boundCoherentAverageOfSquares();
                Messenger::print("NeutronSQ: Removed <b>**2 normalisation from reference data ('{}'), factor = {}.\n",
                                 uniqueName(), weights.boundCoherentAverageOfSquares());
            }
            else if (normType == NeutronSQModule::SquareOfAverageNormalisation)
            {
                referenceData.values() *= weights.boundCoherentSquareOfAverage();
                Messenger::print("NeutronSQ: Removed <b**2> normalisation from reference data ('{}'), factor = {}.\n",
                                 uniqueName(), weights.boundCoherentSquareOfAverage());
            }
        }

        // Get window function to use for transformation of S(Q) to g(r)
        const WindowFunction &referenceWindowFunction =
            keywords_.retrieve<WindowFunction>("ReferenceWindowFunction", WindowFunction());
        if (referenceWindowFunction.function() == WindowFunction::NoWindow)
            Messenger::print("No window function will be applied in Fourier transform of S(Q) to g(r).");
        else
            Messenger::print("Window function to be applied in Fourier transform of reference data is {} ({}).",
                             WindowFunction::functionType(referenceWindowFunction.function()),
                             referenceWindowFunction.parameterSummary());

        // Store the reference data in processing
        referenceData.setName(uniqueName());
        auto &storedData = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "ReferenceData", uniqueName(),
                                                              GenericItem::ProtectedFlag);
        storedData.setObjectTag(fmt::format("{}//ReferenceData", uniqueName()));
        storedData = referenceData;

        // Calculate and store the FT of the reference data in processing
        referenceData.setName(uniqueName());
        auto &storedDataFT = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "ReferenceDataFT",
                                                                uniqueName(), GenericItem::ProtectedFlag);
        storedDataFT.setObjectTag(fmt::format("{}//ReferenceDataFT", uniqueName()));
        storedDataFT = referenceData;
        auto rho = 0.1;
        if (dissolve.processingModuleData().contains("EffectiveRho", rdfModule->uniqueName()))
            rho = GenericListHelper<double>::value(dissolve.processingModuleData(), "EffectiveRho", rdfModule->uniqueName());
        else
            Messenger::warn("Couldn't locate effective atomic density from '{}', so Fourier transform of reference data will "
                            "use assumed atomic density of 0.1.\n",
                            rdfModule->uniqueName());
        Fourier::sineFT(storedDataFT, 1.0 / (2.0 * PI * PI * rho), 0.0, 0.05, 30.0, referenceWindowFunction);

        // Save data?
        if (keywords_.asBool("SaveReference"))
        {
            if (procPool.isMaster())
            {
                Data1DExportFileFormat exportFormat(fmt::format("{}-ReferenceData.q", uniqueName()));
                if (!exportFormat.exportData(storedData))
                    return procPool.decideFalse();
                Data1DExportFileFormat exportFormatFT(fmt::format("{}-ReferenceData.r", uniqueName()));
                if (!exportFormatFT.exportData(storedDataFT))
                    return procPool.decideFalse();
                procPool.decideTrue();
            }
            else if (!procPool.decision())
                return false;
        }
    }

    return true;
}

// Run main processing
bool NeutronSQModule::process(Dissolve &dissolve, ProcessPool &procPool)
{
    /*
     * Calculate neutron structure factors from existing S(Q) data
     *
     * This is a serial routine, with each process constructing its own copy of the data.
     * Partial calculation routines called by this routine are parallel.
     */

    const auto *sqModule = keywords_.retrieve<const SQModule *>("SourceSQs", nullptr);
    if (!sqModule)
        return Messenger::error("A source SQ module must be provided.\n");
    const auto *rdfModule = sqModule->keywords().retrieve<const RDFModule *>("SourceRDFs", nullptr);
    if (!rdfModule)
        return Messenger::error("A source RDF module (in the SQ module) must be provided.\n");
    auto normalisation = keywords_.enumeration<NeutronSQModule::NormalisationType>("Normalisation");
    const auto saveSQ = keywords_.asBool("SaveSQ");
    const auto saveGR = keywords_.asBool("SaveGR");

    // Print argument/parameter summary
    Messenger::print("NeutronSQ: Source unweighted S(Q) will be taken from module '{}'.\n", sqModule->uniqueName());
    const WindowFunction &referenceWindowFunction =
        keywords_.retrieve<WindowFunction>("ReferenceWindowFunction", WindowFunction());
    if (referenceWindowFunction.function() == WindowFunction::NoWindow)
        Messenger::print("No window function will be applied when calculating representative g(r) from S(Q).");
    else
        Messenger::print("Window function to be applied when calculating representative g(r) from S(Q) is {} ({}).",
                         WindowFunction::functionType(referenceWindowFunction.function()),
                         referenceWindowFunction.parameterSummary());
    if (normalisation == NeutronSQModule::NoNormalisation)
        Messenger::print("NeutronSQ: No normalisation will be applied to total F(Q).\n");
    else if (normalisation == NeutronSQModule::AverageOfSquaresNormalisation)
        Messenger::print("NeutronSQ: Total F(Q) will be normalised to <b>**2");
    else if (normalisation == NeutronSQModule::SquareOfAverageNormalisation)
        Messenger::print("NeutronSQ: Total F(Q) will be normalised to <b**2>");
    if (saveGR)
        Messenger::print("NeutronSQ: Weighted partial g(r) and total G(r) will be saved.\n");
    if (saveSQ)
        Messenger::print("NeutronSQ: Weighted partial S(Q) and total F(Q) will be saved.\n");
    Messenger::print("\n");

    /*
     * Transform UnweightedSQ from provided SQ data into WeightedSQ.
     */

    bool created;

    // Get unweighted S(Q) from the specified SQMOdule
    if (!dissolve.processingModuleData().contains("UnweightedSQ", sqModule->uniqueName()))
        return Messenger::error("Couldn't locate unweighted S(Q) data from the SQModule '{}'.\n", sqModule->uniqueName());
    const auto &unweightedSQ =
        GenericListHelper<PartialSet>::value(dissolve.processingModuleData(), "UnweightedSQ", sqModule->uniqueName());

    // Calculate and store weights
    auto &weights = GenericListHelper<NeutronWeights>::realise(dissolve.processingModuleData(), "FullWeights", uniqueName_,
                                                               GenericItem::InRestartFileFlag);
    if (!calculateWeights(rdfModule, weights))
        return false;

    // Create, print, and store weights
    Messenger::print("Isotopologue and isotope composition:\n\n");
    weights.createFromIsotopologues(exchangeableTypes_);
    weights.print();

    // Does a PartialSet for the weighted S(Q) already exist for this Configuration?
    auto &weightedSQ = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "WeightedSQ", uniqueName_,
                                                              GenericItem::InRestartFileFlag, &created);
    if (created)
        weightedSQ.setUpPartials(unweightedSQ.atomTypes(), uniqueName(), "weighted", "sq", "Q, 1/Angstroms");

    // Calculate weighted S(Q)
    calculateWeightedSQ(unweightedSQ, weightedSQ, weights, normalisation);

    // Set names of resources (Data1D) within the PartialSet
    weightedSQ.setObjectTags(fmt::format("{}//{}", uniqueName_, "WeightedSQ"));

    // Save data if requested
    if (saveSQ && (!MPIRunMaster(procPool, weightedSQ.save())))
        return false;

    /*
     * Transform UnweightedGR from underlying RDF data into WeightedGR.
     */

    // Get summed unweighted g(r) from the RDFMOdule
    if (!dissolve.processingModuleData().contains("UnweightedGR", rdfModule->uniqueName()))
        return Messenger::error("Couldn't locate summed unweighted g(r) data.\n");
    const auto &unweightedGR =
        GenericListHelper<PartialSet>::value(dissolve.processingModuleData(), "UnweightedGR", rdfModule->uniqueName());

    // Create/retrieve PartialSet for summed weighted g(r)
    auto &weightedGR = GenericListHelper<PartialSet>::realise(dissolve.processingModuleData(), "WeightedGR", uniqueName_,
                                                              GenericItem::InRestartFileFlag, &created);
    if (created)
        weightedGR.setUpPartials(unweightedGR.atomTypes(), uniqueName_, "weighted", "gr", "r, Angstroms");

    // Calculate weighted g(r)
    calculateWeightedGR(unweightedGR, weightedGR, weights, normalisation);

    // Set names of resources (Data1D) within the PartialSet
    weightedGR.setObjectTags(fmt::format("{}//{}", uniqueName_, "WeightedGR"));

    // Save data if requested
    if (saveGR && (!MPIRunMaster(procPool, weightedSQ.save())))
        return false;

    // Calculate representative total g(r) from FT of calculated S(Q)
    auto &repGR = GenericListHelper<Data1D>::realise(dissolve.processingModuleData(), "RepresentativeTotalGR", uniqueName_,
                                                     GenericItem::InRestartFileFlag);
    repGR = weightedSQ.total();
    auto rMin = weightedGR.total().xAxis().firstValue();
    auto rMax = weightedGR.total().xAxis().lastValue();
    auto rho = 0.1;
    if (dissolve.processingModuleData().contains("EffectiveRho", rdfModule->uniqueName()))
        rho = GenericListHelper<double>::value(dissolve.processingModuleData(), "EffectiveRho", rdfModule->uniqueName());
    else
        Messenger::warn("Couldn't locate effective atomic density for RDF module.\n");

    Fourier::sineFT(repGR, 1.0 / (2.0 * PI * PI * rho), rMin, 0.05, rMax, referenceWindowFunction);
    repGR.setObjectTag(fmt::format("{}//RepresentativeTotalGR", uniqueName_));

    return true;
}
