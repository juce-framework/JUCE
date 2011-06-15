/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
    setBoundingBox (RelativeParallelogram (RelativePoint (0.0f, 0.0f),
                                           RelativePoint (50.0f, 0.0f),
                                           RelativePoint (0.0f, 20.0f)));
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : bounds (other.bounds),
      fontSizeControlPoint (other.fontSizeControlPoint),
      font (other.font),
      text (other.text),
      colour (other.colour),
      justification (other.justification)
{
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        refreshBounds();
    }
}

void DrawableText::setColour (const Colour& newColour)
{
    if (colour != newColour)
    {
        colour = newColour;
        repaint();
    }
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    if (font != newFont)
    {
        font = newFont;

        if (applySizeAndScale)
        {
            setFontSizeControlPoint (RelativePoint (RelativeParallelogram::getPointForInternalCoord (resolvedPoints,
                                       Point<float> (font.getHorizontalScale() * font.getHeight(), font.getHeight()))));
        }

        refreshBounds();
    }
}

void DrawableText::setJustification (const Justification& newJustification)
{
    justification = newJustification;
    repaint();
}

void DrawableText::setBoundingBox (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        refreshBounds();
    }
}

void DrawableText::setFontSizeControlPoint (const RelativePoint& newPoint)
{
    if (fontSizeControlPoint != newPoint)
    {
        fontSizeControlPoint = newPoint;
        refreshBounds();
    }
}

void DrawableText::refreshBounds()
{
    if (bounds.isDynamic() || fontSizeControlPoint.isDynamic())
    {
        Drawable::Positioner<DrawableText>* const p = new Drawable::Positioner<DrawableText> (*this);
        setPositioner (p);
        p->apply();
    }
    else
    {
        setPositioner (0);
        recalculateCoordinates (0);
    }
}

bool DrawableText::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    ok = pos.addPoint (bounds.bottomLeft) && ok;
    return pos.addPoint (fontSizeControlPoint) && ok;
}

void DrawableText::recalculateCoordinates (Expression::Scope* scope)
{
    bounds.resolveThreePoints (resolvedPoints, scope);

    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    const Point<float> fontCoords (RelativeParallelogram::getInternalCoordForPoint (resolvedPoints, fontSizeControlPoint.resolve (scope)));
    const float fontHeight = jlimit (0.01f, jmax (0.01f, h), fontCoords.getY());
    const float fontWidth = jlimit (0.01f, jmax (0.01f, w), fontCoords.getX());

    scaledFont = font;
    scaledFont.setHeight (fontHeight);
    scaledFont.setHorizontalScale (fontWidth / fontHeight);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

const AffineTransform DrawableText::getArrangementAndTransform (GlyphArrangement& glyphs) const
{
    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    glyphs.addFittedText (scaledFont, text, 0, 0, w, h, justification, 0x100000);

    return AffineTransform::fromTargetPoints (0, 0, resolvedPoints[0].getX(), resolvedPoints[0].getY(),
                                              w, 0, resolvedPoints[1].getX(), resolvedPoints[1].getY(),
                                              0, h, resolvedPoints[2].getX(), resolvedPoints[2].getY());
}

//==============================================================================
void DrawableText::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    g.setColour (colour);

    GlyphArrangement ga;
    const AffineTransform transform (getArrangementAndTransform (ga));
    ga.draw (g, transform);
}

Rectangle<float> DrawableText::getDrawableBounds() const
{
    return RelativeParallelogram::getBoundingBox (resolvedPoints);
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
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

String DrawableText::ValueTreeWrapper::getText() const
{
    return state [text].toString();
}

void DrawableText::ValueTreeWrapper::setText (const String& newText, UndoManager* undoManager)
{
    state.setProperty (text, newText, undoManager);
}

Value DrawableText::ValueTreeWrapper::getTextValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (text, undoManager);
}

Colour DrawableText::ValueTreeWrapper::getColour() const
{
    return Colour::fromString (state [colour].toString());
}

void DrawableText::ValueTreeWrapper::setColour (const Colour& newColour, UndoManager* undoManager)
{
    state.setProperty (colour, newColour.toString(), undoManager);
}

Justification DrawableText::ValueTreeWrapper::getJustification() const
{
    return Justification ((int) state [justification]);
}

void DrawableText::ValueTreeWrapper::setJustification (const Justification& newJustification, UndoManager* undoManager)
{
    state.setProperty (justification, newJustification.getFlags(), undoManager);
}

Font DrawableText::ValueTreeWrapper::getFont() const
{
    return Font::fromString (state [font]);
}

void DrawableText::ValueTreeWrapper::setFont (const Font& newFont, UndoManager* undoManager)
{
    state.setProperty (font, newFont.toString(), undoManager);
}

Value DrawableText::ValueTreeWrapper::getFontValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (font, undoManager);
}

RelativeParallelogram DrawableText::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state [topLeft].toString(), state [topRight].toString(), state [bottomLeft].toString());
}

void DrawableText::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

RelativePoint DrawableText::ValueTreeWrapper::getFontSizeControlPoint() const
{
    return state [fontSizeAnchor].toString();
}

void DrawableText::ValueTreeWrapper::setFontSizeControlPoint (const RelativePoint& p, UndoManager* undoManager)
{
    state.setProperty (fontSizeAnchor, p.toString(), undoManager);
}

//==============================================================================
void DrawableText::refreshFromValueTree (const ValueTree& tree, ComponentBuilder&)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    const RelativeParallelogram newBounds (v.getBoundingBox());
    const RelativePoint newFontPoint (v.getFontSizeControlPoint());
    const Colour newColour (v.getColour());
    const Justification newJustification (v.getJustification());
    const String newText (v.getText());
    const Font newFont (v.getFont());

    if (text != newText || font != newFont || justification != newJustification
         || colour != newColour || bounds != newBounds || newFontPoint != fontSizeControlPoint)
    {
        setBoundingBox (newBounds);
        setFontSizeControlPoint (newFontPoint);
        setColour (newColour);
        setFont (newFont, false);
        setJustification (newJustification);
        setText (newText);
    }
}

ValueTree DrawableText::createValueTree (ComponentBuilder::ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    v.setText (text, nullptr);
    v.setFont (font, nullptr);
    v.setJustification (justification, nullptr);
    v.setColour (colour, nullptr);
    v.setBoundingBox (bounds, nullptr);
    v.setFontSizeControlPoint (fontSizeControlPoint, nullptr);

    return tree;
}


END_JUCE_NAMESPACE
