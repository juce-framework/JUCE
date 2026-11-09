/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
    DrawableText() = default;

    //==============================================================================
    /** Sets the text to display.*/
    void setText (const String& newText);

    /** Sets whether whitespace should always be preserved when drawing. The default value is false.

        Historically, DrawableText would sometimes trim the provided text during drawing based on
        its contents. With this value set to true, trimming will never occur.
    */
    void setPreserveWhitespace (bool shouldPreserveWhitespace);

    /** Returns the currently displayed text */
    const String& getText() const noexcept                              { return text;}

    /** Sets the colour of the text. */
    void setColour (Colour newColour);

    /** Sets a fill type for the text. */
    void setFill (const FillType& newFill);

    /** Returns the current text colour. */
    Colour getColour() const noexcept                                   { return colour; }

    /** Changes the properties of the outline that will be drawn around the text.

        By default the text has no stroke. Specifying any StrokeOptions with a width
        of 0.0f will be similarly not drawn.

        @see setStrokeThickness, setStrokeColour
    */
    void setStrokeOptions (const StrokeOptions& newStrokeOptions);

    /** Returns the currently specified stroke attributes. A stroke with a width of 0.0f
        is considered invisible, and will not be drawn.

        @see setFill
    */
    const StrokeOptions& getStrokeOptions()                             { return strokeOptions; }

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

    bool hitTest (Point<float> p) const override;

    float getFontHeight() const noexcept                                { return fontHeight; }
    void setFontHeight (float newHeight);

    float getFontHorizontalScale() const noexcept                       { return fontHScale; }
    void setFontHorizontalScale (float newScale);

    //==============================================================================
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBoundsUntransformed() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    bool replaceColour (Colour originalColour, Colour replacementColour) override;
    /** @internal */
    std::optional<String> getContainedText() const override;

private:
    template <typename Object, typename Member, typename Other>
    void setMember (Member Object::* member, const Other& value);

    Rectangle<float> getBoundingRectangle() const;

    void update();

    void paint (Graphics& g) const override;
    AffineTransform getTransform() const override;

    //==============================================================================
    Parallelogram<float> bounds { { 0.0f, 0.0f, 50.0f, 20.0f } };
    Font font { FontOptions (15.0f) };
    Font scaledFont { FontOptions{} };
    float fontHeight = font.getHeight();
    float fontHScale = font.getHorizontalScale();
    String text;
    bool preserveWhitespace = false;
    Colour colour { Colours::black };
    FillType fill;
    Justification justification { Justification::centredLeft };
    StrokeOptions strokeOptions = StrokeOptions{}.withWidth (0.0f);
    Path strokePath;

    Rectangle<float> getTextArea() const;
    AffineTransform getTextTransform() const;

    JUCE_LEAK_DETECTOR (DrawableText)
};

} // namespace juce
