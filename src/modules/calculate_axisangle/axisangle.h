// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "module/module.h"
#include "procedure/procedure.h"

// Forward Declarations
class Collect1DProcedureNode;
class Collect2DProcedureNode;
class Process1DProcedureNode;
class Process2DProcedureNode;
class SelectProcedureNode;
class SpeciesSite;

// Calculate Axis Angle Module
class CalculateAxisAngleModule : public Module
{
    public:
    CalculateAxisAngleModule();
    ~CalculateAxisAngleModule();

    /*
     * Instances
     */
    public:
    // Create instance of this module
    Module *createInstance() const;

    /*
     * Definition
     */
    public:
    // Return type of module
    std::string_view type() const;
    // Return category for module
    std::string_view category() const;
    // Return brief description of module
    std::string_view brief() const;
    // Return the number of Configuration targets this Module requires
    int nRequiredTargets() const;

    /*
     * Initialisation
     */
    protected:
    // Perform any necessary initialisation for the Module
    void initialise();

    /*
     * Processing
     */
    private:
    // Run set-up stage
    bool setUp(Dissolve &dissolve, ProcessPool &procPool);
    // Run main processing
    bool process(Dissolve &dissolve, ProcessPool &procPool);

    /*
     * Functions / Data
     */
    private:
    // Analysis procedure to be run
    Procedure analyser_;
    // SelectNode for site A
    SelectProcedureNode *selectA_;
    // SelectNode for site B
    SelectProcedureNode *selectB_;
    // Collect1DNode for A-B RDF
    Collect1DProcedureNode *collectDistance_;
    // Collect1DNode for A-B angle histogram
    Collect1DProcedureNode *collectAngle_;
    // Collect2DNode for distance-angle data
    Collect2DProcedureNode *collectDAngle_;
    // Process1DNode for A-B RDF
    Process1DProcedureNode *processDistance_;
    // Process1DNode for A-B-C angle histogram
    Process1DProcedureNode *processAngle_;
    // Process2DNode for distance-angle data
    Process2DProcedureNode *processDAngle_;

    /*
     * GUI Widget
     */
    public:
    // Return a new widget controlling this Module
    ModuleWidget *createWidget(QWidget *parent, Dissolve &dissolve);
};
