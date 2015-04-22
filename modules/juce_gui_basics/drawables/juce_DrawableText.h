/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_DRAWABLETEXT_H_INCLUDED
#define JUCE_DRAWABLETEXT_H_INCLUDED


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
    DrawableText (const DrawableText&);

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
    void setJustification (Justification newJustification);

    /** Returns the current justification. */
    Justification getJustification() const noexcept                     { return justification; }

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
    void paint (Graphics&) override;
    /** @internal */
    Drawable* createCopy() const override;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const override;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;

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
        void setJustification (Justification newJustification, UndoManager* undoManager);

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

    DrawableText& operator= (const DrawableText&);
    JUCE_LEAK_DETECTOR (DrawableText)
};


#endif   // JUCE_DRAWABLETEXT_H_INCLUDED
