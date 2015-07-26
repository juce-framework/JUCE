/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if (! defined (JUCE_CORETEXT_AVAILABLE)) \
     && (JUCE_IOS || (JUCE_MAC && MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4))
 #define JUCE_CORETEXT_AVAILABLE 1
#endif

const float referenceFontSize = 1024.0f;

#if JUCE_CORETEXT_AVAILABLE

#if JUCE_MAC && MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_5
extern "C"
{
    void CTRunGetAdvances (CTRunRef, CFRange, CGSize buffer[]);
    const CGSize* CTRunGetAdvancesPtr (CTRunRef);
}
#endif

static CTFontRef getCTFontFromTypeface (const Font& f);

namespace CoreTextTypeLayout
{
    static String findBestAvailableStyle (const Font& font, CGAffineTransform& requiredTransform)
    {
        const StringArray availableStyles (Font::findAllTypefaceStyles (font.getTypefaceName()));
        const String style (font.getTypefaceStyle());

        if (! availableStyles.contains (style))
        {
            if (font.isItalic())  // Fake-up an italic font if there isn't a real one.
                requiredTransform = CGAffineTransformMake (1.0f, 0, 0.25f, 1.0f, 0, 0);

            return availableStyles[0];
        }

        return style;
    }

    // Workaround for Apple bug in CTFontCreateWithFontDescriptor in Garageband/Logic on 10.6
   #if JUCE_MAC && ((! defined (MAC_OS_X_VERSION_10_7)) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7)
    static CTFontRef getFontWithTrait (CTFontRef ctFontRef, CTFontSymbolicTraits trait)
    {
        if (CTFontRef newFont = CTFontCreateCopyWithSymbolicTraits (ctFontRef, 0.0f, nullptr, trait, trait))
        {
            CFRelease (ctFontRef);
            return newFont;
        }

        return ctFontRef;
    }

    static CTFontRef useStyleFallbackIfNecessary (CTFontRef ctFontRef, CFStringRef cfFontFamily,
                                                  const float fontSizePoints, const Font& font)
    {
        CFStringRef cfActualFontFamily = (CFStringRef) CTFontCopyAttribute (ctFontRef, kCTFontFamilyNameAttribute);

        if (CFStringCompare (cfFontFamily, cfActualFontFamily, 0) != kCFCompareEqualTo)
        {
            CFRelease (ctFontRef);
            ctFontRef = CTFontCreateWithName (cfFontFamily, fontSizePoints, nullptr);

            if (font.isItalic())   ctFontRef = getFontWithTrait (ctFontRef, kCTFontItalicTrait);
            if (font.isBold())     ctFontRef = getFontWithTrait (ctFontRef, kCTFontBoldTrait);
        }

        CFRelease (cfActualFontFamily);
        return ctFontRef;
    }
   #endif

    static float getFontTotalHeight (CTFontRef font)
    {
        return std::abs ((float) CTFontGetAscent (font)) + std::abs ((float) CTFontGetDescent (font));
    }

    static float getHeightToPointsFactor (CTFontRef font)
    {
        return referenceFontSize / getFontTotalHeight (font);
    }

    static CTFontRef getFontWithPointSize (CTFontRef font, float size)
    {
        CTFontRef newFont = CTFontCreateCopyWithAttributes (font, size, nullptr, nullptr);
        CFRelease (font);
        return newFont;
    }

