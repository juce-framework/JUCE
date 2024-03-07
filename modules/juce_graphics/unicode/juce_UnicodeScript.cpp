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

inline TextScript mapTextScript (UnicodeTextScript type)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")

   #define CASE(in, out) case UnicodeTextScript::in: return TextScript::out
    switch (type)
    {
        CASE (Common,       common);
        CASE (Emoji,        emoji);
        CASE (Arabic,       arabic);
        CASE (Armenian,     armenian);
        CASE (Bengali,      bengali);
        CASE (Bopomofo,     bopomofo);
        CASE (Cyrillic,     cyrillic);
        CASE (Devanagari,   devanagari);
        CASE (Ethiopic,     ethiopic);
        CASE (Georgian,     georgian);
        CASE (Greek,        greek);
        CASE (Gujarati,     gujarati);
        CASE (Gurmukhi,     gurmukhi);
        CASE (Hangul,       hangul);
        CASE (Han,          han);
        CASE (Hebrew,       hebrew);
        CASE (Hiragana,     hiragana);
        CASE (Katakana,     katakana);
        CASE (Kannada,      kannada);
        CASE (Khmer,        khmer);
        CASE (Lao,          lao);
        CASE (Latin,        latin);
        CASE (Malayalam,    malayalam);
        CASE (Myanmar,      myanmar);
        CASE (Oriya,        oriya);
        CASE (Sinhala,      sinhala);
        CASE (Tamil,        tamil);
        CASE (Telugu,       telugu);
        CASE (Thaana,       thaana);
        CASE (Thai,         thai);
        CASE (Tibetan,      tibetan);

        CASE (Adlam,        adlam);
        CASE (Balinese,     balinese);
        CASE (Bamum,        bamum);
        CASE (Batak,        batak);
        CASE (Chakma,       chakma);
        CASE (Cham,         cham);
        CASE (Cherokee,     cherokee);
        CASE (Javanese,     javanese);
        CASE (Kayah_Li,     kayahLi);
        CASE (Tai_Tham,     taiTham);
        CASE (Lepcha,       lepcha);
        CASE (Limbu,        limbu);
        CASE (Lisu,         lisu);
        CASE (Mandaic,      mandaic);
        CASE (Meetei_Mayek, meeteiMayek);
        CASE (Newa,         newa);
        CASE (Nko,          nko);
        CASE (Ol_Chiki,     olChiki);
        CASE (Osage,        osage);
        CASE (Miao,         miao);
        CASE (Saurashtra,   saurashtra);
        CASE (Sundanese,    sundanese);
        CASE (Syloti_Nagri, sylotiNagri);
        CASE (Syriac,       syriac);
        CASE (Tai_Le,       taiLe);
        CASE (New_Tai_Lue,  newTaiLue);
        CASE (Tai_Viet,     taiViet);
        CASE (Tifinagh,     tifinagh);
        CASE (Vai,          vai);
        CASE (Wancho,       wancho);
        CASE (Yi,           yi);

        CASE (Hanifi_Rohingya,        hanifiRohingya);
        CASE (Nyiakeng_Puachue_Hmong, nyiakengPuachueHmong);
        CASE (Canadian_Aboriginal,    canadianAboriginalSyllabics);

    default: break;
    }
   #undef CASE

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return TextScript::common;
}

// TR24
// https://www.unicode.org/reports/tr24/tr24-32.html
namespace tr24
{

template <typename Callback>
void inline analyseScripts (const Span<UnicodeAnalysisPoint> points, Callback&& callback)
{
    bool once = false;
    UnicodeTextScript previousBaseTextScript = UnicodeTextScript::Common;

    for (size_t i = 0; i < points.size(); i++)
    {
        const auto& entry      = points[i].data;
        auto script            = entry.script;

        if (! std::exchange (once, true))
        {
            if (script == UnicodeTextScript::Inherited)
                script = UnicodeTextScript::Common;

            previousBaseTextScript = script;
        }

        if (script == UnicodeTextScript::Common && entry.emoji == EmojiType::extended)
            script = UnicodeTextScript::Emoji;

        // Last part is a hack..
        if (script == UnicodeTextScript::Common || script == UnicodeTextScript::Inherited)
            script = previousBaseTextScript;

        callback ((int) i, mapTextScript (script));
        previousBaseTextScript = script;
    }
}
}

}
