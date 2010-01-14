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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
  #define SUPPORT_10_4_FONTS 1
  #define NEW_CGFONT_FUNCTIONS_UNAVAILABLE (CGFontCreateWithFontName == 0)

  #if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
    #define SUPPORT_ONLY_10_4_FONTS 1
  #endif

  END_JUCE_NAMESPACE
  @interface NSFont (PrivateHack)
    - (NSGlyph) _defaultGlyphForChar: (unichar) theChar;
  @end
  BEGIN_JUCE_NAMESPACE
#endif

//==============================================================================
class MacTypeface  : public Typeface
{
public:
    //==============================================================================
    MacTypeface (const Font& font)
        : Typeface (font.getTypefaceName())
    {
        const ScopedAutoReleasePool pool;
        renderingTransform = CGAffineTransformIdentity;

        bool needsItalicTransform = false;

#if JUCE_IPHONE
        NSString* fontName = juceStringToNS (font.getTypefaceName());

        if (font.isItalic() || font.isBold())
        {
            NSArray* familyFonts = [UIFont fontNamesForFamilyName: juceStringToNS (font.getTypefaceName())];

            for (NSString* i in familyFonts)
            {
                const String fn (nsStringToJuce (i));
                const String afterDash (fn.fromFirstOccurrenceOf (T("-"), false, false));

                const bool probablyBold = afterDash.containsIgnoreCase (T("bold")) || fn.endsWithIgnoreCase (T("bold"));
                const bool probablyItalic = afterDash.containsIgnoreCase (T("oblique"))
                                             || afterDash.containsIgnoreCase (T("italic"))
                                             || fn.endsWithIgnoreCase (T("oblique"))
                                             || fn.endsWithIgnoreCase (T("italic"));

                if (probablyBold == font.isBold()
                     && probablyItalic == font.isItalic())
                {
                    fontName = i;
                    needsItalicTransform = false;
                    break;
                }
                else if (probablyBold && (! probablyItalic) && probablyBold == font.isBold())
                {
                    fontName = i;
                    needsItalicTransform = true; // not ideal, so carry on in case we find a better one
                }
            }

            if (needsItalicTransform)
                renderingTransform.c = 0.15f;
        }

        fontRef = CGFontCreateWithFontName ((CFStringRef) fontName);

        const int ascender = abs (CGFontGetAscent (fontRef));
        const float totalHeight = ascender + abs (CGFontGetDescent (fontRef));
        ascent = ascender / totalHeight;
        unitsToHeightScaleFactor = 1.0f / totalHeight;
        fontHeightToCGSizeFactor = CGFontGetUnitsPerEm (fontRef) / totalHeight;
#else
        nsFont = [NSFont fontWithName: juceStringToNS (font.getTypefaceName()) size: 1024];

        if (font.isItalic())
        {
            NSFont* newFont = [[NSFontManager sharedFontManager] convertFont: nsFont
                                                                 toHaveTrait: NSItalicFontMask];

            if (newFont == nsFont)
                needsItalicTransform = true; // couldn't find a proper italic version, so fake it with a transform..

            nsFont = newFont;
        }

        if (font.isBold())
            nsFont = [[NSFontManager sharedFontManager] convertFont: nsFont toHaveTrait: NSBoldFontMask];

        [nsFont retain];

        ascent = fabsf ((float) [nsFont ascender]);
        float totalSize = ascent + fabsf ((float) [nsFont descender]);
        ascent /= totalSize;

        pathTransform = AffineTransform::identity.scale (1.0f / totalSize, 1.0f / totalSize);

        if (needsItalicTransform)
        {
            pathTransform = pathTransform.sheared (-0.15f, 0.0f);
            renderingTransform.c = 0.15f;
        }

#if SUPPORT_ONLY_10_4_FONTS
        ATSFontRef atsFont = ATSFontFindFromName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

        if (atsFont == 0)
            atsFont = ATSFontFindFromPostScriptName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

        fontRef = CGFontCreateWithPlatformFont ((void*) &atsFont);

        const float totalHeight = fabsf ([nsFont ascender]) + fabsf([nsFont descender]);
        unitsToHeightScaleFactor = 1.0f / totalHeight;
        fontHeightToCGSizeFactor = 1024.0f / totalHeight;
#else
  #if SUPPORT_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
        {
            ATSFontRef atsFont = ATSFontFindFromName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

            if (atsFont == 0)
                atsFont = ATSFontFindFromPostScriptName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

            fontRef = CGFontCreateWithPlatformFont ((void*) &atsFont);

            const float totalHeight = fabsf ([nsFont ascender]) + fabsf([nsFont descender]);
            unitsToHeightScaleFactor = 1.0f / totalHeight;
            fontHeightToCGSizeFactor = 1024.0f / totalHeight;
        }
        else
  #endif
        {
            fontRef = CGFontCreateWithFontName ((CFStringRef) [nsFont fontName]);

            const int totalHeight = abs (CGFontGetAscent (fontRef)) + abs (CGFontGetDescent (fontRef));
            unitsToHeightScaleFactor = 1.0f / totalHeight;
            fontHeightToCGSizeFactor = CGFontGetUnitsPerEm (fontRef) / (float) totalHeight;
        }
#endif

#endif
    }

