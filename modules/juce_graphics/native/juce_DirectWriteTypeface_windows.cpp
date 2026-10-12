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

StringArray Font::findAllTypefaceNamesImpl()
{
    SharedResourcePointer<Direct2DFactories> factories;
    return factories->getFonts().findAllTypefaceNames();
}

StringArray Font::findAllTypefaceStylesImpl (const String& family)
{
    if (FontStyleHelpers::isPlaceholderFamilyName (family))
        return findAllTypefaceStyles (FontStyleHelpers::getConcreteFamilyNameFromPlaceholder (family));

    SharedResourcePointer<Direct2DFactories> factories;
    return factories->getFonts().findAllTypefaceStyles (family);
}

class WindowsDirectWriteTypeface final : public Typeface
{
public:
    Ptr cloneWithVariableSettings (Span<const FontVariableSetting> settings) const override
    {
        ComSmartPtr<IDWriteFontFace5> fontFace5;
        ComSmartPtr<IDWriteFontResource> resource;

        if (FAILED (dwFontFace->QueryInterface (fontFace5.resetAndGetPointerAddress()))
            || FAILED (fontFace5->GetFontResource (resource.resetAndGetPointerAddress())))
        {
            // This version of Windows doesn't support DirectWrite3 (Windows 10 Build 16299).
            jassertfalse;
            return {};
        }

        const auto registry = getNativeDetails()->getVariableRegistry();
        auto sanitisedVariables = registry->sanitiseVariables (settings);

        std::vector<DWRITE_FONT_AXIS_VALUE> axes (sanitisedVariables.size());
        std::transform (sanitisedVariables.begin(),
                        sanitisedVariables.end(),
                        axes.begin(),
                        [] (FontVariableSetting var)
        {
            return DWRITE_FONT_AXIS_VALUE { (DWRITE_FONT_AXIS_TAG) ByteOrder::swap (var.tag.getTag()),
                                            var.value };
        });

        ComSmartPtr<IDWriteFontFace5> configuredFontFace;
        auto hr = resource->CreateFontFace (DWRITE_FONT_SIMULATIONS_NONE,
                                            axes.data(),
                                            (UINT32) axes.size(),
                                            configuredFontFace.resetAndGetPointerAddress());

        if (FAILED (hr))
            return {};

        HbFace hbFace { hb_directwrite_face_create (configuredFontFace), IncrementRef::no };
        HbFont font { hb_font_create (hbFace.get()), IncrementRef::no };

        const auto dwMetrics = getDwriteMetrics (*fontFace5);

        return new WindowsDirectWriteTypeface (dwFont,
                                               configuredFontFace,
                                               std::move (font),
                                               dwMetrics,
                                               registry,
                                               scopedCollectionRegistration,
                                               std::move (sanitisedVariables));
    }

    static Ptr from (const Font& f)
    {
        const auto name = f.getTypefaceName();
        const auto style = f.getTypefaceStyle();

        SharedResourcePointer<Direct2DFactories> factories;
        const auto family = factories->getFonts().getFamilyByName (name.toWideCharPointer());

        if (family == nullptr)
            return getLastResortTypeface (f);

        // Try matching the typeface style first
        const auto fonts = AggregateFontCollection::getAllFontsInFamily (family);
        const auto matchingStyle = std::find_if (fonts.begin(), fonts.end(), [&] (const auto& ptr)
        {
            return style.compareIgnoreCase (getFontFaceName (ptr)) == 0;
        });

        if (matchingStyle != fonts.end())
            return fromFont (*matchingStyle, nullptr, &f, MetricsMechanism::dwriteOnly);

        // No matching typeface style, so let dwrite try to find a reasonable substitute
        const auto weight = f.isBold() ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
        const auto italic = f.isItalic() ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

        ComSmartPtr<IDWriteFont> dwFont;

        if (FAILED (family->GetFirstMatchingFont (weight, DWRITE_FONT_STRETCH_NORMAL, italic, dwFont.resetAndGetPointerAddress())) || dwFont == nullptr)
            return {};

        return fromFont (dwFont, nullptr, &f, MetricsMechanism::dwriteOnly);
    }

    static Ptr from (Span<const std::byte> blob)
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

