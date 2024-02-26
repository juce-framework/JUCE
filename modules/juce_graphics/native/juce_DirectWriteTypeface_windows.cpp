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

static String getLocalisedName (IDWriteLocalizedStrings* names)
{
    jassert (names != nullptr);

    uint32 index = 0;
    BOOL exists = false;
    [[maybe_unused]] auto hr = names->FindLocaleName (L"en-us", &index, &exists);

    if (! exists)
        index = 0;

    uint32 length = 0;
    hr = names->GetStringLength (index, &length);

    HeapBlock<wchar_t> name (length + 1);
    hr = names->GetString (index, name, length + 1);

    return static_cast<const wchar_t*> (name);
}

static String getFontFamilyName (IDWriteFontFamily* family)
{
    jassert (family != nullptr);
    ComSmartPtr<IDWriteLocalizedStrings> familyNames;
    auto hr = family->GetFamilyNames (familyNames.resetAndGetPointerAddress());
    jassertquiet (SUCCEEDED (hr));
    return getLocalisedName (familyNames);
}

static String getFontFaceName (IDWriteFont* font)
{
    jassert (font != nullptr);
    ComSmartPtr<IDWriteLocalizedStrings> faceNames;
    auto hr = font->GetFaceNames (faceNames.resetAndGetPointerAddress());
    jassertquiet (SUCCEEDED (hr));
    return getLocalisedName (faceNames);
}

template <typename Range>
static StringArray stringArrayFromRange (Range&& range)
{
    StringArray result;

    for (const auto& item : range)
        result.add (item);

    return result;
}

class AggregateFontCollection
{
public:
    explicit AggregateFontCollection (ComSmartPtr<IDWriteFontCollection> baseCollection)
        : collections { std::move (baseCollection) } {}

    StringArray findAllTypefaceNames()
    {
        const std::scoped_lock lock { mutex };

        std::set<String> strings;

        for (const auto& collection : collections)
        {
            const auto count = collection->GetFontFamilyCount();

            for (auto i = decltype (count){}; i < count; ++i)
            {
                ComSmartPtr<IDWriteFontFamily> family;

                if (FAILED (collection->GetFontFamily (i, family.resetAndGetPointerAddress())) || family == nullptr)
                    continue;

                strings.insert (getFontFamilyName (family));
            }
        }

        return stringArrayFromRange (strings);
    }

    StringArray findAllTypefaceStyles (const String& family)
    {
        const std::scoped_lock lock { mutex };

        for (const auto& collection : collections)
        {
            BOOL fontFound = false;
            uint32 fontIndex = 0;

            if (FAILED (collection->FindFamilyName (family.toWideCharPointer(), &fontIndex, &fontFound)) || ! fontFound)
                continue;

            ComSmartPtr<IDWriteFontFamily> fontFamily;

            if (FAILED (collection->GetFontFamily (fontIndex, fontFamily.resetAndGetPointerAddress())) || fontFamily == nullptr)
                continue;

            // Get the font faces
            const auto fontFacesCount = fontFamily->GetFontCount();
            std::set<String> results;

            for (uint32 i = 0; i < fontFacesCount; ++i)
            {
                ComSmartPtr<IDWriteFont> dwFont;

                if (FAILED (fontFamily->GetFont (i, dwFont.resetAndGetPointerAddress())) || dwFont->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE)
                    continue;

                results.insert (getFontFaceName (dwFont));
            }

            return stringArrayFromRange (results);
        }

        return {};
    }

    ComSmartPtr<IDWriteFontFamily> getFamilyByName (const wchar_t* name)
    {
        for (const auto& collection : collections)
        {
            const auto fontIndex = [&]
            {
                BOOL   found = false;
                UINT32 index = 0;

                return (SUCCEEDED (collection->FindFamilyName (name, &index, &found)) && found)
                       ? index
                       : (UINT32) -1;
            }();

            if (fontIndex == (UINT32) -1)
                continue;

            ComSmartPtr<IDWriteFontFamily> family;

            if (FAILED (collection->GetFontFamily (fontIndex, family.resetAndGetPointerAddress())) || family == nullptr)
                continue;

            return family;
        }

        return {};
    }

