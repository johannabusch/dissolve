/*
	*** RDF Module
	*** src/modules/rdf/rdf.h
	Copyright T. Youngs 2012-2018

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_RDFMODULE_H
#define DUQ_RDFMODULE_H

#include "module/module.h"
#include "classes/partialset.h"
#include "classes/braggpeak.h"

// Forward Declarations
class PartialSet;
class Weights;

// RDF Module
class RDFModule : public Module
{
	public:
	// Constructor
	RDFModule();
	// Destructor
	~RDFModule();


	/*
	 * Instances
	 */
	protected:
	// List of all instances of this Module type
	static List<Module> instances_;

	public:
	// Return list of all created instances of this Module
	List<Module>& instances();
	// Create instance of this module
	Module* createInstance();


	/*
	 * Definition
	 */
	public:
	// Return name of module
	const char* name();
	// Return brief description of module
	const char* brief();
	// Return instance type for module
	InstanceType instanceType();
	// Return the maximum number of Configurations the Module can target (or -1 for any number)
	int nTargetableConfigurations();
	// Modules upon which this Module depends to have run first
	const char* dependentModules();
	// Set up supplied dependent module (only if it has been auto-added)
	bool setUpDependentModule(Module* depMod);


	/*
	 * Options
	 */
	protected:
	// Set up keywords for Module
	void setUpKeywords();
	// Parse complex keyword line, returning true (1) on success, false (0) for recognised but failed, and -1 for not recognised
	int parseComplexKeyword(ModuleKeywordBase* keyword, LineParser& parser, DUQ* duq, GenericList& targetList, const char* prefix);

	public:
	// Partial Calculation Method enum
	enum PartialsMethod { AutoMethod, TestMethod, SimpleMethod, CellsMethod, nPartialsMethods };
	// Convert character string to PartialsMethod
	static PartialsMethod partialsMethod(const char* s);
	// Return character string for PartialsMethod
	static const char* partialsMethod(PartialsMethod pm);
	// Averaging scheme enum
	enum AveragingScheme { SimpleAveraging, ExponentialAveraging, nAveragingSchemes };
	// Convert character string to AveragingScheme
	static AveragingScheme averagingScheme(const char* s);
	// Return character string for AveragingScheme
	static const char* averagingScheme(AveragingScheme as);


	/*
	 * Processing
	 */
	private:
	// Run main processing
	bool process(DUQ& duq, ProcessPool& procPool);

	public:
	// Whether the Module has a processing stage
	bool hasProcessing();


	/*
	 * Members / Functions
	 */
	private:
	// Calculate partial g(r) in serial with simple double-loop
	bool calculateGRTestSerial(Configuration* cfg, PartialSet& partialSet);
	// Calculate partial g(r) with optimised double-loop
	bool calculateGRSimple(ProcessPool& procPool, Configuration* cfg, PartialSet& partialSet);
	// Calculate partial g(r) utilising Cell neighbour lists
	bool calculateGRCells(ProcessPool& procPool, Configuration* cfg, PartialSet& partialSet);
	// Perform averaging of named partial g(r)
	bool performGRAveraging(GenericList& moduleData, const char* name, const char* prefix, int nSetsInAverage, RDFModule::AveragingScheme averagingScheme);

	public:
	// (Re)calculate partial g(r) for the specified Configuration
	bool calculateGR(ProcessPool& procPool, Configuration* cfg, RDFModule::PartialsMethod method, bool allIntra, bool& alreadyUpToDate);
	// Calculate smoothed/broadened partial g(r) from supplied partials
	static bool calculateUnweightedGR(PartialSet& originalgr, PartialSet& weightedgr, BroadeningFunction intraBroadening, int smoothing);
	// Test supplied PartialSets against each other
	static bool testReferencePartials(PartialSet& setA, PartialSet& setB, double testThreshold);
	// Test reference data against calculated PartialSet set
	static bool testReferencePartials(GenericList& sourceModuleData, const char* sourceModuleUniqueName, PartialSet& partialgr, const char* dataPrefix, double testThreshold);


	/*
	 * GUI Widget
	 */
	public:
	// Return a new widget controlling this Module
	ModuleWidget* createWidget(QWidget* parent, DUQ& dUQ);
};

#endif
