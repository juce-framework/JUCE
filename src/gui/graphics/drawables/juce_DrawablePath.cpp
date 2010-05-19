/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_DrawablePath.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
DrawablePath::DrawablePath()
    : mainFill (Colours::black),
      strokeFill (Colours::transparentBlack),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setFill (const FillType& newFill)
{
    mainFill = newFill;
}

void DrawablePath::setStrokeFill (const FillType& newFill)
{
    strokeFill = newFill;
}

void DrawablePath::setStrokeType (const PathStrokeType& newStrokeType)
{
    strokeType = newStrokeType;
    updateOutline();
}

void DrawablePath::setStrokeThickness (const float newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

//==============================================================================
void DrawablePath::render (const Drawable::RenderingContext& context) const
{
    {
        FillType f (mainFill);
        if (f.isGradient())
            f.gradient->multiplyOpacity (context.opacity);

        f.transform = f.transform.followedBy (context.transform);
        context.g.setFillType (f);
        context.g.fillPath (path, context.transform);
    }

    if (strokeType.getStrokeThickness() > 0.0f)
    {
        FillType f (strokeFill);
        if (f.isGradient())
            f.gradient->multiplyOpacity (context.opacity);

        f.transform = f.transform.followedBy (context.transform);
        context.g.setFillType (f);
        context.g.fillPath (stroke, context.transform);
    }
}

void DrawablePath::updateOutline()
{
    stroke.clear();
    strokeType.createStrokedPath (stroke, path, AffineTransform::identity, 4.0f);
}

const Rectangle<float> DrawablePath::getBounds() const
{
    if (strokeType.getStrokeThickness() > 0.0f)
        return stroke.getBounds();
    else
        return path.getBounds();
}

bool DrawablePath::hitTest (float x, float y) const
{
    return path.contains (x, y)
            || stroke.contains (x, y);
}

Drawable* DrawablePath::createCopy() const
{
    DrawablePath* const dp = new DrawablePath();

    dp->path = path;
    dp->stroke = stroke;
    dp->mainFill = mainFill;
    dp->strokeFill = strokeFill;
    dp->strokeType = strokeType;
    return dp;
}

//==============================================================================
const Identifier DrawablePath::valueTreeType ("Path");

namespace DrawablePathHelpers
{
    static const Identifier type ("type");
    static const Identifier solid ("solid");
    static const Identifier colour ("colour");
    static const Identifier gradient ("gradient");
    static const Identifier x1 ("x1");
    static const Identifier x2 ("x2");
    static const Identifier y1 ("y1");
    static const Identifier y2 ("y2");
    static const Identifier radial ("radial");
    static const Identifier colours ("colours");
    static const Identifier fill ("fill");
    static const Identifier stroke ("stroke");
    static const Identifier jointStyle ("jointStyle");
    static const Identifier capStyle ("capStyle");
    static const Identifier strokeWidth ("strokeWidth");
    static const Identifier path ("path");

    static bool updateFillType (const ValueTree& v, FillType& fillType)
    {
        const String type (v[type].toString());

        if (type.equalsIgnoreCase (solid))
        {
            const String colourString (v [colour].toString());
            const Colour newColour (colourString.isEmpty() ? (uint32) 0xff000000
                                                           : (uint32) colourString.getHexValue32());

            if (fillType.isColour() && fillType.colour == newColour)
                return false;

            fillType.setColour (newColour);
            return true;
        }
        else if (type.equalsIgnoreCase (gradient))
        {
            ColourGradient g;
            g.point1.setXY (v[x1], v[y1]);
            g.point2.setXY (v[x2], v[y2]);
            g.isRadial = v[radial];

            StringArray colourSteps;
            colourSteps.addTokens (v[colours].toString(), false);

            for (int i = 0; i < colourSteps.size() / 2; ++i)
                g.addColour (colourSteps[i * 2].getDoubleValue(),
                             Colour ((uint32)  colourSteps[i * 2 + 1].getHexValue32()));

            if (fillType.isGradient() && *fillType.gradient == g)
                return false;

            fillType.setGradient (g);
            return true;
        }

        jassertfalse;
        return false;
    }

    static ValueTree createFillType (const Identifier& tagName, const FillType& fillType)
    {
        ValueTree v (tagName);

        if (fillType.isColour())
        {
            v.setProperty (type, "solid", 0);
            v.setProperty (colour, String::toHexString ((int) fillType.colour.getARGB()), 0);
        }
        else if (fillType.isGradient())
        {
            v.setProperty (type, "gradient", 0);
            v.setProperty (x1, fillType.gradient->point1.getX(), 0);
            v.setProperty (y1, fillType.gradient->point1.getY(), 0);
            v.setProperty (x2, fillType.gradient->point2.getX(), 0);
            v.setProperty (y2, fillType.gradient->point2.getY(), 0);
            v.setProperty (radial, fillType.gradient->isRadial, 0);

            String s;
            for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
                s << " " << fillType.gradient->getColourPosition (i)
                  << " " << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

            v.setProperty (colours, s.trimStart(), 0);
        }
        else
        {
            jassertfalse; //xxx
        }

        return v;
    }
}

const Rectangle<float> DrawablePath::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    jassert (tree.hasType (valueTreeType));

    Rectangle<float> damageRect;
    setName (tree [idProperty]);

    bool needsRedraw = DrawablePathHelpers::updateFillType (tree.getChildWithName (DrawablePathHelpers::fill), mainFill);
    needsRedraw = DrawablePathHelpers::updateFillType (tree.getChildWithName (DrawablePathHelpers::stroke), strokeFill) || needsRedraw;

    const String jointStyle (tree [DrawablePathHelpers::jointStyle].toString());
    const String endStyle (tree [DrawablePathHelpers::capStyle].toString());

    PathStrokeType newStroke (tree [DrawablePathHelpers::strokeWidth],
                              jointStyle == "curved" ? PathStrokeType::curved
                                                     : (jointStyle == "bevel" ? PathStrokeType::beveled
                                                                              : PathStrokeType::mitered),
                              endStyle == "square" ? PathStrokeType::square
                                                   : (endStyle == "round" ? PathStrokeType::rounded
                                                                          : PathStrokeType::butt));

    Path newPath;
    newPath.restoreFromString (tree [DrawablePathHelpers::path]);

    if (strokeType != newStroke || path != newPath)
    {
        damageRect = getBounds();
        path.swapWithPath (newPath);
        strokeType = newStroke;
        needsRedraw = true;
    }

    if (needsRedraw)
        damageRect = damageRect.getUnion (getBounds());

    return damageRect;
}

const ValueTree DrawablePath::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree v (valueTreeType);

    v.addChild (DrawablePathHelpers::createFillType (DrawablePathHelpers::fill, mainFill), -1, 0);
    v.addChild (DrawablePathHelpers::createFillType (DrawablePathHelpers::stroke, strokeFill), -1, 0);

    if (getName().isNotEmpty())
        v.setProperty (idProperty, getName(), 0);

    v.setProperty (DrawablePathHelpers::strokeWidth, (double) strokeType.getStrokeThickness(), 0);
    v.setProperty (DrawablePathHelpers::jointStyle, strokeType.getJointStyle() == PathStrokeType::mitered
                                                    ? "miter" : (strokeType.getJointStyle() == PathStrokeType::curved ? "curved" : "bevel"), 0);
    v.setProperty (DrawablePathHelpers::capStyle, strokeType.getEndStyle() == PathStrokeType::butt
                                                    ? "butt" : (strokeType.getEndStyle() == PathStrokeType::square ? "square" : "round"), 0);
    v.setProperty (DrawablePathHelpers::path, path.toString(), 0);

    return v;
}


END_JUCE_NAMESPACE
