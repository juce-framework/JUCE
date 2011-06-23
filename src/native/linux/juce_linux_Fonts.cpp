/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
class FreeTypeFontFace
{
public:
    //==============================================================================
    enum FontStyle
    {
        Plain = 0,
        Bold = 1,
        Italic = 2
    };

    //==============================================================================
    FreeTypeFontFace (const String& familyName)
      : hasSerif (false),
        monospaced (false)
    {
        family = familyName;
    }

    void setFileName (const String& name, const int faceIndex, FontStyle style)
    {
        if (names [(int) style].fileName.isEmpty())
        {
            names [(int) style].fileName = name;
            names [(int) style].faceIndex = faceIndex;
        }
    }

    const String& getFamilyName() const noexcept    { return family; }

    const String& getFileName (const int style, int& faceIndex) const noexcept
    {
        faceIndex = names[style].faceIndex;
        return names[style].fileName;
    }

    void setMonospaced (bool mono) noexcept     { monospaced = mono; }
    bool getMonospaced() const noexcept         { return monospaced; }

    void setSerif (const bool serif) noexcept   { hasSerif = serif; }
    bool getSerif() const noexcept              { return hasSerif; }

private:
    //==============================================================================
    String family;

    struct FontNameIndex
    {
        String fileName;
        int faceIndex;
    };

    FontNameIndex names[4];
    bool hasSerif, monospaced;
};

//==============================================================================
class LinuxFontFileIterator
{
public:
    LinuxFontFileIterator()
        : index (0)
    {
        fontDirs.addTokens (CharPointer_UTF8 (getenv ("JUCE_FONT_PATH")), ";,", String::empty);
        fontDirs.removeEmptyStrings (true);

        if (fontDirs.size() == 0)
        {
            const ScopedPointer<XmlElement> fontsInfo (XmlDocument::parse (File ("/etc/fonts/fonts.conf")));

            if (fontsInfo != nullptr)
            {
                forEachXmlChildElementWithTagName (*fontsInfo, e, "dir")
                {
                    fontDirs.add (e->getAllSubText().trim());
                }
            }
        }

        if (fontDirs.size() == 0)
            fontDirs.add ("/usr/X11R6/lib/X11/fonts");

        fontDirs.removeEmptyStrings (true);
    }

    bool next()
    {
        if (iter != nullptr)
        {
            while (iter->next())
                if (getFile().hasFileExtension ("ttf;pfb;pcf"))
                    return true;
        }

        if (index >= fontDirs.size())
            return false;

        iter = new DirectoryIterator (fontDirs [index++], true);
        return next();
    }

    File getFile() const    { jassert (iter != nullptr); return iter->getFile(); }

private:
    StringArray fontDirs;
    int index;
    ScopedPointer<DirectoryIterator> iter;
};


//==============================================================================
class FreeTypeInterface  : public DeletedAtShutdown
{
public:
    //==============================================================================
    FreeTypeInterface()
        : ftLib (0),
          lastFace (0),
          lastBold (false),
          lastItalic (false)
    {
        if (FT_Init_FreeType (&ftLib) != 0)
        {
            ftLib = 0;
            DBG ("Failed to initialize FreeType");
        }

        LinuxFontFileIterator fontFileIterator;

        while (fontFileIterator.next())
        {
            FT_Face face;
            int faceIndex = 0;
            int numFaces = 0;

            do
            {
                if (FT_New_Face (ftLib, fontFileIterator.getFile().getFullPathName().toUTF8(),
                                 faceIndex, &face) == 0)
                {
                    if (faceIndex == 0)
                        numFaces = face->num_faces;

                    if ((face->face_flags & FT_FACE_FLAG_SCALABLE) != 0)
                    {
                        FreeTypeFontFace* const newFace = findOrCreate (face->family_name, true);
                        int style = (int) FreeTypeFontFace::Plain;

                        if ((face->style_flags & FT_STYLE_FLAG_BOLD) != 0)
                            style |= (int) FreeTypeFontFace::Bold;

                        if ((face->style_flags & FT_STYLE_FLAG_ITALIC) != 0)
                            style |= (int) FreeTypeFontFace::Italic;

                        newFace->setFileName (fontFileIterator.getFile().getFullPathName(),
                                              faceIndex, (FreeTypeFontFace::FontStyle) style);
                        newFace->setMonospaced ((face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) != 0);

                        // Surely there must be a better way to do this?
                        const String name (face->family_name);
                        newFace->setSerif (! (name.containsIgnoreCase ("Sans")
                                               || name.containsIgnoreCase ("Verdana")
                                               || name.containsIgnoreCase ("Arial")));

                        //DBG (fontFileIterator.getFile().getFullPathName() << "  -  " << name);
                    }

                    FT_Done_Face (face);
                }

                ++faceIndex;
            }
            while (faceIndex < numFaces);
        }
    }

