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

struct TypefaceLegacyMetrics
{
    float ascent{};  // in em units
    float descent{}; // in em units

    float getScaledAscent()  const { return ascent  * getHeightToPointsFactor(); }
    float getScaledDescent() const { return descent * getHeightToPointsFactor(); }

    float getPointsToHeightFactor() const { return ascent + descent; }
    float getHeightToPointsFactor() const { return 1.0f / getPointsToHeightFactor(); }
};

using HbFont   = std::unique_ptr<hb_font_t, FunctionPointerDestructor<hb_font_destroy>>;
using HbFace   = std::unique_ptr<hb_face_t, FunctionPointerDestructor<hb_face_destroy>>;
using HbBuffer = std::unique_ptr<hb_buffer_t, FunctionPointerDestructor<hb_buffer_destroy>>;

class Typeface::Native
{
public:
    explicit Native (hb_font_t* fontRef)
        : Native (fontRef, findLegacyMetrics (fontRef)) {}

    Native (hb_font_t* fontRef, TypefaceLegacyMetrics metrics)
        : font (fontRef), legacyMetrics (metrics) {}

    auto* getFont() const { return font; }

    auto getLegacyMetrics() const { return legacyMetrics; }

private:
    static TypefaceLegacyMetrics findLegacyMetrics (hb_font_t* f)
    {
        hb_font_extents_t extents{};

        if (! hb_font_get_h_extents (f, &extents))
        {
            // jassertfalse;
            return { 0.5f, 0.5f };
        }

        const auto ascent  = std::abs ((float) extents.ascender);
        const auto descent = std::abs ((float) extents.descender);
        const auto upem    = (float) hb_face_get_upem (hb_font_get_face (f));

        TypefaceLegacyMetrics result;
        result.ascent  = ascent  / upem;
        result.descent = descent / upem;
        return result;
    }

    hb_font_t* font = nullptr;
    TypefaceLegacyMetrics legacyMetrics;
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
            const Font f (placeholder, 15.0f, Font::plain);
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
        auto* blob = hb_blob_create_or_fail (bytes.data(),
                                             (unsigned int) bytes.size(),
                                             HB_MEMORY_MODE_DUPLICATE,
                                             nullptr,
                                             nullptr);
        const ScopeGuard scope { [&] { hb_blob_destroy (blob); } };

        const auto count = hb_face_count (blob);

        if (count < 1)
        {
            // Attempted to create a font from invalid data. Perhaps the font format was unrecognised.
            jassertfalse;
            return {};
        }

        return HbFace { hb_face_create (blob, index) };
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

static Path getTypefaceGlyph (const Typeface& typeface, int glyphNumber)
{
    static const auto funcs = getPathDrawFuncs();

    auto* font = typeface.getNativeDetails().getFont();

    Path result;
    hb_font_draw_glyph (font, (hb_codepoint_t) glyphNumber, funcs.get(), &result);

    // Convert to em units
    result.applyTransform (AffineTransform::scale (1.0f / (float) hb_face_get_upem (hb_font_get_face (font))).scaled (1.0f, -1.0f));

    return result;
}

void Typeface::getOutlineForGlyph (int glyphNumber, Path& path)
{
    const auto metrics = getNativeDetails().getLegacyMetrics();

    // getTypefaceGlyph returns glyphs in em space, getOutlineForGlyph returns glyphs in "special JUCE units" space
    path = getTypefaceGlyph (*this, glyphNumber);
    path.applyTransform (AffineTransform::scale (metrics.getHeightToPointsFactor()));
}

void Typeface::applyVerticalHintingTransform (float, Path&)
{
    jassertfalse;
}

EdgeTable* Typeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform, float)
{
    Path path;
    getOutlineForGlyph (glyphNumber, path);

    if (path.isEmpty())
        return nullptr;

    return new EdgeTable (path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                          path, transform);
}

float Typeface::getAscent()               const { return getNativeDetails().getLegacyMetrics().getScaledAscent(); }
float Typeface::getDescent()              const { return getNativeDetails().getLegacyMetrics().getScaledDescent(); }
float Typeface::getHeightToPointsFactor() const { return getNativeDetails().getLegacyMetrics().getHeightToPointsFactor(); }

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
static void doSimpleShape (const Typeface& typeface, const String& text, Consumer&& consumer)
{
    HbBuffer buffer { hb_buffer_create() };
    hb_buffer_add_utf8 (buffer.get(), text.toRawUTF8(), -1, 0, -1);
    hb_buffer_set_cluster_level (buffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
    hb_buffer_guess_segment_properties (buffer.get());

    const auto& native = typeface.getNativeDetails();
    auto* font = native.getFont();

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

    const auto heightToPoints = native.getLegacyMetrics().getHeightToPointsFactor();
    const auto upem = hb_face_get_upem (hb_font_get_face (font));

    Point<hb_position_t> cursor{};

    for (auto i = decltype (numGlyphs){}; i < numGlyphs; ++i)
    {
        const auto& info = infos[i];
        const auto& position = positions[i];
        consumer (std::make_optional (info.codepoint),
                  heightToPoints * ((float) cursor.x + (float) position.x_offset) / (float) upem);
        cursor += Point { position.x_advance, position.y_advance };
    }

    consumer (std::optional<hb_codepoint_t>{}, heightToPoints * (float) cursor.x / (float) upem);
}

float Typeface::getStringWidth (const String& text)
{
    float x{};
    doSimpleShape (*this, text, [&] (auto, auto xOffset)
    {
        x = xOffset;
    });
    return x;
}

void Typeface::getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets)
{
    doSimpleShape (*this, text, [&] (auto codepoint, auto xOffset)
    {
        if (codepoint.has_value())
            glyphs.add ((int) *codepoint);

        xOffsets.add (xOffset);
    });
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
            Font font (ptr->getName(), ptr->getStyle(), 12.0f);

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
