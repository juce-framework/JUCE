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
        SBScript previousBaseTextScript = SBScriptZYYY;

        for (const auto [i, value] : enumerate (points))
        {
            auto script = value.getScriptType();

            if (! std::exchange (once, true))
            {
                if (script == SBScriptZINH)
                    script = SBScriptZYYY;

                previousBaseTextScript = script;
            }

            if (script == SBScriptZYYY || script == SBScriptZINH)
                script = previousBaseTextScript;

            callback ((int) i, mapTextScript (script));
            previousBaseTextScript = script;
        }
    }

private:
    // The Unicode script spec lists a large number of scripts, some of which are recommended to be ignored.
    // We map them to a script that we support here.
    static TextScript mapTextScript (SBScript type)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")

        switch (type)
        {
            case SBScriptZYYY:          return TextScript::common;
            case SBScriptARAB:          return TextScript::arabic;
            case SBScriptARMN:          return TextScript::armenian;
            case SBScriptBENG:          return TextScript::bengali;
            case SBScriptBOPO:          return TextScript::bopomofo;
            case SBScriptCYRL:          return TextScript::cyrillic;
            case SBScriptDEVA:          return TextScript::devanagari;
            case SBScriptETHI:          return TextScript::ethiopic;
            case SBScriptGEOR:          return TextScript::georgian;
            case SBScriptGREK:          return TextScript::greek;
            case SBScriptGUJR:          return TextScript::gujarati;
            case SBScriptGURU:          return TextScript::gurmukhi;
            case SBScriptHANG:          return TextScript::hangul;
            case SBScriptHANI:          return TextScript::han;
            case SBScriptHEBR:          return TextScript::hebrew;
            case SBScriptHIRA:          return TextScript::hiragana;
            case SBScriptKANA:          return TextScript::katakana;
            case SBScriptKNDA:          return TextScript::kannada;
            case SBScriptKHMR:          return TextScript::khmer;
            case SBScriptLAOO:          return TextScript::lao;
            case SBScriptLATN:          return TextScript::latin;
            case SBScriptMLYM:          return TextScript::malayalam;
            case SBScriptMYMR:          return TextScript::myanmar;
            case SBScriptORYA:          return TextScript::oriya;
            case SBScriptSINH:          return TextScript::sinhala;
            case SBScriptTAML:          return TextScript::tamil;
            case SBScriptTELU:          return TextScript::telugu;
            case SBScriptTHAA:          return TextScript::thaana;
            case SBScriptTHAI:          return TextScript::thai;
            case SBScriptTIBT:          return TextScript::tibetan;

            case SBScriptADLM:          return TextScript::adlam;
            case SBScriptBALI:          return TextScript::balinese;
            case SBScriptBAMU:          return TextScript::bamum;
            case SBScriptBATK:          return TextScript::batak;
            case SBScriptCAKM:          return TextScript::chakma;
            case SBScriptCHAM:          return TextScript::cham;
            case SBScriptCHER:          return TextScript::cherokee;
            case SBScriptJAVA:          return TextScript::javanese;
            case SBScriptKALI:          return TextScript::kayahLi;
            case SBScriptLANA:          return TextScript::taiTham;
            case SBScriptLEPC:          return TextScript::lepcha;
            case SBScriptLIMB:          return TextScript::limbu;
            case SBScriptLISU:          return TextScript::lisu;
            case SBScriptMAND:          return TextScript::mandaic;
            case SBScriptMTEI:          return TextScript::meeteiMayek;
            case SBScriptNEWA:          return TextScript::newa;
            case SBScriptNKOO:          return TextScript::nko;
            case SBScriptOLCK:          return TextScript::olChiki;
            case SBScriptOSGE:          return TextScript::osage;
            case SBScriptPLRD:          return TextScript::miao;
            case SBScriptSAUR:          return TextScript::saurashtra;
            case SBScriptSUND:          return TextScript::sundanese;
            case SBScriptSYLO:          return TextScript::sylotiNagri;
            case SBScriptSYRC:          return TextScript::syriac;
            case SBScriptTALE:          return TextScript::taiLe;
            case SBScriptTALU:          return TextScript::newTaiLue;
            case SBScriptTAVT:          return TextScript::taiViet;
            case SBScriptTFNG:          return TextScript::tifinagh;
            case SBScriptVAII:          return TextScript::vai;
            case SBScriptWCHO:          return TextScript::wancho;
            case SBScriptYIII:          return TextScript::yi;

            case SBScriptROHG:          return TextScript::hanifiRohingya;
            case SBScriptHMNP:          return TextScript::nyiakengPuachueHmong;
            case SBScriptCANS:          return TextScript::canadianAboriginalSyllabics;

            default: break;
        }

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return TextScript::common;
    }
};

}
