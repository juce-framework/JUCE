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

#include "win32_headers.h"
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/gui/graphics/fonts/juce_Font.h"
#include "../../../src/juce_appframework/application/juce_DeletedAtShutdown.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/basics/juce_Singleton.h"
#include "../../../src/juce_appframework/gui/graphics/imaging/juce_Image.h"


//==============================================================================
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    UNICODE_FUNCTION (GetGlyphOutlineW, DWORD, (HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*))
    UNICODE_FUNCTION (GetTextMetricsW, BOOL, (HDC, LPTEXTMETRICW))
    UNICODE_FUNCTION (GetKerningPairsW, DWORD, (HDC, DWORD, LPKERNINGPAIR))
    UNICODE_FUNCTION (EnumFontFamiliesExW, int, (HDC, LPLOGFONTW, FONTENUMPROCW, LPARAM, DWORD))
    UNICODE_FUNCTION (CreateFontIndirectW, HFONT, (CONST LOGFONTW *));

    static void juce_initialiseUnicodeFileFontFunctions()
    {
        static bool initialised = false;

        if (! initialised)
        {
            initialised = true;

            if ((SystemStats::getOperatingSystemType() & SystemStats::WindowsNT) != 0)
            {
                HMODULE h = LoadLibraryA ("gdi32.dll");
                UNICODE_FUNCTION_LOAD (GetGlyphOutlineW)
                UNICODE_FUNCTION_LOAD (GetTextMetricsW)
                UNICODE_FUNCTION_LOAD (GetKerningPairsW)
                UNICODE_FUNCTION_LOAD (EnumFontFamiliesExW)
                UNICODE_FUNCTION_LOAD (CreateFontIndirectW)
            }
        }
    }
#endif


//==============================================================================
#if JUCE_ENABLE_WIN98_COMPATIBILITY
static int CALLBACK fontEnum2 (ENUMLOGFONTEX* lpelfe,
                               NEWTEXTMETRICEX*,
                               int type,
                               LPARAM lParam)
{
    if (lpelfe != 0 && type == TRUETYPE_FONTTYPE)
    {
        const String fontName (lpelfe->elfLogFont.lfFaceName);

        ((StringArray*) lParam)->addIfNotAlreadyThere (fontName.removeCharacters (T("@")));
    }

    return 1;
}
#endif

static int CALLBACK wfontEnum2 (ENUMLOGFONTEXW* lpelfe,
                                NEWTEXTMETRICEXW*,
                                int type,
                                LPARAM lParam)
{
    if (lpelfe != 0 && type == TRUETYPE_FONTTYPE)
    {
        const String fontName (lpelfe->elfLogFont.lfFaceName);

        ((StringArray*) lParam)->addIfNotAlreadyThere (fontName.removeCharacters (T("@")));
    }

    return 1;
}

#if JUCE_ENABLE_WIN98_COMPATIBILITY
static int CALLBACK fontEnum1 (ENUMLOGFONTEX* lpelfe,
                               NEWTEXTMETRICEX*,
                               int type,
                               LPARAM lParam)
{
    if (lpelfe != 0
        && ((type & (DEVICE_FONTTYPE | RASTER_FONTTYPE)) == 0))
    {
        LOGFONT lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;

        const String fontName (lpelfe->elfLogFont.lfFaceName);
        fontName.copyToBuffer (lf.lfFaceName, LF_FACESIZE - 1);

        HDC dc = CreateCompatibleDC (0);
        EnumFontFamiliesEx (dc, &lf,
                            (FONTENUMPROC) &fontEnum2,
                            lParam, 0);
        DeleteDC (dc);
    }

    return 1;
}
#endif

static int CALLBACK wfontEnum1 (ENUMLOGFONTEXW* lpelfe,
                                NEWTEXTMETRICEXW*,
                                int type,
                                LPARAM lParam)
{
    if (lpelfe != 0
        && ((type & (DEVICE_FONTTYPE | RASTER_FONTTYPE)) == 0))
    {
        LOGFONTW lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;

        const String fontName (lpelfe->elfLogFont.lfFaceName);
        fontName.copyToBuffer (lf.lfFaceName, LF_FACESIZE - 1);

        HDC dc = CreateCompatibleDC (0);
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        wEnumFontFamiliesExW (dc, &lf,
                              (FONTENUMPROCW) &wfontEnum2,
                              lParam, 0);
#else
        EnumFontFamiliesExW (dc, &lf,
                             (FONTENUMPROCW) &wfontEnum2,
                             lParam, 0);
#endif
        DeleteDC (dc);
    }

    return 1;
}

