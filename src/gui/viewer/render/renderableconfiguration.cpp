/*
	*** Renderable - Configuration
	*** src/gui/viewer/render/renderableconfiguration.cpp
	Copyright T. Youngs 2019

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

#include "gui/viewer/render/renderableconfiguration.h"
#include "gui/viewer/render/renderablegroupmanager.h"
#include "gui/viewer/render/view.h"
#include "data/elementcolours.h"
#include "classes/box.h"
#include "classes/speciesatom.h"
#include "classes/speciesbond.h"

// Constructor
RenderableConfiguration::RenderableConfiguration(const Configuration* source, const char* objectTag) : Renderable(Renderable::ConfigurationRenderable, objectTag), source_(source)
{
	// Set defaults
	displayStyle_ = LinesStyle;
	linesAtomRadius_ = 0.05;
	spheresAtomRadius_ = 0.3;
	spheresBondRadius_ = 0.1;

	// Create basic primitives
	atomPrimitive_ = createPrimitive(GL_TRIANGLES, false);
	atomPrimitive_->sphere(1.0, 8, 10);
	bondPrimitive_ = createPrimitive(GL_TRIANGLES, false);
	bondPrimitive_->cylinder(0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1, 8);
	unitCellPrimitive_ = createPrimitive(GL_LINES, false);
	unitCellPrimitive_->wireOrthorhomboid(1.0, 1.0, 1.0, 0.5, 0.5, 0.5);
	lineConfigurationPrimitive_ = createPrimitive(GL_LINES, true);
	lineInteractionPrimitive_ = createPrimitive(GL_LINES, true);
	lineInteractionPrimitive_->setNoInstances();
}

// Destructor
RenderableConfiguration::~RenderableConfiguration()
{
}

/*
 * Data
 */

// Return whether a valid data source is available (attempting to set it if not)
bool RenderableConfiguration::validateDataSource()
{
	// If there is no valid source set, attempt to set it now...
	if (!source_) source_ = Configuration::findObject(objectTag_);

	return source_;
}

// Return version of data
int RenderableConfiguration::dataVersion() const
{
	return (source_ ? source_->contentsVersion() : -99);
}

/*
 * Transform / Limits
 */

// Transform data according to current settings
void RenderableConfiguration::transformData()
{
	if (!source_) return;

	// If the transformed data are already up-to-date, no need to do anything
	if (transformDataVersion_ == dataVersion()) return;

	// Loop over Atoms, seeking extreme x, y, and z values
	const DynamicArray<Atom>& atoms = source_->constAtoms();
	for (int n=0; n<atoms.nItems(); ++n)
	{
		const Atom* i = atoms.constValue(n);
		if (n == 0)
		{
			transformMin_ = i->r();
			transformMax_ = i->r();
		}
		else
		{
			if (i->r().x < transformMin_.x) transformMin_.x = i->r().x;
			else if (i->r().x > transformMax_.x) transformMax_.x = i->r().x;
			if (i->r().y < transformMin_.y) transformMin_.y = i->r().y;
			else if (i->r().y > transformMax_.y) transformMax_.y = i->r().y;
			if (i->r().z < transformMin_.z) transformMin_.z = i->r().z;
			else if (i->r().z > transformMax_.z) transformMax_.z = i->r().z;
		}
	}

	axisTransformMinPositive_.x = transformMin_.x < 0.0 ? 0.01 : transformMin_.x;
	axisTransformMinPositive_.y = transformMin_.y < 0.0 ? 0.01 : transformMin_.y;
	axisTransformMinPositive_.z = transformMin_.z < 0.0 ? 0.01 : transformMin_.z;
	axisTransformMaxPositive_.x = transformMax_.x < 0.0 ? 1.0 : transformMax_.x;
	axisTransformMaxPositive_.y = transformMax_.y < 0.0 ? 1.0 : transformMax_.y;
	axisTransformMaxPositive_.z = transformMax_.z < 0.0 ? 1.0 : transformMax_.z;

	// Update the transformed data 'version'
	transformDataVersion_ = dataVersion();
}

