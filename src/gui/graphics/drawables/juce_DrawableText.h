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
    DrawableText (const DrawableText& other);

    /** Destructor. */
    ~DrawableText();

    //==============================================================================
    /** Sets the text to display.*/
    void setText (const String& newText);

    /** Sets the colour of the text. */
    void setColour (const Colour& newColour);

    /** Returns the current text colour. */
    const Colour& getColour() const throw()                 { return colour; }

    /** Sets the font to use.
        Note that the font height and horizontal scale are actually based upon the position
        of the fontSizeAndScaleAnchor parameter to setBounds(). If applySizeAndScale is true, then
        the height and scale control point will be moved to match the dimensions of the font supplied;
        if it is false, then the new font's height and scale are ignored.
    */
    void setFont (const Font& newFont, bool applySizeAndScale);

    /** Changes the justification of the text within the bounding box. */
    void setJustification (const Justification& newJustification);

    /** Sets the bounding box and the control point that controls the font size.
        The three bounding box points define the parallelogram within which the text will be
        placed. The fontSizeAndScaleAnchor specifies a position within that parallelogram, whose
        Y position (relative to the parallelogram's origin and possibly distorted shape) specifies
        the font's height, and its X defines the font's horizontal scale.
    */
    void setBounds (const RelativePoint& boundingBoxTopLeft,
                    const RelativePoint& boundingBoxTopRight,
                    const RelativePoint& boundingBoxBottomLeft,
                    const RelativePoint& fontSizeAndScaleAnchor);

    /** Returns the origin of the text bounding box. */
    const RelativePoint& getBoundingBoxTopLeft() const throw()          { return controlPoints[0]; }
    /** Returns the top-right of the text bounding box. */
    const RelativePoint& getBoundingBoxTopRight() const throw()         { return controlPoints[1]; }
    /** Returns the bottom-left of the text bounding box. */
    const RelativePoint& getBoundingBoxBottomLeft() const throw()       { return controlPoints[2]; }
    /** Returns the point within the text bounding box which defines the size and scale of the font. */
    const RelativePoint& getFontSizeAndScaleAnchor() const throw()      { return controlPoints[3]; }

    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    const Rectangle<float> getBounds() const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void invalidatePoints();
    /** @internal */
    const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }

    //==============================================================================
    /** Internally-used class for wrapping a DrawableText's state into a ValueTree. */
    class ValueTreeWrapper   : public ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        const String getText() const;
        void setText (const String& newText, UndoManager* undoManager);

        const Colour getColour() const;
        void setColour (const Colour& newColour, UndoManager* undoManager);

        const Justification getJustification() const;
        void setJustification (const Justification& newJustification, UndoManager* undoManager);

        const Font getFont() const;
        void setFont (const Font& newFont, UndoManager* undoManager);

        const RelativePoint getBoundingBoxTopLeft() const;
        void setBoundingBoxTopLeft (const RelativePoint& p, UndoManager* undoManager);

        const RelativePoint getBoundingBoxTopRight() const;
        void setBoundingBoxTopRight (const RelativePoint& p, UndoManager* undoManager);

        const RelativePoint getBoundingBoxBottomLeft() const;
        void setBoundingBoxBottomLeft (const RelativePoint& p, UndoManager* undoManager);

        const RelativePoint getFontSizeAndScaleAnchor() const;
        void setFontSizeAndScaleAnchor (const RelativePoint& p, UndoManager* undoManager);

        static const Identifier text, colour, font, justification, topLeft, topRight, bottomLeft, fontSizeAnchor;
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    RelativePoint controlPoints[4];
    Font font;
    String text;
    Colour colour;
    Justification justification;

    void resolveCorners (Point<float>* corners) const;

    DrawableText& operator= (const DrawableText&);
};


#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
