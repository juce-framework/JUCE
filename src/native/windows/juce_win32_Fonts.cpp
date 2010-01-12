/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
static int CALLBACK wfontEnum2 (ENUMLOGFONTEXW* lpelfe,
                                NEWTEXTMETRICEXW*,
                                int type,
                                LPARAM lParam)
{
    if (lpelfe != 0 && (type & RASTER_FONTTYPE) == 0)
    {
        const String fontName (lpelfe->elfLogFont.lfFaceName);

        ((StringArray*) lParam)->addIfNotAlreadyThere (fontName.removeCharacters (T("@")));
    }

    return 1;
}

static int CALLBACK wfontEnum1 (ENUMLOGFONTEXW* lpelfe,
                                NEWTEXTMETRICEXW*,
                                int type,
                                LPARAM lParam)
{
    if (lpelfe != 0 && (type & RASTER_FONTTYPE) == 0)
    {
        LOGFONTW lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;

        const String fontName (lpelfe->elfLogFont.lfFaceName);
        fontName.copyToBuffer (lf.lfFaceName, LF_FACESIZE - 1);

        HDC dc = CreateCompatibleDC (0);
        EnumFontFamiliesEx (dc, &lf,
                            (FONTENUMPROCW) &wfontEnum2,
                            lParam, 0);
        DeleteDC (dc);
    }

    return 1;
}

const StringArray Font::findAllTypefaceNames() throw()
{
    StringArray results;
    HDC dc = CreateCompatibleDC (0);

    {
        LOGFONTW lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;
        lf.lfFaceName[0] = 0;

        EnumFontFamiliesEx (dc, &lf,
                            (FONTENUMPROCW) &wfontEnum1,
                            (LPARAM) &results, 0);
    }

    DeleteDC (dc);

    results.sort (true);
    return results;
}

extern bool juce_IsRunningInWine();

void Font::getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed) throw()
{
    if (juce_IsRunningInWine())
    {
        // If we're running in Wine, then use fonts that might be available on Linux..
        defaultSans  = "Bitstream Vera Sans";
        defaultSerif = "Bitstream Vera Serif";
        defaultFixed = "Bitstream Vera Sans Mono";
    }
    else
    {
        defaultSans  = "Verdana";
        defaultSerif = "Times";
        defaultFixed = "Lucida Console";
    }
}


//==============================================================================
class FontDCHolder  : private DeletedAtShutdown
{
public:
    //==============================================================================
    FontDCHolder() throw()
        : dc (0), numKPs (0), size (0),
          bold (false), italic (false)
    {
    }

    ~FontDCHolder() throw()
    {
        if (dc != 0)
        {
            DeleteDC (dc);
            DeleteObject (fontH);
        }

        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FontDCHolder);

    //==============================================================================
    HDC loadFont (const String& fontName_, const bool bold_, const bool italic_, const int size_) throw()
    {
        if (fontName != fontName_ || bold != bold_ || italic != italic_ || size != size_)
        {
            fontName = fontName_;
            bold = bold_;
            italic = italic_;
            size = size_;

            if (dc != 0)
            {
                DeleteDC (dc);
                DeleteObject (fontH);
                kps.free();
            }

            fontH = 0;

            dc = CreateCompatibleDC (0);
            SetMapperFlags (dc, 0);
            SetMapMode (dc, MM_TEXT);

            LOGFONTW lfw;
            zerostruct (lfw);

            lfw.lfCharSet = DEFAULT_CHARSET;
            lfw.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lfw.lfOutPrecision = OUT_OUTLINE_PRECIS;
            lfw.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
            lfw.lfQuality = PROOF_QUALITY;
            lfw.lfItalic = (BYTE) (italic ? TRUE : FALSE);
            lfw.lfWeight = bold ? FW_BOLD : FW_NORMAL;
            fontName.copyToBuffer (lfw.lfFaceName, LF_FACESIZE - 1);

            lfw.lfHeight = size > 0 ? size : -256;
            HFONT standardSizedFont = CreateFontIndirect (&lfw);

            if (standardSizedFont != 0)
            {
                if (SelectObject (dc, standardSizedFont) != 0)
                {
                    fontH = standardSizedFont;

                    if (size == 0)
                    {
                        OUTLINETEXTMETRIC otm;
                        if (GetOutlineTextMetrics (dc, sizeof (otm), &otm) != 0)
                        {
                            lfw.lfHeight = -(int) otm.otmEMSquare;
                            fontH = CreateFontIndirect (&lfw);

                            SelectObject (dc, fontH);
                            DeleteObject (standardSizedFont);
                        }
                    }
                }
                else
                {
                    jassertfalse
                }
            }
            else
            {
                jassertfalse
            }
        }

        return dc;
    }

