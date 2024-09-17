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
        const std::scoped_lock lock { mutex };

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

    ComSmartPtr<IDWriteFont> findFontForFace (IDWriteFontFace* face)
    {
        for (const auto& collection : collections)
        {
            ComSmartPtr<IDWriteFont> result;

            if (SUCCEEDED (collection->GetFontFromFontFace (face, result.resetAndGetPointerAddress())))
                return result;
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
    explicit MemoryFontFileStream (std::shared_ptr<const MemoryBlock> blockIn)
        : block (std::move (blockIn))
    {
    }

    JUCE_COMRESULT GetFileSize (UINT64* fileSize) noexcept override
    {
        *fileSize = block->getSize();
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
        if (fileOffset + fragmentSize > block->getSize())
        {
            *fragmentStart   = nullptr;
            *fragmentContext = nullptr;
            return E_INVALIDARG;
        }

        *fragmentStart   = addBytesToPointer (block->getData(), fileOffset);
        *fragmentContext = this;
        return S_OK;
    }

    void WINAPI ReleaseFileFragment (void*) noexcept override {}

private:
    std::shared_ptr<const MemoryBlock> block;
};

class MemoryFontFileLoader final : public ComBaseClassHelper<IDWriteFontFileLoader>
{
public:
    explicit MemoryFontFileLoader (MemoryBlock blob)
        : block (std::make_shared<const MemoryBlock> (std::move (blob)))
    {
    }

    HRESULT WINAPI CreateStreamFromKey (const void* fontFileReferenceKey,
                                        UINT32 keySize,
                                        IDWriteFontFileStream** fontFileStream) noexcept override
    {
        if (keySize != Uuid::size())
            return E_INVALIDARG;

        Uuid requestedKey { static_cast<const uint8*> (fontFileReferenceKey) };

        if (requestedKey == uuid)
        {
            *fontFileStream = new MemoryFontFileStream { block };
            return S_OK;
        }

        return E_INVALIDARG;
    }

    Uuid getUuid() const { return uuid; }

private:
    std::shared_ptr<const MemoryBlock> block;
    Uuid uuid;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryFontFileLoader)
};

class FontFileEnumerator final : public ComBaseClassHelper<IDWriteFontFileEnumerator>
{
public:
    FontFileEnumerator (IDWriteFactory& factoryIn, ComSmartPtr<MemoryFontFileLoader> loaderIn)
        : factory (factoryIn), loader (loaderIn) {}

    HRESULT WINAPI GetCurrentFontFile (IDWriteFontFile** fontFile) noexcept override
    {
        *fontFile = nullptr;

        if (! isPositiveAndBelow (rawDataIndex, 1))
            return E_FAIL;

        const auto uuid = loader->getUuid();
        return factory.CreateCustomFontFileReference (uuid.getRawData(),
                                                      (UINT32) uuid.size(),
                                                      loader,
                                                      fontFile);
    }

    HRESULT WINAPI MoveNext (BOOL* hasCurrentFile) noexcept override
    {
        ++rawDataIndex;
        *hasCurrentFile = rawDataIndex < 1 ? TRUE : FALSE;
        return S_OK;
    }

private:
    IDWriteFactory& factory;
    ComSmartPtr<MemoryFontFileLoader> loader;
    size_t rawDataIndex = std::numeric_limits<size_t>::max();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontFileEnumerator)
};

class DirectWriteCustomFontCollectionLoader final : public ComBaseClassHelper<IDWriteFontCollectionLoader>
{
public:
    explicit DirectWriteCustomFontCollectionLoader (IDWriteFactory& factoryIn)
        : factory (factoryIn)
    {
    }

    ~DirectWriteCustomFontCollectionLoader() override
    {
        for (const auto& loader : fileLoaders)
            factory.UnregisterFontFileLoader (loader);
    }

    Uuid addRawFontData (Span<const std::byte> blob)
    {
        const auto loader = becomeComSmartPtrOwner (new MemoryFontFileLoader { { blob.data(), blob.size() } });

        factory.RegisterFontFileLoader (loader);

        fileLoaders.push_back (loader);

        return fileLoaders.back()->getUuid();
    }

