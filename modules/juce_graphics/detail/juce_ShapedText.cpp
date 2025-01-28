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

namespace juce::detail
{

class ShapedText::Impl
{
public:
    Impl (String textIn, Options optionsIn)
        : options { std::move (optionsIn) },
          text { std::move (textIn) }
    {
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
    JustifiedText justifiedText { &simpleShapedText, options };
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

const JustifiedText& ShapedText::getJustifiedText() const { return impl->getJustifiedText(); }

const SimpleShapedText& ShapedText::getSimpleShapedText() const { return impl->getSimpleShapedText(); }

} // namespace juce::detail
