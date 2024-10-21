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

namespace juce::dsp
{
    DEFINE_SSE_SIMD_CONST (int32_t, float, kAllBitsSet)     = { -1, -1, -1, -1 };
    DEFINE_SSE_SIMD_CONST (int32_t, float, kEvenHighBit)    = { static_cast<int32_t> (0x80000000), 0, static_cast<int32_t> (0x80000000), 0 };
    DEFINE_SSE_SIMD_CONST (float, float, kOne)              = { 1.0f, 1.0f, 1.0f, 1.0f };

    DEFINE_SSE_SIMD_CONST (int64_t, double, kAllBitsSet)    = { -1LL, -1LL };
    DEFINE_SSE_SIMD_CONST (int64_t, double, kEvenHighBit)   = { static_cast<int64_t> (0x8000000000000000), 0 };
    DEFINE_SSE_SIMD_CONST (double, double, kOne)            = { 1.0, 1.0 };

    DEFINE_SSE_SIMD_CONST (int8_t, int8_t, kAllBitsSet)     = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

    DEFINE_SSE_SIMD_CONST (uint8_t, uint8_t, kAllBitsSet)   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    DEFINE_SSE_SIMD_CONST (uint8_t, uint8_t, kHighBit)      = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };

    DEFINE_SSE_SIMD_CONST (int16_t, int16_t, kAllBitsSet)   = { -1, -1, -1, -1, -1, -1, -1, -1 };

    DEFINE_SSE_SIMD_CONST (uint16_t, uint16_t, kAllBitsSet) = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
    DEFINE_SSE_SIMD_CONST (uint16_t, uint16_t, kHighBit)    = { 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000 };

    DEFINE_SSE_SIMD_CONST (int32_t, int32_t, kAllBitsSet)   = { -1, -1, -1, -1 };

    DEFINE_SSE_SIMD_CONST (uint32_t, uint32_t, kAllBitsSet) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    DEFINE_SSE_SIMD_CONST (uint32_t, uint32_t, kHighBit)    = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };

    DEFINE_SSE_SIMD_CONST (int64_t, int64_t, kAllBitsSet)   = { -1, -1 };

    DEFINE_SSE_SIMD_CONST (uint64_t, uint64_t, kAllBitsSet) = { 0xffffffffffffffff, 0xffffffffffffffff };
    DEFINE_SSE_SIMD_CONST (uint64_t, uint64_t, kHighBit)    = { 0x8000000000000000, 0x8000000000000000 };
}
