// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "data/ff.h"

// Forward Declarations
/* none */

// Py4OH Forcefield
class Forcefield_Ludwig_Py5 : public Forcefield
{
    public:
    Forcefield_Ludwig_Py5() = default;
    ~Forcefield_Ludwig_Py5() = default;

    /*
     * Set Up
     */
    protected:
    // Set up / create all forcefield terms
    bool setUp();

    /*
     * Definition
     */
    public:
    // Return name of Forcefield
    std::string_view name() const;
    // Return description for Forcefield
    std::string_view description() const;
    // Return short-range interaction style for AtomTypes
    Forcefield::ShortRangeType shortRangeType() const;
};
