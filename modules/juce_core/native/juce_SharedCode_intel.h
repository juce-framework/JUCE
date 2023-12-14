/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