    ~FreeTypeInterface()
    {
        if (lastFace != 0)
            FT_Done_Face (lastFace);

        if (ftLib != 0)
            FT_Done_FreeType (ftLib);

        clearSingletonInstance();
    }

    //==============================================================================
    FreeTypeFontFace* findOrCreate (const String& familyName, const bool create = false)
    {
        for (int i = 0; i < faces.size(); i++)
            if (faces[i]->getFamilyName() == familyName)
                return faces[i];

        if (! create)
            return nullptr;

        FreeTypeFontFace* newFace = new FreeTypeFontFace (familyName);
        faces.add (newFace);

        return newFace;
    }

    // Create a FreeType face object for a given font
    FT_Face createFT_Face (const String& fontName, const bool bold, const bool italic)
    {
        FT_Face face = 0;

        if (fontName == lastFontName && bold == lastBold && italic == lastItalic)
        {
            face = lastFace;
        }
        else
        {
            if (lastFace != 0)
            {
                FT_Done_Face (lastFace);
                lastFace = 0;
            }

            lastFontName = fontName;
            lastBold = bold;
            lastItalic = italic;

            FreeTypeFontFace* const ftFace = findOrCreate (fontName);

            if (ftFace != 0)
            {
                int style = (int) FreeTypeFontFace::Plain;

                if (bold)
                    style |= (int) FreeTypeFontFace::Bold;

                if (italic)
                    style |= (int) FreeTypeFontFace::Italic;

                int faceIndex;
                String fileName (ftFace->getFileName (style, faceIndex));

                if (fileName.isEmpty())
                {
                    style ^= (int) FreeTypeFontFace::Bold;

                    fileName = ftFace->getFileName (style, faceIndex);

                    if (fileName.isEmpty())
                    {
                        style ^= (int) FreeTypeFontFace::Bold;
                        style ^= (int) FreeTypeFontFace::Italic;

                        fileName = ftFace->getFileName (style, faceIndex);

                        if (! fileName.length())
                        {
                            style ^= (int) FreeTypeFontFace::Bold;
                            fileName = ftFace->getFileName (style, faceIndex);
                        }
                    }
                }

                if (! FT_New_Face (ftLib, fileName.toUTF8(), faceIndex, &lastFace))
                {
                    face = lastFace;

                    // If there isn't a unicode charmap then select the first one.
                    if (FT_Select_Charmap (face, ft_encoding_unicode))
                        FT_Set_Charmap (face, face->charmaps[0]);
                }
            }
        }

        return face;
    }