    static CTFontRef createCTFont (const Font& font, const float fontSizePoints, CGAffineTransform& transformRequired)
    {
        CFStringRef cfFontFamily = FontStyleHelpers::getConcreteFamilyName (font).toCFString();
        CFStringRef cfFontStyle = findBestAvailableStyle (font, transformRequired).toCFString();
        CFStringRef keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
        CFTypeRef values[] = { cfFontFamily, cfFontStyle };

        CFDictionaryRef fontDescAttributes = CFDictionaryCreate (nullptr, (const void**) &keys,
                                                                 (const void**) &values,
                                                                 numElementsInArray (keys),
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
        CFRelease (cfFontStyle);

        CTFontDescriptorRef ctFontDescRef = CTFontDescriptorCreateWithAttributes (fontDescAttributes);
        CFRelease (fontDescAttributes);

        CTFontRef ctFontRef = CTFontCreateWithFontDescriptor (ctFontDescRef, fontSizePoints, nullptr);
        CFRelease (ctFontDescRef);

       #if JUCE_MAC && ((! defined (MAC_OS_X_VERSION_10_7)) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7)
        ctFontRef = useStyleFallbackIfNecessary (ctFontRef, cfFontFamily, fontSizePoints, font);
       #endif

        CFRelease (cfFontFamily);

        return ctFontRef;
    }

    //==============================================================================
    struct Advances
    {
        Advances (CTRunRef run, const CFIndex numGlyphs)
            : advances (CTRunGetAdvancesPtr (run))
        {
            if (advances == nullptr)
            {
                local.malloc ((size_t) numGlyphs);
                CTRunGetAdvances (run, CFRangeMake (0, 0), local);
                advances = local;
            }
        }

        const CGSize* advances;
        HeapBlock<CGSize> local;
    };

    struct Glyphs
    {
        Glyphs (CTRunRef run, const size_t numGlyphs)
            : glyphs (CTRunGetGlyphsPtr (run))
        {
            if (glyphs == nullptr)
            {
                local.malloc (numGlyphs);
                CTRunGetGlyphs (run, CFRangeMake (0, 0), local);
                glyphs = local;
            }
        }

        const CGGlyph* glyphs;
        HeapBlock<CGGlyph> local;
    };

    struct Positions
    {
        Positions (CTRunRef run, const size_t numGlyphs)
            : points (CTRunGetPositionsPtr (run))
        {
            if (points == nullptr)
            {
                local.malloc (numGlyphs);
                CTRunGetPositions (run, CFRangeMake (0, 0), local);
                points = local;
            }
        }

        const CGPoint* points;
        HeapBlock<CGPoint> local;
    };

    struct LineInfo
    {
        LineInfo (CTFrameRef frame, CTLineRef line, CFIndex lineIndex)
        {
            CTFrameGetLineOrigins (frame, CFRangeMake (lineIndex, 1), &origin);
            CTLineGetTypographicBounds (line, &ascent,  &descent, &leading);
        }

        CGPoint origin;
        CGFloat ascent, descent, leading;
    };

    static CTFontRef getOrCreateFont (const Font& f)
    {
        if (CTFontRef ctf = getCTFontFromTypeface (f))
        {
            CFRetain (ctf);
            return ctf;
        }

        CGAffineTransform transform;
        return createCTFont (f, referenceFontSize, transform);
    }

    //==============================================================================
    static CFAttributedStringRef createCFAttributedString (const AttributedString& text)
    {
       #if JUCE_IOS
        CGColorSpaceRef rgbColourSpace = CGColorSpaceCreateDeviceRGB();
       #endif

        CFStringRef cfText = text.getText().toCFString();
        CFMutableAttributedStringRef attribString = CFAttributedStringCreateMutable (kCFAllocatorDefault, 0);
        CFAttributedStringReplaceString (attribString, CFRangeMake(0, 0), cfText);
        CFRelease (cfText);

        const int numCharacterAttributes = text.getNumAttributes();

        for (int i = 0; i < numCharacterAttributes; ++i)
        {
            const AttributedString::Attribute& attr = *text.getAttribute (i);

            if (attr.range.getStart() > CFAttributedStringGetLength (attribString))
                continue;

            Range<int> range (attr.range);
            range.setEnd (jmin (range.getEnd(), (int) CFAttributedStringGetLength (attribString)));

            if (const Font* const f = attr.getFont())
            {
                if (CTFontRef ctFontRef = getOrCreateFont (*f))
                {
                    ctFontRef = getFontWithPointSize (ctFontRef, f->getHeight() * getHeightToPointsFactor (ctFontRef));

                    CFAttributedStringSetAttribute (attribString, CFRangeMake (range.getStart(), range.getLength()),
                                                    kCTFontAttributeName, ctFontRef);
                    CFRelease (ctFontRef);
                }
            }

            if (const Colour* const col = attr.getColour())
            {
               #if JUCE_IOS
                const CGFloat components[] = { col->getFloatRed(),
                                               col->getFloatGreen(),
                                               col->getFloatBlue(),
                                               col->getFloatAlpha() };
                CGColorRef colour = CGColorCreate (rgbColourSpace, components);
               #else
                CGColorRef colour = CGColorCreateGenericRGB (col->getFloatRed(),
                                                             col->getFloatGreen(),
                                                             col->getFloatBlue(),
                                                             col->getFloatAlpha());
               #endif

                CFAttributedStringSetAttribute (attribString,
                                                CFRangeMake (range.getStart(), range.getLength()),
                                                kCTForegroundColorAttributeName, colour);
                CGColorRelease (colour);
            }
        }

        // Paragraph Attributes
        CTTextAlignment ctTextAlignment = kCTLeftTextAlignment;
        CTLineBreakMode ctLineBreakMode = kCTLineBreakByWordWrapping;
        const CGFloat ctLineSpacing = text.getLineSpacing();

        switch (text.getJustification().getOnlyHorizontalFlags())
        {
            case Justification::left:                   break;
            case Justification::right:                  ctTextAlignment = kCTRightTextAlignment; break;
            case Justification::horizontallyCentred:    ctTextAlignment = kCTCenterTextAlignment; break;
            case Justification::horizontallyJustified:  ctTextAlignment = kCTJustifiedTextAlignment; break;
            default:                                    jassertfalse; break; // Illegal justification flags
        }

        switch (text.getWordWrap())
        {
            case AttributedString::byWord:      break;
            case AttributedString::none:        ctLineBreakMode = kCTLineBreakByClipping; break;
            case AttributedString::byChar:      ctLineBreakMode = kCTLineBreakByCharWrapping; break;
            default: break;
        }

        CTParagraphStyleSetting settings[] =
        {
            { kCTParagraphStyleSpecifierAlignment,              sizeof (CTTextAlignment), &ctTextAlignment },
            { kCTParagraphStyleSpecifierLineBreakMode,          sizeof (CTLineBreakMode), &ctLineBreakMode },

           #if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
            { kCTParagraphStyleSpecifierLineSpacingAdjustment,  sizeof (CGFloat),         &ctLineSpacing }
           #else
            { kCTParagraphStyleSpecifierLineSpacing,            sizeof (CGFloat),         &ctLineSpacing }
           #endif
        };

        CTParagraphStyleRef ctParagraphStyleRef = CTParagraphStyleCreate (settings, (size_t) numElementsInArray (settings));
        CFAttributedStringSetAttribute (attribString, CFRangeMake (0, CFAttributedStringGetLength (attribString)),
                                        kCTParagraphStyleAttributeName, ctParagraphStyleRef);
        CFRelease (ctParagraphStyleRef);
       #if JUCE_IOS
        CGColorSpaceRelease (rgbColourSpace);
       #endif
        return attribString;
    }

    static CTFrameRef createCTFrame (const AttributedString& text, CGRect bounds)
    {
        CFAttributedStringRef attribString = createCFAttributedString (text);
        CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString (attribString);
        CFRelease (attribString);

        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddRect (path, nullptr, bounds);

        CTFrameRef frame = CTFramesetterCreateFrame (framesetter, CFRangeMake (0, 0), path, nullptr);
        CFRelease (framesetter);
        CGPathRelease (path);

        return frame;
    }

    static Range<float> getLineVerticalRange (CTFrameRef frame, CFArrayRef lines, int lineIndex)
    {
        LineInfo info (frame, (CTLineRef) CFArrayGetValueAtIndex (lines, lineIndex), lineIndex);
        return Range<float> ((float) (info.origin.y - info.descent),
                             (float) (info.origin.y + info.ascent));
    }

    static float findCTFrameHeight (CTFrameRef frame)
    {
        CFArrayRef lines = CTFrameGetLines (frame);
        const CFIndex numLines = CFArrayGetCount (lines);

        if (numLines == 0)
            return 0;

        Range<float> range (getLineVerticalRange (frame, lines, 0));

        if (numLines > 1)
            range = range.getUnionWith (getLineVerticalRange (frame, lines, (int) numLines - 1));

        return range.getLength();
    }

    static void drawToCGContext (const AttributedString& text, const Rectangle<float>& area,
                                 const CGContextRef& context, const float flipHeight)
    {
        CTFrameRef frame = createCTFrame (text, CGRectMake ((CGFloat) area.getX(), flipHeight - (CGFloat) area.getBottom(),
                                                            (CGFloat) area.getWidth(), (CGFloat) area.getHeight()));

        const int verticalJustification = text.getJustification().getOnlyVerticalFlags();

        if (verticalJustification == Justification::verticallyCentred
             || verticalJustification == Justification::bottom)
        {
            float adjust = area.getHeight() - findCTFrameHeight (frame);

            if (verticalJustification == Justification::verticallyCentred)
                adjust *= 0.5f;

            CGContextSaveGState (context);
            CGContextTranslateCTM (context, 0, -adjust);
            CTFrameDraw (frame, context);
            CGContextRestoreGState (context);
        }
        else
        {
            CTFrameDraw (frame, context);
        }

        CFRelease (frame);
    }

    static void createLayout (TextLayout& glyphLayout, const AttributedString& text)
    {
        const CGFloat boundsHeight = glyphLayout.getHeight();
        CTFrameRef frame = createCTFrame (text, CGRectMake (0, 0, glyphLayout.getWidth(), boundsHeight));

        CFArrayRef lines = CTFrameGetLines (frame);
        const CFIndex numLines = CFArrayGetCount (lines);

        glyphLayout.ensureStorageAllocated ((int) numLines);

        for (CFIndex i = 0; i < numLines; ++i)
        {
            CTLineRef line = (CTLineRef) CFArrayGetValueAtIndex (lines, i);

            CFArrayRef runs = CTLineGetGlyphRuns (line);
            const CFIndex numRuns = CFArrayGetCount (runs);

            const CFRange cfrlineStringRange = CTLineGetStringRange (line);
            const CFIndex lineStringEnd = cfrlineStringRange.location + cfrlineStringRange.length - 1;
            const Range<int> lineStringRange ((int) cfrlineStringRange.location, (int) lineStringEnd);

            LineInfo lineInfo (frame, line, i);

            TextLayout::Line* const glyphLine = new TextLayout::Line (lineStringRange,
                                                                      Point<float> ((float) lineInfo.origin.x,
                                                                                    (float) (boundsHeight - lineInfo.origin.y)),
                                                                      (float) lineInfo.ascent,
                                                                      (float) lineInfo.descent,
                                                                      (float) lineInfo.leading,
                                                                      (int) numRuns);
            glyphLayout.addLine (glyphLine);

            for (CFIndex j = 0; j < numRuns; ++j)
            {
                CTRunRef run = (CTRunRef) CFArrayGetValueAtIndex (runs, j);
                const CFIndex numGlyphs = CTRunGetGlyphCount (run);
                const CFRange runStringRange = CTRunGetStringRange (run);

                TextLayout::Run* const glyphRun = new TextLayout::Run (Range<int> ((int) runStringRange.location,
                                                                                   (int) (runStringRange.location + runStringRange.length - 1)),
                                                                       (int) numGlyphs);
                glyphLine->runs.add (glyphRun);

                CFDictionaryRef runAttributes = CTRunGetAttributes (run);

                CTFontRef ctRunFont;
                if (CFDictionaryGetValueIfPresent (runAttributes, kCTFontAttributeName, (const void **) &ctRunFont))
                {
                    CFStringRef cfsFontName = CTFontCopyPostScriptName (ctRunFont);
                    CTFontRef ctFontRef = CTFontCreateWithName (cfsFontName, referenceFontSize, nullptr);
                    CFRelease (cfsFontName);

                    const float fontHeightToPointsFactor = getHeightToPointsFactor (ctFontRef);
                    CFRelease (ctFontRef);

                    CFStringRef cfsFontFamily = (CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontFamilyNameAttribute);
                    CFStringRef cfsFontStyle  = (CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontStyleNameAttribute);

                    glyphRun->font = Font (String::fromCFString (cfsFontFamily),
                                           String::fromCFString (cfsFontStyle),
                                           (float) (CTFontGetSize (ctRunFont) / fontHeightToPointsFactor));

                    CFRelease (cfsFontStyle);
                    CFRelease (cfsFontFamily);
                }

                CGColorRef cgRunColor;
                if (CFDictionaryGetValueIfPresent (runAttributes, kCTForegroundColorAttributeName, (const void**) &cgRunColor)
                     && CGColorGetNumberOfComponents (cgRunColor) == 4)
                {
                    const CGFloat* const components = CGColorGetComponents (cgRunColor);

                    glyphRun->colour = Colour::fromFloatRGBA ((float) components[0],
                                                              (float) components[1],
                                                              (float) components[2],
                                                              (float) components[3]);
                }

                const Glyphs glyphs (run, (size_t) numGlyphs);
                const Advances advances (run, numGlyphs);
                const Positions positions (run, (size_t) numGlyphs);

                for (CFIndex k = 0; k < numGlyphs; ++k)
                    glyphRun->glyphs.add (TextLayout::Glyph (glyphs.glyphs[k], Point<float> ((float) positions.points[k].x,
                                                                                             (float) positions.points[k].y),
                                                             (float) advances.advances[k].width));
            }
        }

        CFRelease (frame);
    }
}


//==============================================================================
class OSXTypeface  : public Typeface
{
public:
    OSXTypeface (const Font& font)
        : Typeface (font.getTypefaceName(),
                    font.getTypefaceStyle()),
          fontRef (nullptr),
          ctFontRef (nullptr),
          fontHeightToPointsFactor (1.0f),
          renderingTransform (CGAffineTransformIdentity),
          isMemoryFont (false),
          attributedStringAtts (nullptr),
          ascent (0.0f),
          unitsToHeightScaleFactor (0.0f)
    {
        ctFontRef = CoreTextTypeLayout::createCTFont (font, referenceFontSize, renderingTransform);

        if (ctFontRef != nullptr)
        {
            fontRef = CTFontCopyGraphicsFont (ctFontRef, nullptr);
            initialiseMetrics();
        }
    }

