/*
	*** CalculateSDF Module Widget - Functions
	*** src/modules/calculate_sdf/gui/modulewidget_funcs.cpp
	Copyright T. Youngs 2012-2019

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "modules/calculate_sdf/gui/modulewidget.h"
#include "modules/calculate_sdf/sdf.h"
#include "modules/calculate_avgmol/avgmol.h"
#include "gui/viewer/render/renderabledata3d.h"
#include "gui/viewer/render/renderablespecies.h"
#include "gui/helpers/comboboxupdater.h"
#include "classes/configuration.h"
#include "classes/coredata.h"

// Constructor
CalculateSDFModuleWidget::CalculateSDFModuleWidget(QWidget* parent, CalculateSDFModule* module, const CoreData& coreData) : ModuleWidget(parent), module_(module), coreData_(coreData)
{
	// Set up user interface
	ui_.setupUi(this);

	// Set up SDF graph
	sdfGraph_ = ui_.SDFPlotWidget->dataViewer();

	View& sdfView = sdfGraph_->view();
	sdfView.setViewType(View::NormalView);
	sdfView.axes().setTitle(0, "X, \\sym{angstrom}");
	sdfView.axes().setRange(0, -10.0, 10.0);
	sdfView.axes().setTitle(1, "Y, \\sym{angstrom}");
	sdfView.axes().setRange(1, -10.0, 10.0);
	sdfView.axes().setTitle(2, "Z, \\sym{angstrom}");
	sdfView.axes().setRange(2, -10.0, 10.0);
	sdfView.setAutoFollowType(View::AllAutoFollow);

	sdfRenderable_ = NULL;
	referenceMolecule_ = NULL;
	referenceMoleculeRenderable_ = NULL;

	// Add on "<None>" option for refernece molecule
	ui_.ReferenceMoleculeCombo->addItem("<None>", VariantPointer<Species>(NULL));

	setGraphDataTargets();

	refreshing_ = false;
}

/*
 * UI
 */

// Update controls within widget
void CalculateSDFModuleWidget::updateControls(int flags)
{
	// Update the viewer's toolbar
	ui_.SDFPlotWidget->updateToolbar();

	// Refresh the graph
	sdfGraph_->postRedisplay();

	// Get cutoff limits from data
	if (sdfRenderable_)
	{
		ui_.DataMinValueLabel->setText(QString::number(sdfRenderable_->valuesMin(), 'e', 4));
		ui_.DataMaxValueLabel->setText(QString::number(sdfRenderable_->valuesMax(), 'e', 4));
	}

	// Update available reference molecule combo
	RefDataList<Species,CharString> refMolecules;
	// -- Find available AvgMol results
	RefList<CalculateAvgMolModule> avgMolModules = coreData_.findModulesByClass<CalculateAvgMolModule>();
	RefListIterator<CalculateAvgMolModule> avgMolIterator(avgMolModules);
	while (CalculateAvgMolModule* module = avgMolIterator.iterate()) refMolecules.append(&module->averageSpecies(), CharString("%s (AvgMol)", module->averageSpecies().name()));
	// -- Add on current species
	ListIterator<Species> speciesIterator(coreData_.constSpecies());
	while (Species* sp = speciesIterator.iterate()) refMolecules.append(sp, CharString("%s (Species)", sp->name()));
	ComboBoxUpdater<Species> refMoleculeUpdater(ui_.ReferenceMoleculeCombo, refMolecules, referenceMolecule_, 1, 0);
}

/*
 * State I/O
 */

// Write widget state through specified LineParser
bool CalculateSDFModuleWidget::writeState(LineParser& parser) const
{
	// Write DataViewer sessions
	if (!sdfGraph_->writeSession(parser)) return false;

	return true;
}

// Read widget state through specified LineParser
bool CalculateSDFModuleWidget::readState(LineParser& parser)
{
	// Read DataViewer sessions
	if (!sdfGraph_->readSession(parser)) return false;

	return true;
}

/*
 * Widgets / Functions
 */

// Set data targets in graphs
void CalculateSDFModuleWidget::setGraphDataTargets()
{
	// Remove any current data
	sdfGraph_->clearRenderables();
	sdfRenderable_ = NULL;
	referenceMoleculeRenderable_ = NULL;

	if (!module_) return;

	// Loop over Configuration targets in Module
	RefListIterator<Configuration> configIterator(module_->targetConfigurations());
	while (Configuration* cfg = configIterator.iterate())
	{
		// Calculated SDF
		sdfRenderable_ = dynamic_cast<RenderableData3D*>(sdfGraph_->createRenderable(Renderable::Data3DRenderable, CharString("%s//Process3D//%s//SDF", module_->uniqueName(), cfg->niceName()), CharString("SDF//%s", cfg->niceName()), cfg->niceName()));
		if (sdfRenderable_)
		{
			sdfRenderable_->setColour(StockColours::BlueStockColour);

			double minValue = sdfRenderable_->valuesMin();
			double maxValue = sdfRenderable_->valuesMax();

			// Set cutoff limits and intial values in spin widgets
			ui_.LowerCutoffSpin->setRange(true, minValue, true, maxValue);
			ui_.LowerCutoffSpin->setValue((maxValue - minValue) * 0.5);
			ui_.LowerCutoffSpin->setSingleStep((maxValue-minValue)*0.05);
			ui_.UpperCutoffSpin->setRange(true, sdfRenderable_->valuesMin(), true, sdfRenderable_->valuesMax());
			ui_.UpperCutoffSpin->setValue(maxValue);
			ui_.UpperCutoffSpin->setSingleStep((maxValue-minValue)*0.05);

			sdfRenderable_->setLowerCutoff((maxValue - minValue) * 0.5);
			sdfRenderable_->setUpperCutoff(maxValue);
		}

		// Reference molecule
		if (referenceMolecule_) referenceMoleculeRenderable_ = dynamic_cast<RenderableSpecies*>(sdfGraph_->createRenderable(Renderable::SpeciesRenderable, referenceMolecule_->objectTag(), "Reference Molecule"));
	}
}

void CalculateSDFModuleWidget::on_LowerCutoffSpin_valueChanged(double value)
{
	if (!sdfRenderable_) return;

	sdfRenderable_->setLowerCutoff(value);

	sdfGraph_->postRedisplay();
}

void CalculateSDFModuleWidget::on_UpperCutoffSpin_valueChanged(double value)
{
	if (!sdfRenderable_) return;

	sdfRenderable_->setUpperCutoff(value);

	sdfGraph_->postRedisplay();
}

void CalculateSDFModuleWidget::on_ReferenceMoleculeCombo_currentIndexChanged(int index)
{
	// Check index...
	if (index == -1) referenceMolecule_ = NULL;
	else referenceMolecule_ = VariantPointer<Species>(ui_.ReferenceMoleculeCombo->currentData());

	setGraphDataTargets();
}