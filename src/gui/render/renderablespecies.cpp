// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#include "gui/render/renderablespecies.h"
#include "base/lineparser.h"
#include "classes/atom.h"
#include "data/elementcolours.h"
#include "gui/render/renderablegroupmanager.h"
#include "gui/render/view.h"

RenderableSpecies::RenderableSpecies(const Species *source, std::string_view objectTag)
    : Renderable(Renderable::SpeciesRenderable, objectTag), source_(source)
{
    // Set defaults
    displayStyle_ = SpheresStyle;
    linesAtomRadius_ = 0.05;
    spheresAtomRadius_ = 0.3;
    spheresBondRadius_ = 0.1;

    // Create basic primitives
    atomPrimitive_ = createPrimitive(GL_TRIANGLES, false);
    atomPrimitive_->sphere(1.0, 8, 10);
    selectedAtomPrimitive_ = createPrimitive(GL_LINES, false);
    selectedAtomPrimitive_->sphere(1.25, 8, 10);
    bondPrimitive_ = createPrimitive(GL_TRIANGLES, false);
    bondPrimitive_->cylinder(0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1, 8);
    unitCellPrimitive_ = createPrimitive(GL_LINES, false);
    // 	unitCellPrimitive_->wireCube(1.0, 4, 0, 0, 0);
    lineSpeciesPrimitive_ = createPrimitive(GL_LINES, true);
    lineSelectionPrimitive_ = createPrimitive(GL_LINES, true);
    lineSelectionPrimitive_->setNoInstances();
    lineInteractionPrimitive_ = createPrimitive(GL_LINES, true);
    lineInteractionPrimitive_->setNoInstances();

    // Set versions
    selectionPrimitiveVersion_ = -1;
}

RenderableSpecies::~RenderableSpecies() {}

/*
 * Data
 */

// Return whether a valid data source is available (attempting to set it if not)
bool RenderableSpecies::validateDataSource()
{
    // Don't try to access source_ if we are not currently permitted to do so
    if (!sourceDataAccessEnabled_)
        return false;

    // If there is no valid source set, attempt to set it now...
    if (!source_)
        source_ = Species::findObject(objectTag_);

    return source_;
}

// Invalidate the current data source
void RenderableSpecies::invalidateDataSource() { source_ = nullptr; }

// Return version of data
int RenderableSpecies::dataVersion() { return (validateDataSource() ? source_->version() : -99); }

/*
 * Transform / Limits
 */

// Transform data according to current settings
void RenderableSpecies::transformValues()
{
    if (!source_)
        return;

    // If the transformed data are already up-to-date, no need to do anything
    if (valuesTransformDataVersion_ == dataVersion())
        return;

    // Loop over Atoms, seeking extreme x, y, and z values
    ListIterator<SpeciesAtom> atomIterator(source_->atoms());
    while (SpeciesAtom *i = atomIterator.iterate())
    {
        if (atomIterator.isFirst())
        {
            limitsMin_ = i->r();
            limitsMax_ = i->r();
        }
        else
        {
            if (i->r().x < limitsMin_.x)
                limitsMin_.x = i->r().x;
            else if (i->r().x > limitsMax_.x)
                limitsMax_.x = i->r().x;
            if (i->r().y < limitsMin_.y)
                limitsMin_.y = i->r().y;
            else if (i->r().y > limitsMax_.y)
                limitsMax_.y = i->r().y;
            if (i->r().z < limitsMin_.z)
                limitsMin_.z = i->r().z;
            else if (i->r().z > limitsMax_.z)
                limitsMax_.z = i->r().z;
        }
    }

    // Need to add on a little extra to the limits since the atoms have a radius
    limitsMin_ -= 1.0;
    limitsMax_ += 1.0;

    positiveLimitsMin_.x = limitsMin_.x < 0.0 ? 0.01 : limitsMin_.x;
    positiveLimitsMin_.y = limitsMin_.y < 0.0 ? 0.01 : limitsMin_.y;
    positiveLimitsMin_.z = limitsMin_.z < 0.0 ? 0.01 : limitsMin_.z;
    positiveLimitsMax_.x = limitsMax_.x < 0.0 ? 1.0 : limitsMax_.x;
    positiveLimitsMax_.y = limitsMax_.y < 0.0 ? 1.0 : limitsMax_.y;
    positiveLimitsMax_.z = limitsMax_.z < 0.0 ? 1.0 : limitsMax_.z;

    // Update the transformed data 'version'
    valuesTransformDataVersion_ = dataVersion();
}

