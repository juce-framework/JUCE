/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

    @tags{GUI}
*/
class JUCE_API  DrawableText  : public Drawable
{
public:
    //==============================================================================
    /** Creates a DrawableText object. */
    DrawableText();
    DrawableText (const DrawableText&);

    /** Destructor. */
    ~DrawableText() override;

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
        Note that the font height and horizontal scale are set using setFontHeight() and
        setFontHorizontalScale(). If applySizeAndScale is true, then these height
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
    Parallelogram<float> getBoundingBox() const noexcept                { return bounds; }

    /** Sets the bounding box that contains the text. */
    void setBoundingBox (Parallelogram<float> newBounds);

    float getFontHeight() const noexcept                                { return fontHeight; }
    void setFontHeight (float newHeight);

    float getFontHorizontalScale() const noexcept                       { return fontHScale; }
    void setFontHorizontalScale (float newScale);

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    bool replaceColour (Colour originalColour, Colour replacementColour) override;

private:
    //==============================================================================
    Parallelogram<float> bounds;
    float fontHeight, fontHScale;
    Font font, scaledFont;
    String text;
    Colour colour;
    Justification justification;

    void refreshBounds();
    Rectangle<int> getTextArea (float width, float height) const;
    AffineTransform getTextTransform (float width, float height) const;

    DrawableText& operator= (const DrawableText&);
    JUCE_LEAK_DETECTOR (DrawableText)
};

} // namespace juce
