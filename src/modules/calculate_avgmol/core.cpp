// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "modules/calculate_avgmol/avgmol.h"

CalculateAvgMolModule::CalculateAvgMolModule() : Module(nRequiredTargets())
{
    // Set unique name for this instance of the Module
    static int instanceId = 0;
    uniqueName_ = fmt::format("{}{:02d}", type(), instanceId++);

    // Initialise Module - set up keywords etc.
    initialise();
}

CalculateAvgMolModule::~CalculateAvgMolModule() {}

/*
 * Instances
 */

// Create instance of this module
Module *CalculateAvgMolModule::createInstance() const { return new CalculateAvgMolModule; }
