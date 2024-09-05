/*
 * Copyright (C) 2018-2021 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SB_PUBLIC_SCRIPT_H
#define _SB_PUBLIC_SCRIPT_H

#include "SBBase.h"

/**
 * Constants that specify the script of a character.
 */
enum {
    SBScriptNil  = 0x00,

    SBScriptZINH = 0x01,    /**< Inherited */
    SBScriptZYYY = 0x02,    /**< Common */
    SBScriptZZZZ = 0x03,    /**< Unknown */

    /* Unicode 1.1  */
    SBScriptARAB = 0x04,    /**< Arabic */
    SBScriptARMN = 0x05,    /**< Armenian */
    SBScriptBENG = 0x06,    /**< Bengali */
    SBScriptBOPO = 0x07,    /**< Bopomofo */
    SBScriptCYRL = 0x08,    /**< Cyrillic */
    SBScriptDEVA = 0x09,    /**< Devanagari */
    SBScriptGEOR = 0x0A,    /**< Georgian */
    SBScriptGREK = 0x0B,    /**< Greek */
    SBScriptGUJR = 0x0C,    /**< Gujarati */
    SBScriptGURU = 0x0D,    /**< Gurmukhi */
    SBScriptHANG = 0x0E,    /**< Hangul */
    SBScriptHANI = 0x0F,    /**< Han */
    SBScriptHEBR = 0x10,    /**< Hebrew */
    SBScriptHIRA = 0x11,    /**< Hiragana */
    SBScriptKANA = 0x12,    /**< Katakana */
    SBScriptKNDA = 0x13,    /**< Kannada */
    SBScriptLAOO = 0x14,    /**< Lao */
    SBScriptLATN = 0x15,    /**< Latin */
    SBScriptMLYM = 0x16,    /**< Malayalam */
    SBScriptORYA = 0x17,    /**< Oriya */
    SBScriptTAML = 0x18,    /**< Tamil */
    SBScriptTELU = 0x19,    /**< Telugu */
    SBScriptTHAI = 0x1A,    /**< Thai */

    /* Unicode 2.0 */
    SBScriptTIBT = 0x1B,    /**< Tibetan */

    /* Unicode 3.0 */
    SBScriptBRAI = 0x1C,    /**< Braille */
    SBScriptCANS = 0x1D,    /**< Canadian_Aboriginal */
    SBScriptCHER = 0x1E,    /**< Cherokee */
    SBScriptETHI = 0x1F,    /**< Ethiopic */
    SBScriptKHMR = 0x20,    /**< Khmer */
    SBScriptMONG = 0x21,    /**< Mongolian */
    SBScriptMYMR = 0x22,    /**< Myanmar */
    SBScriptOGAM = 0x23,    /**< Ogham */
    SBScriptRUNR = 0x24,    /**< Runic */
    SBScriptSINH = 0x25,    /**< Sinhala */
    SBScriptSYRC = 0x26,    /**< Syriac */
    SBScriptTHAA = 0x27,    /**< Thaana */
    SBScriptYIII = 0x28,    /**< Yi */

    /* Unicode 3.1 */
    SBScriptDSRT = 0x29,    /**< Deseret */
    SBScriptGOTH = 0x2A,    /**< Gothic */
    SBScriptITAL = 0x2B,    /**< Old_Italic */

    /* Unicode 3.2 */
    SBScriptBUHD = 0x2C,    /**< Buhid */
    SBScriptHANO = 0x2D,    /**< Hanunoo */
    SBScriptTAGB = 0x2E,    /**< Tagbanwa */
    SBScriptTGLG = 0x2F,    /**< Tagalog */

    /* Unicode 4.0 */
    SBScriptCPRT = 0x30,    /**< Cypriot */
    SBScriptLIMB = 0x31,    /**< Limbu */
    SBScriptLINB = 0x32,    /**< Linear_B */
    SBScriptOSMA = 0x33,    /**< Osmanya */
    SBScriptSHAW = 0x34,    /**< Shavian */
    SBScriptTALE = 0x35,    /**< Tai_Le */
    SBScriptUGAR = 0x36,    /**< Ugaritic */

    /* Unicode 4.1 */
    SBScriptBUGI = 0x37,    /**< Buginese */
    SBScriptCOPT = 0x38,    /**< Coptic */
    SBScriptGLAG = 0x39,    /**< Glagolitic */
    SBScriptKHAR = 0x3A,    /**< Kharoshthi */
    SBScriptSYLO = 0x3B,    /**< Syloti_Nagri */
    SBScriptTALU = 0x3C,    /**< New_Tai_Lue */
    SBScriptTFNG = 0x3D,    /**< Tifinagh */
    SBScriptXPEO = 0x3E,    /**< Old_Persian */

