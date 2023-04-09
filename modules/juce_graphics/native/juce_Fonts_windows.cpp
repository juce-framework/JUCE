/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/*  This is some quick-and-dirty code to extract the typeface name from a lump of TTF file data.
    It's needed because although win32 will happily load a TTF file from in-memory data, it won't
    tell you the name of the damned font that it just loaded.. and in order to actually use the font,
    you need to know its name!! Anyway, this awful hack seems to work for most fonts.
*/
namespace TTFNameExtractor
{
    struct OffsetTable
    {
        uint32 version;
        uint16 numTables, searchRange, entrySelector, rangeShift;
    };

    struct TableDirectory
    {
        char tag[4];
        uint32 checkSum, offset, length;
    };

    struct NamingTable
    {
        uint16 formatSelector;
        uint16 numberOfNameRecords;
        uint16 offsetStartOfStringStorage;
    };

    struct NameRecord
    {
        uint16 platformID, encodingID, languageID;
        uint16 nameID, stringLength, offsetFromStorageArea;
    };

    static String parseNameRecord (MemoryInputStream& input, const NameRecord& nameRecord,
                                   const int64 directoryOffset, const int64 offsetOfStringStorage)
    {
        String result;
        auto oldPos = input.getPosition();
        input.setPosition (directoryOffset + offsetOfStringStorage + ByteOrder::swapIfLittleEndian (nameRecord.offsetFromStorageArea));
        auto stringLength = (int) ByteOrder::swapIfLittleEndian (nameRecord.stringLength);
        auto platformID = ByteOrder::swapIfLittleEndian (nameRecord.platformID);

        if (platformID == 0 || platformID == 3)
        {
            auto numChars = stringLength / 2 + 1;
            HeapBlock<uint16> buffer;
            buffer.calloc (numChars + 1);
            input.read (buffer, stringLength);

            for (int i = 0; i < numChars; ++i)
                buffer[i] = ByteOrder::swapIfLittleEndian (buffer[i]);

            static_assert (sizeof (CharPointer_UTF16::CharType) == sizeof (uint16), "Sanity check UTF-16 type");
            result = CharPointer_UTF16 ((CharPointer_UTF16::CharType*) buffer.getData());
        }
        else
        {
            HeapBlock<char> buffer;
            buffer.calloc (stringLength + 1);
            input.read (buffer, stringLength);
            result = CharPointer_UTF8 (buffer.getData());
        }

        input.setPosition (oldPos);
        return result;
    }

    static String parseNameTable (MemoryInputStream& input, int64 directoryOffset)
    {
        input.setPosition (directoryOffset);

        NamingTable namingTable = {};
        input.read (&namingTable, sizeof (namingTable));

        for (int i = 0; i < (int) ByteOrder::swapIfLittleEndian (namingTable.numberOfNameRecords); ++i)
        {
            NameRecord nameRecord = {};
            input.read (&nameRecord, sizeof (nameRecord));

            if (ByteOrder::swapIfLittleEndian (nameRecord.nameID) == 4)
            {
                const String result (parseNameRecord (input, nameRecord, directoryOffset,
                                                      ByteOrder::swapIfLittleEndian (namingTable.offsetStartOfStringStorage)));

                if (result.isNotEmpty())
                    return result;
            }
        }

        return {};
    }

    static String getTypefaceNameFromFile (MemoryInputStream& input)
    {
        OffsetTable offsetTable = {};
        input.read (&offsetTable, sizeof (offsetTable));

        for (int i = 0; i < (int) ByteOrder::swapIfLittleEndian (offsetTable.numTables); ++i)
        {
            TableDirectory tableDirectory;
            zerostruct (tableDirectory);
            input.read (&tableDirectory, sizeof (tableDirectory));

            if (String (tableDirectory.tag, sizeof (tableDirectory.tag)).equalsIgnoreCase ("name"))
                return parseNameTable (input, ByteOrder::swapIfLittleEndian (tableDirectory.offset));
        }

        return {};
    }
}

namespace FontEnumerators
{
    static int CALLBACK fontEnum2 (ENUMLOGFONTEXW* lpelfe, NEWTEXTMETRICEXW*, int type, LPARAM lParam)
    {
        if (lpelfe != nullptr && (type & RASTER_FONTTYPE) == 0)
        {
            const String fontName (lpelfe->elfLogFont.lfFaceName);
            ((StringArray*) lParam)->addIfNotAlreadyThere (fontName.removeCharacters ("@"));
        }

        return 1;
    }