    OSXTypeface (const void* data, size_t dataSize)
        : Typeface (String(), String()),
          fontRef (nullptr),
          ctFontRef (nullptr),
          fontHeightToPointsFactor (1.0f),
          renderingTransform (CGAffineTransformIdentity),
          isMemoryFont (true),
          attributedStringAtts (nullptr),
          ascent (0.0f),
          unitsToHeightScaleFactor (0.0f)
    {
        CFDataRef cfData = CFDataCreate (kCFAllocatorDefault, (const UInt8*) data, (CFIndex) dataSize);
        CGDataProviderRef provider = CGDataProviderCreateWithCFData (cfData);
        CFRelease (cfData);

        fontRef = CGFontCreateWithDataProvider (provider);
        CGDataProviderRelease (provider);

        if (fontRef != nullptr)
        {
            ctFontRef = CTFontCreateWithGraphicsFont (fontRef, referenceFontSize, nullptr, nullptr);

            if (ctFontRef != nullptr)
            {
                if (CFStringRef fontName = CTFontCopyName (ctFontRef, kCTFontFamilyNameKey))
                {
                    name = String::fromCFString (fontName);
                    CFRelease (fontName);
                }

                if (CFStringRef fontStyle = CTFontCopyName (ctFontRef, kCTFontStyleNameKey))
                {
                    style = String::fromCFString (fontStyle);
                    CFRelease (fontStyle);
                }

                initialiseMetrics();
            }
        }
    }

