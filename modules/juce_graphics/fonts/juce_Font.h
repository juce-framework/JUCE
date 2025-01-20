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

/**
    Represents a particular font, including its size, style, etc.

    Apart from the typeface to be used, a Font object also dictates whether
    the font is bold, italic, underlined, how big it is, and its kerning and
    horizontal scale factor.

    @see Typeface

    @tags{Graphics}
*/
class JUCE_API  Font  final
{
public:
    //==============================================================================
    /** A combination of these values is used by the constructor to specify the
        style of font to use.
    */
    enum FontStyleFlags
    {
        plain       = 0,    /**< indicates a plain, non-bold, non-italic version of the font. @see setStyleFlags */
        bold        = 1,    /**< boldens the font. @see setStyleFlags */
        italic      = 2,    /**< finds an italic version of the font. @see setStyleFlags */
        underlined  = 4     /**< underlines the font. @see setStyleFlags */
    };

    /** Constructs a Font from a set of options describing the font. */
    Font (FontOptions options);

    //==============================================================================
    /** Creates a sans-serif font in a given size.

        @param fontHeight   the height in pixels (can be fractional)
        @param styleFlags   the style to use - this can be a combination of the
                            Font::bold, Font::italic and Font::underlined, or
                            just Font::plain for the normal style.
        @see FontStyleFlags, getDefaultSansSerifFontName
    */
    [[deprecated ("Use the constructor that takes a FontOptions argument")]]
    Font (float fontHeight, int styleFlags = plain);

    /** Creates a font with a given typeface and parameters.

        @param typefaceName the font family of the typeface to use
        @param fontHeight   the height in pixels (can be fractional)
        @param styleFlags   the style to use - this can be a combination of the
                            Font::bold, Font::italic and Font::underlined, or
                            just Font::plain for the normal style.
        @see FontStyleFlags, getDefaultSansSerifFontName
    */
    [[deprecated ("Use the constructor that takes a FontOptions argument")]]
    Font (const String& typefaceName, float fontHeight, int styleFlags);

    /** Creates a font with a given typeface and parameters.

        @param typefaceName  the font family of the typeface to use
        @param typefaceStyle the font style of the typeface to use
        @param fontHeight    the height in pixels (can be fractional)
    */
    [[deprecated ("Use the constructor that takes a FontOptions argument")]]
    Font (const String& typefaceName, const String& typefaceStyle, float fontHeight);

    /** Creates a font for a typeface. */
    [[deprecated ("Use the constructor that takes a FontOptions argument")]]
    Font (const Typeface::Ptr& typeface);

    /** Creates a basic sans-serif font at a default height.

        You should use one of the other constructors for creating a font that you're planning
        on drawing with - this constructor is here to help initialise objects before changing
        the font's settings later.
    */
    [[deprecated ("Use the constructor that takes a FontOptions argument")]]
    Font();

    /** Creates a copy of another Font object. */
    Font (const Font& other) noexcept;

    /** Move constructor */
    Font (Font&& other) noexcept;

    /** Move assignment operator */
    Font& operator= (Font&& other) noexcept;

    /** Copies this font from another one. */
    Font& operator= (const Font& other) noexcept;

    bool operator== (const Font& other) const noexcept;
    bool operator!= (const Font& other) const noexcept;

    /** Destructor. */
    ~Font() noexcept;

    //==============================================================================
    /** Changes the font family of the typeface.

        e.g. "Arial", "Courier", etc.

        This may also be set to Font::getDefaultSansSerifFontName(), Font::getDefaultSerifFontName(),
        or Font::getDefaultMonospacedFontName(), which are not actual platform-specific font family names,
        but are generic font family names that are used to represent the various default fonts.
        If you need to know the exact typeface font family being used, you can call
        Font::getTypefacePtr()->getName(), which will give you the platform-specific font family.

        If a suitable font isn't found on the machine, it'll just use a default instead.
    */
    void setTypefaceName (const String& faceName);

