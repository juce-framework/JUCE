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

/** This map was created using the data provided by IBM:
    https://www.ibm.com/docs/en/i/7.2?topic=tables-unicode-lowercase-uppercase-conversion-mapping-table

    There isn't a straightforward function to do this programmatically.

    The 'first' (left column) is comprised of the lowercase versions of characters, and the other is uppercase.
*/
const std::unordered_map<juce_wchar, juce_wchar> caseConversionMap =
{
    { 0x0061, 0x0041 },
    { 0x0062, 0x0042 },
    { 0x0063, 0x0043 },
    { 0x0064, 0x0044 },
    { 0x0065, 0x0045 },
    { 0x0066, 0x0046 },
    { 0x0067, 0x0047 },
    { 0x0068, 0x0048 },
    { 0x0069, 0x0049 },
    { 0x006a, 0x004a },
    { 0x006b, 0x004b },
    { 0x006c, 0x004c },
    { 0x006d, 0x004d },
    { 0x006e, 0x004e },
    { 0x006f, 0x004f },
    { 0x0070, 0x0050 },
    { 0x0071, 0x0051 },
    { 0x0072, 0x0052 },
    { 0x0073, 0x0053 },
    { 0x0074, 0x0054 },
    { 0x0075, 0x0055 },
    { 0x0076, 0x0056 },
    { 0x0077, 0x0057 },
    { 0x0078, 0x0058 },
    { 0x0079, 0x0059 },
    { 0x007a, 0x005a },
    { 0x00e0, 0x00c0 },
    { 0x00e1, 0x00c1 },
    { 0x00e2, 0x00c2 },
    { 0x00e3, 0x00c3 },
    { 0x00e4, 0x00c4 },
    { 0x00e5, 0x00c5 },
    { 0x00e6, 0x00c6 },
    { 0x00e7, 0x00c7 },
    { 0x00e8, 0x00c8 },
    { 0x00e9, 0x00c9 },
    { 0x00ea, 0x00ca },
    { 0x00eb, 0x00cb },
    { 0x00ec, 0x00cc },
    { 0x00ed, 0x00cd },
    { 0x00ee, 0x00ce },
    { 0x00ef, 0x00cf },
    { 0x00f0, 0x00d0 },
    { 0x00f1, 0x00d1 },
    { 0x00f2, 0x00d2 },
    { 0x00f3, 0x00d3 },
    { 0x00f4, 0x00d4 },
    { 0x00f5, 0x00d5 },
    { 0x00f6, 0x00d6 },
    { 0x00f8, 0x00d8 },
    { 0x00f9, 0x00d9 },
    { 0x00fa, 0x00da },
    { 0x00fb, 0x00db },
    { 0x00fc, 0x00dc },
    { 0x00fd, 0x00dd },
    { 0x00fe, 0x00de },
    { 0x00ff, 0x0178 },
    { 0x0101, 0x0100 },
    { 0x0103, 0x0102 },
    { 0x0105, 0x0104 },
    { 0x0107, 0x0106 },
    { 0x0109, 0x0108 },
    { 0x010b, 0x010a },
    { 0x010d, 0x010c },
    { 0x010f, 0x010e },
    { 0x0111, 0x0110 },
    { 0x0113, 0x0112 },
    { 0x0115, 0x0114 },
    { 0x0117, 0x0116 },
    { 0x0119, 0x0118 },
    { 0x011b, 0x011a },
    { 0x011d, 0x011c },
    { 0x011f, 0x011e },
    { 0x0121, 0x0120 },
    { 0x0123, 0x0122 },
    { 0x0125, 0x0124 },
    { 0x0127, 0x0126 },
    { 0x0129, 0x0128 },
    { 0x012b, 0x012a },
    { 0x012d, 0x012c },
    { 0x012f, 0x012e },
    { 0x0131, 0x0049 },
    { 0x0133, 0x0132 },
    { 0x0135, 0x0134 },
    { 0x0137, 0x0136 },
    { 0x013a, 0x0139 },
    { 0x013c, 0x013b },
    { 0x013e, 0x013d },
    { 0x0140, 0x013f },
    { 0x0142, 0x0141 },
    { 0x0144, 0x0143 },
    { 0x0146, 0x0145 },
    { 0x0148, 0x0147 },
    { 0x014b, 0x014a },
    { 0x014d, 0x014c },
    { 0x014f, 0x014e },
    { 0x0151, 0x0150 },
    { 0x0153, 0x0152 },
    { 0x0155, 0x0154 },
    { 0x0157, 0x0156 },
    { 0x0159, 0x0158 },
    { 0x015b, 0x015a },
    { 0x015d, 0x015c },
    { 0x015f, 0x015e },
    { 0x0161, 0x0160 },
    { 0x0163, 0x0162 },
    { 0x0165, 0x0164 },
    { 0x0167, 0x0166 },
    { 0x0169, 0x0168 },
    { 0x016b, 0x016a },
    { 0x016d, 0x016c },
    { 0x016f, 0x016e },
    { 0x0171, 0x0170 },
    { 0x0173, 0x0172 },
    { 0x0175, 0x0174 },
    { 0x0177, 0x0176 },
    { 0x017a, 0x0179 },
    { 0x017c, 0x017b },
    { 0x017e, 0x017d },
    { 0x0183, 0x0182 },
    { 0x0185, 0x0184 },
    { 0x0188, 0x0187 },
    { 0x018c, 0x018b },
    { 0x0192, 0x0191 },
    { 0x0199, 0x0198 },
    { 0x01a1, 0x01a0 },
    { 0x01a3, 0x01a2 },
    { 0x01a5, 0x01a4 },
    { 0x01a8, 0x01a7 },
    { 0x01ad, 0x01ac },
    { 0x01b0, 0x01af },
    { 0x01b4, 0x01b3 },
    { 0x01b6, 0x01b5 },
    { 0x01b9, 0x01b8 },
    { 0x01bd, 0x01bc },
    { 0x01c6, 0x01c4 },
    { 0x01c9, 0x01c7 },
    { 0x01cc, 0x01ca },
    { 0x01ce, 0x01cd },
    { 0x01d0, 0x01cf },
    { 0x01d2, 0x01d1 },
    { 0x01d4, 0x01d3 },
    { 0x01d6, 0x01d5 },
    { 0x01d8, 0x01d7 },
    { 0x01da, 0x01d9 },
    { 0x01dc, 0x01db },
    { 0x01df, 0x01de },
    { 0x01e1, 0x01e0 },
    { 0x01e3, 0x01e2 },
    { 0x01e5, 0x01e4 },
    { 0x01e7, 0x01e6 },
    { 0x01e9, 0x01e8 },
    { 0x01eb, 0x01ea },
    { 0x01ed, 0x01ec },
    { 0x01ef, 0x01ee },
    { 0x01f3, 0x01f1 },
    { 0x01f5, 0x01f4 },
    { 0x01fb, 0x01fa },
    { 0x01fd, 0x01fc },
    { 0x01ff, 0x01fe },
    { 0x0201, 0x0200 },
    { 0x0203, 0x0202 },
    { 0x0205, 0x0204 },
    { 0x0207, 0x0206 },
    { 0x0209, 0x0208 },
    { 0x020b, 0x020a },
    { 0x020d, 0x020c },
    { 0x020f, 0x020e },
    { 0x0211, 0x0210 },
    { 0x0213, 0x0212 },
    { 0x0215, 0x0214 },
    { 0x0217, 0x0216 },
    { 0x0253, 0x0181 },
    { 0x0254, 0x0186 },
    { 0x0257, 0x018a },
    { 0x0258, 0x018e },
    { 0x0259, 0x018f },
    { 0x025b, 0x0190 },
    { 0x0260, 0x0193 },
    { 0x0263, 0x0194 },
    { 0x0268, 0x0197 },
    { 0x0269, 0x0196 },
    { 0x026f, 0x019c },
    { 0x0272, 0x019d },
    { 0x0275, 0x019f },
    { 0x0283, 0x01a9 },
    { 0x0288, 0x01ae },
    { 0x028a, 0x01b1 },
    { 0x028b, 0x01b2 },
    { 0x0292, 0x01b7 },
    { 0x03ac, 0x0386 },
    { 0x03ad, 0x0388 },
    { 0x03ae, 0x0389 },
    { 0x03af, 0x038a },
    { 0x03b1, 0x0391 },
    { 0x03b2, 0x0392 },
    { 0x03b3, 0x0393 },
    { 0x03b4, 0x0394 },
    { 0x03b5, 0x0395 },
    { 0x03b6, 0x0396 },
    { 0x03b7, 0x0397 },
    { 0x03b8, 0x0398 },
    { 0x03b9, 0x0399 },
    { 0x03ba, 0x039a },
    { 0x03bb, 0x039b },
    { 0x03bc, 0x039c },
    { 0x03bd, 0x039d },
    { 0x03be, 0x039e },
    { 0x03bf, 0x039f },
    { 0x03c0, 0x03a0 },
    { 0x03c1, 0x03a1 },
    { 0x03c3, 0x03a3 },
    { 0x03c4, 0x03a4 },
    { 0x03c5, 0x03a5 },
    { 0x03c6, 0x03a6 },
    { 0x03c7, 0x03a7 },
    { 0x03c8, 0x03a8 },
    { 0x03c9, 0x03a9 },
    { 0x03ca, 0x03aa },
    { 0x03cb, 0x03ab },
    { 0x03cc, 0x038c },
    { 0x03cd, 0x038e },
    { 0x03ce, 0x038f },
    { 0x03e3, 0x03e2 },
    { 0x03e5, 0x03e4 },
    { 0x03e7, 0x03e6 },
    { 0x03e9, 0x03e8 },
    { 0x03eb, 0x03ea },
    { 0x03ed, 0x03ec },
    { 0x03ef, 0x03ee },
    { 0x0430, 0x0410 },
    { 0x0431, 0x0411 },
    { 0x0432, 0x0412 },
    { 0x0433, 0x0413 },
    { 0x0434, 0x0414 },
    { 0x0435, 0x0415 },
    { 0x0436, 0x0416 },
    { 0x0437, 0x0417 },
    { 0x0438, 0x0418 },
    { 0x0439, 0x0419 },
    { 0x043a, 0x041a },
    { 0x043b, 0x041b },
    { 0x043c, 0x041c },
    { 0x043d, 0x041d },
    { 0x043e, 0x041e },
    { 0x043f, 0x041f },
    { 0x0440, 0x0420 },
    { 0x0441, 0x0421 },
    { 0x0442, 0x0422 },
    { 0x0443, 0x0423 },
    { 0x0444, 0x0424 },
    { 0x0445, 0x0425 },
    { 0x0446, 0x0426 },
    { 0x0447, 0x0427 },
    { 0x0448, 0x0428 },
    { 0x0449, 0x0429 },
    { 0x044a, 0x042a },
    { 0x044b, 0x042b },
    { 0x044c, 0x042c },
    { 0x044d, 0x042d },
    { 0x044e, 0x042e },
    { 0x044f, 0x042f },
    { 0x0451, 0x0401 },
    { 0x0452, 0x0402 },
    { 0x0453, 0x0403 },
    { 0x0454, 0x0404 },
    { 0x0455, 0x0405 },
    { 0x0456, 0x0406 },
    { 0x0457, 0x0407 },
    { 0x0458, 0x0408 },
    { 0x0459, 0x0409 },
    { 0x045a, 0x040a },
    { 0x045b, 0x040b },
    { 0x045c, 0x040c },
    { 0x045e, 0x040e },
    { 0x045f, 0x040f },
    { 0x0461, 0x0460 },
    { 0x0463, 0x0462 },
    { 0x0465, 0x0464 },
    { 0x0467, 0x0466 },
    { 0x0469, 0x0468 },
    { 0x046b, 0x046a },
    { 0x046d, 0x046c },
    { 0x046f, 0x046e },
    { 0x0471, 0x0470 },
    { 0x0473, 0x0472 },
    { 0x0475, 0x0474 },
    { 0x0477, 0x0476 },
    { 0x0479, 0x0478 },
    { 0x047b, 0x047a },
    { 0x047d, 0x047c },
    { 0x047f, 0x047e },
    { 0x0481, 0x0480 },
    { 0x0491, 0x0490 },
    { 0x0493, 0x0492 },
    { 0x0495, 0x0494 },
    { 0x0497, 0x0496 },
    { 0x0499, 0x0498 },
    { 0x049b, 0x049a },
    { 0x049d, 0x049c },
    { 0x049f, 0x049e },
    { 0x04a1, 0x04a0 },
    { 0x04a3, 0x04a2 },
    { 0x04a5, 0x04a4 },
    { 0x04a7, 0x04a6 },
    { 0x04a9, 0x04a8 },
    { 0x04ab, 0x04aa },
    { 0x04ad, 0x04ac },
    { 0x04af, 0x04ae },
    { 0x04b1, 0x04b0 },
    { 0x04b3, 0x04b2 },
    { 0x04b5, 0x04b4 },
    { 0x04b7, 0x04b6 },
    { 0x04b9, 0x04b8 },
    { 0x04bb, 0x04ba },
    { 0x04bd, 0x04bc },
    { 0x04bf, 0x04be },
    { 0x04c2, 0x04c1 },
    { 0x04c4, 0x04c3 },
    { 0x04c8, 0x04c7 },
    { 0x04cc, 0x04cb },
    { 0x04d1, 0x04d0 },
    { 0x04d3, 0x04d2 },
    { 0x04d5, 0x04d4 },
    { 0x04d7, 0x04d6 },
    { 0x04d9, 0x04d8 },
    { 0x04db, 0x04da },
    { 0x04dd, 0x04dc },
    { 0x04df, 0x04de },
    { 0x04e1, 0x04e0 },
    { 0x04e3, 0x04e2 },
    { 0x04e5, 0x04e4 },
    { 0x04e7, 0x04e6 },
    { 0x04e9, 0x04e8 },
    { 0x04eb, 0x04ea },
    { 0x04ef, 0x04ee },
    { 0x04f1, 0x04f0 },
    { 0x04f3, 0x04f2 },
    { 0x04f5, 0x04f4 },
    { 0x04f9, 0x04f8 },
    { 0x0561, 0x0531 },
    { 0x0562, 0x0532 },
    { 0x0563, 0x0533 },
    { 0x0564, 0x0534 },
    { 0x0565, 0x0535 },
    { 0x0566, 0x0536 },
    { 0x0567, 0x0537 },
    { 0x0568, 0x0538 },
    { 0x0569, 0x0539 },
    { 0x056a, 0x053a },
    { 0x056b, 0x053b },
    { 0x056c, 0x053c },
    { 0x056d, 0x053d },
    { 0x056e, 0x053e },
    { 0x056f, 0x053f },
    { 0x0570, 0x0540 },
    { 0x0571, 0x0541 },
    { 0x0572, 0x0542 },
    { 0x0573, 0x0543 },
    { 0x0574, 0x0544 },
    { 0x0575, 0x0545 },
    { 0x0576, 0x0546 },
    { 0x0577, 0x0547 },
    { 0x0578, 0x0548 },
    { 0x0579, 0x0549 },
    { 0x057a, 0x054a },
    { 0x057b, 0x054b },
    { 0x057c, 0x054c },
    { 0x057d, 0x054d },
    { 0x057e, 0x054e },
    { 0x057f, 0x054f },
    { 0x0580, 0x0550 },
    { 0x0581, 0x0551 },
    { 0x0582, 0x0552 },
    { 0x0583, 0x0553 },
    { 0x0584, 0x0554 },
    { 0x0585, 0x0555 },
    { 0x0586, 0x0556 },
    { 0x10d0, 0x10a0 },
    { 0x10d1, 0x10a1 },
    { 0x10d2, 0x10a2 },
    { 0x10d3, 0x10a3 },
    { 0x10d4, 0x10a4 },
    { 0x10d5, 0x10a5 },
    { 0x10d6, 0x10a6 },
    { 0x10d7, 0x10a7 },
    { 0x10d8, 0x10a8 },
    { 0x10d9, 0x10a9 },
    { 0x10da, 0x10aa },
    { 0x10db, 0x10ab },
    { 0x10dc, 0x10ac },
    { 0x10dd, 0x10ad },
    { 0x10de, 0x10ae },
    { 0x10df, 0x10af },
    { 0x10e0, 0x10b0 },
    { 0x10e1, 0x10b1 },
    { 0x10e2, 0x10b2 },
    { 0x10e3, 0x10b3 },
    { 0x10e4, 0x10b4 },
    { 0x10e5, 0x10b5 },
    { 0x10e6, 0x10b6 },
    { 0x10e7, 0x10b7 },
    { 0x10e8, 0x10b8 },
    { 0x10e9, 0x10b9 },
    { 0x10ea, 0x10ba },
    { 0x10eb, 0x10bb },
    { 0x10ec, 0x10bc },
    { 0x10ed, 0x10bd },
    { 0x10ee, 0x10be },
    { 0x10ef, 0x10bf },
    { 0x10f0, 0x10c0 },
    { 0x10f1, 0x10c1 },
    { 0x10f2, 0x10c2 },
    { 0x10f3, 0x10c3 },
    { 0x10f4, 0x10c4 },
    { 0x10f5, 0x10c5 },
    { 0x1e01, 0x1e00 },
    { 0x1e03, 0x1e02 },
    { 0x1e05, 0x1e04 },
    { 0x1e07, 0x1e06 },
    { 0x1e09, 0x1e08 },
    { 0x1e0b, 0x1e0a },
    { 0x1e0d, 0x1e0c },
    { 0x1e0f, 0x1e0e },
    { 0x1e11, 0x1e10 },
    { 0x1e13, 0x1e12 },
    { 0x1e15, 0x1e14 },
    { 0x1e17, 0x1e16 },
    { 0x1e19, 0x1e18 },
    { 0x1e1b, 0x1e1a },
    { 0x1e1d, 0x1e1c },
    { 0x1e1f, 0x1e1e },
    { 0x1e21, 0x1e20 },
    { 0x1e23, 0x1e22 },
    { 0x1e25, 0x1e24 },
    { 0x1e27, 0x1e26 },
    { 0x1e29, 0x1e28 },
    { 0x1e2b, 0x1e2a },
    { 0x1e2d, 0x1e2c },
    { 0x1e2f, 0x1e2e },
    { 0x1e31, 0x1e30 },
    { 0x1e33, 0x1e32 },
    { 0x1e35, 0x1e34 },
    { 0x1e37, 0x1e36 },
    { 0x1e39, 0x1e38 },
    { 0x1e3b, 0x1e3a },
    { 0x1e3d, 0x1e3c },
    { 0x1e3f, 0x1e3e },
    { 0x1e41, 0x1e40 },
    { 0x1e43, 0x1e42 },
    { 0x1e45, 0x1e44 },
    { 0x1e47, 0x1e46 },
    { 0x1e49, 0x1e48 },
    { 0x1e4b, 0x1e4a },
    { 0x1e4d, 0x1e4c },
    { 0x1e4f, 0x1e4e },
    { 0x1e51, 0x1e50 },
    { 0x1e53, 0x1e52 },
    { 0x1e55, 0x1e54 },
    { 0x1e57, 0x1e56 },
    { 0x1e59, 0x1e58 },
    { 0x1e5b, 0x1e5a },
    { 0x1e5d, 0x1e5c },
    { 0x1e5f, 0x1e5e },
    { 0x1e61, 0x1e60 },
    { 0x1e63, 0x1e62 },
    { 0x1e65, 0x1e64 },
    { 0x1e67, 0x1e66 },
    { 0x1e69, 0x1e68 },
    { 0x1e6b, 0x1e6a },
    { 0x1e6d, 0x1e6c },
    { 0x1e6f, 0x1e6e },
    { 0x1e71, 0x1e70 },
    { 0x1e73, 0x1e72 },
    { 0x1e75, 0x1e74 },
    { 0x1e77, 0x1e76 },
    { 0x1e79, 0x1e78 },
    { 0x1e7b, 0x1e7a },
    { 0x1e7d, 0x1e7c },
    { 0x1e7f, 0x1e7e },
    { 0x1e81, 0x1e80 },
    { 0x1e83, 0x1e82 },
    { 0x1e85, 0x1e84 },
    { 0x1e87, 0x1e86 },
    { 0x1e89, 0x1e88 },
    { 0x1e8b, 0x1e8a },
    { 0x1e8d, 0x1e8c },
    { 0x1e8f, 0x1e8e },
    { 0x1e91, 0x1e90 },
    { 0x1e93, 0x1e92 },
    { 0x1e95, 0x1e94 },
    { 0x1ea1, 0x1ea0 },
    { 0x1ea3, 0x1ea2 },
    { 0x1ea5, 0x1ea4 },
    { 0x1ea7, 0x1ea6 },
    { 0x1ea9, 0x1ea8 },
    { 0x1eab, 0x1eaa },
    { 0x1ead, 0x1eac },
    { 0x1eaf, 0x1eae },
    { 0x1eb1, 0x1eb0 },
    { 0x1eb3, 0x1eb2 },
    { 0x1eb5, 0x1eb4 },
    { 0x1eb7, 0x1eb6 },
    { 0x1eb9, 0x1eb8 },
    { 0x1ebb, 0x1eba },
    { 0x1ebd, 0x1ebc },
    { 0x1ebf, 0x1ebe },
    { 0x1ec1, 0x1ec0 },
    { 0x1ec3, 0x1ec2 },
    { 0x1ec5, 0x1ec4 },
    { 0x1ec7, 0x1ec6 },
    { 0x1ec9, 0x1ec8 },
    { 0x1ecb, 0x1eca },
    { 0x1ecd, 0x1ecc },
    { 0x1ecf, 0x1ece },
    { 0x1ed1, 0x1ed0 },
    { 0x1ed3, 0x1ed2 },
    { 0x1ed5, 0x1ed4 },
    { 0x1ed7, 0x1ed6 },
    { 0x1ed9, 0x1ed8 },
    { 0x1edb, 0x1eda },
    { 0x1edd, 0x1edc },
    { 0x1edf, 0x1ede },
    { 0x1ee1, 0x1ee0 },
    { 0x1ee3, 0x1ee2 },
    { 0x1ee5, 0x1ee4 },
    { 0x1ee7, 0x1ee6 },
    { 0x1ee9, 0x1ee8 },
    { 0x1eeb, 0x1eea },
    { 0x1eed, 0x1eec },
    { 0x1eef, 0x1eee },
    { 0x1ef1, 0x1ef0 },
    { 0x1ef3, 0x1ef2 },
    { 0x1ef5, 0x1ef4 },
    { 0x1ef7, 0x1ef6 },
    { 0x1ef9, 0x1ef8 },
    { 0x1f00, 0x1f08 },
    { 0x1f01, 0x1f09 },
    { 0x1f02, 0x1f0a },
    { 0x1f03, 0x1f0b },
    { 0x1f04, 0x1f0c },
    { 0x1f05, 0x1f0d },
    { 0x1f06, 0x1f0e },
    { 0x1f07, 0x1f0f },
    { 0x1f10, 0x1f18 },
    { 0x1f11, 0x1f19 },
    { 0x1f12, 0x1f1a },
    { 0x1f13, 0x1f1b },
    { 0x1f14, 0x1f1c },
    { 0x1f15, 0x1f1d },
    { 0x1f20, 0x1f28 },
    { 0x1f21, 0x1f29 },
    { 0x1f22, 0x1f2a },
    { 0x1f23, 0x1f2b },
    { 0x1f24, 0x1f2c },
    { 0x1f25, 0x1f2d },
    { 0x1f26, 0x1f2e },
    { 0x1f27, 0x1f2f },
    { 0x1f30, 0x1f38 },
    { 0x1f31, 0x1f39 },
    { 0x1f32, 0x1f3a },
    { 0x1f33, 0x1f3b },
    { 0x1f34, 0x1f3c },
    { 0x1f35, 0x1f3d },
    { 0x1f36, 0x1f3e },
    { 0x1f37, 0x1f3f },
    { 0x1f40, 0x1f48 },
    { 0x1f41, 0x1f49 },
    { 0x1f42, 0x1f4a },
    { 0x1f43, 0x1f4b },
    { 0x1f44, 0x1f4c },
    { 0x1f45, 0x1f4d },
    { 0x1f51, 0x1f59 },
    { 0x1f53, 0x1f5b },
    { 0x1f55, 0x1f5d },
    { 0x1f57, 0x1f5f },
    { 0x1f60, 0x1f68 },
    { 0x1f61, 0x1f69 },
    { 0x1f62, 0x1f6a },
    { 0x1f63, 0x1f6b },
    { 0x1f64, 0x1f6c },
    { 0x1f65, 0x1f6d },
    { 0x1f66, 0x1f6e },
    { 0x1f67, 0x1f6f },
    { 0x1f80, 0x1f88 },
    { 0x1f81, 0x1f89 },
    { 0x1f82, 0x1f8a },
    { 0x1f83, 0x1f8b },
    { 0x1f84, 0x1f8c },
    { 0x1f85, 0x1f8d },
    { 0x1f86, 0x1f8e },
    { 0x1f87, 0x1f8f },
    { 0x1f90, 0x1f98 },
    { 0x1f91, 0x1f99 },
    { 0x1f92, 0x1f9a },
    { 0x1f93, 0x1f9b },
    { 0x1f94, 0x1f9c },
    { 0x1f95, 0x1f9d },
    { 0x1f96, 0x1f9e },
    { 0x1f97, 0x1f9f },
    { 0x1fa0, 0x1fa8 },
    { 0x1fa1, 0x1fa9 },
    { 0x1fa2, 0x1faa },
    { 0x1fa3, 0x1fab },
    { 0x1fa4, 0x1fac },
    { 0x1fa5, 0x1fad },
    { 0x1fa6, 0x1fae },
    { 0x1fa7, 0x1faf },
    { 0x1fb0, 0x1fb8 },
    { 0x1fb1, 0x1fb9 },
    { 0x1fd0, 0x1fd8 },
    { 0x1fd1, 0x1fd9 },
    { 0x1fe0, 0x1fe8 },
    { 0x1fe1, 0x1fe9 },
    { 0x24d0, 0x24b6 },
    { 0x24d1, 0x24b7 },
    { 0x24d2, 0x24b8 },
    { 0x24d3, 0x24b9 },
    { 0x24d4, 0x24ba },
    { 0x24d5, 0x24bb },
    { 0x24d6, 0x24bc },
    { 0x24d7, 0x24bd },
    { 0x24d8, 0x24be },
    { 0x24d9, 0x24bf },
    { 0x24da, 0x24c0 },
    { 0x24db, 0x24c1 },
    { 0x24dc, 0x24c2 },
    { 0x24dd, 0x24c3 },
    { 0x24de, 0x24c4 },
    { 0x24df, 0x24c5 },
    { 0x24e0, 0x24c6 },
    { 0x24e1, 0x24c7 },
    { 0x24e2, 0x24c8 },
    { 0x24e3, 0x24c9 },
    { 0x24e4, 0x24ca },
    { 0x24e5, 0x24cb },
    { 0x24e6, 0x24cc },
    { 0x24e7, 0x24cd },
    { 0x24e8, 0x24ce },
    { 0x24e9, 0x24cf },
    { 0xff41, 0xff21 },
    { 0xff42, 0xff22 },
    { 0xff43, 0xff23 },
    { 0xff44, 0xff24 },
    { 0xff45, 0xff25 },
    { 0xff46, 0xff26 },
    { 0xff47, 0xff27 },
    { 0xff48, 0xff28 },
    { 0xff49, 0xff29 },
    { 0xff4a, 0xff2a },
    { 0xff4b, 0xff2b },
    { 0xff4c, 0xff2c },
    { 0xff4d, 0xff2d },
    { 0xff4e, 0xff2e },
    { 0xff4f, 0xff2f },
    { 0xff50, 0xff30 },
    { 0xff51, 0xff31 },
    { 0xff52, 0xff32 },
    { 0xff53, 0xff33 },
    { 0xff54, 0xff34 },
    { 0xff55, 0xff35 },
    { 0xff56, 0xff36 },
    { 0xff57, 0xff37 },
    { 0xff58, 0xff38 },
    { 0xff59, 0xff39 },
    { 0xff5a, 0xff3a }
};

juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) noexcept
{
    const auto iter = caseConversionMap.find (character);
    if (iter != std::cend (caseConversionMap))
        return iter->second;

    return character;
}

juce_wchar CharacterFunctions::toLowerCase (const juce_wchar character) noexcept
{
    for (const auto& v : caseConversionMap)
        if (v.second == character)
            return v.first;

    return character;
}

bool CharacterFunctions::isUpperCase (const juce_wchar character) noexcept
{
    return toLowerCase (character) != character;
}

bool CharacterFunctions::isLowerCase (const juce_wchar character) noexcept
{
    return caseConversionMap.find (character) != std::cend (caseConversionMap);
}

//==============================================================================
bool CharacterFunctions::isWhitespace (const char character) noexcept
{
    return character == ' ' || (character <= 13 && character >= 9);
}

bool CharacterFunctions::isWhitespace (const juce_wchar character) noexcept
{
    return iswspace ((wint_t) character) != 0;
}

bool CharacterFunctions::isDigit (const char character) noexcept
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) noexcept
{
    return iswdigit ((wint_t) character) != 0;
}

bool CharacterFunctions::isLetter (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) noexcept
{
    return iswalpha ((wint_t) character) != 0;
}

bool CharacterFunctions::isLetterOrDigit (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) noexcept
{
    return iswalnum ((wint_t) character) != 0;
}