const StringArray Font::findAllTypefaceNames() throw()
{
    StringArray results;
    HDC dc = CreateCompatibleDC (0);

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wEnumFontFamiliesExW != 0)
    {
        LOGFONTW lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;
        lf.lfFaceName[0] = 0;

        wEnumFontFamiliesExW (dc, &lf,
                              (FONTENUMPROCW) &wfontEnum1,
                              (LPARAM) &results, 0);
    }
    else
    {
        LOGFONT lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;
        lf.lfFaceName[0] = 0;

        EnumFontFamiliesEx (dc, &lf,
                            (FONTENUMPROC) &fontEnum1,
                            (LPARAM) &results, 0);
    }
#else
    {
        LOGFONTW lf;
        zerostruct (lf);

        lf.lfWeight = FW_DONTCARE;
        lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = FF_DONTCARE;
        lf.lfFaceName[0] = 0;

        EnumFontFamiliesExW (dc, &lf,
                             (FONTENUMPROCW) &wfontEnum1,
                             (LPARAM) &results, 0);
    }
#endif

    DeleteDC (dc);

    results.sort (true);
    return results;
}

extern bool juce_IsRunningInWine() throw();

void Font::getDefaultFontNames (String& defaultSans,
                                String& defaultSerif,
                                String& defaultFixed) throw()
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
    HDC dc;
    String fontName;
    KERNINGPAIR* kps;
    int numKPs;
    bool bold, italic;
    int size;

    FontDCHolder (const FontDCHolder&);
    const FontDCHolder& operator= (const FontDCHolder&);

public:
    HFONT fontH;

    //==============================================================================
    FontDCHolder() throw()
        : dc (0),
          kps (0),
          numKPs (0),
          bold (false),
          italic (false),
          size (0)
    {
#if JUCE_ENABLE_WIN98_COMPATIBILITY
        juce_initialiseUnicodeFileFontFunctions();
#endif
    }

    ~FontDCHolder() throw()
    {
        if (dc != 0)
        {
            DeleteDC (dc);
            DeleteObject (fontH);
            juce_free (kps);
        }

        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FontDCHolder);

    //==============================================================================
    HDC loadFont (const String& fontName_,
                  const bool bold_,
                  const bool italic_,
                  const int size_) throw()
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

                juce_free (kps);
                kps = 0;
            }

            fontH = 0;

            dc = CreateCompatibleDC (0);
            SetMapperFlags (dc, 0);
            SetMapMode (dc, MM_TEXT);

#if JUCE_ENABLE_WIN98_COMPATIBILITY
            LOGFONT lf;
            LOGFONTW lfw;
            HFONT standardSizedFont = 0;

            if (wCreateFontIndirectW != 0)
            {
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
                standardSizedFont = wCreateFontIndirectW (&lfw);
            }
            else
            {
                zerostruct (lf);

                lf.lfCharSet = DEFAULT_CHARSET;
                lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
                lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
                lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
                lf.lfQuality = PROOF_QUALITY;
                lf.lfItalic = (BYTE) (italic ? TRUE : FALSE);
                lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
                fontName.copyToBuffer (lf.lfFaceName, LF_FACESIZE - 1);

                lf.lfHeight = size > 0 ? size : -256;
                standardSizedFont = CreateFontIndirect (&lf);
            }
#else
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
            HFONT standardSizedFont = CreateFontIndirectW (&lfw);
#endif

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
#if JUCE_ENABLE_WIN98_COMPATIBILITY
                            if (wCreateFontIndirectW != 0)
                            {
                                lfw.lfHeight = -(int) otm.otmEMSquare;
                                fontH = wCreateFontIndirectW (&lfw);
                            }
                            else
                            {
                                lf.lfHeight = -(int) otm.otmEMSquare;
                                fontH = CreateFontIndirect (&lf);
                            }
#else
                            lfw.lfHeight = -(int) otm.otmEMSquare;
                            fontH = CreateFontIndirectW (&lfw);