    void addCollection (ComSmartPtr<IDWriteFontCollection> collection)
    {
        const std::scoped_lock lock { mutex };
        collections.push_back (std::move (collection));
    }

    void removeCollection (ComSmartPtr<IDWriteFontCollection> collection)
    {
        const std::scoped_lock lock { mutex };
        const auto iter = std::find (collections.begin(), collections.end(), collection);

        if (iter != collections.end())
            collections.erase (iter);
    }

    struct MapResult
    {
        ComSmartPtr<IDWriteFont> font;
        UINT32 length{};
        float scale{};
    };

    /*  Tries matching against each collection in turn.
        If any collection is able to match the entire string, then uses the appropriate font
        from that collection.
        Otherwise, returns the font that is able to match the longest sequence of characters,
        preferring user-provided fonts.
    */
    MapResult mapCharacters (IDWriteFontFallback* fallback,
                             IDWriteTextAnalysisSource* analysisSource,
                             UINT32 textPosition,
                             UINT32 textLength,
                             wchar_t const* baseFamilyName,
                             DWRITE_FONT_WEIGHT baseWeight,
                             DWRITE_FONT_STYLE baseStyle,
                             DWRITE_FONT_STRETCH baseStretch) noexcept
    {
        const std::scoped_lock lock { mutex };

        // For reasons I don't understand, the system may pick better substitutions when passing
        // nullptr, instead of the system collection, as the "default collection to use".
        auto collectionsToCheck = collections;
        collectionsToCheck.insert (collectionsToCheck.begin(), nullptr);

        MapResult bestMatch;
        for (const auto& collection : collectionsToCheck)
        {
            MapResult result;
            const auto status = fallback->MapCharacters (analysisSource,
                                                         textPosition,
                                                         textLength,
                                                         collection,
                                                         baseFamilyName,
                                                         baseWeight,
                                                         baseStyle,
                                                         baseStretch,
                                                         &result.length,
                                                         result.font.resetAndGetPointerAddress(),
                                                         &result.scale);

            if (FAILED (status) || result.font == nullptr)
                continue;

            if (result.length == textLength)
                return result;

            if (result.length >= bestMatch.length)
                bestMatch = result;
        }

        return bestMatch;
    }

private:
    std::vector<ComSmartPtr<IDWriteFontCollection>> collections;
    std::mutex mutex;
};

class MemoryFontFileStream final : public ComBaseClassHelper<IDWriteFontFileStream>
{
public:
    explicit MemoryFontFileStream (MemoryBlock d) : rawData (std::move (d)) {}

    JUCE_COMRESULT GetFileSize (UINT64* fileSize) noexcept override
    {
        *fileSize = rawData.getSize();
        return S_OK;
    }

    JUCE_COMRESULT GetLastWriteTime (UINT64* lastWriteTime) noexcept override
    {
        *lastWriteTime = 0;
        return S_OK;
    }

    JUCE_COMRESULT ReadFileFragment (const void** fragmentStart,
                                     UINT64 fileOffset,
                                     UINT64 fragmentSize,
                                     void** fragmentContext) noexcept override
    {
        if (fileOffset + fragmentSize > rawData.getSize())
        {
            *fragmentStart   = nullptr;
            *fragmentContext = nullptr;
            return E_INVALIDARG;
        }

        *fragmentStart   = addBytesToPointer (rawData.getData(), fileOffset);
        *fragmentContext = this;
        return S_OK;
    }

    void WINAPI ReleaseFileFragment (void*) noexcept override {}

private:
    MemoryBlock rawData;
};

class MemoryFontFileLoader final : public ComBaseClassHelper<IDWriteFontFileLoader>
{
public:
    HRESULT WINAPI CreateStreamFromKey (const void* fontFileReferenceKey,
                                        UINT32 keySize,
                                        IDWriteFontFileStream** fontFileStream) noexcept override
    {
        *fontFileStream = new MemoryFontFileStream { MemoryBlock { fontFileReferenceKey, keySize } };
        return S_OK;
    }
};