    void initialiseMetrics()
    {
        const float ctAscent  = std::abs ((float) CTFontGetAscent (ctFontRef));
        const float ctDescent = std::abs ((float) CTFontGetDescent (ctFontRef));
        const float ctTotalHeight = ctAscent + ctDescent;

        ascent = ctAscent / ctTotalHeight;
        unitsToHeightScaleFactor = 1.0f / ctTotalHeight;
        pathTransform = AffineTransform::identity.scale (unitsToHeightScaleFactor);

        fontHeightToPointsFactor = referenceFontSize / ctTotalHeight;

        const short zero = 0;
        CFNumberRef numberRef = CFNumberCreate (0, kCFNumberShortType, &zero);

        CFStringRef keys[] = { kCTFontAttributeName, kCTLigatureAttributeName };
        CFTypeRef values[] = { ctFontRef, numberRef };
        attributedStringAtts = CFDictionaryCreate (nullptr, (const void**) &keys, (const void**) &values, numElementsInArray (keys),
                                                   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFRelease (numberRef);
    }

    ~OSXTypeface()
    {
        if (attributedStringAtts != nullptr)
            CFRelease (attributedStringAtts);

        if (fontRef != nullptr)
            CGFontRelease (fontRef);

        if (ctFontRef != nullptr)
            CFRelease (ctFontRef);
    }

    float getAscent() const override                 { return ascent; }
    float getDescent() const override                { return 1.0f - ascent; }
    float getHeightToPointsFactor() const override   { return fontHeightToPointsFactor; }

    float getStringWidth (const String& text) override
    {
        float x = 0;

        if (ctFontRef != nullptr && text.isNotEmpty())
        {
            CFStringRef cfText = text.toCFString();
            CFAttributedStringRef attribString = CFAttributedStringCreate (kCFAllocatorDefault, cfText, attributedStringAtts);
            CFRelease (cfText);

            CTLineRef line = CTLineCreateWithAttributedString (attribString);
            CFArrayRef runArray = CTLineGetGlyphRuns (line);

            for (CFIndex i = 0; i < CFArrayGetCount (runArray); ++i)
            {
                CTRunRef run = (CTRunRef) CFArrayGetValueAtIndex (runArray, i);
                CFIndex length = CTRunGetGlyphCount (run);

                const CoreTextTypeLayout::Advances advances (run, length);

                for (int j = 0; j < length; ++j)
                    x += (float) advances.advances[j].width;
            }

            CFRelease (line);
            CFRelease (attribString);

            x *= unitsToHeightScaleFactor;
        }

        return x;
    }

    void getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array <float>& xOffsets) override
    {
        xOffsets.add (0);

        if (ctFontRef != nullptr && text.isNotEmpty())
        {
            float x = 0;

            CFStringRef cfText = text.toCFString();
            CFAttributedStringRef attribString = CFAttributedStringCreate (kCFAllocatorDefault, cfText, attributedStringAtts);
            CFRelease (cfText);

            CTLineRef line = CTLineCreateWithAttributedString (attribString);
            CFArrayRef runArray = CTLineGetGlyphRuns (line);

            for (CFIndex i = 0; i < CFArrayGetCount (runArray); ++i)
            {
                CTRunRef run = (CTRunRef) CFArrayGetValueAtIndex (runArray, i);
                CFIndex length = CTRunGetGlyphCount (run);

                const CoreTextTypeLayout::Advances advances (run, length);
                const CoreTextTypeLayout::Glyphs glyphs (run, (size_t) length);

                for (int j = 0; j < length; ++j)
                {
                    x += (float) advances.advances[j].width;
                    xOffsets.add (x * unitsToHeightScaleFactor);
                    resultGlyphs.add (glyphs.glyphs[j]);
                }
            }

            CFRelease (line);
            CFRelease (attribString);
        }
    }

