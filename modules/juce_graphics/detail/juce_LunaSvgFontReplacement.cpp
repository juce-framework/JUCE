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

namespace juce::detail
{

/*  Very nearly a drop in replacement for lunasvg::Font. This allows us to bypass lunasvg's Font
    rendering and plug in JUCE's Font related functionality.
*/
class LunaSvgFontReplacement
{
public:
    LunaSvgFontReplacement() = default;

    LunaSvgFontReplacement (float size, const std::string& family, bool bold, bool italic)
    {
        const auto getStyleFlags = [] (const bool b, const bool i) noexcept
        {
            int flags{};

            if (b) flags |= Font::bold;
            if (i) flags |= Font::italic;

            return flags;
        };

        font = std::invoke (
            [&]
            {
                juce::StringArray tokens;
                tokens.addTokens (family, ",", "");
                std::vector<String> fallbacks ((size_t) tokens.size());

                std::transform (tokens.begin(),
                                tokens.end(),
                                fallbacks.begin(),
                                [] (const String& s) { return s.trim(); });

                auto options = FontOptions{}.withPointHeight (size)
                                            .withFallbacks (fallbacks)
                                            .withStyleFlags (getStyleFlags (bold, italic));

                if (! fallbacks.empty())
                    options = options.withName (fallbacks[0]);

                return Font { options };
            });

        GlyphArrangement arr;
        arr.addLineOfText (*font, "x", 0.0f, 0.0f);

        fontXHeight = std::invoke ([&]
        {
            if (arr.getNumGlyphs() == 0)
                return font->getHeight();

            auto typefacePtr = font->getTypefacePtr();

            if (typefacePtr == nullptr)
                return 0.0f;

            const auto xBoundsNormalised = typefacePtr->getGlyphBounds (arr.getGlyph (0).getGlyphIndex());
            return xBoundsNormalised.getHeight() * font->getHeightInPoints();
        });
    }

    float ascent() const
    {
        if (! font.has_value())
            return 0.0f;

        return font->getAscentInPoints();
    }

    float descent() const
    {
        if (! font.has_value())
            return 0.0f;

        return font->getDescentInPoints();
    }

    float height() const
    {
        if (! font.has_value())
            return 0.0f;

        // This is a little strange. Checking the numerical values inside lunasvg, we need to return
        // the font height, NOT the point height. But fontXHeight needs to be calculated using the
        // point height. This way we get the same text layout as in lunasvg. But the mix of the two
        // values is perplexing.
        return font->getHeight();
    }

    float lineGap() const
    {
        if (! font.has_value())
            return 0.0f;

        return font->getLineGapInPoints();
    }

    float xHeight() const
    {
        return fontXHeight;
    }

    float measureText (const std::u32string_view& text) const
    {
        if (! font.has_value())
            return 0.0f;

        const auto s = juce::String { juce::CharPointer_UTF32 (reinterpret_cast<const CharPointer_UTF32::CharType*> (text.data())),
                                      text.length() };
        return GlyphArrangement::getStringWidth (*font, s);
    }

    float size() const
    {
        if (! font.has_value())
            return 0.0f;

        return font->getHeightInPoints();
    }

    bool isNull() const
    {
        if (! font.has_value())
            return 0.0f;

        return ! font.has_value();
    }

    const Font& getFont() const
    {
        return *font;
    }

private:
    std::optional<Font> font;
    float fontXHeight = 0.0f;
};

} // namespace juce::detail
