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

#ifndef __JUCE_DRAWABLETEXT_JUCEHEADER__
#define __JUCE_DRAWABLETEXT_JUCEHEADER__

#include "juce_Drawable.h"
#include "../positioning/juce_RelativeParallelogram.h"


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

    /** Returns the currently displayed text */
    const String& getText() const noexcept                              { return text;}

    /** Sets the colour of the text. */
    void setColour (Colour newColour);

    /** Returns the current text colour. */
    Colour getColour() const noexcept                                   { return colour; }

    /** Sets the font to use.
        Note that the font height and horizontal scale are set as RelativeCoordinates using
        setFontHeight and setFontHorizontalScale. If applySizeAndScale is true, then these height
        and scale values will be changed to match the dimensions of the font supplied;
        if it is false, then the new font object's height and scale are ignored.
    */
    void setFont (const Font& newFont, bool applySizeAndScale);

    /** Returns the current font. */
    const Font& getFont() const noexcept                                { return font; }

    /** Changes the justification of the text within the bounding box. */
    void setJustification (const Justification& newJustification);

    /** Returns the current justification. */
    const Justification& getJustification() const noexcept              { return justification; }

    /** Returns the parallelogram that defines the text bounding box. */
    const RelativeParallelogram& getBoundingBox() const noexcept        { return bounds; }

    /** Sets the bounding box that contains the text. */
    void setBoundingBox (const RelativeParallelogram& newBounds);

    const RelativeCoordinate& getFontHeight() const                     { return fontHeight; }
    void setFontHeight (const RelativeCoordinate& newHeight);

    const RelativeCoordinate& getFontHorizontalScale() const            { return fontHScale; }
    void setFontHorizontalScale (const RelativeCoordinate& newScale);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    Rectangle<float> getDrawableBounds() const;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableText's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        String getText() const;
        void setText (const String& newText, UndoManager* undoManager);
        Value getTextValue (UndoManager* undoManager);

        Colour getColour() const;
        void setColour (Colour newColour, UndoManager* undoManager);

        Justification getJustification() const;
        void setJustification (const Justification& newJustification, UndoManager* undoManager);

        Font getFont() const;
        void setFont (const Font& newFont, UndoManager* undoManager);
        Value getFontValue (UndoManager* undoManager);

        RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager);

        RelativeCoordinate getFontHeight() const;
        void setFontHeight (const RelativeCoordinate& newHeight, UndoManager* undoManager);

        RelativeCoordinate getFontHorizontalScale() const;
        void setFontHorizontalScale (const RelativeCoordinate& newScale, UndoManager* undoManager);

        static const Identifier text, colour, font, justification, topLeft, topRight, bottomLeft, fontHeight, fontHScale;
    };

private:
    //==============================================================================
    RelativeParallelogram bounds;
    RelativeCoordinate fontHeight, fontHScale;
    Point<float> resolvedPoints[3];
    Font font, scaledFont;
    String text;
    Colour colour;
    Justification justification;

    friend class Drawable::Positioner<DrawableText>;
    bool registerCoordinates (RelativeCoordinatePositionerBase&);
    void recalculateCoordinates (Expression::Scope*);
    void refreshBounds();
    const AffineTransform getArrangementAndTransform (GlyphArrangement& glyphs) const;

    DrawableText& operator= (const DrawableText&);
    JUCE_LEAK_DETECTOR (DrawableText)
};


#endif   // __JUCE_DRAWABLETEXT_JUCEHEADER__
