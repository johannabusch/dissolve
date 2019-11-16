/*
	*** Dissolve GUI - Tabs
	*** src/gui/gui_tabs.cpp
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

#include "gui/gui.h"
#include "gui/configurationtab.h"
#include "gui/forcefieldtab.h"
#include "gui/layertab.h"
#include "gui/moduletab.h"
#include "gui/speciestab.h"
#include "gui/workspacetab.h"
#include "main/dissolve.h"
#include "classes/configuration.h"
#include "classes/species.h"
#include <QInputDialog>

/*
 * Private Slots
 */

void DissolveWindow::on_MainTabs_currentChanged(int index)
{
	if (refreshing_) return;

	// Retrieve the widget corresponding to the index provided - it will be a MainTab widget, from which all our tab widgets are derived
	MainTab* currentTab = dynamic_cast<MainTab*>(ui_.MainTabs->widget(index));
	if (!currentTab)
	{
		Messenger::print("Can't cast this tab widget (index %i) into a MainTab.\n", index);
		return;
	}

	// Update contents
	currentTab->updateControls();

	// Update menu items
	updateMenus();
}

/*
 * Private
 */

// Remove tabs related to the current data
void DissolveWindow::clearTabs()
{
	// Removal of the tab and widget will be handled by the destructors
	speciesTabs_.clear();
	configurationTabs_.clear();
	processingLayerTabs_.clear();
	moduleTabs_.clear();
}

// Reconcile tabs, making them consistent with the current data
void DissolveWindow::reconcileTabs()
{
	// Species - Global tab indices run from 1 (first tab after ForcefieldTab) to 1+nSpecies
	ListIterator<Species> speciesIterator(dissolve_.species());
	int currentTabIndex = 0;
	int baseIndex = 1;
	while (Species* sp = speciesIterator.iterate())
	{
		// Loop over existing tabs
		while (currentTabIndex < speciesTabs_.nItems())
		{
			// If the existing tab is displaying the current Species already, then we can move on. Otherwise, delete it
			if (speciesTabs_[currentTabIndex]->species() == sp) break;
			else speciesTabs_.remove(currentTabIndex);
		}

		// If the current tab index is (now) out of range, add a new one
		if (currentTabIndex == speciesTabs_.nItems())
		{
			SpeciesTab* newTab = new SpeciesTab(this, dissolve_, ui_.MainTabs, sp->name(), sp);
			speciesTabs_.own(newTab);
			ui_.MainTabs->insertTab(baseIndex + currentTabIndex, newTab, sp->name());
			ui_.MainTabs->setTabTextColour(newTab->page(), QColor(0, 81, 0));
			ui_.MainTabs->setTabIcon(newTab->page(), QIcon(":/tabs/icons/tabs_species.svg"));
		}

		++currentTabIndex;
	}
	baseIndex += dissolve_.nSpecies();

	// Configurations - Global tab indices run from 1+nSpecies (first tab after last Species) to 1+nSpecies+nConfigurations
	ListIterator<Configuration> configurationIterator(dissolve_.configurations());
	currentTabIndex = 0;
	while (Configuration* cfg = configurationIterator.iterate())
	{
		// Loop over existing tabs
		while (currentTabIndex < configurationTabs_.nItems())
		{
			// If the existing tab is displaying the current Configuration already, then we can move on. Otherwise, delete it
			if (configurationTabs_[currentTabIndex]->configuration() == cfg) break;
			else configurationTabs_.remove(currentTabIndex);
		}

		// If the current tab index is (now) out of range, add a new one
		if (currentTabIndex == configurationTabs_.nItems())
		{
			ConfigurationTab* newTab = new ConfigurationTab(this, dissolve_, ui_.MainTabs, cfg->name(), cfg);
			configurationTabs_.own(newTab);
			ui_.MainTabs->insertTab(baseIndex + currentTabIndex, newTab, cfg->name());
			ui_.MainTabs->setTabTextColour(newTab->page(), QColor(0, 81, 0));
			ui_.MainTabs->setTabIcon(newTab->page(), QIcon(":/tabs/icons/tabs_configuration.svg"));
		}

		++currentTabIndex;
	}
	baseIndex += dissolve_.nConfigurations();

	// Processing Layers - Global tab indices run from 1+nSpecies+nConfigurations (first tab after last Configuration) to 1+nSpecies+nConfigurations+nProcessingLayers
	ListIterator<ModuleLayer> processingLayerIterator(dissolve_.processingLayers());
	currentTabIndex = 0;
	while (ModuleLayer* layer = processingLayerIterator.iterate())
	{
		// Loop over existing tabs
		while (currentTabIndex < processingLayerTabs_.nItems())
		{
			// If the existing tab is displaying the current ModuleLayer already, then we can move on. Otherwise, delete it
			if (processingLayerTabs_[currentTabIndex]->moduleLayer() == layer) break;
			else processingLayerTabs_.remove(currentTabIndex);
		}

		// If the current tab index is (now) out of range, add a new one
		if (currentTabIndex == processingLayerTabs_.nItems())
		{
			LayerTab* newTab = new LayerTab(this, dissolve_, ui_.MainTabs, layer->name(), layer);
			processingLayerTabs_.own(newTab);
			ui_.MainTabs->insertTab(baseIndex + currentTabIndex, newTab, layer->name());
			ui_.MainTabs->setTabTextColour(newTab->page(), QColor(0, 81, 0));
			ui_.MainTabs->setTabIcon(newTab->page(), QIcon(":/tabs/icons/tabs_modulelayer.svg"));
		}

		++currentTabIndex;
	}
	baseIndex += dissolve_.nProcessingLayers();
}