    HRESULT WINAPI CreateEnumeratorFromKey (IDWriteFactory* factoryIn,
                                            const void* collectionKey,
                                            UINT32 collectionKeySize,
                                            IDWriteFontFileEnumerator** fontFileEnumerator) noexcept override
    {
        if (collectionKeySize != Uuid::size())
            return E_INVALIDARG;

        const Uuid requestedCollectionKey { static_cast<const uint8*> (collectionKey) };

        for (const auto& loader : fileLoaders)
        {
            if (loader->getUuid() != requestedCollectionKey)
                continue;

            *fontFileEnumerator = new FontFileEnumerator { *factoryIn, loader };
            return S_OK;
        }

        return E_INVALIDARG;
    }

private:
    IDWriteFactory& factory;
    std::vector<ComSmartPtr<MemoryFontFileLoader>> fileLoaders;
};

//==============================================================================
class Direct2DFactories
{
public:
    Direct2DFactories()
    {
        ComSmartPtr<IDWriteFontCollection> collection;

        if (SUCCEEDED (directWriteFactory->GetSystemFontCollection (collection.resetAndGetPointerAddress(), FALSE)) && collection != nullptr)
            fonts.emplace (collection);
        else
            jassertfalse;
    }

    ~Direct2DFactories()
    {
        if (directWriteFactory == nullptr)
            return;

        directWriteFactory->UnregisterFontCollectionLoader (collectionLoader);
    }

    [[nodiscard]] ComSmartPtr<IDWriteFactory> getDWriteFactory() const { return directWriteFactory; }
    [[nodiscard]] ComSmartPtr<IDWriteFactory4> getDWriteFactory4() const { return directWriteFactory4; }
    [[nodiscard]] AggregateFontCollection& getFonts() { jassert (fonts.has_value()); return *fonts; }
    [[nodiscard]] ComSmartPtr<DirectWriteCustomFontCollectionLoader> getCollectionLoader() const { return collectionLoader; }

private:
    DynamicLibrary direct2dDll { "d2d1.dll" }, directWriteDll { "DWrite.dll" };