    /** Returns the font family of the typeface that this font uses.

        e.g. "Arial", "Courier", etc.

        This may also be set to Font::getDefaultSansSerifFontName(), Font::getDefaultSerifFontName(),
        or Font::getDefaultMonospacedFontName(), which are not actual platform-specific font family names,
        but are generic font family names that are used to represent the various default fonts.

        If you need to know the exact typeface font family being used, you can call
        Font::getTypefacePtr()->getName(), which will give you the platform-specific font family.
    */
    String getTypefaceName() const noexcept;

    //==============================================================================
    /** Returns the font style of the typeface that this font uses.
        @see withTypefaceStyle, getAvailableStyles()
    */
    String getTypefaceStyle() const noexcept;

    /** Changes the font style of the typeface.
        @see getAvailableStyles()
    */
    void setTypefaceStyle (const String& newStyle);

    /** Returns a copy of this font with a new typeface style.
        @see getAvailableStyles()
    */
    [[nodiscard]] Font withTypefaceStyle (const String& newStyle) const;

    /** Returns a list of the styles that this font can use. */
    StringArray getAvailableStyles() const;

    //==============================================================================
    /** Sets the names of the fallback font families that should be tried, in order,
        when searching for glyphs that are missing in the main typeface, specified via
        setTypefaceName() or Font(const Typeface::Ptr&).
    */
    void setPreferredFallbackFamilies (const StringArray& fallbacks);

    /** Returns the names of the fallback font families.
    */
    StringArray getPreferredFallbackFamilies() const;

    /** When drawing text using this Font, specifies whether glyphs that are missing in the main
        typeface should be replaced with glyphs from other fonts.
        To find missing glyphs, the typefaces for the preferred fallback families will be checked
        in order, followed by the system fallback fonts. The system fallback font is likely to be
        different on each platform.

        Fallback is enabled by default.
    */
    void setFallbackEnabled (bool enabled);

    /** Returns true if fallback is enabled, or false otherwise. */
    bool getFallbackEnabled() const;

    //==============================================================================
    /** Returns a typeface font family that represents the default sans-serif font.

        This is also the typeface that will be used when a font is created without
        specifying any typeface details.

        Note that this method just returns a generic placeholder string that means "the default
        sans-serif font" - it's not the actual font family of this font.

        @see setTypefaceName, getDefaultSerifFontName, getDefaultMonospacedFontName
    */
    static const String& getDefaultSansSerifFontName();

    /** Returns a typeface font family that represents the system UI font.

        Note that this method just returns a generic placeholder string that means "the default
        UI font" - it's not the actual font family of this font.

        @see getDefaultSansSerifFontName, setTypefaceName
    */
    static const String& getSystemUIFontName();

    /** Returns a typeface font family that represents the default serif font.

        Note that this method just returns a generic placeholder string that means "the default
        serif font" - it's not the actual font family of this font.

        @see setTypefaceName, getDefaultSansSerifFontName, getDefaultMonospacedFontName
    */
    static const String& getDefaultSerifFontName();

    /** Returns a typeface font family that represents the default monospaced font.

        Note that this method just returns a generic placeholder string that means "the default
        monospaced font" - it's not the actual font family of this font.

        @see setTypefaceName, getDefaultSansSerifFontName, getDefaultSerifFontName
    */
    static const String& getDefaultMonospacedFontName();

    /** Returns a font style name that represents the default style.

        Note that this method just returns a generic placeholder string that means "the default
        font style" - it's not the actual name of the font style of any particular font.

        @see setTypefaceStyle
    */
    static const String& getDefaultStyle();

    /** Returns the default system typeface for the given font.

        Note: This will only ever return the typeface for the font's "main" family.
        Before attempting to render glyphs from this typeface, it's a good idea to check
        that those glyphs are present in the typeface, and to select a different
        face if necessary.
    */
    static Typeface::Ptr getDefaultTypefaceForFont (const Font& font);

    //==============================================================================
    /** Returns a copy of this font with a new height. */
    [[nodiscard]] Font withHeight (float height) const;

    /** Returns a copy of this font with a new height, specified in points. */
    [[nodiscard]] Font withPointHeight (float heightInPoints) const;

    /** Changes the font's height.

        The font will be scaled so that the sum of the ascender and descender is equal to the
        provided height in logical pixels.

        @see setPointHeight, getHeight, withHeight, setHeightWithoutChangingWidth
    */
    void setHeight (float newHeight);

