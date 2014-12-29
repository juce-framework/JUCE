/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

//==================================================================================================
#if JUCE_USE_DIRECTWRITE
namespace DirectWriteTypeLayout
{
    class CustomDirectWriteTextRenderer   : public ComBaseClassHelper<IDWriteTextRenderer>
    {
    public:
        CustomDirectWriteTextRenderer (IDWriteFontCollection* const fonts, const AttributedString& as)
            : ComBaseClassHelper<IDWriteTextRenderer> (0),
              attributedString (as),
              fontCollection (fonts),
              currentLine (-1),
              lastOriginY (-10000.0f)
        {
        }

        JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
        {
            if (refId == __uuidof (IDWritePixelSnapping))
                return castToType <IDWritePixelSnapping> (result);

            return ComBaseClassHelper<IDWriteTextRenderer>::QueryInterface (refId, result);
        }

        JUCE_COMRESULT IsPixelSnappingDisabled (void* /*clientDrawingContext*/, BOOL* isDisabled)
        {
            *isDisabled = FALSE;
            return S_OK;
        }

        JUCE_COMRESULT GetCurrentTransform (void*, DWRITE_MATRIX*)                                          { return S_OK; }
        JUCE_COMRESULT GetPixelsPerDip (void*, FLOAT*)                                                      { return S_OK; }
        JUCE_COMRESULT DrawUnderline (void*, FLOAT, FLOAT, DWRITE_UNDERLINE const*, IUnknown*)              { return S_OK; }
        JUCE_COMRESULT DrawStrikethrough (void*, FLOAT, FLOAT, DWRITE_STRIKETHROUGH const*, IUnknown*)      { return S_OK; }
        JUCE_COMRESULT DrawInlineObject (void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*)  { return E_NOTIMPL; }

        JUCE_COMRESULT DrawGlyphRun (void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE,
                                     DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const* runDescription,
                                     IUnknown* clientDrawingEffect)
        {
            TextLayout* const layout = static_cast<TextLayout*> (clientDrawingContext);

            if (! (baselineOriginY >= -1.0e10f && baselineOriginY <= 1.0e10f))
                baselineOriginY = 0; // DirectWrite sometimes sends NaNs in this parameter

            if (baselineOriginY != lastOriginY)
            {
                lastOriginY = baselineOriginY;
                ++currentLine;

                if (currentLine >= layout->getNumLines())
                {
                    jassert (currentLine == layout->getNumLines());
                    TextLayout::Line* const newLine = new TextLayout::Line();
                    layout->addLine (newLine);

                    newLine->lineOrigin = Point<float> (baselineOriginX, baselineOriginY);
                }
            }

            TextLayout::Line& glyphLine = layout->getLine (currentLine);

            DWRITE_FONT_METRICS dwFontMetrics;
            glyphRun->fontFace->GetMetrics (&dwFontMetrics);

            glyphLine.ascent  = jmax (glyphLine.ascent,  scaledFontSize (dwFontMetrics.ascent,  dwFontMetrics, glyphRun));
            glyphLine.descent = jmax (glyphLine.descent, scaledFontSize (dwFontMetrics.descent, dwFontMetrics, glyphRun));

            TextLayout::Run* const glyphRunLayout = new TextLayout::Run (Range<int> (runDescription->textPosition,
                                                                                     runDescription->textPosition + runDescription->stringLength),
                                                                         glyphRun->glyphCount);
            glyphLine.runs.add (glyphRunLayout);

            glyphRun->fontFace->GetMetrics (&dwFontMetrics);
            const float totalHeight = std::abs ((float) dwFontMetrics.ascent) + std::abs ((float) dwFontMetrics.descent);
            const float fontHeightToEmSizeFactor = (float) dwFontMetrics.designUnitsPerEm / totalHeight;

            glyphRunLayout->font = getFontForRun (glyphRun, glyphRun->fontEmSize / fontHeightToEmSizeFactor);
            glyphRunLayout->colour = getColourOf (static_cast<ID2D1SolidColorBrush*> (clientDrawingEffect));

            const Point<float> lineOrigin (layout->getLine (currentLine).lineOrigin);
            float x = baselineOriginX - lineOrigin.x;

            for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
            {
                const float advance = glyphRun->glyphAdvances[i];

                if ((glyphRun->bidiLevel & 1) != 0)
                    x -= advance;  // RTL text

                glyphRunLayout->glyphs.add (TextLayout::Glyph (glyphRun->glyphIndices[i],
                                                               Point<float> (x, baselineOriginY - lineOrigin.y),
                                                               advance));

                if ((glyphRun->bidiLevel & 1) == 0)
                    x += advance;  // LTR text
            }

            return S_OK;
        }

