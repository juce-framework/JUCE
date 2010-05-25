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


//==============================================================================
DrawableText::DrawableText()
    : colour (Colours::white)
{
}

DrawableText::DrawableText (const DrawableText& other)
    : text (other.text),
      colour (other.colour)
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
void DrawableText::render (const Drawable::RenderingContext& context) const
{
    context.g.setColour (colour.withMultipliedAlpha (context.opacity));
    text.draw (context.g, context.transform);
}

const Rectangle<float> DrawableText::getBounds() const
{
    return text.getBoundingBox (0, -1, false);
}

bool DrawableText::hitTest (float x, float y) const
{
    return text.findGlyphIndexAt (x, y) >= 0;
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

DrawableText::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

const Rectangle<float> DrawableText::refreshFromValueTree (const ValueTree& tree, ImageProvider*)
{
    ValueTreeWrapper v (tree);
    setName (v.getID());

    jassertfalse; // xxx not finished!

    return Rectangle<float>();
}

const ValueTree DrawableText::createValueTree (ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);

    jassertfalse; // xxx not finished!

    return tree;
}


END_JUCE_NAMESPACE