    /** Changes the font's height.

        The argument specifies the size of the font's em-square in logical pixels.

        @see setHeight, getHeight, withHeight, setHeightWithoutChangingWidth
    */
    void setPointHeight (float newHeight);

    /** Changes the font's height without changing its width.
        This alters the horizontal scale to compensate for the change in height.
    */
    void setHeightWithoutChangingWidth (float newHeight);

    /** Returns the total height of this font, in pixels.
        This is the maximum height, from the top of the ascent to the bottom of the
        descenders.

        @see withHeight, setHeightWithoutChangingWidth, getAscent
    */
    float getHeight() const noexcept;

    /** Returns the total height of this font, in points.
        This is the maximum height, from the top of the ascent to the bottom of the
        descenders.

        @see withPointHeight, getHeight
    */
    float getHeightInPoints() const;

    /** Returns the height of the font above its baseline, in pixels.
        This is the maximum height from the baseline to the top.
        @see getHeight, getDescent
    */
    float getAscent() const;

    /** Returns the height of the font above its baseline, in points.
        This is the maximum height from the baseline to the top.
        @see getHeight, getDescent
    */
    float getAscentInPoints() const;

    /** Returns the amount that the font descends below its baseline, in pixels.
        This is calculated as (getHeight() - getAscent()).
        @see getAscent, getHeight
    */
    float getDescent() const;

    /** Returns the amount that the font descends below its baseline, in points.
        This is calculated as (getHeight() - getAscent()).
        @see getAscent, getHeight
    */
    float getDescentInPoints() const;

    //==============================================================================
    /** Returns the font's style flags.
        This will return a bitwise-or'ed combination of values from the FontStyleFlags
        enum, to describe whether the font is bold, italic, etc.
        @see FontStyleFlags, withStyle
    */
    int getStyleFlags() const noexcept;

    /** Returns a copy of this font with the given set of style flags.
        @param styleFlags     a bitwise-or'ed combination of values from the FontStyleFlags enum.
        @see FontStyleFlags, getStyleFlags
    */
    [[nodiscard]] Font withStyle (int styleFlags) const;

    /** Changes the font's style.
        @param newFlags     a bitwise-or'ed combination of values from the FontStyleFlags enum.
        @see FontStyleFlags, withStyle
    */
    void setStyleFlags (int newFlags);

    //==============================================================================
    /** Makes the font bold or non-bold. */
    void setBold (bool shouldBeBold);

    /** Returns a copy of this font with the bold attribute set.
        If the font does not have a bold version, this will return the default font.
     */
    [[nodiscard]] Font boldened() const;

    /** Returns true if the font is bold. */
    bool isBold() const noexcept;

    /** Makes the font italic or non-italic. */
    void setItalic (bool shouldBeItalic);
    /** Returns a copy of this font with the italic attribute set. */
    [[nodiscard]] Font italicised() const;
    /** Returns true if the font is italic. */
    bool isItalic() const noexcept;

    /** Makes the font underlined or non-underlined. */
    void setUnderline (bool shouldBeUnderlined);
    /** Returns true if the font is underlined. */
    bool isUnderlined() const noexcept;

    /** Returns the kind of metrics used by this Font. */
    TypefaceMetricsKind getMetricsKind() const noexcept;

    //==============================================================================
    /** Returns the font's horizontal scale.
        A value of 1.0 is the normal scale, less than this will be narrower, greater
        than 1.0 will be stretched out.

        @see withHorizontalScale
    */
    float getHorizontalScale() const noexcept;

    /** Returns a copy of this font with a new horizontal scale.
        @param scaleFactor  a value of 1.0 is the normal scale, less than this will be
                            narrower, greater than 1.0 will be stretched out.
        @see getHorizontalScale
    */
    [[nodiscard]] Font withHorizontalScale (float scaleFactor) const;

    /** Changes the font's horizontal scale factor.
        @param scaleFactor  a value of 1.0 is the normal scale, less than this will be
                            narrower, greater than 1.0 will be stretched out.
    */
    void setHorizontalScale (float scaleFactor);

