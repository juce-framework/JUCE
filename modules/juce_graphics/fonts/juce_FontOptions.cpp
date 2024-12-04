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

struct FontValues
{
    static float limitFontHeight (const float height) noexcept
    {
        return jlimit (0.1f, 10000.0f, height);
    }

    inline static constexpr float defaultFontHeight = 14.0f;
    static float minimumHorizontalScale;

    FontValues() = delete;
};

float FontValues::minimumHorizontalScale = 0.7f;

FontOptions::FontOptions()
    : FontOptions (FontValues::defaultFontHeight, Font::plain)
{
}

FontOptions::FontOptions (float fontHeight)
    : FontOptions (fontHeight, Font::plain)
{
}

FontOptions::FontOptions (float fontHeight, int styleFlags)
    : FontOptions ({}, fontHeight, styleFlags)
{
}

FontOptions::FontOptions (const String& typefaceName, float fontHeight, int styleFlags)
    : FontOptions (typefaceName, FontStyleHelpers::getStyleName (styleFlags), fontHeight)
{
    underlined = (styleFlags & Font::FontStyleFlags::underlined) != 0;
}

FontOptions::FontOptions (const String& typefaceName, const String& typefaceStyle, float fontHeight)
    : name (typefaceName),
      style (typefaceStyle),
      height (FontValues::limitFontHeight (fontHeight))
{
}

FontOptions::FontOptions (const Typeface::Ptr& ptr)
    : name (ptr->getName()),
      style (ptr->getStyle()),
      typeface (ptr),
      height (FontValues::defaultFontHeight)
{
}

auto FontOptions::tie() const
{
    return std::tuple (name,
                       style,
                       typeface.get(),
                       fallbacks,
                       metricsKind,
                       height,
                       pointHeight,
                       tracking,
                       horizontalScale,
                       fallbackEnabled,
                       underlined);
}

bool FontOptions::operator== (const FontOptions& other) const { return tie() == other.tie(); }
bool FontOptions::operator!= (const FontOptions& other) const { return tie() != other.tie(); }
bool FontOptions::operator<  (const FontOptions& other) const { return tie() <  other.tie(); }
bool FontOptions::operator<= (const FontOptions& other) const { return tie() <= other.tie(); }
bool FontOptions::operator>  (const FontOptions& other) const { return tie() >  other.tie(); }
bool FontOptions::operator>= (const FontOptions& other) const { return tie() >= other.tie(); }

} // namespace juce
