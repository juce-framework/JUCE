/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    void draw (Graphics& g, const AffineTransform& transform) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    GlyphArrangement text;
    Colour colour;

    DrawableText (const DrawableText&);
    const DrawableText& operator= (const DrawableText&);
};


#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
