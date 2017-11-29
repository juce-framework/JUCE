/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
    /** @internal */
    Path getOutlineAsPath() const override;

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
    Rectangle<int> getTextArea (float width, float height) const;
    AffineTransform getTextTransform (float width, float height) const;

    DrawableText& operator= (const DrawableText&);
    JUCE_LEAK_DETECTOR (DrawableText)
};

} // namespace juce