// Calculate min/max y value over specified x range (if possible in the underlying data)
bool RenderableConfiguration::yRangeOverX(double xMin, double xMax, double& yMin, double& yMax)
{
	yMin = 0.0;
	yMax = 1.0;

	return true;
}

/*
 * Rendering Primitives
 */

// Create cylinder bond between supplied atoms in specified assembly
void RenderableConfiguration::createCylinderBond(PrimitiveAssembly& assembly, const Atom* i, const Atom* j, const Vec3<double> vij, bool drawFromAtoms, double radialScaling)
{
	Matrix4 A;
	Vec3<double> unit = vij;
	const double mag = unit.magAndNormalise();

	// Create rotation matrix for Bond
	A.setColumn(2, unit.x, unit.y, unit.z, 0.0);
	A.setColumn(0, unit.orthogonal(), 0.0);
	A.setColumn(1, unit * A.columnAsVec3(0), 0.0);
	A.columnMultiply(2, 0.5*mag);
	A.applyScaling(radialScaling, radialScaling, 1.0);

	// If drawing from individual Atoms, locate on each Atom and draw the bond halves from there. If not, locate to the bond centre.
	if (drawFromAtoms)
	{
		// Render half of Bond in colour of Atom j
		A.setTranslation(i->r());
		const float* colour = ElementColours::colour(j->speciesAtom()->element());
		assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

		// Render half of Bond in colour of Atom i
		A.setTranslation(j->r());
		A.columnMultiply(2,-1.0);
		colour = ElementColours::colour(i->speciesAtom()->element());
		assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);
	}
	else
	{
		A.setTranslation(i->r()+vij*0.5);

		// Render half of Bond in colour of Atom j
		const float* colour = ElementColours::colour(j->speciesAtom()->element());
		assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

		// Render half of Bond in colour of Atom i
		A.columnMultiply(2,-1.0);
		colour = ElementColours::colour(i->speciesAtom()->element());
		assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);
	}
}

