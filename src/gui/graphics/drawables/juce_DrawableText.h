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

    /** Returns the parallelogram that defines the text bounding box. */
    const RelativeParallelogram& getBoundingBox() const throw()         { return bounds; }

    /** Sets the bounding box that contains the text. */
    void setBoundingBox (const RelativeParallelogram& newBounds);

    /** Returns the point within the bounds that defines the font's size and scale. */
    const RelativePoint& getFontSizeControlPoint() const throw()        { return fontSizeControlPoint; }

    /** Sets the control point that defines the font's height and horizontal scale.
        This position is a point within the bounding box parallelogram, whose Y position (relative
        to the parallelogram's origin and possibly distorted shape) specifies the font's height,
        and its X defines the font's horizontal scale.
    */
    void setFontSizeControlPoint (const RelativePoint& newPoint);


    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }
    /** @internal */
    const Rectangle<float> getDrawableBounds() const;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableText's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        const String getText() const;
        void setText (const String& newText, UndoManager* undoManager);
        Value getTextValue (UndoManager* undoManager);

        const Colour getColour() const;
        void setColour (const Colour& newColour, UndoManager* undoManager);

        const Justification getJustification() const;
        void setJustification (const Justification& newJustification, UndoManager* undoManager);

        const Font getFont() const;
        void setFont (const Font& newFont, UndoManager* undoManager);
        Value getFontValue (UndoManager* undoManager);

        const RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager);

        const RelativePoint getFontSizeControlPoint() const;
        void setFontSizeControlPoint (const RelativePoint& p, UndoManager* undoManager);

        static const Identifier text, colour, font, justification, topLeft, topRight, bottomLeft, fontSizeAnchor;
    };

private:
    //==============================================================================
    RelativeParallelogram bounds;
    RelativePoint fontSizeControlPoint;
    Font font;
    String text;
    Colour colour;
    Justification justification;

    void refreshBounds();

    DrawableText& operator= (const DrawableText&);
    JUCE_LEAK_DETECTOR (DrawableText);
};


#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
