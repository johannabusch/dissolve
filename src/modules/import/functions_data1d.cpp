/*
	*** Import Module - Data1D Functions
	*** src/modules/import/functions_data1d.cpp
	Copyright T. Youngs 2012-2019

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

#include "modules/import/import.h"
#include "math/data1d.h"
#include "math/filters.h"
#include "base/lineparser.h"

// Read Data1D in specified format
bool ImportModule::readData1D(Data1DImportFileFormat::Data1DImportFormat format, LineParser& parser, Data1D& data)
{
	// Check supplied format
	if (format == Data1DImportFileFormat::XYData1D) return readXYData1D(parser, data);
	else if (format == Data1DImportFileFormat::HistogramData1D) return readHistogramData1D(parser, data);
	else if (format == Data1DImportFileFormat::GudrunMintData1D) return readGudrunMintData1D(parser, data);

	Messenger::error("Don't know how to load Data1D of format '%s'.\n", Data1DImportFileFormat().format(format));

	return false;
}

// Read simple XY data from specified file
bool ImportModule::readXYData1D(LineParser& parser, Data1D& data)
{
	return data.load(parser);
}

// Read simple histogram data from specified file
bool ImportModule::readHistogramData1D(LineParser& parser, Data1D& data)
{
	// Read in the data first (assuming simple XY format)
	if (!data.load(parser)) return false;

	// Convert bin boundaries to centre-bin values
	Filters::convertBinBoundaries(data);

	return true;
}

// Read Gudrun merged interference cross-section (mint) data from specified file
bool ImportModule::readGudrunMintData1D(LineParser& parser, Data1D& data)
{
	// Read in the data first (assuming simple XY format)
	if (!data.load(parser)) return false;

	// Gudrun mint01 files are in histogram format, so convert bin boundaries to centre-bin values
	Filters::convertBinBoundaries(data);

	return true;
}