    ~MacTypeface()
    {
#if ! JUCE_IPHONE
        [nsFont release];
#endif
        if (fontRef != 0)
            CGFontRelease (fontRef);
    }

    float getAscent() const
    {
        return ascent;
    }

    float getDescent() const
    {
        return 1.0f - ascent;
    }

    float getStringWidth (const String& text)
    {
        if (fontRef == 0 || text.isEmpty())
            return 0;

        const int length = text.length();
        HeapBlock <CGGlyph> glyphs;
        createGlyphsForString (text, length, glyphs);

        float x = 0;

#if SUPPORT_ONLY_10_4_FONTS
        HeapBlock <NSSize> advances (length);
        [nsFont getAdvancements: advances forGlyphs: (NSGlyph*) glyphs count: length];

        for (int i = 0; i < length; ++i)
            x += advances[i].width;
#else
  #if SUPPORT_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
        {
            HeapBlock <NSSize> advances (length);
            [nsFont getAdvancements: advances forGlyphs: (NSGlyph*) glyphs count: length];

            for (int i = 0; i < length; ++i)
                x += advances[i].width;
        }
        else
  #endif
        {
            HeapBlock <int> advances (length);

            if (CGFontGetGlyphAdvances (fontRef, glyphs, length, advances))
                for (int i = 0; i < length; ++i)
                    x += advances[i];
        }
#endif

        return x * unitsToHeightScaleFactor;
    }

