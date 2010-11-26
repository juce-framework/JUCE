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

#include "juce_DrawableShape.h"
#include "juce_DrawableComposite.h"


//==============================================================================
DrawableShape::DrawableShape()
    : strokeType (0.0f),
      mainFill (Colours::black),
      strokeFill (Colours::black)
{
}

DrawableShape::DrawableShape (const DrawableShape& other)
    : strokeType (other.strokeType),
      mainFill (other.mainFill),
      strokeFill (other.strokeFill)
{
}

DrawableShape::~DrawableShape()
{
}

void DrawableShape::setFill (const FillType& newFill)
{
    mainFill = newFill;
}

void DrawableShape::setStrokeFill (const FillType& newFill)
{
    strokeFill = newFill;
}

void DrawableShape::setStrokeType (const PathStrokeType& newStrokeType)
{
    strokeType = newStrokeType;
    strokeChanged();
}

void DrawableShape::setStrokeThickness (const float newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

bool DrawableShape::isStrokeVisible() const throw()
{
    return strokeType.getStrokeThickness() > 0.0f && ! strokeFill.isInvisible();
}

bool DrawableShape::refreshFillTypes (const FillAndStrokeState& newState,
                                      Expression::EvaluationContext* /*nameFinder*/,
                                      ImageProvider* imageProvider)
{
    bool hasChanged = false;

    {
        const FillType f (newState.getMainFill (getParent(), imageProvider));

        if (mainFill != f)
        {
            hasChanged = true;
            mainFill = f;
        }
    }

    {
        const FillType f (newState.getStrokeFill (getParent(), imageProvider));

        if (strokeFill != f)
        {
            hasChanged = true;
            strokeFill = f;
        }
    }

    return hasChanged;
}

void DrawableShape::writeTo (FillAndStrokeState& state, ImageProvider* imageProvider, UndoManager* undoManager) const
{
    state.setMainFill (mainFill, 0, 0, 0, imageProvider, undoManager);
    state.setStrokeFill (strokeFill, 0, 0, 0, imageProvider, undoManager);
    state.setStrokeType (strokeType, undoManager);
}

//==============================================================================
void DrawableShape::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    g.setFillType (mainFill);
    g.fillPath (path);

    if (isStrokeVisible())
    {
        g.setFillType (strokeFill);
        g.fillPath (strokePath);
    }
}

void DrawableShape::pathChanged()
{
    strokeChanged();
}

