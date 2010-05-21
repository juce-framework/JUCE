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
#include "juce_DrawableComposite.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
DrawablePath::DrawablePath()
    : mainFill (Colours::black),
      strokeFill (Colours::transparentBlack),
      strokeType (0.0f),
      pathNeedsUpdating (true),
      strokeNeedsUpdating (true)
{
}

DrawablePath::DrawablePath (const DrawablePath& other)
    : mainFill (other.mainFill),
      strokeFill (other.strokeFill),
      strokeType (other.strokeType),
      pathNeedsUpdating (true),
      strokeNeedsUpdating (true)
{
    if (other.relativePath != 0)
        relativePath = new RelativePointPath (*other.relativePath);
    else
        path = other.path;
}

DrawablePath::~DrawablePath()
{
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    strokeNeedsUpdating = true;
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
    strokeNeedsUpdating = true;
}

void DrawablePath::setStrokeThickness (const float newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

void DrawablePath::updatePath() const
{
    if (pathNeedsUpdating)
    {
        pathNeedsUpdating = false;

        if (relativePath != 0)
        {
            path.clear();
            relativePath->createPath (path, parent);
            strokeNeedsUpdating = true;
        }
    }
}

void DrawablePath::updateStroke() const
{
    if (strokeNeedsUpdating)
    {
        strokeNeedsUpdating = false;
        updatePath();
        stroke.clear();
        strokeType.createStrokedPath (stroke, path, AffineTransform::identity, 4.0f);
    }
}

const Path& DrawablePath::getPath() const
{
    updatePath();
    return path;
}

const Path& DrawablePath::getStrokePath() const
{
    updateStroke();
    return stroke;
}

bool DrawablePath::isStrokeVisible() const throw()
{
    return strokeType.getStrokeThickness() > 0.0f && ! strokeFill.isInvisible();
}

void DrawablePath::invalidatePoints()
{
    pathNeedsUpdating = true;
    strokeNeedsUpdating = true;
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
        context.g.fillPath (getPath(), context.transform);
    }

    if (isStrokeVisible())
    {
        FillType f (strokeFill);
        if (f.isGradient())
            f.gradient->multiplyOpacity (context.opacity);

        f.transform = f.transform.followedBy (context.transform);
        context.g.setFillType (f);
        context.g.fillPath (getStrokePath(), context.transform);
    }
}

const Rectangle<float> DrawablePath::getBounds() const
{
    if (isStrokeVisible())
        return getStrokePath().getBounds();
    else
        return getPath().getBounds();
}

bool DrawablePath::hitTest (float x, float y) const
{
    return getPath().contains (x, y)
            || (isStrokeVisible() && getStrokePath().contains (x, y));
}

Drawable* DrawablePath::createCopy() const
{
    return new DrawablePath (*this);
}

//==============================================================================
const Identifier DrawablePath::valueTreeType ("Path");

const Identifier DrawablePath::ValueTreeWrapper::fill ("fill");
const Identifier DrawablePath::ValueTreeWrapper::stroke ("stroke");
const Identifier DrawablePath::ValueTreeWrapper::jointStyle ("jointStyle");
const Identifier DrawablePath::ValueTreeWrapper::capStyle ("capStyle");
const Identifier DrawablePath::ValueTreeWrapper::strokeWidth ("strokeWidth");
const Identifier DrawablePath::ValueTreeWrapper::path ("path");

//==============================================================================
DrawablePath::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

const FillType DrawablePath::ValueTreeWrapper::getMainFill() const
{
    return readFillType (state.getChildWithName (fill));
}

void DrawablePath::ValueTreeWrapper::setMainFill (const FillType& newFill, UndoManager* undoManager)
{
    replaceFillType (fill, newFill, undoManager);
}

const FillType DrawablePath::ValueTreeWrapper::getStrokeFill() const
{
    return readFillType (state.getChildWithName (stroke));
}

void DrawablePath::ValueTreeWrapper::setStrokeFill (const FillType& newFill, UndoManager* undoManager)
{
    replaceFillType (stroke, newFill, undoManager);
}

const PathStrokeType DrawablePath::ValueTreeWrapper::getStrokeType() const
{
    const String jointStyleString (state [jointStyle].toString());
    const String capStyleString (state [capStyle].toString());

    return PathStrokeType (state [strokeWidth],
                           jointStyleString == "curved" ? PathStrokeType::curved
                                                        : (jointStyleString == "bevel" ? PathStrokeType::beveled
                                                                                       : PathStrokeType::mitered),
                           capStyleString == "square" ? PathStrokeType::square
                                                      : (capStyleString == "round" ? PathStrokeType::rounded
                                                                                   : PathStrokeType::butt));
}

void DrawablePath::ValueTreeWrapper::setStrokeType (const PathStrokeType& newStrokeType, UndoManager* undoManager)
{
    state.setProperty (strokeWidth, (double) newStrokeType.getStrokeThickness(), undoManager);
    state.setProperty (jointStyle, newStrokeType.getJointStyle() == PathStrokeType::mitered
                                     ? "miter" : (newStrokeType.getJointStyle() == PathStrokeType::curved ? "curved" : "bevel"), undoManager);
    state.setProperty (capStyle, newStrokeType.getEndStyle() == PathStrokeType::butt
                                   ? "butt" : (newStrokeType.getEndStyle() == PathStrokeType::square ? "square" : "round"), undoManager);
}

void DrawablePath::ValueTreeWrapper::getPath (RelativePointPath& p) const
{
    RelativePointPath newPath (state [path]);
    p.swapWith (newPath);
}

void DrawablePath::ValueTreeWrapper::setPath (const String& newPath, UndoManager* undoManager)
{
    state.setProperty (path, newPath, undoManager);
}

const Rectangle<float> DrawablePath::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    Rectangle<float> damageRect;
    ValueTreeWrapper v (tree);
    setName (v.getID());

    bool needsRedraw = false;
    const FillType newFill (v.getMainFill());

    if (mainFill != newFill)
    {
        needsRedraw = true;
        mainFill = newFill;
    }

    const FillType newStrokeFill (v.getStrokeFill());

    if (strokeFill != newStrokeFill)
    {
        needsRedraw = true;
        strokeFill = newStrokeFill;
    }

    const PathStrokeType newStroke (v.getStrokeType());

    ScopedPointer<RelativePointPath> newRelativePath (new RelativePointPath());
    v.getPath (*newRelativePath);

    Path newPath;
    newRelativePath->createPath (newPath, parent);

    if (! newRelativePath->containsAnyDynamicPoints())
        newRelativePath = 0;

    if (strokeType != newStroke || path != newPath)
    {
        damageRect = getBounds();
        path.swapWithPath (newPath);
        strokeType = newStroke;
        needsRedraw = true;
    }

    relativePath = newRelativePath.release();

    if (needsRedraw)
        damageRect = damageRect.getUnion (getBounds());

    return damageRect;
}

const ValueTree DrawablePath::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    v.setMainFill (mainFill, 0);
    v.setStrokeFill (strokeFill, 0);
    v.setStrokeType (strokeType, 0);

    if (relativePath != 0)
        v.setPath (relativePath->toString(), 0);
    else
        v.setPath (path.toString(), 0);

    return tree;
}


END_JUCE_NAMESPACE
