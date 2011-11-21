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

namespace FontEnumerators
{
    int CALLBACK fontEnum2 (ENUMLOGFONTEXW* lpelfe, NEWTEXTMETRICEXW*, int type, LPARAM lParam)
    {
        if (lpelfe != nullptr && (type & RASTER_FONTTYPE) == 0)
        {
            const String fontName (lpelfe->elfLogFont.lfFaceName);
            ((StringArray*) lParam)->addIfNotAlreadyThere (fontName.removeCharacters ("@"));
        }

        return 1;
    }

    int CALLBACK fontEnum1 (ENUMLOGFONTEXW* lpelfe, NEWTEXTMETRICEXW*, int type, LPARAM lParam)
    {
        if (lpelfe != nullptr && (type & RASTER_FONTTYPE) == 0)
        {
            LOGFONTW lf = { 0 };
            lf.lfWeight = FW_DONTCARE;
            lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
            lf.lfQuality = DEFAULT_QUALITY;
            lf.lfCharSet = DEFAULT_CHARSET;
            lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lf.lfPitchAndFamily = FF_DONTCARE;

            const String fontName (lpelfe->elfLogFont.lfFaceName);
            fontName.copyToUTF16 (lf.lfFaceName, sizeof (lf.lfFaceName));

            HDC dc = CreateCompatibleDC (0);
            EnumFontFamiliesEx (dc, &lf,
                                (FONTENUMPROCW) &fontEnum2,
                                lParam, 0);
            DeleteDC (dc);
        }

        return 1;
    }
}

StringArray Font::findAllTypefaceNames()
{
    StringArray results;
    HDC dc = CreateCompatibleDC (0);

    {
        LOGFONTW lf = { 0 };
        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;

        EnumFontFamiliesEx (dc, &lf,
                            (FONTENUMPROCW) &FontEnumerators::fontEnum1,
                            (LPARAM) &results, 0);
    }

    DeleteDC (dc);

    results.sort (true);
    return results;
}

extern bool juce_IsRunningInWine();

struct DefaultFontNames
{
    DefaultFontNames()
    {
        if (juce_IsRunningInWine())
        {
            // If we're running in Wine, then use fonts that might be available on Linux..
            defaultSans     = "Bitstream Vera Sans";
            defaultSerif    = "Bitstream Vera Serif";
            defaultFixed    = "Bitstream Vera Sans Mono";
        }
        else
        {
            defaultSans     = "Verdana";
            defaultSerif    = "Times";
            defaultFixed    = "Lucida Console";
            defaultFallback = "Tahoma";  // (contains plenty of unicode characters)
        }
    }

    String defaultSans, defaultSerif, defaultFixed, defaultFallback;
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    String faceName (font.getTypefaceName());

    if (faceName == Font::getDefaultSansSerifFontName())       faceName = defaultNames.defaultSans;
    else if (faceName == Font::getDefaultSerifFontName())      faceName = defaultNames.defaultSerif;
    else if (faceName == Font::getDefaultMonospacedFontName()) faceName = defaultNames.defaultFixed;

    Font f (font);
    f.setTypefaceName (faceName);
    return Typeface::createSystemTypefaceFor (f);
}

//==============================================================================
class WindowsTypeface   : public Typeface
{
public:
    WindowsTypeface (const Font& font)
        : Typeface (font.getTypefaceName()),
          fontH (0),
          previousFontH (0),
          dc (CreateCompatibleDC (0)),
          ascent (1.0f),
          defaultGlyph (-1),
          bold (font.isBold()),
          italic (font.isItalic())
    {
        loadFont();

        if (GetTextMetrics (dc, &tm))
        {
            ascent = tm.tmAscent / (float) tm.tmHeight;
            defaultGlyph = getGlyphForChar (dc, tm.tmDefaultChar);
            createKerningPairs (dc, (float) tm.tmHeight);
        }
    }

    ~WindowsTypeface()
    {
        SelectObject (dc, previousFontH); // Replacing the previous font before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (dc);

        if (fontH != 0)
            DeleteObject (fontH);
    }

    float getAscent() const     { return ascent; }
    float getDescent() const    { return 1.0f - ascent; }