class FontFileEnumerator final : public ComBaseClassHelper<IDWriteFontFileEnumerator>
{
public:
    FontFileEnumerator (IDWriteFactory& factoryIn, IDWriteFontFileLoader& loaderIn, MemoryBlock keyIn)
        : factory (factoryIn), loader (loaderIn), key (keyIn) {}

    HRESULT WINAPI GetCurrentFontFile (IDWriteFontFile** fontFile) noexcept override
    {
        *fontFile = nullptr;

        if (! isPositiveAndBelow (rawDataIndex, 1))
            return E_FAIL;

        return factory.CreateCustomFontFileReference (key.getData(),
                                                      (UINT32) key.getSize(),
                                                      &loader,
                                                      fontFile);
    }

    HRESULT WINAPI MoveNext (BOOL* hasCurrentFile) noexcept override
    {
        ++rawDataIndex;
        *hasCurrentFile = rawDataIndex < 1 ? TRUE : FALSE;
        return S_OK;
    }

    IDWriteFactory& factory;
    IDWriteFontFileLoader& loader;
    MemoryBlock key;
    size_t rawDataIndex = std::numeric_limits<size_t>::max();
};

class DirectWriteCustomFontCollectionLoader final : public ComBaseClassHelper<IDWriteFontCollectionLoader>
{
public:
    explicit DirectWriteCustomFontCollectionLoader (IDWriteFontFileLoader& loaderIn)
        : loader (loaderIn) {}

    HRESULT WINAPI CreateEnumeratorFromKey (IDWriteFactory* factory,
                                            const void* collectionKey,
                                            UINT32 collectionKeySize,
                                            IDWriteFontFileEnumerator** fontFileEnumerator) noexcept override
    {
        *fontFileEnumerator = new FontFileEnumerator { *factory, loader, MemoryBlock { collectionKey, collectionKeySize } };
        return S_OK;
    }

private:
    IDWriteFontFileLoader& loader;
};

class Direct2DFactories
{
public:
    Direct2DFactories()
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        if (direct2dDll.open ("d2d1.dll"))
        {
            JUCE_LOAD_WINAPI_FUNCTION (direct2dDll, D2D1CreateFactory, d2d1CreateFactory,
                                       HRESULT, (D2D1_FACTORY_TYPE, REFIID, D2D1_FACTORY_OPTIONS*, void**))

            if (d2d1CreateFactory != nullptr)
            {
                D2D1_FACTORY_OPTIONS options;
                options.debugLevel = D2D1_DEBUG_LEVEL_NONE;

                d2d1CreateFactory (D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof (ID2D1Factory), &options,
                                   (void**) d2dFactory.resetAndGetPointerAddress());
            }
        }

        if (directWriteDll.open ("DWrite.dll"))
        {
            JUCE_LOAD_WINAPI_FUNCTION (directWriteDll, DWriteCreateFactory, dWriteCreateFactory,
                                       HRESULT, (DWRITE_FACTORY_TYPE, REFIID, IUnknown**))

            if (dWriteCreateFactory == nullptr)
                return;

            for (const auto uuid : { __uuidof (IDWriteFactory3), __uuidof (IDWriteFactory2), __uuidof (IDWriteFactory) })
            {
                dWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED, uuid,
                                     (IUnknown**) directWriteFactory.resetAndGetPointerAddress());

                if (directWriteFactory != nullptr)
                    break;
            }

            if (directWriteFactory != nullptr)
            {
                directWriteFactory->RegisterFontFileLoader (fileLoader);
                directWriteFactory->RegisterFontCollectionLoader (collectionLoader);

                ComSmartPtr<IDWriteFontCollection> collection;

                if (SUCCEEDED (directWriteFactory->GetSystemFontCollection (collection.resetAndGetPointerAddress(), FALSE)) && collection != nullptr)
                    fonts.emplace (collection);
                else
                    jassertfalse;
            }