    void getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array <float>& xOffsets)
    {
        xOffsets.add (0);

        if (fontRef == 0 || text.isEmpty())
            return;

        const int length = text.length();
        HeapBlock <CGGlyph> glyphs;
        createGlyphsForString (text, length, glyphs);

#if SUPPORT_ONLY_10_4_FONTS
        HeapBlock <NSSize> advances (length);
        [nsFont getAdvancements: advances forGlyphs: (NSGlyph*) glyphs count: length];

        int x = 0;
        for (int i = 0; i < length; ++i)
        {
            x += advances[i].width;
            xOffsets.add (x * unitsToHeightScaleFactor);
            resultGlyphs.add (((NSGlyph*) glyphs)[i]);
        }

#else
  #if SUPPORT_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
        {
            HeapBlock <NSSize> advances (length);
            [nsFont getAdvancements: advances forGlyphs: (NSGlyph*) glyphs count: length];

            float x = 0;
            for (int i = 0; i < length; ++i)
            {
                x += advances[i].width;
                xOffsets.add (x * unitsToHeightScaleFactor);
                resultGlyphs.add (((NSGlyph*) glyphs)[i]);
            }
        }
        else
  #endif
        {
            HeapBlock <int> advances (length);

            if (CGFontGetGlyphAdvances (fontRef, glyphs, length, advances))
            {
                int x = 0;
                for (int i = 0; i < length; ++i)
                {
                    x += advances [i];
                    xOffsets.add (x * unitsToHeightScaleFactor);
                    resultGlyphs.add (glyphs[i]);
                }
            }
        }
#endif
    }

    bool getOutlineForGlyph (int glyphNumber, Path& path)
    {
#if JUCE_IPHONE
        return false;
#else
        if (nsFont == 0)
            return false;

        // we might need to apply a transform to the path, so it mustn't have anything else in it
        jassert (path.isEmpty());

        const ScopedAutoReleasePool pool;

        NSBezierPath* bez = [NSBezierPath bezierPath];
        [bez moveToPoint: NSMakePoint (0, 0)];
        [bez appendBezierPathWithGlyph: (NSGlyph) glyphNumber
                                inFont: nsFont];

        for (int i = 0; i < [bez elementCount]; ++i)
        {
            NSPoint p[3];
            switch ([bez elementAtIndex: i associatedPoints: p])
            {
            case NSMoveToBezierPathElement:
                path.startNewSubPath ((float) p[0].x, (float) -p[0].y);
                break;
            case NSLineToBezierPathElement:
                path.lineTo ((float) p[0].x, (float) -p[0].y);
                break;
            case NSCurveToBezierPathElement:
                path.cubicTo ((float) p[0].x, (float) -p[0].y, (float) p[1].x, (float) -p[1].y, (float) p[2].x, (float) -p[2].y);
                break;
            case NSClosePathBezierPathElement:
                path.closeSubPath();
                break;
            default:
                jassertfalse
                break;
            }
        }

        path.applyTransform (pathTransform);
        return true;
#endif
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    CGFontRef fontRef;
    float fontHeightToCGSizeFactor;
    CGAffineTransform renderingTransform;

private:
    float ascent, unitsToHeightScaleFactor;

#if JUCE_IPHONE

#else
    NSFont* nsFont;
    AffineTransform pathTransform;
#endif

    void createGlyphsForString (const juce_wchar* const text, const int length, HeapBlock <CGGlyph>& glyphs)
    {
#if SUPPORT_10_4_FONTS
  #if ! SUPPORT_ONLY_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
  #endif
        {
            glyphs.malloc (sizeof (NSGlyph) * length, 1);
            NSGlyph* const g = (NSGlyph*) glyphs;

            for (int i = 0; i < length; ++i)
                g[i] = (NSGlyph) [nsFont _defaultGlyphForChar: text[i]];

            return;
        }
#endif

#if ! SUPPORT_ONLY_10_4_FONTS
        if (charToGlyphMapper == 0)
            charToGlyphMapper = new CharToGlyphMapper (fontRef);

        glyphs.malloc (length);

        for (int i = 0; i < length; ++i)
            glyphs[i] = (CGGlyph) charToGlyphMapper->getGlyphForCharacter (text[i]);
#endif
    }

#if ! SUPPORT_ONLY_10_4_FONTS
    // Reads a CGFontRef's character map table to convert unicode into glyph numbers
    class CharToGlyphMapper
    {
    public:
        CharToGlyphMapper (CGFontRef fontRef)
            : segCount (0), endCode (0), startCode (0), idDelta (0),
              idRangeOffset (0), glyphIndexes (0)
        {
            CFDataRef cmapTable = CGFontCopyTableForTag (fontRef, 'cmap');

            if (cmapTable != 0)
            {
                const int numSubtables = getValue16 (cmapTable, 2);

                for (int i = 0; i < numSubtables; ++i)
                {
                    if (getValue16 (cmapTable, i * 8 + 4) == 0) // check for platform ID of 0
                    {
                        const int offset = getValue32 (cmapTable, i * 8 + 8);

                        if (getValue16 (cmapTable, offset) == 4) // check that it's format 4..
                        {
                            const int length = getValue16 (cmapTable, offset + 2);
                            const int segCountX2 =  getValue16 (cmapTable, offset + 6);
                            segCount = segCountX2 / 2;
                            const int endCodeOffset = offset + 14;
                            const int startCodeOffset = endCodeOffset + 2 + segCountX2;
                            const int idDeltaOffset = startCodeOffset + segCountX2;
                            const int idRangeOffsetOffset = idDeltaOffset + segCountX2;
                            const int glyphIndexesOffset = idRangeOffsetOffset + segCountX2;

                            endCode = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + endCodeOffset, segCountX2);
                            startCode = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + startCodeOffset, segCountX2);
                            idDelta = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + idDeltaOffset, segCountX2);
                            idRangeOffset = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + idRangeOffsetOffset, segCountX2);
                            glyphIndexes = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + glyphIndexesOffset, offset + length - glyphIndexesOffset);
                        }

                        break;
                    }
                }

                CFRelease (cmapTable);
            }
        }

        ~CharToGlyphMapper()
        {
            if (endCode != 0)
            {
                CFRelease (endCode);
                CFRelease (startCode);
                CFRelease (idDelta);
                CFRelease (idRangeOffset);
                CFRelease (glyphIndexes);
            }
        }

        int getGlyphForCharacter (const juce_wchar c) const
        {
            for (int i = 0; i < segCount; ++i)
            {
                if (getValue16 (endCode, i * 2) >= c)
                {
                    const int start = getValue16 (startCode, i * 2);
                    if (start > c)
                        break;

                    const int delta = getValue16 (idDelta, i * 2);
                    const int rangeOffset = getValue16 (idRangeOffset, i * 2);

                    if (rangeOffset == 0)
                        return delta + c;
                    else
                        return getValue16 (glyphIndexes, 2 * ((rangeOffset / 2) + (c - start) - (segCount - i)));
                }
            }

            // If we failed to find it "properly", this dodgy fall-back seems to do the trick for most fonts!
            return jmax (-1, c - 29);
        }

    private:
        int segCount;
        CFDataRef endCode, startCode, idDelta, idRangeOffset, glyphIndexes;

        static uint16 getValue16 (CFDataRef data, const int index)
        {
            return CFSwapInt16BigToHost (*(UInt16*) (CFDataGetBytePtr (data) + index));
        }

        static uint32 getValue32 (CFDataRef data, const int index)
        {
            return CFSwapInt32BigToHost (*(UInt32*) (CFDataGetBytePtr (data) + index));
        }
    };

    ScopedPointer <CharToGlyphMapper> charToGlyphMapper;
#endif
};

const Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new MacTypeface (font);
}

//==============================================================================
const StringArray Font::findAllTypefaceNames() throw()
{
    StringArray names;

    const ScopedAutoReleasePool pool;

#if JUCE_IPHONE
    NSArray* fonts = [UIFont familyNames];
#else
    NSArray* fonts = [[NSFontManager sharedFontManager] availableFontFamilies];
#endif

    for (unsigned int i = 0; i < [fonts count]; ++i)
        names.add (nsStringToJuce ((NSString*) [fonts objectAtIndex: i]));

    names.sort (true);
    return names;
}

void Font::getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed) throw()
{
#if JUCE_IPHONE
    defaultSans  = "Helvetica";
    defaultSerif = "Times New Roman";
    defaultFixed = "Courier New";
#else
    defaultSans  = "Lucida Grande";
    defaultSerif = "Times New Roman";
    defaultFixed = "Monaco";
#endif
}

#endif
