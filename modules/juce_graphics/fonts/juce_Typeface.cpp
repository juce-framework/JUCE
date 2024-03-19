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

class HbScale
{
    static constexpr float factor = 1 << 16;

public:
    HbScale() = delete;

    static constexpr hb_position_t juceToHb (float pos)
    {
        return (hb_position_t) (pos * factor);
    }

    static constexpr float hbToJuce (hb_position_t pos)
    {
        return (float) pos / (float) factor;
    }
};

//==============================================================================
#if JUCE_MAC || JUCE_IOS
template <CTFontOrientation orientation>
void getAdvancesForGlyphs (hb_font_t* hbFont, CTFontRef ctFont, Span<const CGGlyph> glyphs, Span<CGSize> advances)
{
    jassert (glyphs.size() == advances.size());

    int x, y;
    hb_font_get_scale (hbFont, &x, &y);
    const auto scaleAdjustment = HbScale::hbToJuce (orientation == kCTFontOrientationHorizontal ? x : y) / CTFontGetSize (ctFont);

    CTFontGetAdvancesForGlyphs (ctFont, orientation, std::data (glyphs), std::data (advances), (CFIndex) std::size (glyphs));

    for (auto& advance : advances)
        (orientation == kCTFontOrientationHorizontal ? advance.width : advance.height) *= scaleAdjustment;
}

template <CTFontOrientation orientation>
static auto getAdvanceFn()
{
    return [] (hb_font_t* f, void*, hb_codepoint_t glyph, void* voidFontRef) -> hb_position_t
    {
        auto* fontRef = static_cast<CTFontRef> (voidFontRef);

        const CGGlyph glyphs[] { (CGGlyph) glyph };
        CGSize advances[std::size (glyphs)]{};
        getAdvancesForGlyphs<orientation> (f, fontRef, glyphs, advances);

        return HbScale::juceToHb ((float) (orientation == kCTFontOrientationHorizontal ? advances->width : advances->height));
    };
}

template <CTFontOrientation orientation>
static auto getAdvancesFn()
{
    return [] (hb_font_t* f,
               void*,
               unsigned int count,
               const hb_codepoint_t* firstGlyph,
               unsigned int glyphStride,
               hb_position_t* firstAdvance,
               unsigned int advanceStride,
               void* voidFontRef)
    {
        auto* fontRef = static_cast<CTFontRef> (voidFontRef);

        std::vector<CGGlyph> glyphs (count);

        for (auto [index, glyph] : enumerate (glyphs))
            glyph = (CGGlyph) *addBytesToPointer (firstGlyph, glyphStride * index);

        std::vector<CGSize> advances (count);

        getAdvancesForGlyphs<orientation> (f, fontRef, glyphs, advances);

        for (auto [index, advance] : enumerate (advances))
            *addBytesToPointer (firstAdvance, advanceStride * index) = HbScale::juceToHb ((float) (orientation == kCTFontOrientationHorizontal ? advance.width : advance.height));
    };
}