            if (d2dFactory != nullptr)
            {
                auto d2dRTProp = D2D1::RenderTargetProperties (D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                                                               D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM,
                                                                                  D2D1_ALPHA_MODE_IGNORE),
                                                               0, 0,
                                                               D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
                                                               D2D1_FEATURE_LEVEL_DEFAULT);

                d2dFactory->CreateDCRenderTarget (&d2dRTProp, directWriteRenderTarget.resetAndGetPointerAddress());
            }
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~Direct2DFactories()
    {
        if (directWriteFactory != nullptr)
        {
            directWriteFactory->UnregisterFontCollectionLoader (collectionLoader);
            directWriteFactory->UnregisterFontFileLoader (fileLoader);
        }
    }

    [[nodiscard]] ComSmartPtr<ID2D1Factory> getD2D1Factory() const { return d2dFactory; }
    [[nodiscard]] ComSmartPtr<IDWriteFactory> getDWriteFactory() const { return directWriteFactory; }
    [[nodiscard]] ComSmartPtr<ID2D1DCRenderTarget> getD2D1DCRenderTarget() const { return directWriteRenderTarget; }
    [[nodiscard]] AggregateFontCollection& getFonts() { jassert (fonts.has_value()); return *fonts; }
    [[nodiscard]] ComSmartPtr<IDWriteFontCollectionLoader> getCollectionLoader() const { return collectionLoader; }

private:
    DynamicLibrary direct2dDll, directWriteDll;
    ComSmartPtr<ID2D1Factory> d2dFactory;
    ComSmartPtr<IDWriteFactory> directWriteFactory;
    ComSmartPtr<ID2D1DCRenderTarget> directWriteRenderTarget;
    std::optional<AggregateFontCollection> fonts;

    ComSmartPtr<IDWriteFontFileLoader> fileLoader = becomeComSmartPtrOwner (new MemoryFontFileLoader);
    ComSmartPtr<IDWriteFontCollectionLoader> collectionLoader = becomeComSmartPtrOwner (new DirectWriteCustomFontCollectionLoader (*fileLoader));

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DFactories)
};

StringArray Font::findAllTypefaceNames()
{
    SharedResourcePointer<Direct2DFactories> factories;
    return factories->getFonts().findAllTypefaceNames();
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    SharedResourcePointer<Direct2DFactories> factories;
    return factories->getFonts().findAllTypefaceStyles (family);
}

extern bool juce_isRunningInWine();

class WindowsDirectWriteTypeface final : public Typeface
{
public:
    ~WindowsDirectWriteTypeface() override
    {
        if (collection != nullptr)
            factories->getFonts().removeCollection (collection);
    }

    float getStringWidth (const String& text) override
    {
        auto utf32 = text.toUTF32();
        auto numChars = utf32.length();
        std::vector<UINT16> results (numChars);

        if (FAILED (dwFontFace->GetGlyphIndices (utf32, (UINT32) numChars, results.data())))
            return {};

        float x = 0;

        for (size_t i = 0; i < numChars; ++i)
            x += getKerning (results[i], (i + 1) < numChars ? results[i + 1] : -1);

        const auto heightToPoints = getNativeDetails().getLegacyMetrics().getHeightToPointsFactor();
        return x * heightToPoints;
    }

