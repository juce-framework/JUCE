/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

static constexpr float referenceFontSize = 1024.0f;

CTFontRef getCTFontFromTypeface (const Font& f);

namespace CoreTextTypeLayout
{
    static float getFontTotalHeight (CTFontRef font)
    {
        return std::abs ((float) CTFontGetAscent (font))
               + std::abs ((float) CTFontGetDescent (font));
    }

    static float getHeightToPointsFactor (CTFontRef font)
    {
        return (float) CTFontGetSize (font) / (float) getFontTotalHeight (font);
    }

    static CFUniquePtr<CTFontRef> getFontWithPointSize (CTFontRef font, float pointSize)
    {
        return CFUniquePtr<CTFontRef> (CTFontCreateCopyWithAttributes (font, pointSize, nullptr, nullptr));
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

        return nullptr;
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

                        Font result (FontOptions { String::fromCFString (cfsFontFamily.get()),
                                                   String::fromCFString (cfsFontStyle.get()),
                                                   (float) (CTFontGetSize (ctRunFont) / fontHeightToPointsFactor) });

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

// This symbol is available on all the platforms we support, but not declared in the CoreText headers on older platforms.
extern "C" CTFontRef CTFontCreateForStringWithLanguage (CTFontRef currentFont,
                                                        CFStringRef string,
                                                        CFRange range,
                                                        CFStringRef language);

class CoreTextTypeface final : public Typeface
{
    static auto& getRegistered()
    {
        class Registered
        {
        public:
            void add (CTFontRef ref)
            {
                const std::scoped_lock lock { mutex };
                CFUniquePtr<CGFontRef> cgFont { CTFontCopyGraphicsFont (ref, nullptr) };

                if (CTFontManagerRegisterGraphicsFont (cgFont.get(), nullptr))
                    map.emplace (ref, std::move (cgFont));
            }

            void remove (CTFontRef ref)
            {
                const std::scoped_lock lock { mutex };

                if (const auto iter = map.find (ref); iter != map.end())
                {
                    CTFontManagerUnregisterGraphicsFont (iter->second.get(), nullptr);
                    map.erase (iter);
                }
            }

            std::set<String> getRegisteredFamilies() const
            {
                const std::scoped_lock lock { mutex };
                std::set<String> result;

                for (const auto& item : map)
                {
                    const CFUniquePtr<CFStringRef> family { CTFontCopyName (item.first, kCTFontFamilyNameKey) };
                    result.insert (String::fromCFString (family.get()));
                }

                return result;
            }

        private:
            std::map<CTFontRef, CFUniquePtr<CGFontRef>> map;
            mutable std::mutex mutex;
        };

        static Registered registered;
        return registered;
    }

public:
    static Typeface::Ptr from (const Font& font)
    {
        CFUniquePtr<CFStringRef> cfFontFamily (FontStyleHelpers::getConcreteFamilyName (font).toCFString());

        if (cfFontFamily == nullptr)
            return {};

        CFUniquePtr<CFStringRef> cfFontStyle (findBestAvailableStyle (font).toCFString());

        if (cfFontStyle == nullptr)
            return {};

        CFStringRef keys[] { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
        CFTypeRef values[] { cfFontFamily.get(), cfFontStyle.get() };

        CFUniquePtr<CFDictionaryRef> fontDescAttributes (CFDictionaryCreate (nullptr,
                                                                             (const void**) &keys,
                                                                             (const void**) &values,
                                                                             numElementsInArray (keys),
                                                                             &kCFTypeDictionaryKeyCallBacks,
                                                                             &kCFTypeDictionaryValueCallBacks));

        if (fontDescAttributes == nullptr)
            return {};

        CFUniquePtr<CTFontDescriptorRef> ctFontDescRef (CTFontDescriptorCreateWithAttributes (fontDescAttributes.get()));

        if (ctFontDescRef == nullptr)
            return {};

        CFUniquePtr<CTFontRef> ctFont { CTFontCreateWithFontDescriptor (ctFontDescRef.get(), 1, nullptr) };

        if (ctFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (ctFont.get()) };

        if (result == nullptr)
            return {};

        FontStyleHelpers::initSynthetics (result.get(), font);

        return new CoreTextTypeface (std::move (ctFont), std::move (result), font.getTypefaceName(), font.getTypefaceStyle());
    }

    static Typeface::Ptr from (Span<const std::byte> data)
    {
        // We can't use CFDataCreate here as this triggers a false positive in ASAN
        // so copy the data manually and use CFDataCreateWithBytesNoCopy
        MemoryBlock copy { data.data(), data.size() };

        const CFUniquePtr<CFDataRef> cfData { CFDataCreateWithBytesNoCopy (kCFAllocatorDefault,
                                                                           static_cast<const UInt8*> (copy.getData()),
                                                                           (CFIndex) copy.getSize(),
                                                                           kCFAllocatorNull) };

        if (cfData == nullptr)
            return {};

       #if JUCE_IOS
        // Workaround for a an obscure iOS bug which can cause the app to dead-lock
        // when loading custom type faces. See: http://www.openradar.me/18778790 and
        // http://stackoverflow.com/questions/40242370/app-hangs-in-simulator
        [UIFont systemFontOfSize: 12];
       #endif

        const CFUniquePtr<CGDataProviderRef> provider { CGDataProviderCreateWithCFData (cfData.get()) };

        if (provider == nullptr)
            return {};

        const CFUniquePtr<CGFontRef> font { CGFontCreateWithDataProvider (provider.get()) };

        if (font == nullptr)
            return {};

        CFUniquePtr<CTFontRef> ctFont { CTFontCreateWithGraphicsFont (font.get(), 1.0f, {}, {}) };

        if (ctFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (ctFont.get()) };

        if (result == nullptr)
            return {};

        const CFUniquePtr<CFStringRef> family { CTFontCopyName (ctFont.get(), kCTFontFamilyNameKey) };
        const CFUniquePtr<CFStringRef> style  { CTFontCopyName (ctFont.get(), kCTFontStyleNameKey) };

        return new CoreTextTypeface (std::move (ctFont),
                                     std::move (result),
                                     String::fromCFString (family.get()),
                                     String::fromCFString (style.get()),
                                     std::move (copy));
    }

    Native getNativeDetails() const override
    {
        return Native { hb.get(), nonPortableMetrics };
    }

    Typeface::Ptr createSystemFallback (const String& c, const String& language) const override
    {
        const CFUniquePtr<CFStringRef> cfText { c.toCFString() };
        const CFUniquePtr<CFStringRef> cfLanguage { language.toCFString() };

        auto* old = ctFont.get();
        const CFUniquePtr<CFStringRef> oldName { CTFontCopyFamilyName (old) };
        const CFUniquePtr<CTFontDescriptorRef> oldDescriptor { CTFontCopyFontDescriptor (old) };
        const CFUniquePtr<CFStringRef> oldStyle { (CFStringRef) CTFontDescriptorCopyAttribute (oldDescriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        CFUniquePtr<CTFontRef> newFont { CTFontCreateForStringWithLanguage (old,
                                                                            cfText.get(),
                                                                            CFRangeMake (0, CFStringGetLength (cfText.get())),
                                                                            cfLanguage.get()) };

        const CFUniquePtr<CFStringRef> newName { CTFontCopyFamilyName (newFont.get()) };
        const CFUniquePtr<CTFontDescriptorRef> descriptor { CTFontCopyFontDescriptor (newFont.get()) };
        const CFUniquePtr<CFStringRef> newStyle { (CFStringRef) CTFontDescriptorCopyAttribute (descriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        HbFont result { hb_coretext_font_create (newFont.get()) };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface { std::move (newFont),
                                      std::move (result),
                                      String::fromCFString (newName.get()),
                                      String::fromCFString (newStyle.get()),
                                      {} };
    }

    static std::set<String> getRegisteredFamilies()
    {
        return getRegistered().getRegisteredFamilies();
    }

    ~CoreTextTypeface() override
    {
        getRegistered().remove (ctFont.get());
    }

    CTFontRef getFontRef() const
    {
        return ctFont.get();
    }

private:
    CoreTextTypeface (CFUniquePtr<CTFontRef> nativeFont,
                      HbFont fontIn,
                      const String& name,
                      const String& style,
                      MemoryBlock data = {})
        : Typeface (name, style),
          ctFont (std::move (nativeFont)),
          hb (std::move (fontIn)),
          storage (std::move (data))
    {
        if (! storage.isEmpty())
            getRegistered().add (ctFont.get());
    }

    static String findBestAvailableStyle (const Font& font)
    {
        const auto availableStyles = Font::findAllTypefaceStyles (font.getTypefaceName());
        const auto style = font.getTypefaceStyle();

        if (availableStyles.contains (style))
            return style;

        return availableStyles[0];
    }

    // We store this, rather than calling hb_coretext_font_get_ct_font, because harfbuzz may
    // override the font cascade list in the returned font.
    CFUniquePtr<CTFontRef> ctFont;
    HbFont hb;
    MemoryBlock storage;
    TypefaceAscentDescent nonPortableMetrics = [&]
    {
        const CFUniquePtr<CGFontRef> cgFont { CTFontCopyGraphicsFont (ctFont.get(), nullptr) };
        const auto upem = (float) CGFontGetUnitsPerEm (cgFont.get());
        return TypefaceAscentDescent { (float) std::abs (CGFontGetAscent (cgFont.get())  / upem),
                                       (float) std::abs (CGFontGetDescent (cgFont.get()) / upem) };
    }();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreTextTypeface)
};

CTFontRef getCTFontFromTypeface (const Font& f)
{
    const auto typeface = f.getTypefacePtr();

    if (auto* tf = dynamic_cast<CoreTextTypeface*> (typeface.get()))
        return tf->getFontRef();

    return {};
}

//==============================================================================
Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return CoreTextTypeface::from (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (Span<const std::byte> data)
{
    return CoreTextTypeface::from (data);
}

void Typeface::scanFolderForFonts (const File& folder)
{
    for (auto& file : folder.findChildFiles (File::findFiles, false, "*.otf;*.ttf"))
        if (auto urlref = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault, file.getFullPathName().toCFString(), kCFURLPOSIXPathStyle, true)))
            CTFontManagerRegisterFontsForURL (urlref.get(), kCTFontManagerScopeProcess, nullptr);
}

StringArray Font::findAllTypefaceNames()
{
    StringArray names;

    // The collection returned from CTFontCollectionCreateFromAvailableFonts doesn't include fonts registered by
    // CTFontManagerRegisterGraphicsFont on iOS, so we need to keep track of registered fonts ourselves.
    auto nameSet = CoreTextTypeface::getRegisteredFamilies();

    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateFromAvailableFonts (nullptr));
    CFUniquePtr<CFArrayRef> fontDescriptorArray (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get()));

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
    {
        auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
        CFUniquePtr<CFStringRef> cfsFontFamily ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute));

        nameSet.insert (String::fromCFString (cfsFontFamily.get()));
    }

    for (auto& item : nameSet)
        names.add (item);

    return names;
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    StringArray results;

    CFUniquePtr<CFStringRef> cfsFontFamily (family.toCFString());
    CFStringRef keys[] { kCTFontFamilyNameAttribute };
    CFTypeRef values[] { cfsFontFamily.get() };

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

bool TextLayout::createNativeLayout (const AttributedString& text)
{
    CoreTextTypeLayout::createLayout (*this, text);
    return true;
}

} // namespace juce
