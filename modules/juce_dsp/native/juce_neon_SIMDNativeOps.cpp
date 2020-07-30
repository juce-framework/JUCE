/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
    namespace dsp
    {
        DEFINE_NEON_SIMD_CONST (int32_t, float, kAllBitsSet)     = { -1, -1, -1, -1 };
        DEFINE_NEON_SIMD_CONST (int32_t, float, kEvenHighBit)    = { static_cast<int32_t>(0x80000000), 0, static_cast<int32_t>(0x80000000), 0 };
        DEFINE_NEON_SIMD_CONST (float, float, kOne)              = { 1.0f, 1.0f, 1.0f, 1.0f };

        DEFINE_NEON_SIMD_CONST (int8_t, int8_t, kAllBitsSet)     = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
        DEFINE_NEON_SIMD_CONST (uint8_t, uint8_t, kAllBitsSet)   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        DEFINE_NEON_SIMD_CONST (int16_t, int16_t, kAllBitsSet)   = { -1, -1, -1, -1, -1, -1, -1, -1 };
        DEFINE_NEON_SIMD_CONST (uint16_t, uint16_t, kAllBitsSet) = { 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff };
        DEFINE_NEON_SIMD_CONST (int32_t, int32_t, kAllBitsSet)   = { -1, -1, -1, -1 };
        DEFINE_NEON_SIMD_CONST (uint32_t, uint32_t, kAllBitsSet) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
        DEFINE_NEON_SIMD_CONST (int64_t, int64_t, kAllBitsSet)   = { -1, -1 };
        DEFINE_NEON_SIMD_CONST (uint64_t, uint64_t, kAllBitsSet) = { 0xffffffffffffffff, 0xffffffffffffffff };
    }
}