bool CharacterFunctions::isPrintable (const char character) noexcept
{
    return (character >= ' ' && character <= '~');
}

bool CharacterFunctions::isPrintable (const juce_wchar character) noexcept
{
    return iswprint ((wint_t) character) != 0;
}

int CharacterFunctions::getHexDigitValue (const juce_wchar digit) noexcept
{
    auto d = (unsigned int) (digit - '0');

    if (d < (unsigned int) 10)
        return (int) d;

    d += (unsigned int) ('0' - 'a');

    if (d < (unsigned int) 6)
        return (int) d + 10;

    d += (unsigned int) ('a' - 'A');

    if (d < (unsigned int) 6)
        return (int) d + 10;

    return -1;
}

double CharacterFunctions::mulexp10 (const double value, int exponent) noexcept
{
    if (exponent == 0)
        return value;

    if (exactlyEqual (value, 0.0))
        return 0;

    const bool negative = (exponent < 0);

    if (negative)
        exponent = -exponent;

    double result = 1.0, power = 10.0;

    for (int bit = 1; exponent != 0; bit <<= 1)
    {
        if ((exponent & bit) != 0)
        {
            exponent ^= bit;
            result *= power;

            if (exponent == 0)
                break;
        }

        power *= power;
    }

    return negative ? (value / result) : (value * result);
}