    private:
        const AttributedString& attributedString;
        IDWriteFontCollection* const fontCollection;
        int currentLine;
        float lastOriginY;

        static float scaledFontSize (int n, const DWRITE_FONT_METRICS& metrics, const DWRITE_GLYPH_RUN* glyphRun) noexcept
        {
            return (std::abs ((float) n) / (float) metrics.designUnitsPerEm) * glyphRun->fontEmSize;
        }

        static Colour getColourOf (ID2D1SolidColorBrush* d2dBrush)
        {
            if (d2dBrush == nullptr)
                return Colours::black;

            const D2D1_COLOR_F colour (d2dBrush->GetColor());
            return Colour::fromFloatRGBA (colour.r, colour.g, colour.b, colour.a);
        }

        Font getFontForRun (DWRITE_GLYPH_RUN const* glyphRun, float fontHeight)
        {
            for (int i = 0; i < attributedString.getNumAttributes(); ++i)
                if (const Font* font = attributedString.getAttribute(i)->getFont())
                    if (WindowsDirectWriteTypeface* wt = dynamic_cast<WindowsDirectWriteTypeface*> (font->getTypeface()))
                        if (wt->getIDWriteFontFace() == glyphRun->fontFace)
                            return font->withHeight (fontHeight);

            ComSmartPtr<IDWriteFont> dwFont;
            HRESULT hr = fontCollection->GetFontFromFontFace (glyphRun->fontFace, dwFont.resetAndGetPointerAddress());
            jassert (dwFont != nullptr);

            ComSmartPtr<IDWriteFontFamily> dwFontFamily;
            hr = dwFont->GetFontFamily (dwFontFamily.resetAndGetPointerAddress());

            return Font (getFontFamilyName (dwFontFamily), getFontFaceName (dwFont), fontHeight);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomDirectWriteTextRenderer)
    };

    //==================================================================================================
    static float getFontHeightToEmSizeFactor (IDWriteFont* const dwFont)
    {
        ComSmartPtr<IDWriteFontFace> dwFontFace;
        dwFont->CreateFontFace (dwFontFace.resetAndGetPointerAddress());

        if (dwFontFace == nullptr)
            return 1.0f;

        DWRITE_FONT_METRICS dwFontMetrics;
        dwFontFace->GetMetrics (&dwFontMetrics);

        const float totalHeight = (float) (std::abs (dwFontMetrics.ascent) + std::abs (dwFontMetrics.descent));
        return dwFontMetrics.designUnitsPerEm / totalHeight;
    }

    void setTextFormatProperties (const AttributedString& text, IDWriteTextFormat* const format)
    {
        DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
        DWRITE_WORD_WRAPPING wrapType = DWRITE_WORD_WRAPPING_WRAP;

        switch (text.getJustification().getOnlyHorizontalFlags())
        {
            case Justification::left:                   break;
            case Justification::right:                  alignment = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
            case Justification::horizontallyCentred:    alignment = DWRITE_TEXT_ALIGNMENT_CENTER; break;
            case Justification::horizontallyJustified:  break; // DirectWrite cannot justify text, default to left alignment
            default:                                    jassertfalse; break; // Illegal justification flags
        }

        switch (text.getWordWrap())
        {
            case AttributedString::none:      wrapType = DWRITE_WORD_WRAPPING_NO_WRAP; break;
            case AttributedString::byWord:    break;
            case AttributedString::byChar:    break; // DirectWrite doesn't support wrapping by character, default to word-wrap
            default:                          jassertfalse; break; // Illegal flags!
        }

        // DirectWrite does not automatically set reading direction
        // This must be set correctly and manually when using RTL Scripts (Hebrew, Arabic)
        if (text.getReadingDirection() == AttributedString::rightToLeft)
        {
            format->SetReadingDirection (DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);

            switch (text.getJustification().getOnlyHorizontalFlags())
            {
                case Justification::left:      alignment = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
                case Justification::right:     alignment = DWRITE_TEXT_ALIGNMENT_LEADING;  break;
                default: break;
            }
        }

        format->SetTextAlignment (alignment);
        format->SetWordWrapping (wrapType);
    }

