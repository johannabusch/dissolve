// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "classes/speciesintra.h"
#include "templates/array2d.h"
#include "templates/listitem.h"

// Forward Declarations
/* none */

/*
 * MasterIntra Definition
 */
class MasterIntra : public SpeciesIntra, public ListItem<MasterIntra>
{
    public:
    MasterIntra();
    ~MasterIntra();

    /*
     * Basic Data
     */
    protected:
    // Name for interaction
    std::string name_;
    // Type of this interaction
    SpeciesIntra::InteractionType type_;

    public:
    // Set name for interaction (if relevant)
    void setName(std::string_view name);
    // Return name for interaction
    std::string_view name() const;
    // Set type of interaction
    void setType(SpeciesIntra::InteractionType type);

    /*
     * SpeciesIntra Virtuals
     */
    public:
    // Set up any necessary parameters
    void setUp();
    // Calculate and return fundamental frequency for the interaction
    double fundamentalFrequency(double reducedMass) const;
    // Return type of this interaction
    SpeciesIntra::InteractionType type() const;
};
