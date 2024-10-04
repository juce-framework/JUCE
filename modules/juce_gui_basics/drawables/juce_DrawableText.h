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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Parallelogram<float> bounds;
    float fontHeight, fontHScale;
    Font font { withDefaultMetrics (FontOptions{}) }, scaledFont { withDefaultMetrics (FontOptions{}) };
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
