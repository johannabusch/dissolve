/*
	*** Analysis Base Node
	*** src/analyse/nodes/node.cpp
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

#include "analyse/nodes/node.h"
#include "classes/site.h"
#include "base/messenger.h"
#include "base/sysfunc.h"

// Constructor
AnalysisNode::AnalysisNode() : ListItem<AnalysisNode>()
{
}

// Destructor
AnalysisNode::~AnalysisNode()
{
}

/*
 * Node Types
 */

// Control keywords
const char* NodeTypes[] = { "Calculate", "Collect1D", "Exclude", "Select", "Sequence" };

// Convert string to node type
AnalysisNode::NodeType AnalysisNode::nodeType(const char* s)
{
	for (int nt=0; nt < AnalysisNode::nNodeTypes; ++nt) if (DissolveSys::sameString(s, NodeTypes[nt])) return (AnalysisNode::NodeType) nt;

	return AnalysisNode::nNodeTypes;
}

// Convert node type to string
const char* AnalysisNode::nodeType(AnalysisNode::NodeType nt)
{
	return NodeTypes[nt];
}

// Return node type
AnalysisNode::NodeType AnalysisNode::type() const
{
	return type_;
}

// Set node name (and nice name)
void AnalysisNode::setName(const char* name)
{
	name_ = name;

	// Generate a nice name (i.e. no spaces, slashes etc.)
	niceName_ = DissolveSys::niceName(name_);
}

// Return node name
const char* AnalysisNode::name() const
{
	return name_.get();
}

// Return nice node name
const char* AnalysisNode::niceName() const
{
	return niceName_.get();
}

/*
 * Execute
 */

// Prepare any necessary data, ready for execution
bool AnalysisNode::prepare(Configuration* cfg, const char* dataPrefix, GenericList& targetList)
{
	return AnalysisNode::Success;
}

// Finalise any necessary data after execution
bool AnalysisNode::finalise(Configuration* cfg, const char* dataPrefix, GenericList& targetList)
{
	return AnalysisNode::Success;
}