/*
	*** Data2D Store
	*** src/classes/data2dstore.h
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

#ifndef DISSOLVE_DATA2DSTORE_H
#define DISSOLVE_DATA2DSTORE_H

#include "math/data2d.h"
#include "io/import/data2d.h"
#include "templates/list.h"
#include "templates/refdatalist.h"

// Forward Declarations
/* none */

// Data2D Store
class Data2DStore
{
	public:
	// Constructor
	Data2DStore();
	// Destructor
	~Data2DStore();


	/*
	 * Data
	 */
	private:
	// List of contained data
	List<Data2D> data_;
	// References for Data2D and associated file/format
	RefDataList<Data2D,Data2DImportFileFormat> dataReferences_;

	public:
	// Load  data into store
	bool addData(Data2DImportFileFormat fileAndFormat, const char* name);
	// Load data into store using specified pool
	bool addData(ProcessPool& pool, Data2DImportFileFormat fileAndFormat, const char* name);
	// Check to see if the named data is present in the store
	bool containsData(const char* name) const;
	// Return named data
	const Data2D& data(const char* name) const;
	// Return list of all data
	const List<Data2D>& data() const;
	// Return list of all data references
	const RefDataList<Data2D,Data2DImportFileFormat>& dataReferences() const;
};

#endif