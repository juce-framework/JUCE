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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_DrawableText.h"


//==============================================================================
DrawableText::DrawableText()
    : colour (Colours::white)
{
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const GlyphArrangement& newText)
{
    text = newText;
}

void DrawableText::setText (const String& newText, const Font& fontToUse)
{
    text.clear();
    text.addLineOfText (fontToUse, newText, 0.0f, 0.0f);
}

void DrawableText::setColour (const Colour& newColour)
{
    colour = newColour;
}

//==============================================================================
void DrawableText::draw (Graphics& g, const AffineTransform& transform) const
{
    const Colour oldColour (g.getCurrentColour()); // save this so we can restore it later

    g.setColour (colour.withMultipliedAlpha (oldColour.getFloatAlpha()));
    text.draw (g, transform);

    g.setColour (oldColour);
}

void DrawableText::getBounds (float& x, float& y, float& width, float& height) const
{
    text.getBoundingBox (0, -1, x, y, width, height, false); // (really returns top, left, bottom, right)
    width -= x;
    height -= y;
}

bool DrawableText::hitTest (float x, float y) const
{
    return text.findGlyphIndexAt (x, y) >= 0;
}

Drawable* DrawableText::createCopy() const
{
    DrawableText* const dt = new DrawableText();

    dt->text = text;
    dt->colour = colour;

    return dt;
}


END_JUCE_NAMESPACE