/*
 * Rendering Primitives
 */

// Create cylinder bond between supplied atoms in specified assembly
void RenderableSpecies::createCylinderBond(PrimitiveAssembly &assembly, const SpeciesAtom *i, const SpeciesAtom *j,
                                           double radialScaling)
{
    Matrix4 A;

    // Get vector between Atoms i->j and move to Bond centre
    Vec3<double> vij = j->r() - i->r();
    A.setTranslation(i->r() + vij * 0.5);
    const auto mag = vij.magAndNormalise();

    // Create rotation matrix for Bond
    A.setColumn(2, vij.x, vij.y, vij.z, 0.0);
    A.setColumn(0, vij.orthogonal(), 0.0);
    A.setColumn(1, vij * A.columnAsVec3(0), 0.0);
    A.columnMultiply(2, 0.5 * mag);
    A.applyScaling(radialScaling, radialScaling, 1.0);

    // Render half of Bond in colour of Atom j
    const float *colour = ElementColours::colour(j->element());
    assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

    // Render half of Bond in colour of Atom i
    A.columnMultiply(2, -1.0);
    colour = ElementColours::colour(i->element());
    assembly.add(bondPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);
}

// Recreate necessary primitives / primitive assemblies for the data
void RenderableSpecies::recreatePrimitives(const View &view, const ColourDefinition &colourDefinition)
{
    Matrix4 A;
    const GLfloat *colour;
    const GLfloat colourBlack[4] = {0.0, 0.0, 0.0, 1.0};

    // Clear existing data
    lineSpeciesPrimitive_->forgetAll();
    lineSelectionPrimitive_->forgetAll();
    speciesAssembly_.clear();
    selectionAssembly_.clear();

    // Check data source
    if (!validateDataSource())
        return;

    // Render according to the current displayStyle
    if (displayStyle_ == LinesStyle)
    {
        // Set basic styling and content for assemblies
        speciesAssembly_.add(false, GL_LINE);
        speciesAssembly_.add(lineSpeciesPrimitive_, A);

        // Draw Atoms
        ListIterator<SpeciesAtom> atomIterator(source_->atoms());
        while (SpeciesAtom *i = atomIterator.iterate())
        {
            // Only draw the atom if it has no bonds, in which case draw it as a 'cross'
            if (i->nBonds() != 0)
                continue;

            const auto r = i->r();
            colour = ElementColours::colour(i->element());

            lineSpeciesPrimitive_->line(r.x - linesAtomRadius_, r.y, r.z, r.x + linesAtomRadius_, r.y, r.z, colour);
            lineSpeciesPrimitive_->line(r.x, r.y - linesAtomRadius_, r.z, r.x, r.y + linesAtomRadius_, r.z, colour);
            lineSpeciesPrimitive_->line(r.x, r.y, r.z - linesAtomRadius_, r.x, r.y, r.z + linesAtomRadius_, colour);
        }

        // Draw bonds
        for (const auto &bond : source_->constBonds())
        {
            // Determine half delta i-j for bond
            const auto ri = bond.i()->r();
            const auto rj = bond.j()->r();
            const auto dij = (rj - ri) * 0.5;

            // Draw bond halves
            lineSpeciesPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z,
                                        ElementColours::colour(bond.i()->element()));
            lineSpeciesPrimitive_->line(rj.x, rj.y, rj.z, rj.x - dij.x, rj.y - dij.y, rj.z - dij.z,
                                        ElementColours::colour(bond.j()->element()));
        }
    }
    else if (displayStyle_ == SpheresStyle)
    {
        // Set basic styling for assemblies
        speciesAssembly_.add(true, GL_FILL);
        selectionAssembly_.add(true, GL_LINE);

        // Draw Atoms
        ListIterator<SpeciesAtom> atomIterator(source_->atoms());
        while (SpeciesAtom *i = atomIterator.iterate())
        {
            A.setIdentity();
            A.setTranslation(i->r());
            A.applyScaling(spheresAtomRadius_);

            // The atom itself
            colour = ElementColours::colour(i->element());
            speciesAssembly_.add(atomPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

            // Is the atom selected?
            if (i->isSelected())
            {
                selectionAssembly_.add(selectedAtomPrimitive_, A, colourBlack[0], colourBlack[1], colourBlack[2],
                                       colourBlack[3]);
            }
        }

        // Draw bonds
        for (const auto &bond : source_->constBonds())
            createCylinderBond(speciesAssembly_, bond.i(), bond.j(), spheresBondRadius_);
    }
}

