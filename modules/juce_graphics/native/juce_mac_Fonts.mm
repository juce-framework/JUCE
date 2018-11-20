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

static constexpr float referenceFontSize = 1024.0f;

static CTFontRef getCTFontFromTypeface (const Font&);

namespace CoreTextTypeLayout
{
    static String findBestAvailableStyle (const Font& font, CGAffineTransform& requiredTransform)
    {
        auto availableStyles = Font::findAllTypefaceStyles (font.getTypefaceName());
        auto style = font.getTypefaceStyle();

        if (! availableStyles.contains (style))
        {
            if (font.isItalic())  // Fake-up an italic font if there isn't a real one.
                requiredTransform = CGAffineTransformMake (1.0f, 0, 0.25f, 1.0f, 0, 0);

            return availableStyles[0];
        }

        return style;
    }

    static float getFontTotalHeight (CTFontRef font)
    {
        return std::abs ((float) CTFontGetAscent (font))
             + std::abs ((float) CTFontGetDescent (font));
    }

    static float getHeightToPointsFactor (CTFontRef font)
    {
        return referenceFontSize / getFontTotalHeight (font);
    }

    static CTFontRef getFontWithPointSize (CTFontRef font, float size)
    {
        auto newFont = CTFontCreateCopyWithAttributes (font, size, nullptr, nullptr);
        CFRelease (font);
        return newFont;
    }

    static CTFontRef createCTFont (const Font& font, const float fontSizePoints, CGAffineTransform& transformRequired)
    {
        auto cfFontFamily = FontStyleHelpers::getConcreteFamilyName (font).toCFString();
        auto cfFontStyle = findBestAvailableStyle (font, transformRequired).toCFString();
        CFStringRef keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
        CFTypeRef values[] = { cfFontFamily, cfFontStyle };

        auto fontDescAttributes = CFDictionaryCreate (nullptr, (const void**) &keys,
                                                      (const void**) &values,
                                                      numElementsInArray (keys),
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);
        CFRelease (cfFontStyle);

        auto ctFontDescRef = CTFontDescriptorCreateWithAttributes (fontDescAttributes);
        CFRelease (fontDescAttributes);

        auto ctFontRef = CTFontCreateWithFontDescriptor (ctFontDescRef, fontSizePoints, nullptr);
        CFRelease (ctFontDescRef);
        CFRelease (cfFontFamily);

        return ctFontRef;
    }

    //==============================================================================
    struct Advances
    {
        Advances (CTRunRef run, CFIndex numGlyphs) : advances (CTRunGetAdvancesPtr (run))
        {
            if (advances == nullptr)
            {
                local.malloc (numGlyphs);
                CTRunGetAdvances (run, CFRangeMake (0, 0), local);
                advances = local;
            }
        }

        const CGSize* advances;
        HeapBlock<CGSize> local;
    };

    struct Glyphs
    {
        Glyphs (CTRunRef run, size_t numGlyphs) : glyphs (CTRunGetGlyphsPtr (run))
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
        Positions (CTRunRef run, size_t numGlyphs) : points (CTRunGetPositionsPtr (run))
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
        if (auto ctf = getCTFontFromTypeface (f))
        {
            CFRetain (ctf);
            return ctf;
        }

        CGAffineTransform transform;
        return createCTFont (f, referenceFontSize, transform);
    }

    //==============================================================================
    static CTTextAlignment getTextAlignment (const AttributedString& text)
    {
        switch (text.getJustification().getOnlyHorizontalFlags())
        {
           #if defined (MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8
            case Justification::right:                  return kCTTextAlignmentRight;
            case Justification::horizontallyCentred:    return kCTTextAlignmentCenter;
            case Justification::horizontallyJustified:  return kCTTextAlignmentJustified;
            default:                                    return kCTTextAlignmentLeft;
           #else
            case Justification::right:                  return kCTRightTextAlignment;
            case Justification::horizontallyCentred:    return kCTCenterTextAlignment;
            case Justification::horizontallyJustified:  return kCTJustifiedTextAlignment;
            default:                                    return kCTLeftTextAlignment;
           #endif
        }
    }