    float getStringWidth (const String& text)
    {
        const CharPointer_UTF16 utf16 (text.toUTF16());
        const size_t numChars = utf16.length();
        HeapBlock<int16> results (numChars + 1);
        results[numChars] = -1;
        float x = 0;

        if (GetGlyphIndices (dc, utf16, (int) numChars, reinterpret_cast <WORD*> (results.getData()),
                             GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
        {
            for (size_t i = 0; i < numChars; ++i)
                x += getKerning (dc, results[i], results[i + 1]);
        }

        return x;
    }

    void getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array <float>& xOffsets)
    {
        const CharPointer_UTF16 utf16 (text.toUTF16());
        const size_t numChars = utf16.length();
        HeapBlock<int16> results (numChars + 1);
        results[numChars] = -1;
        float x = 0;

        if (GetGlyphIndices (dc, utf16, (int) numChars, reinterpret_cast <WORD*> (results.getData()),
                             GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
        {
            resultGlyphs.ensureStorageAllocated ((int) numChars);
            xOffsets.ensureStorageAllocated ((int) numChars + 1);

            for (size_t i = 0; i < numChars; ++i)
            {
                resultGlyphs.add (results[i]);
                xOffsets.add (x);
                x += getKerning (dc, results[i], results[i + 1]);
            }
        }

        xOffsets.add (x);
    }

    bool getOutlineForGlyph (int glyphNumber, Path& glyphPath)
    {
        if (glyphNumber < 0)
            glyphNumber = defaultGlyph;

        GLYPHMETRICS gm;
        // (although GetGlyphOutline returns a DWORD, it may be -1 on failure, so treat it as signed int..)
        const int bufSize = (int) GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX,
                                                   &gm, 0, 0, &identityMatrix);

        if (bufSize > 0)
        {
            HeapBlock<char> data (bufSize);
            GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX, &gm,
                             bufSize, data, &identityMatrix);

            const TTPOLYGONHEADER* pheader = reinterpret_cast<TTPOLYGONHEADER*> (data.getData());

            const float scaleX = 1.0f / tm.tmHeight;
            const float scaleY = -scaleX;

            while ((char*) pheader < data + bufSize)
            {
                glyphPath.startNewSubPath (scaleX * pheader->pfxStart.x.value,
                                           scaleY * pheader->pfxStart.y.value);

                const TTPOLYCURVE* curve = (const TTPOLYCURVE*) ((const char*) pheader + sizeof (TTPOLYGONHEADER));
                const char* const curveEnd = ((const char*) pheader) + pheader->cb;

                while ((const char*) curve < curveEnd)
                {
                    if (curve->wType == TT_PRIM_LINE)
                    {
                        for (int i = 0; i < curve->cpfx; ++i)
                            glyphPath.lineTo (scaleX * curve->apfx[i].x.value,
                                              scaleY * curve->apfx[i].y.value);
                    }
                    else if (curve->wType == TT_PRIM_QSPLINE)
                    {
                        for (int i = 0; i < curve->cpfx - 1; ++i)
                        {
                            const float x2 = scaleX * curve->apfx[i].x.value;
                            const float y2 = scaleY * curve->apfx[i].y.value;
                            float x3       = scaleX * curve->apfx[i + 1].x.value;
                            float y3       = scaleY * curve->apfx[i + 1].y.value;

                            if (i < curve->cpfx - 2)
                            {
                                x3 = 0.5f * (x2 + x3);
                                y3 = 0.5f * (y2 + y3);
                            }

                            glyphPath.quadraticTo (x2, y2, x3, y3);
                        }
                    }

                    curve = (const TTPOLYCURVE*) &(curve->apfx [curve->cpfx]);
                }

                pheader = (const TTPOLYGONHEADER*) curve;

                glyphPath.closeSubPath();
            }
        }

        return true;
    }

private:
    static const MAT2 identityMatrix;
    HFONT fontH;
    HGDIOBJ previousFontH;
    HDC dc;
    TEXTMETRIC tm;
    float ascent;
    int defaultGlyph;
    bool bold, italic;

    struct KerningPair
    {
        int glyph1, glyph2;
        float kerning;

        bool operator== (const KerningPair& other) const noexcept
        {
            return glyph1 == other.glyph1 && glyph2 == other.glyph2;
        }

        bool operator< (const KerningPair& other) const noexcept
        {
            return glyph1 < other.glyph1
                    || (glyph1 == other.glyph1 && glyph2 < other.glyph2);
        }
    };

    SortedSet<KerningPair> kerningPairs;

    void loadFont()
    {
        SetMapperFlags (dc, 0);
        SetMapMode (dc, MM_TEXT);

        LOGFONTW lf = { 0 };
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfItalic = (BYTE) (italic ? TRUE : FALSE);
        lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
        lf.lfHeight = -256;
        name.copyToUTF16 (lf.lfFaceName, sizeof (lf.lfFaceName));

        HFONT standardSizedFont = CreateFontIndirect (&lf);

        if (standardSizedFont != 0)
        {
            if ((previousFontH = SelectObject (dc, standardSizedFont)) != 0)
            {
                fontH = standardSizedFont;

                OUTLINETEXTMETRIC otm;
                if (GetOutlineTextMetrics (dc, sizeof (otm), &otm) != 0)
                {
                    lf.lfHeight = -(int) otm.otmEMSquare;
                    fontH = CreateFontIndirect (&lf);

                    SelectObject (dc, fontH);
                    DeleteObject (standardSizedFont);
                }
            }
        }
    }

    void createKerningPairs (HDC dc, const float height)
    {
        HeapBlock<KERNINGPAIR> rawKerning;
        const DWORD numKPs = GetKerningPairs (dc, 0, 0);
        rawKerning.calloc (numKPs);
        GetKerningPairs (dc, numKPs, rawKerning);

        kerningPairs.ensureStorageAllocated ((int) numKPs);

        for (DWORD i = 0; i < numKPs; ++i)
        {
            KerningPair kp;
            kp.glyph1 = getGlyphForChar (dc, rawKerning[i].wFirst);
            kp.glyph2 = getGlyphForChar (dc, rawKerning[i].wSecond);

            const int standardWidth = getGlyphWidth (dc, kp.glyph1);
            kp.kerning = (standardWidth + rawKerning[i].iKernAmount) / height;
            kerningPairs.add (kp);

            kp.glyph2 = -1;  // add another entry for the standard width version..
            kp.kerning = standardWidth / height;
            kerningPairs.add (kp);
        }
    }

    static int getGlyphForChar (HDC dc, juce_wchar character)
    {
        const WCHAR charToTest[] = { (WCHAR) character, 0 };
        WORD index = 0;

        if (GetGlyphIndices (dc, charToTest, 1, &index, GGI_MARK_NONEXISTING_GLYPHS) == GDI_ERROR
              || index == 0xffff)
            return -1;

        return index;
    }

    static int getGlyphWidth (HDC dc, int glyphNumber)
    {
        GLYPHMETRICS gm;
        gm.gmCellIncX = 0;
        GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX, &gm, 0, 0, &identityMatrix);
        return gm.gmCellIncX;
    }

    float getKerning (HDC dc, const int glyph1, const int glyph2)
    {
        KerningPair kp;
        kp.glyph1 = glyph1;
        kp.glyph2 = glyph2;
        int index = kerningPairs.indexOf (kp);

        if (index < 0)
        {
            kp.glyph2 = -1;
            index = kerningPairs.indexOf (kp);

            if (index < 0)
            {
                kp.glyph2 = -1;
                kp.kerning = getGlyphWidth (dc, kp.glyph1) / (float) tm.tmHeight;
                kerningPairs.add (kp);
                return kp.kerning;
            }
        }

        return kerningPairs.getReference (index).kerning;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTypeface);
};

const MAT2 WindowsTypeface::identityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
   #if JUCE_USE_DIRECTWRITE
    const Direct2DFactories& factories = Direct2DFactories::getInstance();

    if (factories.systemFonts != nullptr)
        return new WindowsDirectWriteTypeface (font, factories.systemFonts);
    else
   #endif
        return new WindowsTypeface (font);
}
