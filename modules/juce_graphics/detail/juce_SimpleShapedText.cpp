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

namespace juce::detail
{

//==============================================================================
constexpr hb_script_t getScriptTag (TextScript type)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
    switch (type)
    {
    case TextScript::common:     return HB_SCRIPT_COMMON;
    case TextScript::arabic:     return HB_SCRIPT_ARABIC;
    case TextScript::armenian:   return HB_SCRIPT_ARMENIAN;
    case TextScript::bengali:    return HB_SCRIPT_BENGALI;
    case TextScript::bopomofo:   return HB_SCRIPT_BOPOMOFO;
    case TextScript::cyrillic:   return HB_SCRIPT_CYRILLIC;
    case TextScript::devanagari: return HB_SCRIPT_DEVANAGARI;
    case TextScript::ethiopic:   return HB_SCRIPT_ETHIOPIC;
    case TextScript::georgian:   return HB_SCRIPT_GEORGIAN;
    case TextScript::greek:      return HB_SCRIPT_GREEK;
    case TextScript::gujarati:   return HB_SCRIPT_GUJARATI;
    case TextScript::gurmukhi:   return HB_SCRIPT_GURMUKHI;
    case TextScript::hangul:     return HB_SCRIPT_HANGUL;
    case TextScript::han:        return HB_SCRIPT_HAN;
    case TextScript::hebrew:     return HB_SCRIPT_HEBREW;
    case TextScript::hiragana:   return HB_SCRIPT_HIRAGANA;
    case TextScript::katakana:   return HB_SCRIPT_KATAKANA;
    case TextScript::kannada:    return HB_SCRIPT_KANNADA;
    case TextScript::khmer:      return HB_SCRIPT_KHMER;
    case TextScript::lao:        return HB_SCRIPT_LAO;
    case TextScript::latin:      return HB_SCRIPT_LATIN;
    case TextScript::malayalam:  return HB_SCRIPT_MALAYALAM;
    case TextScript::oriya:      return HB_SCRIPT_ORIYA;
    case TextScript::sinhala:    return HB_SCRIPT_SINHALA;
    case TextScript::tamil:      return HB_SCRIPT_TAMIL;
    case TextScript::telugu:     return HB_SCRIPT_TELUGU;
    case TextScript::thaana:     return HB_SCRIPT_THAANA;
    case TextScript::thai:       return HB_SCRIPT_THAI;
    case TextScript::tibetan:    return HB_SCRIPT_TIBETAN;
    case TextScript::adlam:      return HB_SCRIPT_ADLAM;
    case TextScript::balinese:   return HB_SCRIPT_BALINESE;
    case TextScript::bamum:      return HB_SCRIPT_BAMUM;
    case TextScript::batak:      return HB_SCRIPT_BATAK;
    case TextScript::chakma:     return HB_SCRIPT_CHAKMA;
    case TextScript::cham:       return HB_SCRIPT_CHAM;
    case TextScript::cherokee:   return HB_SCRIPT_CHEROKEE;
    case TextScript::javanese:   return HB_SCRIPT_JAVANESE;
    case TextScript::kayahLi:    return HB_SCRIPT_KAYAH_LI;
    case TextScript::taiTham:    return HB_SCRIPT_TAI_THAM;
    case TextScript::lepcha:     return HB_SCRIPT_LEPCHA;
    case TextScript::limbu:      return HB_SCRIPT_LIMBU;
    case TextScript::lisu:       return HB_SCRIPT_LISU;
    case TextScript::mandaic:    return HB_SCRIPT_MANDAIC;
    case TextScript::meeteiMayek:return HB_SCRIPT_MEETEI_MAYEK;
    case TextScript::newa:       return HB_SCRIPT_NEWA;
    case TextScript::nko:        return HB_SCRIPT_NKO;
    case TextScript::olChiki:    return HB_SCRIPT_OL_CHIKI;
    case TextScript::osage:      return HB_SCRIPT_OSAGE;
    case TextScript::miao:       return HB_SCRIPT_MIAO;
    case TextScript::saurashtra: return HB_SCRIPT_SAURASHTRA;
    case TextScript::sundanese:  return HB_SCRIPT_SUNDANESE;
    case TextScript::sylotiNagri:return HB_SCRIPT_SYLOTI_NAGRI;
    case TextScript::syriac:     return HB_SCRIPT_SYRIAC;
    case TextScript::taiLe:      return HB_SCRIPT_TAI_LE;
    case TextScript::newTaiLue:  return HB_SCRIPT_NEW_TAI_LUE;
    case TextScript::tifinagh:   return HB_SCRIPT_TIFINAGH;
    case TextScript::vai:        return HB_SCRIPT_VAI;
    case TextScript::wancho:     return HB_SCRIPT_WANCHO;
    case TextScript::yi:         return HB_SCRIPT_YI;

    case TextScript::hanifiRohingya:               return HB_SCRIPT_HANIFI_ROHINGYA;
    case TextScript::canadianAboriginalSyllabics:  return HB_SCRIPT_CANADIAN_SYLLABICS;
    case TextScript::nyiakengPuachueHmong:         return HB_SCRIPT_NYIAKENG_PUACHUE_HMONG;

    default: break;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return HB_SCRIPT_COMMON;
}

SimpleShapedText::SimpleShapedText (const String* data,
                                    const ShapedTextOptions& options)
    : string (*data)
{
    shape (string, options);
}

enum class ControlCharacter
{
    crFollowedByLf,
    cr,
    lf,
    tab
};

template <typename CharPtr>
static std::optional<ControlCharacter> findControlCharacter (CharPtr it, CharPtr end)
{
    constexpr juce_wchar lf = 0x0a;
    constexpr juce_wchar cr = 0x0d;
    constexpr juce_wchar tab = 0x09;

    switch (*it)
    {
        case lf:
            return ControlCharacter::lf;

        case tab:
            return ControlCharacter::tab;

        case cr:
        {
            const auto next = it + 1;
            return next != end && *next == lf ? ControlCharacter::crFollowedByLf
                                              : ControlCharacter::cr;
        }
    }

    return {};
}

static auto findControlCharacters (Span<const juce_wchar> string)
{
    std::map<size_t, ControlCharacter> result;
    size_t index = 0;

    for (auto it = string.begin(); it != string.end(); ++it, ++index)
        if (const auto cc = findControlCharacter (it, string.end()))
            result[index] = *cc;

    return result;
}

static constexpr hb_feature_t hbFeature (FontFeatureSetting setting)
{
    return { setting.tag.getTag(),
             setting.value,
             HB_FEATURE_GLOBAL_START,
             HB_FEATURE_GLOBAL_END };
}

enum class LigatureEnabledState
{
    normal,
    disabled
};

static std::vector<hb_feature_t> getHarfbuzzFeatures (Span<const FontFeatureSetting> settings,
                                                      LigatureEnabledState ligatureEnabledstate)
{
    // Font feature settings *should* always be sorted.
    jassert (std::is_sorted (settings.begin(), settings.end()));

    std::vector<hb_feature_t> features;

    features.reserve (settings.size());
    std::transform (settings.begin(), settings.end(), std::back_inserter (features), hbFeature);

    if (ligatureEnabledstate == LigatureEnabledState::disabled)
    {
        static constexpr FontFeatureTag tagsAffectedByTracking[] { FontFeatureTag { "liga" },
                                                                   FontFeatureTag { "clig" },
                                                                   FontFeatureTag { "hlig" },
                                                                   FontFeatureTag { "dlig" },
                                                                   FontFeatureTag { "calt" } };

        static constexpr auto less = [] (hb_feature_t a, hb_feature_t b)
        {
            return a.tag < b.tag;
        };

        for (auto tag : tagsAffectedByTracking)
            OrderedContainerHelpers::insertOrAssign (features, hbFeature ({ tag, 0 }), less);
    }

    return features;
}

/*  Returns glyphs in logical order as that favours wrapping. */
static std::vector<ShapedGlyph> lowLevelShape (Span<const juce_wchar> string,
                                               Range<int64> range,
                                               const Font& font,
                                               TextScript script,
                                               const String& language,
                                               uint8_t embeddingLevel)
{
    HbBuffer buffer { hb_buffer_create() };
    hb_buffer_clear_contents (buffer.get());

    hb_buffer_set_cluster_level (buffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_set_script (buffer.get(), getScriptTag (script));
    hb_buffer_set_language (buffer.get(), hb_language_from_string (language.toRawUTF8(), -1));

    hb_buffer_set_direction (buffer.get(),
                             (embeddingLevel % 2) == 0 ? HB_DIRECTION_LTR : HB_DIRECTION_RTL);

    hb_buffer_add_utf32 (buffer.get(),
                         unalignedPointerCast<const uint32_t*> (string.data()),
                         (int) range.getStart(),
                         0,
                         0);

    const Span shapedSpan { string.data() + range.getStart(), (size_t) range.getLength() };
    const auto controlChars = findControlCharacters (shapedSpan);

    for (const auto pair : enumerate (shapedSpan, size_t{}))
        hb_buffer_add (buffer.get(), static_cast<hb_codepoint_t> (pair.value), (unsigned int) pair.index);

    hb_buffer_add_utf32 (buffer.get(),
                         unalignedPointerCast<const uint32_t*> (shapedSpan.data() + shapedSpan.size()),
                         (int) string.size() - (int) range.getEnd(),
                         0,
                         0);

    hb_buffer_guess_segment_properties (buffer.get());

    auto nativeFont = font.getNativeDetails().font;

    if (nativeFont == nullptr)
        return {};

    const auto tracking = font.getExtraKerningFactor();
    const auto trackingIsDefault = approximatelyEqual (tracking, 0.0f, absoluteTolerance (0.001f));

    const auto features = getHarfbuzzFeatures (font.getFeatureSettings(),
                                               trackingIsDefault ? LigatureEnabledState::normal
                                                                 : LigatureEnabledState::disabled);

    hb_shape (nativeFont.get(), buffer.get(), features.data(), (unsigned int) features.size());

    const auto [infos, positions] = [&buffer]
    {
        unsigned int count{};

        return std::make_pair (Span { hb_buffer_get_glyph_infos     (buffer.get(), &count), (size_t) count },
                               Span { hb_buffer_get_glyph_positions (buffer.get(), &count), (size_t) count });
    }();

    jassert (infos.size() == positions.size());

    const auto missingGlyph = hb_buffer_get_not_found_glyph (buffer.get());

    [[maybe_unused]] const auto unknownGlyph = std::find_if (infos.begin(), infos.end(), [missingGlyph] (hb_glyph_info_t inf)
    {
        return inf.codepoint == missingGlyph;
    });

    // It this is hit, the typeface can't display one or more characters.
    // This normally shouldn't happen if font fallback is enabled, unless the String contains
    // control characters that JUCE doesn't know how to handle appropriately.
    jassert (unknownGlyph == infos.end());

    [[maybe_unused]] const auto trackingAmount = ! trackingIsDefault
                                               ? font.getHeight() * font.getHorizontalScale() * tracking
                                               : 0;

    std::vector<size_t> clusterLookup;
    std::vector<size_t> characterLookup;
    std::vector<ShapedGlyph> glyphs;

    std::optional<int64> lastCluster;

    const auto ltr = (embeddingLevel % 2) == 0;

    const auto getNextCluster = [&ltr, &infosCapt = infos, &range] (size_t visualIndex)
    {
        const auto next = (int64) visualIndex + (ltr ? 1 : -1);

        if (next < 0)
            return ltr ? range.getStart() : range.getEnd();

        if (next >= (int64) infosCapt.size())
            return ltr ? range.getEnd() : range.getStart();

        return (int64) infosCapt[(size_t) next].cluster + range.getStart();
    };

    for (size_t visualIndex = 0; visualIndex < infos.size(); ++visualIndex)
    {
        const auto glyphId = infos[visualIndex].codepoint;
        const auto xAdvanceBase = HbScale::hbToJuce (positions[visualIndex].x_advance);
        const auto yAdvanceBase = -HbScale::hbToJuce (positions[visualIndex].y_advance);

        // For certain OS, Font and glyph ID combinations harfbuzz will not find extents data and
        // hb_font_get_glyph_extents will return false. In such cases Typeface::getGlyphBounds
        // will return an empty rectangle. Here we need to distinguish this situation from the one
        // where extents information is available and is an empty rectangle, which indicates a
        // whitespace.
        const auto extentsDataAvailable = std::invoke ([&]
        {
            hb_glyph_extents_t extents{};
            return hb_font_get_glyph_extents (font.getTypefacePtr()->getNativeDetails().getFont(),
                                              (hb_codepoint_t) glyphId,
                                              &extents);
        });

        const auto whitespace = extentsDataAvailable
                                && font.getTypefacePtr()->getGlyphBounds (font.getMetricsKind(), (int) glyphId).isEmpty()
                                && xAdvanceBase > 0;

        const auto newline = std::invoke ([&controlChars, &shapingInfos = infos, visualIndex]
        {
           const auto it = controlChars.find ((size_t) shapingInfos[visualIndex].cluster);

           if (it == controlChars.end())
               return false;

           return it->second == ControlCharacter::cr || it->second == ControlCharacter::lf;
        });

        const auto cluster = (int64) infos[visualIndex].cluster + range.getStart();

        const auto numLigaturePlaceholders = std::max ((int64) 0,
                                                       std::abs (getNextCluster (visualIndex) - cluster) - 1);

        // Tracking is only applied at the beginning of a new cluster to avoid inserting it before
        // diacritic marks.
        const auto appliedTracking = std::exchange (lastCluster, cluster) != cluster
                                   ? trackingAmount
                                   : 0;

        const auto advanceMultiplier = numLigaturePlaceholders == 0 ? 1.0f
                                                                    : 1.0f / (float) (numLigaturePlaceholders + 1);

        Point<float> advance { xAdvanceBase * advanceMultiplier + appliedTracking, yAdvanceBase * advanceMultiplier };

        const auto ligatureClusterNumber = cluster + (ltr ? 0 : numLigaturePlaceholders);

        glyphs.push_back ({
            glyphId,
            ligatureClusterNumber,
            (infos[visualIndex].mask & HB_GLYPH_FLAG_UNSAFE_TO_BREAK) != 0,
            whitespace,
            newline,
            numLigaturePlaceholders == 0 ? (int8_t) 0 : (int8_t) -numLigaturePlaceholders ,
            advance,
            Point<float> { HbScale::hbToJuce (positions[visualIndex].x_offset),
                           -HbScale::hbToJuce (positions[visualIndex].y_offset) },
        });

        for (int l = 0; l < numLigaturePlaceholders; ++l)
        {
            const auto clusterDiff = l + 1;

            glyphs.push_back ({
                glyphId,
                ligatureClusterNumber + (ltr ? clusterDiff : -clusterDiff),
                true,
                whitespace,
                newline,
                (int8_t) (l + 1),
                advance,
                Point<float>{},
            });
        }
    }

    if (! ltr)
        std::reverse (glyphs.begin(), glyphs.end());

    return glyphs;
}

template <typename T>
struct SubSpanLookup
{
    explicit SubSpanLookup (Span<T> enclosingSpan)
        : enclosing (enclosingSpan)
    {}

    auto getRange (Span<T> span) const
    {
        jassert (enclosing.begin() <= span.begin() && enclosing.size() >= span.size());

        return Range<int64>::withStartAndLength ((int64) std::distance (enclosing.begin(), span.begin()),
                                                 (int64) span.size());
    }

    auto getSpan (Range<int64> r) const
    {
        jassert (r.getStart() + r.getLength() <= (int64) enclosing.size());

        return Span<T> { enclosing.begin() + r.getStart(), (size_t) r.getLength() };
    }

private:
    Span<T> enclosing;
};

template <typename T>
static auto makeSubSpanLookup (Span<T> s) { return SubSpanLookup<T> { s }; }

struct CanBreakBeforeIterator
{
    explicit CanBreakBeforeIterator (Span<const Unicode::Codepoint> s)
        : span (s),
          cursor (span.begin())
    {}

    const Unicode::Codepoint* next()
    {
        while (++cursor < span.begin() + span.size())
        {
            // Disallow soft break before a hard break
            const auto nextCodepointIsLinebreak = [&]
            {
                const auto nextCursor = cursor + 1;

                if (! (nextCursor < span.begin() + span.size()))
                    return false;

                return nextCursor->codepoint == 0x0a || nextCursor->codepoint == 0x0d;
            }();

            if (cursor->breaking == TextBreakType::soft && ! nextCodepointIsLinebreak)
                return cursor + 1;  // Use the same "can break before" semantics as Harfbuzz
        }

        return nullptr;
    }

    Span<const Unicode::Codepoint> span;
    const Unicode::Codepoint* cursor;
};

/*  Returns integers relative to the initialising Span's begin(), before which a linebreak is
    possible.

    Can be restricted to a sub-range using reset().
*/
struct IntegralCanBreakBeforeIterator
{
    explicit IntegralCanBreakBeforeIterator (Span<const Unicode::Codepoint> s)
        : span (s),
          it (span)
    {}

    void reset()
    {
        reset ({ std::numeric_limits<int64>::min(),  std::numeric_limits<int64>::max() });
    }

    void reset (Range<int64> r)
    {
        jassert ((size_t) r.getLength() <= span.size());

        restrictedTo = r;
        it = CanBreakBeforeIterator { span };
        rangeEndReturned = false;
    }

    std::optional<int64> next()
    {
        const auto intValue = [&] (auto p) { return (int64) std::distance (span.begin(), p); };

        for (auto* ptr = it.next(); ptr != nullptr; ptr = it.next())
        {
            const auto v = intValue (ptr);

            if (v > restrictedTo.getEnd())
                break;

            if (restrictedTo.getStart() < v && v <= restrictedTo.getEnd())
                return v;
        }

        if (std::exchange (rangeEndReturned, true) == false)
            return std::min ((int64) span.size(), restrictedTo.getEnd());

        return std::nullopt;
    }

private:
    Span<const Unicode::Codepoint> span;
    CanBreakBeforeIterator it;
    Range<int64> restrictedTo { std::numeric_limits<int64>::min(),  std::numeric_limits<int64>::max() };

    bool rangeEndReturned = false;
};

struct ShapingParams
{
    TextScript script;
    String language;
    uint8_t embeddingLevel;
    Font resolvedFont;
};

// Used to avoid signedness warning for types for which std::size() is int
template <typename T>
static auto makeSpan (T& array)
{
    return Span { array.getRawDataPointer(), (size_t) array.size() };
}

static detail::RangedValues<Font> findSuitableFontsForText (const Font& font,
                                                            Span<const juce_wchar> string,
                                                            const String& language = {})
{
    detail::RangedValues<std::optional<Font>> fonts;
    detail::Ranges::Operations ops;
    fonts.set ({ 0, (int64) string.size() }, font, ops);
    ops.clear();

    const auto getResult = [&]
    {
        detail::RangedValues<Font> result;

        for (const auto [r, v] : fonts)
        {
            result.set (r, v.value_or (font), ops);
            ops.clear();
        }

        return result;
    };

    if (! font.getFallbackEnabled())
        return getResult();

    const auto markMissingGlyphs = [&]
    {
        std::vector<int64> fontNotFound;

        for (const auto [r, f] : fonts)
        {
            for (auto i = r.getStart(); i < r.getEnd(); ++i)
            {
                if (f.has_value() && ! isFontSuitableForCodepoint (*f, string[(size_t) i]))
                    fontNotFound.push_back (i);
            }
        }

        for (const auto i : fontNotFound)
        {
            fonts.set ({ i, i + 1 }, std::nullopt, ops);
            ops.clear();
        }

        return fontNotFound.size();
    };

    // We keep calling findSuitableFontForText for sub-ranges without a suitable font until we
    // can't find any more suitable fonts or all codepoints have one
    for (auto numMissingGlyphs = markMissingGlyphs(); numMissingGlyphs > 0;)
    {
        std::vector<std::pair<Range<int64>, Font>> changes;

        for (const auto [r, f] : fonts)
        {
            if (f.has_value())
                continue;

            const CharPointer_UTF32 bPtr { string.data() + (int) r.getStart() };
            const CharPointer_UTF32 ePtr { string.data() + (int) r.getEnd() };
            changes.emplace_back (r, font.findSuitableFontForText (String (bPtr, ePtr), language));
        }

        for (const auto& c : changes)
        {
            fonts.set (c.first, c.second, ops);
            ops.clear();
        }

        if (const auto newNumMissingGlyphs = markMissingGlyphs();
            std::exchange (numMissingGlyphs, newNumMissingGlyphs) == newNumMissingGlyphs)
        {
            // We failed to resolve any more fonts during the last pass
            break;
        }
    }

    return getResult();
}

static RangedValues<Font> resolveFontsWithFallback (Span<const juce_wchar> string,
                                                    const RangedValues<Font>& fonts)
{
    RangedValues<Font> resolved;
    detail::Ranges::Operations ops;

    for (const auto [r, f] : fonts)
    {
        const auto intersected = r.getIntersectionWith ({ 0, (int64) string.size() });
        auto rf = findSuitableFontsForText (f,
                                            { string.data() + intersected.getStart(),
                                              (size_t) intersected.getLength() });

        for (const auto [subRange, font] : rf)
        {
            resolved.set (subRange + r.getStart(), font, ops, MergeEqualItemsNo{});
            ops.clear();
        }
    }

    return resolved;
}

struct GlyphsStorage
{
    std::shared_ptr<std::vector<ShapedGlyph>> data;
    bool ltr{};
    Font font;
};

struct OwnedGlyphsSpan
{
public:
    OwnedGlyphsSpan (GlyphsStorage subOwnedGlyphsSpanIn,
                     Span<const ShapedGlyph> glyphsIn,
                     Range<int64> textRangeIn,
                     size_t visualOrderIn)
        : subOwnedGlyphsSpan { std::move (subOwnedGlyphsSpanIn) },
          glyphs { glyphsIn },
          textRange { textRangeIn },
          visualOrder { visualOrderIn }
    {}

    auto& operator* ()        { return glyphs; }
    auto& operator* () const  { return glyphs; }

    auto operator-> ()        { return &glyphs; }
    auto operator-> () const  { return &glyphs; }

    bool operator== (const OwnedGlyphsSpan& other) const
    {
        return glyphs.data() == other.glyphs.data()
               && glyphs.size() == other.glyphs.size();
    }

    bool operator!= (const OwnedGlyphsSpan& other) const
    {
        return ! (*this == other);
    }

    auto getVisualOrder() const { return visualOrder; }
    auto isLtr()          const { return subOwnedGlyphsSpan.ltr; }
    auto getTextRange()   const { return textRange; }
    const auto& getFont() const { return subOwnedGlyphsSpan.font; }

    void setTextRange (Range<int64> newRange)
    {
        textRange = newRange;
    }

private:
    GlyphsStorage subOwnedGlyphsSpan;
    Span<const ShapedGlyph> glyphs;
    Range<int64> textRange;
    size_t visualOrder;
};

/* Objects of this type contain a ShapedGlyph range that terminates with a glyph after which
   soft-wrapping is possible. There are no soft-break opportunities anywhere else inside the range.
*/
using WrappedGlyphs = std::vector<OwnedGlyphsSpan>;

/* Contains a WrappedGlyphs object and marks a location (a particular glyph) somewhere inside it.

   Allows keeping track of partially consuming such objects to support mid-word breaking where the
   line is shorter than a single word.
*/
struct WrappedGlyphsCursor
{
    WrappedGlyphsCursor (const OwnedGlyphsSpan* dataIn, size_t num)
        : data { dataIn, num }
    {}

    bool empty() const
    {
        return data.empty() || data.back()->empty();
    }

    bool isBeyondEnd() const
    {
        return empty() || data.size() <= index.i;
    }

    auto& operator+= (size_t d)
    {
        while (d > 0 && ! isBeyondEnd())
        {
            const auto delta = std::min (d, data[index.i]->size() - index.j);
            index.j += delta;
            d -= delta;

            if (index.j == data[index.i]->size())
            {
                ++index.i;
                index.j = 0;
            }
        }

        return *this;
    }

    auto& operator++()
    {
        return *this += 1;
    }

    auto& operator*()       { return (*data[index.i])[index.j]; }
    auto& operator*() const { return (*data[index.i])[index.j]; }

    auto* operator->()       { return &(*data[index.i])[index.j]; }
    auto* operator->() const { return &(*data[index.i])[index.j]; }

    size_t size() const
    {
        if (empty() || isBeyondEnd())
            return 0;

        size_t size{};

        for (auto copy = *this; ! copy.isBeyondEnd(); ++copy)
            ++size;

        return size;
    }

    auto getTextRange() const
    {
        Range<int64> textRange;

        for (const auto& chunk : data)
            textRange = textRange.getUnionWith (chunk.getTextRange());

        return textRange;
    }

    bool operator== (const WrappedGlyphsCursor& other) const
    {
        const auto tie = [] (auto& x) { return std::tuple (x.data.data(), x.data.size(), x.index); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const WrappedGlyphsCursor& other) const
    {
        return ! operator== (other);
    }

    auto& back()       { return data.back()->back(); }
    auto& back() const { return data.back()->back(); }

    struct ShapedGlyphSpan
    {
        const ShapedGlyph* start;
        const ShapedGlyph* end;
        size_t visualOrder;
        Range<int64> textRange;
        Font font;
    };

    std::vector<ShapedGlyphSpan> getShapedGlyphSpansUpTo (const WrappedGlyphsCursor& end) const
    {
        std::vector<ShapedGlyphSpan> spans;

        if (data.data() != end.data.data() || data.size() != end.data.size())
        {
            jassertfalse;
            return spans;
        }

        for (auto indexCopy = index; indexCopy < end.index;)
        {
            auto& chunk = data[indexCopy.i];

            const auto glyphsStart = chunk->begin() + indexCopy.j;
            const auto glyphsEnd = chunk->end() - (indexCopy.i < end.index.i ? 0 : chunk->size() - end.index.j);

            const auto directionalStart = chunk.isLtr() ? glyphsStart : glyphsEnd - 1;
            const auto directionalEnd = chunk.isLtr() ? glyphsEnd : glyphsStart - 1;

            const auto textStart = glyphsStart->cluster;
            const auto textEnd = glyphsEnd < chunk->end() ? glyphsEnd->cluster : chunk.getTextRange().getEnd();

            spans.push_back ({ directionalStart,
                               directionalEnd,
                               chunk.getVisualOrder(),
                               { textStart, textEnd },
                               chunk.getFont() });

            ++indexCopy.i;
            indexCopy.j = 0;
        }

        return spans;
    }

private:
    struct Index
    {
        size_t i{}, j{};

        auto asTuple() const { return std::make_tuple (i, j); }
        bool operator== (const Index& other) const { return asTuple() == other.asTuple(); }
        bool operator<  (const Index& other) const { return asTuple() <  other.asTuple(); }
    };

    Span<const OwnedGlyphsSpan> data;
    Index index;
};

template <typename T>
static auto rangedValuesWithOffset (detail::RangedValues<T> rv, int64 offset = 0)
{
    detail::Ranges::Operations ops;
    rv.shift (std::numeric_limits<int64>::min(), -offset, ops);
    ops.clear();
    rv.eraseUpTo (0, ops);
    return rv;
}

/*  Increment b by the requested number of steps, or to e, whichever is reached first. */
template <typename CharPtr>
static auto incrementCharPtr (CharPtr b, CharPtr e, int64 steps)
{
    while (b != e && steps > 0)
    {
        ++b;
        --steps;
    }

    return b;
}

static std::vector<juce_wchar> sanitiseString (const String& stringIn, Range<int64> lineRange)
{
    std::vector<juce_wchar> result;

    const auto end = stringIn.end();
    const auto beginOfRange = incrementCharPtr (stringIn.begin(), end, lineRange.getStart());
    const auto endOfRange = incrementCharPtr (beginOfRange, end, lineRange.getLength());

    result.reserve (beginOfRange.lengthUpTo (endOfRange));

    for (auto it = beginOfRange; it != endOfRange; ++it)
    {
        result.push_back (std::invoke ([&]
        {
            const auto cc = findControlCharacter (it, end);

            if (! cc.has_value())
                return *it;

            constexpr juce_wchar wordJoiner       = 0x2060;
            constexpr juce_wchar nonBreakingSpace = 0x00a0;

            return cc == ControlCharacter::crFollowedByLf ? wordJoiner : nonBreakingSpace;
        }));
    }

    return result;
}

struct Shaper
{
    Shaper (const String& stringIn, Range<int64> lineRange, const ShapedTextOptions& options)
        : string (sanitiseString (stringIn, lineRange))
    {
        const auto analysis = Unicode::performAnalysis (stringIn.substring ((int) lineRange.getStart(),
                                                                            (int) lineRange.getEnd()));

        const BidiAlgorithm bidiAlgorithm { string };
        const auto bidiParagraph = bidiAlgorithm.createParagraph (options.getReadingDirection());
        const auto bidiLine = bidiParagraph.createLine (bidiParagraph.getLength());
        bidiLine.computeVisualOrder (visualOrder);

        const auto bidiLevels = bidiParagraph.getResolvedLevels();

        const auto fonts = resolveFontsWithFallback (string,
                                                     rangedValuesWithOffset (options.getFontsForRange(),
                                                                             lineRange.getStart()));

        detail::Ranges::Operations ops;

        for (Unicode::ScriptRunIterator scriptIter { makeSpan (analysis) }; auto scriptRun = scriptIter.next();)
        {
            const auto offsetInText = (size_t) std::distance (analysis.getRawDataPointer(), scriptRun->data());
            const auto length = scriptRun->size();

            const auto begin = bidiLevels.data() + offsetInText;
            const auto numRemainingElems = bidiLevels.size() > offsetInText ? bidiLevels.size() - offsetInText
                                                                            : 0;

            // If this assertion is hit, the input string is probably invalid according to the
            // Unicode parsing rules. If you know your string is a valid one, please reach out to us
            jassert (numRemainingElems >= length);

            const auto end = begin + std::min (length, numRemainingElems);

            for (auto it = begin; it != end;)
            {
                const auto next = std::find_if (it, end, [&] (const auto& l) { return l != *it; });
                const auto bidiStart = (int64) std::distance (bidiLevels.data(), it);
                const auto bidiLength = (int64) std::distance (it, next);
                const auto bidiRange = Range<int64>::withStartAndLength (bidiStart, bidiLength);

                for (const auto [range, font] : fonts.getIntersectionsWith (bidiRange))
                {
                    shaperRuns.set (range,
                                    { scriptRun->front().script,
                                      options.getLanguage(),
                                      *it,
                                      font },
                                    ops,
                                    MergeEqualItemsNo{});
                    ops.clear();
                }

                it = next;
            }
        }

        IntegralCanBreakBeforeIterator softBreakIterator { makeSpan (analysis) };

        for (auto breakBefore = softBreakIterator.next();
             breakBefore.has_value();
             breakBefore = softBreakIterator.next())
        {
            auto v = *breakBefore;

            if (softBreakBeforePoints.empty() || softBreakBeforePoints.back() != v)
                softBreakBeforePoints.push_back (v);
        }
    }

    WrappedGlyphs getChunksUpToNextSafeBreak (int64 startFrom)
    {
        const auto nextSoftBreakBefore = std::invoke ([&]
                                                      {
                                                          const auto it = std::upper_bound (softBreakBeforePoints.begin(),
                                                                                            softBreakBeforePoints.end(),
                                                                                            startFrom);

                                                          if (it == softBreakBeforePoints.end())
                                                              return (int64) visualOrder.size();

                                                          return *it;
                                                      });

        detail::Ranges::Operations ops;

        if (! shapedGlyphs.getRanges().covers ({ startFrom, nextSoftBreakBefore }))
        {
            for (auto it = shaperRuns.find (startFrom);
                 it != shaperRuns.end() && it->range.getStart() < nextSoftBreakBefore;
                 ++it)
            {
                const Range<int64> shapingRange { std::max (startFrom, it->range.getStart()), it->range.getEnd() };
                jassert (! shapingRange.isEmpty());

                auto g = lowLevelShape (string,
                                        shapingRange,
                                        it->value.resolvedFont,
                                        it->value.script,
                                        it->value.language,
                                        it->value.embeddingLevel);

                shapedGlyphs.set (shapingRange,
                                  {
                                      std::make_shared<std::vector<ShapedGlyph>> (std::move (g)),
                                      it->value.embeddingLevel % 2 == 0,
                                      it->value.resolvedFont
                                  },
                                  ops,
                                  MergeEqualItemsNo{});
                ops.clear();
            }
        }

        auto glyphsIt = shapedGlyphs.find (startFrom);
        WrappedGlyphs result;

        while (true)
        {
            // The stored glyphs data can be empty if there are input codepoints for which we failed to
            // resolve a valid Typeface::Ptr.
            if (glyphsIt == shapedGlyphs.end() || glyphsIt->value.data->empty())
                break;

            const ShapedGlyph* start = glyphsIt->value.data->data();
            const ShapedGlyph* const endIt = glyphsIt->value.data->data() + glyphsIt->value.data->size();

            while (start < endIt && start->cluster < startFrom)
                ++start;

            const ShapedGlyph* end = start;

            while (end < endIt && end->cluster < nextSoftBreakBefore)
                ++end;

            const auto startingCluster = std::max (startFrom, start->cluster);

            if (! result.empty())
                result.back().setTextRange (result.back().getTextRange().withEnd (startingCluster));

            if ((int64) visualOrder.size() <= start->cluster)
            {
                // If this assertion is hit, the input string is probably invalid according to the
                // Unicode parsing rules. If you know your string is a valid one, please reach out to us.
                jassertfalse;
                return result;
            }

            result.push_back ({ glyphsIt->value,
                                Span<const ShapedGlyph> { start, (size_t) std::distance (start, end) },
                                { startingCluster, nextSoftBreakBefore },
                                visualOrder[(size_t) start->cluster] });

            if (end != endIt && end->cluster >= nextSoftBreakBefore)
                break;

            ++glyphsIt;
        }

        return result;
    }

    std::vector<juce_wchar> string;
    std::vector<size_t> visualOrder;
    RangedValues<ShapingParams> shaperRuns;
    std::vector<int64> softBreakBeforePoints;
    RangedValues<GlyphsStorage> shapedGlyphs;
};

struct LineState
{
    LineState() = default;

    LineState (float w, bool f)
        : maxWidth { w },
          trailingWhitespaceCanExtendBeyondMargin { f }
    {}

    bool isInTrailingPosition (const ShapedGlyph& glyph) const
    {
        return glyph.cluster >= largestVisualOrderInLine;
    }

    bool isEmpty() const
    {
        return largestVisualOrderInLine < 0;
    }

    int64 largestVisualOrderInLine = -1;
    float maxWidth{};
    float width{};
    bool trailingWhitespaceCanExtendBeyondMargin;
};

struct WrappedGlyphsCursorRange
{
    WrappedGlyphsCursor begin, end;
};

class LineOfWrappedGlyphCursorRanges
{
public:
    LineOfWrappedGlyphCursorRanges() = default;

    LineOfWrappedGlyphCursorRanges (float maxWidth, bool trailingWhitespaceCanExtendBeyondMargin)
        : state { maxWidth, trailingWhitespaceCanExtendBeyondMargin }
    {}

    /*  Consumes as many glyphs from the provided cursor as the line will still fit. Returns the end
        cursor i.e. the state of the cursor after the glyphs have been consumed.

        If the line is empty it will partially consume a WrappedGlyphsCursor, otherwise only all of it
        or none of it.

        Always consumes at least one glyph. If forceConsumeFirstWord is true, it consumes at least
        one word.
    */
    WrappedGlyphsCursor consume (const WrappedGlyphsCursor& glyphIt, bool forceConsumeFirstWord)
    {
        if (forceConsumeFirstWord && state.isEmpty())
        {
            auto [newState, newIt] = consumeIf (state, glyphIt, [] (auto&, auto&) { return true; });
            consumedChunks.push_back ({ glyphIt, newIt });
            state = std::move (newState);
            return newIt;
        }

        auto [newState, newIt] = consumeIf (state, glyphIt, [] (auto& nextState, auto& glyph)
                                            {
                                                const auto remainingWidth = nextState.maxWidth - nextState.width;

                                                return nextState.isEmpty()
                                                       || glyph.advance.getX() <= remainingWidth
                                                       || (nextState.trailingWhitespaceCanExtendBeyondMargin
                                                           && glyph.isWhitespace()
                                                           && nextState.isInTrailingPosition (glyph));
                                            });

        // A OwnedGlyphsSpan always ends in the first valid breakpoint. We can only consume all of it or
        // none of it. Unless the line is still empty, which means that it's too short to fit even
        // a single word.
        if (! state.isEmpty() && ! newIt.isBeyondEnd())
            return glyphIt;

        if (newIt != glyphIt)
            consumedChunks.push_back ({ glyphIt, newIt });

        state = std::move (newState);

        return newIt;
    }

    const auto& getConsumedChunks() const
    {
        return consumedChunks;
    }

private:
    static std::pair<LineState, WrappedGlyphsCursor> consumeIf (const LineState& state,
                                                                const WrappedGlyphsCursor& it,
                                                                std::function<bool (const LineState&, const ShapedGlyph&)> predicate)
    {
        auto newState = state;
        auto newIt = it;

        while (! newIt.isBeyondEnd() && predicate (newState, *newIt))
        {
            newState.width += newIt->advance.getX();
            newState.largestVisualOrderInLine = std::max (newState.largestVisualOrderInLine, newIt->cluster);
            ++newIt;
        }

        return { std::move (newState), std::move (newIt) };
    }

    LineState state;
    std::vector<WrappedGlyphsCursorRange> consumedChunks;
};

struct LineDataAndChunkStorage
{
    std::vector<WrappedGlyphs> chunkStorage;
    std::vector<std::vector<WrappedGlyphsCursorRange>> lines;
};

struct FillLinesOptions
{
    FillLinesOptions withWidth (float x) const
    {
        return withMember (*this, &FillLinesOptions::width, x);
    }

    FillLinesOptions withFirstLinePadding (float x) const
    {
        return withMember (*this, &FillLinesOptions::firstLinePadding, x);
    }

    FillLinesOptions withTrailingWhitespaceCanExtendBeyondMargin (bool x = true) const
    {
        return withMember (*this, &FillLinesOptions::trailingWhitespaceCanExtendBeyondMargin, x);
    }

    FillLinesOptions withForceConsumeFirstWord (bool x = true) const
    {
        return withMember (*this, &FillLinesOptions::forceConsumeFirstWord, x);
    }

    LineDataAndChunkStorage fillLines (Shaper& shaper) const
    {
        LineDataAndChunkStorage result;
        LineOfWrappedGlyphCursorRanges line { width - firstLinePadding, trailingWhitespaceCanExtendBeyondMargin };

        for (auto chunks = shaper.getChunksUpToNextSafeBreak (0);
             ! chunks.empty();)
        {
            result.chunkStorage.push_back (std::move (chunks));
            WrappedGlyphsCursor cursor { result.chunkStorage.back().data(),
                                         result.chunkStorage.back().size() };

            while (! cursor.isBeyondEnd())
            {
                cursor = line.consume (cursor, forceConsumeFirstWord);

                if (! cursor.isBeyondEnd())
                {
                    result.lines.push_back (line.getConsumedChunks());
                    line = LineOfWrappedGlyphCursorRanges { width, trailingWhitespaceCanExtendBeyondMargin };
                }
            }

            chunks = shaper.getChunksUpToNextSafeBreak (cursor.getTextRange().getEnd());
        }

        result.lines.push_back (line.getConsumedChunks());

        return result;
    }

    float width{};
    float firstLinePadding{};
    bool trailingWhitespaceCanExtendBeyondMargin = false;
    bool forceConsumeFirstWord = false;
};

static auto getShapedGlyphSpansInVisualOrder (const std::vector<WrappedGlyphsCursorRange>& lineData)
{
    std::vector<WrappedGlyphsCursor::ShapedGlyphSpan> glyphSpans;

    for (const auto& chunk : lineData)
    {
        auto spans = chunk.begin.getShapedGlyphSpansUpTo (chunk.end);
        glyphSpans.insert (glyphSpans.begin(), spans.begin(), spans.end());
    }

    std::sort (glyphSpans.begin(),
               glyphSpans.end(),
               [] (const auto& a, const auto& b)
               {
                   return a.visualOrder < b.visualOrder;
               });

    return glyphSpans;
}

static auto getLineRanges (const String& data)
{
    std::vector<Range<int64>> lineRanges;

    const auto analysis = Unicode::performAnalysis (data);
    const auto spanLookup = makeSubSpanLookup (makeSpan (analysis));

    for (Unicode::LineBreakIterator lineIter { makeSpan (analysis) }; auto lineRun = lineIter.next();)
        lineRanges.push_back (spanLookup.getRange (*lineRun));

    return lineRanges;
}

static void foldLinesBeyondLineLimit (std::vector<std::vector<WrappedGlyphsCursorRange>>& lines,
                                      size_t maxNumLines)
{
    if (lines.size() <= maxNumLines || maxNumLines == 0)
        return;

    auto& lastLine = lines[maxNumLines - 1];

    for (auto i = maxNumLines; i < lines.size(); ++i)
        lastLine.insert (lastLine.end(), lines[i].begin(), lines[i].end());

    lines.erase (iteratorWithAdvance (lines.begin(), maxNumLines), lines.end());
}

void SimpleShapedText::shape (const String& data,
                              const ShapedTextOptions& options)
{
    detail::Ranges::Operations ops;

    for (const auto& lineRange : getLineRanges (data))
    {
        Shaper shaper { data, lineRange, options };
        auto lineDataAndStorage = FillLinesOptions{}.withWidth (options.getWordWrapWidth().value_or ((float) 1e6))
                                                    .withFirstLinePadding (options.getFirstLineIndent())
                                                    .withTrailingWhitespaceCanExtendBeyondMargin (! options.getTrailingWhitespacesShouldFit())
                                                    .withForceConsumeFirstWord (! options.getAllowBreakingInsideWord())
                                                    .fillLines (shaper);
        auto& lineData = lineDataAndStorage.lines;

        foldLinesBeyondLineLimit (lineData, (size_t) options.getMaxNumLines() - lineNumbersForGlyphRanges.size());

        if (lineNumbersForGlyphRanges.size() >= (size_t) options.getMaxNumLines())
            break;

        for (const auto& line : lineData)
        {
            const auto glyphSpansInLine = getShapedGlyphSpansInVisualOrder (line);

            const auto lineStart = (int64) glyphsInVisualOrder.size();

            for (const auto& s : glyphSpansInLine)
            {
                const auto start = (int64) glyphsInVisualOrder.size();
                bool ltr = true;

                if (s.start < s.end)
                {
                    for (auto it = s.start; it < s.end; ++it)
                        glyphsInVisualOrder.push_back (*it);
                }
                else
                {
                    ltr = false;

                    for (auto it = s.start; it > s.end; --it)
                        glyphsInVisualOrder.push_back (*it);
                }

                const auto end = (int64) glyphsInVisualOrder.size();

                for (auto i = start; i < end; ++i)
                    glyphsInVisualOrder[(size_t) i].cluster += lineRange.getStart();

                glyphLookup.set (s.textRange + lineRange.getStart(), { { start, end }, ltr }, ops, MergeEqualItemsNo{});
                ops.clear();
                resolvedFonts.set ({ start, end }, s.font, ops);
                ops.clear();
            }

            const auto lineTextRange = std::accumulate (glyphSpansInLine.begin(),
                                                        glyphSpansInLine.end(),
                                                        std::make_pair (std::numeric_limits<int64>::max(),
                                                                        std::numeric_limits<int64>::min()),
                                                        [&] (auto sum, auto& elem) -> std::pair<int64, int64>
                                                        {
                                                            const auto r = elem.textRange + lineRange.getStart();

                                                            return { std::min (sum.first, r.getStart()),
                                                                     std::max (sum.second, r.getEnd()) };
                                                        });

            lineTextRanges.set ({ lineTextRange.first, lineTextRange.second }, ops);
            ops.clear();

            const auto lineEnd = (int64) glyphsInVisualOrder.size();
            lineNumbersForGlyphRanges.set ({ lineStart, lineEnd}, (int64) lineNumbersForGlyphRanges.size(), ops);
            ops.clear();
        }
    }
}

Span<const ShapedGlyph> SimpleShapedText::getGlyphs (Range<int64> glyphRange) const
{
    const auto r = glyphRange.getIntersectionWith ({ 0, (int64) glyphsInVisualOrder.size() });

    return { glyphsInVisualOrder.data() + r.getStart(), (size_t) r.getLength() };
}

Span<const ShapedGlyph> SimpleShapedText::getGlyphs() const
{
    return glyphsInVisualOrder;
}

juce_wchar SimpleShapedText::getCodepoint (int64 glyphIndex) const
{
    return string[(int) glyphsInVisualOrder[(size_t) glyphIndex].cluster];
}

Range<int64> SimpleShapedText::getTextRange (int64 glyphIndex) const
{
    jassert (isPositiveAndBelow (glyphIndex, getNumGlyphs()));

    // A single glyph can span multiple input codepoints. We can discover this by checking the
    // neighbouring glyphs cluster values. If neighbouring values differ by more than one, then the
    // missing clusters belong to a single glyph.
    //
    // However, we only have to check glyphs that are in the same bidi run as this one, hence the
    // lookup.
    const auto startingCodepoint = glyphsInVisualOrder[(size_t) glyphIndex].cluster;
    const auto glyphRange = glyphLookup.getItemWithEnclosingRange (startingCodepoint)->value.glyphRange;

    const auto glyphRun = Span<const ShapedGlyph> { glyphsInVisualOrder.data() + glyphRange.getStart(),
                                                    (size_t) glyphRange.getLength() };

    const auto indexInRun = glyphIndex - glyphRange.getStart();

    const auto cluster = glyphRun[(size_t) indexInRun].cluster;

    const auto nextAdjacentCluster = [&]
    {
        auto left = [&]
        {
            for (auto i = indexInRun; i >= 0; --i)
                if (auto c = glyphRun[(size_t) i].cluster; c != cluster)
                    return c;

            return cluster;
        }();

        auto right = [&]
        {
            for (auto i = indexInRun; i < (decltype (i)) glyphRun.size(); ++i)
                if (auto c = glyphRun[(size_t) i].cluster; c != cluster)
                    return c;

            return cluster;
        }();

        return std::max (left, right);
    }();

    return Range<int64>::withStartAndLength (cluster, std::max ((int64) 1, nextAdjacentCluster - cluster));
}

bool SimpleShapedText::isLtr (int64 glyphIndex) const
{
    const auto it = glyphLookup.find (glyphsInVisualOrder[(size_t) glyphIndex].cluster);
    jassert (it != glyphLookup.end());
    return it->value.ltr;
}

/*  Returns the first element that equals value, if such an element exists.

    Otherwise, returns the last element that is smaller than value, if such an element exists.

    Returns end otherwise.


    NB: lower_bound:     equal or greater
        upper_bound:     greater
        lessThanOrEqual: less than or equal
*/
template <typename It, typename Value, typename Callback>
auto equalOrLessThan (It begin, It end, Value v, Callback extractValue)
{
    if (begin == end)
        return end;

    auto it = std::lower_bound (begin,
                                end,
                                v,
                                [&extractValue] (auto& elem, auto& value)
                                {
                                    return extractValue (elem) < value;
                                });

    if (it == begin || (it != end && extractValue (*it) == v))
        return it;

    --it;

    return it;
}

void SimpleShapedText::getGlyphRanges (Range<int64> textRange, std::vector<Range<int64>>& outRanges) const
{
    outRanges.clear();
    Ranges glyphRanges { std::move (outRanges) };
    detail::Ranges::Operations ops;

    for (const auto is : glyphLookup.getIntersectionsWith (textRange))
    {
        const auto textSubRange = is.range;
        const auto glyphsSubRange = is.value.glyphRange;
        const auto& subRangeLookup = is.value;
        const auto glyphs = getGlyphs (glyphsSubRange);

        const auto getGlyphSubRange = [&] (auto begin, auto end)
        {
            auto startIt = equalOrLessThan (begin,
                                            end,
                                            textSubRange.getStart(),
                                            [] (auto& elem) -> auto& { return elem.cluster; });

            auto endIt = std::lower_bound (begin,
                                           end,
                                           textSubRange.getEnd(),
                                           [] (auto& elem, auto& value) { return elem.cluster < value; });

            return Range<int64> { (int64) std::distance (begin, startIt), std::distance (begin, endIt) };
        };

        if (subRangeLookup.ltr)
        {
            glyphRanges.set (getGlyphSubRange (glyphs.begin(), glyphs.end()) + glyphsSubRange.getStart(), ops);
        }
        else
        {
            const auto reverseRange = getGlyphSubRange (std::reverse_iterator { glyphs.end() },
                                                        std::reverse_iterator { glyphs.begin() });

            glyphRanges.set ({ glyphsSubRange.getEnd() - reverseRange.getEnd(),
                               glyphsSubRange.getEnd() - reverseRange.getStart() },
                             ops);
        }

        ops.clear();
    }

    outRanges = std::move (glyphRanges.getRanges());
}

int64 SimpleShapedText::getTextIndexAfterGlyph (int64 glyphIndex) const
{
    const auto& cluster = glyphsInVisualOrder[(size_t) glyphIndex].cluster;
    auto it = glyphLookup.find (glyphsInVisualOrder[(size_t) glyphIndex].cluster);

    if (it->value.ltr)
    {
        for (auto i = glyphIndex + 1; i < it->value.glyphRange.getEnd(); ++i)
            if (const auto nextCluster = glyphsInVisualOrder[(size_t) i].cluster; nextCluster != cluster)
                return nextCluster;
    }
    else
    {
        for (auto i = glyphIndex - 1; i >= it->value.glyphRange.getStart(); --i)
            if (const auto nextCluster = glyphsInVisualOrder[(size_t) i].cluster; nextCluster != cluster)
                return nextCluster;
    }

    return it->range.getEnd();
}

#if JUCE_UNIT_TESTS

struct SimpleShapedTextTests : public UnitTest
{
    SimpleShapedTextTests()
        : UnitTest ("SimpleShapedText", UnitTestCategories::text)
    {
    }

    static constexpr const char* testStrings[]
    {
        "Some trivial text",
        "Text with \r\n\r\n line feed and new line characters",
        "\nPrepending new line character",
        "\n\nMultiple prepending new line characters",
        "\n\nMultiple prepending and trailing line feed or new line characters\n\r\n",
        "Try right-clicking on a slider for an options menu. \n\nAlso, holding down CTRL while dragging will turn on a slider's velocity-sensitive mode",
    };

    void runTest (const char* text, float maxWidth)
    {
        const auto defaultTypeface = Font::getDefaultTypefaceForFont (FontOptions{});

        if (defaultTypeface == nullptr)
        {
            DBG ("Skipping test: No default typeface found!");
            return;
        }

        String testString { text };

        SimpleShapedText st { &testString, ShapedTextOptions{}.withFont (FontOptions { defaultTypeface })
                                                              .withWordWrapWidth (maxWidth) };

        auto success = true;

        for (int64 glyphIndex = 0; glyphIndex < st.getNumGlyphs(); ++glyphIndex)
        {
            const auto textRange = st.getTextRange (glyphIndex);

            // This assumption holds for LTR text if no ligatures are used
            success &= textRange.getStart() == glyphIndex && textRange.getLength() == 1;
        }

        expect (success, String { "Failed for test string: " } + testString.replace ("\r", "<CR>")
                                                                           .replace ("\n", "<LF>"));
    }

    void runTest() override
    {
        beginTest ("getTextRange: LTR Latin text without ligatures - no soft breaks");
        {
            for (auto* testString : testStrings)
                runTest (testString, 100'000.0f);
        }

        beginTest ("getTextRange: LTR Latin text without ligatures - with soft breaks");
        {
            for (auto* testString : testStrings)
                runTest (testString, 60.0f);
        }
    }
};

class HarfbuzzFontFeatureTests : public UnitTest
{
public:
    HarfbuzzFontFeatureTests()
        : UnitTest ("HarfbuzzFontFeatureTests", UnitTestCategories::text)
    {
    }

    void runTest() override
    {
        static constexpr FontFeatureSetting input[] =
        {
            FontFeatureSetting { "calt", 1 },
            FontFeatureSetting { "clig", 1 },
            FontFeatureSetting { "dlig", 1 },
            FontFeatureSetting { "hlig", 1 },
            FontFeatureSetting { "liga", 1 }
        };

        static constexpr auto compare = [] (hb_feature_t a, hb_feature_t b)
        {
            return a.value == b.value;
        };

        beginTest ("Disabling ligatures overrides existing ligature feature values in feature set");
        {
            static constexpr hb_feature_t expected[] =
            {
                hbFeature ({ "calt", 0 }),
                hbFeature ({ "clig", 0 }),
                hbFeature ({ "dlig", 0 }),
                hbFeature ({ "hlig", 0 }),
                hbFeature ({ "liga", 0 })
            };

            auto result = getHarfbuzzFeatures (input, LigatureEnabledState::disabled);

            expect (std::equal (result.begin(),
                                result.end(),
                                std::begin (expected),
                                std::end (expected),
                                compare));
        }

        beginTest ("Disabling ligatures appends ligature features in a disabled state to an empty feature set");
        {
            static constexpr hb_feature_t expected[] =
            {
                hbFeature ({ "calt", 0 }),
                hbFeature ({ "clig", 0 }),
                hbFeature ({ "dlig", 0 }),
                hbFeature ({ "hlig", 0 }),
                hbFeature ({ "liga", 0 })
            };

            auto result = getHarfbuzzFeatures ({}, LigatureEnabledState::disabled);

            expect (std::equal (result.begin(),
                                result.end(),
                                std::begin (expected),
                                std::end (expected),
                                compare));
        }

        beginTest ("Enabling ligatures has no effect on ligature features in feature set");
        {
            static constexpr FontFeatureSetting featureSet[] =
            {
                FontFeatureSetting { "calt", 1 },
                FontFeatureSetting { "clig", 0 },
                FontFeatureSetting { "dlig", 1 },
                FontFeatureSetting { "hlig", 0 },
                FontFeatureSetting { "liga", 1 }
            };

            static constexpr hb_feature_t expected[] =
            {
                hbFeature ({ "calt", 1 }),
                hbFeature ({ "clig", 0 }),
                hbFeature ({ "dlig", 1 }),
                hbFeature ({ "hlig", 0 }),
                hbFeature ({ "liga", 1 })
            };

            auto result = getHarfbuzzFeatures (featureSet, LigatureEnabledState::normal);

            expect (std::equal (result.begin(),
                                result.end(),
                                std::begin (expected),
                                std::end (expected),
                                compare));
        }

        beginTest ("Only ligature features are disabled in an existing feature set");
        {
            static constexpr FontFeatureSetting featureSet[] =
            {
                FontFeatureSetting { "calt", 1 },
                FontFeatureSetting { "fak1", 0 },
                FontFeatureSetting { "fak2", 1 }
            };

            static constexpr hb_feature_t expected[] =
            {
                hbFeature ({ "calt", 0 }),
                hbFeature ({ "clig", 0 }),
                hbFeature ({ "dlig", 0 }),
                hbFeature ({ "fak1", 0 }),
                hbFeature ({ "fak2", 1 }),
                hbFeature ({ "hlig", 0 }),
                hbFeature ({ "liga", 0 })
            };

            auto result = getHarfbuzzFeatures (featureSet, LigatureEnabledState::disabled);

            expect (std::equal (result.begin(),
                                result.end(),
                                std::begin (expected),
                                std::end (expected),
                                compare));
        }

        beginTest ("An empty feature set is not mutated when ligatures are enabled");
        {
            auto result = getHarfbuzzFeatures ({}, LigatureEnabledState::normal);
            expect (result.empty());
        }
    }
};

static SimpleShapedTextTests simpleShapedTextTests;
static HarfbuzzFontFeatureTests harfbuzzFontFeatureTests;

#endif

} // namespace juce::detail
