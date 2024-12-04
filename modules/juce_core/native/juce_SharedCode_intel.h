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

#if JUCE_INTEL && ! JUCE_NO_INLINE_ASM

namespace juce::SystemStatsHelpers
{

static void doCPUID (uint32& a, uint32& b, uint32& c, uint32& d, uint32 type)
{
    uint32 la = a, lb = b, lc = c, ld = d;

   #if JUCE_32BIT && defined (__pic__)
    asm ("mov %%ebx, %%edi\n"
         "cpuid\n"
         "xchg %%edi, %%ebx\n"
           : "=a" (la), "=D" (lb), "=c" (lc), "=d" (ld)
           : "a" (type), "c" (0));
   #else
    asm ("cpuid\n"
           : "=a" (la), "=b" (lb), "=c" (lc), "=d" (ld)
           : "a" (type), "c" (0));
   #endif

    a = la; b = lb; c = lc; d = ld;
}

static void getCPUInfo (bool& hasMMX,
                        bool& hasSSE,
                        bool& hasSSE2,
                        bool& has3DNow,
                        bool& hasSSE3,
                        bool& hasSSSE3,
                        bool& hasFMA3,
                        bool& hasSSE41,
                        bool& hasSSE42,
                        bool& hasAVX,
                        bool& hasFMA4,
                        bool& hasAVX2,
                        bool& hasAVX512F,
                        bool& hasAVX512DQ,
                        bool& hasAVX512IFMA,
                        bool& hasAVX512PF,
                        bool& hasAVX512ER,
                        bool& hasAVX512CD,
                        bool& hasAVX512BW,
                        bool& hasAVX512VL,
                        bool& hasAVX512VBMI,
                        bool& hasAVX512VPOPCNTDQ)
{
    uint32 a = 0, b = 0, d = 0, c = 0;
    SystemStatsHelpers::doCPUID (a, b, c, d, 1);

    hasMMX   = (d & (1u << 23)) != 0;
    hasSSE   = (d & (1u << 25)) != 0;
    hasSSE2  = (d & (1u << 26)) != 0;
    has3DNow = (b & (1u << 31)) != 0;
    hasSSE3  = (c & (1u <<  0)) != 0;
    hasSSSE3 = (c & (1u <<  9)) != 0;
    hasFMA3  = (c & (1u << 12)) != 0;
    hasSSE41 = (c & (1u << 19)) != 0;
    hasSSE42 = (c & (1u << 20)) != 0;
    hasAVX   = (c & (1u << 28)) != 0;

    SystemStatsHelpers::doCPUID (a, b, c, d, 0x80000001);
    hasFMA4  = (c & (1u << 16)) != 0;

    SystemStatsHelpers::doCPUID (a, b, c, d, 7);
    hasAVX2            = (b & (1u <<  5)) != 0;
    hasAVX512F         = (b & (1u << 16)) != 0;
    hasAVX512DQ        = (b & (1u << 17)) != 0;
    hasAVX512IFMA      = (b & (1u << 21)) != 0;
    hasAVX512PF        = (b & (1u << 26)) != 0;
    hasAVX512ER        = (b & (1u << 27)) != 0;
    hasAVX512CD        = (b & (1u << 28)) != 0;
    hasAVX512BW        = (b & (1u << 30)) != 0;
    hasAVX512VL        = (b & (1u << 31)) != 0;
    hasAVX512VBMI      = (c & (1u <<  1)) != 0;
    hasAVX512VPOPCNTDQ = (c & (1u << 14)) != 0;
}

} // namespace juce::SystemStatsHelpers

#endif
