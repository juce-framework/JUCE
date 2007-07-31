/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_FONT_JUCEHEADER__
#define __JUCE_FONT_JUCEHEADER__

#include "juce_Typeface.h"
#include "../geometry/juce_Path.h"
#include "../../../../juce_core/text/juce_StringArray.h"
#include "../../../../juce_core/containers/juce_ReferenceCountedObject.h"
#include "../../../../juce_core/containers/juce_OwnedArray.h"


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
        @see FontStyleFlags, getDefaultSansSerifFontName, setDefaultSansSerifFontName
    */
    Font (const float fontHeight,
          const int styleFlags = plain) throw();

    /** Creates a font with a given typeface and parameters.

        @param typefaceName the name of the typeface to use
        @param fontHeight   the height in pixels (can be fractional)
        @param styleFlags   the style to use - this can be a combination of the
                            Font::bold, Font::italic and Font::underlined, or
                            just Font::plain for the normal style.
        @see FontStyleFlags, getDefaultSansSerifFontName, setDefaultSansSerifFontName
    */
    Font (const String& typefaceName,
          const float fontHeight,
          const int styleFlags) throw();

    /** Creates a copy of another Font object. */
    Font (const Font& other) throw();

    /** Creates a font based on a typeface.

        The font object stores its own internal copy of the typeface, so you can safely
        delete the one passed in after calling this.
    */
    Font (const Typeface& typeface) throw();

    /** Creates a basic sans-serif font at a default height.

        You should use one of the other constructors for creating a font that you're planning
        on drawing with - this constructor is here to help initialise objects before changing
        the font's settings later.
    */
    Font() throw();

    /** Copies this font from another one. */
    const Font& operator= (const Font& other) throw();

    bool operator== (const Font& other) const throw();
    bool operator!= (const Font& other) const throw();

    /** Destructor. */
    ~Font() throw();

    //==============================================================================
    /** Changes the name of the typeface family.

        e.g. "Arial", "Courier", etc.

        If a suitable font isn't found on the machine, it'll just use a default instead.
    */
    void setTypefaceName (const String& faceName) throw();

    /** Returns the name of the typeface family that this font uses.

        e.g. "Arial", "Courier", etc.
    */
    const String& getTypefaceName() const throw()               { return typefaceName; }

    /** Returns a platform-specific font family that is recommended for sans-serif fonts.

        This is the typeface that will be used when a font is created without
        specifying another name.

        @see setTypefaceName, getDefaultSerifFontName, getDefaultMonospacedFontName,
             setDefaultSansSerifFontName
    */
    static const String getDefaultSansSerifFontName() throw();

    /** Returns a platform-specific font family that is recommended for serif fonts.

        @see setTypefaceName, getDefaultSansSerifFontName, getDefaultMonospacedFontName
    */
    static const String getDefaultSerifFontName() throw();

    /** Returns a platform-specific font family that is recommended for monospaced fonts.

        @see setTypefaceName, getDefaultSansSerifFontName, getDefaultSerifFontName
    */
    static const String getDefaultMonospacedFontName() throw();

    /** Changes the default sans-serif typeface family name.

        This changes the value that is returned by getDefaultSansSerifFontName(), so
        changing this will change the default system font used.

        @see getDefaultSansSerifFontName
    */
    static void setDefaultSansSerifFontName (const String& name) throw();

    //==============================================================================
    /** Returns the total height of this font.

        This is the maximum height, from the top of the ascent to the bottom of the
        descenders.

        @see setHeight, setHeightWithoutChangingWidth, getAscent
    */
    float getHeight() const throw()                             { return height; }

    /** Changes the font's height.

        @see getHeight, setHeightWithoutChangingWidth
    */
    void setHeight (float newHeight) throw();

    /** Changes the font's height without changing its width.

        This alters the horizontal scale to compensate for the change in height.
    */
    void setHeightWithoutChangingWidth (float newHeight) throw();

    /** Returns the height of the font above its baseline.

        This is the maximum height from the baseline to the top.

        @see getHeight, getDescent
    */
    float getAscent() const throw();

    /** Returns the amount that the font descends below its baseline.

        This is calculated as (getHeight() - getAscent()).

        @see getAscent, getHeight
    */
    float getDescent() const throw();

    //==============================================================================
    /** Returns the font's style flags.

        This will return a bitwise-or'ed combination of values from the FontStyleFlags
        enum, to describe whether the font is bold, italic, etc.

        @see FontStyleFlags
    */
    int getStyleFlags() const throw()                           { return styleFlags; }

    /** Changes the font's style.

        @param newFlags     a bitwise-or'ed combination of values from the FontStyleFlags
                            enum, to set the font's properties
        @see FontStyleFlags
    */
    void setStyleFlags (const int newFlags) throw();

    //==============================================================================
    /** Makes the font bold or non-bold. */
    void setBold (const bool shouldBeBold) throw();
    /** Returns true if the font is bold. */
    bool isBold() const throw();

    /** Makes the font italic or non-italic. */
    void setItalic (const bool shouldBeItalic) throw();
    /** Returns true if the font is italic. */
    bool isItalic() const throw();

    /** Makes the font underlined or non-underlined. */
    void setUnderline (const bool shouldBeUnderlined) throw();
    /** Returns true if the font is underlined. */
    bool isUnderlined() const throw();

    //==============================================================================
    /** Changes the font's horizontal scale factor.

        @param scaleFactor  a value of 1.0 is the normal scale, less than this will be
                            narrower, greater than 1.0 will be stretched out.
    */
    void setHorizontalScale (const float scaleFactor) throw();

    /** Returns the font's horizontal scale.

        A value of 1.0 is the normal scale, less than this will be narrower, greater
        than 1.0 will be stretched out.

        @see setHorizontalScale
    */
    float getHorizontalScale() const throw()                { return horizontalScale; }

    /** Changes the font's kerning.

        @param extraKerning     a multiple of the font's height that will be added
                                to space between the characters. So a value of zero is
                                normal spacing, positive values spread the letters out,
                                negative values make them closer together.
    */
    void setExtraKerningFactor (const float extraKerning) throw();

    /** Returns the font's kerning.

        This is the extra space added between adjacent characters, as a proportion
        of the font's height.

        A value of zero is normal spacing, positive values will spread the letters
        out more, and negative values make them closer together.
    */
    float getExtraKerningFactor() const throw()             { return kerning; }


    //==============================================================================
    /** Changes all the font's characteristics with one call. */
    void setSizeAndStyle (const float newHeight,
                          const int newStyleFlags,
                          const float newHorizontalScale,
                          const float newKerningAmount) throw();

    /** Resets this font's characteristics.

        This is basically like saying "myFont = Font();", because it resets the
        typeface, size, style, etc to a default state. Not very useful to most
        people, its raison d'etre is to help the Graphics class be more efficient.
    */
    void resetToDefaultState() throw();

    //==============================================================================
    /** Returns the total width of a string as it would be drawn using this font.

        For a more accurate floating-point result, use getStringWidthFloat().
    */
    int getStringWidth (const String& text) const throw();

    /** Returns the total width of a string as it would be drawn using this font.

        @see getStringWidth
    */
    float getStringWidthFloat (const String& text) const throw();

    //==============================================================================
    /** Returns the typeface used by this font.

        Note that the object returned may go out of scope if this font is deleted
        or has its style changed.
    */
    Typeface* getTypeface() const throw();

    /** Creates an array of Font objects to represent all the fonts on the system.

        If you just need the names of the typefaces, you can also use
        findAllTypefaceNames() instead.

        @param results  the array to which new Font objects will be added.
    */
    static void findFonts (OwnedArray<Font>& results) throw();

    /** Returns a list of all the available typeface names.

        The names returned can be passed into setTypefaceName().

        You can use this instead of findFonts() if you only need their names, and not
        font objects.
    */
    static const StringArray findAllTypefaceNames() throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    friend class FontGlyphAlphaMap;
    friend class TypefaceCache;

    String typefaceName;
    float height, horizontalScale, kerning;
    mutable float ascent;
    int styleFlags;
    mutable Typeface::Ptr typeface;

    // platform-specific calls
    static void getDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed) throw();

    friend void JUCE_PUBLIC_FUNCTION initialiseJuce_GUI();
    static void initialiseDefaultFontNames() throw();
};

#endif   // __JUCE_FONT_JUCEHEADER__
