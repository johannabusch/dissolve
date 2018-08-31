/*
	*** Analysis Node - Collect1D
	*** src/analyse/nodes/collect1d.cpp
	Copyright T. Youngs 2012-2018

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

#include "analyse/nodes/collect1d.h"
#include "analyse/nodecontextstack.h"
#include "analyse/nodes/calculate.h"
#include "math/data1d.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "templates/genericlisthelper.h"

// Constructor
AnalysisCollect1DNode::AnalysisCollect1DNode() : AnalysisNode()
{
	type_ = AnalysisNode::Collect1DNode;

	minimum_ = 0.0;
	maximum_ = 10.0;
	binWidth_ = 0.1;
}

// Destructor
AnalysisCollect1DNode::~AnalysisCollect1DNode()
{
}

/*
 * Node Keywords
 */

// Node Keywords (note ordering for efficiency)
const char* Collect1DNodeKeywords[] = { "EndCollect1D", "LabelX", "LabelY", "QuantityX", "RangeX" };

// Convert string to node keyword
AnalysisCollect1DNode::Collect1DNodeKeyword AnalysisCollect1DNode::collect1DNodeKeyword(const char* s)
{
	for (int nk=0; nk < AnalysisCollect1DNode::nCollect1DNodeKeywords; ++nk) if (DissolveSys::sameString(s, Collect1DNodeKeywords[nk])) return (AnalysisCollect1DNode::Collect1DNodeKeyword) nk;

	return AnalysisCollect1DNode::nCollect1DNodeKeywords;
}

// Convert node keyword to string
const char* AnalysisCollect1DNode::collect1DNodeKeyword(AnalysisCollect1DNode::Collect1DNodeKeyword nk)
{
	return Collect1DNodeKeywords[nk];
}

/*
 * Data
 */



/*
 * Execute
 */

// Prepare any necessary data, ready for execution
bool AnalysisCollect1DNode::prepare(Configuration* cfg, const char* dataPrefix, GenericList& targetList)
{
	// Construct our data name, and search for it in the supplied list
	bool created;
	Histogram1D& target = GenericListHelper<Histogram1D>::realise(targetList, name(), dataPrefix, GenericItem::InRestartFileFlag, &created);
	if (created)
	{
		Messenger::printVerbose("One-dimensional histogram data for '%s' was not in the target list, so it will now be initialised...\n", name());
		target.initialise(minimum_, maximum_, binWidth_);
	}

	// Zero the current bins, ready for the new pass
	target.zeroBins();

	// Store a pointer to the data
	histogram_ = &target;

	return true;
}

// Execute node, targetting the supplied Configuration
AnalysisNode::NodeExecutionResult AnalysisCollect1DNode::execute(ProcessPool& procPool, Configuration* cfg, const char* dataPrefix, GenericList& targetList)
{
#ifdef CHECKS
	if (!observable_)
	{
		Messenger::error("No AnalysisCalculateNode pointer set in AnalysisCollect1DNode '%s'.\n", name());
		return AnalysisNode::Failure;
	}
#endif
	// Bin the current value of the observable
	histogram_->bin(observable_->value());

	return AnalysisNode::Success;
}

// Finalise any necessary data after execution
bool AnalysisCollect1DNode::finalise(Configuration* cfg, const char* dataPrefix, GenericList& targetList)
{
#ifdef CHECKS
	if (!histogram_)
	{
		Messenger::error("No Data1D pointer set in AnalysisCollect1DNode '%s'.\n", name());
		return AnalysisNode::Failure;
	}
#endif
	// Accumulate the current binned data
	histogram_->accumulate();

	return true;
}

/*
 * Read / Write
 */

// Read structure from specified LineParser
bool AnalysisCollect1DNode::read(LineParser& parser, NodeContextStack& contextStack)
{
	// The current line in the parser must also contain a node name (which is not necessarily unique...)
	if (parser.nArgs() != 2) return Messenger::error("A Collect1D node must be given a suitable name.\n");
	setName(parser.argc(1));

	// Read until we encounter the EndCollect1D keyword, or we fail for some reason
	while (!parser.eofOrBlank())
	{
		// Read and parse the next line
		if (parser.getArgsDelim(LineParser::Defaults+LineParser::SkipBlanks+LineParser::StripComments) != LineParser::Success) return false;

		// Is the first argument on the current line a valid control keyword?
		Collect1DNodeKeyword nk = collect1DNodeKeyword(parser.argc(0));
		switch (nk)
		{
			case (Collect1DNodeKeyword::EndCollect1DKeyword):
				return true;
			case (Collect1DNodeKeyword::LabelXKeyword):
			case (Collect1DNodeKeyword::LabelYKeyword):
				axisLabels_[nk-Collect1DNodeKeyword::LabelXKeyword] = parser.argc(1);
				break;
			case (Collect1DNodeKeyword::QuantityXKeyword):
				// Determine observable from supplied argument
				observable_ = contextStack.calculateNodeInScope(parser.argc(1));
				if (!observable_) return Messenger::error("Unrecognised Calculate node '%s' given to %s keyword.\n", parser.argc(1), collect1DNodeKeyword(nk));
				break;
			case (Collect1DNodeKeyword::RangeXKeyword):
				// Check that we have the right number of arguments first...
				if (parser.nArgs() != 4) return Messenger::error("Collect1D node keyword '%s' expects exactly three arguments (%i given).\n", collect1DNodeKeyword(Collect1DNodeKeyword::RangeXKeyword), parser.nArgs() - 1);
				minimum_ = parser.argd(1);
				maximum_ = parser.argd(2);
				binWidth_ = parser.argd(3);
				break;
			case (Collect1DNodeKeyword::nCollect1DNodeKeywords):
				return Messenger::error("Unrecognised Collect1D node keyword '%s' found.\n", parser.argc(0));
				break;
			default:
				return Messenger::error("Epic Developer Fail - Don't know how to deal with the Collect1D node keyword '%s'.\n", parser.argc(0));
		}
	}

	return true;
}

// Write structure to specified LineParser
bool AnalysisCollect1DNode::write(LineParser& parser, const char* prefix)
{
	// TODO
}
