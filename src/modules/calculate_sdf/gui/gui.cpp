// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "main/dissolve.h"
#include "modules/calculate_sdf/gui/modulewidget.h"
#include "modules/calculate_sdf/sdf.h"

// Return a new widget controlling this Module
ModuleWidget *CalculateSDFModule::createWidget(QWidget *parent, Dissolve &dissolve)
{
    return new CalculateSDFModuleWidget(parent, this, dissolve.coreData());
}
