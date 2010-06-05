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

#include "juce_DrawableText.h"
#include "juce_DrawableComposite.h"


//==============================================================================
DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : text (other.text),
      font (other.font),
      colour (other.colour),
      justification (other.justification)
{
    for (int i = 0; i < numElementsInArray (controlPoints); ++i)
        controlPoints[i] = other.controlPoints[i];
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    text = newText;
}

void DrawableText::setColour (const Colour& newColour)
{
    colour = newColour;
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    font = newFont;

    if (applySizeAndScale)
    {
        const Line<float> left (Point<float>(), controlPoints[2].resolve (getParent()));
        const Line<float> top (Point<float>(), controlPoints[1].resolve (getParent()));

        controlPoints[3] = RelativePoint (controlPoints[0].resolve (getParent())
                                            + left.getPointAlongLine (font.getHeight())
                                            + top.getPointAlongLine (font.getHorizontalScale() * font.getHeight()));
    }
}

void DrawableText::setJustification (const Justification& newJustification)
{
    justification = newJustification;
}

void DrawableText::setBounds (const RelativePoint& boundingBoxTopLeft,
                              const RelativePoint& boundingBoxTopRight,
                              const RelativePoint& boundingBoxBottomLeft,
                              const RelativePoint& fontSizeAndScaleAnchor)
{
    controlPoints[0] = boundingBoxTopLeft;
    controlPoints[1] = boundingBoxTopRight;
    controlPoints[2] = boundingBoxBottomLeft;
    controlPoints[3] = fontSizeAndScaleAnchor;
}

//==============================================================================
static const Point<float> findNormalisedCoordWithinParallelogram (const Point<float>& origin,
                                                                  Point<float> topRight,
                                                                  Point<float> bottomLeft,
                                                                  Point<float> target)
{
    topRight -= origin;
    bottomLeft -= origin;
    target -= origin;

    return Point<float> (Line<float> (Point<float>(), topRight).getIntersection (Line<float> (target, target - bottomLeft)).getDistanceFromOrigin(),
                         Line<float> (Point<float>(), bottomLeft).getIntersection (Line<float> (target, target - topRight)).getDistanceFromOrigin());
}

void DrawableText::render (const Drawable::RenderingContext& context) const
{
    Point<float> points[4];
    for (int i = 0; i < 4; ++i)
        points[i] = controlPoints[i].resolve (getParent());

    const float w = Line<float> (points[0], points[1]).getLength();
    const float h = Line<float> (points[0], points[2]).getLength();

    const Point<float> fontCoords (findNormalisedCoordWithinParallelogram (points[0], points[1], points[2], points[3]));
    const float fontHeight = jlimit (1.0f, h, fontCoords.getY());
    const float fontWidth = jlimit (0.01f, w, fontCoords.getX());

    Font f (font);
    f.setHeight (fontHeight);
    f.setHorizontalScale (fontWidth / fontHeight);

    context.g.setColour (colour.withMultipliedAlpha (context.opacity));

    GlyphArrangement ga;
    ga.addFittedText (f, text, 0, 0, w, h, justification, 0x100000);
    ga.draw (context.g,
             AffineTransform::fromTargetPoints (0, 0, points[0].getX(), points[0].getY(),
                                                w, 0, points[1].getX(), points[1].getY(),
                                                0, h, points[2].getX(), points[2].getY())
                .followedBy (context.transform));
}

void DrawableText::resolveCorners (Point<float>* const corners) const
{
    for (int i = 0; i < 3; ++i)
        corners[i] = controlPoints[i].resolve (parent);

    corners[3] = corners[1] + (corners[2] - corners[0]);
}

const Rectangle<float> DrawableText::getBounds() const
{
    Point<float> corners[4];
    resolveCorners (corners);
    return Rectangle<float>::findAreaContainingPoints (corners, 4);
}

bool DrawableText::hitTest (float x, float y) const
{
    Point<float> corners[4];
    resolveCorners (corners);

    Path p;
    p.startNewSubPath (corners[0].getX(), corners[0].getY());
    p.lineTo (corners[1].getX(), corners[1].getY());
    p.lineTo (corners[3].getX(), corners[3].getY());
    p.lineTo (corners[2].getX(), corners[2].getY());
    p.closeSubPath();

    return p.contains (x, y);
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
}

void DrawableText::invalidatePoints()
{
}

//==============================================================================
const Identifier DrawableText::valueTreeType ("Text");