    bool getOutlineForGlyph (int glyphNumber, Path& path) override
    {
        jassert (path.isEmpty());  // we might need to apply a transform to the path, so this must be empty

        CGPathRef pathRef = CTFontCreatePathForGlyph (ctFontRef, (CGGlyph) glyphNumber, &renderingTransform);
        if (pathRef == 0)
            return false;

        CGPathApply (pathRef, &path, pathApplier);
        CFRelease (pathRef);

        if (! pathTransform.isIdentity())
            path.applyTransform (pathTransform);

        return true;
    }

    //==============================================================================
    CGFontRef fontRef;
    CTFontRef ctFontRef;

    float fontHeightToPointsFactor;
    CGAffineTransform renderingTransform;

    bool isMemoryFont;

private:
    CFDictionaryRef attributedStringAtts;
    float ascent, unitsToHeightScaleFactor;
    AffineTransform pathTransform;

    static void pathApplier (void* info, const CGPathElement* const element)
    {
        Path& path = *static_cast<Path*> (info);
        const CGPoint* const p = element->points;

        switch (element->type)
        {
            case kCGPathElementMoveToPoint:         path.startNewSubPath ((float) p[0].x, (float) -p[0].y); break;
            case kCGPathElementAddLineToPoint:      path.lineTo          ((float) p[0].x, (float) -p[0].y); break;
            case kCGPathElementAddQuadCurveToPoint: path.quadraticTo     ((float) p[0].x, (float) -p[0].y,
                                                                          (float) p[1].x, (float) -p[1].y); break;
            case kCGPathElementAddCurveToPoint:     path.cubicTo         ((float) p[0].x, (float) -p[0].y,
                                                                          (float) p[1].x, (float) -p[1].y,
                                                                          (float) p[2].x, (float) -p[2].y); break;
            case kCGPathElementCloseSubpath:        path.closeSubPath(); break;
            default:                                jassertfalse; break;
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSXTypeface)
};

CTFontRef getCTFontFromTypeface (const Font& f)
{
    if (OSXTypeface* tf = dynamic_cast<OSXTypeface*> (f.getTypeface()))
        return tf->ctFontRef;

    return 0;
}

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