    /** Returns the minimum horizontal scale to which fonts may be squashed when trying to
        create a layout.
        @see setDefaultMinimumHorizontalScaleFactor
    */
    static float getDefaultMinimumHorizontalScaleFactor() noexcept;

    /** Sets the minimum horizontal scale to which fonts may be squashed when trying to
        create a text layout.
        @see getDefaultMinimumHorizontalScaleFactor
    */
    static void setDefaultMinimumHorizontalScaleFactor (float newMinimumScaleFactor) noexcept;

    /** Returns the font's tracking, i.e. spacing applied between characters in
        addition to the kerning defined by the font.

        This is the extra space added between adjacent characters, as a proportion
        of the font's height.

        A value of zero is normal spacing, positive values will spread the letters
        out more, and negative values make them closer together.
    */
    float getExtraKerningFactor() const noexcept;

    /** Returns a copy of this font with a new tracking factor.
        @param extraKerning     a multiple of the font's height that will be added
                                to space between the characters. So a value of zero is
                                normal spacing, positive values spread the letters out,
                                negative values make them closer together.
    */
    [[nodiscard]] Font withExtraKerningFactor (float extraKerning) const;

    /** Changes the font's tracking.
        @param extraKerning     a multiple of the font's height that will be added
                                to space between the characters. So a value of zero is
                                normal spacing, positive values spread the letters out,
                                negative values make them closer together.
    */
    void setExtraKerningFactor (float extraKerning);

    /** @see setAscentOverride() */
    std::optional<float> getAscentOverride() const noexcept;

    /** This is designed to mirror CSS's ascent-override property.

        When the font size is specified in points (using setPointHeight(),
        FontOptions::withPointHeight(), etc.), then the font's ascent value in points will be equal
        to the font's size in points multiplied by the override value. That is, if the font size
        is 14pt and the ascent override is 0.5f, then the ascent will be 7pt.

        When the font size is *not* specified in points (using setHeight(),
        FontOptions::withHeight(), etc.), then the behaviour is more subtle.
        The ascent override still specifies the size of the font's ascender as a proportion of the
        font's em size.
        However, the point size of the font is now found by multiplying the JUCE height by the
        height-to-point factor, where this factor is equal to
        (1.0f / (ascent-in-em-units + descent-in-em-units)).
        As an example, if the JUCE font height is 14, the ascent override is 0.5f, and the
        descent override is 0.5f, then the font size will be 14pt and the ascent will be 7pt.
        Changing the ascent override to 1.0f and the descent override to 0.0f will preserve the
        font size of 14pt but give an ascender of 14pt and a descender of 0pt.
        Changing the ascent and descent overrides both to 1.0f will result in the
        font's size changing to 7pt with an ascent of 3.5pt.

        @see setDescentOverride()
    */
    void setAscentOverride (std::optional<float>);

    /** @see setDescentOverride() */
    std::optional<float> getDescentOverride() const noexcept;

    /** This is designed to mirror CSS's descent-override property.

        Specifies a value to replace the built-in typeface descent metric.
        The final descent value will be found by multiplying the provided value by the font
        size. You may also pass std::nullopt to use the descent value specified in the typeface.

        The documentation for setAscentOverride() includes a more thorough discussion
        of the mechanism used for overriding.

        @see setAscentOverride()
    */
    void setDescentOverride (std::optional<float>);

    //==============================================================================
    /** Changes all the font's characteristics with one call. */
    void setSizeAndStyle (float newHeight,
                          int newStyleFlags,
                          float newHorizontalScale,
                          float newKerningAmount);

    /** Changes all the font's characteristics with one call. */
    void setSizeAndStyle (float newHeight,
                          const String& newStyle,
                          float newHorizontalScale,
                          float newKerningAmount);

    //==============================================================================
    /** Returns the total width of a string as it would be drawn using this font.
        For a more accurate floating-point result, use getStringWidthFloat().

        This function does not take font fallback into account. If this font doesn't
        include glyphs to represent all characters in the string, then the width
        will be computed as though those characters were replaced with the "glyph not
        found" character.

        If you are trying to find the amount of space required to display a given string,
        you'll get more accurate results by actually measuring the results of whichever
        text layout engine (e.g. GlyphArrangement, TextLayout) you'll use when displaying
        the string.

        @see TextLayout::getStringWidth(), GlyphArrangement::getStringWidthInt()
    */
    [[deprecated ("Use GlyphArrangement or TextLayout to compute text layouts")]]
    int getStringWidth (const String& text) const;

