// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "data/ffimproperterm.h"
#include "data/ff.h"
#include "data/ffatomtype.h"

ForcefieldImproperTerm::ForcefieldImproperTerm(std::string_view typeI, std::string_view typeJ, std::string_view typeK,
                                               std::string_view typeL, SpeciesImproper::ImproperFunction form,
                                               const std::vector<double> parameters)
{
    typeI_ = typeI;
    typeJ_ = typeJ;
    typeK_ = typeK;
    typeL_ = typeL;
    form_ = form;
    parameters_ = parameters;
    if (!SpeciesImproper::improperFunctions().validNArgs(form, parameters_.size()))
        throw(std::runtime_error("Incorrect number of parameters in constructed ForcefieldImproperTerm."));
}

/*
 * Data
 */

// Return if this term matches the atom types supplied
bool ForcefieldImproperTerm::isMatch(const ForcefieldAtomType &i, const ForcefieldAtomType &j, const ForcefieldAtomType &k,
                                     const ForcefieldAtomType &l) const
{
    if (DissolveSys::sameWildString(typeI_, i.equivalentName()) && DissolveSys::sameWildString(typeJ_, j.equivalentName()) &&
        DissolveSys::sameWildString(typeK_, k.equivalentName()) && DissolveSys::sameWildString(typeL_, l.equivalentName()))
        return true;
    if (DissolveSys::sameWildString(typeL_, i.equivalentName()) && DissolveSys::sameWildString(typeK_, j.equivalentName()) &&
        DissolveSys::sameWildString(typeJ_, k.equivalentName()) && DissolveSys::sameWildString(typeI_, l.equivalentName()))
        return true;

    return false;
}

// Return functional form index of interaction
SpeciesImproper::ImproperFunction ForcefieldImproperTerm::form() const { return form_; }

// Return array of parameters
const std::vector<double> &ForcefieldImproperTerm::parameters() const { return parameters_; }
