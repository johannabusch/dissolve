// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "base/parameters.h"
#include "classes/speciesimproper.h"

// Forward Declarations
class Forcefield;
class ForcefieldAtomType;

// Forcefield Improper Term
class ForcefieldImproperTerm
{
    public:
    ForcefieldImproperTerm(std::string_view typeI = "", std::string_view typeJ = "", std::string_view typeK = "",
                           std::string_view typeL = "", SpeciesImproper::ImproperFunction form = SpeciesImproper::NoForm,
                           const std::vector<double> parameters = {});
    ~ForcefieldImproperTerm() = default;

    /*
     * Data
     */
    private:
    // Type names involved in interaction
    std::string typeI_, typeJ_, typeK_, typeL_;
    // Functional form of interaction
    SpeciesImproper::ImproperFunction form_;
    // Parameters for interaction
    std::vector<double> parameters_;

    public:
    // Return if this term matches the atom types supplied
    bool isMatch(const ForcefieldAtomType &i, const ForcefieldAtomType &j, const ForcefieldAtomType &k,
                 const ForcefieldAtomType &l) const;
    // Return functional form index of interaction
    SpeciesImproper::ImproperFunction form() const;
    // Return array of parameters
    const std::vector<double> &parameters() const;
};
