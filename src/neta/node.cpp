// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "base/messenger.h"
#include "base/sysfunc.h"
#include "neta/connection.h"
#include "neta/node.h"
#include "neta/presence.h"
#include "neta/ring.h"
#include "templates/reflist.h"

// Return enum options for NodeTypes
EnumOptions<NETANode::NodeType> NETANode::nodeTypes()
{
    static EnumOptionsList NETANodeTypes = EnumOptionsList()
                                           << EnumOption(BasicNode, "Basic") << EnumOption(ConnectionNode, "Connection")
                                           << EnumOption(LogicNode, "Logic") << EnumOption(PresenceNode, "Presence")
                                           << EnumOption(RingNode, "Ring") << EnumOption(RootNode, "Root");

    static EnumOptions<NETANode::NodeType> options("NodeTypes", NETANodeTypes);

    return options;
}

// Return enum options for ComparisonOperator
EnumOptions<NETANode::ComparisonOperator> NETANode::comparisonOperators()
{
    static EnumOptionsList ComparisonOperatorOptions =
        EnumOptionsList() << EnumOption(EqualTo, "=") << EnumOption(NotEqualTo, "!=") << EnumOption(GreaterThan, ">")
                          << EnumOption(LessThan, "<") << EnumOption(GreaterThanEqualTo, ">=")
                          << EnumOption(LessThanEqualTo, "<=");

    static EnumOptions<NETANode::ComparisonOperator> options("ComparisonOperator", ComparisonOperatorOptions);

    return options;
}

NETANode::NETANode(NETADefinition *parent, NETANode::NodeType type)
{
    reverseLogic_ = false;
    parent_ = parent;
    nodeType_ = type;
}

NETANode::~NETANode() { clear(); }

/*
 * Node Type and Parent
 */

// Return node type
NETANode::NodeType NETANode::nodeType() const { return nodeType_; }

// Return parent
NETADefinition *NETANode::parent() const { return parent_; }

/*
 * Atom Targets
 */

// Add element target to node
bool NETANode::addElementTarget(const Element &el)
{
    return Messenger::error("NETA {} does not accept element targets.\n", nodeTypes().keyword(nodeType_));
}

// Add forcefield type target to node
bool NETANode::addFFTypeTarget(const ForcefieldAtomType &ffType)
{
    return Messenger::error("NETA {} does not accept forcefield atomtype targets.\n", nodeTypes().keyword(nodeType_));
}

/*
 * Branching and Node Generation
 */

// Clear all nodes
void NETANode::clear() { branch_.clear(); }

// Create connectivity node in the branch
std::shared_ptr<NETAConnectionNode>
NETANode::createConnectionNode(std::vector<std::reference_wrapper<const Element>> targetElements,
                               std::vector<std::reference_wrapper<const ForcefieldAtomType>> targetAtomTypes)
{
    auto node = std::make_shared<NETAConnectionNode>(parent_, targetElements, targetAtomTypes);
    branch_.push_back(node);

    return node;
}

// Create presence node in the branch
std::shared_ptr<NETAPresenceNode>
NETANode::createPresenceNode(std::vector<std::reference_wrapper<const Element>> targetElements,
                             std::vector<std::reference_wrapper<const ForcefieldAtomType>> targetAtomTypes)
{
    auto node = std::make_shared<NETAPresenceNode>(parent_, targetElements, targetAtomTypes);
    branch_.push_back(node);

    return node;
}

// Create ring node in the branch
std::shared_ptr<NETARingNode> NETANode::createRingNode()
{
    // Create the new node and own it
    auto node = std::make_shared<NETARingNode>(parent_);
    branch_.push_back(node);

    return node;
}

/*
 * Modifiers
 */

// Return whether the specified modifier is valid for this node
bool NETANode::isValidModifier(std::string_view s) const { return false; }

// Set value and comparator for specified modifier
bool NETANode::setModifier(std::string_view modifier, ComparisonOperator op, int value) { return false; }

/*
 * Flags
 */

// Return whether the specified flag is valid for this node
bool NETANode::isValidFlag(std::string_view s) const { return false; }

// Set specified flag
bool NETANode::setFlag(std::string_view flag, bool state) { return false; }

/*
 * Value Comparison
 */

// Return result of comparison between values provided
bool NETANode::compareValues(int lhsValue, ComparisonOperator op, int rhsValue)
{
    auto result = false;

    switch (op)
    {
        case (EqualTo):
            result = (lhsValue == rhsValue);
            break;
        case (NotEqualTo):
            result = (lhsValue != rhsValue);
            break;
        case (GreaterThan):
            result = (lhsValue > rhsValue);
            break;
        case (LessThan):
            result = (lhsValue < rhsValue);
            break;
        case (GreaterThanEqualTo):
            result = (lhsValue >= rhsValue);
            break;
        case (LessThanEqualTo):
            result = (lhsValue <= rhsValue);
            break;
        default:
            Messenger::error("Unrecognised operator (id = {}) in NETANode::valueComparison.\n", op);
            break;
    }

    return result;
}

/*
 * Scoring
 */

// Set node to use reverse logic
void NETANode::setReverseLogic() { reverseLogic_ = true; }

// Evaluate the node and return its score
int NETANode::score(const SpeciesAtom *i, std::vector<const SpeciesAtom*> &atomData) const
{
    auto totalScore = 0;

    // Loop over branch nodes in sequence
    for (auto node : branch_)
    {
        // Get the score from the node, returning early if NoMatch is encountered
        auto nodeScore = node->score(i, atomData);
        if (nodeScore == NETANode::NoMatch)
            return NETANode::NoMatch;

        totalScore += nodeScore;
    }

    return totalScore;
}