/*  This function overrides the callbacks that fetch glyph advances for fonts on macOS.
    The built-in OpenType glyph metric callbacks that HarfBuzz uses by default for fonts such as
    "Apple Color Emoji" don't always return correct advances, resulting in emoji that may overlap
    with subsequent characters. This may be to do with ignoring the 'trak' table, but I'm not an
    expert, so I'm not sure!

    In any case, using CTFontGetAdvancesForGlyphs produces much nicer advances for emoji on Apple
    platforms, as long as the CTFont is set to the size that will eventually be rendered.

    This might need a bit of testing to make sure that it correctly handles advances for
    custom (non-Apple?) fonts.

    @param hb       a hb_font_t to update with Apple-specific advances
    @param fontRef  the CTFontRef (normally with a custom point size) that will be queried when computing advances
*/
static void overrideCTFontAdvances (hb_font_t* hb, CTFontRef fontRef)
{
    using HbFontFuncs = std::unique_ptr<hb_font_funcs_t, FunctionPointerDestructor<hb_font_funcs_destroy>>;
    const HbFontFuncs funcs { hb_font_funcs_create() };

    // We pass the CTFontRef as user data to each of these functions.
    // We don't pass a custom destructor for the user data, as that will be handled by the custom
    // destructor for the hb_font_funcs_t.
    hb_font_funcs_set_glyph_h_advance_func  (funcs.get(), getAdvanceFn <kCTFontOrientationHorizontal>(), (void*) fontRef, nullptr);
    hb_font_funcs_set_glyph_v_advance_func  (funcs.get(), getAdvanceFn <kCTFontOrientationVertical>(),   (void*) fontRef, nullptr);
    hb_font_funcs_set_glyph_h_advances_func (funcs.get(), getAdvancesFn<kCTFontOrientationHorizontal>(), (void*) fontRef, nullptr);
    hb_font_funcs_set_glyph_v_advances_func (funcs.get(), getAdvancesFn<kCTFontOrientationVertical>(),   (void*) fontRef, nullptr);

    // We want to keep a copy of the font around so that all of our custom callbacks can query it,
    // so retain it here and release it once the custom functions are no longer in use.
    jassert (fontRef != nullptr);
    CFRetain (fontRef);

    hb_font_set_funcs (hb, funcs.get(), (void*) fontRef, [] (void* ptr)
    {
        CFRelease ((CTFontRef) ptr);
    });
}
#endif

struct TypefaceAscentDescent
{
    float ascent{};  // in em units
    float descent{}; // in em units

    float getScaledAscent()  const { return ascent  * getHeightToPointsFactor(); }
    float getScaledDescent() const { return descent * getHeightToPointsFactor(); }

    float getPointsToHeightFactor() const { return ascent + descent; }
    float getHeightToPointsFactor() const { return 1.0f / getPointsToHeightFactor(); }

    TypefaceMetrics getTypefaceMetrics() const
    {
        return { getScaledAscent(), getHeightToPointsFactor() };
    }
};

using HbFont   = std::unique_ptr<hb_font_t, FunctionPointerDestructor<hb_font_destroy>>;
using HbFace   = std::unique_ptr<hb_face_t, FunctionPointerDestructor<hb_face_destroy>>;
using HbBuffer = std::unique_ptr<hb_buffer_t, FunctionPointerDestructor<hb_buffer_destroy>>;
using HbBlob   = std::unique_ptr<hb_blob_t, FunctionPointerDestructor<hb_blob_destroy>>;

class Typeface::Native
{
public:
    Native (hb_font_t* fontRef, TypefaceAscentDescent nonPortableMetricsIn)
        : font (fontRef), nonPortable (nonPortableMetricsIn)
    {
    }

    auto* getFont() const { return font; }

    TypefaceAscentDescent getMetrics (TypefaceMetricsKind kind) const
    {
        switch (kind)
        {
            case TypefaceMetricsKind::legacy:
                return nonPortable;

            case TypefaceMetricsKind::portable:
                return portable;
        }

        return {};
    }

    HbFont getFontAtSizeAndScale (TypefaceMetricsKind kind, float height, float horizontalScale) const
    {
        HbFont subFont { hb_font_create_sub_font (font) };
        const auto points = height * getMetrics (kind).getHeightToPointsFactor();

        hb_font_set_ptem (subFont.get(), points);
        hb_font_set_scale (subFont.get(), HbScale::juceToHb (points * horizontalScale), HbScale::juceToHb (points));

       #if JUCE_MAC || JUCE_IOS
        overrideCTFontAdvances (subFont.get(), hb_coretext_font_get_ct_font (subFont.get()));
       #endif

        return subFont;
    }

private:
    static TypefaceAscentDescent findPortableMetrics (hb_font_t* f, TypefaceAscentDescent fallback)
    {
        hb_font_extents_t extents{};

        if (! hb_font_get_h_extents (f, &extents))
            return fallback;

        const auto ascent  = std::abs ((float) extents.ascender);
        const auto descent = std::abs ((float) extents.descender);
        const auto upem    = (float) hb_face_get_upem (hb_font_get_face (f));

        TypefaceAscentDescent result;
        result.ascent  = ascent  / upem;
        result.descent = descent / upem;
        return result;
    }