    static CTLineBreakMode getLineBreakMode (const AttributedString& text)
    {
        switch (text.getWordWrap())
        {
            case AttributedString::none:        return kCTLineBreakByClipping;
            case AttributedString::byChar:      return kCTLineBreakByCharWrapping;
            default:                            return kCTLineBreakByWordWrapping;
        }
    }

    static CTWritingDirection getWritingDirection (const AttributedString& text)
    {
        switch (text.getReadingDirection())
        {
            case AttributedString::rightToLeft:   return kCTWritingDirectionRightToLeft;
            case AttributedString::leftToRight:   return kCTWritingDirectionLeftToRight;
            default:                              return kCTWritingDirectionNatural;
        }
    }

    //==============================================================================
    static CFAttributedStringRef createCFAttributedString (const AttributedString& text)
    {
       #if JUCE_IOS
        auto rgbColourSpace = CGColorSpaceCreateDeviceRGB();
       #endif

        auto attribString = CFAttributedStringCreateMutable (kCFAllocatorDefault, 0);
        auto cfText = text.getText().toCFString();
        CFAttributedStringReplaceString (attribString, CFRangeMake (0, 0), cfText);
        CFRelease (cfText);

        auto numCharacterAttributes = text.getNumAttributes();
        auto attribStringLen = CFAttributedStringGetLength (attribString);

        for (int i = 0; i < numCharacterAttributes; ++i)
        {
            auto& attr = text.getAttribute (i);
            auto rangeStart = attr.range.getStart();

            if (rangeStart >= attribStringLen)
                continue;

            auto range = CFRangeMake (rangeStart, jmin (attr.range.getEnd(), (int) attribStringLen) - rangeStart);

            if (auto ctFontRef = getOrCreateFont (attr.font))
            {
                ctFontRef = getFontWithPointSize (ctFontRef, attr.font.getHeight() * getHeightToPointsFactor (ctFontRef));
                CFAttributedStringSetAttribute (attribString, range, kCTFontAttributeName, ctFontRef);

                auto extraKerning = attr.font.getExtraKerningFactor();

                if (extraKerning != 0)
                {
                    extraKerning *= attr.font.getHeight();

                    auto numberRef = CFNumberCreate (0, kCFNumberFloatType, &extraKerning);
                    CFAttributedStringSetAttribute (attribString, range, kCTKernAttributeName, numberRef);
                    CFRelease (numberRef);
                }

                CFRelease (ctFontRef);
            }

            {
                auto col = attr.colour;

               #if JUCE_IOS
                const CGFloat components[] = { col.getFloatRed(),
                                               col.getFloatGreen(),
                                               col.getFloatBlue(),
                                               col.getFloatAlpha() };
                auto colour = CGColorCreate (rgbColourSpace, components);
               #else
                auto colour = CGColorCreateGenericRGB (col.getFloatRed(),
                                                       col.getFloatGreen(),
                                                       col.getFloatBlue(),
                                                       col.getFloatAlpha());
               #endif

                CFAttributedStringSetAttribute (attribString, range, kCTForegroundColorAttributeName, colour);
                CGColorRelease (colour);
            }
        }

        // Paragraph Attributes
        auto ctTextAlignment = getTextAlignment (text);
        auto ctLineBreakMode = getLineBreakMode (text);
        auto ctWritingDirection = getWritingDirection (text);
        CGFloat ctLineSpacing = text.getLineSpacing();

        CTParagraphStyleSetting settings[] =
        {
            { kCTParagraphStyleSpecifierAlignment,              sizeof (CTTextAlignment), &ctTextAlignment },
            { kCTParagraphStyleSpecifierLineBreakMode,          sizeof (CTLineBreakMode), &ctLineBreakMode },
            { kCTParagraphStyleSpecifierBaseWritingDirection,   sizeof (CTWritingDirection), &ctWritingDirection},
            { kCTParagraphStyleSpecifierLineSpacingAdjustment,  sizeof (CGFloat),         &ctLineSpacing }
        };

        auto ctParagraphStyleRef = CTParagraphStyleCreate (settings, (size_t) numElementsInArray (settings));
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
        auto attribString = createCFAttributedString (text);
        auto framesetter = CTFramesetterCreateWithAttributedString (attribString);
        CFRelease (attribString);

        auto path = CGPathCreateMutable();
        CGPathAddRect (path, nullptr, bounds);

        auto frame = CTFramesetterCreateFrame (framesetter, CFRangeMake (0, 0), path, nullptr);
        CFRelease (framesetter);
        CGPathRelease (path);

        return frame;
    }