    /** Returns the total width of a string as it would be drawn using this font.
        @see getStringWidth

        This function does not take font fallback into account. If this font doesn't
        include glyphs to represent all characters in the string, then the width
        will be computed as though those characters were replaced with the "glyph not
        found" character.

        If you are trying to find the amount of space required to display a given string,
        you'll get more accurate results by actually measuring the results of whichever
        text layout engine (e.g. GlyphArrangement, TextLayout) you'll use when displaying
        the string.

        @see TextLayout::getStringWidth(), GlyphArrangement::getStringWidth()
    */
    [[deprecated ("Use GlyphArrangement or TextLayout to compute text layouts")]]
    float getStringWidthFloat (const String& text) const;

    //==============================================================================
    /** Returns the main typeface used by this font.

        Note: This will only ever return the typeface for the "main" family.
        Before attempting to render glyphs from this typeface, it's a good idea to check
        that those glyphs are present in the typeface, and to select a different
        face if necessary.
    */
    Typeface::Ptr getTypefacePtr() const;

    /** Creates an array of Font objects to represent all the fonts on the system.

        If you just need the font family names of the typefaces, you can also use
        findAllTypefaceNames() instead.

        @param results  the array to which new Font objects will be added.
    */
    static void findFonts (Array<Font>& results);

    /** Returns a list of all the available typeface font families.

        The names returned can be passed into setTypefaceName().

        You can use this instead of findFonts() if you only need their font family names,
        and not font objects.
    */
    static StringArray findAllTypefaceNames();

    /** Returns a list of all the available typeface font styles.

        The names returned can be passed into setTypefaceStyle().

        You can use this instead of findFonts() if you only need their styles, and not
        font objects.
    */
    static StringArray findAllTypefaceStyles (const String& family);

    //==============================================================================
    /** Attempts to locate a visually similar font that is capable of rendering the
        provided string.

        If fallback is disabled on this Font by setFallbackEnabled(), then this will
        always return a copy of the current Font.

        Otherwise, the current font, then each of the fallback fonts specified by
        setPreferredFallbackFamilies() will be checked, and the first Font that is
        capable of rendering the string will be returned. If none of these fonts is
        suitable, then the system font fallback mechanism will be used to locate a
        font from the currently installed fonts. If the system also cannot find any
        suitable font, then a copy of the original Font will be returned.

        Note that most fonts don't contain glyphs for all possible unicode codepoints,
        and instead may contain e.g. just the glyphs required for a specific script. So,
        if the provided text would be displayed using several scripts (multiple languages,
        emoji, etc.) then there's a good chance that no single font will be able to
        render the entire text. Shorter strings will generally produce better fallback
        results than longer strings, with the caveat that the system may take control
        characters such as combining marks and variation selectors into account when
        selecting suitable fonts, so querying fallbacks character-by-character is likely
        to produce poor results.
    */
    Font findSuitableFontForText (const String& text, const String& language = {}) const;

    //==============================================================================
    /** Creates a string to describe this font.
        The string will contain information to describe the font's typeface, size, and
        style. To recreate the font from this string, use fromString().
    */
    String toString() const;

    /** Recreates a font from its stringified encoding.
        This method takes a string that was created by toString(), and recreates the
        original font.
    */
    static Font fromString (const String& fontDescription);

    /** @internal */
    class Native;

    /** @internal

        At the moment, this is a way to get at the hb_font_t that backs this font.
        The typeface's hb_font_t is sized appropriately for this font instance.
        The font may also have synthetic slant and bold applied.
        This is only for internal use!
    */
    Native getNativeDetails() const;

private:
    //==============================================================================
    static bool compare (const Font&, const Font&) noexcept;

    void dupeInternalIfShared();
    float getHeightToPointsFactor() const;

    friend struct GraphicsFontHelpers;

    class SharedFontInternal;
    ReferenceCountedObjectPtr<SharedFontInternal> font;

    JUCE_LEAK_DETECTOR (Font)
};

} // namespace juce