// Send primitives for rendering
const void RenderableSpecies::sendToGL(const double pixelScaling)
{
    // Set appropriate lighting for the species and interaction assemblies
    if (displayStyle_ == LinesStyle)
        glDisable(GL_LIGHTING);
    else
        glEnable(GL_LIGHTING);
    speciesAssembly_.sendToGL(pixelScaling);
    interactionAssembly_.sendToGL(pixelScaling);

    // Selection assembly is always drawn with no lighting as it comprises only of lines
    glDisable(GL_LIGHTING);
    selectionAssembly_.sendToGL(pixelScaling);
}

// Recreate selection Primitive
void RenderableSpecies::recreateSelectionPrimitive()
{
    if (selectionPrimitiveVersion_ == source_->atomSelectionVersion())
        return;

    // Clear existing data
    selectionAssembly_.clear();
    lineSelectionPrimitive_->forgetAll();

    const GLfloat *colour;
    Matrix4 A;

    if (displayStyle_ == LinesStyle)
    {
        // Set basic styling for assemblies
        selectionAssembly_.add(false, GL_LINE);
        selectionAssembly_.add(LineStyle(3.0));
        selectionAssembly_.add(lineSelectionPrimitive_, A);

        // Draw selection
        ListIterator<SpeciesAtom> atomIterator(source_->atoms());
        while (SpeciesAtom *i = atomIterator.iterate())
        {
            // If not selected, continue
            if (!i->isSelected())
                continue;

            // Get element colour
            colour = ElementColours::colour(i->element());

            // If the atom has no bonds, draw it as a 'cross', otherwise render all bond halves
            if (i->nBonds() == 0)
            {
                const auto r = i->r();

                lineSelectionPrimitive_->line(r.x - linesAtomRadius_, r.y, r.z, r.x + linesAtomRadius_, r.y, r.z, colour);
                lineSelectionPrimitive_->line(r.x, r.y - linesAtomRadius_, r.z, r.x, r.y + linesAtomRadius_, r.z, colour);
                lineSelectionPrimitive_->line(r.x, r.y, r.z - linesAtomRadius_, r.x, r.y, r.z + linesAtomRadius_, colour);
            }
            else
            {
                // Draw all bonds from this atom
                for (const SpeciesBond &bond : i->bonds())
                {
                    const auto ri = i->r();
                    const auto dij = (bond.partner(i)->r() - ri) * 0.5;

                    // Draw bond halves
                    lineSelectionPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z, colour);
                }
            }
        }
    }
    else if (displayStyle_ == SpheresStyle)
    {
        // Set basic styling
        selectionAssembly_.add(true, GL_LINE);

        ListIterator<SpeciesAtom> atomIterator(source_->atoms());
        while (SpeciesAtom *i = atomIterator.iterate())
        {
            if (!i->isSelected())
                continue;

            A.setIdentity();
            A.setTranslation(i->r());
            A.applyScaling(spheresAtomRadius_);

            selectionAssembly_.add(selectedAtomPrimitive_, A, 0.5, 0.5, 0.5, 1.0);
        }
    }

    selectionPrimitiveVersion_ = source_->atomSelectionVersion();
}

// Clear interaction Primitive
void RenderableSpecies::clearInteractionPrimitive()
{
    interactionAssembly_.clear();
    lineInteractionPrimitive_->forgetAll();
}

