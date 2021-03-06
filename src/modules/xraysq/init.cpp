// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "keywords/types.h"
#include "modules/sq/sq.h"
#include "modules/xraysq/xraysq.h"

// Return enum option info for NormalisationType
EnumOptions<XRaySQModule::NormalisationType> XRaySQModule::normalisationTypes()
{
    static EnumOptionsList NormalisationTypeOptions =
        EnumOptionsList() << EnumOption(XRaySQModule::NoNormalisation, "None")
                          << EnumOption(XRaySQModule::AverageOfSquaresNormalisation, "AverageOfSquares")
                          << EnumOption(XRaySQModule::SquareOfAverageNormalisation, "SquareOfAverage");

    static EnumOptions<XRaySQModule::NormalisationType> options("NormalisationType", NormalisationTypeOptions,
                                                                XRaySQModule::NoNormalisation);

    return options;
}

// Perform any necessary initialisation for the Module
void XRaySQModule::initialise()
{
    // Calculation
    keywords_.add("Calculation", new ModuleKeyword<const SQModule>("SQ"), "SourceSQs",
                  "Source unweighted S(Q) to transform into xray-weighted S(Q)");
    keywords_.add("Calculation",
                  new EnumOptionsKeyword<XRayFormFactors::XRayFormFactorData>(XRayFormFactors::xRayFormFactorData() =
                                                                                  XRayFormFactors::WaasmaierKirfel1995),
                  "FormFactors", "Form factors to use for weighting");
    keywords_.add("Calculation",
                  new EnumOptionsKeyword<XRaySQModule::NormalisationType>(XRaySQModule::normalisationTypes() =
                                                                              XRaySQModule::NoNormalisation),
                  "Normalisation", "Normalisation to apply to total weighted F(Q)");
    keywords_.add("Calculation", new WindowFunctionKeyword(WindowFunction(WindowFunction::NoWindow)), "WindowFunction",
                  "Window function to apply when Fourier-transforming g(r) to S(Q)");

    // Reference Data
    keywords_.add("Reference Data", new FileAndFormatKeyword(referenceFQ_, "EndReference"), "Reference", "F(Q) reference data",
                  "<format> <filename>", KeywordBase::ModificationRequiresSetUpOption);
    keywords_.add("Reference Data", new BoolKeyword(false), "ReferenceIgnoreFirst",
                  "Ignore the first point in the supplied reference data", KeywordBase::ModificationRequiresSetUpOption);
    keywords_.add("Reference Data", new WindowFunctionKeyword(WindowFunction(WindowFunction::Lorch0Window)),
                  "ReferenceWindowFunction", "Window function to apply when Fourier-transforming reference S(Q) to g(r)",
                  KeywordBase::ModificationRequiresSetUpOption);

    // Export
    keywords_.add("Export", new BoolKeyword(false), "SaveFormFactors",
                  "Whether to save combined form factor weightings for atomtype pairs", "<True|False>");
    keywords_.add("Export", new BoolKeyword(false), "SaveReference",
                  "Whether to save the reference data and its Fourier transform", "<True|False>");
    keywords_.add("Export", new BoolKeyword(false), "SaveGR",
                  "Whether to save weighted g(r) and G(r) to disk after calculation", "<True|False>");
    keywords_.add("Export", new BoolKeyword(false), "SaveSQ",
                  "Whether to save weighted S(Q) and F(Q) to disk after calculation", "<True|False>");
}

// Return file and format for reference total F(Q)
const Data1DImportFileFormat &XRaySQModule::referenceFQFileAndFormat() { return referenceFQ_; }
