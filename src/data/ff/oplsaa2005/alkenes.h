// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "data/ff/oplsaa2005/alkanes.h"

// Forward Declarations
/* none */

// OPLS-AA/2005 Alkenes Forcefield
class Forcefield_OPLSAA2005_Alkenes : public Forcefield_OPLSAA2005_Alkanes
{
    public:
    Forcefield_OPLSAA2005_Alkenes() = default;
    ~Forcefield_OPLSAA2005_Alkenes() = default;

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
};