#endif

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
#if JUCE_ENABLE_WIN98_COMPATIBILITY
            if (wGetKerningPairsW != 0)
            {
                numKPs = wGetKerningPairsW (dc, 0, 0);
                kps = (KERNINGPAIR*) juce_calloc (sizeof (KERNINGPAIR) * numKPs);
                wGetKerningPairsW (dc, numKPs, kps);
            }
            else
            {
                numKPs = GetKerningPairs (dc, 0, 0);
                kps = (KERNINGPAIR*) juce_calloc (sizeof (KERNINGPAIR) * numKPs);
                GetKerningPairs (dc, numKPs, kps);
            }
#else
            numKPs = GetKerningPairsW (dc, 0, 0);
            kps = (KERNINGPAIR*) juce_calloc (sizeof (KERNINGPAIR) * numKPs);
            GetKerningPairsW (dc, numKPs, kps);
#endif
        }

        numKPs_ = numKPs;
        return kps;
    }
};

juce_ImplementSingleton_SingleThreaded (FontDCHolder);


//==============================================================================
static void addGlyphToTypeface (HDC dc,
                                juce_wchar character,
                                Typeface& dest,
                                bool addKerning)
{
    Path destShape;
    GLYPHMETRICS gm;

    float height;
    BOOL ok = false;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wGetTextMetricsW != 0)
    {
        TEXTMETRICW tm;
        ok = wGetTextMetricsW (dc, &tm);

        height = (float) tm.tmHeight;
    }
    else
    {
        TEXTMETRIC tm;
        ok = GetTextMetrics (dc, &tm);

        height = (float) tm.tmHeight;
    }
#else
    TEXTMETRICW tm;
    ok = GetTextMetricsW (dc, &tm);

    height = (float) tm.tmHeight;
#endif

    if (! ok)
    {
        dest.addGlyph (character, destShape, 0);
        return;
    }

    const float scaleX = 1.0f / height;
    const float scaleY = -1.0f / height;
    static const MAT2 identityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

    int bufSize;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wGetGlyphOutlineW != 0)
        bufSize = wGetGlyphOutlineW (dc, character, GGO_NATIVE,
                                     &gm, 0, 0, &identityMatrix);
    else
        bufSize = GetGlyphOutline (dc, character, GGO_NATIVE,
                                   &gm, 0, 0, &identityMatrix);
#else
    bufSize = GetGlyphOutlineW (dc, character, GGO_NATIVE,
                                &gm, 0, 0, &identityMatrix);
#endif

    if (bufSize > 0)
    {
        char* const data = (char*) juce_malloc (bufSize);

#if JUCE_ENABLE_WIN98_COMPATIBILITY
        if (wGetGlyphOutlineW != 0)
            wGetGlyphOutlineW (dc, character, GGO_NATIVE, &gm,
                               bufSize, data, &identityMatrix);
        else
            GetGlyphOutline (dc, character, GGO_NATIVE, &gm,
                             bufSize, data, &identityMatrix);
#else
        GetGlyphOutlineW (dc, character, GGO_NATIVE, &gm,
                          bufSize, data, &identityMatrix);
#endif

        const TTPOLYGONHEADER* pheader = (TTPOLYGONHEADER*) data;

        while ((char*) pheader < data + bufSize)
        {
            #define remapX(v) (scaleX * (v).x.value)
            #define remapY(v) (scaleY * (v).y.value)

            float x = remapX (pheader->pfxStart);
            float y = remapY (pheader->pfxStart);

            destShape.startNewSubPath (x, y);

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

                        destShape.lineTo (x, y);
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

                        destShape.quadraticTo (x2, y2, x3, y3);

                        x = x3;
                        y = y3;
                    }
                }

                curve = (const TTPOLYCURVE*) &(curve->apfx [curve->cpfx]);
            }

            pheader = (const TTPOLYGONHEADER*) curve;

            destShape.closeSubPath();
        }

        juce_free (data);
    }

    dest.addGlyph (character, destShape, gm.gmCellIncX / height);

    if (addKerning)
    {
        int numKPs;
        const KERNINGPAIR* const kps = FontDCHolder::getInstance()->getKerningPairs (numKPs);

        for (int i = 0; i < numKPs; ++i)
        {
            if (kps[i].wFirst == character)
            {
                dest.addKerningPair (kps[i].wFirst,
                                     kps[i].wSecond,
                                     kps[i].iKernAmount / height);
            }
        }
    }
}

