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
                requiredTransform = CGAffineTransformMake (1.0f, 0, 0.1f, 1.0f, 0, 0);

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

    static CFUniquePtr<CTFontRef> getFontWithPointSize (CTFontRef font, float pointSize)
    {
        return CFUniquePtr<CTFontRef> (CTFontCreateCopyWithAttributes (font, pointSize, nullptr, nullptr));
    }

    static CFUniquePtr<CTFontRef> createCTFont (const Font& font, const float fontSizePoints, CGAffineTransform& transformRequired)
    {
        CFUniquePtr<CFStringRef> cfFontFamily (FontStyleHelpers::getConcreteFamilyName (font).toCFString());
        CFUniquePtr<CFStringRef> cfFontStyle (findBestAvailableStyle (font, transformRequired).toCFString());
        CFStringRef keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
        CFTypeRef values[] = { cfFontFamily.get(), cfFontStyle.get() };

        CFUniquePtr<CFDictionaryRef> fontDescAttributes (CFDictionaryCreate (nullptr,
                                                                             (const void**) &keys,
                                                                             (const void**) &values,
                                                                             numElementsInArray (keys),
                                                                             &kCFTypeDictionaryKeyCallBacks,
                                                                             &kCFTypeDictionaryValueCallBacks));

        CFUniquePtr<CTFontDescriptorRef> ctFontDescRef (CTFontDescriptorCreateWithAttributes (fontDescAttributes.get()));

        return CFUniquePtr<CTFontRef> (CTFontCreateWithFontDescriptor (ctFontDescRef.get(), fontSizePoints, nullptr));
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

    static CFUniquePtr<CTFontRef> getOrCreateFont (const Font& f)
    {
        if (auto ctf = getCTFontFromTypeface (f))
        {
            CFRetain (ctf);
            return CFUniquePtr<CTFontRef> (ctf);
        }

        CGAffineTransform transform;
        return createCTFont (f, referenceFontSize, transform);
    }

    //==============================================================================
    static CTTextAlignment getTextAlignment (const AttributedString& text)
    {
        const auto flags = text.getJustification().getOnlyHorizontalFlags();

        if (@available (macOS 10.8, *))
        {
            switch (flags)
            {
                case Justification::right:                  return kCTTextAlignmentRight;
                case Justification::horizontallyCentred:    return kCTTextAlignmentCenter;
                case Justification::horizontallyJustified:  return kCTTextAlignmentJustified;
                default:                                    return kCTTextAlignmentLeft;
            }
        }

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

        switch (flags)
        {
            case Justification::right:                  return kCTRightTextAlignment;
            case Justification::horizontallyCentred:    return kCTCenterTextAlignment;
            case Justification::horizontallyJustified:  return kCTJustifiedTextAlignment;
            default:                                    return kCTLeftTextAlignment;
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    static CTLineBreakMode getLineBreakMode (const AttributedString& text)
    {
        switch (text.getWordWrap())
        {
            case AttributedString::none:        return kCTLineBreakByClipping;
            case AttributedString::byChar:      return kCTLineBreakByCharWrapping;
            case AttributedString::byWord:
            default:                            return kCTLineBreakByWordWrapping;
        }
    }

    static CTWritingDirection getWritingDirection (const AttributedString& text)
    {
        switch (text.getReadingDirection())
        {
            case AttributedString::rightToLeft:   return kCTWritingDirectionRightToLeft;
            case AttributedString::leftToRight:   return kCTWritingDirectionLeftToRight;
            case AttributedString::natural:
            default:                              return kCTWritingDirectionNatural;
        }
    }

    //==============================================================================
    // A flatmap that properly retains/releases font refs
    class FontMap
    {
    public:
        void emplace (CTFontRef ctFontRef, Font value)
        {
            pairs.emplace (std::lower_bound (pairs.begin(), pairs.end(), ctFontRef), ctFontRef, std::move (value));
        }

        const Font* find (CTFontRef ctFontRef) const
        {
            const auto iter = std::lower_bound (pairs.begin(), pairs.end(), ctFontRef);

            if (iter == pairs.end())
                return nullptr;

            if (iter->key.get() != ctFontRef)
                return nullptr;

            return &iter->value;
        }

    private:
        struct Pair
        {
            Pair (CTFontRef ref, Font font) : key (ref), value (std::move (font)) { CFRetain (ref); }

            bool operator< (CTFontRef other) const { return key.get() < other; }

            CFUniquePtr<CTFontRef> key;
            Font value;
        };

        std::vector<Pair> pairs;
    };

    struct AttributedStringAndFontMap
    {
        CFUniquePtr<CFAttributedStringRef> string;
        FontMap fontMap;
    };

    static AttributedStringAndFontMap createCFAttributedString (const AttributedString& text)
    {
        FontMap fontMap;

        const detail::ColorSpacePtr rgbColourSpace { CGColorSpaceCreateWithName (kCGColorSpaceSRGB) };

        auto attribString = CFAttributedStringCreateMutable (kCFAllocatorDefault, 0);
        CFUniquePtr<CFStringRef> cfText (text.getText().toCFString());

        CFAttributedStringReplaceString (attribString, CFRangeMake (0, 0), cfText.get());

        const auto numCharacterAttributes = text.getNumAttributes();
        const auto attribStringLen = CFAttributedStringGetLength (attribString);
        const auto beginPtr = text.getText().toUTF16();
        auto currentPosition = beginPtr;

        for (int i = 0; i < numCharacterAttributes; currentPosition += text.getAttribute (i).range.getLength(), ++i)
        {
            const auto& attr = text.getAttribute (i);
            const auto wordBegin = currentPosition.getAddress() - beginPtr.getAddress();

            if (attribStringLen <= wordBegin)
                continue;

            const auto wordEndAddress = (currentPosition + attr.range.getLength()).getAddress();
            const auto wordEnd = jmin (attribStringLen, (CFIndex) (wordEndAddress - beginPtr.getAddress()));
            const auto range = CFRangeMake (wordBegin, wordEnd - wordBegin);

            if (auto ctFontRef = getOrCreateFont (attr.font))
            {
                ctFontRef = getFontWithPointSize (ctFontRef.get(), attr.font.getHeight() * getHeightToPointsFactor (ctFontRef.get()));
                fontMap.emplace (ctFontRef.get(), attr.font);

                CFAttributedStringSetAttribute (attribString, range, kCTFontAttributeName, ctFontRef.get());

                if (attr.font.isUnderlined())
                {
                    auto underline = kCTUnderlineStyleSingle;

                    CFUniquePtr<CFNumberRef> numberRef (CFNumberCreate (nullptr, kCFNumberIntType, &underline));
                    CFAttributedStringSetAttribute (attribString, range, kCTUnderlineStyleAttributeName, numberRef.get());
                }

                auto extraKerning = attr.font.getExtraKerningFactor();

                if (! approximatelyEqual (extraKerning, 0.0f))
                {
                    extraKerning *= attr.font.getHeight();

                    CFUniquePtr<CFNumberRef> numberRef (CFNumberCreate (nullptr, kCFNumberFloatType, &extraKerning));
                    CFAttributedStringSetAttribute (attribString, range, kCTKernAttributeName, numberRef.get());
                }
            }

            {
                auto col = attr.colour;

                const CGFloat components[] = { col.getFloatRed(),
                                               col.getFloatGreen(),
                                               col.getFloatBlue(),
                                               col.getFloatAlpha() };
                auto colour = CGColorCreate (rgbColourSpace.get(), components);

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
            { kCTParagraphStyleSpecifierAlignment,              sizeof (CTTextAlignment),    &ctTextAlignment },
            { kCTParagraphStyleSpecifierLineBreakMode,          sizeof (CTLineBreakMode),    &ctLineBreakMode },
            { kCTParagraphStyleSpecifierBaseWritingDirection,   sizeof (CTWritingDirection), &ctWritingDirection},
            { kCTParagraphStyleSpecifierLineSpacingAdjustment,  sizeof (CGFloat),            &ctLineSpacing }
        };

        CFUniquePtr<CTParagraphStyleRef> ctParagraphStyleRef (CTParagraphStyleCreate (settings, (size_t) numElementsInArray (settings)));
        CFAttributedStringSetAttribute (attribString, CFRangeMake (0, CFAttributedStringGetLength (attribString)),
                                        kCTParagraphStyleAttributeName, ctParagraphStyleRef.get());
        return { CFUniquePtr<CFAttributedStringRef> (attribString), std::move (fontMap) };
    }

    struct FramesetterAndFontMap
    {
        CFUniquePtr<CTFramesetterRef> framesetter;
        FontMap fontMap;
    };

    static FramesetterAndFontMap createCTFramesetter (const AttributedString& text)
    {
        auto attribStringAndMap = createCFAttributedString (text);
        return { CFUniquePtr<CTFramesetterRef> (CTFramesetterCreateWithAttributedString (attribStringAndMap.string.get())),
                 std::move (attribStringAndMap.fontMap) };
    }

    static CFUniquePtr<CTFrameRef> createCTFrame (CTFramesetterRef framesetter, CGRect bounds)
    {
        auto path = CGPathCreateMutable();
        CGPathAddRect (path, nullptr, bounds);

        CFUniquePtr<CTFrameRef> frame (CTFramesetterCreateFrame (framesetter, CFRangeMake (0, 0), path, nullptr));
        CGPathRelease (path);

        return frame;
    }

    struct FrameAndFontMap
    {
        CFUniquePtr<CTFrameRef> frame;
        FontMap fontMap;
    };

    static FrameAndFontMap createCTFrame (const AttributedString& text, CGRect bounds)
    {
        auto framesetterAndMap = createCTFramesetter (text);
        return { createCTFrame (framesetterAndMap.framesetter.get(), bounds),
                 std::move (framesetterAndMap.fontMap) };
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

    static bool areAllFontsDefaultWidth (const AttributedString& text)
    {
        auto numCharacterAttributes = text.getNumAttributes();

        for (int i = 0; i < numCharacterAttributes; ++i)
            if (text.getAttribute (i).font.getHorizontalScale() != 1.0f)
                return false;

        return true;
    }

    static bool drawToCGContext (const AttributedString& text, const Rectangle<float>& area,
                                 const CGContextRef& context, float flipHeight)
    {
        if (! areAllFontsDefaultWidth (text))
            return false;

        auto framesetter = createCTFramesetter (text).framesetter;

        // Ugly hack to fix a bug in OS X Sierra where the CTFrame needs to be slightly
        // larger than the font height - otherwise the CTFrame will be invalid

        CFRange fitrange;
        auto suggestedSingleLineFrameSize =
            CTFramesetterSuggestFrameSizeWithConstraints (framesetter.get(), CFRangeMake (0, 0), nullptr,
                                                          CGSizeMake (CGFLOAT_MAX, CGFLOAT_MAX), &fitrange);
        auto minCTFrameHeight = (float) suggestedSingleLineFrameSize.height;

        auto verticalJustification = text.getJustification().getOnlyVerticalFlags();

        auto ctFrameArea = [area, minCTFrameHeight, verticalJustification]
        {
            if (minCTFrameHeight < area.getHeight())
                return area;

            if (verticalJustification == Justification::verticallyCentred)
                return area.withSizeKeepingCentre (area.getWidth(), minCTFrameHeight);

            auto frameArea = area.withHeight (minCTFrameHeight);

            if (verticalJustification == Justification::bottom)
                return frameArea.withBottomY (area.getBottom());

            return frameArea;
        }();

        auto frame = createCTFrame (framesetter.get(), CGRectMake ((CGFloat) ctFrameArea.getX(), flipHeight - (CGFloat) ctFrameArea.getBottom(),
                                                                   (CGFloat) ctFrameArea.getWidth(), (CGFloat) ctFrameArea.getHeight()));

        auto textMatrix = CGContextGetTextMatrix (context);
        CGContextSaveGState (context);

        if (verticalJustification == Justification::verticallyCentred
         || verticalJustification == Justification::bottom)
        {
            auto adjust = ctFrameArea.getHeight() - findCTFrameHeight (frame.get());

            if (verticalJustification == Justification::verticallyCentred)
                adjust *= 0.5f;

            CGContextTranslateCTM (context, 0, -adjust);
        }

        CTFrameDraw (frame.get(), context);

        CGContextRestoreGState (context);
        CGContextSetTextMatrix (context, textMatrix);

        return true;
    }

    static void createLayout (TextLayout& glyphLayout, const AttributedString& text)
    {
        auto boundsHeight = glyphLayout.getHeight();
        auto frameAndMap = createCTFrame (text, CGRectMake (0, 0, glyphLayout.getWidth(), boundsHeight));
        auto lines = CTFrameGetLines (frameAndMap.frame.get());
        auto numLines = CFArrayGetCount (lines);

        glyphLayout.ensureStorageAllocated ((int) numLines);

        for (CFIndex i = 0; i < numLines; ++i)
        {
            auto line = (CTLineRef) CFArrayGetValueAtIndex (lines, i);
            auto runs = CTLineGetGlyphRuns (line);
            auto numRuns = CFArrayGetCount (runs);

            auto cfrlineStringRange = CTLineGetStringRange (line);
            auto lineStringEnd = cfrlineStringRange.location + cfrlineStringRange.length;
            Range<int> lineStringRange ((int) cfrlineStringRange.location, (int) lineStringEnd);

            LineInfo lineInfo (frameAndMap.frame.get(), line, i);

            auto glyphLine = std::make_unique<TextLayout::Line> (lineStringRange,
                                                                 Point<float> ((float) lineInfo.origin.x,
                                                                               (float) (boundsHeight - lineInfo.origin.y)),
                                                                 (float) lineInfo.ascent,
                                                                 (float) lineInfo.descent,
                                                                 (float) lineInfo.leading,
                                                                 (int) numRuns);

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
                    glyphRun->font = [&]
                    {
                        if (auto* it = frameAndMap.fontMap.find (ctRunFont))
                            return *it;

                        CFUniquePtr<CFStringRef> cfsFontName (CTFontCopyPostScriptName (ctRunFont));
                        CFUniquePtr<CTFontRef> ctFontRef (CTFontCreateWithName (cfsFontName.get(), referenceFontSize, nullptr));

                        auto fontHeightToPointsFactor = getHeightToPointsFactor (ctFontRef.get());

                        CFUniquePtr<CFStringRef> cfsFontFamily ((CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontFamilyNameAttribute));
                        CFUniquePtr<CFStringRef> cfsFontStyle ((CFStringRef) CTFontCopyAttribute (ctRunFont, kCTFontStyleNameAttribute));

                        Font result (String::fromCFString (cfsFontFamily.get()),
                                     String::fromCFString (cfsFontStyle.get()),
                                     (float) (CTFontGetSize (ctRunFont) / fontHeightToPointsFactor));

                        auto isUnderlined = [&]
                        {
                            CFNumberRef underlineStyle;

                            if (CFDictionaryGetValueIfPresent (runAttributes, kCTUnderlineStyleAttributeName, (const void**) &underlineStyle))
                            {
                                if (CFGetTypeID (underlineStyle) == CFNumberGetTypeID())
                                {
                                    int value = 0;
                                    CFNumberGetValue (underlineStyle, kCFNumberLongType, (void*) &value);

                                    return value != 0;
                                }
                            }

                            return false;
                        }();

                        result.setUnderline (isUnderlined);
                        return result;
                    }();
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
                    glyphRun->glyphs.add (TextLayout::Glyph (glyphs.glyphs[k],
                                                             convertToPointFloat (positions.points[k]),
                                                             (float) advances.advances[k].width));
            }

            glyphLayout.addLine (std::move (glyphLine));
        }
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
            fontRef = CTFontCopyGraphicsFont (ctFontRef.get(), nullptr);
            initialiseMetrics();
        }
    }

    OSXTypeface (const void* data, size_t dataSize)
        : Typeface ({}, {}), canBeUsedForLayout (false), dataCopy (data, dataSize)
    {
        // We can't use CFDataCreate here as this triggers a false positive in ASAN
        // so copy the data manually and use CFDataCreateWithBytesNoCopy
        CFUniquePtr<CFDataRef> cfData (CFDataCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*) dataCopy.getData(),
                                                                    (CFIndex) dataCopy.getSize(), kCFAllocatorNull));
        auto provider = CGDataProviderCreateWithCFData (cfData.get());

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
            if (@available (macOS 10.11, *))
                canBeUsedForLayout = CTFontManagerRegisterGraphicsFont (fontRef, nullptr);

            ctFontRef.reset (CTFontCreateWithGraphicsFont (fontRef, referenceFontSize, nullptr, nullptr));

            if (ctFontRef != nullptr)
            {
                if (auto fontName = CFUniquePtr<CFStringRef> (CTFontCopyName (ctFontRef.get(), kCTFontFamilyNameKey)))
                    name = String::fromCFString (fontName.get());

                if (auto fontStyle = CFUniquePtr<CFStringRef> (CTFontCopyName (ctFontRef.get(), kCTFontStyleNameKey)))
                    style = String::fromCFString (fontStyle.get());

                initialiseMetrics();
            }
        }
    }

    void initialiseMetrics()
    {
        auto ctAscent  = std::abs ((float) CTFontGetAscent  (ctFontRef.get()));
        auto ctDescent = std::abs ((float) CTFontGetDescent (ctFontRef.get()));
        auto ctTotalHeight = ctAscent + ctDescent;

        ascent = ctAscent / ctTotalHeight;
        unitsToHeightScaleFactor = 1.0f / ctTotalHeight;
        pathTransform = AffineTransform::scale (unitsToHeightScaleFactor);

        fontHeightToPointsFactor = referenceFontSize / ctTotalHeight;

        const short zero = 0;
        CFUniquePtr<CFNumberRef> numberRef (CFNumberCreate (nullptr, kCFNumberShortType, &zero));

        CFStringRef keys[] = { kCTFontAttributeName, kCTLigatureAttributeName };
        CFTypeRef values[] = { ctFontRef.get(), numberRef.get() };
        attributedStringAtts.reset (CFDictionaryCreate (nullptr, (const void**) &keys,
                                                        (const void**) &values, numElementsInArray (keys),
                                                        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    }

    ~OSXTypeface() override
    {
        if (fontRef != nullptr)
        {
            if (@available (macOS 10.8, *))
                if (dataCopy.getSize() != 0)
                    CTFontManagerUnregisterGraphicsFont (fontRef, nullptr);

            CGFontRelease (fontRef);
        }
    }

    float getAscent() const override                 { return ascent; }
    float getDescent() const override                { return 1.0f - ascent; }
    float getHeightToPointsFactor() const override   { return fontHeightToPointsFactor; }

    float getStringWidth (const String& text) override
    {
        float x = 0;

        if (ctFontRef != nullptr && text.isNotEmpty())
        {
            CFUniquePtr<CFStringRef> cfText (text.toCFString());
            CFUniquePtr<CFAttributedStringRef> attribString (CFAttributedStringCreate (kCFAllocatorDefault, cfText.get(), attributedStringAtts.get()));

            CFUniquePtr<CTLineRef> line (CTLineCreateWithAttributedString (attribString.get()));
            auto runArray = CTLineGetGlyphRuns (line.get());

            for (CFIndex i = 0; i < CFArrayGetCount (runArray); ++i)
            {
                auto run = (CTRunRef) CFArrayGetValueAtIndex (runArray, i);
                auto length = CTRunGetGlyphCount (run);

                const CoreTextTypeLayout::Advances advances (run, length);

                for (int j = 0; j < length; ++j)
                    x += (float) advances.advances[j].width;
            }

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

            CFUniquePtr<CFStringRef> cfText (text.toCFString());
            CFUniquePtr<CFAttributedStringRef> attribString (CFAttributedStringCreate (kCFAllocatorDefault, cfText.get(), attributedStringAtts.get()));

            CFUniquePtr<CTLineRef> line (CTLineCreateWithAttributedString (attribString.get()));
            auto runArray = CTLineGetGlyphRuns (line.get());

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
        }
    }

    bool getOutlineForGlyph (int glyphNumber, Path& path) override
    {
        jassert (path.isEmpty());  // we might need to apply a transform to the path, so this must be empty

        if (auto pathRef = CFUniquePtr<CGPathRef> (CTFontCreatePathForGlyph (ctFontRef.get(), (CGGlyph) glyphNumber, &renderingTransform)))
        {
            CGPathApply (pathRef.get(), &path, pathApplier);

            if (! pathTransform.isIdentity())
                path.applyTransform (pathTransform);

            return true;
        }

        return false;
    }

    //==============================================================================
    CGFontRef fontRef = {};
    CFUniquePtr<CTFontRef> ctFontRef;

    float fontHeightToPointsFactor = 1.0f;
    CGAffineTransform renderingTransform = CGAffineTransformIdentity;

    bool canBeUsedForLayout;

private:
    MemoryBlock dataCopy;
    CFUniquePtr<CFDictionaryRef> attributedStringAtts;
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
    const auto typeface = f.getTypefacePtr();

    if (auto* tf = dynamic_cast<OSXTypeface*> (typeface.get()))
        return tf->ctFontRef.get();

    return {};
}

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

   #if JUCE_MAC
    // CTFontManager only exists on OS X 10.6 and later, it does not exist on iOS
    CFUniquePtr<CFArrayRef> fontFamilyArray (CTFontManagerCopyAvailableFontFamilyNames());

    for (CFIndex i = 0; i < CFArrayGetCount (fontFamilyArray.get()); ++i)
    {
        auto family = String::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (fontFamilyArray.get(), i));

        if (! family.startsWithChar ('.')) // ignore fonts that start with a '.'
            names.addIfNotAlreadyThere (family);
    }
   #else
    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateFromAvailableFonts (nullptr));
    CFUniquePtr<CFArrayRef> fontDescriptorArray (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get()));

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
    {
        auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
        CFUniquePtr<CFStringRef> cfsFontFamily ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute));

        names.addIfNotAlreadyThere (String::fromCFString (cfsFontFamily.get()));
    }
   #endif

    names.sort (true);
    return names;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    StringArray results;

    CFUniquePtr<CFStringRef> cfsFontFamily (family.toCFString());
    CFStringRef keys[] = { kCTFontFamilyNameAttribute };
    CFTypeRef values[] = { cfsFontFamily.get() };

    CFUniquePtr<CFDictionaryRef> fontDescAttributes (CFDictionaryCreate (nullptr, (const void**) &keys, (const void**) &values, numElementsInArray (keys),
                                                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

    CFUniquePtr<CTFontDescriptorRef> ctFontDescRef (CTFontDescriptorCreateWithAttributes (fontDescAttributes.get()));
    CFUniquePtr<CFArrayRef> fontFamilyArray (CFArrayCreate (kCFAllocatorDefault, (const void**) &ctFontDescRef, 1, &kCFTypeArrayCallBacks));

    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateWithFontDescriptors (fontFamilyArray.get(), nullptr));

    if (auto fontDescriptorArray = CFUniquePtr<CFArrayRef> (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get())))
    {
        for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
        {
            auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
            CFUniquePtr<CFStringRef> cfsFontStyle ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontStyleNameAttribute));
            results.add (String::fromCFString (cfsFontStyle.get()));
        }
    }

    return results;
}


//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)                  { return *new OSXTypeface (font); }
Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t size)     { return *new OSXTypeface (data, size); }

void Typeface::scanFolderForFonts (const File& folder)
{
    for (auto& file : folder.findChildFiles (File::findFiles, false, "*.otf;*.ttf"))
        if (auto urlref = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault, file.getFullPathName().toCFString(), kCFURLPOSIXPathStyle, true)))
            CTFontManagerRegisterFontsForURL (urlref.get(), kCTFontManagerScopeProcess, nullptr);
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
    auto faceName = font.getTypefaceName();

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
        auto typeface = text.getAttribute (i).font.getTypefacePtr();

        if (auto tf = dynamic_cast<OSXTypeface*> (typeface.get()))
            if (tf->canBeUsedForLayout)
                continue;

        return false;
    }

    return true;
}

bool TextLayout::createNativeLayout (const AttributedString& text)
{
    if (canAllTypefacesBeUsedInLayout (text) && CoreTextTypeLayout::areAllFontsDefaultWidth (text))
    {
        CoreTextTypeLayout::createLayout (*this, text);
        return true;
    }

    return false;
}

} // namespace juce
