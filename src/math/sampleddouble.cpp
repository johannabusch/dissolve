/*
	*** SampledDouble
	*** src/base/sampleddouble.cpp
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

#include "math/sampleddouble.h"
#include "base/lineparser.h"
#include <math.h>

// Constructor
SampledDouble::SampledDouble()
{
	value_ = 0.0;
	count_ = 0;
	mean_ = 0.0;
	m2_ = 0.0;
}

/*
 * Data
 */

// Return current value
double SampledDouble::value() const
{
	return value_;
}

// Accumulate current value into statistics
void SampledDouble::accumulate()
{
	++count_;
	double delta = value_ - mean_;
	mean_ += delta / count_;
	m2_ += delta * (value_ - mean_);
}

// Return number of samples contributing to averages etc.
int SampledDouble::count() const
{
	return count_;
}

// Return mean of sampled data
double SampledDouble::mean() const
{
	return mean_;
}

// Return variance of sampled data
double SampledDouble::variance() const
{
	return (count_ < 2 ? 0.0 : m2_ / (count_ - 1));
}

// Return standard deviation of sampled data
double SampledDouble::stDev() const
{
	return (count_ < 2 ? 0.0 : sqrt(m2_ / count_ - 1));
}

/*
 * Operators
 */

// Conversion (double)
SampledDouble::operator double&()
{
	return value_;
}

// <<
void SampledDouble::operator<<(double x)
{
	value_ = x;
	accumulate();
}

/*
 * GenericItemBase Implementations
 */

// Return class name
const char* SampledDouble::itemClassName()
{
	return "SampledDouble";
}

// Write data through specified LineParser
bool SampledDouble::write(LineParser& parser)
{
	return parser.writeLineF("%f  %i  %f  %f\n", value_, count_, mean_, m2_);
}

// Read data through specified LineParser
bool SampledDouble::read(LineParser& parser)
{
	if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success) return false;
	value_ = parser.argd(0);
	count_ = parser.argi(1);
	mean_ = parser.argd(2);
	m2_ = parser.argd(3);

	return true;
}