    hb_font_t* font = nullptr;

    TypefaceAscentDescent nonPortable;
    TypefaceAscentDescent portable = findPortableMetrics (font, nonPortable);
};

struct FontStyleHelpers
{
    static void initSynthetics (hb_font_t* hb, const Font& font)
    {
        const auto styles = Font::findAllTypefaceStyles (font.getTypefaceName());

        if (styles.contains (font.getTypefaceStyle()))
            return;

        if (font.isItalic())
            hb_font_set_synthetic_slant (hb, 0.1f);

        if (font.isBold())
            hb_font_set_synthetic_bold (hb, 0.04f, 0.04f, true);
    }

    static const char* getStyleName (const bool bold,
                                     const bool italic) noexcept
    {
        if (bold && italic) return "Bold Italic";
        if (bold)           return "Bold";
        if (italic)         return "Italic";
        return "Regular";
    }

    static const char* getStyleName (const int styleFlags) noexcept
    {
        return getStyleName ((styleFlags & Font::bold) != 0,
                             (styleFlags & Font::italic) != 0);
    }

    static bool isBold (const String& style) noexcept
    {
        return style.containsWholeWordIgnoreCase ("Bold");
    }

    static bool isItalic (const String& style) noexcept
    {
        return style.containsWholeWordIgnoreCase ("Italic")
               || style.containsWholeWordIgnoreCase ("Oblique");
    }

    static bool isPlaceholderFamilyName (const String& family)
    {
        return family == Font::getDefaultSansSerifFontName()
            || family == Font::getDefaultSerifFontName()
            || family == Font::getDefaultMonospacedFontName();
    }

    struct ConcreteFamilyNames
    {
        ConcreteFamilyNames()
            : sans  (findName (Font::getDefaultSansSerifFontName())),
              serif (findName (Font::getDefaultSerifFontName())),
              mono  (findName (Font::getDefaultMonospacedFontName()))
        {
        }

        String lookUp (const String& placeholder)
        {
            if (placeholder == Font::getDefaultSansSerifFontName())  return sans;
            if (placeholder == Font::getDefaultSerifFontName())      return serif;
            if (placeholder == Font::getDefaultMonospacedFontName()) return mono;

            return findName (placeholder);
        }

    private:
        static String findName (const String& placeholder)
        {
            const Font f (FontOptions { placeholder, 15.0f, Font::plain });
            return Font::getDefaultTypefaceForFont (f)->getName();
        }

        String sans, serif, mono;
    };

    static String getConcreteFamilyNameFromPlaceholder (const String& placeholder)
    {
        static ConcreteFamilyNames names;
        return names.lookUp (placeholder);
    }

    static String getConcreteFamilyName (const Font& font)
    {
        const String& family = font.getTypefaceName();

        return isPlaceholderFamilyName (family) ? getConcreteFamilyNameFromPlaceholder (family)
                                                : family;
    }

    static HbFace getFaceForBlob (Span<const char> bytes, unsigned int index)
    {
        HbBlob blob { hb_blob_create_or_fail (bytes.data(),
                                              (unsigned int) bytes.size(),
                                              HB_MEMORY_MODE_DUPLICATE,
                                              nullptr,
                                              nullptr) };

        const auto count = hb_face_count (blob.get());

        if (index < count)
            return HbFace { hb_face_create (blob.get(), index) };

        // Attempted to create a font from invalid data. Perhaps the font format was unrecognised.
        jassertfalse;
        return {};
    }
};

