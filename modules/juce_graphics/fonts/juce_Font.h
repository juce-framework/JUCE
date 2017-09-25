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
    Represents a particular font, including its size, style, etc.

    Apart from the typeface to be used, a Font object also dictates whether
    the font is bold, italic, underlined, how big it is, and its kerning and
    horizontal scale factor.

    @see Typeface
*/
class JUCE_API  Font
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

    //==============================================================================
    /** Creates a sans-serif font in a given size.

        @param fontHeight   the height in pixels (can be fractional)
        @param styleFlags   the style to use - this can be a combination of the
                            Font::bold, Font::italic and Font::underlined, or
                            just Font::plain for the normal style.
        @see FontStyleFlags, getDefaultSansSerifFontName
    */
    Font (float fontHeight, int styleFlags = plain);

    /** Creates a font with a given typeface and parameters.

        @param typefaceName the font family of the typeface to use
        @param fontHeight   the height in pixels (can be fractional)
        @param styleFlags   the style to use - this can be a combination of the
                            Font::bold, Font::italic and Font::underlined, or
                            just Font::plain for the normal style.
        @see FontStyleFlags, getDefaultSansSerifFontName
    */
    Font (const String& typefaceName, float fontHeight, int styleFlags);

    /** Creates a font with a given typeface and parameters.

        @param typefaceName  the font family of the typeface to use
        @param typefaceStyle the font style of the typeface to use
        @param fontHeight    the height in pixels (can be fractional)
    */
    Font (const String& typefaceName, const String& typefaceStyle, float fontHeight);

    /** Creates a copy of another Font object. */
    Font (const Font& other) noexcept;

    /** Creates a font for a typeface. */
    Font (const Typeface::Ptr& typeface);

    /** Creates a basic sans-serif font at a default height.

        You should use one of the other constructors for creating a font that you're planning
        on drawing with - this constructor is here to help initialise objects before changing
        the font's settings later.
    */
    Font();

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
        Font::getTypeface()->getName(), which will give you the platform-specific font family.

        If a suitable font isn't found on the machine, it'll just use a default instead.
    */
    void setTypefaceName (const String& faceName);

    /** Returns the font family of the typeface that this font uses.

        e.g. "Arial", "Courier", etc.

        This may also be set to Font::getDefaultSansSerifFontName(), Font::getDefaultSerifFontName(),
        or Font::getDefaultMonospacedFontName(), which are not actual platform-specific font family names,
        but are generic font familiy names that are used to represent the various default fonts.

        If you need to know the exact typeface font family being used, you can call
        Font::getTypeface()->getName(), which will give you the platform-specific font family.
    */
    const String& getTypefaceName() const noexcept;

    //==============================================================================
    /** Returns the font style of the typeface that this font uses.
        @see withTypefaceStyle, getAvailableStyles()
    */
    const String& getTypefaceStyle() const noexcept;

    /** Changes the font style of the typeface.
        @see getAvailableStyles()
    */
    void setTypefaceStyle (const String& newStyle);

    /** Returns a copy of this font with a new typeface style.
        @see getAvailableStyles()
    */
    Font withTypefaceStyle (const String& newStyle) const;

    /** Returns a list of the styles that this font can use. */
    StringArray getAvailableStyles() const;

    //==============================================================================
    /** Returns a typeface font family that represents the default sans-serif font.

        This is also the typeface that will be used when a font is created without
        specifying any typeface details.

        Note that this method just returns a generic placeholder string that means "the default
        sans-serif font" - it's not the actual font family of this font.

        @see setTypefaceName, getDefaultSerifFontName, getDefaultMonospacedFontName
    */
    static const String& getDefaultSansSerifFontName();

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

    /** Returns the default system typeface for the given font. */
    static Typeface::Ptr getDefaultTypefaceForFont (const Font& font);

    //==============================================================================
    /** Returns a copy of this font with a new height. */
    Font withHeight (float height) const;

    /** Returns a copy of this font with a new height, specified in points. */
    Font withPointHeight (float heightInPoints) const;

    /** Changes the font's height.
        @see getHeight, withHeight, setHeightWithoutChangingWidth
    */
    void setHeight (float newHeight);

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
    Font withStyle (int styleFlags) const;

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
    Font boldened() const;

    /** Returns true if the font is bold. */
    bool isBold() const noexcept;

    /** Makes the font italic or non-italic. */
    void setItalic (bool shouldBeItalic);
    /** Returns a copy of this font with the italic attribute set. */
    Font italicised() const;
    /** Returns true if the font is italic. */
    bool isItalic() const noexcept;

    /** Makes the font underlined or non-underlined. */
    void setUnderline (bool shouldBeUnderlined);
    /** Returns true if the font is underlined. */
    bool isUnderlined() const noexcept;


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
    Font withHorizontalScale (float scaleFactor) const;

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

    /** Returns the font's kerning.

        This is the extra space added between adjacent characters, as a proportion
        of the font's height.

        A value of zero is normal spacing, positive values will spread the letters
        out more, and negative values make them closer together.
    */
    float getExtraKerningFactor() const noexcept;

    /** Returns a copy of this font with a new kerning factor.
        @param extraKerning     a multiple of the font's height that will be added
                                to space between the characters. So a value of zero is
                                normal spacing, positive values spread the letters out,
                                negative values make them closer together.
    */
    Font withExtraKerningFactor (float extraKerning) const;

    /** Changes the font's kerning.
        @param extraKerning     a multiple of the font's height that will be added
                                to space between the characters. So a value of zero is
                                normal spacing, positive values spread the letters out,
                                negative values make them closer together.
    */
    void setExtraKerningFactor (float extraKerning);

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
    */
    int getStringWidth (const String& text) const;

    /** Returns the total width of a string as it would be drawn using this font.
        @see getStringWidth
    */
    float getStringWidthFloat (const String& text) const;

    /** Returns the series of glyph numbers and their x offsets needed to represent a string.

        An extra x offset is added at the end of the run, to indicate where the right hand
        edge of the last character is.
    */
    void getGlyphPositions (const String& text, Array <int>& glyphs, Array <float>& xOffsets) const;

    //==============================================================================
    /** Returns the typeface used by this font.

        Note that the object returned may go out of scope if this font is deleted
        or has its style changed.
    */
    Typeface* getTypeface() const;

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
    /** Returns the font family of the typeface to be used for rendering glyphs that aren't
        found in the requested typeface.
    */
    static const String& getFallbackFontName();

    /** Sets the (platform-specific) font family of the typeface to use to find glyphs that
        aren't available in whatever font you're trying to use.
    */
    static void setFallbackFontName (const String& name);

    /** Returns the font style of the typeface to be used for rendering glyphs that aren't
        found in the requested typeface.
    */
    static const String& getFallbackFontStyle();

    /** Sets the (platform-specific) font style of the typeface to use to find glyphs that
        aren't available in whatever font you're trying to use.
    */
    static void setFallbackFontStyle (const String& style);

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


private:
    //==============================================================================
    class SharedFontInternal;
    ReferenceCountedObjectPtr<SharedFontInternal> font;
    void dupeInternalIfShared();
    void checkTypefaceSuitability();
    float getHeightToPointsFactor() const;

    JUCE_LEAK_DETECTOR (Font)
};

} // namespace juce