    void getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets) override
    {
        auto utf32 = text.toUTF32();
        auto numChars = utf32.length();
        std::vector<UINT16> results (numChars);
        float x = 0;

        const auto heightToPoints = getNativeDetails().getLegacyMetrics().getHeightToPointsFactor();

        if (SUCCEEDED (dwFontFace->GetGlyphIndices (utf32, (UINT32) numChars, results.data())))
        {
            resultGlyphs.ensureStorageAllocated ((int) numChars);
            xOffsets.ensureStorageAllocated ((int) numChars + 1);

            for (size_t i = 0; i < numChars; ++i)
            {
                resultGlyphs.add (results[i]);
                xOffsets.add (x * heightToPoints);
                x += getKerning (results[i], (i + 1) < numChars ? results[i + 1] : -1);
            }
        }

        xOffsets.add (x * heightToPoints);
    }

    static Typeface::Ptr from (const Font& f)
    {
        const auto name = f.getTypefaceName();
        const auto style = f.getTypefaceStyle();

        SharedResourcePointer<Direct2DFactories> factories;
        const auto family = factories->getFonts().getFamilyByName (name.toWideCharPointer());

        if (family == nullptr)
            return {};

        const auto weight = f.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
        const auto italic = f.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

        ComSmartPtr<IDWriteFont> dwFont;

        if (FAILED (family->GetFirstMatchingFont (weight, DWRITE_FONT_STRETCH_NORMAL, italic, dwFont.resetAndGetPointerAddress())) || dwFont == nullptr)
            return {};

        ComSmartPtr<IDWriteFontFace> dwFontFace;

        if (FAILED (dwFont->CreateFontFace (dwFontFace.resetAndGetPointerAddress())) || dwFontFace == nullptr)
            return {};

        const HbFace hbFace { hb_directwrite_face_create (dwFontFace) };
        HbFont hbFont { hb_font_create (hbFace.get()) };

        FontStyleHelpers::initSynthetics (hbFont.get(), f);
        return new WindowsDirectWriteTypeface (name, style, dwFont, dwFontFace, std::move (hbFont));
    }

    static Typeface::Ptr from (Span<const std::byte> blob)
    {
        return from (MemoryBlock { blob.data(), blob.size() });
    }

    Native getNativeDetails() const override
    {
        return Native { hbFont.get() };
    }

    IDWriteFontFace* getIDWriteFontFace() const { return dwFontFace; }