   #if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5 && ! JUCE_IOS
    // CTFontManager only exists on OS X 10.6 and later, it does not exist on iOS
    CFArrayRef fontFamilyArray = CTFontManagerCopyAvailableFontFamilyNames();

    for (CFIndex i = 0; i < CFArrayGetCount (fontFamilyArray); ++i)
    {
        const String family (String::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (fontFamilyArray, i)));

        if (! family.startsWithChar ('.')) // ignore fonts that start with a '.'
            names.addIfNotAlreadyThere (family);
    }

    CFRelease (fontFamilyArray);
   #else
    CTFontCollectionRef fontCollectionRef = CTFontCollectionCreateFromAvailableFonts (nullptr);
    CFArrayRef fontDescriptorArray = CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef);
    CFRelease (fontCollectionRef);

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray); ++i)
    {
        CTFontDescriptorRef ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray, i);
        CFStringRef cfsFontFamily = (CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute);

        names.addIfNotAlreadyThere (String::fromCFString (cfsFontFamily));

        CFRelease (cfsFontFamily);
    }

    CFRelease (fontDescriptorArray);
   #endif

    names.sort (true);
    return names;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    StringArray results;

    CFStringRef cfsFontFamily = family.toCFString();
    CFStringRef keys[] = { kCTFontFamilyNameAttribute };
    CFTypeRef values[] = { cfsFontFamily };

    CFDictionaryRef fontDescAttributes = CFDictionaryCreate (nullptr, (const void**) &keys, (const void**) &values, numElementsInArray (keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease (cfsFontFamily);

    CTFontDescriptorRef ctFontDescRef = CTFontDescriptorCreateWithAttributes (fontDescAttributes);
    CFRelease (fontDescAttributes);

    CFArrayRef fontFamilyArray = CFArrayCreate (kCFAllocatorDefault, (const void**) &ctFontDescRef, 1, &kCFTypeArrayCallBacks);
    CFRelease (ctFontDescRef);

    CTFontCollectionRef fontCollectionRef = CTFontCollectionCreateWithFontDescriptors (fontFamilyArray, nullptr);
    CFRelease (fontFamilyArray);

    CFArrayRef fontDescriptorArray = CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef);
    CFRelease (fontCollectionRef);

    if (fontDescriptorArray != nullptr)
    {
        for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray); ++i)
        {
            CTFontDescriptorRef ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray, i);
            CFStringRef cfsFontStyle = (CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontStyleNameAttribute);
            results.add (String::fromCFString (cfsFontStyle));
            CFRelease (cfsFontStyle);
        }

        CFRelease (fontDescriptorArray);
    }

    return results;
}

#else

//==============================================================================
// The stuff that follows is a mash-up that supports pre-OSX 10.5 APIs.
// (Hopefully all of this can be ditched at some point in the future).