    /* Unicode 5.0 */
    SBScriptBALI = 0x3F,    /**< Balinese */
    SBScriptNKOO = 0x40,    /**< Nko */
    SBScriptPHAG = 0x41,    /**< Phags_Pa */
    SBScriptPHNX = 0x42,    /**< Phoenician */
    SBScriptXSUX = 0x43,    /**< Cuneiform */

    /* Unicode 5.1 */
    SBScriptCARI = 0x44,    /**< Carian */
    SBScriptCHAM = 0x45,    /**< Cham */
    SBScriptKALI = 0x46,    /**< Kayah_Li */
    SBScriptLEPC = 0x47,    /**< Lepcha */
    SBScriptLYCI = 0x48,    /**< Lycian */
    SBScriptLYDI = 0x49,    /**< Lydian */
    SBScriptOLCK = 0x4A,    /**< Ol_Chiki */
    SBScriptRJNG = 0x4B,    /**< Rejang */
    SBScriptSAUR = 0x4C,    /**< Saurashtra */
    SBScriptSUND = 0x4D,    /**< Sundanese */
    SBScriptVAII = 0x4E,    /**< Vai */

    /* Unicode 5.2 */
    SBScriptARMI = 0x4F,    /**< Imperial_Aramaic */
    SBScriptAVST = 0x50,    /**< Avestan */
    SBScriptBAMU = 0x51,    /**< Bamum */
    SBScriptEGYP = 0x52,    /**< Egyptian_Hieroglyphs */
    SBScriptJAVA = 0x53,    /**< Javanese */
    SBScriptKTHI = 0x54,    /**< Kaithi */
    SBScriptLANA = 0x55,    /**< Tai_Tham */
    SBScriptLISU = 0x56,    /**< Lisu */
    SBScriptMTEI = 0x57,    /**< Meetei_Mayek */
    SBScriptORKH = 0x58,    /**< Old_Turkic */
    SBScriptPHLI = 0x59,    /**< Inscriptional_Pahlavi */
    SBScriptPRTI = 0x5A,    /**< Inscriptional_Parthian */
    SBScriptSAMR = 0x5B,    /**< Samaritan */
    SBScriptSARB = 0x5C,    /**< Old_South_Arabian */
    SBScriptTAVT = 0x5D,    /**< Tai_Viet */

    /* Unicode 6.0 */
    SBScriptBATK = 0x5E,    /**< Batak */
    SBScriptBRAH = 0x5F,    /**< Brahmi */
    SBScriptMAND = 0x60,    /**< Mandaic */

    /* Unicode 6.1 */
    SBScriptCAKM = 0x61,    /**< Chakma */
    SBScriptMERC = 0x62,    /**< Meroitic_Cursive */
    SBScriptMERO = 0x63,    /**< Meroitic_Hieroglyphs */
    SBScriptPLRD = 0x64,    /**< Miao */
    SBScriptSHRD = 0x65,    /**< Sharada */
    SBScriptSORA = 0x66,    /**< Sora_Sompeng */
    SBScriptTAKR = 0x67,    /**< Takri */

    /* Unicode 7.0 */
    SBScriptAGHB = 0x68,    /**< Caucasian_Albanian */
    SBScriptBASS = 0x69,    /**< Bassa_Vah */
    SBScriptDUPL = 0x6A,    /**< Duployan */
    SBScriptELBA = 0x6B,    /**< Elbasan */
    SBScriptGRAN = 0x6C,    /**< Grantha */
    SBScriptHMNG = 0x6D,    /**< Pahawh_Hmong */
    SBScriptKHOJ = 0x6E,    /**< Khojki */
    SBScriptLINA = 0x6F,    /**< Linear_A */
    SBScriptMAHJ = 0x70,    /**< Mahajani */
    SBScriptMANI = 0x71,    /**< Manichaean */
    SBScriptMEND = 0x72,    /**< Mende_Kikakui */
    SBScriptMODI = 0x73,    /**< Modi */
    SBScriptMROO = 0x74,    /**< Mro */
    SBScriptNARB = 0x75,    /**< Old_North_Arabian */
    SBScriptNBAT = 0x76,    /**< Nabataean */
    SBScriptPALM = 0x77,    /**< Palmyrene */
    SBScriptPAUC = 0x78,    /**< Pau_Cin_Hau */
    SBScriptPERM = 0x79,    /**< Old_Permic */
    SBScriptPHLP = 0x7A,    /**< Psalter_Pahlavi */
    SBScriptSIDD = 0x7B,    /**< Siddham */
    SBScriptSIND = 0x7C,    /**< Khudawadi */
    SBScriptTIRH = 0x7D,    /**< Tirhuta */
    SBScriptWARA = 0x7E,    /**< Warang_Citi */