    const ComSmartPtr<ID2D1Factory> d2dFactory = [&]() -> ComSmartPtr<ID2D1Factory>
    {
        JUCE_LOAD_WINAPI_FUNCTION (direct2dDll,
                                   D2D1CreateFactory,
                                   d2d1CreateFactory,
                                   HRESULT,
                                   (D2D1_FACTORY_TYPE, REFIID, D2D1_FACTORY_OPTIONS*, void**))

        if (d2d1CreateFactory == nullptr)
            return {};

        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = D2D1_DEBUG_LEVEL_NONE;

        ComSmartPtr<ID2D1Factory> result;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        d2d1CreateFactory (D2D1_FACTORY_TYPE_SINGLE_THREADED,
                            __uuidof (ID2D1Factory),
                            &options,
                           (void**) result.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return result;
    }();

    const ComSmartPtr<IDWriteFactory> directWriteFactory = [&]() -> ComSmartPtr<IDWriteFactory>
    {
        JUCE_LOAD_WINAPI_FUNCTION (directWriteDll,
                                   DWriteCreateFactory,
                                   dWriteCreateFactory,
                                   HRESULT,
                                   (DWRITE_FACTORY_TYPE, REFIID, IUnknown**))

        if (dWriteCreateFactory == nullptr)
            return {};

        ComSmartPtr<IDWriteFactory> result;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        dWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof (IDWriteFactory),
                             (IUnknown**) result.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return result;
    }();

    const ComSmartPtr<DirectWriteCustomFontCollectionLoader> collectionLoader = [&]() -> ComSmartPtr<DirectWriteCustomFontCollectionLoader>
    {
        auto result = becomeComSmartPtrOwner (new DirectWriteCustomFontCollectionLoader { *directWriteFactory });
        directWriteFactory->RegisterFontCollectionLoader (result);

        return result;
    }();

    const ComSmartPtr<IDWriteFactory4> directWriteFactory4 = directWriteFactory.getInterface<IDWriteFactory4>();

    std::optional<AggregateFontCollection> fonts;

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

    static Typeface::Ptr from (const Font& f)
    {
        const auto name = f.getTypefaceName();
        const auto style = f.getTypefaceStyle();

        SharedResourcePointer<Direct2DFactories> factories;
        const auto family = factories->getFonts().getFamilyByName (name.toWideCharPointer());

        if (family == nullptr)
            return getLastResortTypeface (f);

        const auto weight = f.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
        const auto italic = f.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

        ComSmartPtr<IDWriteFont> dwFont;

        if (FAILED (family->GetFirstMatchingFont (weight, DWRITE_FONT_STRETCH_NORMAL, italic, dwFont.resetAndGetPointerAddress())) || dwFont == nullptr)
            return {};

        return fromFont (dwFont, nullptr, &f, MetricsMechanism::dwriteOnly);
    }

    static Typeface::Ptr from (Span<const std::byte> blob)
    {
        SharedResourcePointer<Direct2DFactories> factories;

        const auto dwFactory = factories->getDWriteFactory();

        if (dwFactory == nullptr)
            return {};

        const auto collectionLoader = factories->getCollectionLoader();
        const auto collectionKey = collectionLoader->addRawFontData ({ blob.data(), blob.size() });

        ComSmartPtr<IDWriteFontCollection> customFontCollection;

        if (FAILED (dwFactory->CreateCustomFontCollection (factories->getCollectionLoader(),
                                                           collectionKey.getRawData(),
                                                           (UINT32) collectionKey.size(),
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

        return fromFont (dwFont, customFontCollection, nullptr, MetricsMechanism::gdiWithDwriteFallback);
    }

    Native getNativeDetails() const override
    {
        return Native { hbFont.get(), nonPortableMetrics };
    }

    Typeface::Ptr createSystemFallback (const String& c, const String& language) const override
    {
        auto factory = factories->getDWriteFactory().getInterface<IDWriteFactory2>();

        if (factory == nullptr)
        {
            // System font fallback is unavailable before Windows 8.1
            jassertfalse;
            return {};
        }

        ComSmartPtr<IDWriteFontFallback> fallback;
        if (FAILED (factory->GetSystemFontFallback (fallback.resetAndGetPointerAddress())) || fallback == nullptr)
            return {};

        auto analysisSource = becomeComSmartPtrOwner (new AnalysisSource (c, language));
        const auto originalName = getLocalisedFamilyName (*dwFont);

        const auto mapped = factories->getFonts().mapCharacters (fallback,
                                                                 analysisSource,
                                                                 0,
                                                                 numUtf16Words (c.toUTF16()),
                                                                 originalName.toWideCharPointer(),
                                                                 dwFont->GetWeight(),
                                                                 dwFont->GetStyle(),
                                                                 dwFont->GetStretch());

        if (mapped.font == nullptr)
            return {};

        return fromFont (mapped.font, nullptr, nullptr, MetricsMechanism::dwriteOnly);
    }

    ComSmartPtr<IDWriteFontFace> getIDWriteFontFace() const { return dwFontFace; }

    static Typeface::Ptr findSystemTypeface()
    {
        NONCLIENTMETRICS nonClientMetrics{};
        nonClientMetrics.cbSize = sizeof (NONCLIENTMETRICS);

        if (! SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICS), &nonClientMetrics, sizeof (NONCLIENTMETRICS)))
            return {};

        SharedResourcePointer<Direct2DFactories> factories;

        ComSmartPtr<IDWriteGdiInterop> interop;
        if (FAILED (factories->getDWriteFactory()->GetGdiInterop (interop.resetAndGetPointerAddress())) || interop == nullptr)
            return {};

        ComSmartPtr<IDWriteFont> dwFont;
        if (FAILED (interop->CreateFontFromLOGFONT (&nonClientMetrics.lfMessageFont, dwFont.resetAndGetPointerAddress())) || dwFont == nullptr)
            return {};

        return fromFont (dwFont, nullptr, nullptr, MetricsMechanism::gdiWithDwriteFallback);
    }

private:
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
                                TypefaceAscentDescent metrics,
                                ComSmartPtr<IDWriteFontCollection> collectionIn = nullptr)
        : Typeface (name, style),
          collection (std::move (collectionIn)),
          dwFont (font),
          dwFontFace (face),
          hbFont (std::move (hbFontIn)),
          nonPortableMetrics (metrics)
    {
        if (collection != nullptr)
            factories->getFonts().addCollection (collection);
    }

    static TypefaceAscentDescent getDwriteMetrics (IDWriteFontFace& face)
    {
        DWRITE_FONT_METRICS dwriteFontMetrics{};
        face.GetMetrics (&dwriteFontMetrics);
        return TypefaceAscentDescent { (float) dwriteFontMetrics.ascent  / (float) dwriteFontMetrics.designUnitsPerEm,
                                       (float) dwriteFontMetrics.descent / (float) dwriteFontMetrics.designUnitsPerEm };
    }

    static std::optional<TypefaceAscentDescent> getGdiMetrics (hb_font_t* font)
    {
        hb_position_t ascent{}, descent{};

        if (! hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_ASCENT,  &ascent) ||
            ! hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_DESCENT, &descent))
            return {};

