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

/** @cond */
namespace juce::dsp
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wignored-attributes")

#ifdef _MSC_VER
 #define DECLARE_NEON_SIMD_CONST(type, name) \
    static __declspec (align (16)) const type name [16 / sizeof (type)]

 #define DEFINE_NEON_SIMD_CONST(type, class_type, name) \
    __declspec (align (16)) const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)]

#else
 #define DECLARE_NEON_SIMD_CONST(type, name) \
    static const type name [16 / sizeof (type)] __attribute__ ((aligned (16)))

 #define DEFINE_NEON_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)] __attribute__ ((aligned (16)))

#endif // _MSC_VER

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Unsigned 32-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint32_t>
{
    //==============================================================================
    using vSIMDType = uint32x4_t;
    using fb = SIMDFallbackOps<uint32_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint32_t s) noexcept                                  { return vdupq_n_u32 (s); }
    static forcedinline vSIMDType load (const uint32_t* a) noexcept                             { return vld1q_u32 (a); }
    static forcedinline void      store (vSIMDType value, uint32_t* a) noexcept                 { vst1q_u32 (a, value); }
    static forcedinline uint32_t  get (vSIMDType v, size_t i) noexcept                          { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, uint32_t s) noexcept              { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                       { return vaddq_u32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                       { return vsubq_u32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                       { return vmulq_u32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                   { return vandq_u32 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                   { return vorrq_u32  (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                   { return veorq_u32 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept                { return vbicq_u32 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                                { return bit_notand (a, vld1q_u32 ((uint32_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                       { return vminq_u32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                       { return vmaxq_u32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                     { return (vSIMDType) vceqq_u32 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                  { return (sum (notEqual (a, b)) == 0); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                  { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept               { return (vSIMDType) vcgtq_u32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept        { return (vSIMDType) vcgeq_u32 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_u32 (a, b, c); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                               { return a; }

    static forcedinline uint32_t sum (vSIMDType a) noexcept
    {
        auto rr = vadd_u32 (vget_high_u32 (a), vget_low_u32 (a));
        return vget_lane_u32 (vpadd_u32 (rr, rr), 0);
    }
};

//==============================================================================
/** Signed 32-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int32_t>
{
    //==============================================================================
    using vSIMDType = int32x4_t;
    using fb = SIMDFallbackOps<int32_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int32_t s) noexcept                                   { return vdupq_n_s32 (s); }
    static forcedinline vSIMDType load (const int32_t* a) noexcept                              { return vld1q_s32 (a); }
    static forcedinline void      store (vSIMDType value, int32_t* a) noexcept                  { vst1q_s32 (a, value); }
    static forcedinline int32_t   get (vSIMDType v, size_t i) noexcept                          { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, int32_t s) noexcept               { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                       { return vaddq_s32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                       { return vsubq_s32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                       { return vmulq_s32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                   { return vandq_s32 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                   { return vorrq_s32 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                   { return veorq_s32 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept                { return vbicq_s32 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                                { return bit_notand (a, vld1q_s32 ((int32_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                       { return vminq_s32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                       { return vmaxq_s32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                     { return (vSIMDType) vceqq_s32 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                  { return (sum (notEqual (a, b)) == 0); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                  { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept               { return (vSIMDType) vcgtq_s32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept        { return (vSIMDType) vcgeq_s32 (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return vmlaq_s32 (a, b, c); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                               { return a; }

    static forcedinline int32_t sum (vSIMDType a) noexcept
    {
        auto rr = vadd_s32 (vget_high_s32 (a), vget_low_s32 (a));
        rr = vpadd_s32 (rr, rr);
        return vget_lane_s32 (rr, 0);
    }
};

//==============================================================================
/** Signed 8-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int8_t>
{
    //==============================================================================
    using vSIMDType = int8x16_t;
    using fb = SIMDFallbackOps<int8_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int8_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int8_t s) noexcept                                   { return vdupq_n_s8 (s); }
    static forcedinline vSIMDType load (const int8_t* a) noexcept                              { return vld1q_s8 (a); }
    static forcedinline void      store (vSIMDType value, int8_t* a) noexcept                  { vst1q_s8 (a, value); }
    static forcedinline int8_t    get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, int8_t s) noexcept               { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_s8 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_s8 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_s8 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return vandq_s8 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return vorrq_s8 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return veorq_s8 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return vbicq_s8 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_s8 ((int8_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_s8 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_s8 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_s8 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_s8 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_s8 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<int32_t>::sum ((SIMDNativeOps<int32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_s8 (a, b, c); }
    static forcedinline int8_t    sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return a; }
};

//==============================================================================
/** Unsigned 8-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint8_t>
{
    //==============================================================================
    using vSIMDType = uint8x16_t;
    using fb = SIMDFallbackOps<uint8_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint8_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint8_t s) noexcept                                  { return vdupq_n_u8 (s); }
    static forcedinline vSIMDType load (const uint8_t* a) noexcept                             { return vld1q_u8 (a); }
    static forcedinline void      store (vSIMDType value, uint8_t* a) noexcept                 { vst1q_u8 (a, value); }
    static forcedinline uint8_t   get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, uint8_t s) noexcept              { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_u8 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_u8 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_u8 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return vandq_u8 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return vorrq_u8 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return veorq_u8 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return vbicq_u8 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_u8 ((uint8_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_u8 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_u8 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_u8 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_u8 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_u8 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<uint32_t>::sum ((SIMDNativeOps<uint32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_u8 (a, b, c); }
    static forcedinline uint8_t   sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return a; }
};

//==============================================================================
/** Signed 16-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int16_t>
{
    //==============================================================================
    using vSIMDType = int16x8_t;
    using fb = SIMDFallbackOps<int16_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int16_t s) noexcept                                  { return vdupq_n_s16 (s); }
    static forcedinline vSIMDType load (const int16_t* a) noexcept                             { return vld1q_s16 (a); }
    static forcedinline void      store (vSIMDType value, int16_t* a) noexcept                 { vst1q_s16 (a, value); }
    static forcedinline int16_t   get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, int16_t s) noexcept              { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_s16 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_s16 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_s16 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return vandq_s16 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return vorrq_s16 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return veorq_s16 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return vbicq_s16 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_s16 ((int16_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_s16 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_s16 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_s16 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_s16 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_s16 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<int32_t>::sum ((SIMDNativeOps<int32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_s16 (a, b, c); }
    static forcedinline int16_t   sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return a; }
};


//==============================================================================
/** Unsigned 16-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint16_t>
{
    //==============================================================================
    using vSIMDType = uint16x8_t;
    using fb = SIMDFallbackOps<uint16_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint16_t s) noexcept                                 { return vdupq_n_u16 (s); }
    static forcedinline vSIMDType load (const uint16_t* a) noexcept                            { return vld1q_u16 (a); }
    static forcedinline void      store (vSIMDType value, uint16_t* a) noexcept                { vst1q_u16 (a, value); }
    static forcedinline uint16_t  get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, uint16_t s) noexcept             { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_u16 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_u16 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_u16 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return vandq_u16 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return vorrq_u16 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return veorq_u16 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return vbicq_u16 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_u16 ((uint16_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_u16 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_u16 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_u16 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_u16 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_u16 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<uint32_t>::sum ((SIMDNativeOps<uint32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_u16 (a, b, c); }
    static forcedinline uint16_t  sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return a; }
};

//==============================================================================
/** Signed 64-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int64_t>
{
    //==============================================================================
    using vSIMDType = int64x2_t;
    using fb = SIMDFallbackOps<int64_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int64_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (int64_t s) noexcept                                  { return vdupq_n_s64 (s); }
    static forcedinline vSIMDType load (const int64_t* a) noexcept                             { return vld1q_s64 (a); }
    static forcedinline void      store (vSIMDType value, int64_t* a) noexcept                 { vst1q_s64 (a, value); }
    static forcedinline int64_t   get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, int64_t s) noexcept              { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_s64 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_s64 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return fb::mul (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return vandq_s64 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return vorrq_s64 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return veorq_s64 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return vbicq_s64 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_s64 ((int64_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return fb::greaterThanOrEqual (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<int32_t>::sum ((SIMDNativeOps<int32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return fb::multiplyAdd (a, b, c); }
    static forcedinline int64_t   sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return a; }
};


//==============================================================================
/** Unsigned 64-bit integer NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint64_t>
{
    //==============================================================================
    using vSIMDType = uint64x2_t;
    using fb = SIMDFallbackOps<uint64_t, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (uint64_t, kAllBitsSet);

    //==============================================================================
    static forcedinline vSIMDType expand (uint64_t s) noexcept                                  { return vdupq_n_u64 (s); }
    static forcedinline vSIMDType load (const uint64_t* a) noexcept                             { return vld1q_u64 (a); }
    static forcedinline void      store (vSIMDType value, uint64_t* a) noexcept                 { vst1q_u64 (a, value); }
    static forcedinline uint64_t  get (vSIMDType v, size_t i) noexcept                          { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, uint64_t s) noexcept              { return fb::set (v, i, s); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                       { return vaddq_u64 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                       { return vsubq_u64 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                       { return fb::mul (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                   { return vandq_u64 (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                   { return vorrq_u64 (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                   { return veorq_u64 (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept                { return vbicq_u64 (b, a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                                { return bit_notand (a, vld1q_u64 ((uint64_t*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                       { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                       { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                     { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                  { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept               { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept        { return fb::greaterThanOrEqual (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                  { return (SIMDNativeOps<uint32_t>::sum ((SIMDNativeOps<uint32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept  { return fb::multiplyAdd (a, b, c); }
    static forcedinline uint64_t  sum (vSIMDType a) noexcept                                    { return fb::sum (a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                               { return a; }
};

    //==============================================================================
/** Single-precision floating point NEON intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<float>
{
    //==============================================================================
    using vSIMDType = float32x4_t;
    using vMaskType = uint32x4_t;
    using fb = SIMDFallbackOps<float, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int32_t, kAllBitsSet);
    DECLARE_NEON_SIMD_CONST (int32_t, kEvenHighBit);
    DECLARE_NEON_SIMD_CONST (float, kOne);

    //==============================================================================
    static forcedinline vSIMDType expand (float s) noexcept                                    { return vdupq_n_f32 (s); }
    static forcedinline vSIMDType load (const float* a) noexcept                               { return vld1q_f32 (a); }
    static forcedinline float     get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, float s) noexcept                { return fb::set (v, i, s); }
    static forcedinline void      store (vSIMDType value, float* a) noexcept                   { vst1q_f32 (a, value); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_f32 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_f32 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_f32 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vandq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vorrq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) veorq_u32 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return (vSIMDType) vbicq_u32 ((vMaskType) b, (vMaskType) a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_f32 ((float*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_f32 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_f32 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_f32 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_f32 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_f32 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<uint32_t>::sum ((SIMDNativeOps<uint32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_f32 (a, b, c); }
    static forcedinline vSIMDType dupeven (vSIMDType a) noexcept                               { return fb::shuffle<(0 << 0) | (0 << 2) | (2 << 4) | (2 << 6)>     (a); }
    static forcedinline vSIMDType dupodd  (vSIMDType a) noexcept                               { return fb::shuffle<(1 << 0) | (1 << 2) | (3 << 4) | (3 << 6)>     (a); }
    static forcedinline vSIMDType swapevenodd (vSIMDType a) noexcept                           { return fb::shuffle<(1 << 0) | (0 << 2) | (3 << 4) | (2 << 6)> (a); }
    static forcedinline vSIMDType oddevensum (vSIMDType a) noexcept                            { return add (fb::shuffle<(2 << 0) | (3 << 2) | (0 << 4) | (1 << 6)> (a), a); }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return vcvtq_f32_s32 (vcvtq_s32_f32 (a)); }

    //==============================================================================
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept
    {
        vSIMDType rr_ir = mul (a, dupeven (b));
        vSIMDType ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vld1q_f32 ((float*) kEvenHighBit)));
    }

    static forcedinline float sum (vSIMDType a) noexcept
    {
        auto rr = vadd_f32 (vget_high_f32 (a), vget_low_f32 (a));
        return vget_lane_f32 (vpadd_f32 (rr, rr), 0);
    }
};

//==============================================================================
/** Double-precision floating point NEON intrinsics does not exist in NEON
    so we need to emulate this.

    @tags{DSP}
*/
#if JUCE_64BIT
template <>
struct SIMDNativeOps<double>
{
    //==============================================================================
    using vSIMDType = float64x2_t;
    using vMaskType = uint64x2_t;
    using fb = SIMDFallbackOps<double, vSIMDType>;

    //==============================================================================
    DECLARE_NEON_SIMD_CONST (int64_t, kAllBitsSet);
    DECLARE_NEON_SIMD_CONST (double, kOne);

    //==============================================================================
    static forcedinline vSIMDType expand (double s) noexcept                                   { return vdupq_n_f64 (s); }
    static forcedinline vSIMDType load (const double* a) noexcept                              { return vld1q_f64 (a); }
    static forcedinline double    get (vSIMDType v, size_t i) noexcept                         { return fb::get (v, i); }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, double s) noexcept               { return fb::set (v, i, s); }
    static forcedinline void      store (vSIMDType value, double* a) noexcept                  { vst1q_f64 (a, value); }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return vaddq_f64 (a, b); }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return vsubq_f64 (a, b); }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return vmulq_f64 (a, b); }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vandq_u64 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) vorrq_u64 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return (vSIMDType) veorq_u64 ((vMaskType) a, (vMaskType) b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return (vSIMDType) vbicq_u64 ((vMaskType) b, (vMaskType) a); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return bit_notand (a, vld1q_f64 ((double*) kAllBitsSet)); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return vminq_f64 (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return vmaxq_f64 (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return (vSIMDType) vceqq_f64 (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return bit_not (equal (a, b)); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return (vSIMDType) vcgtq_f64 (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return (vSIMDType) vcgeq_f64 (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return (SIMDNativeOps<uint32_t>::sum ((SIMDNativeOps<uint32_t>::vSIMDType) notEqual (a, b)) == 0); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return vmlaq_f64 (a, b, c); }
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept                 { return fb::cmplxmul (a, b); }
    static forcedinline double    sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType oddevensum (vSIMDType a) noexcept                            { return a; }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return vcvtq_f64_s64 (vcvtq_s64_f64 (a)); }
};
#else
template <>
struct SIMDNativeOps<double>
{
    //==============================================================================
    using vSIMDType = struct { double v[2]; };
    using fb = SIMDFallbackOps<double, vSIMDType>;

    static forcedinline vSIMDType expand (double s) noexcept                                   { return {{s, s}}; }
    static forcedinline vSIMDType load (const double* a) noexcept                              { return {{a[0], a[1]}}; }
    static forcedinline void      store (vSIMDType v, double* a) noexcept                      { a[0] = v.v[0]; a[1] = v.v[1]; }
    static forcedinline double    get (vSIMDType v, size_t i) noexcept                         { return v.v[i]; }
    static forcedinline vSIMDType set (vSIMDType v, size_t i, double s) noexcept               { v.v[i] = s; return v; }
    static forcedinline vSIMDType add (vSIMDType a, vSIMDType b) noexcept                      { return {{a.v[0] + b.v[0], a.v[1] + b.v[1]}}; }
    static forcedinline vSIMDType sub (vSIMDType a, vSIMDType b) noexcept                      { return {{a.v[0] - b.v[0], a.v[1] - b.v[1]}}; }
    static forcedinline vSIMDType mul (vSIMDType a, vSIMDType b) noexcept                      { return {{a.v[0] * b.v[0], a.v[1] * b.v[1]}}; }
    static forcedinline vSIMDType bit_and (vSIMDType a, vSIMDType b) noexcept                  { return fb::bit_and (a, b); }
    static forcedinline vSIMDType bit_or  (vSIMDType a, vSIMDType b) noexcept                  { return fb::bit_or  (a, b); }
    static forcedinline vSIMDType bit_xor (vSIMDType a, vSIMDType b) noexcept                  { return fb::bit_xor (a, b); }
    static forcedinline vSIMDType bit_notand (vSIMDType a, vSIMDType b) noexcept               { return fb::bit_notand (a, b); }
    static forcedinline vSIMDType bit_not (vSIMDType a) noexcept                               { return fb::bit_not (a); }
    static forcedinline vSIMDType min (vSIMDType a, vSIMDType b) noexcept                      { return fb::min (a, b); }
    static forcedinline vSIMDType max (vSIMDType a, vSIMDType b) noexcept                      { return fb::max (a, b); }
    static forcedinline vSIMDType equal (vSIMDType a, vSIMDType b) noexcept                    { return fb::equal (a, b); }
    static forcedinline vSIMDType notEqual (vSIMDType a, vSIMDType b) noexcept                 { return fb::notEqual (a, b); }
    static forcedinline vSIMDType greaterThan (vSIMDType a, vSIMDType b) noexcept              { return fb::greaterThan (a, b); }
    static forcedinline vSIMDType greaterThanOrEqual (vSIMDType a, vSIMDType b) noexcept       { return fb::greaterThanOrEqual (a, b); }
    static forcedinline bool      allEqual (vSIMDType a, vSIMDType b) noexcept                 { return fb::allEqual (a, b); }
    static forcedinline vSIMDType multiplyAdd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept { return fb::multiplyAdd (a, b, c); }
    static forcedinline vSIMDType cmplxmul (vSIMDType a, vSIMDType b) noexcept                 { return fb::cmplxmul (a, b); }
    static forcedinline double    sum (vSIMDType a) noexcept                                   { return fb::sum (a); }
    static forcedinline vSIMDType oddevensum (vSIMDType a) noexcept                            { return a; }
    static forcedinline vSIMDType truncate (vSIMDType a) noexcept                              { return fb::truncate (a); }
};
#endif // JUCE_64BIT

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce::dsp
/** @endcond */
