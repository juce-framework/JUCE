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

#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__
#define __JUCE_DRAWABLETEXT_JUCEHEADER__

#include "juce_Drawable.h"
#include "../fonts/juce_GlyphArrangement.h"


//==============================================================================
/**
    A drawable object which renders a line of text.

    @see Drawable
*/
class JUCE_API  DrawableText  : public Drawable
{
public:
    //==============================================================================
    /** Creates a DrawableText object. */
    DrawableText();

    /** Destructor. */
    virtual ~DrawableText();

    //==============================================================================
    /** Sets the block of text to render */
    void setText (const GlyphArrangement& newText);

    /** Sets a single line of text to render.

        This is a convenient method of adding a single line - for
        more complex text, use the setText() that takes a
        GlyphArrangement instead.
    */
    void setText (const String& newText, const Font& fontToUse);

    /** Returns the text arrangement that was set with setText(). */
    const GlyphArrangement& getText() const throw()         { return text; }

    /** Sets the colour of the text. */
    void setColour (const Colour& newColour);

    /** Returns the current text colour. */
    const Colour& getColour() const throw()                 { return colour; }


    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    ValueTree createValueTree() const throw();
    /** @internal */
    static DrawableText* createFromValueTree (const ValueTree& tree) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    GlyphArrangement text;
    Colour colour;

    DrawableText (const DrawableText&);
    const DrawableText& operator= (const DrawableText&);
};


#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
