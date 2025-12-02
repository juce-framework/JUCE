/*
 * Copyright (C) 2016-2025 Muhammad Tayyab Akram
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

#include <juce_graphics/unicode/sheenbidi/Headers/SheenBidi/SBConfig.h>
#include <juce_graphics/unicode/sheenbidi/Headers/SheenBidi/SBVersion.h>

#include "SBBase.h"

static const char LibraryVersion[] = SHEENBIDI_VERSION_STRING;

#define TAG(a, b, c, d) \
(SBUInt32)              \
(                       \
   ((SBUInt8)(a) << 24) \
 | ((SBUInt8)(b) << 16) \
 | ((SBUInt8)(c) <<  8) \
 | ((SBUInt8)(d) <<  0) \
)

SB_INTERNAL void SBUIntegerNormalizeRange(SBUInteger actualLength,
    SBUInteger *rangeOffset, SBUInteger *rangeLength)
{
    /**
     * Assume:
     *      Actual Length = 10
     *
     * Case 1:
     *      Offset = 0, Length = 10
     * Result:
     *      Offset = 0, Length = 10
     *
     * Case 2:
     *      Offset = 0, Length = 11
     * Result:
     *      Offset = 0, Length = 10
     *
     * Case 3:
     *      Offset = 1, Length = -1 (MAX)
     * Result:
     *      Offset = 1, Length = 9
     *
     * Case 4:
     *      Offset = 10, Length = 0
     * Result:
     *      Offset = Invalid, Length = 0
     *
     * Case 5:
     *      Offset = -1 (MAX), Length = 1
     * Result:
     *      Offset = Invalid, Length = 0
     */

    if (*rangeOffset < actualLength) {
        SBUInteger possibleLimit = *rangeOffset + *rangeLength;

        if (*rangeOffset <= possibleLimit && possibleLimit <= actualLength) {
            /* The range is valid. Nothing to do here. */
        } else {
            *rangeLength = actualLength - *rangeOffset;
        }
    } else {
        *rangeOffset = SBInvalidIndex;
        *rangeLength = 0;
    }
}

SB_INTERNAL SBBoolean SBUIntegerVerifyRange(SBUInteger actualLength,
    SBUInteger rangeOffset, SBUInteger rangeLength)
{
    SBUInteger possibleLimit = rangeOffset + rangeLength;

    return rangeOffset < actualLength
        && rangeOffset <= possibleLimit
        && possibleLimit <= actualLength;
}