const Identifier DrawableText::ValueTreeWrapper::text ("text");
const Identifier DrawableText::ValueTreeWrapper::colour ("colour");
const Identifier DrawableText::ValueTreeWrapper::font ("font");
const Identifier DrawableText::ValueTreeWrapper::justification ("justification");
const Identifier DrawableText::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableText::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableText::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableText::ValueTreeWrapper::fontSizeAnchor ("fontSizeAnchor");

//==============================================================================
DrawableText::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

const String DrawableText::ValueTreeWrapper::getText() const
{
    return state [text].toString();
}

void DrawableText::ValueTreeWrapper::setText (const String& newText, UndoManager* undoManager)
{
    state.setProperty (text, newText, undoManager);
}

const Colour DrawableText::ValueTreeWrapper::getColour() const
{
    return Colour::fromString (state [colour].toString());
}

void DrawableText::ValueTreeWrapper::setColour (const Colour& newColour, UndoManager* undoManager)
{
    state.setProperty (colour, newColour.toString(), undoManager);
}

const Justification DrawableText::ValueTreeWrapper::getJustification() const
{
    return Justification ((int) state [justification]);
}

void DrawableText::ValueTreeWrapper::setJustification (const Justification& newJustification, UndoManager* undoManager)
{
    state.setProperty (justification, newJustification.getFlags(), undoManager);
}

const Font DrawableText::ValueTreeWrapper::getFont() const
{
    return Font::fromString (state [font]);
}

void DrawableText::ValueTreeWrapper::setFont (const Font& newFont, UndoManager* undoManager)
{
    state.setProperty (font, newFont.toString(), undoManager);
}

const RelativePoint DrawableText::ValueTreeWrapper::getBoundingBoxTopLeft() const
{
    return state [topLeft].toString();
}

void DrawableText::ValueTreeWrapper::setBoundingBoxTopLeft (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (topLeft, p.toString(), undoManager);
}

const RelativePoint DrawableText::ValueTreeWrapper::getBoundingBoxTopRight() const
{
    return state [topRight].toString();
}

void DrawableText::ValueTreeWrapper::setBoundingBoxTopRight (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (topRight, p.toString(), undoManager);
}

const RelativePoint DrawableText::ValueTreeWrapper::getBoundingBoxBottomLeft() const
{
    return state [bottomLeft].toString();
}

void DrawableText::ValueTreeWrapper::setBoundingBoxBottomLeft (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (bottomLeft, p.toString(), undoManager);
}

const RelativePoint DrawableText::ValueTreeWrapper::getFontSizeAndScaleAnchor() const
{
    return state [fontSizeAnchor].toString();
}

void DrawableText::ValueTreeWrapper::setFontSizeAndScaleAnchor (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (fontSizeAnchor, p.toString(), undoManager);
}

const Rectangle<float> DrawableText::refreshFromValueTree (const ValueTree& tree, ImageProvider*)
{
    ValueTreeWrapper v (tree);
    setName (v.getID());

    const RelativePoint p1 (v.getBoundingBoxTopLeft()), p2 (v.getBoundingBoxTopRight()),
                        p3 (v.getBoundingBoxBottomLeft()), p4 (v.getFontSizeAndScaleAnchor());

    const Colour newColour (v.getColour());
    const Justification newJustification (v.getJustification());
    const String newText (v.getText());
    const Font newFont (v.getFont());

    if (text != newText || font != newFont || justification != newJustification || colour != newColour
         || p1 != controlPoints[0] || p2 != controlPoints[1] || p3 != controlPoints[2] || p4 != controlPoints[3])
    {
        const Rectangle<float> damage (getBounds());

        setBounds (p1, p2, p3, p4);
        setColour (newColour);
        setFont (newFont, false);
        setJustification (newJustification);
        setText (newText);

        return damage.getUnion (getBounds());

    }

    return Rectangle<float>();
}

const ValueTree DrawableText::createValueTree (ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    v.setText (text, 0);
    v.setFont (font, 0);
    v.setJustification (justification, 0);
    v.setColour (colour, 0);
    v.setBoundingBoxTopLeft (controlPoints[0], 0);
    v.setBoundingBoxTopRight (controlPoints[1], 0);
    v.setBoundingBoxBottomLeft (controlPoints[2], 0);
    v.setFontSizeAndScaleAnchor (controlPoints[3], 0);

    return tree;
}


END_JUCE_NAMESPACE
