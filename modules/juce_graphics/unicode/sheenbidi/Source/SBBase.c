/*
 * Copyright (C) 2016-2019 Muhammad Tayyab Akram
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

#include <juce_graphics/unicode/sheenbidi/Headers/SBConfig.h>

#include "BidiTypeLookup.h"
#include "GeneralCategoryLookup.h"
#include "PairingLookup.h"
#include "ScriptLookup.h"
#include "SBBase.h"

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

SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint)
{
    return LookupBidiType(codepoint);
}

SBGeneralCategory SBCodepointGetGeneralCategory(SBCodepoint codepoint)
{
    return LookupGeneralCategory(codepoint);
}

SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint)
{
    return LookupMirror(codepoint);
}

SBScript SBCodepointGetScript(SBCodepoint codepoint)
{
    return LookupScript(codepoint);
}

SBUInt32 SBScriptGetOpenTypeTag(SBScript script)
{
    /* Reference: https://docs.microsoft.com/en-us/typography/opentype/spec/scripttags */
    /* Dated: 07/24/2017 */

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
 /* case SBScriptBYZM:
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
    case SBScriptHANI:
        return TAG('h', 'a', 'n', 'i');
    case SBScriptCOPT:
        return TAG('c', 'o', 'p', 't');
    case SBScriptCPRT:
        return TAG('c', 'p', 'r', 't');
    case SBScriptCYRL:
        return TAG('c', 'y', 'r', 'l');
 /* case SBScriptDFLT:
        return TAG('D', 'F', 'L', 'T'); */
    case SBScriptDSRT:
        return TAG('d', 's', 'r', 't');
 /* case SBScriptDEVA:
        return TAG('d', 'e', 'v', 'a'); */
    case SBScriptDEVA:
        return TAG('d', 'e', 'v', '2');
    case SBScriptDUPL:
        return TAG('d', 'u', 'p', 'l');
    case SBScriptEGYP:
        return TAG('e', 'g', 'y', 'p');
    case SBScriptELBA:
        return TAG('e', 'l', 'b', 'a');
    case SBScriptETHI:
        return TAG('e', 't', 'h', 'i');
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
 /* case SBScriptGURU:
        return TAG('g', 'u', 'r', 'u'); */
    case SBScriptGURU:
        return TAG('g', 'u', 'r', '2');
    case SBScriptHANG:
        return TAG('h', 'a', 'n', 'g');
 /* case SBScriptJAMO:
        return TAG('j', 'a', 'm', 'o'); */
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
    case SBScriptKALI:
        return TAG('k', 'a', 'l', 'i');
    case SBScriptKHAR:
        return TAG('k', 'h', 'a', 'r');
    case SBScriptKHMR:
        return TAG('k', 'h', 'm', 'r');
    case SBScriptKHOJ:
        return TAG('k', 'h', 'o', 'j');
    case SBScriptSIND:
        return TAG('s', 'i', 'n', 'd');
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
 /* case SBScriptMATH:
        return TAG('m', 'a', 't', 'h'); */
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
 /* case SBScriptMUSC:
        return TAG('m', 'u', 's', 'c'); */
 /* case SBScriptMYMR:
        return TAG('m', 'y', 'm', 'r'); */
    case SBScriptMYMR:
        return TAG('m', 'y', 'm', '2');
    case SBScriptNBAT:
        return TAG('n', 'b', 'a', 't');
    case SBScriptNEWA:
        return TAG('n', 'e', 'w', 'a');
    case SBScriptTALU:
        return TAG('t', 'a', 'l', 'u');
    case SBScriptNKOO:
        return TAG('n', 'k', 'o', ' ');
 /* case SBScriptORYA:
        return TAG('o', 'r', 'y', 'a'); */
    case SBScriptORYA:
        return TAG('o', 'r', 'y', '2');
    case SBScriptOGAM:
        return TAG('o', 'g', 'a', 'm');
    case SBScriptOLCK:
        return TAG('o', 'l', 'c', 'k');
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
    case SBScriptSARB:
        return TAG('s', 'a', 'r', 'b');
    case SBScriptORKH:
        return TAG('o', 'r', 'k', 'h');
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
    case SBScriptSORA:
        return TAG('s', 'o', 'r', 'a');
    case SBScriptXSUX:
        return TAG('x', 's', 'u', 'x');
    case SBScriptSUND:
        return TAG('s', 'u', 'n', 'd');
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
    case SBScriptUGAR:
        return TAG('u', 'g', 'a', 'r');
    case SBScriptVAII:
        return TAG('v', 'a', 'i', ' ');
    case SBScriptWARA:
        return TAG('w', 'a', 'r', 'a');
    case SBScriptYIII:
        return TAG('y', 'i', ' ', ' ');
    default:
        return TAG('D', 'F', 'L', 'T');
    }
}