// Add core (permanent) tabs
void DissolveWindow::addCoreTabs()
{
	// Forcefield
	forcefieldTab_ = new ForcefieldTab(this, dissolve_, ui_.MainTabs, "Forcefield");
	ui_.MainTabs->addTab(forcefieldTab_->page(), "Forcefield");
	ui_.MainTabs->setTabIcon(forcefieldTab_->page(), QIcon(":/tabs/icons/tabs_ff.svg"));
}

// Add on an empty workspace tab
MainTab* DissolveWindow::addWorkspaceTab(const char* title)
{
	// Check that a tab with this title doesn't already exist
	MainTab* tab = findTab(title);
	if (!tab)
	{
		WorkspaceTab* newWorkspace = new WorkspaceTab(this, dissolve_, ui_.MainTabs, title);
		workspaceTabs_.own(newWorkspace);

		// Add the new tab directly in to our tabs - it will not be managed in reconcileTabs().
		ui_.MainTabs->addTab(newWorkspace, title);
		ui_.MainTabs->addTabCloseButton(newWorkspace);
		ui_.MainTabs->setTabIcon(newWorkspace->page(), QIcon(":/tabs/icons/tabs_workspace.svg"));
		return newWorkspace;
	}
	else Messenger::printVerbose("Tab '%s' already exists, so returning that instead...\n", title);

	return tab;
}

// Find tab with title specified
MainTab* DissolveWindow::findTab(const char* title)
{
	if (DissolveSys::sameString(title, "Forcefield")) return forcefieldTab_;

	for (SpeciesTab* tab = speciesTabs_.first(); tab != NULL; tab = tab->next()) if (DissolveSys::sameString(title, tab->title())) return tab;
	for (ConfigurationTab* tab = configurationTabs_.first(); tab != NULL; tab = tab->next()) if (DissolveSys::sameString(title, tab->title())) return tab;
	for (LayerTab* tab = processingLayerTabs_.first(); tab != NULL; tab = tab->next()) if (DissolveSys::sameString(title, tab->title())) return tab;
	for (ModuleTab* tab = moduleTabs_.first(); tab != NULL; tab = tab->next()) if (DissolveSys::sameString(title, tab->title())) return tab;
	for (WorkspaceTab* tab = workspaceTabs_.first(); tab != NULL; tab = tab->next()) if (DissolveSys::sameString(title, tab->title())) return tab;

	return NULL;
}

// Find tab with specified page widget
MainTab* DissolveWindow::findTab(QWidget* page)
{
	if (forcefieldTab_ && (forcefieldTab_->page() == page)) return forcefieldTab_;

	MainTab* result = speciesTab(page);
	if (!result) result = configurationTab(page);
	if (!result) result = processingLayerTab(page);
	if (!result) result = moduleTab(page);
	if (!result) result = workspaceTab(page);

	return result;
}

// Find SpeciesTab containing specified page widget
SpeciesTab* DissolveWindow::speciesTab(QWidget* page)
{
	for (SpeciesTab* tab = speciesTabs_.first(); tab != NULL; tab = tab->next()) if (tab->page() == page) return tab;

	return NULL;
}