SBUInt32 SBScriptGetOpenTypeTag(SBScript script)
{
    /* Reference: https://docs.microsoft.com/en-us/typography/opentype/spec/scripttags */
    /* Dated: 05/31/2024 */

    switch (script) {
    case SBScriptADLM:
        return TAG('a', 'd', 'l', 'm');
    case SBScriptAHOM:
        return TAG('a', 'h', 'o', 'm');
    case SBScriptHLUW:
        return TAG('h', 'l', 'u', 'w');
    case SBScriptARAB:
        return TAG('a', 'r', 'a', 'b');
    case SBScriptARMN:
        return TAG('a', 'r', 'm', 'n');
    case SBScriptAVST:
        return TAG('a', 'v', 's', 't');
    case SBScriptBALI:
        return TAG('b', 'a', 'l', 'i');
    case SBScriptBAMU:
        return TAG('b', 'a', 'm', 'u');
    case SBScriptBASS:
        return TAG('b', 'a', 's', 's');
    case SBScriptBATK:
        return TAG('b', 'a', 't', 'k');
 /* case SBScriptBENG:
        return TAG('b', 'e', 'n', 'g'); */
    case SBScriptBENG:
        return TAG('b', 'n', 'g', '2');
    case SBScriptBHKS:
        return TAG('b', 'h', 'k', 's');
    case SBScriptBOPO:
        return TAG('b', 'o', 'p', 'o');
    case SBScriptBRAH:
        return TAG('b', 'r', 'a', 'h');
    case SBScriptBRAI:
        return TAG('b', 'r', 'a', 'i');
    case SBScriptBUGI:
        return TAG('b', 'u', 'g', 'i');
    case SBScriptBUHD:
        return TAG('b', 'u', 'h', 'd');
 /* case SBScript____:
        return TAG('b', 'y', 'z', 'm'); */
    case SBScriptCANS:
        return TAG('c', 'a', 'n', 's');
    case SBScriptCARI:
        return TAG('c', 'a', 'r', 'i');
    case SBScriptAGHB:
        return TAG('a', 'g', 'h', 'b');
    case SBScriptCAKM:
        return TAG('c', 'a', 'k', 'm');
    case SBScriptCHAM:
        return TAG('c', 'h', 'a', 'm');
    case SBScriptCHER:
        return TAG('c', 'h', 'e', 'r');
    case SBScriptCHRS:
        return TAG('c', 'h', 'r', 's');
    case SBScriptHANI:
        return TAG('h', 'a', 'n', 'i');
    case SBScriptCOPT:
        return TAG('c', 'o', 'p', 't');
    case SBScriptCPRT:
        return TAG('c', 'p', 'r', 't');
    case SBScriptCPMN:
        return TAG('c', 'p', 'm', 'n');
    case SBScriptCYRL:
        return TAG('c', 'y', 'r', 'l');
    case SBScriptDSRT:
        return TAG('d', 's', 'r', 't');
 /* case SBScriptDEVA:
        return TAG('d', 'e', 'v', 'a'); */
    case SBScriptDEVA:
        return TAG('d', 'e', 'v', '2');
    case SBScriptDIAK:
        return TAG('d', 'i', 'a', 'k');
    case SBScriptDOGR:
        return TAG('d', 'o', 'g', 'r');
    case SBScriptDUPL:
        return TAG('d', 'u', 'p', 'l');
    case SBScriptEGYP:
        return TAG('e', 'g', 'y', 'p');
    case SBScriptELBA:
        return TAG('e', 'l', 'b', 'a');
    case SBScriptELYM:
        return TAG('e', 'l', 'y', 'm');
    case SBScriptETHI:
        return TAG('e', 't', 'h', 'i');
    case SBScriptGARA:
        return TAG('g', 'a', 'r', 'a');
    case SBScriptGEOR:
        return TAG('g', 'e', 'o', 'r');
    case SBScriptGLAG:
        return TAG('g', 'l', 'a', 'g');
    case SBScriptGOTH:
        return TAG('g', 'o', 't', 'h');
    case SBScriptGRAN:
        return TAG('g', 'r', 'a', 'n');
    case SBScriptGREK:
        return TAG('g', 'r', 'e', 'k');
 /* case SBScriptGUJR:
        return TAG('g', 'u', 'j', 'r'); */
    case SBScriptGUJR:
        return TAG('g', 'j', 'r', '2');
    case SBScriptGONG:
        return TAG('g', 'o', 'n', 'g');
 /* case SBScriptGURU:
        return TAG('g', 'u', 'r', 'u'); */
    case SBScriptGURU:
        return TAG('g', 'u', 'r', '2');
    case SBScriptGUKH:
        return TAG('g', 'u', 'k', 'h');
    case SBScriptHANG:
        return TAG('h', 'a', 'n', 'g');
 /* case SBScriptHANG:
        return TAG('j', 'a', 'm', 'o'); */
    case SBScriptROHG:
        return TAG('r', 'o', 'h', 'g');
    case SBScriptHANO:
        return TAG('h', 'a', 'n', 'o');
    case SBScriptHATR:
        return TAG('h', 'a', 't', 'r');
    case SBScriptHEBR:
        return TAG('h', 'e', 'b', 'r');
    case SBScriptHIRA:
        return TAG('k', 'a', 'n', 'a');
    case SBScriptARMI:
        return TAG('a', 'r', 'm', 'i');
    case SBScriptPHLI:
        return TAG('p', 'h', 'l', 'i');
    case SBScriptPRTI:
        return TAG('p', 'r', 't', 'i');
    case SBScriptJAVA:
        return TAG('j', 'a', 'v', 'a');
    case SBScriptKTHI:
        return TAG('k', 't', 'h', 'i');
 /* case SBScriptKNDA:
        return TAG('k', 'n', 'd', 'a'); */
    case SBScriptKNDA:
        return TAG('k', 'n', 'd', '2');
    case SBScriptKANA:
        return TAG('k', 'a', 'n', 'a');
    case SBScriptKAWI:
        return TAG('k', 'a', 'w', 'i');
    case SBScriptKALI:
        return TAG('k', 'a', 'l', 'i');
    case SBScriptKHAR:
        return TAG('k', 'h', 'a', 'r');
    case SBScriptKITS:
        return TAG('k', 'i', 't', 's');
    case SBScriptKHMR:
        return TAG('k', 'h', 'm', 'r');
    case SBScriptKHOJ:
        return TAG('k', 'h', 'o', 'j');
    case SBScriptSIND:
        return TAG('s', 'i', 'n', 'd');
    case SBScriptKRAI:
        return TAG('k', 'r', 'a', 'i');
    case SBScriptLAOO:
        return TAG('l', 'a', 'o', ' ');
    case SBScriptLATN:
        return TAG('l', 'a', 't', 'n');
    case SBScriptLEPC:
        return TAG('l', 'e', 'p', 'c');
    case SBScriptLIMB:
        return TAG('l', 'i', 'm', 'b');
    case SBScriptLINA:
        return TAG('l', 'i', 'n', 'a');
    case SBScriptLINB:
        return TAG('l', 'i', 'n', 'b');
    case SBScriptLISU:
        return TAG('l', 'i', 's', 'u');
    case SBScriptLYCI:
        return TAG('l', 'y', 'c', 'i');
    case SBScriptLYDI:
        return TAG('l', 'y', 'd', 'i');
    case SBScriptMAHJ:
        return TAG('m', 'a', 'h', 'j');
    case SBScriptMAKA:
        return TAG('m', 'a', 'k', 'a');
 /* case SBScriptMLYM:
        return TAG('m', 'l', 'y', 'm'); */
    case SBScriptMLYM:
        return TAG('m', 'l', 'm', '2');
    case SBScriptMAND:
        return TAG('m', 'a', 'n', 'd');
    case SBScriptMANI:
        return TAG('m', 'a', 'n', 'i');
    case SBScriptMARC:
        return TAG('m', 'a', 'r', 'c');
    case SBScriptGONM:
        return TAG('g', 'o', 'n', 'm');
 /* case SBScript____:
        return TAG('m', 'a', 't', 'h'); */
    case SBScriptMEDF:
        return TAG('m', 'e', 'd', 'f');
    case SBScriptMTEI:
        return TAG('m', 't', 'e', 'i');
    case SBScriptMEND:
        return TAG('m', 'e', 'n', 'd');
    case SBScriptMERC:
        return TAG('m', 'e', 'r', 'c');
    case SBScriptMERO:
        return TAG('m', 'e', 'r', 'o');
    case SBScriptPLRD:
        return TAG('p', 'l', 'r', 'd');
    case SBScriptMODI:
        return TAG('m', 'o', 'd', 'i');
    case SBScriptMONG:
        return TAG('m', 'o', 'n', 'g');
    case SBScriptMROO:
        return TAG('m', 'r', 'o', 'o');
    case SBScriptMULT:
        return TAG('m', 'u', 'l', 't');
 /* case SBScript___:
        return TAG('m', 'u', 's', 'c'); */
 /* case SBScriptMYMR:
        return TAG('m', 'y', 'm', 'r'); */
    case SBScriptMYMR:
        return TAG('m', 'y', 'm', '2');
    case SBScriptNBAT:
        return TAG('n', 'b', 'a', 't');
    case SBScriptNAGM:
        return TAG('n', 'a', 'g', 'm');
    case SBScriptNAND:
        return TAG('n', 'a', 'n', 'd');
    case SBScriptNEWA:
        return TAG('n', 'e', 'w', 'a');
    case SBScriptTALU:
        return TAG('t', 'a', 'l', 'u');
    case SBScriptNKOO:
        return TAG('n', 'k', 'o', ' ');
    case SBScriptNSHU:
        return TAG('n', 's', 'h', 'u');
    case SBScriptHMNP:
        return TAG('h', 'm', 'n', 'p');
 /* case SBScriptORYA:
        return TAG('o', 'r', 'y', 'a'); */
    case SBScriptORYA:
        return TAG('o', 'r', 'y', '2');
    case SBScriptOGAM:
        return TAG('o', 'g', 'a', 'm');
    case SBScriptOLCK:
        return TAG('o', 'l', 'c', 'k');
    case SBScriptONAO:
        return TAG('o', 'n', 'a', 'o');
    case SBScriptITAL:
        return TAG('i', 't', 'a', 'l');
    case SBScriptHUNG:
        return TAG('h', 'u', 'n', 'g');
    case SBScriptNARB:
        return TAG('n', 'a', 'r', 'b');
    case SBScriptPERM:
        return TAG('p', 'e', 'r', 'm');
    case SBScriptXPEO:
        return TAG('x', 'p', 'e', 'o');
    case SBScriptSOGO:
        return TAG('s', 'o', 'g', 'o');
    case SBScriptSARB:
        return TAG('s', 'a', 'r', 'b');
    case SBScriptORKH:
        return TAG('o', 'r', 'k', 'h');
    case SBScriptOUGR:
        return TAG('o', 'u', 'g', 'r');
    case SBScriptOSGE:
        return TAG('o', 's', 'g', 'e');
    case SBScriptOSMA:
        return TAG('o', 's', 'm', 'a');
    case SBScriptHMNG:
        return TAG('h', 'm', 'n', 'g');
    case SBScriptPALM:
        return TAG('p', 'a', 'l', 'm');
    case SBScriptPAUC:
        return TAG('p', 'a', 'u', 'c');
    case SBScriptPHAG:
        return TAG('p', 'h', 'a', 'g');
    case SBScriptPHNX:
        return TAG('p', 'h', 'n', 'x');
    case SBScriptPHLP:
        return TAG('p', 'h', 'l', 'p');
    case SBScriptRJNG:
        return TAG('r', 'j', 'n', 'g');
    case SBScriptRUNR:
        return TAG('r', 'u', 'n', 'r');
    case SBScriptSAMR:
        return TAG('s', 'a', 'm', 'r');
    case SBScriptSAUR:
        return TAG('s', 'a', 'u', 'r');
    case SBScriptSHRD:
        return TAG('s', 'h', 'r', 'd');
    case SBScriptSHAW:
        return TAG('s', 'h', 'a', 'w');
    case SBScriptSIDD:
        return TAG('s', 'i', 'd', 'd');
    case SBScriptSGNW:
        return TAG('s', 'g', 'n', 'w');
    case SBScriptSINH:
        return TAG('s', 'i', 'n', 'h');
    case SBScriptSOGD:
        return TAG('s', 'o', 'g', 'd');
    case SBScriptSORA:
        return TAG('s', 'o', 'r', 'a');
    case SBScriptSOYO:
        return TAG('s', 'o', 'y', 'o');
    case SBScriptXSUX:
        return TAG('x', 's', 'u', 'x');
    case SBScriptSUND:
        return TAG('s', 'u', 'n', 'd');
    case SBScriptSUNU:
        return TAG('s', 'u', 'n', 'u');
    case SBScriptSYLO:
        return TAG('s', 'y', 'l', 'o');
    case SBScriptSYRC:
        return TAG('s', 'y', 'r', 'c');
    case SBScriptTGLG:
        return TAG('t', 'g', 'l', 'g');
    case SBScriptTAGB:
        return TAG('t', 'a', 'g', 'b');
    case SBScriptTALE:
        return TAG('t', 'a', 'l', 'e');
    case SBScriptLANA:
        return TAG('l', 'a', 'n', 'a');
    case SBScriptTAVT:
        return TAG('t', 'a', 'v', 't');
    case SBScriptTAKR:
        return TAG('t', 'a', 'k', 'r');
 /* case SBScriptTAML:
        return TAG('t', 'a', 'm', 'l'); */
    case SBScriptTAML:
        return TAG('t', 'm', 'l', '2');
    case SBScriptTNSA:
        return TAG('t', 'n', 's', 'a');
    case SBScriptTANG:
        return TAG('t', 'a', 'n', 'g');
 /* case SBScriptTELU:
        return TAG('t', 'e', 'l', 'u'); */
    case SBScriptTELU:
        return TAG('t', 'e', 'l', '2');
    case SBScriptTHAA:
        return TAG('t', 'h', 'a', 'a');
    case SBScriptTHAI:
        return TAG('t', 'h', 'a', 'i');
    case SBScriptTIBT:
        return TAG('t', 'i', 'b', 't');
    case SBScriptTFNG:
        return TAG('t', 'f', 'n', 'g');
    case SBScriptTIRH:
        return TAG('t', 'i', 'r', 'h');
    case SBScriptTODR:
        return TAG('t', 'o', 'd', 'r');
    case SBScriptTOTO:
        return TAG('t', 'o', 't', 'o');
    case SBScriptTUTG:
        return TAG('t', 'u', 't', 'g');
    case SBScriptUGAR:
        return TAG('u', 'g', 'a', 'r');
    case SBScriptVAII:
        return TAG('v', 'a', 'i', ' ');
    case SBScriptVITH:
        return TAG('v', 'i', 't', 'h');
    case SBScriptWCHO:
        return TAG('w', 'c', 'h', 'o');
    case SBScriptWARA:
        return TAG('w', 'a', 'r', 'a');
    case SBScriptYEZI:
        return TAG('y', 'e', 'z', 'i');
    case SBScriptYIII:
        return TAG('y', 'i', ' ', ' ');
    case SBScriptZANB:
        return TAG('z', 'a', 'n', 'b');
    default:
        return TAG('D', 'F', 'L', 'T');
    }
}

