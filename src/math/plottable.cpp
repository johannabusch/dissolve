// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "math/plottable.h"
#include "templates/array2d.h"
#include "templates/array3d.h"

PlottableData::PlottableData(PlottableData::PlottableDataType type) { type_ = type; }

/*
 * Basic Information
 */

// Set name of plottable
void PlottableData::setName(std::string_view name) { name_ = name; }

// Return name of plottable
std::string_view PlottableData::name() const { return name_; }

/*
 * Axis Information
 */

// Return y axis Array (const)
const Array<double> &PlottableData::constYAxis() const
{
    Messenger::error("Tried to retrieve y axis array from a PlottableData that doesn't have a y axis.\n");
    static Array<double> dummy;
    return dummy;
}

// Return z axis Array (const)
const Array<double> &PlottableData::constZAxis() const
{
    Messenger::error("Tried to retrieve z axis array from a PlottableData that doesn't have a z axis.\n");
    static Array<double> dummy;
    return dummy;
}

/*
 * Values / Errors
 */

// Return values Array
const Array<double> &PlottableData::constValues() const
{
    Messenger::error(
        "Tried to retrieve a one-dimensional value array from a PlottableData that doesn't know how to return one.\n");
    static Array<double> dummy;
    return dummy;
}

// Return values Array
const Array2D<double> &PlottableData::constValues2D() const
{
    Messenger::error(
        "Tried to retrieve a two-dimensional value array from a PlottableData that doesn't know how to return one.\n");
    static Array2D<double> dummy;
    return dummy;
}

// Return three-dimensional values Array
const Array3D<double> &PlottableData::constValues3D() const
{
    Messenger::error("Tried to retrieve a three-dimensional value array from a PlottableData that doesn't know how to "
                     "return one.\n");
    static Array3D<double> dummy;
    return dummy;
}

// Return whether the values have associated errors
bool PlottableData::valuesHaveErrors() const { return false; }

// Return errors Array
const Array<double> &PlottableData::constErrors() const
{
    Messenger::error(
        "Tried to retrieve a one-dimensional error array from a PlottableData that doesn't know how to return one.\n");
    static Array<double> dummy;
    return dummy;
}

// Return errors Array
const Array2D<double> &PlottableData::constErrors2D() const
{
    Messenger::error(
        "Tried to retrieve a two-dimensional error array from a PlottableData that doesn't know how to return one.\n");
    static Array2D<double> dummy;
    return dummy;
}

// Return three-dimensional errors Array
const Array3D<double> &PlottableData::constErrors3D() const
{
    Messenger::error("Tried to retrieve a three-dimensional error array from a PlottableData that doesn't know how to "
                     "return one.\n");
    static Array3D<double> dummy;
    return dummy;
}
