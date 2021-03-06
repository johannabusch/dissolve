// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "gui/modulewidget.h"
#include "modules/calculate_rdf/gui/ui_modulewidget.h"

// Forward Declarations
class CalculateRDFModule;
class DataViewer;

// Module Widget
class CalculateRDFModuleWidget : public ModuleWidget
{
    // All Qt declarations derived from QObject must include this macro
    Q_OBJECT

    private:
    // Associated Module
    CalculateRDFModule *module_;
    // DataViewer contained within this widget
    DataViewer *rdfGraph_;

    public:
    CalculateRDFModuleWidget(QWidget *parent, CalculateRDFModule *module);

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::CalculateRDFModuleWidget ui_;

    public:
    // Update controls within widget
    void updateControls(int flags = ModuleWidget::DefaultUpdateFlag);

    /*
     * State I/O
     */
    public:
    // Write widget state through specified LineParser
    bool writeState(LineParser &parser) const;
    // Read widget state through specified LineParser
    bool readState(LineParser &parser);

    /*
     * Widgets / Functions
     */
    private:
    // Set data targets in graphs
    void setGraphDataTargets();

    private slots:
};