    static int CALLBACK fontEnum1 (ENUMLOGFONTEXW* lpelfe, NEWTEXTMETRICEXW*, int type, LPARAM lParam)
    {
        if (lpelfe != nullptr && (type & RASTER_FONTTYPE) == 0)
        {
            LOGFONTW lf = {};
            lf.lfWeight = FW_DONTCARE;
            lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
            lf.lfQuality = DEFAULT_QUALITY;
            lf.lfCharSet = DEFAULT_CHARSET;
            lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lf.lfPitchAndFamily = FF_DONTCARE;

            const String fontName (lpelfe->elfLogFont.lfFaceName);
            fontName.copyToUTF16 (lf.lfFaceName, sizeof (lf.lfFaceName));

            auto dc = CreateCompatibleDC (nullptr);
            EnumFontFamiliesEx (dc, &lf, (FONTENUMPROCW) &fontEnum2, lParam, 0);
            DeleteDC (dc);
        }

        return 1;
    }
}

StringArray Font::findAllTypefaceNames()
{
    StringArray results;

   #if JUCE_USE_DIRECTWRITE
    SharedResourcePointer<Direct2DFactories> factories;

    if (factories->systemFonts != nullptr)
    {
        ComSmartPtr<IDWriteFontFamily> fontFamily;
        uint32 fontFamilyCount = 0;
        fontFamilyCount = factories->systemFonts->GetFontFamilyCount();

        for (uint32 i = 0; i < fontFamilyCount; ++i)
        {
            auto hr = factories->systemFonts->GetFontFamily (i, fontFamily.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                results.addIfNotAlreadyThere (getFontFamilyName (fontFamily));
        }
    }
    else
   #endif
    {
        auto dc = CreateCompatibleDC (nullptr);

        {
            LOGFONTW lf = {};
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
    }

    results.sort (true);
    return results;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    StringArray results;

   #if JUCE_USE_DIRECTWRITE
    SharedResourcePointer<Direct2DFactories> factories;

    if (factories->systemFonts != nullptr)
    {
        BOOL fontFound = false;
        uint32 fontIndex = 0;
        [[maybe_unused]] auto hr = factories->systemFonts->FindFamilyName (family.toWideCharPointer(), &fontIndex, &fontFound);

        if (! fontFound)
            fontIndex = 0;

        // Get the font family using the search results
        // Fonts like: Times New Roman, Times New Roman Bold, Times New Roman Italic are all in the same font family
        ComSmartPtr<IDWriteFontFamily> fontFamily;
        hr = factories->systemFonts->GetFontFamily (fontIndex, fontFamily.resetAndGetPointerAddress());

        // Get the font faces
        ComSmartPtr<IDWriteFont> dwFont;
        uint32 fontFacesCount = 0;
        fontFacesCount = fontFamily->GetFontCount();

        for (uint32 i = 0; i < fontFacesCount; ++i)
        {
            hr = fontFamily->GetFont (i, dwFont.resetAndGetPointerAddress());

            // Ignore any algorithmically generated bold and oblique styles..
            if (dwFont->GetSimulations() == DWRITE_FONT_SIMULATIONS_NONE)
                results.addIfNotAlreadyThere (getFontFaceName (dwFont));
        }
    }
    else
   #endif
    {
        results.add ("Regular");
        results.add ("Italic");
        results.add ("Bold");
        results.add ("Bold Italic");
    }

    return results;
}

extern bool juce_isRunningInWine();

struct DefaultFontNames
{
    DefaultFontNames()
    {
        if (juce_isRunningInWine())
        {
            // If we're running in Wine, then use fonts that might be available on Linux..
            defaultSans     = "Bitstream Vera Sans";
            defaultSerif    = "Bitstream Vera Serif";
            defaultFixed    = "Bitstream Vera Sans Mono";
        }
        else
        {
            defaultSans     = "Verdana";
            defaultSerif    = "Times New Roman";
            defaultFixed    = "Lucida Console";
            defaultFallback = "Tahoma";  // (contains plenty of unicode characters)
        }
    }

    String defaultSans, defaultSerif, defaultFixed, defaultFallback;
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    Font newFont (font);
    auto faceName = font.getTypefaceName();

    if (faceName == getDefaultSansSerifFontName())       newFont.setTypefaceName (defaultNames.defaultSans);
    else if (faceName == getDefaultSerifFontName())      newFont.setTypefaceName (defaultNames.defaultSerif);
    else if (faceName == getDefaultMonospacedFontName()) newFont.setTypefaceName (defaultNames.defaultFixed);

    if (font.getTypefaceStyle() == getDefaultStyle())
        newFont.setTypefaceStyle ("Regular");

    return Typeface::createSystemTypefaceFor (newFont);
}

//==============================================================================
class WindowsTypeface   : public Typeface
{
public:
    WindowsTypeface (const Font& font)  : Typeface (font.getTypefaceName(),
                                                    font.getTypefaceStyle())
    {
        loadFont();
    }

    WindowsTypeface (const void* data, size_t dataSize)
        : Typeface (String(), String())
    {
        DWORD numInstalled = 0;
        memoryFont = AddFontMemResourceEx (const_cast<void*> (data), (DWORD) dataSize,
                                           nullptr, &numInstalled);

        MemoryInputStream m (data, dataSize, false);
        name = TTFNameExtractor::getTypefaceNameFromFile (m);
        loadFont();
    }

    ~WindowsTypeface()
    {
        SelectObject (dc, previousFontH); // Replacing the previous font before deleting the DC avoids a warning in BoundsChecker
        DeleteDC (dc);

        if (fontH != nullptr)
            DeleteObject (fontH);

        if (memoryFont != nullptr)
            RemoveFontMemResourceEx (memoryFont);
    }

    float getAscent() const                 { return ascent; }
    float getDescent() const                { return 1.0f - ascent; }
    float getHeightToPointsFactor() const   { return heightToPointsFactor; }

    float getStringWidth (const String& text)
    {
        auto utf16 = text.toUTF16();
        auto numChars = utf16.length();
        HeapBlock<uint16> results (numChars);
        float x = 0;

        if (GetGlyphIndices (dc, utf16, (int) numChars, reinterpret_cast<WORD*> (results.getData()),
                             GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
        {
            for (size_t i = 0; i < numChars; ++i)
                x += getKerning (dc, results[i], (i + 1) < numChars ? results[i + 1] : -1);
        }

        return x;
    }

    void getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets)
    {
        auto utf16 = text.toUTF16();
        auto numChars = utf16.length();
        HeapBlock<uint16> results (numChars);
        float x = 0;

        if (GetGlyphIndices (dc, utf16, (int) numChars, reinterpret_cast<WORD*> (results.getData()),
                             GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR)
        {
            resultGlyphs.ensureStorageAllocated ((int) numChars);
            xOffsets.ensureStorageAllocated ((int) numChars + 1);

            for (size_t i = 0; i < numChars; ++i)
            {
                resultGlyphs.add (results[i]);
                xOffsets.add (x);
                x += getKerning (dc, results[i], (i + 1) < numChars ? results[i + 1] : -1);
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
        auto bufSize = (int) GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX,
                                              &gm, 0, nullptr, &identityMatrix);

        if (bufSize > 0)
        {
            HeapBlock<char> data (bufSize);
            GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX, &gm,
                             (DWORD) bufSize, data, &identityMatrix);

            auto pheader = reinterpret_cast<const TTPOLYGONHEADER*> (data.getData());

            auto scaleX = 1.0f / (float) tm.tmHeight;
            auto scaleY = -scaleX;

            while ((char*) pheader < data + bufSize)
            {
                glyphPath.startNewSubPath (scaleX * pheader->pfxStart.x.value,
                                           scaleY * pheader->pfxStart.y.value);

                auto curve = unalignedPointerCast<const TTPOLYCURVE*> ((const char*) pheader + sizeof (TTPOLYGONHEADER));
                auto curveEnd = ((const char*) pheader) + pheader->cb;

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
                            auto x2 = scaleX * curve->apfx[i].x.value;
                            auto y2 = scaleY * curve->apfx[i].y.value;
                            auto x3 = scaleX * curve->apfx[i + 1].x.value;
                            auto y3 = scaleY * curve->apfx[i + 1].y.value;

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

                pheader = unalignedPointerCast<const TTPOLYGONHEADER*> (curve);

                glyphPath.closeSubPath();
            }
        }

        return true;
    }

private:
    static const MAT2 identityMatrix;
    HFONT fontH = {};
    HGDIOBJ previousFontH = {};
    HDC dc { CreateCompatibleDC (nullptr) };
    TEXTMETRIC tm;
    HANDLE memoryFont = {};
    float ascent = 1.0f, heightToPointsFactor = 1.0f;
    int defaultGlyph = -1, heightInPoints = 0;
    std::unordered_map<uint64, float> kerningPairs;

    static uint64 kerningPairIndex (int glyph1, int glyph2)
    {
        return (((uint64) (uint32) glyph1) << 32) | (uint64) (uint32) glyph2;
    }

    void loadFont()
    {
        SetMapperFlags (dc, 0);
        SetMapMode (dc, MM_TEXT);

        LOGFONTW lf = {};
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfItalic = (BYTE) (style.contains ("Italic") ? TRUE : FALSE);
        lf.lfWeight = style.contains ("Bold") ? FW_BOLD : FW_NORMAL;
        lf.lfHeight = -256;
        name.copyToUTF16 (lf.lfFaceName, sizeof (lf.lfFaceName));

        auto standardSizedFont = CreateFontIndirect (&lf);

        if (standardSizedFont != nullptr)
        {
            if ((previousFontH = SelectObject (dc, standardSizedFont)) != nullptr)
            {
                fontH = standardSizedFont;
                OUTLINETEXTMETRIC otm;

                if (GetOutlineTextMetrics (dc, sizeof (otm), &otm) != 0)
                {
                    heightInPoints = (int) otm.otmEMSquare;
                    lf.lfHeight = -heightInPoints;
                    fontH = CreateFontIndirect (&lf);

                    SelectObject (dc, fontH);
                    DeleteObject (standardSizedFont);
                }
            }
        }

        if (GetTextMetrics (dc, &tm))
        {
            auto dpi = (float) (GetDeviceCaps (dc, LOGPIXELSX) + GetDeviceCaps (dc, LOGPIXELSY)) / 2.0f;
            heightToPointsFactor = (dpi / (float) GetDeviceCaps (dc, LOGPIXELSY)) * (float) heightInPoints / (float) tm.tmHeight;
            ascent = (float) tm.tmAscent / (float) tm.tmHeight;
            std::unordered_map<int, int> glyphsForChars;
            defaultGlyph = getGlyphForChar (dc, glyphsForChars, tm.tmDefaultChar);
            createKerningPairs (dc, glyphsForChars, (float) tm.tmHeight);
        }
    }

    void createKerningPairs (HDC hdc, std::unordered_map<int, int>& glyphsForChars, float height)
    {
        HeapBlock<KERNINGPAIR> rawKerning;
        auto numKPs = GetKerningPairs (hdc, 0, nullptr);
        rawKerning.calloc (numKPs);
        GetKerningPairs (hdc, numKPs, rawKerning);

        std::unordered_map<int, int> widthsForGlyphs;

        for (DWORD i = 0; i < numKPs; ++i)
        {
            auto glyph1 = getGlyphForChar (hdc, glyphsForChars, rawKerning[i].wFirst);
            auto glyph2 = getGlyphForChar (hdc, glyphsForChars, rawKerning[i].wSecond);
            auto standardWidth = getGlyphWidth (hdc, widthsForGlyphs, glyph1);

            kerningPairs[kerningPairIndex (glyph1, glyph2)] = (float) (standardWidth + rawKerning[i].iKernAmount) / height;
            kerningPairs[kerningPairIndex (glyph1, -1)]     = (float) standardWidth / height;
        }
    }

    static int getGlyphForChar (HDC dc, std::unordered_map<int, int>& cache, juce_wchar character)
    {
        auto existing = cache.find ((int) character);

        if (existing != cache.end())
            return existing->second;

        const WCHAR charToTest[] = { (WCHAR) character, 0 };
        WORD index = 0;

        if (GetGlyphIndices (dc, charToTest, 1, &index, GGI_MARK_NONEXISTING_GLYPHS) == GDI_ERROR
              || index == 0xffff)
            return -1;

        cache[(int) character] = index;
        return index;
    }

    static int getGlyphWidth (HDC dc, std::unordered_map<int, int>& cache, int glyphNumber)
    {
        auto existing = cache.find (glyphNumber);

        if (existing != cache.end())
            return existing->second;

        auto width = getGlyphWidth (dc, glyphNumber);
        cache[glyphNumber] = width;
        return width;
    }

    static int getGlyphWidth (HDC dc, int glyphNumber)
    {
        GLYPHMETRICS gm;
        gm.gmCellIncX = 0;
        GetGlyphOutline (dc, (UINT) glyphNumber, GGO_NATIVE | GGO_GLYPH_INDEX, &gm, 0, nullptr, &identityMatrix);
        return gm.gmCellIncX;
    }

    float getKerning (HDC hdc, int glyph1, int glyph2)
    {
        auto pair = kerningPairs.find (kerningPairIndex (glyph1, glyph2));

        if (pair != kerningPairs.end())
            return pair->second;

        auto single = kerningPairs.find (kerningPairIndex (glyph1, -1));

        if (single != kerningPairs.end())
            return single->second;

        auto width = (float) getGlyphWidth (hdc, glyph1) / (float) tm.tmHeight;
        kerningPairs[kerningPairIndex (glyph1, -1)] = width;
        return width;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTypeface)
};

const MAT2 WindowsTypeface::identityMatrix = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
   #if JUCE_USE_DIRECTWRITE
    SharedResourcePointer<Direct2DFactories> factories;

    if (factories->systemFonts != nullptr)
    {
        std::unique_ptr<WindowsDirectWriteTypeface> wtf (new WindowsDirectWriteTypeface (font, factories->systemFonts));

        if (wtf->loadedOk() && wtf->isFontFound())
            return wtf.release();
    }
   #endif

    return new WindowsTypeface (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t dataSize)
{
    return new WindowsTypeface (data, dataSize);
}

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not implemented on this platform
}

} // namespace juce