        const auto upem = (float) hb_face_get_upem (hb_font_get_face (font));
        return TypefaceAscentDescent { (float) std::abs (ascent) / upem, (float) std::abs (descent) / upem };
    }

    enum class MetricsMechanism
    {
        dwriteOnly,
        gdiWithDwriteFallback,
    };

    static Typeface::Ptr fromFont (ComSmartPtr<IDWriteFont> dwFont,
                                   ComSmartPtr<IDWriteFontCollection> collection,
                                   const Font* fontForSynthetics,
                                   MetricsMechanism mm)
    {
        ComSmartPtr<IDWriteFontFace> dwFace;

        if (FAILED (dwFont->CreateFontFace (dwFace.resetAndGetPointerAddress())) || dwFace == nullptr)
            return {};

        const auto name = getLocalisedFamilyName (*dwFont);
        const auto style = getLocalisedStyle (*dwFont);

        const HbFace hbFace { hb_directwrite_face_create (dwFace) };
        HbFont font { hb_font_create (hbFace.get()) };
        const auto dwMetrics = getDwriteMetrics (*dwFace);

        const auto metrics = mm == MetricsMechanism::gdiWithDwriteFallback
                           ? getGdiMetrics (font.get()).value_or (dwMetrics)
                           : dwMetrics;

        if (fontForSynthetics != nullptr)
            FontStyleHelpers::initSynthetics (font.get(), *fontForSynthetics);

        return new WindowsDirectWriteTypeface (name,
                                               style,
                                               dwFont,
                                               dwFace,
                                               std::move (font),
                                               metrics,
                                               collection);
    }

    // This attempts to replicate the behaviour of the non-directwrite typeface lookup in JUCE 7 and older
    static Typeface::Ptr getLastResortTypeface (const Font& font)
    {
        auto* dc = CreateCompatibleDC (nullptr);
        const ScopeGuard deleteDC { [&] { DeleteDC (dc); } };

        SetMapperFlags (dc, 0);
        SetMapMode (dc, MM_TEXT);

        const auto style = font.getTypefaceStyle();

        LOGFONTW lf{};
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfItalic = (BYTE) (style.contains ("Italic") ? TRUE : FALSE);
        lf.lfWeight = style.contains ("Bold") ? FW_BOLD : FW_NORMAL;
        lf.lfHeight = -256;
        font.getTypefaceName().copyToUTF16 (lf.lfFaceName, sizeof (lf.lfFaceName));

        auto* hfont = CreateFontIndirectW (&lf);
        const ScopeGuard deleteFont { [&] { DeleteObject (hfont); } };

        auto* prevFont = hfont != nullptr ? SelectObject (dc, hfont) : nullptr;
        const ScopeGuard reinstateFont { [&] { if (prevFont != nullptr) SelectObject (dc, prevFont); } };

        SharedResourcePointer<Direct2DFactories> factories;

        ComSmartPtr<IDWriteGdiInterop> interop;
        if (FAILED (factories->getDWriteFactory()->GetGdiInterop (interop.resetAndGetPointerAddress())) || interop == nullptr)
            return {};

        ComSmartPtr<IDWriteFontFace> dwFontFace;
        if (FAILED (interop->CreateFontFaceFromHdc (dc, dwFontFace.resetAndGetPointerAddress())) || dwFontFace == nullptr)
            return {};

        const auto dwFont = factories->getFonts().findFontForFace (dwFontFace);

        if (dwFont == nullptr)
            return {};

        return fromFont (dwFont, nullptr, nullptr, MetricsMechanism::gdiWithDwriteFallback);
    }

    SharedResourcePointer<Direct2DFactories> factories;
    ComSmartPtr<IDWriteFontCollection> collection;
    ComSmartPtr<IDWriteFont> dwFont;
    ComSmartPtr<IDWriteFontFace> dwFontFace;
    HbFont hbFont;
    TypefaceAscentDescent nonPortableMetrics;
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

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
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

Typeface::Ptr Typeface::findSystemTypeface()
{
    return WindowsDirectWriteTypeface::findSystemTypeface();
}

void Typeface::scanFolderForFonts (const File&)
{
    // TODO(reuk)
}

} // namespace juce
