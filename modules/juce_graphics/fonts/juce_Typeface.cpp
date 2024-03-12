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

Rectangle<float> Typeface::getGlyphBounds (TypefaceMetricsKind kind, int glyphNumber) const
{
    auto* font = getNativeDetails().getFont();

    hb_glyph_extents_t extents{};
    if (! hb_font_get_glyph_extents (font, (hb_codepoint_t) glyphNumber, &extents))
        return {};

    const auto native = getNativeDetails();
    const auto metrics = native.getMetrics (kind);
    const auto scale = metrics.getHeightToPointsFactor() / (float) hb_face_get_upem (hb_font_get_face (font));

    return Rectangle { (float) extents.width, (float) extents.height }
            .withPosition ((float) extents.x_bearing, (float) extents.y_bearing)
            .transformedBy (AffineTransform::scale (scale).scaled (1.0f, -1.0f));
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

struct GlyphAdvance
{
    static constexpr auto marshallingVersion = std::nullopt;

    int glyph;
    float advance;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive (named ("g", item.glyph),
                 named ("a", item.advance));
    }
};

struct MetricsRecord
{
    static constexpr auto marshallingVersion = std::nullopt;

    String name;
    int os;
    float ascent;
    float descent;
    float heightToPoints;
    std::vector<GlyphAdvance> positionsA, positionsB;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive (named ("name", item.name),
                 named ("os", item.os),
                 named ("ascent", item.ascent),
                 named ("descent", item.descent),
                 named ("heightToPoints", item.heightToPoints),
                 named ("A", item.positionsA),
                 named ("B", item.positionsB));
    }
};

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

        // The intent of this test is to record the font metrics reported by the system for a few
        // different fonts, so that the same metrics continue to be reported when the font loader
        // mechanism is changed.
        beginTest ("Metrics regression");
        {
           #define JUCE_TEST_TYPEFACES             \
            X (Karla_Regular_Typo_On_Offsets_Off)  \
            X (Karla_Regular_Typo_Off_Offsets_Off)

            const std::map<String, Span<const unsigned char>> typefaceData
            {
               #define X(name) { #name, FontBinaryData::name },
                JUCE_TEST_TYPEFACES
               #undef X
            };

           #undef JUCE_TEST_TYPEFACES

            static constexpr auto generate = false;

            if constexpr (generate)
            {
                DBG ("Copy the following lines into the TypefaceTests::inputs array in juce_Typeface.cpp");

                for (const auto& [typeName, data] : typefaceData)
                {
                    const auto typeface = loadTypeface (data);
                    const auto record = makeMetricsRecord (typeName, typeface);

                    if (const auto converted = ToVar::convert (record))
                        DBG ("R\"(" << JSON::toString (*converted, JSON::FormatOptions{}.withSpacing (JSON::Spacing::none)) << ")\",");
                    else
                        jassertfalse;
                }
            }
            else
            {
                std::vector<MetricsRecord> records;

                for (const auto& input : inputs)
                    if (const auto converted = FromVar::convert<MetricsRecord> (JSON::fromString (input)))
                        records.push_back (*converted);

                std::map<std::tuple<String, int>, MetricsRecord> uniqueRecords;

                for (const auto& record : records)
                    uniqueRecords.emplace (std::tuple (record.name, record.os), record);

                // If this is hit, there are duplicate records for one or more fonts on this platform.
                expect (uniqueRecords.size() == records.size());

                const auto os = getOS();

                for (const auto& [typeName, data] : typefaceData)
                {
                    const auto typeface = loadTypeface (data);
                    const auto iter = uniqueRecords.find ({ typeName, os });

                    if (iter == uniqueRecords.end())
                    {
                        // Missing typeface metrics info for this platform
                        jassertfalse;
                        continue;
                    }

                    const auto& match = iter->second;
                    const auto current = makeMetricsRecord (typeName, typeface);

                    // New ascent/descent values might be slightly different to the old values due
                    // to the order in which multiplications and divisions occur.
                    const auto tolerance = absoluteTolerance (0.00001f);
                    expect (approximatelyEqual (match.ascent,  current.ascent,  tolerance));
                    expect (approximatelyEqual (match.descent, current.descent, tolerance));

                    // The freetype implementation of getHeightToPointsFactor() previously called
                    // through to CustomTypeface, which incorrectly returned the ascent value.
                    expect (os == SystemStats::OperatingSystemType::Linux
                            || approximatelyEqual (match.heightToPoints, current.heightToPoints, tolerance));

                    for (const auto& member : { &MetricsRecord::positionsA, &MetricsRecord::positionsB })
                    {
                        const auto& matchedPositions = match.*member;
                        const auto& currentPositions = current.*member;
                        const auto pair = std::mismatch (matchedPositions.begin(),
                                                         matchedPositions.end(),
                                                         currentPositions.begin(),
                                                         currentPositions.end(),
                                                         &glyphsEqual);
                        expect (pair.first  == matchedPositions.end());
                        expect (pair.second == currentPositions.end());
                    }
                }
            }
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

    static std::vector<GlyphAdvance> getGlyphPositions (Typeface::Ptr typeface, StringRef text)
    {
        Array<int> glyphs;
        Array<float> positions;

        typeface->getGlyphPositions (TypefaceMetricsKind::legacy, text, glyphs, positions);

        std::vector<GlyphAdvance> result;

        for (const auto [index, glyph] : enumerate (glyphs, int{}))
            result.push_back ({ glyph, positions[index + 1] - positions[index] });

        return result;
    }

    static int getOS()
    {
        return SystemStats::getOperatingSystemType()
               & (  SystemStats::OperatingSystemType::Android
                  | SystemStats::OperatingSystemType::Linux
                  | SystemStats::OperatingSystemType::MacOSX
                  | SystemStats::OperatingSystemType::iOS
                  | SystemStats::OperatingSystemType::Windows);
    }

    static MetricsRecord makeMetricsRecord (StringRef name, Typeface::Ptr typeface)
    {
        const auto metrics = typeface->getMetrics (TypefaceMetricsKind::legacy);
        return { name,
                 getOS(),
                 metrics.ascent,
                 1.0f - metrics.ascent,
                 metrics.heightToPoints,
                 getGlyphPositions (typeface, "the quick brown fox jumps over the lazy dog"),
                 getGlyphPositions (typeface, "SPHINX OF BLACK QUARTZ JUDGE MY VOW") };
    }

    static bool glyphsEqual (const GlyphAdvance& a, const GlyphAdvance& b)
    {
        // This tolerance is a bit larger than I'd like, mostly because the advances computed by different
        // Android versions seem to differ a bit. Other platforms may too; it's just especially
        // noticeable on Android.
        return approximatelyEqual (a.advance, b.advance, relativeTolerance (0.02f));
    }

    // Paste generated inputs here. Replace any previous entries for the current platform.
    static constexpr const char* inputs[]
    {
        // Android 29
        R"({"name":"Karla_Regular_Typo_Off_Offsets_Off","os":2048,"ascent":0.798751175403595,"descent":0.2012487947940826,"heightToPoints":0.960614800453186,"A":[{"g":116,"a":0.360230565071106},{"g":104,"a":0.5778698325157166},{"g":101,"a":0.4915645718574524},{"g":32,"a":0.2288964986801147},{"g":113,"a":0.5628602504730225},{"g":117,"a":0.5816223621368408},{"g":105,"a":0.2851824760437012},{"g":99,"a":0.4953169822692871},{"g":107,"a":0.5403459072113037},{"g":32,"a":0.2288961410522461},{"g":98,"a":0.5703654289245605},{"g":114,"a":0.348973274230957},{"g":111,"a":0.5253362655639648},{"g":119,"a":0.6941943168640137},{"g":110,"a":0.5778698921203613},{"g":32,"a":0.2288961410522461},{"g":102,"a":0.3302116394042969},{"g":111,"a":0.5253362655639648},{"g":120,"a":0.4915647506713867},{"g":32,"a":0.2288961410522461},{"g":106,"a":0.3001918792724609},{"g":117,"a":0.5816221237182617},{"g":109,"a":0.8968238830566406},{"g":112,"a":0.5628604888916016},{"g":115,"a":0.5065746307373047},{"g":32,"a":0.2288961410522461},{"g":111,"a":0.5253362655639648},{"g":118,"a":0.4840602874755859},{"g":101,"a":0.4915637969970703},{"g":114,"a":0.3489742279052734},{"g":32,"a":0.2288961410522461},{"g":116,"a":0.3602304458618164},{"g":104,"a":0.5778694152832031},{"g":101,"a":0.4915647506713867},{"g":32,"a":0.2288970947265625},{"g":108,"a":0.270172119140625},{"g":97,"a":0.5290889739990234},{"g":122,"a":0.45404052734375},{"g":121,"a":0.4465351104736328},{"g":32,"a":0.2288970947265625},{"g":100,"a":0.5703659057617188},{"g":111,"a":0.5253353118896484},{"g":103,"a":0.544097900390625}],"B":[{"g":83,"a":0.5816222429275513},{"g":80,"a":0.5328409671783447},{"g":72,"a":0.6416606903076172},{"g":73,"a":0.2701729536056519},{"g":78,"a":0.6566703319549561},{"g":88,"a":0.6153938770294189},{"g":32,"a":0.2288963794708252},{"g":79,"a":0.6191463470458984},{"g":70,"a":0.5028219223022461},{"g":32,"a":0.2288961410522461},{"g":66,"a":0.5966320037841797},{"g":76,"a":0.4502882957458496},{"g":65,"a":0.551602840423584},{"g":67,"a":0.5891270637512207},{"g":75,"a":0.5853748321533203},{"g":32,"a":0.2288966178894043},{"g":81,"a":0.6266512870788574},{"g":85,"a":0.6266508102416992},{"g":65,"a":0.5516023635864258},{"g":82,"a":0.5891275405883789},{"g":84,"a":0.4803075790405273},{"g":90,"a":0.5666122436523438},{"g":32,"a":0.2288970947265625},{"g":74,"a":0.3864965438842773},{"g":85,"a":0.6266517639160156},{"g":68,"a":0.6341552734375},{"g":71,"a":0.6191463470458984},{"g":69,"a":0.5403461456298828},{"g":32,"a":0.2288961410522461},{"g":77,"a":0.8142709732055664},{"g":89,"a":0.5253362655639648},{"g":32,"a":0.2288970947265625},{"g":86,"a":0.5328407287597656},{"g":79,"a":0.6191463470458984},{"g":87,"a":0.8593006134033203}]})",
        R"({"name":"Karla_Regular_Typo_On_Offsets_Off","os":2048,"ascent":0.7691983580589294,"descent":0.2308017015457153,"heightToPoints":0.8438819050788879,"A":[{"g":116,"a":0.3164557218551636},{"g":104,"a":0.507647693157196},{"g":101,"a":0.4318302273750305},{"g":32,"a":0.2010811567306519},{"g":113,"a":0.4944621324539185},{"g":117,"a":0.5109440088272095},{"g":105,"a":0.2505276203155518},{"g":99,"a":0.4351265430450439},{"g":107,"a":0.4746835231781006},{"g":32,"a":0.2010812759399414},{"g":98,"a":0.5010550022125244},{"g":114,"a":0.3065662384033203},{"g":111,"a":0.4614977836608887},{"g":119,"a":0.6098365783691406},{"g":110,"a":0.5076479911804199},{"g":32,"a":0.2010812759399414},{"g":102,"a":0.2900843620300293},{"g":111,"a":0.4614977836608887},{"g":120,"a":0.4318304061889648},{"g":32,"a":0.2010812759399414},{"g":106,"a":0.2637128829956055},{"g":117,"a":0.5109443664550781},{"g":109,"a":0.7878427505493164},{"g":112,"a":0.4944620132446289},{"g":115,"a":0.4450159072875977},{"g":32,"a":0.2010812759399414},{"g":111,"a":0.4614973068237305},{"g":118,"a":0.4252376556396484},{"g":101,"a":0.4318304061889648},{"g":114,"a":0.3065662384033203},{"g":32,"a":0.2010812759399414},{"g":116,"a":0.3164558410644531},{"g":104,"a":0.5076475143432617},{"g":101,"a":0.4318304061889648},{"g":32,"a":0.2010812759399414},{"g":108,"a":0.2373418807983398},{"g":97,"a":0.4647941589355469},{"g":122,"a":0.3988656997680664},{"g":121,"a":0.3922739028930664},{"g":32,"a":0.2010812759399414},{"g":100,"a":0.5010547637939453},{"g":111,"a":0.4614973068237305},{"g":103,"a":0.4779796600341797}],"B":[{"g":83,"a":0.510944128036499},{"g":80,"a":0.4680907130241394},{"g":72,"a":0.5636867880821228},{"g":73,"a":0.2373417615890503},{"g":78,"a":0.5768723487854004},{"g":88,"a":0.5406119823455811},{"g":32,"a":0.2010810375213623},{"g":79,"a":0.5439083576202393},{"g":70,"a":0.4417195320129395},{"g":32,"a":0.2010812759399414},{"g":66,"a":0.5241298675537109},{"g":76,"a":0.3955693244934082},{"g":65,"a":0.4845728874206543},{"g":67,"a":0.5175371170043945},{"g":75,"a":0.5142402648925781} ,{"g":32,"a":0.2010812759399414},{"g":81,"a":0.5505013465881348},{"g":85,"a":0.5505008697509766},{"g":65,"a":0.4845724105834961},{"g":82,"a":0.5175371170043945},{"g":84,"a":0.421940803527832},{"g":90,"a":0.4977588653564453},{"g":32,"a":0.2010812759399414},{"g":74,"a":0.3395309448242188},{"g":85,"a":0.5505008697509766},{"g":68,"a":0.557093620300293},{"g":71,"a":0.5439081192016602},{"g":69,"a":0.4746837615966797},{"g":32,"a":0.2010812759399414},{"g":77,"a":0.7153215408325195},{"g":89,"a":0.4614982604980469},{"g":32,"a":0.2010812759399414},{"g":86,"a":0.4680910110473633},{"g":79,"a":0.5439081192016602},{"g":87,"a":0.7548789978027344}]})",

        // macOS
        R"({"name":"Karla_Regular_Typo_Off_Offsets_Off","os":256,"ascent":0.7987481951713562,"descent":0.2012518048286438,"heightToPoints":0.9606144428253174,"A":[{"g":88,"a":0.3611910045146942},{"g":76,"a":0.5797308683395386},{"g":73,"a":0.4932755827903748},{"g":4,"a":0.2281458377838135},{"g":85,"a":0.5614793300628662},{"g":89,"a":0.5816519260406494},{"g":77,"a":0.2862632274627686},{"g":71,"a":0.4956769943237305},{"g":79,"a":0.5413064956665039},{"g":4,"a":0.2281460762023926},{"g":70,"a":0.5691637992858887},{"g":86,"a":0.347261905670166},{"g":83,"a":0.5249757766723633},{"g":91,"a":0.6959657669067383},{"g":82,"a":0.5797305107116699},{"g":4,"a":0.2281460762023926},{"g":74,"a":0.3290104866027832},{"g":83,"a":0.5249757766723633},{"g":92,"a":0.4913549423217773},{"g":4,"a":0.2281455993652344},{"g":78,"a":0.3011531829833984},{"g":89,"a":0.5816526412963867},{"g":81,"a":0.8986549377441406},{"g":84,"a":0.5614786148071289},{"g":87,"a":0.5048036575317383},{"g":4,"a":0.2281455993652344},{"g":83,"a":0.5249767303466797},{"g":90,"a":0.4827079772949219},{"g":73,"a":0.4932756423950195},{"g":86,"a":0.3472623825073242},{"g":4,"a":0.2281455993652344},{"g":88,"a":0.3611917495727539},{"g":76,"a":0.5797309875488281},{"g":73,"a":0.4932746887207031},{"g":4,"a":0.2281455993652344},{"g":80,"a":0.2689723968505859},{"g":69,"a":0.5288190841674805},{"g":94,"a":0.4558124542236328},{"g":93,"a":0.4471664428710938},{"g":4,"a":0.2281436920166016},{"g":72,"a":0.5691661834716797},{"g":83,"a":0.5249748229980469},{"g":75,"a":0.542266845703125}],"B":[{"g":55,"a":0.580691397190094},{"g":52,"a":0.531700074672699},{"g":44,"a":0.642170786857605},{"g":45,"a":0.2689720392227173},{"g":50,"a":0.6570601463317871},{"g":60,"a":0.616234302520752},{"g":4,"a":0.2281458377838135},{"g":51,"a":0.6200768947601318},{"g":42,"a":0.5033621788024902},{"g":4,"a":0.2281460762023926},{"g":38,"a":0.5970215797424316},{"g":48,"a":0.4490876197814941},{"g":37,"a":0.5518732070922852},{"g":39,"a":0.5888566970825195},{"g":47,"a":0.5835733413696289},{"g":4,"a":0.2281460762023926},{"g":53,"a":0.6258401870727539},{"g":57,"a":0.6272811889648438},{"g":37,"a":0.5518722534179688},{"g":54,"a":0.5883769989013672},{"g":56,"a":0.4807872772216797},{"g":62,"a":0.5682039260864258},{"g":4,"a":0.2281455993652344},{"g":46,"a":0.3876075744628906},{"g":57,"a":0.6272811889648438},{"g":40,"a":0.6340055465698242},{"g":43,"a":0.6186361312866211},{"g":41,"a":0.5398654937744141},{"g":4,"a":0.2281455993652344},{"g":49,"a":0.8136405944824219},{"g":61,"a":0.5268974304199219},{"g":4,"a":0.2281455993652344},{"g":58,"a":0.5326614379882812},{"g":51,"a":0.6200752258300781},{"g":59,"a":0.8587894439697266}]})",
        R"({"name":"Karla_Regular_Typo_On_Offsets_Off","os":256,"ascent":0.7987481951713562,"descent":0.2012518048286438,"heightToPoints":0.9606144428253174,"A":[{"g":88,"a":0.3611910045146942},{"g":76,"a":0.5797308683395386},{"g":73,"a":0.4932755827903748},{"g":4,"a":0.2281458377838135},{"g":85,"a":0.5614793300628662},{"g":89,"a":0.5816519260406494},{"g":77,"a":0.2862632274627686},{"g":71,"a":0.4956769943237305},{"g":79,"a":0.5413064956665039},{"g":4,"a":0.2281460762023926},{"g":70,"a":0.5691637992858887},{"g":86,"a":0.347261905670166},{"g":83,"a":0.5249757766723633},{"g":91,"a":0.6959657669067383},{"g":82,"a":0.5797305107116699},{"g":4,"a":0.2281460762023926},{"g":74,"a":0.3290104866027832},{"g":83,"a":0.5249757766723633},{"g":92,"a":0.4913549423217773},{"g":4,"a":0.2281455993652344},{"g":78,"a":0.3011531829833984},{"g":89,"a":0.5816526412963867},{"g":81,"a":0.8986549377441406},{"g":84,"a":0.5614786148071289},{"g":87,"a":0.5048036575317383},{"g":4,"a":0.2281455993652344},{"g":83,"a":0.5249767303466797},{"g":90,"a":0.4827079772949219},{"g":73,"a":0.4932756423950195},{"g":86,"a":0.3472623825073242},{"g":4,"a":0.2281455993652344},{"g":88,"a":0.3611917495727539},{"g":76,"a":0.5797309875488281},{"g":73,"a":0.4932746887207031},{"g":4,"a":0.2281455993652344},{"g":80,"a":0.2689723968505859},{"g":69,"a":0.5288190841674805},{"g":94,"a":0.4558124542236328},{"g":93,"a":0.4471664428710938},{"g":4,"a":0.2281436920166016},{"g":72,"a":0.5691661834716797},{"g":83,"a":0.5249748229980469},{"g":75,"a":0.542266845703125}],"B":[{"g":55,"a":0.580691397190094},{"g":52,"a":0.531700074672699},{"g":44,"a":0.642170786857605},{"g":45,"a":0.2689720392227173},{"g":50,"a":0.6570601463317871},{"g":60,"a":0.616234302520752},{"g":4,"a":0.2281458377838135},{"g":51,"a":0.6200768947601318},{"g":42,"a":0.5033621788024902},{"g":4,"a":0.2281460762023926},{"g":38,"a":0.5970215797424316},{"g":48,"a":0.4490876197814941},{"g":37,"a":0.5518732070922852},{"g":39,"a":0.5888566970825195},{"g":47,"a":0.5835733413696289},{"g":4,"a":0.2281460762023926},{"g":53,"a":0.6258401870727539},{"g":57,"a":0.6272811889648438},{"g":37,"a":0.5518722534179688},{"g":54,"a":0.5883769989013672},{"g":56,"a":0.4807872772216797},{"g":62,"a":0.5682039260864258},{"g":4,"a":0.2281455993652344},{"g":46,"a":0.3876075744628906},{"g":57,"a":0.6272811889648438},{"g":40,"a":0.6340055465698242},{"g":43,"a":0.6186361312866211},{"g":41,"a":0.5398654937744141},{"g":4,"a":0.2281455993652344},{"g":49,"a":0.8136405944824219},{"g":61,"a":0.5268974304199219},{"g":4,"a":0.2281455993652344},{"g":58,"a":0.5326614379882812},{"g":51,"a":0.6200752258300781},{"g":59,"a":0.8587894439697266}]})",

        // iOS
        R"({"name":"Karla_Regular_Typo_Off_Offsets_Off","os":4096,"ascent":0.798751175403595,"descent":0.201248824596405,"heightToPoints":0.960614800453186,"A":[{"g":88,"a":0.3611911535263062},{"g":76,"a":0.5797310471534729},{"g":73,"a":0.4932757019996643},{"g":4,"a":0.228145956993103},{"g":85,"a":0.5614794492721558},{"g":89,"a":0.5816521644592285},{"g":77,"a":0.2862634658813477},{"g":71,"a":0.4956772327423096},{"g":79,"a":0.5413064956665039},{"g":4,"a":0.2281460762023926},{"g":70,"a":0.5691642761230469},{"g":86,"a":0.347261905670166},{"g":83,"a":0.5249762535095215},{"g":91,"a":0.6959657669067383},{"g":82,"a":0.5797309875488281},{"g":4,"a":0.2281460762023926},{"g":74,"a":0.3290104866027832},{"g":83,"a":0.5249757766723633},{"g":92,"a":0.4913549423217773},{"g":4,"a":0.2281465530395508},{"g":78,"a":0.3011531829833984},{"g":89,"a":0.5816516876220703},{"g":81,"a":0.898655891418457},{"g":84,"a":0.5614795684814453},{"g":87,"a":0.5048036575317383},{"g":4,"a":0.2281455993652344},{"g":83,"a":0.5249767303466797},{"g":90,"a":0.4827079772949219},{"g":73,"a":0.4932756423950195},{"g":86,"a":0.3472623825073242},{"g":4,"a":0.2281465530395508},{"g":88,"a":0.3611917495727539},{"g":76,"a":0.5797309875488281},{"g":73,"a":0.4932746887207031},{"g":4,"a":0.2281455993652344},{"g":80,"a":0.2689723968505859},{"g":69,"a":0.5288190841674805},{"g":94,"a":0.4558124542236328},{"g":93,"a":0.4471664428710938},{"g":4,"a":0.2281455993652344},{"g":72,"a":0.5691642761230469},{"g":83,"a":0.5249748229980469},{"g":75,"a":0.5422687530517578}],"B":[{"g":55,"a":0.5806916356086731},{"g":52,"a":0.5317003130912781},{"g":44,"a":0.6421709060668945},{"g":45,"a":0.2689721584320068},{"g":50,"a":0.6570606231689453},{"g":60,"a":0.616234302520752},{"g":4,"a":0.2281458377838135},{"g":51,"a":0.6200771331787109},{"g":42,"a":0.5033621788024902},{"g":4,"a":0.2281465530395508},{"g":38,"a":0.5970220565795898},{"g":48,"a":0.4490876197814941},{"g":37,"a":0.5518732070922852},{"g":39,"a":0.5888566970825195},{"g":47,"a":0.5835738182067871},{"g":4,"a":0.2281460762023926},{"g":53,"a":0.6258401870727539},{"g":57,"a":0.6272811889648438},{"g":37,"a":0.5518732070922852},{"g":54,"a":0.5883769989013672},{"g":56,"a":0.4807872772216797},{"g":62,"a":0.5682039260864258},{"g":4,"a":0.2281465530395508},{"g":46,"a":0.3876075744628906},{"g":57,"a":0.6272811889648438},{"g":40,"a":0.6340055465698242},{"g":43,"a":0.6186361312866211},{"g":41,"a":0.5398654937744141},{"g":4,"a":0.2281465530395508},{"g":49,"a":0.8136405944824219},{"g":61,"a":0.5268974304199219},{"g":4,"a":0.2281455993652344},{"g":58,"a":0.5326614379882812},{"g":51,"a":0.6200771331787109},{"g":59,"a":0.8587875366210938}]})",
        R"({"name":"Karla_Regular_Typo_On_Offsets_Off","os":4096,"ascent":0.798751175403595,"descent":0.201248824596405,"heightToPoints":0.960614800453186,"A":[{"g":88,"a":0.3611911535263062},{"g":76,"a":0.5797310471534729},{"g":73,"a":0.4932757019996643},{"g":4,"a":0.228145956993103},{"g":85,"a":0.5614794492721558},{"g":89,"a":0.5816521644592285},{"g":77,"a":0.2862634658813477},{"g":71,"a":0.4956772327423096},{"g":79,"a":0.5413064956665039},{"g":4,"a":0.2281460762023926},{"g":70,"a":0.5691642761230469},{"g":86,"a":0.347261905670166},{"g":83,"a":0.5249762535095215},{"g":91,"a":0.6959657669067383},{"g":82,"a":0.5797309875488281},{"g":4,"a":0.2281460762023926},{"g":74,"a":0.3290104866027832},{"g":83,"a":0.5249757766723633},{"g":92,"a":0.4913549423217773},{"g":4,"a":0.2281465530395508},{"g":78,"a":0.3011531829833984},{"g":89,"a":0.5816516876220703},{"g":81,"a":0.898655891418457},{"g":84,"a":0.5614795684814453},{"g":87,"a":0.5048036575317383},{"g":4,"a":0.2281455993652344},{"g":83,"a":0.5249767303466797},{"g":90,"a":0.4827079772949219},{"g":73,"a":0.4932756423950195},{"g":86,"a":0.3472623825073242},{"g":4,"a":0.2281465530395508},{"g":88,"a":0.3611917495727539},{"g":76,"a":0.5797309875488281},{"g":73,"a":0.4932746887207031},{"g":4,"a":0.2281455993652344},{"g":80,"a":0.2689723968505859},{"g":69,"a":0.5288190841674805},{"g":94,"a":0.4558124542236328},{"g":93,"a":0.4471664428710938},{"g":4,"a":0.2281455993652344},{"g":72,"a":0.5691642761230469},{"g":83,"a":0.5249748229980469},{"g":75,"a":0.5422687530517578}],"B":[{"g":55,"a":0.5806916356086731},{"g":52,"a":0.5317003130912781},{"g":44,"a":0.6421709060668945},{"g":45,"a":0.2689721584320068},{"g":50,"a":0.6570606231689453},{"g":60,"a":0.616234302520752},{"g":4,"a":0.2281458377838135},{"g":51,"a":0.6200771331787109},{"g":42,"a":0.5033621788024902},{"g":4,"a":0.2281465530395508},{"g":38,"a":0.5970220565795898},{"g":48,"a":0.4490876197814941},{"g":37,"a":0.5518732070922852},{"g":39,"a":0.5888566970825195},{"g":47,"a":0.5835738182067871},{"g":4,"a":0.2281460762023926},{"g":53,"a":0.6258401870727539},{"g":57,"a":0.6272811889648438},{"g":37,"a":0.5518732070922852},{"g":54,"a":0.5883769989013672},{"g":56,"a":0.4807872772216797},{"g":62,"a":0.5682039260864258},{"g":4,"a":0.2281465530395508},{"g":46,"a":0.3876075744628906},{"g":57,"a":0.6272811889648438},{"g":40,"a":0.6340055465698242},{"g":43,"a":0.6186361312866211},{"g":41,"a":0.5398654937744141},{"g":4,"a":0.2281465530395508},{"g":49,"a":0.8136405944824219},{"g":61,"a":0.5268974304199219},{"g":4,"a":0.2281455993652344},{"g":58,"a":0.5326614379882812},{"g":51,"a":0.6200771331787109},{"g":59,"a":0.8587875366210938}]})",

        // Windows
        R"({"name":"Karla_Regular_Typo_Off_Offsets_Off","os":512,"ascent":0.7509419918060303,"descent":0.2490580081939697,"heightToPoints":0.7535794973373413,"A":[{"g":88,"a":0.2833458781242371},{"g":76,"a":0.4547852277755737},{"g":73,"a":0.3869630694389343},{"g":4,"a":0.1789751052856445},{"g":85,"a":0.4404672384262085},{"g":89,"a":0.4562925100326538},{"g":77,"a":0.2245666980743408},{"g":71,"a":0.3888471126556396},{"g":79,"a":0.4246420860290527},{"g":4,"a":0.1789751052856445},{"g":70,"a":0.4464957714080811},{"g":86,"a":0.2724192142486572},{"g":83,"a":0.4118313789367676},{"g":91,"a":0.5459685325622559},{"g":82,"a":0.4547853469848633},{"g":4,"a":0.1789751052856445},{"g":74,"a":0.2581009864807129},{"g":83,"a":0.4118313789367676},{"g":92,"a":0.3854560852050781},{"g":4,"a":0.1789751052856445},{"g":78,"a":0.2362470626831055},{"g":89,"a":0.4562921524047852},{"g":81,"a":0.7049732208251953},{"g":84,"a":0.4404668807983398},{"g":87,"a":0.3960056304931641},{"g":4,"a":0.1789751052856445},{"g":83,"a":0.4118309020996094},{"g":90,"a":0.3786735534667969},{"g":73,"a":0.386962890625},{"g":86,"a":0.2724189758300781},{"g":4,"a":0.1789751052856445},{"g":88,"a":0.2833461761474609},{"g":76,"a":0.4547853469848633},{"g":73,"a":0.386962890625},{"g":4,"a":0.1789751052856445},{"g":80,"a":0.2110023498535156},{"g":69,"a":0.4148454666137695},{"g":94,"a":0.3575735092163086},{"g":93,"a":0.3507909774780273},{"g":4,"a":0.1789751052856445},{"g":72,"a":0.4464960098266602},{"g":83,"a":0.4118309020996094},{"g":75,"a":0.4253959655761719}],"B":[{"g":55,"a":0.455538809299469},{"g":52,"a":0.4171062111854553},{"g":44,"a":0.5037678480148315},{"g":45,"a":0.2110022306442261},{"g":50,"a":0.5154484510421753},{"g":60,"a":0.4834213256835938},{"g":4,"a":0.1789751052856445},{"g":51,"a":0.4864356517791748},{"g":42,"a":0.3948755264282227},{"g":4,"a":0.1789751052856445},{"g":38,"a":0.4683494567871094},{"g":48,"a":0.3522982597351074},{"g":37,"a":0.4329314231872559},{"g":39,"a":0.4619441032409668},{"g":47,"a":0.4577994346618652},{"g":4,"a":0.1789751052856445},{"g":53,"a":0.4909572601318359},{"g":57,"a":0.4920873641967773},{"g":37,"a":0.4329314231872559},{"g":54,"a":0.4615674018859863},{"g":56,"a":0.377166748046875},{"g":62,"a":0.4457426071166992},{"g":4,"a":0.1789751052856445},{"g":46,"a":0.3040695190429688},{"g":57,"a":0.4920873641967773},{"g":40,"a":0.4973621368408203},{"g":43,"a":0.4853048324584961},{"g":41,"a":0.4235115051269531},{"g":4,"a":0.1789751052856445},{"g":49,"a":0.6382818222045898},{"g":61,"a":0.4133386611938477},{"g":4,"a":0.1789751052856445},{"g":58,"a":0.4238882064819336},{"g":51,"a":0.4864358901977539},{"g":59,"a":0.6737003326416016}]})",
        R"({"name":"Karla_Regular_Typo_On_Offsets_Off","os":512,"ascent":0.7509419918060303,"descent":0.2490580081939697,"heightToPoints":0.7535794973373413,"A":[{"g":88,"a":0.2833458781242371},{"g":76,"a":0.4547852277755737},{"g":73,"a":0.3869630694389343},{"g":4,"a":0.1789751052856445},{"g":85,"a":0.4404672384262085},{"g":89,"a":0.4562925100326538},{"g":77,"a":0.2245666980743408},{"g":71,"a":0.3888471126556396},{"g":79,"a":0.4246420860290527},{"g":4,"a":0.1789751052856445},{"g":70,"a":0.4464957714080811},{"g":86,"a":0.2724192142486572},{"g":83,"a":0.4118313789367676},{"g":91,"a":0.5459685325622559},{"g":82,"a":0.4547853469848633},{"g":4,"a":0.1789751052856445},{"g":74,"a":0.2581009864807129},{"g":83,"a":0.4118313789367676},{"g":92,"a":0.3854560852050781},{"g":4,"a":0.1789751052856445},{"g":78,"a":0.2362470626831055},{"g":89,"a":0.4562921524047852},{"g":81,"a":0.7049732208251953},{"g":84,"a":0.4404668807983398},{"g":87,"a":0.3960056304931641},{"g":4,"a":0.1789751052856445},{"g":83,"a":0.4118309020996094},{"g":90,"a":0.3786735534667969},{"g":73,"a":0.386962890625},{"g":86,"a":0.2724189758300781},{"g":4,"a":0.1789751052856445},{"g":88,"a":0.2833461761474609},{"g":76,"a":0.4547853469848633},{"g":73,"a":0.386962890625},{"g":4,"a":0.1789751052856445},{"g":80,"a":0.2110023498535156},{"g":69,"a":0.4148454666137695},{"g":94,"a":0.3575735092163086},{"g":93,"a":0.3507909774780273},{"g":4,"a":0.1789751052856445},{"g":72,"a":0.4464960098266602},{"g":83,"a":0.4118309020996094},{"g":75,"a":0.4253959655761719}],"B":[{"g":55,"a":0.455538809299469},{"g":52,"a":0.4171062111854553},{"g":44,"a":0.5037678480148315},{"g":45,"a":0.2110022306442261},{"g":50,"a":0.5154484510421753},{"g":60,"a":0.4834213256835938},{"g":4,"a":0.1789751052856445},{"g":51,"a":0.4864356517791748},{"g":42,"a":0.3948755264282227},{"g":4,"a":0.1789751052856445},{"g":38,"a":0.4683494567871094},{"g":48,"a":0.3522982597351074},{"g":37,"a":0.4329314231872559},{"g":39,"a":0.4619441032409668},{"g":47,"a":0.4577994346618652},{"g":4,"a":0.1789751052856445},{"g":53,"a":0.4909572601318359},{"g":57,"a":0.4920873641967773},{"g":37,"a":0.4329314231872559},{"g":54,"a":0.4615674018859863},{"g":56,"a":0.377166748046875},{"g":62,"a":0.4457426071166992},{"g":4,"a":0.1789751052856445},{"g":46,"a":0.3040695190429688},{"g":57,"a":0.4920873641967773},{"g":40,"a":0.4973621368408203},{"g":43,"a":0.4853048324584961},{"g":41,"a":0.4235115051269531},{"g":4,"a":0.1789751052856445},{"g":49,"a":0.6382818222045898},{"g":61,"a":0.4133386611938477},{"g":4,"a":0.1789751052856445},{"g":58,"a":0.4238882064819336},{"g":51,"a":0.4864358901977539},{"g":59,"a":0.6737003326416016}]})",

        // Linux (Fedora 39)
        R"({"name":"Karla_Regular_Typo_Off_Offsets_Off","os":1024,"ascent":0.798751175403595,"descent":0.201248824596405,"heightToPoints":0.798751175403595,"A":[{"g":116,"a":0.3611911535263062},{"g":104,"a":0.5797309875488281},{"g":101,"a":0.4932756423950195},{"g":32,"a":0.228145956993103},{"g":113,"a":0.5614793300628662},{"g":117,"a":0.5816521644592285},{"g":105,"a":0.2862632274627686},{"g":99,"a":0.4956772327423096},{"g":107,"a":0.5413064956665039},{"g":32,"a":0.2281460762023926},{"g":98,"a":0.5691642761230469},{"g":114,"a":0.3472623825073242},{"g":111,"a":0.5249757766723633},{"g":119,"a":0.6959652900695801},{"g":110,"a":0.5797309875488281},{"g":32,"a":0.2281460762023926},{"g":102,"a":0.3290104866027832},{"g":111,"a":0.5249757766723633},{"g":120,"a":0.4913539886474609},{"g":32,"a":0.2281455993652344},{"g":106,"a":0.3011531829833984},{"g":117,"a":0.5816526412963867},{"g":109,"a":0.8986549377441406},{"g":112,"a":0.5614795684814453},{"g":115,"a":0.5048027038574219},{"g":32,"a":0.2281455993652344},{"g":111,"a":0.5249757766723633},{"g":118,"a":0.4827089309692383},{"g":101,"a":0.4932756423950195},{"g":114,"a":0.3472623825073242},{"g":32,"a":0.2281455993652344},{"g":116,"a":0.3611907958984375},{"g":104,"a":0.5797309875488281},{"g":101,"a":0.4932756423950195},{"g":32,"a":0.2281455993652344},{"g":108,"a":0.2689723968505859},{"g":97,"a":0.5288181304931641},{"g":122,"a":0.4558124542236328},{"g":121,"a":0.4471664428710938},{"g":32,"a":0.2281455993652344},{"g":100,"a":0.5691642761230469},{"g":111,"a":0.5249767303466797},{"g":103,"a":0.542266845703125}],"B":[{"g":83,"a":0.5806916356086731},{"g":80,"a":0.5317003130912781},{"g":72,"a":0.6421709060668945},{"g":73,"a":0.2689721584320068},{"g":78,"a":0.6570603847503662},{"g":88,"a":0.616234302520752},{"g":32,"a":0.2281460762023926},{"g":79,"a":0.6200766563415527},{"g":70,"a":0.5033621788024902},{"g":32,"a":0.2281460762023926},{"g":66,"a":0.5970220565795898},{"g":76,"a":0.4490876197814941},{"g":65,"a":0.5518732070922852},{"g":67,"a":0.5888566970825195},{"g":75,"a":0.5835733413696289},{"g":32,"a":0.2281460762023926},{"g":81,"a":0.6258406639099121},{"g":85,"a":0.6272811889648438},{"g":65,"a":0.5518732070922852},{"g":82,"a":0.5883769989013672},{"g":84,"a":0.4807872772216797},{"g":90,"a":0.5682039260864258},{"g":32,"a":0.2281455993652344},{"g":74,"a":0.387608528137207},{"g":85,"a":0.6272811889648438},{"g":68,"a":0.6340055465698242},{"g":71,"a":0.6186361312866211},{"g":69,"a":0.5398654937744141},{"g":32,"a":0.2281455993652344},{"g":77,"a":0.8136405944824219},{"g":89,"a":0.5268974304199219},{"g":32,"a":0.2281455993652344},{"g":86,"a":0.5403461456298828},{"g":79,"a":0.6200771331787109},{"g":87,"a":0.8587894439697266}]})",
        R"({"name":"Karla_Regular_Typo_On_Offsets_Off","os":1024,"ascent":0.7691982984542847,"descent":0.2308017015457153,"heightToPoints":0.7691982984542847,"A":[{"g":116,"a":0.3172995746135712},{"g":104,"a":0.5092827081680298},{"g":101,"a":0.4333332777023315},{"g":32,"a":0.200421929359436},{"g":113,"a":0.4932489395141602},{"g":117,"a":0.5109704732894897},{"g":105,"a":0.2514767646789551},{"g":99,"a":0.4354431629180908},{"g":107,"a":0.4755275249481201},{"g":32,"a":0.2004220485687256},{"g":98,"a":0.5},{"g":114,"a":0.3050632476806641},{"g":111,"a":0.461181640625},{"g":119,"a":0.6113924980163574},{"g":110,"a":0.5092825889587402},{"g":32,"a":0.2004218101501465},{"g":102,"a":0.289029598236084},{"g":111,"a":0.461181640625},{"g":120,"a":0.431645393371582},{"g":32,"a":0.2004218101501465},{"g":106,"a":0.264556884765625},{"g":117,"a":0.5109701156616211},{"g":109,"a":0.7894515991210938},{"g":112,"a":0.4932489395141602},{"g":115,"a":0.4434595108032227},{"g":32,"a":0.2004222869873047},{"g":111,"a":0.461181640625},{"g":118,"a":0.4240503311157227},{"g":101,"a":0.4333333969116211},{"g":114,"a":0.3050632476806641},{"g":32,"a":0.2004222869873047},{"g":116,"a":0.3172998428344727},{"g":104,"a":0.5092830657958984},{"g":101,"a":0.4333333969116211},{"g":32,"a":0.2004222869873047},{"g":108,"a":0.2362871170043945},{"g":97,"a":0.4645566940307617},{"g":122,"a":0.4004220962524414},{"g":121,"a":0.392827033996582},{"g":32,"a":0.2004222869873047},{"g":100,"a":0.5},{"g":111,"a":0.4611806869506836},{"g":103,"a":0.4763717651367188}],"B":[{"g":83,"a":0.5101265907287598},{"g":80,"a":0.4670885801315308},{"g":72,"a":0.564134955406189},{"g":73,"a":0.2362868785858154},{"g":78,"a":0.5772151947021484},{"g":88,"a":0.5413501262664795},{"g":32,"a":0.2004220485687256},{"g":79,"a":0.5447256565093994},{"g":70,"a":0.4421942234039307},{"g":32,"a":0.2004218101501465},{"g":66,"a":0.524472713470459},{"g":76,"a":0.3945145606994629},{"g":65,"a":0.4848103523254395},{"g":67,"a":0.5172996520996094},{"g":75,"a":0.5126581192016602},{"g":32,"a":0.2004218101501465},{"g":81,"a":0.5497889518737793},{"g":85,"a":0.5510544776916504},{"g":65,"a":0.4848098754882812},{"g":82,"a":0.5168771743774414},{"g":84,"a":0.42236328125},{"g":90,"a":0.4991559982299805},{"g":32,"a":0.2004222869873047},{"g":74,"a":0.3405065536499023},{"g":85,"a":0.5510549545288086},{"g":68,"a":0.5569620132446289},{"g":71,"a":0.5434598922729492},{"g":69,"a":0.4742612838745117},{"g":32,"a":0.2004222869873047},{"g":77,"a":0.7147674560546875},{"g":89,"a":0.4628696441650391},{"g":32,"a":0.2004222869873047},{"g":86,"a":0.4746837615966797},{"g":79,"a":0.5447254180908203},{"g":87,"a":0.7544307708740234}]})",
    };

};

static TypefaceTests typefaceTests;

#endif

} // namespace juce
