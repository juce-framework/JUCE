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
// https://www.unicode.org/reports/tr31/#Table_Recommended_Scripts
enum class TextScript
{
    // Recommend scripts
    common,
    arabic,
    armenian,
    bengali,
    bopomofo,
    cyrillic,
    devanagari,
    ethiopic,
    georgian,
    greek,
    gujarati,
    gurmukhi,
    hangul,
    han,
    hebrew,
    hiragana,
    katakana,
    kannada,
    khmer,
    lao,
    latin,
    malayalam,
    myanmar,
    oriya,
    sinhala,
    tamil,
    telugu,
    thaana,
    thai,
    tibetan,

    // Limited use
    adlam,
    balinese,
    bamum,
    batak,
    chakma,
    canadianAboriginalSyllabics,
    cham,
    cherokee,
    nyiakengPuachueHmong,
    javanese,
    kayahLi,
    taiTham,
    lepcha,
    limbu,
    lisu,
    mandaic,
    meeteiMayek,
    newa,
    nko,
    olChiki,
    osage,
    miao,
    hanifiRohingya,
    saurashtra,
    sundanese,
    sylotiNagri,
    syriac,
    taiLe,
    newTaiLue,
    taiViet,
    tifinagh,
    vai,
    wancho,
    yi,

    emoji,

    scriptCount
};

// https://www.unicode.org/reports/tr24/tr24-32.html
class TR24
{
public:
    TR24() = delete;

    template <typename Callback>
    static void analyseScripts (Span<const UnicodeAnalysisPoint> points, Callback&& callback)
    {
        bool once = false;
        auto previousBaseTextScript = UnicodeScriptType::common;

        for (const auto [i, value] : enumerate (points))
        {
            const auto& entry = value.data;
            auto script       = entry.script;

            if (! std::exchange (once, true))
            {
                if (script == UnicodeScriptType::inherited)
                    script = UnicodeScriptType::common;

                previousBaseTextScript = script;
            }

            if (script == UnicodeScriptType::common && entry.emoji == EmojiType::extended)
                script = UnicodeScriptType::emoji;

            if (script == UnicodeScriptType::common || script == UnicodeScriptType::inherited)
                script = previousBaseTextScript;

            callback ((int) i, mapTextScript (script));
            previousBaseTextScript = script;
        }
    }

private:
    // The Unicode script spec lists a large number of scripts, some of which are recommended to be ignored.
    // We map them to a script that we support here.
    static TextScript mapTextScript (UnicodeScriptType type)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")

        switch (type)
        {
            case UnicodeScriptType::common:         return TextScript::common;
            case UnicodeScriptType::emoji:          return TextScript::emoji;
            case UnicodeScriptType::arabic:         return TextScript::arabic;
            case UnicodeScriptType::armenian:       return TextScript::armenian;
            case UnicodeScriptType::bengali:        return TextScript::bengali;
            case UnicodeScriptType::bopomofo:       return TextScript::bopomofo;
            case UnicodeScriptType::cyrillic:       return TextScript::cyrillic;
            case UnicodeScriptType::devanagari:     return TextScript::devanagari;
            case UnicodeScriptType::ethiopic:       return TextScript::ethiopic;
            case UnicodeScriptType::georgian:       return TextScript::georgian;
            case UnicodeScriptType::greek:          return TextScript::greek;
            case UnicodeScriptType::gujarati:       return TextScript::gujarati;
            case UnicodeScriptType::gurmukhi:       return TextScript::gurmukhi;
            case UnicodeScriptType::hangul:         return TextScript::hangul;
            case UnicodeScriptType::han:            return TextScript::han;
            case UnicodeScriptType::hebrew:         return TextScript::hebrew;
            case UnicodeScriptType::hiragana:       return TextScript::hiragana;
            case UnicodeScriptType::katakana:       return TextScript::katakana;
            case UnicodeScriptType::kannada:        return TextScript::kannada;
            case UnicodeScriptType::khmer:          return TextScript::khmer;
            case UnicodeScriptType::lao:            return TextScript::lao;
            case UnicodeScriptType::latin:          return TextScript::latin;
            case UnicodeScriptType::malayalam:      return TextScript::malayalam;
            case UnicodeScriptType::myanmar:        return TextScript::myanmar;
            case UnicodeScriptType::oriya:          return TextScript::oriya;
            case UnicodeScriptType::sinhala:        return TextScript::sinhala;
            case UnicodeScriptType::tamil:          return TextScript::tamil;
            case UnicodeScriptType::telugu:         return TextScript::telugu;
            case UnicodeScriptType::thaana:         return TextScript::thaana;
            case UnicodeScriptType::thai:           return TextScript::thai;
            case UnicodeScriptType::tibetan:        return TextScript::tibetan;

            case UnicodeScriptType::adlam:          return TextScript::adlam;
            case UnicodeScriptType::balinese:       return TextScript::balinese;
            case UnicodeScriptType::bamum:          return TextScript::bamum;
            case UnicodeScriptType::batak:          return TextScript::batak;
            case UnicodeScriptType::chakma:         return TextScript::chakma;
            case UnicodeScriptType::cham:           return TextScript::cham;
            case UnicodeScriptType::cherokee:       return TextScript::cherokee;
            case UnicodeScriptType::javanese:       return TextScript::javanese;
            case UnicodeScriptType::kayah_li:       return TextScript::kayahLi;
            case UnicodeScriptType::tai_tham:       return TextScript::taiTham;
            case UnicodeScriptType::lepcha:         return TextScript::lepcha;
            case UnicodeScriptType::limbu:          return TextScript::limbu;
            case UnicodeScriptType::lisu:           return TextScript::lisu;
            case UnicodeScriptType::mandaic:        return TextScript::mandaic;
            case UnicodeScriptType::meetei_mayek:   return TextScript::meeteiMayek;
            case UnicodeScriptType::newa:           return TextScript::newa;
            case UnicodeScriptType::nko:            return TextScript::nko;
            case UnicodeScriptType::ol_chiki:       return TextScript::olChiki;
            case UnicodeScriptType::osage:          return TextScript::osage;
            case UnicodeScriptType::miao:           return TextScript::miao;
            case UnicodeScriptType::saurashtra:     return TextScript::saurashtra;
            case UnicodeScriptType::sundanese:      return TextScript::sundanese;
            case UnicodeScriptType::syloti_nagri:   return TextScript::sylotiNagri;
            case UnicodeScriptType::syriac:         return TextScript::syriac;
            case UnicodeScriptType::tai_le:         return TextScript::taiLe;
            case UnicodeScriptType::new_tai_lue:    return TextScript::newTaiLue;
            case UnicodeScriptType::tai_viet:       return TextScript::taiViet;
            case UnicodeScriptType::tifinagh:       return TextScript::tifinagh;
            case UnicodeScriptType::vai:            return TextScript::vai;
            case UnicodeScriptType::wancho:         return TextScript::wancho;
            case UnicodeScriptType::yi:             return TextScript::yi;

            case UnicodeScriptType::hanifi_rohingya:        return TextScript::hanifiRohingya;
            case UnicodeScriptType::nyiakeng_puachue_hmong: return TextScript::nyiakengPuachueHmong;
            case UnicodeScriptType::canadian_aboriginal:    return TextScript::canadianAboriginalSyllabics;

            default: break;
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return TextScript::common;
    }
};

}
