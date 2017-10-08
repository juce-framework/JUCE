/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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

#ifndef DOXYGEN

#ifdef _MSC_VER
 #define DECLARE_NEON_SIMD_CONST(type, name) \
    static __declspec(align(16)) const type name [16 / sizeof (type)]

 #define DEFINE_NEON_SIMD_CONST(type, class_type, name) \
    __declspec(align(16)) const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)]

#else
 #define DECLARE_NEON_SIMD_CONST(type, name) \
    static const type name [16 / sizeof (type)] __attribute__((aligned(16)))

 #define DEFINE_NEON_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)] __attribute__((aligned(16)))

#endif

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Single-precision floating point NEON intrinsics. */
template <>
struct SIMDNativeOps<float>
{
    //==============================================================================
    typedef float32x4_t vSIMDType;
    typedef uint32x4_t vMaskType;
    typedef SIMDFallbackOps<float, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int32_t, kAllBitsSet);
    DECLARE_NEON_SIMD_CONST (int32_t, kEvenHighBit);
    DECLARE_NEON_SIMD_CONST (float, kOne);

    //==============================================================================
    static forcedinline vSIMDType expand (float s) noexcept                          { return vdupq_n_f32 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_f32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_f32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_f32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return (vSIMDType) vandq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return (vSIMDType) vorrq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return (vSIMDType) veorq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return (vSIMDType) vbicq_u32 ((vMaskType) b, (vMaskType) a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                   { return bit_notand (a, vld1q_f32 ((float*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_f32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_f32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_f32 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_f32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_f32 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_f32 (a, b, c); }
    static forcedinline vSIMDType dupeven (vSIMDType a) noexcept { return fb::shuffle<(0 << 0) | (0 << 2) | (2 << 4) | (2 << 6)>     (a); }
    static forcedinline vSIMDType dupodd  (vSIMDType a) noexcept { return fb::shuffle<(1 << 0) | (1 << 2) | (3 << 4) | (3 << 6)>     (a); }
    static forcedinline vSIMDType swapevenodd (vSIMDType a) noexcept { return fb::shuffle<(1 << 0) | (0 << 2) | (3 << 4) | (2 << 6)> (a); }
    static forcedinline float sum (vSIMDType a) noexcept { return fb::sum (a); }
    static forcedinline vSIMDType oddevensum (vSIMDType a) noexcept { return add (fb::shuffle<(2 << 0) | (3 << 2) | (0 << 4) | (1 << 6)> (a), a); }

    //==============================================================================
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType rr_ir = mul (a, dupeven (b));
        vSIMDType ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vld1q_f32 ((float*) kEvenHighBit)));
    }
};

//==============================================================================
/** Double-precision floating point NEON intrinsics does not exist in NEON
    so we need to emulate this.
*/
template <>
struct SIMDNativeOps<double>
{
    //==============================================================================
    typedef struct { double values [2]; } vSIMDType;
    typedef SIMDFallbackOps<double, vSIMDType> fb;

    static forcedinline vSIMDType expand (double s) noexcept                     { return fb::expand (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return fb::add (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return fb::sub (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return fb::mul (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return fb::bit_and (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return fb::bit_or  (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return fb::bit_xor (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return fb::bit_notand (a, b); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return fb::bit_not (a); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return fb::greaterThanOrEqual (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return fb::multiplyAdd (a, b, c); }
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept { return fb::cmplxmul (a, b); }
    static forcedinline double sum (vSIMDType a) noexcept { return fb::sum (a); }
    static forcedinline vSIMDType oddevensum (vSIMDType a) noexcept { return a; }
};

//==============================================================================
/** Signed 8-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<int8_t>
{
    //==============================================================================
    typedef int8x16_t vSIMDType;
    typedef SIMDFallbackOps<int8_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int8_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int8_t s) noexcept                     { return vdupq_n_s8 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_s8 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_s8 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_s8 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_s8 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_s8 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_s8 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_s8 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_s8 ((int8_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_s8 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_s8 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_s8 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_s8 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_s8 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_s8 (a, b, c); }
    static forcedinline int8_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};

//==============================================================================
/** Unsigned 8-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<uint8_t>
{
    //==============================================================================
    typedef uint8x16_t vSIMDType;
    typedef SIMDFallbackOps<uint8_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint8_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint8_t s) noexcept                     { return vdupq_n_u8 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_u8 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_u8 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_u8 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_u8 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_u8 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_u8 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_u8 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_u8 ((uint8_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_u8 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_u8 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_u8 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_u8 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_u8 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_u8 (a, b, c); }
    static forcedinline uint8_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};

//==============================================================================
/** Signed 16-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<int16_t>
{
    //==============================================================================
    typedef int16x8_t vSIMDType;
    typedef SIMDFallbackOps<int16_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int16_t s) noexcept                    { return vdupq_n_s16 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_s16 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_s16 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_s16 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_s16 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_s16 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_s16 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_s16 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_s16 ((int16_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_s16 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_s16 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_s16 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_s16 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_s16 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_s16 (a, b, c); }
    static forcedinline int16_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};


//==============================================================================
/** Unsigned 16-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<uint16_t>
{
    //==============================================================================
    typedef uint16x8_t vSIMDType;
    typedef SIMDFallbackOps<uint16_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint16_t s) noexcept                    { return vdupq_n_u16 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_u16 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_u16 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_u16 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_u16 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_u16 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_u16 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_u16 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_u16 ((uint16_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_u16 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_u16 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_u16 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_u16 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_u16 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_u16 (a, b, c); }
    static forcedinline uint16_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};

//==============================================================================
/** Signed 32-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<int32_t>
{
    //==============================================================================
    typedef int32x4_t vSIMDType;
    typedef SIMDFallbackOps<int32_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int32_t s) noexcept                    { return vdupq_n_s32 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_s32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_s32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_s32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_s32 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_s32 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_s32 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_s32 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_s32 ((int32_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_s32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_s32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_s32 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_s32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_s32 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_s32 (a, b, c); }
    static forcedinline int32_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};


//==============================================================================
/** Unsigned 32-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<uint32_t>
{
    //==============================================================================
    typedef uint32x4_t vSIMDType;
    typedef SIMDFallbackOps<uint32_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint32_t s) noexcept                    { return vdupq_n_u32 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_u32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_u32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return vmulq_u32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_u32 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_u32  (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_u32 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_u32 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_u32 ((uint32_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return vminq_u32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return vmaxq_u32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vceqq_u32 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return (vSIMDType) vcgtq_u32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return (vSIMDType) vcgeq_u32 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_u32 (a, b, c); }
    static forcedinline uint32_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};

//==============================================================================
/** Signed 64-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<int64_t>
{
    //==============================================================================
    typedef int64x2_t vSIMDType;
    typedef SIMDFallbackOps<int64_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int64_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int64_t s) noexcept                    { return vdupq_n_s64 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_s64 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_s64 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return fb::mul (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_s64 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_s64 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_s64 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_s64 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_s64 ((int64_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return fb::greaterThanOrEqual (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return fb::multiplyAdd (a, b, c); }
    static forcedinline int64_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};


//==============================================================================
/** Unsigned 64-bit integer NEON intrinsics. */
template <>
struct SIMDNativeOps<uint64_t>
{
    //==============================================================================
    typedef uint64x2_t vSIMDType;
    typedef SIMDFallbackOps<uint64_t, vSIMDType> fb;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint64_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint64_t s) noexcept                   { return vdupq_n_u64 (s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept        { return vaddq_u64 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept        { return vsubq_u64 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept        { return fb::mul (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept    { return vandq_u64 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept    { return vorrq_u64 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept    { return veorq_u64 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept { return vbicq_u64 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                 { return bit_notand (a, vld1q_u64 ((uint64_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                    { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                    { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                  { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept               { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept            { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept     { return fb::greaterThanOrEqual (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return fb::multiplyAdd (a, b, c); }
    static forcedinline uint64_t sum (vSIMDType a) noexcept { return fb::sum (a); }
};

#endif

} // namespace dsp
} // namespace juce