//==============================================================================
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
  #define SUPPORT_10_4_FONTS 1
  #define NEW_CGFONT_FUNCTIONS_UNAVAILABLE (CGFontCreateWithFontName == 0)

  #if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
    #define SUPPORT_ONLY_10_4_FONTS 1
  #endif

  } // (juce namespace)

  @interface NSFont (PrivateHack)
    - (NSGlyph) _defaultGlyphForChar: (unichar) theChar;
  @end

  namespace juce
  {
#endif

//==============================================================================
class OSXTypeface  : public Typeface
{
public:
    OSXTypeface (const Font& font)
        : Typeface (font.getTypefaceName(), font.getTypefaceStyle())
    {
        JUCE_AUTORELEASEPOOL
        {
            renderingTransform = CGAffineTransformIdentity;

            NSDictionary* nsDict = [NSDictionary dictionaryWithObjectsAndKeys:
                                       juceStringToNS (name), NSFontFamilyAttribute,
                                       juceStringToNS (style), NSFontFaceAttribute, nil];

            NSFontDescriptor* nsFontDesc = [NSFontDescriptor fontDescriptorWithFontAttributes: nsDict];
            nsFont = [NSFont fontWithDescriptor: nsFontDesc size: referenceFontSize];

            [nsFont retain];

          #if SUPPORT_ONLY_10_4_FONTS
            initWithATSFont();
          #else
           #if SUPPORT_10_4_FONTS
            if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
            {
                initWithATSFont();
            }
            else
           #endif
            {
                fontRef = CGFontCreateWithFontName ((CFStringRef) [nsFont fontName]);

                const float absAscent = std::abs ((float) CGFontGetAscent (fontRef));
                const float totalHeight = absAscent + std::abs ((float) CGFontGetDescent (fontRef));

                ascent = absAscent / totalHeight;
                unitsToHeightScaleFactor = 1.0f / totalHeight;

                const float nsFontAscent  = std::abs ([nsFont ascender]);
                const float nsFontDescent = std::abs ([nsFont descender]);

                fontHeightToPointsFactor = referenceFontSize / (nsFontAscent + nsFontDescent);
           }
          #endif

            pathTransform = AffineTransform::identity.scale (unitsToHeightScaleFactor);
        }
    }

    ~OSXTypeface()
    {
       #if ! JUCE_IOS
        [nsFont release];
       #endif

        if (fontRef != 0)
            CGFontRelease (fontRef);
    }

   #if SUPPORT_10_4_FONTS
    void initWithATSFont()
    {
        ATSFontRef atsFont = ATSFontFindFromName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

        if (atsFont == 0)
            atsFont = ATSFontFindFromPostScriptName ((CFStringRef) [nsFont fontName], kATSOptionFlagsDefault);

        fontRef = CGFontCreateWithPlatformFont (&atsFont);

        const float absAscent = std::abs ([nsFont ascender]);
        const float absDescent = std::abs ([nsFont descender]);
        const float totalHeight = absAscent + absDescent;

        unitsToHeightScaleFactor = 1.0f / totalHeight;
        fontHeightToPointsFactor = referenceFontSize / totalHeight;
        ascent = absAscent / totalHeight;
    }
   #endif


    float getAscent() const override                 { return ascent; }
    float getDescent() const override                { return 1.0f - ascent; }
    float getHeightToPointsFactor() const override   { return fontHeightToPointsFactor; }

    float getStringWidth (const String& text) override
    {
        if (fontRef == 0 || text.isEmpty())
            return 0;

        const int length = text.length();
        HeapBlock <CGGlyph> glyphs;
        createGlyphsForString (text.getCharPointer(), length, glyphs);

        float x = 0;

#if SUPPORT_ONLY_10_4_FONTS
        HeapBlock <NSSize> advances (length);
        [nsFont getAdvancements: advances forGlyphs: reinterpret_cast<NSGlyph*> (glyphs.getData()) count: length];

        for (int i = 0; i < length; ++i)
            x += advances[i].width;
#else
       #if SUPPORT_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
        {
            HeapBlock <NSSize> advances (length);
            [nsFont getAdvancements: advances forGlyphs: reinterpret_cast<NSGlyph*> (glyphs.getData()) count: length];

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

    void getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets) override
    {
        xOffsets.add (0);

        if (fontRef == 0 || text.isEmpty())
            return;

        const int length = text.length();
        HeapBlock <CGGlyph> glyphs;
        createGlyphsForString (text.getCharPointer(), length, glyphs);

#if SUPPORT_ONLY_10_4_FONTS
        HeapBlock <NSSize> advances (length);
        [nsFont getAdvancements: advances forGlyphs: reinterpret_cast <NSGlyph*> (glyphs.getData()) count: length];

        int x = 0;
        for (int i = 0; i < length; ++i)
        {
            x += advances[i].width;
            xOffsets.add (x * unitsToHeightScaleFactor);
            resultGlyphs.add (reinterpret_cast <NSGlyph*> (glyphs.getData())[i]);
        }

#else
       #if SUPPORT_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
        {
            HeapBlock <NSSize> advances (length);
            NSGlyph* const nsGlyphs = reinterpret_cast<NSGlyph*> (glyphs.getData());
            [nsFont getAdvancements: advances forGlyphs: nsGlyphs count: length];

            float x = 0;
            for (int i = 0; i < length; ++i)
            {
                x += advances[i].width;
                xOffsets.add (x * unitsToHeightScaleFactor);
                resultGlyphs.add (nsGlyphs[i]);
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

    bool getOutlineForGlyph (int glyphNumber, Path& path) override
    {
       #if JUCE_IOS
        return false;
       #else
        if (nsFont == nil)
            return false;

        // we might need to apply a transform to the path, so it mustn't have anything else in it
        jassert (path.isEmpty());

        JUCE_AUTORELEASEPOOL
        {
            NSBezierPath* bez = [NSBezierPath bezierPath];
            [bez moveToPoint: NSMakePoint (0, 0)];
            [bez appendBezierPathWithGlyph: (NSGlyph) glyphNumber
                                    inFont: nsFont];

            for (int i = 0; i < [bez elementCount]; ++i)
            {
                NSPoint p[3];
                switch ([bez elementAtIndex: i associatedPoints: p])
                {
                    case NSMoveToBezierPathElement:     path.startNewSubPath ((float) p[0].x, (float) -p[0].y); break;
                    case NSLineToBezierPathElement:     path.lineTo  ((float) p[0].x, (float) -p[0].y); break;
                    case NSCurveToBezierPathElement:    path.cubicTo ((float) p[0].x, (float) -p[0].y,
                                                                      (float) p[1].x, (float) -p[1].y,
                                                                      (float) p[2].x, (float) -p[2].y); break;
                    case NSClosePathBezierPathElement:  path.closeSubPath(); break;
                    default:                            jassertfalse; break;
                }
            }

            path.applyTransform (pathTransform);
        }
        return true;
       #endif
    }

    //==============================================================================
    CGFontRef fontRef;
    float fontHeightToPointsFactor;
    CGAffineTransform renderingTransform;

private:
    float ascent, unitsToHeightScaleFactor;

   #if ! JUCE_IOS
    NSFont* nsFont;
    AffineTransform pathTransform;
   #endif

    void createGlyphsForString (String::CharPointerType text, const int length, HeapBlock <CGGlyph>& glyphs)
    {
      #if SUPPORT_10_4_FONTS
       #if ! SUPPORT_ONLY_10_4_FONTS
        if (NEW_CGFONT_FUNCTIONS_UNAVAILABLE)
       #endif
        {
            glyphs.malloc (sizeof (NSGlyph) * length, 1);
            NSGlyph* const nsGlyphs = reinterpret_cast<NSGlyph*> (glyphs.getData());

            for (int i = 0; i < length; ++i)
                nsGlyphs[i] = (NSGlyph) [nsFont _defaultGlyphForChar: text.getAndAdvance()];

            return;
        }
      #endif

       #if ! SUPPORT_ONLY_10_4_FONTS
        if (charToGlyphMapper == nullptr)
            charToGlyphMapper = new CharToGlyphMapper (fontRef);

        glyphs.malloc (length);

        for (int i = 0; i < length; ++i)
            glyphs[i] = (CGGlyph) charToGlyphMapper->getGlyphForCharacter (text.getAndAdvance());
       #endif
    }

   #if ! SUPPORT_ONLY_10_4_FONTS
    // Reads a CGFontRef's character map table to convert unicode into glyph numbers
    class CharToGlyphMapper
    {
    public:
        CharToGlyphMapper (CGFontRef cgFontRef)
            : segCount (0), endCode (0), startCode (0), idDelta (0),
              idRangeOffset (0), glyphIndexes (0)
        {
            CFDataRef cmapTable = CGFontCopyTableForTag (cgFontRef, 'cmap');

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

                            endCode       = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + endCodeOffset, segCountX2);
                            startCode     = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + startCodeOffset, segCountX2);
                            idDelta       = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + idDeltaOffset, segCountX2);
                            idRangeOffset = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + idRangeOffsetOffset, segCountX2);
                            glyphIndexes  = CFDataCreate (kCFAllocatorDefault, CFDataGetBytePtr (cmapTable) + glyphIndexesOffset, offset + length - glyphIndexesOffset);
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

                    return getValue16 (glyphIndexes, 2 * ((rangeOffset / 2) + (c - start) - (segCount - i)));
                }
            }

            // If we failed to find it "properly", this dodgy fall-back seems to do the trick for most fonts!
            return jmax (-1, (int) c - 29);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSXTypeface)
};

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

    JUCE_AUTORELEASEPOOL
    {
       #if JUCE_IOS
        for (NSString* name in [UIFont familyNames])
       #else
        for (NSString* name in [[NSFontManager sharedFontManager] availableFontFamilies])
       #endif
            names.add (nsStringToJuce (name));

        names.sort (true);
    }

    return names;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    StringArray results;

    JUCE_AUTORELEASEPOOL
    {
        for (NSArray* style in [[NSFontManager sharedFontManager] availableMembersOfFontFamily: juceStringToNS (family)])
            results.add (nsStringToJuce ((NSString*) [style objectAtIndex: 1]));
    }

    return results;
}