    void addAttributedRange (const AttributedString::Attribute& attr, IDWriteTextLayout* textLayout,
                             const int textLen, ID2D1RenderTarget* const renderTarget, IDWriteFontCollection* const fontCollection)
    {
        DWRITE_TEXT_RANGE range;
        range.startPosition = attr.range.getStart();
        range.length = jmin (attr.range.getLength(), textLen - attr.range.getStart());

        if (const Font* const font = attr.getFont())
        {
            const String familyName (FontStyleHelpers::getConcreteFamilyName (*font));

            BOOL fontFound = false;
            uint32 fontIndex;
            fontCollection->FindFamilyName (familyName.toWideCharPointer(),
                                            &fontIndex, &fontFound);

            if (! fontFound)
                fontIndex = 0;

            ComSmartPtr<IDWriteFontFamily> fontFamily;
            HRESULT hr = fontCollection->GetFontFamily (fontIndex, fontFamily.resetAndGetPointerAddress());

            ComSmartPtr<IDWriteFont> dwFont;
            uint32 fontFacesCount = 0;
            fontFacesCount = fontFamily->GetFontCount();

            for (int i = fontFacesCount; --i >= 0;)
            {
                hr = fontFamily->GetFont (i, dwFont.resetAndGetPointerAddress());

                if (font->getTypefaceStyle() == getFontFaceName (dwFont))
                    break;
            }

            textLayout->SetFontFamilyName (familyName.toWideCharPointer(), range);
            textLayout->SetFontWeight (dwFont->GetWeight(), range);
            textLayout->SetFontStretch (dwFont->GetStretch(), range);
            textLayout->SetFontStyle (dwFont->GetStyle(), range);

            const float fontHeightToEmSizeFactor = getFontHeightToEmSizeFactor (dwFont);
            textLayout->SetFontSize (font->getHeight() * fontHeightToEmSizeFactor, range);
        }

        if (const Colour* const colour = attr.getColour())
        {
            ComSmartPtr<ID2D1SolidColorBrush> d2dBrush;
            renderTarget->CreateSolidColorBrush (D2D1::ColorF (D2D1::ColorF (colour->getFloatRed(),
                                                                             colour->getFloatGreen(),
                                                                             colour->getFloatBlue(),
                                                                             colour->getFloatAlpha())),
                                                 d2dBrush.resetAndGetPointerAddress());

            // We need to call SetDrawingEffect with a legimate brush to get DirectWrite to break text based on colours
            textLayout->SetDrawingEffect (d2dBrush, range);
        }
    }

    bool setupLayout (const AttributedString& text, const float maxWidth, const float maxHeight,
                      ID2D1RenderTarget* const renderTarget, IDWriteFactory* const directWriteFactory,
                      IDWriteFontCollection* const fontCollection, ComSmartPtr<IDWriteTextLayout>& textLayout)
    {
        // To add color to text, we need to create a D2D render target
        // Since we are not actually rendering to a D2D context we create a temporary GDI render target

        Font defaultFont;
        BOOL fontFound = false;
        uint32 fontIndex;
        fontCollection->FindFamilyName (defaultFont.getTypeface()->getName().toWideCharPointer(), &fontIndex, &fontFound);

        if (! fontFound)
            fontIndex = 0;

        ComSmartPtr<IDWriteFontFamily> dwFontFamily;
        HRESULT hr = fontCollection->GetFontFamily (fontIndex, dwFontFamily.resetAndGetPointerAddress());

        ComSmartPtr<IDWriteFont> dwFont;
        hr = dwFontFamily->GetFirstMatchingFont (DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                                                 dwFont.resetAndGetPointerAddress());

        const float defaultFontHeightToEmSizeFactor = getFontHeightToEmSizeFactor (dwFont);

        jassert (directWriteFactory != nullptr);

        ComSmartPtr<IDWriteTextFormat> dwTextFormat;
        hr = directWriteFactory->CreateTextFormat (defaultFont.getTypefaceName().toWideCharPointer(), fontCollection,
                                                   DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                                   defaultFont.getHeight() * defaultFontHeightToEmSizeFactor,
                                                   L"en-us", dwTextFormat.resetAndGetPointerAddress());

        setTextFormatProperties (text, dwTextFormat);

        {
            DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
            ComSmartPtr<IDWriteInlineObject> trimmingSign;
            hr = directWriteFactory->CreateEllipsisTrimmingSign (dwTextFormat, trimmingSign.resetAndGetPointerAddress());
            hr = dwTextFormat->SetTrimming (&trimming, trimmingSign);
        }

        const int textLen = text.getText().length();

        hr = directWriteFactory->CreateTextLayout (text.getText().toWideCharPointer(), textLen, dwTextFormat,
                                                   maxWidth, maxHeight, textLayout.resetAndGetPointerAddress());

        if (FAILED (hr) || textLayout == nullptr)
            return false;

        const int numAttributes = text.getNumAttributes();

        for (int i = 0; i < numAttributes; ++i)
            addAttributedRange (*text.getAttribute (i), textLayout, textLen, renderTarget, fontCollection);

        return true;
    }