// Recreate necessary primitives / primitive assemblies for the data
void RenderableConfiguration::recreatePrimitives(const View& view, const ColourDefinition& colourDefinition)
{
	Matrix4 A;
	const GLfloat* colour;
	const GLfloat colourBlack[4] = { 0.0, 0.0, 0.0, 1.0 };
	const Atom* i, *j;
	Vec3<double> ri, rj;

	// Clear existing data
	lineConfigurationPrimitive_->forgetAll();
	configurationAssembly_.clear();
	unitCellAssembly_.clear();

	// Grab the Configuration's Box and CellArray
	const Box* box = source_->box();
	const CellArray& cellArray = source_->constCells();

	// Render according to the current displayStyle
	if (displayStyle_ == LinesStyle)
	{
		// Set basic styling and content for assemblies
		configurationAssembly_.add(false, GL_LINE);
		configurationAssembly_.add(lineConfigurationPrimitive_, A);
		const SpeciesBond* b;

		// Draw Atoms
		const DynamicArray<Atom>& atoms = source_->constAtoms();
		for (int n=0; n<atoms.nItems(); ++n)
		{
			// Get the Atom pointer
			i = atoms.constValue(n);

			// If the atom has no bonds draw it as a 'cross'
			if (i->speciesAtom()->nBonds() == 0)
			{
				const Vec3<double> r = i->r();
				colour = ElementColours::colour(i->speciesAtom()->element());

				lineConfigurationPrimitive_->line(r.x - linesAtomRadius_, r.y, r.z, r.x + linesAtomRadius_, r.y, r.z, colour);
				lineConfigurationPrimitive_->line(r.x, r.y - linesAtomRadius_, r.z, r.x, r.y + linesAtomRadius_, r.z, colour);
				lineConfigurationPrimitive_->line(r.x, r.y, r.z - linesAtomRadius_, r.x, r.y, r.z + linesAtomRadius_, colour);
			}
			else
			{
				// Draw all bonds from this atom
				const PointerArray<SpeciesBond>& bonds = i->speciesAtom()->bonds();
				for (int n=0; n<bonds.nItems(); ++n)
				{
					b = bonds.at(n);

					// Blindly get partner Atom 'j' - don't check if it is the true partner, only if it is the same as 'i' (in which case we skip it, ensuring we draw every bond only once)
					j = i->molecule()->atom(b->indexJ());
					if (i == j) continue;

					ri = i->r();
					rj = j->r();

					// Determine half delta i-j for bond
					const Vec3<double> dij = (cellArray.useMim(i->cell(), j->cell()) ? box->minimumVector(ri, rj) : rj - ri) * 0.5;

					// Draw bond halves
					lineConfigurationPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z, ElementColours::colour(b->i()->element()));
					lineConfigurationPrimitive_->line(rj.x, rj.y, rj.z, rj.x - dij.x, rj.y - dij.y, rj.z - dij.z, ElementColours::colour(b->j()->element()));
				}
			}
		}
	}
	else if (displayStyle_ == SpheresStyle)
	{
		// Set basic styling for assemblies
		configurationAssembly_.add(true, GL_FILL);

		// Draw Atoms
		const DynamicArray<Atom>& atoms = source_->constAtoms();
		for (int n=0; n<atoms.nItems(); ++n)
		{
			const Atom* i = atoms.constValue(n);

			A.setIdentity();
			A.setTranslation(i->r());
			A.applyScaling(spheresAtomRadius_);

			// The atom itself
			colour = ElementColours::colour(i->speciesAtom()->element());
			configurationAssembly_.add(atomPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

			// Bonds from this atom
			const PointerArray<SpeciesBond>& bonds = i->speciesAtom()->bonds();
			for (int n=0; n<bonds.nItems(); ++n)
			{
				// Blindly get partner Atom 'j' - don't check if it is the true partner, only if it is the same as 'i' (in which case we skip it, ensuring we draw every bond only once)
				j = i->molecule()->atom(bonds.at(n)->indexJ());
				if (i == j) continue;

				if (cellArray.useMim(i->cell(), j->cell())) createCylinderBond(configurationAssembly_, i, j, box->minimumVector(i->r(), j->r()), true, spheresBondRadius_);
				else createCylinderBond(configurationAssembly_, i, j, j->r() - i->r(), false, spheresBondRadius_);
			}
		}
	}

	// Add unit cell
	A.setIdentity();
	A = source_->box()->axes();
	unitCellAssembly_.add(unitCellPrimitive_, A, colourBlack);
}

// Send primitives for rendering
const void RenderableConfiguration::sendToGL(const double pixelScaling)
{
	// Set appropriate lighting for the configuration and interaction assemblies
	if (displayStyle_ == LinesStyle) glDisable(GL_LIGHTING);
	else glEnable(GL_LIGHTING);
	configurationAssembly_.sendToGL(pixelScaling);
	interactionAssembly_.sendToGL(pixelScaling);

	// Draw unit cell
	glDisable(GL_LIGHTING);
	unitCellAssembly_.sendToGL(pixelScaling);
}

/*
 * Style
 */

// Display Style Keywords
const char* ConfigurationDisplayStyleKeywords[] = { "Lines", "Spheres" };

// Convert display style index to text string
const char* RenderableConfiguration::displayStyle(int id)
{
	if ((id < 0) || (id >= RenderableConfiguration::nDisplayStyles)) return "INVALID_STYLE";

	return ConfigurationDisplayStyleKeywords[id];
}

// Convert text string to display style index
int RenderableConfiguration::displayStyle(const char* s)
{
	for (int n=0; n<nDisplayStyles; ++n) if (DissolveSys::sameString(s, ConfigurationDisplayStyleKeywords[n])) return (RenderableConfiguration::DisplayStyle) n;

	return -1;
}