void DrawableShape::strokeChanged()
{
    strokePath.clear();
    strokeType.createStrokedPath (strokePath, path, AffineTransform::identity, 4.0f);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

const Rectangle<float> DrawableShape::getDrawableBounds() const
{
    if (isStrokeVisible())
        return strokePath.getBounds();
    else
        return path.getBounds();
}

bool DrawableShape::hitTest (int x, int y) const
{
    const float globalX = (float) (x - originRelativeToComponent.getX());
    const float globalY = (float) (y - originRelativeToComponent.getY());

    return path.contains (globalX, globalY)
            || (isStrokeVisible() && strokePath.contains (globalX, globalY));
}

//==============================================================================
const Identifier DrawableShape::FillAndStrokeState::type ("type");
const Identifier DrawableShape::FillAndStrokeState::colour ("colour");
const Identifier DrawableShape::FillAndStrokeState::colours ("colours");
const Identifier DrawableShape::FillAndStrokeState::fill ("Fill");
const Identifier DrawableShape::FillAndStrokeState::stroke ("Stroke");
const Identifier DrawableShape::FillAndStrokeState::path ("Path");
const Identifier DrawableShape::FillAndStrokeState::jointStyle ("jointStyle");
const Identifier DrawableShape::FillAndStrokeState::capStyle ("capStyle");
const Identifier DrawableShape::FillAndStrokeState::strokeWidth ("strokeWidth");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint1 ("point1");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint2 ("point2");
const Identifier DrawableShape::FillAndStrokeState::gradientPoint3 ("point3");
const Identifier DrawableShape::FillAndStrokeState::radial ("radial");
const Identifier DrawableShape::FillAndStrokeState::imageId ("imageId");
const Identifier DrawableShape::FillAndStrokeState::imageOpacity ("imageOpacity");

DrawableShape::FillAndStrokeState::FillAndStrokeState (const ValueTree& state_)
    : Drawable::ValueTreeWrapperBase (state_)
{
}

const FillType DrawableShape::FillAndStrokeState::getMainFill (Expression::EvaluationContext* nameFinder,
                                                               ImageProvider* imageProvider) const
{
    return readFillType (state.getChildWithName (fill), 0, 0, 0, nameFinder, imageProvider);
}

ValueTree DrawableShape::FillAndStrokeState::getMainFillState()
{
    ValueTree v (state.getChildWithName (fill));
    if (v.isValid())
        return v;

    setMainFill (Colours::black, 0, 0, 0, 0, 0);
    return getMainFillState();
}

void DrawableShape::FillAndStrokeState::setMainFill (const FillType& newFill, const RelativePoint* gp1, const RelativePoint* gp2,
                                                     const RelativePoint* gp3, ImageProvider* imageProvider, UndoManager* undoManager)
{
    ValueTree v (state.getOrCreateChildWithName (fill, undoManager));
    writeFillType (v, newFill, gp1, gp2, gp3, imageProvider, undoManager);
}

const FillType DrawableShape::FillAndStrokeState::getStrokeFill (Expression::EvaluationContext* nameFinder,
                                                                 ImageProvider* imageProvider) const
{
    return readFillType (state.getChildWithName (stroke), 0, 0, 0, nameFinder, imageProvider);
}

ValueTree DrawableShape::FillAndStrokeState::getStrokeFillState()
{
    ValueTree v (state.getChildWithName (stroke));
    if (v.isValid())
        return v;

    setStrokeFill (Colours::black, 0, 0, 0, 0, 0);
    return getStrokeFillState();
}

void DrawableShape::FillAndStrokeState::setStrokeFill (const FillType& newFill, const RelativePoint* gp1, const RelativePoint* gp2,
                                                       const RelativePoint* gp3, ImageProvider* imageProvider, UndoManager* undoManager)
{
    ValueTree v (state.getOrCreateChildWithName (stroke, undoManager));
    writeFillType (v, newFill, gp1, gp2, gp3, imageProvider, undoManager);
}

const PathStrokeType DrawableShape::FillAndStrokeState::getStrokeType() const
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

void DrawableShape::FillAndStrokeState::setStrokeType (const PathStrokeType& newStrokeType, UndoManager* undoManager)
{
    state.setProperty (strokeWidth, (double) newStrokeType.getStrokeThickness(), undoManager);
    state.setProperty (jointStyle, newStrokeType.getJointStyle() == PathStrokeType::mitered
                                     ? "miter" : (newStrokeType.getJointStyle() == PathStrokeType::curved ? "curved" : "bevel"), undoManager);
    state.setProperty (capStyle, newStrokeType.getEndStyle() == PathStrokeType::butt
                                     ? "butt" : (newStrokeType.getEndStyle() == PathStrokeType::square ? "square" : "round"), undoManager);
}

const FillType DrawableShape::FillAndStrokeState::readFillType (const ValueTree& v, RelativePoint* const gp1, RelativePoint* const gp2, RelativePoint* const gp3,
                                                                Expression::EvaluationContext* const nameFinder, ImageProvider* imageProvider)
{
    const String newType (v[type].toString());

    if (newType == "solid")
    {
        const String colourString (v [colour].toString());
        return FillType (Colour (colourString.isEmpty() ? (uint32) 0xff000000
                                                        : (uint32) colourString.getHexValue32()));
    }
    else if (newType == "gradient")
    {
        RelativePoint p1 (v [gradientPoint1]), p2 (v [gradientPoint2]), p3 (v [gradientPoint3]);

        ColourGradient g;

        if (gp1 != 0)  *gp1 = p1;
        if (gp2 != 0)  *gp2 = p2;
        if (gp3 != 0)  *gp3 = p3;

        g.point1 = p1.resolve (nameFinder);
        g.point2 = p2.resolve (nameFinder);
        g.isRadial = v[radial];

        StringArray colourSteps;
        colourSteps.addTokens (v[colours].toString(), false);

        for (int i = 0; i < colourSteps.size() / 2; ++i)
            g.addColour (colourSteps[i * 2].getDoubleValue(),
                         Colour ((uint32)  colourSteps[i * 2 + 1].getHexValue32()));

        FillType fillType (g);

        if (g.isRadial)
        {
            const Point<float> point3 (p3.resolve (nameFinder));
            const Point<float> point3Source (g.point1.getX() + g.point2.getY() - g.point1.getY(),
                                             g.point1.getY() + g.point1.getX() - g.point2.getX());

            fillType.transform = AffineTransform::fromTargetPoints (g.point1.getX(), g.point1.getY(), g.point1.getX(), g.point1.getY(),
                                                                    g.point2.getX(), g.point2.getY(), g.point2.getX(), g.point2.getY(),
                                                                    point3Source.getX(), point3Source.getY(), point3.getX(), point3.getY());
        }

        return fillType;
    }
    else if (newType == "image")
    {
        Image im;
        if (imageProvider != 0)
            im = imageProvider->getImageForIdentifier (v[imageId]);

        FillType f (im, AffineTransform::identity);
        f.setOpacity ((float) v.getProperty (imageOpacity, 1.0f));
        return f;
    }

    jassert (! v.isValid());
    return FillType();
}

namespace DrawableShapeHelpers
{
    const Point<float> calcThirdGradientPoint (const FillType& fillType)
    {
        const ColourGradient& g = *fillType.gradient;
        const Point<float> point3Source (g.point1.getX() + g.point2.getY() - g.point1.getY(),
                                         g.point1.getY() + g.point1.getX() - g.point2.getX());

        return point3Source.transformedBy (fillType.transform);
    }
}

void DrawableShape::FillAndStrokeState::writeFillType (ValueTree& v, const FillType& fillType,
                                                       const RelativePoint* const gp1, const RelativePoint* const gp2, const RelativePoint* gp3,
                                                       ImageProvider* imageProvider, UndoManager* const undoManager)
{
    if (fillType.isColour())
    {
        v.setProperty (type, "solid", undoManager);
        v.setProperty (colour, String::toHexString ((int) fillType.colour.getARGB()), undoManager);
    }
    else if (fillType.isGradient())
    {
        v.setProperty (type, "gradient", undoManager);
        v.setProperty (gradientPoint1, gp1 != 0 ? gp1->toString() : fillType.gradient->point1.toString(), undoManager);
        v.setProperty (gradientPoint2, gp2 != 0 ? gp2->toString() : fillType.gradient->point2.toString(), undoManager);
        v.setProperty (gradientPoint3, gp3 != 0 ? gp3->toString() : DrawableShapeHelpers::calcThirdGradientPoint (fillType).toString(), undoManager);

        v.setProperty (radial, fillType.gradient->isRadial, undoManager);

        String s;
        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
            s << ' ' << fillType.gradient->getColourPosition (i)
              << ' ' << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

        v.setProperty (colours, s.trimStart(), undoManager);
    }
    else if (fillType.isTiledImage())
    {
        v.setProperty (type, "image", undoManager);

        if (imageProvider != 0)
            v.setProperty (imageId, imageProvider->getIdentifierForImage (fillType.image), undoManager);

        if (fillType.getOpacity() < 1.0f)
            v.setProperty (imageOpacity, fillType.getOpacity(), undoManager);
        else
            v.removeProperty (imageOpacity, undoManager);
    }
    else
    {
        jassertfalse;
    }
}

END_JUCE_NAMESPACE
