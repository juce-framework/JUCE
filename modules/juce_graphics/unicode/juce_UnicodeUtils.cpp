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

// This only make sense with integral/enum types.
// We use it for enums and std::pair<enum, enum>.
template <typename T, typename... TT>
static constexpr bool any (T b, TT... bs)
{
    return ((b == bs) || ...);
}

// Order is important!!!!
enum class LineBreakType : uint8_t
{
    al, bk,  cm, cr, gl, lf, nl, sp, wj,
    zw, zwj, ai, b2, ba, bb, cb, cj, cl, cp,
    eb, em,  ex, h2, h3, hl, hy, in, is, jl,
    id, jt,  jv, ns, nu, op, po, pr, qu, ri,
    sa, sg,  sy, xx, opw
};

// Order is important!!!!
enum class EastAsianWidthType : uint8_t
{
    N,
    narrow,
    ambiguous,
    full,
    half,
    wide
};

// Order is important!!!!
enum class BidiType : uint8_t
{
    // Strong: Left to right
    ltr,

    // Strong: Right to left
    rtl,

    // Strong: Arabic Right to left
    al,

    // Weak: European number
    en,

    // Weak: Arabic umber
    an,

    // Weak: European number seperator
    es,

    // Weak: European number terminator
    et,

    // Weak: Common number seperator
    cs,

    // Weak: onspacing mark
    nsm,

    // Weak: Boundary
    bn,

    // eutral: Paragraph seperator
    b,

    // eutral: Segment seperator
    s,

    // eutral: Whitespace
    ws,

    // eutral: Other s
    on,

    // Explicit Formatting: LTR Embedding
    lre,

    // Explicit Formatting: LTR Override
    lro,

    // Explicit Formatting: RTL Embedding
    rle,

    // Explicit Formatting: RTL Overide
    rlo,

    // Explicit Formatting: Pop Directional Format
    pdf,

    // Explicit Formatting: LTR Isolate
    lri,

    // Explicit Formatting: RTL Isolate
    rli,

    // Explicit Formatting: First Strong Isolate
    fsi,

    // Explicit Formatting: Pop Directional Isolate
    pdi,

    none
};


enum class VerticalTransformType : uint8_t
{
    R, U, Tr, Tu
};

// https://www.unicode.org/reports/tr51/tr51-21.html
enum class EmojiType : uint8_t
{
    yes,
    presentation,
    modifier,
    modifierBase,
    component,
    extended,
    no
};

// This is an internal type
enum class UnicodeTextScript : uint8_t
{
    Common, Inherited, Han, Arabic, Hiragana, Adlam, Mende_Kikakui, Ethiopic, Wancho,
    Toto, Nyiakeng_Puachue_Hmong, Glagolitic, Latin, SignWriting, Greek, Duployan,
    Nushu, Katakana, Tangut, Khitan_Small_Script, Miao, Medefaidrin, Pahawh_Hmong,
    Bassa_Vah, Tangsa, Mro, Bamum, Cypro_Minoan, Cuneiform, Tamil, Lisu, Makasar,
    Gunjala_Gondi, Masaram_Gondi, Marchen, Bhaiksuki, Pau_Cin_Hau, Canadian_Aboriginal,
    Soyombo, Zanabazar_Square, Nandinagari, Dives_Akuru, Warang_Citi, Dogra, Ahom,
    Takri, Mongolian, Modi, Siddham, Tirhuta, Newa, Grantha, Khudawadi, Multani,
    Khojki, Sinhala, Sharada, Mahajani, Chakma, Sora_Sompeng, Kaithi, Brahmi, Elymaic,
    Chorasmian, Sogdian, Yezidi, Hanifi_Rohingya, Psalter_Pahlavi, Avestan, Manichaean,
    Kharoshthi, Meroitic_Cursive, Lydian, Phoenician, Hatran, Nabataean, Palmyrene,
    Imperial_Aramaic, Cypriot, Vithkuqi, Caucasian_Albanian, Elbasan, Osage,
    Osmanya, Shavian, Deseret, Ugaritic, Gothic, Carian, Lycian, Hangul, Cyrillic,
    Hebrew, Armenian, Meetei_Mayek, Cherokee, Tai_Viet, Myanmar, Cham, Javanese, Rejang,
    Kayah_Li, Devanagari, Saurashtra, Phags_Pa, Syloti_Nagri, Vai, Yi, Bopomofo,
    Tifinagh, Georgian, Coptic, Braille, Sundanese, Ol_Chiki, Lepcha, Batak, Balinese,
    Tai_Tham, Buginese, Khmer, Limbu, Tai_Le, Tagbanwa, Buhid, Hanunoo, Tagalog, Runic,
    Ogham, Tibetan, Lao, Thai, Malayalam, Kannada, Telugu, Oriya, Gujarati, Gurmukhi,
    Bengali, Syriac, Mandaic, Samaritan, Nko, Thaana,

    Linear_A,
    Linear_B,

    New_Tai_Lue,

    Old_Hungarian,
    Old_Turkic,
    Old_Uyghur,
    Old_Sogdian,
    Old_South_Arabian,
    Old_North_Arabian,
    Old_Persian,
    Old_Permic,
    Old_Italic,

    Inscriptional_Pahlavi,
    Inscriptional_Parthian,

    Anatolian_Hieroglyphs,
    Egyptian_Hieroglyphs,
    Meroitic_Hieroglyphs,

    Emoji
};

enum class GraphemeBreakType : uint8_t
{
    other, cr, lf, control, extend, regionalIndicator, prepend, spacingMark,
    l, v, t, lv, lvt, zwj
};


namespace generated
{
#include "juce_UnicodeData.cpp"
#ifdef JUCE_UNIT_TESTS
 #include "juce_UnicodeTestData.cpp"
#endif
}

using UnicodeData = generated::UnicodeEntry;

struct UnicodeAnalysisPoint
{
    uint32_t    character;
    UnicodeData data;

    struct
    {
        uint16_t level;
    } bidi;
};

static UnicodeData getUnicodeDataForCodepoint (uint32_t codepoint)
{
    static const Array<UnicodeData> data = []
    {
        using namespace generated;

        Array<UnicodeData> arr;

        MemoryInputStream mStream {compressedUnicodeData, std::size (compressedUnicodeData), false};
        GZIPDecompressorInputStream zStream {&mStream, false};

        // TODO: error checking
        arr.resize (uncompressedUnicodeDataSize / sizeof (UnicodeData));
        zStream.read (arr.getRawDataPointer(), uncompressedUnicodeDataSize);

        return arr;
    }();

    return data[(int) codepoint];
}

// https://www.unicode.org/Public/UCD/latest/ucd/Jamo.txt
static inline bool isJamoSymbol (uint32_t cp)
{
    return ((cp >= 0x1100 && cp <= 0x1112) ||
            (cp >= 0x1161 && cp <= 0x1175) ||
            (cp >= 0x11A8 && cp <= 0x11C2));
}

static inline EmojiType getEmojiType (uint32_t cp)
{
    return getUnicodeDataForCodepoint (cp).emoji;
}

} // namespace juce