    //==============================================================================
    KERNINGPAIR* getKerningPairs (int& numKPs_) throw()
    {
        if (kps == 0)
        {
            numKPs = GetKerningPairs (dc, 0, 0);
            kps.calloc (numKPs);
            GetKerningPairs (dc, numKPs, kps);
        }

        numKPs_ = numKPs;
        return kps;
    }


private:
    //==============================================================================
    HFONT fontH;
    HDC dc;
    String fontName;
    HeapBlock <KERNINGPAIR> kps;
    int numKPs, size;
    bool bold, italic;

    FontDCHolder (const FontDCHolder&);
    const FontDCHolder& operator= (const FontDCHolder&);
};

juce_ImplementSingleton_SingleThreaded (FontDCHolder);


//==============================================================================
class WindowsTypeface   : public CustomTypeface
{
public:
    WindowsTypeface (const Font& font)
    {
        HDC dc = FontDCHolder::getInstance()->loadFont (font.getTypefaceName(),
                                                        font.isBold(), font.isItalic(), 0);

        TEXTMETRIC tm;
        tm.tmAscent = tm.tmHeight = 1;
        tm.tmDefaultChar = 0;
        GetTextMetrics (dc, &tm);

        setCharacteristics (font.getTypefaceName(),
                            tm.tmAscent / (float) tm.tmHeight,
                            font.isBold(), font.isItalic(),
                            tm.tmDefaultChar);
    }

    bool loadGlyphIfPossible (const juce_wchar character)
    {
        HDC dc = FontDCHolder::getInstance()->loadFont (name, isBold, isItalic, 0);

        GLYPHMETRICS gm;

        {
            const WCHAR charToTest[] = { (WCHAR) character, 0 };
            WORD index = 0;

            if (GetGlyphIndices (dc, charToTest, 1, &index, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR
                 && index == 0xffff)
            {
                return false;
            }
        }

        Path glyphPath;

        TEXTMETRIC tm;
        if (! GetTextMetrics (dc, &tm))
        {
            addGlyph (character, glyphPath, 0);
            return true;
        }

        const float height = (float) tm.tmHeight;
        static const MAT2 identityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

        const int bufSize = GetGlyphOutline (dc, character, GGO_NATIVE,
                                             &gm, 0, 0, &identityMatrix);

        if (bufSize > 0)
        {
            HeapBlock <char> data (bufSize);

            GetGlyphOutline (dc, character, GGO_NATIVE, &gm,
                             bufSize, data, &identityMatrix);

            const TTPOLYGONHEADER* pheader = (TTPOLYGONHEADER*) data;

            const float scaleX = 1.0f / height;
            const float scaleY = -1.0f / height;

            while ((char*) pheader < data + bufSize)
            {
                #define remapX(v) (scaleX * (v).x.value)
                #define remapY(v) (scaleY * (v).y.value)

                float x = remapX (pheader->pfxStart);
                float y = remapY (pheader->pfxStart);

                glyphPath.startNewSubPath (x, y);

                const TTPOLYCURVE* curve = (const TTPOLYCURVE*) ((const char*) pheader + sizeof (TTPOLYGONHEADER));
                const char* const curveEnd = ((const char*) pheader) + pheader->cb;

                while ((const char*) curve < curveEnd)
                {
                    if (curve->wType == TT_PRIM_LINE)
                    {
                        for (int i = 0; i < curve->cpfx; ++i)
                        {
                            x = remapX (curve->apfx [i]);
                            y = remapY (curve->apfx [i]);

                            glyphPath.lineTo (x, y);
                        }
                    }
                    else if (curve->wType == TT_PRIM_QSPLINE)
                    {
                        for (int i = 0; i < curve->cpfx - 1; ++i)
                        {
                            const float x2 = remapX (curve->apfx [i]);
                            const float y2 = remapY (curve->apfx [i]);
                            float x3, y3;

                            if (i < curve->cpfx - 2)
                            {
                                x3 = 0.5f * (x2 + remapX (curve->apfx [i + 1]));
                                y3 = 0.5f * (y2 + remapY (curve->apfx [i + 1]));
                            }
                            else
                            {
                                x3 = remapX (curve->apfx [i + 1]);
                                y3 = remapY (curve->apfx [i + 1]);
                            }

                            glyphPath.quadraticTo (x2, y2, x3, y3);

                            x = x3;
                            y = y3;
                        }
                    }

                    curve = (const TTPOLYCURVE*) &(curve->apfx [curve->cpfx]);
                }

                pheader = (const TTPOLYGONHEADER*) curve;

                glyphPath.closeSubPath();
            }
        }

        addGlyph (character, glyphPath, gm.gmCellIncX / height);

        int numKPs;
        const KERNINGPAIR* const kps = FontDCHolder::getInstance()->getKerningPairs (numKPs);

        for (int i = 0; i < numKPs; ++i)
        {
            if (kps[i].wFirst == character)
                addKerningPair (kps[i].wFirst, kps[i].wSecond,
                                kps[i].iKernAmount / height);
        }

        return true;
    }
};

const Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new WindowsTypeface (font);
}


#endif