// Recreate interaction Primitive to display drawing interaction (from existing atom to existing atom)
void RenderableSpecies::recreateDrawInteractionPrimitive(SpeciesAtom *fromAtom, SpeciesAtom *toAtom)
{
    // Clear existing data
    clearInteractionPrimitive();

    Matrix4 A;

    // Render based on the current drawing style
    if (displayStyle_ == LinesStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(false, GL_LINE);
        interactionAssembly_.add(lineInteractionPrimitive_, A);

        // Determine half delta i-j for bond
        const auto ri = fromAtom->r();
        const auto rj = toAtom->r();
        const auto dij = (rj - ri) * 0.5;

        // Draw bond halves
        lineInteractionPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z,
                                        ElementColours::colour(fromAtom->element()));
        lineInteractionPrimitive_->line(rj.x, rj.y, rj.z, rj.x - dij.x, rj.y - dij.y, rj.z - dij.z,
                                        ElementColours::colour(toAtom->element()));
    }
    else if (displayStyle_ == SpheresStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(true, GL_FILL);

        // Draw the temporary bond between the atoms
        createCylinderBond(interactionAssembly_, fromAtom, toAtom, spheresBondRadius_);
    }
}

// Recreate interaction Primitive to display drawing interaction (from existing atom to point)
void RenderableSpecies::recreateDrawInteractionPrimitive(SpeciesAtom *fromAtom, Vec3<double> toPoint, Element *toElement)
{
    // Clear existing data
    clearInteractionPrimitive();

    Matrix4 A;

    // Temporary SpeciesAtom
    static SpeciesAtom j;
    j.setElement(toElement);
    j.setCoordinates(toPoint);

    // Render based on the current drawing style
    if (displayStyle_ == LinesStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(false, GL_LINE);
        interactionAssembly_.add(lineInteractionPrimitive_, A);

        // Determine half delta i-j for bond
        const auto ri = fromAtom->r();
        const auto rj = j.r();
        const auto dij = (rj - ri) * 0.5;

        // Draw bond halves
        lineInteractionPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z,
                                        ElementColours::colour(fromAtom->element()));
        lineInteractionPrimitive_->line(rj.x, rj.y, rj.z, rj.x - dij.x, rj.y - dij.y, rj.z - dij.z,
                                        ElementColours::colour(j.element()));
    }
    else if (displayStyle_ == SpheresStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(true, GL_FILL);

        // Draw the temporary atom
        A.setTranslation(j.r());
        A.applyScaling(spheresAtomRadius_);
        const float *colour = ElementColours::colour(j.element());
        interactionAssembly_.add(atomPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

        // Draw the temporary bond between the atoms
        createCylinderBond(interactionAssembly_, fromAtom, &j, spheresBondRadius_);
    }
}