SBUInt32 SBScriptGetUnicodeTag(SBScript script)
{
    switch (script) {
    case SBScriptADLM:
        return TAG('A', 'd', 'l', 'm');
    case SBScriptAGHB:
        return TAG('A', 'g', 'h', 'b');
    case SBScriptAHOM:
        return TAG('A', 'h', 'o', 'm');
    case SBScriptARAB:
        return TAG('A', 'r', 'a', 'b');
    case SBScriptARMI:
        return TAG('A', 'r', 'm', 'i');
    case SBScriptARMN:
        return TAG('A', 'r', 'm', 'n');
    case SBScriptAVST:
        return TAG('A', 'v', 's', 't');
    case SBScriptBALI:
        return TAG('B', 'a', 'l', 'i');
    case SBScriptBAMU:
        return TAG('B', 'a', 'm', 'u');
    case SBScriptBASS:
        return TAG('B', 'a', 's', 's');
    case SBScriptBATK:
        return TAG('B', 'a', 't', 'k');
    case SBScriptBENG:
        return TAG('B', 'e', 'n', 'g');
    case SBScriptBHKS:
        return TAG('B', 'h', 'k', 's');
    case SBScriptBOPO:
        return TAG('B', 'o', 'p', 'o');
    case SBScriptBRAH:
        return TAG('B', 'r', 'a', 'h');
    case SBScriptBRAI:
        return TAG('B', 'r', 'a', 'i');
    case SBScriptBUGI:
        return TAG('B', 'u', 'g', 'i');
    case SBScriptBUHD:
        return TAG('B', 'u', 'h', 'd');
    case SBScriptCAKM:
        return TAG('C', 'a', 'k', 'm');
    case SBScriptCANS:
        return TAG('C', 'a', 'n', 's');
    case SBScriptCARI:
        return TAG('C', 'a', 'r', 'i');
    case SBScriptCHAM:
        return TAG('C', 'h', 'a', 'm');
    case SBScriptCHER:
        return TAG('C', 'h', 'e', 'r');
    case SBScriptCHRS:
        return TAG('C', 'h', 'r', 's');
    case SBScriptCOPT:
        return TAG('C', 'o', 'p', 't');
    case SBScriptCPMN:
        return TAG('C', 'p', 'm', 'n');
    case SBScriptCPRT:
        return TAG('C', 'p', 'r', 't');
    case SBScriptCYRL:
        return TAG('C', 'y', 'r', 'l');
    case SBScriptDEVA:
        return TAG('D', 'e', 'v', 'a');
    case SBScriptDIAK:
        return TAG('D', 'i', 'a', 'k');
    case SBScriptDOGR:
        return TAG('D', 'o', 'g', 'r');
    case SBScriptDSRT:
        return TAG('D', 's', 'r', 't');
    case SBScriptDUPL:
        return TAG('D', 'u', 'p', 'l');
    case SBScriptEGYP:
        return TAG('E', 'g', 'y', 'p');
    case SBScriptELBA:
        return TAG('E', 'l', 'b', 'a');
    case SBScriptELYM:
        return TAG('E', 'l', 'y', 'm');
    case SBScriptETHI:
        return TAG('E', 't', 'h', 'i');
    case SBScriptGARA:
        return TAG('G', 'a', 'r', 'a');
    case SBScriptGEOR:
        return TAG('G', 'e', 'o', 'r');
    case SBScriptGLAG:
        return TAG('G', 'l', 'a', 'g');
    case SBScriptGONG:
        return TAG('G', 'o', 'n', 'g');
    case SBScriptGONM:
        return TAG('G', 'o', 'n', 'm');
    case SBScriptGOTH:
        return TAG('G', 'o', 't', 'h');
    case SBScriptGRAN:
        return TAG('G', 'r', 'a', 'n');
    case SBScriptGREK:
        return TAG('G', 'r', 'e', 'k');
    case SBScriptGUJR:
        return TAG('G', 'u', 'j', 'r');
    case SBScriptGUKH:
        return TAG('G', 'u', 'k', 'h');
    case SBScriptGURU:
        return TAG('G', 'u', 'r', 'u');
    case SBScriptHANG:
        return TAG('H', 'a', 'n', 'g');
    case SBScriptHANI:
        return TAG('H', 'a', 'n', 'i');
    case SBScriptHANO:
        return TAG('H', 'a', 'n', 'o');
    case SBScriptHATR:
        return TAG('H', 'a', 't', 'r');
    case SBScriptHEBR:
        return TAG('H', 'e', 'b', 'r');
    case SBScriptHIRA:
        return TAG('H', 'i', 'r', 'a');
    case SBScriptHLUW:
        return TAG('H', 'l', 'u', 'w');
    case SBScriptHMNG:
        return TAG('H', 'm', 'n', 'g');
    case SBScriptHMNP:
        return TAG('H', 'm', 'n', 'p');
    case SBScriptHUNG:
        return TAG('H', 'u', 'n', 'g');
    case SBScriptITAL:
        return TAG('I', 't', 'a', 'l');
    case SBScriptJAVA:
        return TAG('J', 'a', 'v', 'a');
    case SBScriptKALI:
        return TAG('K', 'a', 'l', 'i');
    case SBScriptKANA:
        return TAG('K', 'a', 'n', 'a');
    case SBScriptKAWI:
        return TAG('K', 'a', 'w', 'i');
    case SBScriptKHAR:
        return TAG('K', 'h', 'a', 'r');
    case SBScriptKHMR:
        return TAG('K', 'h', 'm', 'r');
    case SBScriptKHOJ:
        return TAG('K', 'h', 'o', 'j');
    case SBScriptKITS:
        return TAG('K', 'i', 't', 's');
    case SBScriptKNDA:
        return TAG('K', 'n', 'd', 'a');
    case SBScriptKRAI:
        return TAG('K', 'r', 'a', 'i');
    case SBScriptKTHI:
        return TAG('K', 't', 'h', 'i');
    case SBScriptLANA:
        return TAG('L', 'a', 'n', 'a');
    case SBScriptLAOO:
        return TAG('L', 'a', 'o', 'o');
    case SBScriptLATN:
        return TAG('L', 'a', 't', 'n');
    case SBScriptLEPC:
        return TAG('L', 'e', 'p', 'c');
    case SBScriptLIMB:
        return TAG('L', 'i', 'm', 'b');
    case SBScriptLINA:
        return TAG('L', 'i', 'n', 'a');
    case SBScriptLINB:
        return TAG('L', 'i', 'n', 'b');
    case SBScriptLISU:
        return TAG('L', 'i', 's', 'u');
    case SBScriptLYCI:
        return TAG('L', 'y', 'c', 'i');
    case SBScriptLYDI:
        return TAG('L', 'y', 'd', 'i');
    case SBScriptMAHJ:
        return TAG('M', 'a', 'h', 'j');
    case SBScriptMAKA:
        return TAG('M', 'a', 'k', 'a');
    case SBScriptMAND:
        return TAG('M', 'a', 'n', 'd');
    case SBScriptMANI:
        return TAG('M', 'a', 'n', 'i');
    case SBScriptMARC:
        return TAG('M', 'a', 'r', 'c');
    case SBScriptMEDF:
        return TAG('M', 'e', 'd', 'f');
    case SBScriptMEND:
        return TAG('M', 'e', 'n', 'd');
    case SBScriptMERC:
        return TAG('M', 'e', 'r', 'c');
    case SBScriptMERO:
        return TAG('M', 'e', 'r', 'o');
    case SBScriptMLYM:
        return TAG('M', 'l', 'y', 'm');
    case SBScriptMODI:
        return TAG('M', 'o', 'd', 'i');
    case SBScriptMONG:
        return TAG('M', 'o', 'n', 'g');
    case SBScriptMROO:
        return TAG('M', 'r', 'o', 'o');
    case SBScriptMTEI:
        return TAG('M', 't', 'e', 'i');
    case SBScriptMULT:
        return TAG('M', 'u', 'l', 't');
    case SBScriptMYMR:
        return TAG('M', 'y', 'm', 'r');
    case SBScriptNAGM:
        return TAG('N', 'a', 'g', 'm');
    case SBScriptNAND:
        return TAG('N', 'a', 'n', 'd');
    case SBScriptNARB:
        return TAG('N', 'a', 'r', 'b');
    case SBScriptNBAT:
        return TAG('N', 'b', 'a', 't');
    case SBScriptNEWA:
        return TAG('N', 'e', 'w', 'a');
    case SBScriptNKOO:
        return TAG('N', 'k', 'o', 'o');
    case SBScriptNSHU:
        return TAG('N', 's', 'h', 'u');
    case SBScriptOGAM:
        return TAG('O', 'g', 'a', 'm');
    case SBScriptOLCK:
        return TAG('O', 'l', 'c', 'k');
    case SBScriptONAO:
        return TAG('O', 'n', 'a', 'o');
    case SBScriptORKH:
        return TAG('O', 'r', 'k', 'h');
    case SBScriptORYA:
        return TAG('O', 'r', 'y', 'a');
    case SBScriptOSGE:
        return TAG('O', 's', 'g', 'e');
    case SBScriptOSMA:
        return TAG('O', 's', 'm', 'a');
    case SBScriptOUGR:
        return TAG('O', 'u', 'g', 'r');
    case SBScriptPALM:
        return TAG('P', 'a', 'l', 'm');
    case SBScriptPAUC:
        return TAG('P', 'a', 'u', 'c');
    case SBScriptPERM:
        return TAG('P', 'e', 'r', 'm');
    case SBScriptPHAG:
        return TAG('P', 'h', 'a', 'g');
    case SBScriptPHLI:
        return TAG('P', 'h', 'l', 'i');
    case SBScriptPHLP:
        return TAG('P', 'h', 'l', 'p');
    case SBScriptPHNX:
        return TAG('P', 'h', 'n', 'x');
    case SBScriptPLRD:
        return TAG('P', 'l', 'r', 'd');
    case SBScriptPRTI:
        return TAG('P', 'r', 't', 'i');
    case SBScriptRJNG:
        return TAG('R', 'j', 'n', 'g');
    case SBScriptROHG:
        return TAG('R', 'o', 'h', 'g');
    case SBScriptRUNR:
        return TAG('R', 'u', 'n', 'r');
    case SBScriptSAMR:
        return TAG('S', 'a', 'm', 'r');
    case SBScriptSARB:
        return TAG('S', 'a', 'r', 'b');
    case SBScriptSAUR:
        return TAG('S', 'a', 'u', 'r');
    case SBScriptSGNW:
        return TAG('S', 'g', 'n', 'w');
    case SBScriptSHAW:
        return TAG('S', 'h', 'a', 'w');
    case SBScriptSHRD:
        return TAG('S', 'h', 'r', 'd');
    case SBScriptSIDD:
        return TAG('S', 'i', 'd', 'd');
    case SBScriptSIND:
        return TAG('S', 'i', 'n', 'd');
    case SBScriptSINH:
        return TAG('S', 'i', 'n', 'h');
    case SBScriptSOGD:
        return TAG('S', 'o', 'g', 'd');
    case SBScriptSOGO:
        return TAG('S', 'o', 'g', 'o');
    case SBScriptSORA:
        return TAG('S', 'o', 'r', 'a');
    case SBScriptSOYO:
        return TAG('S', 'o', 'y', 'o');
    case SBScriptSUND:
        return TAG('S', 'u', 'n', 'd');
    case SBScriptSUNU:
        return TAG('S', 'u', 'n', 'u');
    case SBScriptSYLO:
        return TAG('S', 'y', 'l', 'o');
    case SBScriptSYRC:
        return TAG('S', 'y', 'r', 'c');
    case SBScriptTAGB:
        return TAG('T', 'a', 'g', 'b');
    case SBScriptTAKR:
        return TAG('T', 'a', 'k', 'r');
    case SBScriptTALE:
        return TAG('T', 'a', 'l', 'e');
    case SBScriptTALU:
        return TAG('T', 'a', 'l', 'u');
    case SBScriptTAML:
        return TAG('T', 'a', 'm', 'l');
    case SBScriptTANG:
        return TAG('T', 'a', 'n', 'g');
    case SBScriptTAVT:
        return TAG('T', 'a', 'v', 't');
    case SBScriptTELU:
        return TAG('T', 'e', 'l', 'u');
    case SBScriptTFNG:
        return TAG('T', 'f', 'n', 'g');
    case SBScriptTGLG:
        return TAG('T', 'g', 'l', 'g');
    case SBScriptTHAA:
        return TAG('T', 'h', 'a', 'a');
    case SBScriptTHAI:
        return TAG('T', 'h', 'a', 'i');
    case SBScriptTIBT:
        return TAG('T', 'i', 'b', 't');
    case SBScriptTIRH:
        return TAG('T', 'i', 'r', 'h');
    case SBScriptTNSA:
        return TAG('T', 'n', 's', 'a');
    case SBScriptTODR:
        return TAG('T', 'o', 'd', 'r');
    case SBScriptTOTO:
        return TAG('T', 'o', 't', 'o');
    case SBScriptTUTG:
        return TAG('T', 'u', 't', 'g');
    case SBScriptUGAR:
        return TAG('U', 'g', 'a', 'r');
    case SBScriptVAII:
        return TAG('V', 'a', 'i', 'i');
    case SBScriptVITH:
        return TAG('V', 'i', 't', 'h');
    case SBScriptWARA:
        return TAG('W', 'a', 'r', 'a');
    case SBScriptWCHO:
        return TAG('W', 'c', 'h', 'o');
    case SBScriptXPEO:
        return TAG('X', 'p', 'e', 'o');
    case SBScriptXSUX:
        return TAG('X', 's', 'u', 'x');
    case SBScriptYEZI:
        return TAG('Y', 'e', 'z', 'i');
    case SBScriptYIII:
        return TAG('Y', 'i', 'i', 'i');
    case SBScriptZANB:
        return TAG('Z', 'a', 'n', 'b');
    case SBScriptZINH:
        return TAG('Z', 'i', 'n', 'h');
    case SBScriptZYYY:
        return TAG('Z', 'y', 'y', 'y');
    case SBScriptZZZZ:
        return TAG('Z', 'z', 'z', 'z');
    default:
        return 0;
    }
}

const char *SBVersionGetString(void)
{
    return LibraryVersion;
}