//==============================================================================
Typeface::Typeface (const String& faceName, const String& faceStyle) noexcept
    : name (faceName),
      style (faceStyle)
{
}

Typeface::~Typeface() = default;

using HbDrawFuncs = std::unique_ptr<hb_draw_funcs_t, FunctionPointerDestructor<hb_draw_funcs_destroy>>;

static HbDrawFuncs getPathDrawFuncs()
{
    HbDrawFuncs funcs { hb_draw_funcs_create() };

    hb_draw_funcs_set_move_to_func (funcs.get(), [] (auto*, void* data, auto*, float x, float y, auto*)
    {
        auto& path = *static_cast<Path*> (data);
        path.startNewSubPath ({ x, y });
    }, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func (funcs.get(), [] (auto*, void* data, auto*, float x, float y, auto*)
    {
        auto& path = *static_cast<Path*> (data);
        path.lineTo ({ x, y });
    }, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func (funcs.get(), [] (auto*, void* data, auto*, float ctlX, float ctlY, float toX, float toY, auto*)
    {
        auto& path = *static_cast<Path*> (data);
        path.quadraticTo ({ ctlX, ctlY }, { toX, toY });
    }, nullptr, nullptr);
    hb_draw_funcs_set_cubic_to_func (funcs.get(), [] (auto*, void* data, auto*, float ctlX1, float ctlY1, float ctlX2, float ctlY2, float toX, float toY, auto*)
    {
        auto& path = *static_cast<Path*> (data);
        path.cubicTo ({ ctlX1, ctlY1 }, { ctlX2, ctlY2 }, { toX, toY });
    }, nullptr, nullptr);
    hb_draw_funcs_set_close_path_func (funcs.get(), [] (auto*, void* data, auto*, auto*)
    {
        auto& path = *static_cast<Path*> (data);
        path.closeSubPath();
    }, nullptr, nullptr);

    return funcs;
}

[[nodiscard]] static Path getGlyphPathInGlyphUnits (hb_codepoint_t glyph, hb_font_t* font)
{
    static const auto funcs = getPathDrawFuncs();

    Path result;
    hb_font_draw_glyph (font, glyph, funcs.get(), &result);
    return result;
}

void Typeface::getOutlineForGlyph (TypefaceMetricsKind kind, int glyphNumber, Path& path) const
{
    const auto native = getNativeDetails();
    auto* font = native.getFont();
    const auto metrics = native.getMetrics (kind);
    const auto scale = metrics.getHeightToPointsFactor() / (float) hb_face_get_upem (hb_font_get_face (font));

    // getTypefaceGlyph returns glyphs in em space, getOutlineForGlyph returns glyphs in "special JUCE units" space
    path = getGlyphPathInGlyphUnits ((hb_codepoint_t) glyphNumber, getNativeDetails().getFont());
    path.applyTransform (AffineTransform::scale (scale, -scale));
}

void Typeface::applyVerticalHintingTransform (float, Path&)
{
    jassertfalse;
}

EdgeTable* Typeface::getEdgeTableForGlyph (TypefaceMetricsKind kind, int glyphNumber, const AffineTransform& transform, float)
{
    Path path;
    getOutlineForGlyph (kind, glyphNumber, path);
    path.applyTransform (transform);

    return new EdgeTable (path.getBounds().getSmallestIntegerContainer().expanded (1, 0), std::move (path), {});
}

static Colour makeColour (hb_color_t c)
{
    return PixelARGB (hb_color_get_alpha (c),
                      hb_color_get_red (c),
                      hb_color_get_green (c),
                      hb_color_get_blue (c));
}

static std::vector<GlyphLayer> getCOLRv0Layers (const Typeface& typeface, int glyphNumber, const AffineTransform& transform)
{
    auto* font = typeface.getNativeDetails().getFont();
    auto* face = hb_font_get_face (font);
    constexpr auto palette = 0;

    auto numLayers = hb_ot_color_glyph_get_layers (face, (hb_codepoint_t) glyphNumber, 0, nullptr, nullptr);
    std::vector<hb_ot_color_layer_t> layers (numLayers);
    hb_ot_color_glyph_get_layers (face, (hb_codepoint_t) glyphNumber, 0, &numLayers, layers.data());

    if (layers.empty())
        return {};

    std::vector<GlyphLayer> result;

    for (const auto& layer : layers)
    {
        const auto hbFillColour = layer.color_index == 0xffff ? std::optional<hb_color_t>() : [&]
        {
            hb_color_t colour{};
            unsigned int numColours = 1;
            hb_ot_color_palette_get_colors (face, palette, layer.color_index, &numColours, &colour);
            return colour;
        }();

        const auto juceFillColour = hbFillColour.has_value() ? makeColour (*hbFillColour) : std::optional<Colour>();

        auto path = getGlyphPathInGlyphUnits (layer.glyph, font);
        path.applyTransform (transform);
        result.push_back ({ ColourLayer
        {
            EdgeTable { path.getBounds().getSmallestIntegerContainer().expanded (1, 0), path, {} },
            juceFillColour
        } });
    }

    return result;
}

static std::vector<GlyphLayer> getBitmapLayer (const Typeface& typeface, int glyphNumber, const AffineTransform& t)
{
    if ((typeface.getColourGlyphFormats() & Typeface::colourGlyphFormatBitmap) == 0)
        return {};

    auto* font = typeface.getNativeDetails().getFont();

    const HbBlob blob { hb_ot_color_glyph_reference_png (font, (hb_codepoint_t) glyphNumber) };

    unsigned int imageDataSize{};
    const char* imageData = hb_blob_get_data (blob.get(), &imageDataSize);
    const auto juceImage = PNGImageFormat::loadFrom (imageData, imageDataSize);

    if (juceImage.isNull())
        return {};

    hb_glyph_extents_t extents{};
    hb_font_get_glyph_extents (font, (hb_codepoint_t) glyphNumber, &extents);

    const auto wDenom = std::max (1, juceImage.getWidth());
    const auto hDenom = std::max (1, juceImage.getHeight());

    const auto transform = AffineTransform::scale ((float) extents.width  / (float) wDenom,
                                                   (float) extents.height / (float) hDenom)
            .translated ((float) extents.x_bearing,
                         (float) extents.y_bearing)
            .followedBy (t);
    return { GlyphLayer { ImageLayer { juceImage, transform } } };
}

std::vector<GlyphLayer> Typeface::getLayersForGlyph (TypefaceMetricsKind kind, int glyphNumber, const AffineTransform& transform, float) const
{
    auto* font = getNativeDetails().getFont();
    const auto metrics = getNativeDetails().getMetrics (kind);
    const auto scale = metrics.getHeightToPointsFactor() / (float) hb_face_get_upem (hb_font_get_face (font));
    const auto combinedTransform = AffineTransform::scale (scale, -scale).followedBy (transform);

    if (auto bitmapLayer = getBitmapLayer (*this, glyphNumber, combinedTransform); ! bitmapLayer.empty())
        return bitmapLayer;

    // Instead of calling through to the 'paint' API, which JUCE can't easily support due to complex
    // gradients and blend modes, attempt to load COLRv0 layers for the glyph, which we'll be able
    // to render more successfully.
    if (auto layers = getCOLRv0Layers (*this, glyphNumber, combinedTransform); ! layers.empty())
        return layers;

    // No bitmap or COLRv0 for this glyph, so just get a simple monochromatic outline
    auto path = getGlyphPathInGlyphUnits ((hb_codepoint_t) glyphNumber, font);

    if (path.isEmpty())
        return {};

    path.applyTransform (combinedTransform);
    return { GlyphLayer { ColourLayer { EdgeTable { path.getBounds().getSmallestIntegerContainer().expanded (1, 0), path, {} }, {} } } };
}

int Typeface::getColourGlyphFormats() const
{
    auto* face = hb_font_get_face (getNativeDetails().getFont());
    return (hb_ot_color_has_png    (face) ? colourGlyphFormatBitmap : 0)
         | (hb_ot_color_has_svg    (face) ? colourGlyphFormatSvg    : 0)
         | (hb_ot_color_has_layers (face) ? colourGlyphFormatCOLRv0 : 0)
         | (hb_ot_color_has_paint  (face) ? colourGlyphFormatCOLRv1 : 0);
}

TypefaceMetrics Typeface::getMetrics (TypefaceMetricsKind kind) const
{
    return getNativeDetails().getMetrics (kind).getTypefaceMetrics();
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const void* fontFileData, size_t fontFileDataSize)
{
    return createSystemTypefaceFor (Span (static_cast<const std::byte*> (fontFileData), fontFileDataSize));
}

//==============================================================================
std::optional<uint32_t> Typeface::getNominalGlyphForCodepoint (juce_wchar cp) const
{
    auto* font = getNativeDetails().getFont();

    if (font == nullptr)
        return {};

    hb_codepoint_t result{};

    if (! hb_font_get_nominal_glyph (font, static_cast<hb_codepoint_t> (cp), &result))
        return {};

    return result;
}

static constexpr auto hbTag (const char (&arr)[5])
{
    return HB_TAG (arr[0], arr[1], arr[2], arr[3]);
}

template <typename Consumer>
static float doSimpleShapeWithNoBreaks (const Typeface& typeface,
                                        TypefaceMetricsKind kind,
                                        const String& text,
                                        float height,
                                        float horizontalScale,
                                        Consumer&& consumer)
{
    HbBuffer buffer { hb_buffer_create() };
    hb_buffer_add_utf8 (buffer.get(), text.toRawUTF8(), -1, 0, -1);
    hb_buffer_set_cluster_level (buffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
    hb_buffer_guess_segment_properties (buffer.get());

    const auto& native = typeface.getNativeDetails();
    const auto sized = native.getFontAtSizeAndScale (kind, height, horizontalScale);
    auto* font = sized.get();

    // Disable ligatures, because TextEditor requires a 1:1 codepoint/glyph mapping for caret
    // positioning to work as expected.
    // Use an alternative method if you require shaping with ligature support.
    static const std::vector<hb_feature_t> features = []
    {
        std::vector<hb_feature_t> result;

        for (const auto key : { hbTag ("liga"), hbTag ("clig"), hbTag ("hlig"), hbTag ("dlig"), hbTag ("calt") })
            result.push_back (hb_feature_t { key, 0, HB_FEATURE_GLOBAL_START, HB_FEATURE_GLOBAL_END });

        return result;
    }();

    hb_shape (font, buffer.get(), features.data(), (unsigned int) features.size());

    unsigned int numGlyphs{};
    auto* infos = hb_buffer_get_glyph_infos (buffer.get(), &numGlyphs);
    auto* positions = hb_buffer_get_glyph_positions (buffer.get(), &numGlyphs);

    Point<hb_position_t> cursor{};

    for (auto i = decltype (numGlyphs){}; i < numGlyphs; ++i)
    {
        const auto& info = infos[i];
        const auto& position = positions[i];
        consumer (info.codepoint, HbScale::hbToJuce (cursor.x + position.x_offset));
        cursor += Point { position.x_advance, position.y_advance };
    }

    return HbScale::hbToJuce (cursor.x);
}

template <typename Consumer>
static float doSimpleShape (const Typeface& typeface,
                            TypefaceMetricsKind kind,
                            const String& originalText,
                            float height,
                            float horizontalScale,
                            Consumer&& consumer)
{
    const juce_wchar zeroWidthSpace = 0x200b;
    const auto text = originalText.replaceCharacter ('\n', zeroWidthSpace);

    float lastX{};

    for (auto iter = text.begin(), end = text.end(); iter != end;)
    {
        const auto next = [&]
        {
            for (auto i = iter; i != end; ++i)
                if (*i == zeroWidthSpace)
                    return i + 1;

            return end;
        }();

        lastX += doSimpleShapeWithNoBreaks (typeface, kind, String (iter, next), height, horizontalScale, [&] (auto codepoint, auto x)
        {
            consumer (codepoint, lastX + x);
        });
        iter = next;
    }

    return lastX;
}

float Typeface::getStringWidth (TypefaceMetricsKind kind, const String& text, float height, float horizontalScale)
{
    return doSimpleShape (*this, kind, text, height, horizontalScale, [&] (auto, auto) {});
}

void Typeface::getGlyphPositions (TypefaceMetricsKind kind,
                                  const String& text,
                                  Array<int>& glyphs,
                                  Array<float>& xOffsets,
                                  float height,
                                  float horizontalScale)
{
    const auto width = doSimpleShape (*this, kind, text, height, horizontalScale, [&] (auto codepoint, auto xOffset)
    {
        glyphs.add ((int) codepoint);
        xOffsets.add (xOffset);
    });

    xOffsets.add (width);
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class TypefaceTests : public UnitTest
{
public:
    TypefaceTests() : UnitTest ("Typeface", UnitTestCategories::graphics) {}

    void runTest() override
    {
        // If we're running these tests standalone, we want singletons to be cleared before the app
        // exists, so as not to alarm th leak detector.
        const ScopedJuceInitialiser_GUI scope;

        const auto systemNames = getFontFamilyNamesAsSet();

        auto ptr = loadTypeface (FontBinaryData::Karla_Regular_Typo_On_Offsets_Off);
        const auto ptrName = ptr->getName();

        // These tests assume that you don't have a font named "karla" installed.
        beginTest ("Setup");
        {
            expect (systemNames.count (ptr->getName()) == 0);
        }

        beginTest ("Creating a font from memory allows it to be discovered by Font::findAllTypefaceNames()");
        {
            const auto newNames = getFontFamilyNamesAsSet();

            expect (newNames.size() == systemNames.size() + 1);
            expect (newNames.count (ptr->getName()) == 1);
        }

        beginTest ("The available styles of memory fonts can be found");
        {
            const auto styles = Font::findAllTypefaceStyles (ptr->getName());
            expect (styles == StringArray { ptr->getStyle() });
        }

        beginTest ("Typefaces loaded from memory are found when creating font instances by name");
        {
            Font font (FontOptions { ptr->getName(), ptr->getStyle(), 12.0f });

            expect (font.getTypefacePtr() != nullptr);
            expect (font.getTypefacePtr()->getName() == ptr->getName());
            expect (font.getTypefacePtr()->getStyle() == ptr->getStyle());
        }

        // Unload font
        ptr = nullptr;

        beginTest ("After a memory font is no longer referenced, it is not returned from Font::findAllTypefaceNames()");
        {
            const auto newNames = getFontFamilyNamesAsSet();
            expect (newNames == systemNames);
        }

        beginTest ("After a memory font is no longer referenced, it has no styles");
        {
            const auto styles = Font::findAllTypefaceStyles (ptrName);
            expect (styles.isEmpty());
        }
    }

    static std::set<String> getFontFamilyNamesAsSet()
    {
        std::set<String> result;

        for (const auto& name : Font::findAllTypefaceNames())
            result.insert (name);

        return result;
    }

    static Typeface::Ptr loadTypeface (Span<const unsigned char> data)
    {
        return Typeface::createSystemTypefaceFor (data.data(), data.size());
    }
};

static TypefaceTests typefaceTests;

#endif

} // namespace juce