#endif

//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new OSXTypeface (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t dataSize)
{
   #if JUCE_CORETEXT_AVAILABLE
    return new OSXTypeface (data, dataSize);
   #else
    jassertfalse; // You need CoreText enabled to use this feature!
    return nullptr;
   #endif
}

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not implemented on this platform
}

struct DefaultFontNames
{
    DefaultFontNames()
       #if JUCE_IOS
        : defaultSans  ("Helvetica"),
          defaultSerif ("Times New Roman"),
          defaultFixed ("Courier New"),
       #else
        : defaultSans  ("Lucida Grande"),
          defaultSerif ("Times New Roman"),
          defaultFixed ("Menlo"),
       #endif
          defaultFallback ("Arial Unicode MS")
    {
    }

    String defaultSans, defaultSerif, defaultFixed, defaultFallback;
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    Font newFont (font);
    const String& faceName = font.getTypefaceName();

    if (faceName == getDefaultSansSerifFontName())       newFont.setTypefaceName (defaultNames.defaultSans);
    else if (faceName == getDefaultSerifFontName())      newFont.setTypefaceName (defaultNames.defaultSerif);
    else if (faceName == getDefaultMonospacedFontName()) newFont.setTypefaceName (defaultNames.defaultFixed);

    if (font.getTypefaceStyle() == getDefaultStyle())
        newFont.setTypefaceStyle ("Regular");

    return Typeface::createSystemTypefaceFor (newFont);
}

#if JUCE_CORETEXT_AVAILABLE
static bool canAllTypefacesBeUsedInLayout (const AttributedString& text)
{
    const int numCharacterAttributes = text.getNumAttributes();

    for (int i = 0; i < numCharacterAttributes; ++i)
    {
        if (const Font* const f = text.getAttribute (i)->getFont())
        {
            if (OSXTypeface* tf = dynamic_cast<OSXTypeface*> (f->getTypeface()))
            {
                if (tf->isMemoryFont)
                    return false;
            }
            else if (dynamic_cast<CustomTypeface*> (f->getTypeface()) != nullptr)
            {
                return false;
            }
        }
    }

    return true;
}
#endif

bool TextLayout::createNativeLayout (const AttributedString& text)
{
   #if JUCE_CORETEXT_AVAILABLE
    // Seems to be an unfathomable bug in CoreText which prevents the layout working with
    // typefaces that were loaded from memory, so have to fallback if we hit any of those..
    if (canAllTypefacesBeUsedInLayout (text))
    {
        CoreTextTypeLayout::createLayout (*this, text);
        return true;
    }
   #endif

    (void) text;
    return false;
}
