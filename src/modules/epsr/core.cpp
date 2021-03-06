// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "modules/epsr/epsr.h"

EPSRModule::EPSRModule() : Module(nRequiredTargets())
{
    // Initialise Module - set up keywords etc.
    initialise();
}

EPSRModule::~EPSRModule() {}

/*
 * Instances
 */

// Create instance of this module
Module *EPSRModule::createInstance() const { return new EPSRModule; }