    bool addGlyph (FT_Face face, CustomTypeface& dest, uint32 character)
    {
        const unsigned int glyphIndex = FT_Get_Char_Index (face, character);
        const float height = (float) (face->ascender - face->descender);
        const float scaleX = 1.0f / height;
        const float scaleY = -1.0f / height;
        Path destShape;

        if (FT_Load_Glyph (face, glyphIndex, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM) != 0
             || face->glyph->format != ft_glyph_format_outline)
        {
            return false;
        }

        const FT_Outline* const outline = &face->glyph->outline;
        const short* const contours = outline->contours;
        const char* const tags = outline->tags;
        FT_Vector* const points = outline->points;

        for (int c = 0; c < outline->n_contours; c++)
        {
            const int startPoint = (c == 0) ? 0 : contours [c - 1] + 1;
            const int endPoint = contours[c];

            for (int p = startPoint; p <= endPoint; p++)
            {
                const float x = scaleX * points[p].x;
                const float y = scaleY * points[p].y;

                if (p == startPoint)
                {
                    if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Conic)
                    {
                        float x2 = scaleX * points [endPoint].x;
                        float y2 = scaleY * points [endPoint].y;

                        if (FT_CURVE_TAG (tags[endPoint]) != FT_Curve_Tag_On)
                        {
                            x2 = (x + x2) * 0.5f;
                            y2 = (y + y2) * 0.5f;
                        }

                        destShape.startNewSubPath (x2, y2);
                    }
                    else
                    {
                        destShape.startNewSubPath (x, y);
                    }
                }

                if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_On)
                {
                    if (p != startPoint)
                        destShape.lineTo (x, y);
                }
                else if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Conic)
                {
                    const int nextIndex = (p == endPoint) ? startPoint : p + 1;
                    float x2 = scaleX * points [nextIndex].x;
                    float y2 = scaleY * points [nextIndex].y;

                    if (FT_CURVE_TAG (tags [nextIndex]) == FT_Curve_Tag_Conic)
                    {
                        x2 = (x + x2) * 0.5f;
                        y2 = (y + y2) * 0.5f;
                    }
                    else
                    {
                        ++p;
                    }

                    destShape.quadraticTo (x, y, x2, y2);
                }
                else if (FT_CURVE_TAG (tags[p]) == FT_Curve_Tag_Cubic)
                {
                    if (p >= endPoint)
                        return false;

                    const int next1 = p + 1;
                    const int next2 = (p == (endPoint - 1)) ? startPoint : p + 2;

                    const float x2 = scaleX * points [next1].x;
                    const float y2 = scaleY * points [next1].y;
                    const float x3 = scaleX * points [next2].x;
                    const float y3 = scaleY * points [next2].y;

                    if (FT_CURVE_TAG (tags[next1]) != FT_Curve_Tag_Cubic
                         || FT_CURVE_TAG (tags[next2]) != FT_Curve_Tag_On)
                        return false;

                    destShape.cubicTo (x, y, x2, y2, x3, y3);
                    p += 2;
                }
            }

            destShape.closeSubPath();
        }

        dest.addGlyph (character, destShape, face->glyph->metrics.horiAdvance / height);

        if ((face->face_flags & FT_FACE_FLAG_KERNING) != 0)
            addKerning (face, dest, character, glyphIndex);

        return true;
    }

    void addKerning (FT_Face face, CustomTypeface& dest, const uint32 character, const uint32 glyphIndex)
    {
        const float height = (float) (face->ascender - face->descender);

        uint32 rightGlyphIndex;
        uint32 rightCharCode = FT_Get_First_Char (face, &rightGlyphIndex);

        while (rightGlyphIndex != 0)
        {
            FT_Vector kerning;

            if (FT_Get_Kerning (face, glyphIndex, rightGlyphIndex, ft_kerning_unscaled, &kerning) == 0)
            {
                if (kerning.x != 0)
                    dest.addKerningPair (character, rightCharCode, kerning.x / height);
            }

            rightCharCode = FT_Get_Next_Char (face, rightCharCode, &rightGlyphIndex);
        }
    }

    // Add a glyph to a font
    bool addGlyphToFont (const uint32 character, const String& fontName,
                         bool bold, bool italic, CustomTypeface& dest)
    {
        FT_Face face = createFT_Face (fontName, bold, italic);

        return face != 0 && addGlyph (face, dest, character);
    }

    //==============================================================================
    void getFamilyNames (StringArray& familyNames) const
    {
        for (int i = 0; i < faces.size(); i++)
            familyNames.add (faces[i]->getFamilyName());
    }

    void getMonospacedNames (StringArray& monoSpaced) const
    {
        for (int i = 0; i < faces.size(); i++)
            if (faces[i]->getMonospaced())
                monoSpaced.add (faces[i]->getFamilyName());
    }

    void getSerifNames (StringArray& serif) const
    {
        for (int i = 0; i < faces.size(); i++)
            if (faces[i]->getSerif())
                serif.add (faces[i]->getFamilyName());
    }

    void getSansSerifNames (StringArray& sansSerif) const
    {
        for (int i = 0; i < faces.size(); i++)
            if (! faces[i]->getSerif())
                sansSerif.add (faces[i]->getFamilyName());
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FreeTypeInterface);

