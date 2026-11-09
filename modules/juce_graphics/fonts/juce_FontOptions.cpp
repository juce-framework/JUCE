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

FontOptions FontOptions::withStyleFlags (int x) const
{
    if (typeface == nullptr)
    {
        auto copy = withMember (*this, &FontOptions::style, FontStyleHelpers::getStyleName (x));
        copy.underlined = (x & Font::FontStyleFlags::underlined) != 0;
        return copy;
    }

    // This field will be ignored if the typeface pointer is non-null.
    // If you want to set a custom style, first set the typeface pointer to null.
    jassertfalse;
    return *this;
}

auto FontOptions::tie() const
{
    return std::tuple (name,
                       style,
                       typeface.get(),
                       fallbacks,
                       features,
                       variables,
                       metricsKind,
                       ascentOverride,
                       descentOverride,
                       height,
                       pointHeight,
                       tracking,
                       horizontalScale,
                       fallbackEnabled,
                       direct2DHinting,
                       underlined);
}

FontOptions FontOptions::withFeatureSetting (FontFeatureSetting newSetting) const
{
    auto copy = *this;

    OrderedContainerHelpers::insertOrAssign (copy.features,
                                             newSetting,
                                             FontComparators::FeatureSettingComparator{});

    return copy;
}

FontOptions FontOptions::withFeatureRemoved (FontFeatureTag featureTag) const
{
    auto copy = *this;

    OrderedContainerHelpers::remove (copy.features,
                                     featureTag,
                                     FontComparators::FeatureSettingComparator{});

    return copy;
}

FontOptions FontOptions::withVariableSetting (FontVariableSetting variableSetting) const
{
    jassert (std::isfinite (variableSetting.value));

    if (typeface == nullptr && style.isEmpty())
    {
        auto copy = *this;
        OrderedContainerHelpers::insertOrAssign (copy.variables,
                                                 variableSetting,
                                                 FontComparators::VariableSettingComparator{});
        return copy;
    }

    // Use Typeface::cloneWithVariableSettings to configure an existing typeface.
    jassertfalse;
    return *this;
}

FontOptions FontOptions::withVariableSettings (Span<const FontVariableSetting> variableSettings) const
{
    if (variableSettings.empty() || (typeface == nullptr && style.isEmpty()))
    {
        auto copy = *this;
        copy.variables = { variableSettings.begin(), variableSettings.end() };
        std::sort (copy.variables.begin(),
                   copy.variables.end(),
                   FontComparators::VariableSettingComparator{});

        return copy;
    }

    // Use Typeface::cloneWithVariableSettings to configure an existing typeface.
    jassertfalse;
    return *this;
}

FontOptions FontOptions::withVariableRemoved (FontFeatureTag featureTag) const
{
    if (typeface == nullptr && style.isEmpty())
    {
        auto copy = *this;
        OrderedContainerHelpers::remove (copy.variables,
                                         featureTag,
                                         FontComparators::VariableSettingComparator{});
        return copy;
    }

    // Use Typeface::cloneWithVariableSettings to configure an existing typeface.
    jassertfalse;
    return *this;
}

bool FontOptions::operator== (const FontOptions& other) const { return tie() == other.tie(); }
bool FontOptions::operator!= (const FontOptions& other) const { return tie() != other.tie(); }
bool FontOptions::operator<  (const FontOptions& other) const { return tie() <  other.tie(); }
bool FontOptions::operator<= (const FontOptions& other) const { return tie() <= other.tie(); }
bool FontOptions::operator>  (const FontOptions& other) const { return tie() >  other.tie(); }
bool FontOptions::operator>= (const FontOptions& other) const { return tie() >= other.tie(); }

#if JUCE_UNIT_TESTS

class FontFeatureContainerTests : public UnitTest
{
public:
    FontFeatureContainerTests() : UnitTest ("FontFeatureContainerTests", UnitTestCategories::fonts)
    {
    }