// Recreate interaction Primitive to display drawing interaction (from point to point)
void RenderableSpecies::recreateDrawInteractionPrimitive(Vec3<double> fromPoint, Element *fromElement, Vec3<double> toPoint,
                                                         Element *toElement)
{
    // Clear existing data
    clearInteractionPrimitive();

    Matrix4 A;

    // Temporary SpeciesAtoms
    static SpeciesAtom i, j;
    i.setElement(fromElement);
    i.setCoordinates(fromPoint);
    j.setElement(toElement);
    j.setCoordinates(toPoint);

    // Render based on the current drawing style
    if (displayStyle_ == LinesStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(false, GL_LINE);
        interactionAssembly_.add(lineInteractionPrimitive_, A);

        // Determine half delta i-j for bond
        const auto ri = i.r();
        const auto rj = j.r();
        const auto dij = (rj - ri) * 0.5;

        // Draw bond halves
        lineInteractionPrimitive_->line(ri.x, ri.y, ri.z, ri.x + dij.x, ri.y + dij.y, ri.z + dij.z,
                                        ElementColours::colour(i.element()));
        lineInteractionPrimitive_->line(rj.x, rj.y, rj.z, rj.x - dij.x, rj.y - dij.y, rj.z - dij.z,
                                        ElementColours::colour(j.element()));
    }
    else if (displayStyle_ == SpheresStyle)
    {
        // Set basic styling for assembly
        interactionAssembly_.add(true, GL_FILL);

        // Draw the temporary atoms
        A.setTranslation(i.r());
        A.applyScaling(spheresAtomRadius_);
        const float *colour = ElementColours::colour(i.element());
        interactionAssembly_.add(atomPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

        A.setIdentity();
        A.setTranslation(j.r());
        A.applyScaling(spheresAtomRadius_);
        colour = ElementColours::colour(j.element());
        interactionAssembly_.add(atomPrimitive_, A, colour[0], colour[1], colour[2], colour[3]);

        // Draw the temporary bond between the atoms
        createCylinderBond(interactionAssembly_, &i, &j, spheresBondRadius_);
    }
}

/*
 * Style
 */

// Return EnumOptions for SpeciesDisplayStyle
EnumOptions<RenderableSpecies::SpeciesDisplayStyle> RenderableSpecies::speciesDisplayStyles()
{
    static EnumOptionsList SpeciesStyleOptions = EnumOptionsList() << EnumOption(RenderableSpecies::LinesStyle, "Lines")
                                                                   << EnumOption(RenderableSpecies::SpheresStyle, "Spheres");

    static EnumOptions<RenderableSpecies::SpeciesDisplayStyle> options("SpeciesDisplayStyle", SpeciesStyleOptions);

    return options;
}

// Set display style for renderable
void RenderableSpecies::setDisplayStyle(SpeciesDisplayStyle displayStyle)
{
    displayStyle_ = displayStyle;

    ++styleVersion_;
}

// Return display style for the renderable
RenderableSpecies::SpeciesDisplayStyle RenderableSpecies::displayStyle() const { return displayStyle_; }

/*
 * Style I/O
 */

// Return enum option info for RenderableKeyword
EnumOptions<RenderableSpecies::SpeciesStyleKeyword> RenderableSpecies::speciesStyleKeywords()
{
    static EnumOptionsList StyleKeywords = EnumOptionsList() << EnumOption(RenderableSpecies::DisplayKeyword, "Display", 1)
                                                             << EnumOption(RenderableSpecies::EndStyleKeyword, "EndStyle");

    static EnumOptions<RenderableSpecies::SpeciesStyleKeyword> options("SpeciesStyleKeyword", StyleKeywords);

    return options;
}

// Write style information
bool RenderableSpecies::writeStyleBlock(LineParser &parser, int indentLevel) const
{
    // Construct indent string
    char *indent = new char[indentLevel * 2 + 1];
    for (auto n = 0; n < indentLevel * 2; ++n)
        indent[n] = ' ';
    indent[indentLevel * 2] = '\0';

    if (!parser.writeLineF("{}{}  {}\n", indent, speciesStyleKeywords().keyword(RenderableSpecies::DisplayKeyword),
                           speciesDisplayStyles().keyword(displayStyle_)))
        return false;

    return true;
}

// Read style information
bool RenderableSpecies::readStyleBlock(LineParser &parser)
{
    while (!parser.eofOrBlank())
    {
        // Get line from file
        if (parser.getArgsDelim(LineParser::SemiColonLineBreaks) != LineParser::Success)
            return false;

        // Do we recognise this keyword and, if so, do we have an appropriate number of arguments?
        if (!speciesStyleKeywords().isValid(parser.argsv(0)))
            return speciesStyleKeywords().errorAndPrintValid(parser.argsv(0));
        auto kwd = speciesStyleKeywords().enumeration(parser.argsv(0));
        if (!speciesStyleKeywords().validNArgs(kwd, parser.nArgs() - 1))
            return false;

        // All OK, so process the keyword
        switch (kwd)
        {
            // Display style
            case (RenderableSpecies::DisplayKeyword):
                if (!speciesDisplayStyles().isValid(parser.argsv(1)))
                    return speciesDisplayStyles().errorAndPrintValid(parser.argsv(1));
                displayStyle_ = speciesDisplayStyles().enumeration(parser.argsv(1));
                break;
            // End of block
            case (RenderableSpecies::EndStyleKeyword):
                return true;
            // Unrecognised Keyword
            default:
                Messenger::warn("Unrecognised display style keyword for RenderableSpecies: {}\n", parser.argsv(0));
                return false;
                break;
        }
    }

    return true;
}