    /* Unicode 8.0 */
    SBScriptAHOM = 0x7F,    /**< Ahom */
    SBScriptHATR = 0x80,    /**< Hatran */
    SBScriptHLUW = 0x81,    /**< Anatolian_Hieroglyphs */
    SBScriptHUNG = 0x82,    /**< Old_Hungarian */
    SBScriptMULT = 0x83,    /**< Multani */
    SBScriptSGNW = 0x84,    /**< SignWriting */

    /* Unicode 9.0 */
    SBScriptADLM = 0x85,    /**< Adlam */
    SBScriptBHKS = 0x86,    /**< Bhaiksuki */
    SBScriptMARC = 0x87,    /**< Marchen */
    SBScriptNEWA = 0x88,    /**< Newa */
    SBScriptOSGE = 0x89,    /**< Osage */
    SBScriptTANG = 0x8A,    /**< Tangut */

    /* Unicode 10.0 */
    SBScriptGONM = 0x8B,    /**< Masaram_Gondi */
    SBScriptNSHU = 0x8C,    /**< Nushu */
    SBScriptSOYO = 0x8D,    /**< Soyombo */
    SBScriptZANB = 0x8E,    /**< Zanabazar_Square */

    /* Unicode 11.0 */
    SBScriptDOGR = 0x8F,    /**< Dogra */
    SBScriptGONG = 0x90,    /**< Gunjala_Gondi */
    SBScriptMAKA = 0x91,    /**< Makasar */
    SBScriptMEDF = 0x92,    /**< Medefaidrin */
    SBScriptROHG = 0x93,    /**< Hanifi_Rohingya */
    SBScriptSOGD = 0x94,    /**< Sogdian */
    SBScriptSOGO = 0x95,    /**< Old_Sogdian */

    /* Unicode 12.0 */
    SBScriptELYM = 0x96,    /**< Elymaic */
    SBScriptHMNP = 0x97,    /**< Nyiakeng_Puachue_Hmong */
    SBScriptNAND = 0x98,    /**< Nandinagari */
    SBScriptWCHO = 0x99,    /**< Wancho */

    /* Unicde 13.0 */
    SBScriptCHRS = 0x9A,    /**< Chorasmian */
    SBScriptDIAK = 0x9B,    /**< Dives_Akuru */
    SBScriptKITS = 0x9C,    /**< Khitan_Small_Script */
    SBScriptYEZI = 0x9D,    /**< Yezidi */

    /* Unicde 14.0 */
    SBScriptCPMN = 0x9E,    /**< Cypro_Minoan */
    SBScriptOUGR = 0x9F,    /**< Old_Uyghur */
    SBScriptTNSA = 0xA0,    /**< Tangsa */
    SBScriptTOTO = 0xA1,    /**< Toto */
    SBScriptVITH = 0xA2,    /**< Vithkuqi */

    /* Unicde 15.1 */
    SBScriptKAWI = 0xA3,    /**< Kawi */
    SBScriptNAGM = 0xA4     /**< Nag_Mundari */
};

/**
 * A type to represent the script of a character.
 */
typedef SBUInt8 SBScript;

/**
 * Returns the OpenType tag of a script as UInt32 in big endian byte order. The association between
 * Unicode Script property and OpenType script tags is taken from the specification:
 * https://docs.microsoft.com/en-us/typography/opentype/spec/scripttags.
 *
 * If more than one tag is associated with a script, then the latest one is retured. For example,
 * Devanagari script has two tags, `deva` and `dev2`. So in this case, `dev2` will be returned.
 *
 * If no tag is associated with a script, then `DFLT` is returned.
 *
 * @param script
 *      The script whose OpenType tag is returned.
 * @return
 *      The OpenType tag of specified script as UInt32 in big endian byte order.
 */
SBUInt32 SBScriptGetOpenTypeTag(SBScript script);

#endif