private:
    float getKerning (int glyph1, int glyph2) const
    {
        const auto face = dwFontFace.getInterface<IDWriteFontFace1>();

        const UINT16 glyphs[] { (UINT16) glyph1, (UINT16) glyph2 };
        INT32 advances [std::size (glyphs)]{};

        if (FAILED (face->GetDesignGlyphAdvances ((UINT32) std::size (glyphs), std::data (glyphs), std::data (advances))))
            return {};

        DWRITE_FONT_METRICS metrics{};
        face->GetMetrics (&metrics);

        // TODO(reuk) incorrect
        return (float) advances[0] / (float) metrics.designUnitsPerEm;
    }

    static UINT32 numUtf16Words (const CharPointer_UTF16& str)
    {
        return (UINT32) (str.findTerminatingNull().getAddress() - str.getAddress());
    }

    class AnalysisSource final : public ComBaseClassHelper<IDWriteTextAnalysisSource>
    {
    public:
        AnalysisSource (String cIn, String langIn)
            : character (cIn), language (langIn) {}

        ~AnalysisSource() override = default;

        JUCE_COMCALL GetLocaleName (UINT32, UINT32*, const WCHAR** localeName) noexcept override
        {
            *localeName = language.isNotEmpty() ? language.toWideCharPointer() : nullptr;
            return S_OK;
        }

        JUCE_COMCALL GetNumberSubstitution (UINT32, UINT32*, IDWriteNumberSubstitution** substitution) noexcept override
        {
            *substitution = nullptr;
            return S_OK;
        }

        DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection() noexcept override
        {
            return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
        }

        JUCE_COMCALL GetTextAtPosition (UINT32 textPosition, const WCHAR** textString, UINT32* textLength) noexcept override
        {
            if (textPosition == 0)
            {
                const auto utf16 = character.toUTF16();
                *textString = utf16.getAddress();
                *textLength = numUtf16Words (utf16);
            }
            else
            {
                // We don't expect this to be hit. If you see this, alert the JUCE team!
                jassertfalse;
                *textString = nullptr;
                *textLength = 0;
            }

            return S_OK;
        }

        JUCE_COMCALL GetTextBeforePosition (UINT32, const WCHAR** textString, UINT32* textLength) noexcept override
        {
            // We don't expect this to be hit. If you see this, alert the JUCE team!
            jassertfalse;

            *textString = nullptr;
            *textLength = 0;
            return S_OK;
        }

    private:
        String character;
        String language;
    };

    static Typeface::Ptr from (MemoryBlock blob)
    {
        SharedResourcePointer<Direct2DFactories> factories;

        const auto dwFactory = factories->getDWriteFactory();

        if (dwFactory == nullptr)
            return {};

        ComSmartPtr<IDWriteFontCollection> customFontCollection;

        if (FAILED (dwFactory->CreateCustomFontCollection (factories->getCollectionLoader(),
                                                           blob.getData(),
                                                           (UINT32) blob.getSize(),
                                                           customFontCollection.resetAndGetPointerAddress())))
            return {};

        if (customFontCollection == nullptr)
            return {};

        ComSmartPtr<IDWriteFontFamily> fontFamily;

        if (FAILED (customFontCollection->GetFontFamily (0, fontFamily.resetAndGetPointerAddress())) || fontFamily == nullptr)
            return {};

        ComSmartPtr<IDWriteFont> dwFont;

        if (FAILED (fontFamily->GetFont (0, dwFont.resetAndGetPointerAddress())) || dwFont == nullptr)
            return {};

        ComSmartPtr<IDWriteFontFace> dwFontFace;

        if (FAILED (dwFont->CreateFontFace (dwFontFace.resetAndGetPointerAddress())) || dwFontFace == nullptr)
            return {};

        const auto name = getLocalisedFamilyName (*fontFamily);
        const auto style = getLocalisedStyle (*dwFont);

        const HbFace hbFace { hb_directwrite_face_create (dwFontFace) };

        return new WindowsDirectWriteTypeface (name, style, dwFont, dwFontFace, HbFont { hb_font_create (hbFace.get()) }, std::move (customFontCollection));
    }

    static String getLocalisedFamilyName (IDWriteFont& font)
    {
        ComSmartPtr<IDWriteFontFamily> family;
        if (FAILED (font.GetFontFamily (family.resetAndGetPointerAddress())) || family == nullptr)
            return {};

        return getLocalisedFamilyName (*family);
    }

    static String getLocalisedFamilyName (IDWriteFontFamily& fontFamily)
    {
        ComSmartPtr<IDWriteLocalizedStrings> familyNames;
        if (FAILED (fontFamily.GetFamilyNames (familyNames.resetAndGetPointerAddress())) || familyNames == nullptr)
            return {};

        return getLocalisedName (familyNames);
    }

    static String getLocalisedStyle (IDWriteFont& font)
    {
        ComSmartPtr<IDWriteLocalizedStrings> faceNames;
        if (FAILED (font.GetFaceNames (faceNames.resetAndGetPointerAddress())) || faceNames == nullptr)
            return {};

        return getLocalisedName (faceNames);
    }

    WindowsDirectWriteTypeface (const String& name,
                                const String& style,
                                ComSmartPtr<IDWriteFont> font,
                                ComSmartPtr<IDWriteFontFace> face,
                                HbFont hbFontIn,
                                ComSmartPtr<IDWriteFontCollection> collectionIn = nullptr)
        : Typeface (name, style),
          collection (std::move (collectionIn)),
          dwFont (font),
          dwFontFace (face),
          hbFont (std::move (hbFontIn))
    {
        if (collection != nullptr)
            factories->getFonts().addCollection (collection);
    }

    SharedResourcePointer<Direct2DFactories> factories;
    ComSmartPtr<IDWriteFontCollection> collection;
    ComSmartPtr<IDWriteFont> dwFont;
    ComSmartPtr<IDWriteFontFace> dwFontFace;
    HbFont hbFont;
};

struct DefaultFontNames
{
    DefaultFontNames()
    {
        if (juce_isRunningInWine())
        {
            // If we're running in Wine, then use fonts that might be available on Linux.
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

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return WindowsDirectWriteTypeface::from (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (Span<const std::byte> data)
{
    return WindowsDirectWriteTypeface::from (data);
}

void Typeface::scanFolderForFonts (const File&)
{
    // TODO(reuk)
}

//==============================================================================
bool TextLayout::createNativeLayout ([[maybe_unused]] const AttributedString& text)
{
    // TODO(reuk) Currently unimplemented
    return false;
}

} // namespace juce