//==============================================================================
void Typeface::findAndAddSystemGlyph (juce_wchar character) throw()
{
    HDC dc = FontDCHolder::getInstance()->loadFont (getName(), isBold(), isItalic(), 0);
    addGlyphToTypeface (dc, character, *this, true);
}

/*Image* Typeface::renderGlyphToImage (juce_wchar character, float& topLeftX, float& topLeftY)
{
    HDC dc = FontDCHolder::getInstance()->loadFont (getName(), isBold(), isItalic(), hintingSize);

    int bufSize;
    GLYPHMETRICS gm;

    const UINT format = GGO_GRAY2_BITMAP;
    const int shift = 6;

    if (wGetGlyphOutlineW != 0)
        bufSize = wGetGlyphOutlineW (dc, character, format, &gm, 0, 0, &identityMatrix);
    else
        bufSize = GetGlyphOutline (dc, character, format, &gm, 0, 0, &identityMatrix);

    Image* im = new Image (Image::SingleChannel, jmax (1, gm.gmBlackBoxX), jmax (1, gm.gmBlackBoxY), true);

    if (bufSize > 0)
    {
        topLeftX = (float) gm.gmptGlyphOrigin.x;
        topLeftY = (float) -gm.gmptGlyphOrigin.y;

        uint8* const data = (uint8*) juce_calloc (bufSize);

        if (wGetGlyphOutlineW != 0)
            wGetGlyphOutlineW (dc, character, format, &gm, bufSize, data, &identityMatrix);
        else
            GetGlyphOutline (dc, character, format, &gm, bufSize, data, &identityMatrix);

        const int stride = ((gm.gmBlackBoxX + 3) & ~3);

        for (int y = gm.gmBlackBoxY; --y >= 0;)
        {
            for (int x = gm.gmBlackBoxX; --x >= 0;)
            {
                const int level = data [x + y * stride] << shift;

                if (level > 0)
                    im->setPixelAt (x, y, Colour ((uint8) 0xff, (uint8) 0xff, (uint8) 0xff, (uint8) jmin (0xff, level)));
            }
        }

        juce_free (data);
    }

    return im;
}*/

//==============================================================================
void Typeface::initialiseTypefaceCharacteristics (const String& fontName,
                                                  bool bold,
                                                  bool italic,
                                                  bool addAllGlyphsToFont) throw()
{
    clear();

    HDC dc = FontDCHolder::getInstance()->loadFont (fontName, bold, italic, 0);

    float height;
    int firstChar, lastChar;

#if JUCE_ENABLE_WIN98_COMPATIBILITY
    if (wGetTextMetricsW != 0)
    {
        TEXTMETRICW tm;
        wGetTextMetricsW (dc, &tm);

        height = (float) tm.tmHeight;
        firstChar = tm.tmFirstChar;
        lastChar = tm.tmLastChar;

        setAscent (tm.tmAscent / height);
        setDefaultCharacter (tm.tmDefaultChar);
    }
    else
    {
        TEXTMETRIC tm;
        GetTextMetrics (dc, &tm);

        height = (float) tm.tmHeight;
        firstChar = tm.tmFirstChar;
        lastChar = tm.tmLastChar;

        setAscent (tm.tmAscent / height);
        setDefaultCharacter (tm.tmDefaultChar);
    }
#else
    {
        TEXTMETRICW tm;
        GetTextMetricsW (dc, &tm);

        height = (float) tm.tmHeight;
        firstChar = tm.tmFirstChar;
        lastChar = tm.tmLastChar;

        setAscent (tm.tmAscent / height);
        setDefaultCharacter (tm.tmDefaultChar);
    }
#endif

    setName (fontName);
    setBold (bold);
    setItalic (italic);

    if (addAllGlyphsToFont)
    {
        for (int character = firstChar; character <= lastChar; ++character)
            addGlyphToTypeface (dc, (juce_wchar) character, *this, false);

        int numKPs;
        const KERNINGPAIR* const kps = FontDCHolder::getInstance()->getKerningPairs (numKPs);

        for (int i = 0; i < numKPs; ++i)
        {
            addKerningPair (kps[i].wFirst,
                            kps[i].wSecond,
                            kps[i].iKernAmount / height);
        }
    }
}

END_JUCE_NAMESPACE
