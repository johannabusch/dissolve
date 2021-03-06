// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "base/lineparser.h"
#include "classes/configuration.h"
#include "gui/charts/moduleblock.h"
#include "gui/gui.h"
#include "gui/keywordwidgets/configurationreflist.h"
#include "gui/modulecontrolwidget.h"
#include "gui/modulewidget.h"
#include "keywordwidgets/configurationreflist.h"
#include "main/dissolve.h"
#include "module/module.h"
#include <QGridLayout>
#include <QLabel>

ModuleControlWidget::ModuleControlWidget(QWidget *parent)
{
    // Set up user interface
    ui_.setupUi(this);

    dissolve_ = nullptr;
    module_ = nullptr;
    configurationsWidget_ = nullptr;
    moduleWidget_ = nullptr;

    // Connect signals from keywords widget
    connect(ui_.ModuleKeywordsWidget, SIGNAL(dataModified()), this, SLOT(keywordDataModified()));
    connect(ui_.ModuleKeywordsWidget, SIGNAL(setUpRequired()), this, SLOT(setUpModule()));
}

ModuleControlWidget::~ModuleControlWidget() {}

/*
 * Module Target
 */

// Set target Module to display
void ModuleControlWidget::setModule(Module *module, Dissolve *dissolve)
{
    module_ = module;
    dissolve_ = dissolve;
    if ((!module_) || (!dissolve_))
    {
        ui_.ModuleKeywordsWidget->setUp(KeywordList(), CoreData());
        return;
    }

    // Set the icon name label
    ui_.ModuleNameLabel->setText(QString("%1 (%2)").arg(QString::fromStdString(std::string(module_->uniqueName())),
                                                        QString::fromStdString(std::string(module->type()))));
    ui_.ModuleIconLabel->setPixmap(ModuleBlock::modulePixmap(module_));

    // Set up our control widgets
    ui_.ModuleKeywordsWidget->setUp(module_->keywords(), dissolve_->constCoreData());

    // Create any additional controls offered by the Module
    moduleWidget_ = module->createWidget(nullptr, *dissolve_);
    if (moduleWidget_ == nullptr)
        Messenger::printVerbose("Module '%s' did not provide a valid controller widget.\n", module->type());
    else
    {
        ui_.ModuleControlsStack->addWidget(moduleWidget_);
        ui_.ModuleOutputButton->setEnabled(true);
    }

    updateControls();
}

// Return target Module for the widget
Module *ModuleControlWidget::module() const { return module_; }

// Run the set-up stage of the associated Module
void ModuleControlWidget::setUpModule()
{
    if ((!module_) || (!dissolve_))
        return;

    // Run the Module's set-up stage
    module_->setUp(*dissolve_, dissolve_->worldPool());

    if (moduleWidget_)
        moduleWidget_->updateControls(ModuleWidget::ResetGraphDataTargetsFlag);
}

/*
 * Update
 */

// Update controls within widget
void ModuleControlWidget::updateControls()
{
    if ((!module_) || (!dissolve_))
        return;

    Locker refreshLocker(refreshLock_);

    // Update keywords
    ui_.ModuleKeywordsWidget->updateControls();

    // Update additional controls (if they exist)
    if (moduleWidget_)
        moduleWidget_->updateControls();
}

// Disable sensitive controls
void ModuleControlWidget::disableSensitiveControls()
{
    ui_.ModuleKeywordsWidget->setEnabled(false);
    if (moduleWidget_)
        moduleWidget_->disableSensitiveControls();
}

// Enable sensitive controls
void ModuleControlWidget::enableSensitiveControls()
{
    ui_.ModuleKeywordsWidget->setEnabled(true);
    if (moduleWidget_)
        moduleWidget_->enableSensitiveControls();
}

/*
 * UI
 */

void ModuleControlWidget::on_ModuleControlsButton_clicked(bool checked)
{
    if (checked)
        ui_.ModuleControlsStack->setCurrentIndex(0);
}

void ModuleControlWidget::on_ModuleOutputButton_clicked(bool checked)
{
    if (checked)
        ui_.ModuleControlsStack->setCurrentIndex(1);
}

// Keyword data for Module has been modified
void ModuleControlWidget::keywordDataModified() { emit(dataModified()); }

/*
 * State I/O
 */

// Read widget state through specified LineParser
bool ModuleControlWidget::readState(LineParser &parser)
{
    Locker refreshLocker(refreshLock_);

    // Write currently-open page...
    if (parser.getArgsDelim() != LineParser::Success)
        return false;
    ui_.ModuleControlsStack->setCurrentIndex(parser.argi(0));
    if (parser.argi(0) == 0)
        ui_.ModuleControlsButton->setChecked(true);
    else if (parser.argi(0) == 1)
        ui_.ModuleOutputButton->setChecked(true);

    // Additional controls state
    if (moduleWidget_ && (!moduleWidget_->readState(parser)))
        return false;

    return true;
}

// Write widget state through specified LineParser
bool ModuleControlWidget::writeState(LineParser &parser) const
{
    // Write currently-open page...
    if (!parser.writeLineF("%i\n", ui_.ModuleControlsStack->currentIndex()))
        return false;

    // Additional controls state
    if (moduleWidget_ && (!moduleWidget_->writeState(parser)))
        return false;

    return true;
}