    void runTest() override
    {
        beginTest ("Features can be enabled");
        {
            const auto options = FontOptions{}.withFeatureEnabled ("clig");

            expectEquals ((int) options.getFeatureSettings().size(), 1);
            expect (compareFeatureLists (options.getFeatureSettings(),
            {
                FontFeatureSetting ("clig", FontFeatureSetting::featureEnabled)
            }));
        }

        beginTest ("Features can be disabled");
        {
            const auto options = FontOptions{}.withFeatureDisabled ("clig");

            expectEquals ((int) options.getFeatureSettings().size(), 1);
            expect (compareFeatureLists (options.getFeatureSettings(),
            {
                FontFeatureSetting ("clig", FontFeatureSetting::featureDisabled)
            }));
        }

        beginTest ("Features can be removed");
        {
            const auto options = FontOptions{}.withFeatureEnabled ("clig")
                                              .withFeatureRemoved ("clig");

            expectEquals ((int) options.getFeatureSettings().size(), 0);
            expect (compareFeatureLists (options.getFeatureSettings(), {}));
        }

        beginTest ("Duplicate features are not allowed");
        {
            const auto options = FontOptions{}.withFeatureEnabled ("clig")
                                              .withFeatureEnabled ("clig");

            expectEquals ((int) options.getFeatureSettings().size(), 1);
        }

        beginTest ("Features are always sorted by tag");
        {
            const auto options = FontOptions{}.withFeatureEnabled ("clig")
                                              .withFeatureDisabled ("blig")
                                              .withFeatureEnabled ("alig");

            expectEquals ((int) options.getFeatureSettings().size(), 3);
            expect (compareFeatureLists (options.getFeatureSettings(),
            {
                FontFeatureSetting ("alig", FontFeatureSetting::featureEnabled),
                FontFeatureSetting ("blig", FontFeatureSetting::featureDisabled),
                FontFeatureSetting ("clig", FontFeatureSetting::featureEnabled)
            }));
        }
    }

private:
    static bool compareFeatureLists (Span<const FontFeatureSetting> input,
                                     std::initializer_list<const FontFeatureSetting> expected)
    {
        return std::equal (input.begin(), input.end(), expected.begin(), expected.end());
    }
};

static FontFeatureContainerTests fontFeatureContainerTests;

class FontVariableSettingsTests : public UnitTest
{
public:
    FontVariableSettingsTests() : UnitTest ("FontVariableSettingsTests", UnitTestCategories::fonts)
    {
    }

    void runTest() override
    {
        beginTest ("Variable settings can be added");
        {
            const auto options = FontOptions{}.withStyle ("")
                                              .withVariableSetting (FontVariableSetting ("wght", 700.0f));

            expectEquals ((int) options.getVariableSettings().size(), 1);
            expect (compareVariableLists (options.getVariableSettings(), { FontVariableSetting ("wght", 700.0f) }));
        }

        beginTest ("Variable settings can be updated");
        {
            const auto options = FontOptions{}.withStyle ("")
                                              .withVariableSetting (FontVariableSetting ("wght", 700.0f))
                                              .withVariableSetting (FontVariableSetting ("wght", 400.0f));

            expectEquals ((int) options.getVariableSettings().size(), 1);
            expect (compareVariableLists (options.getVariableSettings(), { FontVariableSetting ("wght", 400.0f) }));
        }

        beginTest ("Multiple variable settings can be added");
        {
            const auto options = FontOptions{}.withStyle ("")
                                              .withVariableSetting (FontVariableSetting ("wght", 700.0f))
                                              .withVariableSetting (FontVariableSetting ("wdth", 75.0f));

            expectEquals ((int) options.getVariableSettings().size(), 2);
        }

        beginTest ("Variable settings are always sorted by tag");
        {
            const auto options = FontOptions{}.withStyle ("")
                                              .withVariableSetting (FontVariableSetting ("wght", 700.0f))
                                              .withVariableSetting (FontVariableSetting ("wdth", 75.0f))
                                              .withVariableSetting (FontVariableSetting ("slnt", -10.0f));

            expectEquals ((int) options.getVariableSettings().size(), 3);
            expect (compareVariableLists (options.getVariableSettings(), { FontVariableSetting ("slnt", -10.0f),
                                                                           FontVariableSetting ("wdth", 75.0f),
                                                                           FontVariableSetting ("wght", 700.0f) }));
        }

        beginTest ("Variable settings can be removed");
        {
            const auto options = FontOptions{}.withStyle ("")
                                              .withVariableSetting (FontVariableSetting ("wght", 700.0f))
                                              .withVariableSetting (FontVariableSetting ("wdth", 75.0f))
                                              .withVariableRemoved (FontFeatureTag ("wght"));

            expectEquals ((int) options.getVariableSettings().size(), 1);
            expect (compareVariableLists (options.getVariableSettings(), { FontVariableSetting ("wdth", 75.0f) }));
        }

        beginTest ("Multiple variable settings can be set at once with withVariableSettings");
        {
            std::vector<FontVariableSetting> settings = { FontVariableSetting ("wght", 700.0f),
                                                          FontVariableSetting ("wdth", 75.0f),
                                                          FontVariableSetting ("slnt", -10.0f) };

            const auto options = FontOptions{}.withStyle ("").withVariableSettings (settings);

            expectEquals ((int) options.getVariableSettings().size(), 3);
            expect (compareVariableLists (options.getVariableSettings(), { FontVariableSetting ("slnt", -10.0f),
                                                                           FontVariableSetting ("wdth", 75.0f),
                                                                           FontVariableSetting ("wght", 700.0f) }));
        }
    }

private:
    static bool compareVariableLists (Span<const FontVariableSetting> input,
                                      std::initializer_list<const FontVariableSetting> expected)
    {
        return std::equal (input.begin(), input.end(), expected.begin(), expected.end());
    }
};

static FontVariableSettingsTests fontVariableSettingsTests;

#endif


} // namespace juce