    static Range<float> getLineVerticalRange (CTFrameRef frame, CFArrayRef lines, int lineIndex)
    {
        LineInfo info (frame, (CTLineRef) CFArrayGetValueAtIndex (lines, lineIndex), lineIndex);

        return { (float) (info.origin.y - info.descent),
                 (float) (info.origin.y + info.ascent) };
    }

    static float findCTFrameHeight (CTFrameRef frame)
    {
        auto lines = CTFrameGetLines (frame);
        auto numLines = CFArrayGetCount (lines);

        if (numLines == 0)
            return 0;

        auto range = getLineVerticalRange (frame, lines, 0);

        if (numLines > 1)
            range = range.getUnionWith (getLineVerticalRange (frame, lines, (int) numLines - 1));

        return range.getLength();
    }

    static void drawToCGContext (const AttributedString& text, const Rectangle<float>& area,
                                 const CGContextRef& context, float flipHeight)
    {
        Rectangle<float> ctFrameArea;
        auto verticalJustification = text.getJustification().getOnlyVerticalFlags();

        // Ugly hack to fix a bug in OS X Sierra where the CTFrame needs to be slightly
        // larger than the font height - otherwise the CTFrame will be invalid
        if (verticalJustification == Justification::verticallyCentred)
            ctFrameArea = area.withSizeKeepingCentre (area.getWidth(), area.getHeight() * 1.1f);
        else if (verticalJustification == Justification::bottom)
            ctFrameArea = area.withTop (area.getY() - (area.getHeight() * 0.1f));
        else
            ctFrameArea = area.withHeight (area.getHeight() * 1.1f);

        auto frame = createCTFrame (text, CGRectMake ((CGFloat) ctFrameArea.getX(), flipHeight - (CGFloat) ctFrameArea.getBottom(),
                                                      (CGFloat) ctFrameArea.getWidth(), (CGFloat) ctFrameArea.getHeight()));

        if (verticalJustification == Justification::verticallyCentred
             || verticalJustification == Justification::bottom)
        {
            auto adjust = ctFrameArea.getHeight() - findCTFrameHeight (frame);

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
        auto boundsHeight = glyphLayout.getHeight();
        auto frame = createCTFrame (text, CGRectMake (0, 0, glyphLayout.getWidth(), boundsHeight));
        auto lines = CTFrameGetLines (frame);
        auto numLines = CFArrayGetCount (lines);

        glyphLayout.ensureStorageAllocated ((int) numLines);

        for (CFIndex i = 0; i < numLines; ++i)
        {
            auto line = (CTLineRef) CFArrayGetValueAtIndex (lines, i);
            auto runs = CTLineGetGlyphRuns (line);
            auto numRuns = CFArrayGetCount (runs);

            auto cfrlineStringRange = CTLineGetStringRange (line);
            auto lineStringEnd = cfrlineStringRange.location + cfrlineStringRange.length - 1;
            Range<int> lineStringRange ((int) cfrlineStringRange.location, (int) lineStringEnd);

            LineInfo lineInfo (frame, line, i);

            auto glyphLine = new TextLayout::Line (lineStringRange,
                                                   Point<float> ((float) lineInfo.origin.x,
                                                                 (float) (boundsHeight - lineInfo.origin.y)),
                                                   (float) lineInfo.ascent,
                                                   (float) lineInfo.descent,
                                                   (float) lineInfo.leading,
                                                   (int) numRuns);
            glyphLayout.addLine (glyphLine);

            for (CFIndex j = 0; j < numRuns; ++j)
            {
                auto run = (CTRunRef) CFArrayGetValueAtIndex (runs, j);
                auto numGlyphs = CTRunGetGlyphCount (run);
                auto runStringRange = CTRunGetStringRange (run);

                auto glyphRun = new TextLayout::Run (Range<int> ((int) runStringRange.location,
                                                                 (int) (runStringRange.location + runStringRange.length - 1)),
                                                     (int) numGlyphs);
                glyphLine->runs.add (glyphRun);

                CFDictionaryRef runAttributes = CTRunGetAttributes (run);

                CTFontRef ctRunFont;
                if (CFDictionaryGetValueIfPresent (runAttributes, kCTFontAttributeName, (const void**) &ctRunFont))
                {
                    auto cfsFontName = CTFontCopyPostScriptName (ctRunFont);
                    auto ctFontRef = CTFontCreateWithName (cfsFontName, referenceFontSize, nullptr);
                    CFRelease (cfsFontName);

                    auto fontHeightToPointsFactor = getHeightToPointsFactor (ctFontRef);
                    CFRelease (ctFontRef);

                    auto cfsFontFamily = (CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontFamilyNameAttribute);
                    auto cfsFontStyle  = (CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontStyleNameAttribute);

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
                    auto* components = CGColorGetComponents (cgRunColor);

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
        : Typeface (font.getTypefaceName(), font.getTypefaceStyle()), canBeUsedForLayout (true)
    {
        ctFontRef = CoreTextTypeLayout::createCTFont (font, referenceFontSize, renderingTransform);

        if (ctFontRef != nullptr)
        {
            fontRef = CTFontCopyGraphicsFont (ctFontRef, nullptr);
            initialiseMetrics();
        }
    }

    OSXTypeface (const void* data, size_t dataSize)
        : Typeface ({}, {}), canBeUsedForLayout (false), dataCopy (data, dataSize)
    {
        // We can't use CFDataCreate here as this triggers a false positive in ASAN
        // so copy the data manually and use CFDataCreateWithBytesNoCopy
        auto cfData = CFDataCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*) dataCopy.getData(),
                                                   (CFIndex) dataCopy.getSize(), kCFAllocatorNull);
        auto provider = CGDataProviderCreateWithCFData (cfData);
        CFRelease (cfData);

       #if JUCE_IOS
        // Workaround for a an obscure iOS bug which can cause the app to dead-lock
        // when loading custom type faces. See: http://www.openradar.me/18778790 and
        // http://stackoverflow.com/questions/40242370/app-hangs-in-simulator
        [UIFont systemFontOfSize: 12];
       #endif

        fontRef = CGFontCreateWithDataProvider (provider);
        CGDataProviderRelease (provider);

        if (fontRef != nullptr)
        {
           #if JUCE_MAC && defined (MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8
            canBeUsedForLayout = CTFontManagerRegisterGraphicsFont (fontRef, nullptr);
           #endif

            ctFontRef = CTFontCreateWithGraphicsFont (fontRef, referenceFontSize, nullptr, nullptr);

            if (ctFontRef != nullptr)
            {
                if (auto fontName = CTFontCopyName (ctFontRef, kCTFontFamilyNameKey))
                {
                    name = String::fromCFString (fontName);
                    CFRelease (fontName);
                }

                if (auto fontStyle = CTFontCopyName (ctFontRef, kCTFontStyleNameKey))
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
        auto ctAscent  = std::abs ((float) CTFontGetAscent (ctFontRef));
        auto ctDescent = std::abs ((float) CTFontGetDescent (ctFontRef));
        auto ctTotalHeight = ctAscent + ctDescent;

        ascent = ctAscent / ctTotalHeight;
        unitsToHeightScaleFactor = 1.0f / ctTotalHeight;
        pathTransform = AffineTransform::scale (unitsToHeightScaleFactor);

        fontHeightToPointsFactor = referenceFontSize / ctTotalHeight;

        const short zero = 0;
        auto numberRef = CFNumberCreate (0, kCFNumberShortType, &zero);

        CFStringRef keys[] = { kCTFontAttributeName, kCTLigatureAttributeName };
        CFTypeRef values[] = { ctFontRef, numberRef };
        attributedStringAtts = CFDictionaryCreate (nullptr, (const void**) &keys,
                                                   (const void**) &values, numElementsInArray (keys),
                                                   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFRelease (numberRef);
    }

    ~OSXTypeface()
    {
        if (attributedStringAtts != nullptr)
            CFRelease (attributedStringAtts);

        if (fontRef != nullptr)
        {
           #if JUCE_MAC && defined (MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8
            CTFontManagerUnregisterGraphicsFont (fontRef, nullptr);
           #endif

            CGFontRelease (fontRef);
        }

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
            auto cfText = text.toCFString();
            auto attribString = CFAttributedStringCreate (kCFAllocatorDefault, cfText, attributedStringAtts);
            CFRelease (cfText);

            auto line = CTLineCreateWithAttributedString (attribString);
            auto runArray = CTLineGetGlyphRuns (line);

            for (CFIndex i = 0; i < CFArrayGetCount (runArray); ++i)
            {
                auto run = (CTRunRef) CFArrayGetValueAtIndex (runArray, i);
                auto length = CTRunGetGlyphCount (run);

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

    void getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets) override
    {
        xOffsets.add (0);

        if (ctFontRef != nullptr && text.isNotEmpty())
        {
            float x = 0;

            auto cfText = text.toCFString();
            auto attribString = CFAttributedStringCreate (kCFAllocatorDefault, cfText, attributedStringAtts);
            CFRelease (cfText);

            auto line = CTLineCreateWithAttributedString (attribString);
            auto runArray = CTLineGetGlyphRuns (line);

            for (CFIndex i = 0; i < CFArrayGetCount (runArray); ++i)
            {
                auto run = (CTRunRef) CFArrayGetValueAtIndex (runArray, i);
                auto length = CTRunGetGlyphCount (run);

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

        if (auto pathRef = CTFontCreatePathForGlyph (ctFontRef, (CGGlyph) glyphNumber, &renderingTransform))
        {
            CGPathApply (pathRef, &path, pathApplier);
            CFRelease (pathRef);

            if (! pathTransform.isIdentity())
                path.applyTransform (pathTransform);

            return true;
        }

        return false;
    }

    //==============================================================================
    CGFontRef fontRef = {};
    CTFontRef ctFontRef = {};

    float fontHeightToPointsFactor = 1.0f;
    CGAffineTransform renderingTransform = CGAffineTransformIdentity;

    bool canBeUsedForLayout;

private:
    MemoryBlock dataCopy;
    CFDictionaryRef attributedStringAtts = {};
    float ascent = 0, unitsToHeightScaleFactor = 0;
    AffineTransform pathTransform;

    static void pathApplier (void* info, const CGPathElement* element)
    {
        auto& path = *static_cast<Path*> (info);
        auto* p = element->points;

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
    if (auto* tf = dynamic_cast<OSXTypeface*> (f.getTypeface()))
        return tf->ctFontRef;

    return {};
}

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

   #if JUCE_MAC
    // CTFontManager only exists on OS X 10.6 and later, it does not exist on iOS
    auto fontFamilyArray = CTFontManagerCopyAvailableFontFamilyNames();

    for (CFIndex i = 0; i < CFArrayGetCount (fontFamilyArray); ++i)
    {
        auto family = String::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (fontFamilyArray, i));

        if (! family.startsWithChar ('.')) // ignore fonts that start with a '.'
            names.addIfNotAlreadyThere (family);
    }

    CFRelease (fontFamilyArray);
   #else
    auto fontCollectionRef = CTFontCollectionCreateFromAvailableFonts (nullptr);
    auto fontDescriptorArray = CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef);
    CFRelease (fontCollectionRef);

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray); ++i)
    {
        auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray, i);
        auto cfsFontFamily = (CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute);

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

    auto cfsFontFamily = family.toCFString();
    CFStringRef keys[] = { kCTFontFamilyNameAttribute };
    CFTypeRef values[] = { cfsFontFamily };

    auto fontDescAttributes = CFDictionaryCreate (nullptr, (const void**) &keys, (const void**) &values, numElementsInArray (keys),
                                                  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease (cfsFontFamily);

    auto ctFontDescRef = CTFontDescriptorCreateWithAttributes (fontDescAttributes);
    CFRelease (fontDescAttributes);

    auto fontFamilyArray = CFArrayCreate (kCFAllocatorDefault, (const void**) &ctFontDescRef, 1, &kCFTypeArrayCallBacks);
    CFRelease (ctFontDescRef);

    auto fontCollectionRef = CTFontCollectionCreateWithFontDescriptors (fontFamilyArray, nullptr);
    CFRelease (fontFamilyArray);

    auto fontDescriptorArray = CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef);
    CFRelease (fontCollectionRef);

    if (fontDescriptorArray != nullptr)
    {
        for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray); ++i)
        {
            auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray, i);
            auto cfsFontStyle = (CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontStyleNameAttribute);
            results.add (String::fromCFString (cfsFontStyle));
            CFRelease (cfsFontStyle);
        }

        CFRelease (fontDescriptorArray);
    }

    return results;
}


//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)                  { return new OSXTypeface (font); }
Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t size)     { return new OSXTypeface (data, size); }

void Typeface::scanFolderForFonts (const File&)
{
    jassertfalse; // not implemented on this platform
}

struct DefaultFontNames
{
   #if JUCE_IOS
    String defaultSans  { "Helvetica" },
           defaultSerif { "Times New Roman" },
           defaultFixed { "Courier New" };
   #else
    String defaultSans  { "Lucida Grande" },
           defaultSerif { "Times New Roman" },
           defaultFixed { "Menlo" };
   #endif
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static DefaultFontNames defaultNames;

