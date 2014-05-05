/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

#ifndef VF_FREETYPEFACES_VFHEADER
#define VF_FREETYPEFACES_VFHEADER

/*============================================================================*/
/**
    FreeType typefaces with font hinting.

    This singleton uses FreeType to parse a TrueType or OpenType font that has
    already been loaded into memory. The resulting TypeFace will support
    font-hinting (adjustments to make glyphs appear crisp at small sizes).

    An easy way to get the font into memory is to directly embed a font into
    your application using the Juce BinaryBuilder. This will ensure that the
    font is always available , giving your program a consistent appearance.

    To use the class, register each font you wish to make available for use
    by calling addFaceFromMemory().  During the call to register you can
    specify what range of font heights you want to apply font hinting to.
    Since each height will count as a separate physical typeface, with its
    own set of cached glyph images, it is best to keep the range small. Usually
    hinting is only needed for small sizes. For example, 14 pixels or less.

    JUCE maintains its own internal storage for caching typefaces. Depending
    on the number of distinct hinted sizes and fonts, you may need to
    increase the size of the typeface cache by calling
    Typeface::setTypefaceCacheSize.

    Most of the time it is useful to change the default typeface used for the
    entire application or plugin, to a hinted font to improve the appearance.
    This example shows how to create a custom LookAndFeel to achieve this:

    @code

    // This is the .ttf file data for Helvetica Neue converted with BinaryBuilder.

    extern char* helveticaneueltcommd_ttf;
    extern int   helveticaneueltcommd_ttfSize;

    class CustomLookAndFeel : public LookAndFeel
    {
    public:
      CustomLookAndFeel()
      {
        // Add the TrueType font "Helvetica Neue LT Com 65 Medium" and
        // use hinting when the font height is between 7 and 12 inclusive.

        FreeTypeFaces::getInstance()->addFaceFromMemory(
          7.f, 12.f,
          helveticaneueltcommd_ttf,
          helveticaneueltcommd_ttfSize);
      }

      const Typeface::Ptr CustomLookAndFeel::getTypefaceForFont (Font const& font)
      {
        Typeface::Ptr tf;

        String faceName (font.getTypefaceName());

        // Make requests for the default sans serif font use our
        // FreeType hinted font instead.

        if (faceName == Font::getDefaultSansSerifFontName())
        {
          // Create a new Font identical to the old one, then
          // switch the name to our hinted font.

          Font f (font);

          // You'll need to know the exact name embedded in the font. There
          // are a variety of free programs for retrieving this information.

          f.setTypefaceName ("Helvetica Neue LT Com 65 Medium");

          // Now get the hinted typeface.

          tf = FreeTypeFaces::createTypefaceForFont (f);
        }

        // If we got here without creating a new typeface
        // then just use the default LookAndFeel behavior.

        if (!tf)
          tf = LookAndFeel::getTypefaceForFont (font);

        return tf;
      }
    };

    @endcode

    @note Multiple typefaces within a single font file are not supported - only
    the first font will be used. A workaround is to break the font file up into
    multiple individual files using available third party tools before running
    BinaryBuilder.

    @ingroup vf_gui
*/
class FreeTypeFaces
{
public:
  /** Add a font to the list of available fonts.

      This parses a font stored in a block of memory, and adds it to the global
      list. After the call returns, the typeface can be found by calling
      createTypefaceForFont() with a matching name.

      If appendStyleToFaceName is true, then the style name found in the font
      is appended to the name of the typeface found in the font to produce the
      final typeface name.

      For example, given a font file containing the typeface
      "Helvetica Neue LT Com" with style "65 Medium", passing true for
      appendStyleToFaceName will result in the typeface having the name
      "Helvetica Neue LT Com 65 Medium". This means that multiple styles will
      be treated as completely separate typefaces: you will need to manually
      specify the full combined name in
      order to access the font.

      Setting appendStyleToFaceName to false is useful when you have variations
      from the same family and want to make use of the style flags in the Font
      structure, such as bold or italics.

      For example, if you have two font files for the family
      "Helvetica Neue LT Com", with styles "65 Medium" and "75 Bold", you
      can call addFaceFromMemory() twice to load them and pass
      appendStyleToFaceName as false. When a request for "Helvetica Neue LT Com"
      is made, the bold flag in the Font object will be checked and the
      appropriate styled typeface returned.

      @note JUCE support for styles may not always work as expected, depending
            on what's in your font file. If the bold or italic styles in the
            Font object are not applied correctly, you may need to manually
            select the appropriate typeface by setting appendStyleToFaceName to
            true when adding the typeface.

      There is currently no JUCE interface for direct specification of the
      font weight. If you have a family with multiple weights you will need
      to treat each weight as its own typeface.

      @param minHintedHeight The minimum font height when checking whether a
                             Font should have hinting activated.

      @param maxHintedHeight The maximum font height when checking whether a
                             Font should have hinting activated.

      @param useFreeTypeRendering This experimental feature is disabled. The
                                  parameter is ignored.

      @param faceFileData A pointer to the memory that holds the font data.

      @param faceFileBytes The number of bytes in the memory pointed to by
                           faceFileData.

      @param appendStyleToFaceName A boolean indicating whether or not the
                                   style name is appended to the family name
                                   to determine the typeface name at the time
                                   it is added.

                                   More.
  */
  static void addFaceFromMemory (float minHintedHeight,
                                 float maxHintedHeight,
                                 bool useFreeTypeRendering,
                                 const void* faceFileData,
                                 int faceFileBytes,
                                 bool appendStyleToFaceName = false);

  /** Create a hinted typeface to match a Font specification.

      The typeface name in the Font object must exactly match the name
      of the font when it was added in addFaceFromMemory().

      @param font A Font specifying the typeface. If the font height in
                  the object falls within the range of heights specified
                  in addFaceFromMemory(), the resulting Typeface will use
                  font hinting.

      @return The Typeface, or nullptr if there was no match.
  */
  static Typeface::Ptr createTypefaceForFont (const Font& font);
};

#endif