    void createLayout (TextLayout& layout, const AttributedString& text, IDWriteFactory* const directWriteFactory,
                       ID2D1Factory* const direct2dFactory, IDWriteFontCollection* const fontCollection)
    {
        // To add color to text, we need to create a D2D render target
        // Since we are not actually rendering to a D2D context we create a temporary GDI render target

        D2D1_RENDER_TARGET_PROPERTIES d2dRTProp = D2D1::RenderTargetProperties (D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                                                                                D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM,
                                                                                                   D2D1_ALPHA_MODE_IGNORE),
                                                                                0, 0,
                                                                                D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
                                                                                D2D1_FEATURE_LEVEL_DEFAULT);
        ComSmartPtr<ID2D1DCRenderTarget> renderTarget;
        HRESULT hr = direct2dFactory->CreateDCRenderTarget (&d2dRTProp, renderTarget.resetAndGetPointerAddress());

        ComSmartPtr<IDWriteTextLayout> dwTextLayout;

        if (! setupLayout (text, layout.getWidth(), layout.getHeight(), renderTarget,
                           directWriteFactory, fontCollection, dwTextLayout))
            return;

        UINT32 actualLineCount = 0;
        hr = dwTextLayout->GetLineMetrics (nullptr, 0, &actualLineCount);

        layout.ensureStorageAllocated (actualLineCount);

        {
            ComSmartPtr<CustomDirectWriteTextRenderer> textRenderer (new CustomDirectWriteTextRenderer (fontCollection, text));
            hr = dwTextLayout->Draw (&layout, textRenderer, 0, 0);
        }

        HeapBlock<DWRITE_LINE_METRICS> dwLineMetrics (actualLineCount);
        hr = dwTextLayout->GetLineMetrics (dwLineMetrics, actualLineCount, &actualLineCount);
        int lastLocation = 0;
        const int numLines = jmin ((int) actualLineCount, layout.getNumLines());

        for (int i = 0; i < numLines; ++i)
        {
            layout.getLine(i).stringRange = Range<int> (lastLocation, (int) lastLocation + dwLineMetrics[i].length);
            lastLocation += dwLineMetrics[i].length;
        }
    }

    void drawToD2DContext (const AttributedString& text, const Rectangle<float>& area, ID2D1RenderTarget* const renderTarget,
                           IDWriteFactory* const directWriteFactory, IDWriteFontCollection* const fontCollection)
    {
        ComSmartPtr<IDWriteTextLayout> dwTextLayout;

        if (setupLayout (text, area.getWidth(), area.getHeight(), renderTarget,
                         directWriteFactory, fontCollection, dwTextLayout))
        {
            ComSmartPtr<ID2D1SolidColorBrush> d2dBrush;
            renderTarget->CreateSolidColorBrush (D2D1::ColorF (D2D1::ColorF (0.0f, 0.0f, 0.0f, 1.0f)),
                                                 d2dBrush.resetAndGetPointerAddress());

            renderTarget->DrawTextLayout (D2D1::Point2F ((float) area.getX(), (float) area.getY()),
                                          dwTextLayout, d2dBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }
    }
}

static bool canAllTypefacesBeUsedInLayout (const AttributedString& text)
{
    const int numCharacterAttributes = text.getNumAttributes();

    for (int i = 0; i < numCharacterAttributes; ++i)
        if (const Font* const font = text.getAttribute (i)->getFont())
            if (dynamic_cast<WindowsDirectWriteTypeface*> (font->getTypeface()) == nullptr)
                return false;

    return true;
}

#endif

bool TextLayout::createNativeLayout (const AttributedString& text)
{
   #if JUCE_USE_DIRECTWRITE
    if (! canAllTypefacesBeUsedInLayout (text))
        return false;

    SharedResourcePointer<Direct2DFactories> factories;

    if (factories->d2dFactory != nullptr && factories->systemFonts != nullptr)
    {
       #if JUCE_64BIT
        // There's a mysterious bug in 64-bit Windows that causes garbage floating-point
        // values to be returned to DrawGlyphRun the first time that it gets used.
        // In lieu of a better plan, this bodge uses a dummy call to work around this.
        static bool hasBeenCalled = false;
        if (! hasBeenCalled)
        {
            hasBeenCalled = true;
            TextLayout dummy;
            DirectWriteTypeLayout::createLayout (dummy, text, factories->directWriteFactory,
                                                 factories->d2dFactory, factories->systemFonts);
        }
       #endif

        DirectWriteTypeLayout::createLayout (*this, text, factories->directWriteFactory,
                                             factories->d2dFactory, factories->systemFonts);
        return true;
    }
   #else
    (void) text;
   #endif

    return false;
}
