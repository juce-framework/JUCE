/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    : mainFill (FillType (Colours::black)),
      strokeFill (FillType (Colours::transparentBlack)),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath) throw()
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setFill (const FillType& newFill) throw()
{
    mainFill = newFill;
}

void DrawablePath::setStrokeFill (const FillType& newFill) throw()
{
    strokeFill = newFill;
}

void DrawablePath::setStrokeType (const PathStrokeType& newStrokeType) throw()
{
    strokeType = newStrokeType;
    updateOutline();
}

void DrawablePath::setStrokeThickness (const float newThickness) throw()
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

void DrawablePath::getBounds (float& x, float& y, float& width, float& height) const
{
    if (strokeType.getStrokeThickness() > 0.0f)
        stroke.getBounds (x, y, width, height);
    else
        path.getBounds (x, y, width, height);
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
static const FillType readFillTypeFromTree (const ValueTree& v)
{
    const String type (v["type"].toString());

    if (type.equalsIgnoreCase (T("solid")))
    {
        const String colour (v ["colour"].toString());
        return FillType (Colour (colour.isEmpty() ? (uint32) 0xff000000
                                                  : (uint32) colour.getHexValue32()));
    }
    else if (type.equalsIgnoreCase ("gradient"))
    {
        ColourGradient g;
        g.x1 = v["x1"];
        g.y1 = v["y1"];
        g.x2 = v["x2"];
        g.y2 = v["y2"];
        g.isRadial = v["radial"];

        StringArray colours;
        colours.addTokens (v["colours"].toString(), false);

        for (int i = 0; i < colours.size() / 2; ++i)
            g.addColour (colours[i * 2].getDoubleValue(),
                         Colour ((uint32)  colours[i * 2 + 1].getHexValue32()));

        return FillType (g);
    }

    jassertfalse
    return FillType();
}

static ValueTree createTreeForFillType (const String& tagName, const FillType& fillType)
{
    ValueTree v (tagName);

    if (fillType.isColour())
    {
        v.setProperty ("type", T("solid"), 0);
        v.setProperty ("colour", String::toHexString ((int) fillType.colour.getARGB()), 0);
    }
    else if (fillType.isGradient())
    {
        v.setProperty ("type", T("gradient"), 0);
        v.setProperty ("x1", fillType.gradient->x1, 0);
        v.setProperty ("y1", fillType.gradient->y1, 0);
        v.setProperty ("x2", fillType.gradient->x2, 0);
        v.setProperty ("y2", fillType.gradient->y2, 0);
        v.setProperty ("radial", fillType.gradient->isRadial, 0);

        String s;
        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
            s << " " << fillType.gradient->getColourPosition (i)
              << " " << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

        v.setProperty ("colours", s.trimStart(), 0);
    }
    else
    {
        jassertfalse //xxx
    }

    return v;
}

ValueTree DrawablePath::createValueTree() const throw()
{
    ValueTree v (T("Path"));

    v.addChild (createTreeForFillType (T("fill"), mainFill), -1, 0);
    v.addChild (createTreeForFillType (T("stroke"), strokeFill), -1, 0);

    if (getName().isNotEmpty())
        v.setProperty ("id", getName(), 0);

    v.setProperty ("strokeWidth", (double) strokeType.getStrokeThickness(), 0);
    v.setProperty ("jointStyle", strokeType.getJointStyle() == PathStrokeType::mitered
                                    ? T("miter") : (strokeType.getJointStyle() == PathStrokeType::curved ? T("curved") : T("bevel")), 0);
    v.setProperty ("capStyle", strokeType.getEndStyle() == PathStrokeType::butt
                                    ? T("butt") : (strokeType.getEndStyle() == PathStrokeType::square ? T("square") : T("round")), 0);
    v.setProperty ("path", path.toString(), 0);

    return v;
}

DrawablePath* DrawablePath::createFromValueTree (const ValueTree& tree) throw()
{
    if (! tree.hasType ("Path"))
        return 0;

    DrawablePath* p = new DrawablePath();

    p->setName (tree ["id"]);
    p->mainFill = readFillTypeFromTree (tree.getChildWithName (T("fill")));
    p->strokeFill = readFillTypeFromTree (tree.getChildWithName (T("stroke")));

    const String jointStyle (tree ["jointStyle"].toString());
    const String endStyle (tree ["capStyle"].toString());

    p->strokeType
        = PathStrokeType (tree ["strokeWidth"],
                          jointStyle.equalsIgnoreCase (T("curved")) ? PathStrokeType::curved
                                                                    : (jointStyle.equalsIgnoreCase (T("bevel")) ? PathStrokeType::beveled
                                                                                                                : PathStrokeType::mitered),
                          endStyle.equalsIgnoreCase (T("square")) ? PathStrokeType::square
                                                                  : (endStyle.equalsIgnoreCase (T("round")) ? PathStrokeType::rounded
                                                                                                            : PathStrokeType::butt));

    p->path.clear();
    p->path.restoreFromString (tree ["path"]);
    p->updateOutline();

    return p;
}


END_JUCE_NAMESPACE