// Find ConfigurationTab containing specified page widget
ConfigurationTab* DissolveWindow::configurationTab(QWidget* page)
{
	for (ConfigurationTab* tab = configurationTabs_.first(); tab != NULL; tab = tab->next()) if (tab->page() == page) return tab;

	return NULL;
}

// Find LayerTab containing specified page widget
LayerTab* DissolveWindow::processingLayerTab(QWidget* page)
{
	for (LayerTab* tab = processingLayerTabs_.first(); tab != NULL; tab = tab->next()) if (tab->page() == page) return tab;

	return NULL;
}

// Find ModuleTab containing specified page widget
ModuleTab* DissolveWindow::moduleTab(QWidget* page)
{
	for (ModuleTab* tab = moduleTabs_.first(); tab != NULL; tab = tab->next()) if (tab->page() == page) return tab;

	return NULL;
}

// Find ModuleTab containing specified Module
ModuleTab* DissolveWindow::moduleTab(Module* module)
{
	for (ModuleTab* tab = moduleTabs_.first(); tab != NULL; tab = tab->next()) if (tab->module() == module) return tab;

	return NULL;
}

// Find WorkspaceTab containing specified page widget
WorkspaceTab* DissolveWindow::workspaceTab(QWidget* page)
{
	for (WorkspaceTab* tab = workspaceTabs_.first(); tab != NULL; tab = tab->next()) if (tab->page() == page) return tab;

	return NULL;
}

// Return current tab
MainTab* DissolveWindow::currentTab() const
{
	if (ui_.MainTabs->currentWidget() == NULL) return NULL;

	// Retrieve the widget corresponding to the index provided - it will be a MainTab widget, from which all our tab widgets are derived
	MainTab* currentTab = dynamic_cast<MainTab*>(ui_.MainTabs->currentWidget());
	if (!currentTab)
	{
		Messenger::print("Can't cast current tab (index %i) into a MainTab.\n", ui_.MainTabs->currentIndex());
		return NULL;
	}

	return currentTab;
}

// Make specified tab the current one
void DissolveWindow::setCurrentTab(MainTab* tab)
{
	ui_.MainTabs->setCurrentWidget(tab->page());
}

// Make specified tab the current one (by index)
void DissolveWindow::setCurrentTab(int tabIndex)
{
	ui_.MainTabs->setCurrentIndex(tabIndex);
}

// Show forcefield tab
void DissolveWindow::showForcefieldTab()
{
	if (forcefieldTab_) ui_.MainTabs->setCurrentWidget(forcefieldTab_->page());
	else Messenger::error("Can't show ForcefieldTab, since it doesn't exist!\n");
}

// Make specified Species tab the current one
void DissolveWindow::setCurrentTab(Species* species)
{
	if (!species) return;

	for (SpeciesTab* tab = speciesTabs_.first(); tab != NULL; tab = tab->next()) if (tab->species() == species)
	{
		ui_.MainTabs->setCurrentWidget(tab->page());
		return;
	}

	Messenger::error("Can't display SpeciesTab for Species '%s' as it doesn't exist.\n", species->name());
}

// Make specified Configuration tab the current one
void DissolveWindow::setCurrentTab(Configuration* cfg)
{
	if (!cfg) return;

	for (ConfigurationTab* tab = configurationTabs_.first(); tab != NULL; tab = tab->next()) if (tab->configuration() == cfg)
	{
		ui_.MainTabs->setCurrentWidget(tab->page());
		return;
	}

	Messenger::error("Can't display ConfigurationTab for Configuration '%s' as it doesn't exist.\n", cfg->name());
}

// Make specified processing layer tab the current one
void DissolveWindow::setCurrentTab(ModuleLayer* layer)
{
	if (!layer) return;

	for (LayerTab* tab = processingLayerTabs_.first(); tab != NULL; tab = tab->next()) if (tab->moduleLayer() == layer)
	{
		ui_.MainTabs->setCurrentWidget(tab->page());
		return;
	}

	Messenger::error("Can't display LayerTab for processing layer '%s' as it doesn't exist.\n", layer->name());
}

// Return reference list of all current tabs
RefList<MainTab> DissolveWindow::allTabs() const
{
	RefList<MainTab> tabs;

	tabs.append(forcefieldTab_);
	for (SpeciesTab* tab = speciesTabs_.first(); tab != NULL; tab = tab->next()) tabs.append(tab);
	for (ConfigurationTab* tab = configurationTabs_.first(); tab != NULL; tab = tab->next()) tabs.append(tab);
	for (LayerTab* tab = processingLayerTabs_.first(); tab != NULL; tab = tab->next()) tabs.append(tab);
	for (ModuleTab* tab = moduleTabs_.first(); tab != NULL; tab = tab->next()) tabs.append(tab);
	for (WorkspaceTab* tab = workspaceTabs_.first(); tab != NULL; tab = tab->next()) tabs.append(tab);

	return tabs;
}