juce_wchar CharacterFunctions::getUnicodeCharFromWindows1252Codepage (const uint8 c) noexcept
{
    if (c < 0x80 || c >= 0xa0)
        return (juce_wchar) c;

    static const uint16 lookup[] = { 0x20AC, 0x0007, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
                                     0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0007, 0x017D, 0x0007,
                                     0x0007, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
                                     0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0007, 0x017E, 0x0178 };

    return (juce_wchar) lookup[c - 0x80];
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

#define QUOTE(x) #x
#define STR(value) QUOTE(value)
#define ASYM_CHARPTR_DOUBLE_PAIR(str, value) std::pair<const char*, double> (STR(str), value)
#define CHARPTR_DOUBLE_PAIR(value) ASYM_CHARPTR_DOUBLE_PAIR(value, value)
#define CHARPTR_DOUBLE_PAIR_COMBOS(value) \
    CHARPTR_DOUBLE_PAIR(value), \
    CHARPTR_DOUBLE_PAIR(-value), \
    ASYM_CHARPTR_DOUBLE_PAIR(+value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(000000 ## value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(+000 ## value, value), \
    ASYM_CHARPTR_DOUBLE_PAIR(-0 ## value, -value)

namespace characterFunctionsTests
{

template <typename CharPointerType>
MemoryBlock memoryBlockFromCharPtr (const typename CharPointerType::CharType* charPtr)
{
    using CharType = typename CharPointerType::CharType;

    MemoryBlock result;
    CharPointerType source (charPtr);

    result.setSize (CharPointerType::getBytesRequiredFor (source) + sizeof (CharType));
    CharPointerType dest { (CharType*) result.getData() };
    dest.writeAll (source);
    return result;
}

template <typename FromCharPointerType, typename ToCharPointerType>
MemoryBlock convert (const MemoryBlock& source, bool removeNullTerminator = false)
{
    using ToCharType   = typename ToCharPointerType  ::CharType;
    using FromCharType = typename FromCharPointerType::CharType;

    FromCharPointerType sourcePtr { (FromCharType*) source.getData() };

    std::vector<juce_wchar> sourceChars;
    size_t requiredSize = 0;
    juce_wchar c;

    while ((c = sourcePtr.getAndAdvance()) != '\0')
    {
        requiredSize += ToCharPointerType::getBytesRequiredFor (c);
        sourceChars.push_back (c);
    }

    if (! removeNullTerminator)
        requiredSize += sizeof (ToCharType);

    MemoryBlock result;
    result.setSize (requiredSize);

    ToCharPointerType dest { (ToCharType*) result.getData() };

    for (auto wc : sourceChars)
        dest.write (wc);

    if (! removeNullTerminator)
        dest.writeNull();

    return result;
}

struct SeparatorStrings
{
    std::vector<MemoryBlock> terminals, nulls;
};

template <typename CharPointerType>
SeparatorStrings getSeparators()
{
    jassertfalse;
    return {};
}

template <>
SeparatorStrings getSeparators<CharPointer_ASCII>()
{
    SeparatorStrings result;

    const CharPointer_ASCII::CharType* terminalCharPtrs[] = {
        "", "-", "+", "e", "e+", "E-", "f", " ", ",", ";", "<", "'", "\"", "_", "k",
        " +", " -", " -e", "-In ", " +n", "n", "  r"
    };

    for (auto ptr : terminalCharPtrs)
        result.terminals.push_back (memoryBlockFromCharPtr<CharPointer_ASCII> (ptr));

    const CharPointer_ASCII::CharType* nullCharPtrs[] = { "." };

    result.nulls = result.terminals;

    for (auto ptr : nullCharPtrs)
        result.nulls.push_back (memoryBlockFromCharPtr<CharPointer_ASCII> (ptr));

    return result;
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF8>()
{
    auto result = getSeparators<CharPointer_ASCII>();

    const CharPointer_UTF8::CharType* terminalCharPtrs[] = {
        "\xe2\x82\xac",                      // €
        "\xf0\x90\x90\xB7",                  // 𐐷
        "\xf0\x9f\x98\x83",                  // 😃
        "\xf0\x9f\x8f\x81\xF0\x9F\x9A\x97"   // 🏁🚗
    };

    for (auto ptr : terminalCharPtrs)
    {
        auto block = memoryBlockFromCharPtr<CharPointer_UTF8> (ptr);

        for (auto vec : { &result.terminals, &result.nulls })
            vec->push_back (block);
    }

    return result;
}

template <typename CharPointerType, typename StorageType>
SeparatorStrings prefixWithAsciiSeparators (const std::vector<std::vector<StorageType>>& terminalCharPtrs)
{
    auto asciiSeparators = getSeparators<CharPointer_ASCII>();

    SeparatorStrings result;

    for (const auto& block : asciiSeparators.terminals)
        result.terminals.push_back (convert<CharPointer_ASCII, CharPointerType> (block));

    for (const auto& block : asciiSeparators.nulls)
        result.nulls.push_back (convert<CharPointer_ASCII, CharPointerType> (block));

    for (auto& t : terminalCharPtrs)
    {
        const auto block = memoryBlockFromCharPtr<CharPointerType> ((typename CharPointerType::CharType*) t.data());

        for (auto vec : { &result.terminals, &result.nulls })
            vec->push_back (block);
    }

    return result;
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF16>()
{
    const std::vector<std::vector<char16_t>> terminalCharPtrs {
        { 0x0                                 },
        { 0x0076, 0x0                         },   // v
        { 0x20ac, 0x0                         },   // €
        { 0xd801, 0xdc37, 0x0                 },   // 𐐷
        { 0x0065, 0xd83d, 0xde03, 0x0         },   // e😃
        { 0xd83c, 0xdfc1, 0xd83d, 0xde97, 0x0 }    // 🏁🚗
    };

    return prefixWithAsciiSeparators<CharPointer_UTF16> (terminalCharPtrs);
}

template <>
SeparatorStrings getSeparators<CharPointer_UTF32>()
{
    const std::vector<std::vector<char32_t>> terminalCharPtrs = {
        { 0x00000076, 0x0             },   // v
        { 0x000020aC, 0x0             },   // €
        { 0x00010437, 0x0             },   // 𐐷
        { 0x00000065, 0x0001f603, 0x0 },   // e😃
        { 0x0001f3c1, 0x0001f697, 0x0 }    // 🏁🚗
    };

    return prefixWithAsciiSeparators<CharPointer_UTF32> (terminalCharPtrs);
}

template <typename TestFunction>
void withAllPrefixesAndSuffixes (const std::vector<MemoryBlock>& prefixes,
                                 const std::vector<MemoryBlock>& suffixes,
                                 const std::vector<MemoryBlock>& testValues,
                                 TestFunction&& test)
{
    for (const auto& prefix : prefixes)
    {
        for (const auto& testValue : testValues)
        {
            MemoryBlock testBlock = prefix;
            testBlock.append (testValue.getData(), testValue.getSize());

            for (const auto& suffix : suffixes)
            {
                MemoryBlock data = testBlock;
                data.append (suffix.getData(), suffix.getSize());

                test (data, suffix);
            }
        }
    }
}

template <typename CharPointerType>
class CharacterFunctionsTests final : public UnitTest
{
public:
    using CharType = typename CharPointerType::CharType;

    CharacterFunctionsTests()
        : UnitTest ("CharacterFunctions", UnitTestCategories::text)
    {}

    void runTest() override
    {
        beginTest ("readDoubleValue");

        const std::pair<const char*, double> trials[] =
        {
            // Integers
            CHARPTR_DOUBLE_PAIR_COMBOS (0),
            CHARPTR_DOUBLE_PAIR_COMBOS (3),
            CHARPTR_DOUBLE_PAIR_COMBOS (4931),
            CHARPTR_DOUBLE_PAIR_COMBOS (5000),
            CHARPTR_DOUBLE_PAIR_COMBOS (9862097),

            // Floating point numbers
            CHARPTR_DOUBLE_PAIR_COMBOS (0.),
            CHARPTR_DOUBLE_PAIR_COMBOS (9.),
            CHARPTR_DOUBLE_PAIR_COMBOS (7.000),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.2),
            CHARPTR_DOUBLE_PAIR_COMBOS (.298630),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.118),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.9000),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.0000001),
            CHARPTR_DOUBLE_PAIR_COMBOS (500.0000001),
            CHARPTR_DOUBLE_PAIR_COMBOS (9862098.2398604),

            // Exponents
            CHARPTR_DOUBLE_PAIR_COMBOS (0e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (0.00000e0),
            CHARPTR_DOUBLE_PAIR_COMBOS (.0e7),
            CHARPTR_DOUBLE_PAIR_COMBOS (0e-5),
            CHARPTR_DOUBLE_PAIR_COMBOS (2E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (4.E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.2000000E0),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.2000000E6),
            CHARPTR_DOUBLE_PAIR_COMBOS (.398e3),
            CHARPTR_DOUBLE_PAIR_COMBOS (10e10),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.4962e+2),
            CHARPTR_DOUBLE_PAIR_COMBOS (3198693.0973e4),
            CHARPTR_DOUBLE_PAIR_COMBOS (10973097.2087E-4),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.3986e00006),
            CHARPTR_DOUBLE_PAIR_COMBOS (2087.3087e+00006),
            CHARPTR_DOUBLE_PAIR_COMBOS (6.0872e-00006),

            CHARPTR_DOUBLE_PAIR_COMBOS (1.7976931348623157e+308),
            CHARPTR_DOUBLE_PAIR_COMBOS (2.2250738585072014e-308),

            CHARPTR_DOUBLE_PAIR_COMBOS (17654321098765432.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (183456789012345678.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (1934567890123456789.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (20345678901234567891.9),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000000),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752e3),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752e100),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000000e-5),
            CHARPTR_DOUBLE_PAIR_COMBOS (10000000000000000303786028427003666890752.000005e-40),

            CHARPTR_DOUBLE_PAIR_COMBOS (1.23456789012345678901234567890),
            CHARPTR_DOUBLE_PAIR_COMBOS (1.23456789012345678901234567890e-111),
        };

        auto asciiToMemoryBlock = [] (const char* asciiPtr, bool removeNullTerminator)
        {
            auto block = memoryBlockFromCharPtr<CharPointer_ASCII> (asciiPtr);
            return convert<CharPointer_ASCII, CharPointerType> (block, removeNullTerminator);
        };

        const auto separators = getSeparators<CharPointerType>();

        for (const auto& trial : trials)
        {
            for (const auto& terminal : separators.terminals)
            {
                MemoryBlock data { asciiToMemoryBlock (trial.first, true) };
                data.append (terminal.getData(), terminal.getSize());

                CharPointerType charPtr { (CharType*) data.getData() };
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), trial.second);
                expect (*charPtr == *(CharPointerType ((CharType*) terminal.getData())));
            }
        }

        auto asciiToMemoryBlocks = [&] (const std::vector<const char*>& asciiPtrs, bool removeNullTerminator)
        {
            std::vector<MemoryBlock> result;

            for (auto* ptr : asciiPtrs)
                result.push_back (asciiToMemoryBlock (ptr, removeNullTerminator));

            return result;
        };

        std::vector<const char*> prefixCharPtrs = { "" , "+", "-" };
        const auto prefixes = asciiToMemoryBlocks (prefixCharPtrs, true);

        {
            std::vector<const char*> nanCharPtrs = { "NaN", "nan", "NAN", "naN" };
            auto nans = asciiToMemoryBlocks (nanCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, nans, [this] (const MemoryBlock& data,
                                                                                      const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                expect (std::isnan (CharacterFunctions::readDoubleValue (charPtr)));
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            std::vector<const char*> infCharPtrs = { "Inf", "inf", "INF", "InF", "1.0E1024", "1.23456789012345678901234567890e123456789" };
            auto infs = asciiToMemoryBlocks (infCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, infs, [this] (const MemoryBlock& data,
                                                                                      const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                auto expected = charPtr[0] == '-' ? -std::numeric_limits<double>::infinity()
                                                  :  std::numeric_limits<double>::infinity();
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), expected);
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            std::vector<const char*> zeroCharPtrs = { "1.0E-400", "1.23456789012345678901234567890e-123456789" };
            auto zeros = asciiToMemoryBlocks (zeroCharPtrs, true);

            withAllPrefixesAndSuffixes (prefixes, separators.terminals, zeros, [this] (const MemoryBlock& data,
                                                                                       const MemoryBlock& suffix)
            {
                CharPointerType charPtr { (CharType*) data.getData() };
                auto expected = charPtr[0] == '-' ? -0.0 : 0.0;
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), expected);
                expect (*charPtr == *(CharPointerType ((CharType*) suffix.getData())));
            });
        }

        {
            for (const auto& n : separators.nulls)
            {
                MemoryBlock data { n.getData(), n.getSize() };
                CharPointerType charPtr { (CharType*) data.getData() };
                expectEquals (CharacterFunctions::readDoubleValue (charPtr), 0.0);
                expect (charPtr == CharPointerType { (CharType*) data.getData() }.findEndOfWhitespace());
            }
        }
    }
};

static CharacterFunctionsTests<CharPointer_ASCII> characterFunctionsTestsAscii;
static CharacterFunctionsTests<CharPointer_UTF8>  characterFunctionsTestsUtf8;
static CharacterFunctionsTests<CharPointer_UTF16> characterFunctionsTestsUtf16;
static CharacterFunctionsTests<CharPointer_UTF32> characterFunctionsTestsUtf32;

}

#endif

} // namespace juce