private:
    //==============================================================================
    FT_Library ftLib;
    FT_Face lastFace;
    String lastFontName;
    bool lastBold, lastItalic;
    OwnedArray<FreeTypeFontFace> faces;
};

juce_ImplementSingleton_SingleThreaded (FreeTypeInterface)


//==============================================================================
class FreetypeTypeface   : public CustomTypeface
{
public:
    FreetypeTypeface (const Font& font)
    {
        FT_Face face = FreeTypeInterface::getInstance()
                            ->createFT_Face (font.getTypefaceName(), font.isBold(), font.isItalic());

        if (face == 0)
        {
           #if JUCE_DEBUG
            String msg ("Failed to create typeface: ");
            msg << font.getTypefaceName() << " " << (font.isBold() ? 'B' : ' ') << (font.isItalic() ? 'I' : ' ');
            DBG (msg);
           #endif
        }
        else
        {
            setCharacteristics (font.getTypefaceName(),
                                face->ascender / (float) (face->ascender - face->descender),
                                font.isBold(), font.isItalic(),
                                L' ');
        }
    }

    bool loadGlyphIfPossible (juce_wchar character)
    {
        return FreeTypeInterface::getInstance()
                    ->addGlyphToFont (character, name, isBold, isItalic, *this);
    }
};

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new FreetypeTypeface (font);
}

//==============================================================================
StringArray Font::findAllTypefaceNames()
{
    StringArray s;
    FreeTypeInterface::getInstance()->getFamilyNames (s);
    s.sort (true);
    return s;
}

namespace LinuxFontHelpers
{
    String pickBestFont (const StringArray& names,
                         const char* const* choicesString)
    {
        const StringArray choices (choicesString);

        int i, j;
        for (j = 0; j < choices.size(); ++j)
            if (names.contains (choices[j], true))
                return choices[j];

        for (j = 0; j < choices.size(); ++j)
            for (i = 0; i < names.size(); i++)
                if (names[i].startsWithIgnoreCase (choices[j]))
                    return names[i];

        for (j = 0; j < choices.size(); ++j)
            for (i = 0; i < names.size(); i++)
                if (names[i].containsIgnoreCase (choices[j]))
                    return names[i];

        return names[0];
    }

    String getDefaultSansSerifFontName()
    {
        StringArray allFonts;
        FreeTypeInterface::getInstance()->getSansSerifNames (allFonts);

        const char* targets[] = { "Verdana", "Bitstream Vera Sans", "Luxi Sans", "Sans", 0 };
        return pickBestFont (allFonts, targets);
    }

    String getDefaultSerifFontName()
    {
        StringArray allFonts;
        FreeTypeInterface::getInstance()->getSerifNames (allFonts);

        const char* targets[] = { "Bitstream Vera Serif", "Times", "Nimbus Roman", "Serif", 0 };
        return pickBestFont (allFonts, targets);
    }

    String getDefaultMonospacedFontName()
    {
        StringArray allFonts;
        FreeTypeInterface::getInstance()->getMonospacedNames (allFonts);

        const char* targets[] = { "Bitstream Vera Sans Mono", "Courier", "Sans Mono", "Mono", 0 };
        return pickBestFont (allFonts, targets);
    }
}

void Font::getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed, String& /*defaultFallback*/)
{
    defaultSans = LinuxFontHelpers::getDefaultSansSerifFontName();
    defaultSerif = LinuxFontHelpers::getDefaultSerifFontName();
    defaultFixed = LinuxFontHelpers::getDefaultMonospacedFontName();
}

#endif