// Return currently-selected Species (if a SpeciesTab is the current one)
Species* DissolveWindow::currentSpecies() const
{
	// Get the currently-selected tab, and make sure it's a SpeciesTab
	MainTab* tab = currentTab();
	if (tab->type() != MainTab::SpeciesTabType) return NULL;

	SpeciesTab* speciesTab = dynamic_cast<SpeciesTab*>(tab);
	return (speciesTab ? speciesTab->species() : NULL);
}

// Return currently-selected Configuration (if a ConfigurationTab is the current one)
Configuration* DissolveWindow::currentConfiguration() const
{
	// Get the currently-selected tab, and make sure it's a SpeciesTab
	MainTab* tab = currentTab();
	if (tab->type() != MainTab::ConfigurationTabType) return NULL;

	ConfigurationTab* configurationTab = dynamic_cast<ConfigurationTab*>(tab);
	return (configurationTab ? configurationTab->configuration() : NULL);
}

// Return currently-selected ModuleLayer (if a LayerTab is the current one)
ModuleLayer* DissolveWindow::currentLayer() const
{
	// Get the currently-selected tab, and make sure it's a SpeciesTab
	MainTab* tab = currentTab();
	if (tab->type() != MainTab::LayerTabType) return NULL;

	LayerTab* layerTab = dynamic_cast<LayerTab*>(tab);
	return (layerTab ? layerTab->moduleLayer() : NULL);
}

/*
 * Public
 */

// Create / go to Module tab for specified Module, provided it has a Module control widget
MainTab* DissolveWindow::addModuleTab(Module* module)
{
	// Does a tab for this Module already exist
	ModuleTab* tab = moduleTab(module);
	if (!tab)
	{
		// Need to create a new ModuleTab
		tab = new ModuleTab(this, dissolve_, ui_.MainTabs, module->uniqueName(), module);
		moduleTabs_.own(tab);
		ui_.MainTabs->addTab(tab->page(), module->uniqueName());

		// Add a close button
		ui_.MainTabs->addTabCloseButton(tab->page());

		// If we are currently running, disable the necessary controls in widget
		if (dissolveState_ == DissolveWindow::RunningState) tab->disableSensitiveControls();
	}

	setCurrentTab(tab);

	return tab;
}

// Remove the ModuleTab for the specifeid Module, if it exists
void DissolveWindow::removeModuleTab(Module* module)
{
	ModuleTab* tab = moduleTab(module);
	if (!tab) return;

	removeTab(tab);
}

/*
 * Public Slots
 */

// Add or go to Module tab for the Module with the unique name provided
void DissolveWindow::showModuleTab(const QString& uniqueName)
{
	// Find the Module
	Module* module = dissolve_.findModuleInstance(qPrintable(uniqueName));
	if (!module) return;

	addModuleTab(module);
}

// Remove the Module tab (if it exists) for the Module with the unique name provided
void DissolveWindow::removeModuleTab(const QString& uniqueName)
{
	// Find the Module
	Module* module = dissolve_.findModuleInstance(qPrintable(uniqueName));
	if (!module) return;

	removeModuleTab(module);
}

// Remove tab containing the specified page widget
void DissolveWindow::removeTab(QWidget* page)
{
	// Delete the tab from the tabwidget first - find its index (based on the page widget pointer) and remove that
	int indexToRemove = ui_.MainTabs->indexOf(page);
	if (indexToRemove == -1) printf("Couldn't remove tab since its page widget (%p) could not be found.\n", page);
	else ui_.MainTabs->removeTab(indexToRemove);

	// Now delete the tab from its list - this will delete the actual page widget
	if (speciesTab(page)) speciesTabs_.remove(speciesTab(page));
	else if (configurationTab(page)) configurationTabs_.remove(configurationTab(page));
	else if (processingLayerTab(page)) processingLayerTabs_.remove(processingLayerTab(page));
	else if (moduleTab(page)) moduleTabs_.remove(moduleTab(page));
	else if (workspaceTab(page)) workspaceTabs_.remove(workspaceTab(page));
	else printf("Couldn't remove tab %p as it could not be found in any list.\n", page);
}