    auto newFont = font;
    auto& faceName = font.getTypefaceName();

    if (faceName == getDefaultSansSerifFontName())       newFont.setTypefaceName (defaultNames.defaultSans);
    else if (faceName == getDefaultSerifFontName())      newFont.setTypefaceName (defaultNames.defaultSerif);
    else if (faceName == getDefaultMonospacedFontName()) newFont.setTypefaceName (defaultNames.defaultFixed);

    if (font.getTypefaceStyle() == getDefaultStyle())
        newFont.setTypefaceStyle ("Regular");

    return Typeface::createSystemTypefaceFor (newFont);
}

static bool canAllTypefacesBeUsedInLayout (const AttributedString& text)
{
    auto numCharacterAttributes = text.getNumAttributes();

    for (int i = 0; i < numCharacterAttributes; ++i)
    {
        if (auto tf = dynamic_cast<OSXTypeface*> (text.getAttribute(i).font.getTypeface()))
            if (tf->canBeUsedForLayout)
                continue;

        return false;
    }

    return true;
}

bool TextLayout::createNativeLayout (const AttributedString& text)
{
    if (canAllTypefacesBeUsedInLayout (text))
    {
        CoreTextTypeLayout::createLayout (*this, text);
        return true;
    }

    return false;
}

} // namespace juce