        return fromFont (dwFont,
                         std::make_unique<ScopedCollectionRegistration> (customFontCollection),
                         nullptr,
                         MetricsMechanism::gdiWithDwriteFallback);
    }

    Ptr createSystemFallback (const String& c, const String& language) const override
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

        ComSmartPtr analysisSource { new AnalysisSource (c, language), IncrementRef::no };

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

    const Native* getNativeDetails() const override
    {
        return native.get();
    }

    static Ptr findSystemTypeface()
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

    class ScopedCollectionRegistration
    {
    public:
        explicit ScopedCollectionRegistration (ComSmartPtr<IDWriteFontCollection> x)
            : collection (std::move (x))
        {
            factories->getFonts().addCollection (collection);
        }

        ~ScopedCollectionRegistration()
        {
            factories->getFonts().removeCollection (collection);
        }

    private:
        SharedResourcePointer<Direct2DFactories> factories;
        ComSmartPtr<IDWriteFontCollection> collection;
    };

    WindowsDirectWriteTypeface (ComSmartPtr<IDWriteFont> font,
                                ComSmartPtr<IDWriteFontFace> face,
                                HbFont hbFontIn,
                                TypefaceVerticalMetrics metrics,
                                std::shared_ptr<VariableAxisRegistry> variableAxisRegistry = {},
                                std::shared_ptr<ScopedCollectionRegistration> registrationIn = {},
                                std::vector<FontVariableSetting> settings = {})
        : WindowsDirectWriteTypeface { font,
                                       face,
                                       std::make_unique<Native> (TypefaceNativeOptions { std::move (hbFontIn),
                                                                                         metrics,
                                                                                         std::move (settings),
                                                                                         {},
                                                                                         this,
                                                                                         variableAxisRegistry }),
                                       registrationIn }
    {
    }

    WindowsDirectWriteTypeface (ComSmartPtr<IDWriteFont> font,
                                ComSmartPtr<IDWriteFontFace> face,
                                std::unique_ptr<Native> nativeIn,
                                std::shared_ptr<ScopedCollectionRegistration> registrationIn)
        : Typeface (nativeIn->getTypefaceName(), nativeIn->getTypefaceStyle()),
          dwFont (font),
          dwFontFace (face),
          native (std::move (nativeIn)),
          scopedCollectionRegistration (std::move (registrationIn))
    {
    }

    static TypefaceVerticalMetrics getDwriteMetrics (IDWriteFontFace& face)
    {
        DWRITE_FONT_METRICS dwriteFontMetrics{};
        face.GetMetrics (&dwriteFontMetrics);
        return TypefaceVerticalMetrics { (float) dwriteFontMetrics.ascent  / (float) dwriteFontMetrics.designUnitsPerEm,
                                         (float) dwriteFontMetrics.descent / (float) dwriteFontMetrics.designUnitsPerEm,
                                         (float) dwriteFontMetrics.lineGap / (float) dwriteFontMetrics.designUnitsPerEm };
    }

    static std::optional<TypefaceVerticalMetrics> getGdiMetrics (hb_font_t* font)
    {
        hb_position_t ascent{}, descent{}, lineGap{};

        if (! hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_ASCENT,  &ascent) ||
            ! hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_HORIZONTAL_CLIPPING_DESCENT, &descent))
            return {};

        hb_ot_metrics_get_position (font, HB_OT_METRICS_TAG_VERTICAL_LINE_GAP, &lineGap);

        const auto upem = (float) hb_face_get_upem (hb_font_get_face (font));

        return TypefaceVerticalMetrics { (float) std::abs (ascent) / upem,
                                         (float) std::abs (descent) / upem,
                                         (float) lineGap / upem };
    }

    enum class MetricsMechanism
    {
        dwriteOnly,
        gdiWithDwriteFallback,
    };

    static Ptr fromFont (ComSmartPtr<IDWriteFont> dwFont,
                         std::shared_ptr<ScopedCollectionRegistration> registration,
                         const Font* fontForSynthetics,
                         MetricsMechanism mm)
    {
        ComSmartPtr<IDWriteFontFace> dwFace;

        if (FAILED (dwFont->CreateFontFace (dwFace.resetAndGetPointerAddress())) || dwFace == nullptr)
            return {};

        HbFace hbFace { hb_directwrite_face_create (dwFace), IncrementRef::no };
        HbFont font { hb_font_create (hbFace.get()), IncrementRef::no };

        const auto dwMetrics = getDwriteMetrics (*dwFace);

        const auto metrics = mm == MetricsMechanism::gdiWithDwriteFallback
                           ? getGdiMetrics (font.get()).value_or (dwMetrics)
                           : dwMetrics;

        if (fontForSynthetics != nullptr)
            FontStyleHelpers::initSynthetics (font.get(), *fontForSynthetics);

        return new WindowsDirectWriteTypeface (dwFont,
                                               dwFace,
                                               std::move (font),
                                               metrics,
                                               {},
                                               registration);
    }

    // This attempts to replicate the behaviour of the non-directwrite typeface lookup in JUCE 7 and older
    static Ptr getLastResortTypeface (const Font& font)
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
    std::unique_ptr<Native> native;
    std::shared_ptr<ScopedCollectionRegistration> scopedCollectionRegistration;
};

struct DefaultFontNames
{
    const String defaultSans     { "Verdana" },
                 defaultSerif    { "Times New Roman" },
                 defaultFixed    { "Lucida Console" },
                 defaultFallback { "Tahoma" };  // (contains plenty of unicode characters)
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

auto Typeface::createFromFontImpl (const Font& font) -> Ptr
{
    return WindowsDirectWriteTypeface::from (font);
}

auto Typeface::createFromDataImpl (Span<const std::byte> data) -> Ptr
{
    return WindowsDirectWriteTypeface::from (data);
}

auto Typeface::findSystemTypeface() -> Ptr
{
    return WindowsDirectWriteTypeface::findSystemTypeface();
}

void Typeface::scanFolderForFonts (const File&)
{
    // TODO(reuk)
}

} // namespace juce
