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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

// This symbol is available on all the platforms we support, but not declared in the CoreText headers on older platforms.
extern "C" CTFontRef CTFontCreateForStringWithLanguage (CTFontRef currentFont,
                                                        CFStringRef string,
                                                        CFRange range,
                                                        CFStringRef language);

class CoreTextTypeface final : public Typeface
{
    static CFUniquePtr<CTFontRef> addOwner (CTFontRef reference)
    {
        CFRetain (reference);
        return CFUniquePtr<CTFontRef> { reference };
    }

public:
    static Typeface::Ptr from (const Font& font)
    {
        auto ctFont = std::invoke ([&]() -> CFUniquePtr<CTFontRef>
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

            return CFUniquePtr<CTFontRef> { CTFontCreateWithFontDescriptor (ctFontDescRef.get(), 1, nullptr) };
        });

        if (ctFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (ctFont.get()), IncrementRef::no };

        if (result == nullptr)
            return {};

        FontStyleHelpers::initSynthetics (result.get(), font);

        return new CoreTextTypeface (std::move (ctFont),
                                     std::move (result));
    }

    static CFUniquePtr<CFDictionaryRef> createVariableDictionary (Span<const FontVariableSetting> variables)
    {
        Array<CFStringRef> keys;
        Array<CFNumberRef> values;

        for (auto v : variables)
        {
            keys.add (v.tag.toString().toCFString());
            const auto valueAsCGFloat = (CGFloat) v.value;
            values.add (CFNumberCreate (kCFAllocatorDefault, kCFNumberCGFloatType, &valueAsCGFloat));
        }

        const ScopeGuard stringReleaser { [&] { for (auto key : keys) CFRelease (key); } };

        return CFUniquePtr<CFDictionaryRef> { CFDictionaryCreate (kCFAllocatorDefault,
                                                                  (const void**) keys.data(),
                                                                  (const void**) values.data(),
                                                                  (CFIndex) variables.size(),
                                                                  &kCFTypeDictionaryKeyCallBacks,
                                                                  nullptr) };
    }

    Typeface::Ptr cloneWithVariableSettings (Span<const FontVariableSetting> variables) const override
    {
        const auto registry = getNativeDetails()->getVariableRegistry();
        auto sanitisedVariables = registry->sanitiseVariables (variables);
        auto variableDict = createVariableDictionary (sanitisedVariables);

        if (variableDict == nullptr)
        {
            jassertfalse;
            return {};
        }

        CFUniquePtr<CGFontRef> baseCGFont { CTFontCopyGraphicsFont (ctFont.get(), nullptr) };

        if (baseCGFont == nullptr)
            return {};

        CFUniquePtr<CGFontRef> newCGFont { CGFontCreateCopyWithVariations (baseCGFont.get(),
                                                                           variableDict.get()) };

        if (newCGFont == nullptr)
            return {};

        CFUniquePtr<CTFontRef> newFont { CTFontCreateWithGraphicsFont (newCGFont.get(), 1.0f, {}, {}) };

        if (newFont == nullptr)
            return {};

        HbFont result { hb_coretext_font_create (newFont.get()), IncrementRef::no };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface (std::move (newFont),
                                     std::move (result),
                                     storage,
                                     registry,
                                     std::move (sanitisedVariables));
    }

    static Typeface::Ptr from (Span<const std::byte> data)
    {
        // We can't use CFDataCreate here as this triggers a false positive in ASAN
        // so copy the data manually and use CFDataCreateWithBytesNoCopy
        auto copy = std::make_shared<MemoryBlock> (data.data(), data.size());

        const CFUniquePtr<CFDataRef> cfData { CFDataCreateWithBytesNoCopy (kCFAllocatorDefault,
                                                                           static_cast<const UInt8*> (copy->getData()),
                                                                           (CFIndex) copy->getSize(),
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

        HbFont result { hb_coretext_font_create (ctFont.get()), IncrementRef::no };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface (std::move (ctFont),
                                     std::move (result),
                                     std::move (copy));
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

        HbFont result { hb_coretext_font_create (newFont.get()), IncrementRef::no };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface (std::move (newFont),
                                     std::move (result));
    }

    const Native* getNativeDetails() const override
    {
        return native.get();
    }

    static Typeface::Ptr findSystemTypeface()
    {
        CFUniquePtr<CTFontRef> defaultCtFont (CTFontCreateUIFontForLanguage (kCTFontUIFontSystem, 0.0, nullptr));
        const CFUniquePtr<CFStringRef> newName { CTFontCopyFamilyName (defaultCtFont.get()) };
        const CFUniquePtr<CTFontDescriptorRef> descriptor { CTFontCopyFontDescriptor (defaultCtFont.get()) };
        const CFUniquePtr<CFStringRef> newStyle { (CFStringRef) CTFontDescriptorCopyAttribute (descriptor.get(),
                                                                                               kCTFontStyleNameAttribute) };

        HbFont result { hb_coretext_font_create (defaultCtFont.get()), IncrementRef::no };

        if (result == nullptr)
            return {};

        return new CoreTextTypeface (std::move (defaultCtFont),
                                     std::move (result));
    }

private:
    static TypefaceVerticalMetrics getNativeMetrics (CTFontRef ctFont)
    {
        const CFUniquePtr<CGFontRef> cgFont { CTFontCopyGraphicsFont (ctFont, nullptr) };

        const auto upem = (float) CGFontGetUnitsPerEm (cgFont.get());
        const auto ascent  = std::abs ((float) CGFontGetAscent  (cgFont.get()) / upem);
        const auto descent = std::abs ((float) CGFontGetDescent (cgFont.get()) / upem);
        const auto leading = std::abs ((float) CGFontGetLeading (cgFont.get()) / upem);

        return { ascent, descent, leading };
    }

    CoreTextTypeface (CFUniquePtr<CTFontRef> nativeFont,
                      HbFont fontIn,
                      std::shared_ptr<MemoryBlock> data = {},
                      std::shared_ptr<VariableAxisRegistry> variableAxisRegistry = {},
                      std::vector<FontVariableSetting> variables = {})
        : CoreTextTypeface (addOwner (nativeFont.get()),
                            std::make_unique<Native> (TypefaceNativeOptions { fontIn,
                                                                              getNativeMetrics (nativeFont.get()),
                                                                              std::move (variables),
                                                                              {},
                                                                              {},
                                                                              variableAxisRegistry }),
                            std::move (data))
    {
    }

    CoreTextTypeface (CFUniquePtr<CTFontRef> nativeFont,
                      std::unique_ptr<Native> nativeIn,
                      std::shared_ptr<MemoryBlock> data)
        : Typeface (nativeIn->getTypefaceName(), nativeIn->getTypefaceStyle()),
          ctFont (std::move (nativeFont)),
          storage (std::move (data)),
          native (std::move (nativeIn))
    {
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
    std::shared_ptr<MemoryBlock> storage;
    std::unique_ptr<Native> native;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreTextTypeface)
};

//==============================================================================
Typeface::Ptr Typeface::createFromFontImpl (const Font& font)
{
    return CoreTextTypeface::from (font);
}

Typeface::Ptr Typeface::createFromDataImpl (Span<const std::byte> data)
{
    return CoreTextTypeface::from (data);
}

void Typeface::scanFolderForFonts (const File& folder)
{
    for (auto& file : folder.findChildFiles (File::findFiles, false, "*.otf;*.ttf"))
        if (auto urlref = CFUniquePtr<CFURLRef> (CFURLCreateWithFileSystemPath (kCFAllocatorDefault, file.getFullPathName().toCFString(), kCFURLPOSIXPathStyle, true)))
            CTFontManagerRegisterFontsForURL (urlref.get(), kCTFontManagerScopeProcess, nullptr);
}

Typeface::Ptr Typeface::findSystemTypeface()
{
    return CoreTextTypeface::findSystemTypeface();
}

StringArray Font::findAllTypefaceNamesImpl()
{
    StringArray names;

    CFUniquePtr<CTFontCollectionRef> fontCollectionRef (CTFontCollectionCreateFromAvailableFonts (nullptr));
    CFUniquePtr<CFArrayRef> fontDescriptorArray (CTFontCollectionCreateMatchingFontDescriptors (fontCollectionRef.get()));

    for (CFIndex i = 0; i < CFArrayGetCount (fontDescriptorArray.get()); ++i)
    {
        auto ctFontDescriptorRef = (CTFontDescriptorRef) CFArrayGetValueAtIndex (fontDescriptorArray.get(), i);
        CFUniquePtr<CFStringRef> cfsFontFamily ((CFStringRef) CTFontDescriptorCopyAttribute (ctFontDescriptorRef, kCTFontFamilyNameAttribute));

        names.add (String::fromCFString (cfsFontFamily.get()));
    }

    return names;
}

StringArray Font::findAllTypefaceStylesImpl (const String& family)
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
            const auto name = String::fromCFString (cfsFontStyle.get());

            results.add (name);
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

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
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

} // namespace juce
