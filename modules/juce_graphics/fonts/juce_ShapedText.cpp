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

/** Class that can visually shape a Unicode string provided a list of Fonts corresponding to
    sub-ranges of the string.
*/
class JUCE_API  ShapedText
{
public:
    using Options = ShapedTextOptions;

    ShapedText();

    explicit ShapedText (String text);

    ShapedText (String text, Options options);

    /** Returns the text which was used to construct this object. */
    const String& getText() const;

    /** Returns the text's codepoint range, to which the glyph under the provided index belongs.

        This range will have a length of at least one, and potentially more than one if ligatures
        are enabled.
    */
    Range<int64> getTextRange (int64 glyphIndex) const;

    /** Returns the widths for each line, that the glyphs would require to be rendered without being
        truncated. This will or will not include the space required by trailing whitespaces in the
        line based on the ShapedTextOptions::withTrailingWhitespacesShouldFit() value.

        This value isn't affected by the Justification parameter, it just reports the amount of
        width that would be required to avoid truncation.
     */
    Span<const float> getMinimumRequiredWidthForLines() const;

    /** Provides access to the data stored in the ShapedText.

        The provided function callback will be called multiple times for "uniform glyph runs", for which all
        callback parameters are the same.

        Between each subsequent callback at least one of the provided parameters will be different.

        The callbacks happen in visual order i.e. left to right, which is irrespective of the
        underlying text's writing direction.

        The callback parameters in order are:
        - the glyphs
        - the positions for each glyph in the previous parameter
        - the Font with which these glyphs should be rendered
        - the range in all glyphs this ShapedText object holds, that correspond to the current glyphs
        - a line number which increases by one for each new line
    */
    void access (const std::function<void (Span<const ShapedGlyph>, Span<Point<float>>, Font, Range<int64>, int64)>&) const;

    /** Draws the text. */
    void draw (const Graphics& g, AffineTransform transform) const;

    /** @internal */
    class Detail;

    /** @internal */
    Detail getDetail() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl;
};

class ShapedText::Impl
{
public:
    Impl (String textIn, Options optionsIn)
        : options { std::move (optionsIn) },
          text { std::move (textIn) }
    {
    }

    void access (const std::function<void (Span<const ShapedGlyph>, Span<Point<float>>, Font, Range<int64>, int64)>& cb) const
    {
        justifiedText.access (cb);
    }

    void draw (const Graphics& g, AffineTransform transform) const
    {
        drawJustifiedText (justifiedText, g, transform);
    }

    auto& getText() const
    {
        return text;
    }

    auto getTextRange (int64 glyphIndex) const
    {
        return simpleShapedText.getTextRange (glyphIndex);
    }

    Span<const float> getMinimumRequiredWidthForLines() const
    {
        return justifiedText.getMinimumRequiredWidthForLines();
    }

    //==============================================================================
    auto& getSimpleShapedText() const { return simpleShapedText; }

    auto& getJustifiedText() const { return justifiedText; }

private:
    ShapedTextOptions options;
    String text;
    SimpleShapedText simpleShapedText { &text, options };
    JustifiedText justifiedText { simpleShapedText, options };
};

ShapedText::ShapedText() : ShapedText ("", {})
{
}

ShapedText::ShapedText (String text) : ShapedText (std::move (text), {})
{
}

ShapedText::ShapedText (String text, Options options)
{
    impl = std::make_shared<Impl> (std::move (text), std::move (options));
}

void ShapedText::access (const std::function<void (Span<const ShapedGlyph>, Span<Point<float>>, Font, Range<int64>, int64)>& cb) const
{
    impl->access (cb);
}

void ShapedText::draw (const Graphics& g, AffineTransform transform) const
{
    impl->draw (g, transform);
}

const String& ShapedText::getText() const
{
    return impl->getText();
}

Range<int64> ShapedText::getTextRange (int64 glyphIndex) const
{
    return impl->getTextRange (glyphIndex);
}

Span<const float> ShapedText::getMinimumRequiredWidthForLines() const
{
    return impl->getMinimumRequiredWidthForLines();
}

class ShapedText::Detail
{
public:
    explicit Detail (const ShapedText* shapedTextIn)
        : shapedText (*shapedTextIn)
    {}

    auto& getJustifiedText() const { return shapedText.impl->getJustifiedText(); }

    auto& getSimpleShapedText() const { return shapedText.impl->getSimpleShapedText(); }

private:
    const ShapedText& shapedText;
};

ShapedText::Detail ShapedText::getDetail() const
{
    return Detail { this };
}

} // namespace juce
