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

#ifndef DOXYGEN

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wignored-attributes")

#ifdef _MSC_VER
 #define DECLARE_SSE_SIMD_CONST(type, name) \
    static __declspec(align(16)) const type name [16 / sizeof (type)]

 #define DEFINE_SSE_SIMD_CONST(type, class_type, name) \
    __declspec(align(16)) const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)]

#else
 #define DECLARE_SSE_SIMD_CONST(type, name) \
    static const type name [16 / sizeof (type)] __attribute__((aligned(16)))

 #define DEFINE_SSE_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name [16 / sizeof (type)] __attribute__((aligned(16)))

#endif

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Single-precision floating point SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<float>
{
    //==============================================================================
    using vSIMDType = __m128;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int32_t, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (int32_t, kEvenHighBit);
    DECLARE_SSE_SIMD_CONST (float, kOne);

    //==============================================================================
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE expand (float s) noexcept                            { return _mm_load1_ps (&s); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE load (const float* a) noexcept                       { return _mm_load_ps (a); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m128 value, float* dest) noexcept             { _mm_store_ps (dest, value); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE add (__m128 a, __m128 b) noexcept                    { return _mm_add_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE sub (__m128 a, __m128 b) noexcept                    { return _mm_sub_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE mul (__m128 a, __m128 b) noexcept                    { return _mm_mul_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE bit_and (__m128 a, __m128 b) noexcept                { return _mm_and_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE bit_or  (__m128 a, __m128 b) noexcept                { return _mm_or_ps  (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE bit_xor (__m128 a, __m128 b) noexcept                { return _mm_xor_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE bit_notand (__m128 a, __m128 b) noexcept             { return _mm_andnot_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE bit_not (__m128 a) noexcept                          { return bit_notand (a, _mm_loadu_ps ((float*) kAllBitsSet)); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE min (__m128 a, __m128 b) noexcept                    { return _mm_min_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE max (__m128 a, __m128 b) noexcept                    { return _mm_max_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE equal (__m128 a, __m128 b) noexcept                  { return _mm_cmpeq_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE notEqual (__m128 a, __m128 b) noexcept               { return _mm_cmpneq_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE greaterThan (__m128 a, __m128 b) noexcept            { return _mm_cmpgt_ps (a, b); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128 a, __m128 b) noexcept     { return _mm_cmpge_ps (a, b); }
    static forcedinline bool   JUCE_VECTOR_CALLTYPE allEqual (__m128 a, __m128 b ) noexcept              { return (_mm_movemask_ps (equal (a, b)) == 0xf); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE multiplyAdd (__m128 a, __m128 b, __m128 c) noexcept  { return _mm_add_ps (a, _mm_mul_ps (b, c)); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE dupeven (__m128 a) noexcept                          { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (2, 2, 0, 0)); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE dupodd (__m128 a) noexcept                           { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (3, 3, 1, 1)); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE swapevenodd (__m128 a) noexcept                      { return _mm_shuffle_ps (a, a, _MM_SHUFFLE (2, 3, 0, 1)); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE oddevensum (__m128 a) noexcept                       { return _mm_add_ps (_mm_shuffle_ps (a, a, _MM_SHUFFLE (1, 0, 3, 2)), a); }
    static forcedinline float  JUCE_VECTOR_CALLTYPE get (__m128 v, size_t i) noexcept                    { return SIMDFallbackOps<float, __m128>::get (v, i); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE set (__m128 v, size_t i, float s) noexcept           { return SIMDFallbackOps<float, __m128>::set (v, i, s); }
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE truncate (__m128 a) noexcept                         { return _mm_cvtepi32_ps (_mm_cvttps_epi32 (a)); }

    //==============================================================================
    static forcedinline __m128 JUCE_VECTOR_CALLTYPE cmplxmul (__m128 a, __m128 b) noexcept
    {
        __m128 rr_ir = mul (a, dupeven (b));
        __m128 ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, _mm_loadu_ps ((float*) kEvenHighBit)));
    }

    static forcedinline float JUCE_VECTOR_CALLTYPE sum (__m128 a) noexcept
    {
       #if defined(__SSE4__)
        __m128 retval = _mm_dp_ps (a, _mm_loadu_ps (kOne), 0xff);
       #elif defined(__SSE3__)
        __m128 retval = _mm_hadd_ps (_mm_hadd_ps (a, a), a);
       #else
        __m128 retval = _mm_add_ps (_mm_shuffle_ps (a, a, 0x4e), a);
        retval = _mm_add_ps (retval, _mm_shuffle_ps (retval, retval, 0xb1));
       #endif
        return _mm_cvtss_f32 (retval);
    }
};

//==============================================================================
/** Double-precision floating point SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<double>
{
    //==============================================================================
    using vSIMDType = __m128d;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int64_t, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (int64_t, kEvenHighBit);
    DECLARE_SSE_SIMD_CONST (double, kOne);

    //==============================================================================
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE vconst (const double* a) noexcept                       { return load (a); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE vconst (const int64_t* a) noexcept                      { return _mm_castsi128_pd (_mm_load_si128 (reinterpret_cast<const __m128i*> (a))); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE expand (double s) noexcept                              { return _mm_load1_pd (&s); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE load (const double* a) noexcept                         { return _mm_load_pd (a); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m128d value, double* dest) noexcept               { _mm_store_pd (dest, value); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE add (__m128d a, __m128d b) noexcept                     { return _mm_add_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE sub (__m128d a, __m128d b) noexcept                     { return _mm_sub_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE mul (__m128d a, __m128d b) noexcept                     { return _mm_mul_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE bit_and (__m128d a, __m128d b) noexcept                 { return _mm_and_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE bit_or  (__m128d a, __m128d b) noexcept                 { return _mm_or_pd  (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE bit_xor (__m128d a, __m128d b) noexcept                 { return _mm_xor_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE bit_notand (__m128d a, __m128d b) noexcept              { return _mm_andnot_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE bit_not (__m128d a) noexcept                            { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE min (__m128d a, __m128d b) noexcept                     { return _mm_min_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE max (__m128d a, __m128d b) noexcept                     { return _mm_max_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE equal (__m128d a, __m128d b) noexcept                   { return _mm_cmpeq_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE notEqual (__m128d a, __m128d b) noexcept                { return _mm_cmpneq_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE greaterThan (__m128d a, __m128d b) noexcept             { return _mm_cmpgt_pd (a, b); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128d a, __m128d b) noexcept      { return _mm_cmpge_pd (a, b); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128d a, __m128d b ) noexcept               { return (_mm_movemask_pd (equal (a, b)) == 0x3); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE multiplyAdd (__m128d a, __m128d b, __m128d c) noexcept  { return _mm_add_pd (a, _mm_mul_pd (b, c)); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE dupeven (__m128d a) noexcept                            { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (0, 0)); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE dupodd (__m128d a) noexcept                             { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (1, 1)); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE swapevenodd (__m128d a) noexcept                        { return _mm_shuffle_pd (a, a, _MM_SHUFFLE2 (0, 1)); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE oddevensum (__m128d a) noexcept                         { return a; }
    static forcedinline double  JUCE_VECTOR_CALLTYPE get (__m128d v, size_t i) noexcept                      { return SIMDFallbackOps<double, __m128d>::get (v, i); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE set (__m128d v, size_t i, double s) noexcept            { return SIMDFallbackOps<double, __m128d>::set (v, i, s); }
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE truncate (__m128d a) noexcept                           { return _mm_cvtepi32_pd (_mm_cvttpd_epi32 (a)); }

    //==============================================================================
    static forcedinline __m128d JUCE_VECTOR_CALLTYPE cmplxmul (__m128d a, __m128d b) noexcept
    {
        __m128d rr_ir = mul (a, dupeven (b));
        __m128d ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline double JUCE_VECTOR_CALLTYPE sum (__m128d a) noexcept
    {
       #if defined(__SSE4__)
        __m128d retval = _mm_dp_pd (a, vconst (kOne), 0xff);
       #elif defined(__SSE3__)
        __m128d retval = _mm_hadd_pd (a, a);
       #else
        __m128d retval = _mm_add_pd (_mm_shuffle_pd (a, a, 0x01), a);
       #endif
        return _mm_cvtsd_f64 (retval);
    }
};

//==============================================================================
/** Signed 8-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int8_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int8_t, kAllBitsSet);

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE vconst (const int8_t* a) noexcept                       { return load (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE load (const int8_t* a) noexcept                         { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m128i v, int8_t* p) noexcept                   { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE expand (int8_t s) noexcept                              { return _mm_set1_epi8 (s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
   #if defined(__SSE4__)
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epi8 (a, b); }
   #else
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
   #endif
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline int8_t  JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<int8_t, __m128i>::get (v, i); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, int8_t s) noexcept            { return SIMDFallbackOps<int8_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline int8_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i lo = _mm_unpacklo_epi8 (a, _mm_setzero_si128());
        __m128i hi = _mm_unpackhi_epi8 (a, _mm_setzero_si128());

        for (int i = 0; i < 3; ++i)
        {
            lo = _mm_hadd_epi16 (lo, lo);
            hi = _mm_hadd_epi16 (hi, hi);
        }

        return static_cast<int8_t> ((_mm_cvtsi128_si32 (lo) & 0xff) + (_mm_cvtsi128_si32 (hi) & 0xff));
       #else
        return SIMDFallbackOps<int8_t, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b)
    {
        // unpack and multiply
        __m128i even = _mm_mullo_epi16 (a, b);
        __m128i odd  = _mm_mullo_epi16 (_mm_srli_epi16 (a, 8), _mm_srli_epi16 (b, 8));

        return _mm_or_si128 (_mm_slli_epi16 (odd, 8),
                             _mm_srli_epi16 (_mm_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Unsigned 8-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint8_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (uint8_t, kHighBit);
    DECLARE_SSE_SIMD_CONST (uint8_t, kAllBitsSet);

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE vconst (const uint8_t* a) noexcept                      { return load (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE load (const uint8_t* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m128i v, uint8_t* p) noexcept                     { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE expand (uint8_t s) noexcept                             { return _mm_set1_epi8 ((int8_t) s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epu8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epu8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi8 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi8 (ssign (a), ssign (b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline uint8_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<uint8_t, __m128i>::get (v, i); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, uint8_t s) noexcept           { return SIMDFallbackOps<uint8_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline uint8_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i lo = _mm_unpacklo_epi8 (a, _mm_setzero_si128());
        __m128i hi = _mm_unpackhi_epi8 (a, _mm_setzero_si128());

        for (int i = 0; i < 3; ++i)
        {
            lo = _mm_hadd_epi16 (lo, lo);
            hi = _mm_hadd_epi16 (hi, hi);
        }

        return static_cast<uint8_t> ((static_cast<uint32_t> (_mm_cvtsi128_si32 (lo)) & 0xffu)
                                   + (static_cast<uint32_t> (_mm_cvtsi128_si32 (hi)) & 0xffu));
       #else
        return SIMDFallbackOps<uint8_t, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b)
    {
        // unpack and multiply
        __m128i even = _mm_mullo_epi16 (a, b);
        __m128i odd  = _mm_mullo_epi16 (_mm_srli_epi16 (a, 8), _mm_srli_epi16 (b, 8));

        return _mm_or_si128 (_mm_slli_epi16 (odd, 8),
                             _mm_srli_epi16 (_mm_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Signed 16-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int16_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE vconst (const int16_t* a) noexcept                      { return load (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE load (const int16_t* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m128i v, int16_t* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE expand (int16_t s) noexcept                             { return _mm_set1_epi16 (s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return _mm_mullo_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi16 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline int16_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<int16_t, __m128i>::get (v, i); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, int16_t s) noexcept           { return SIMDFallbackOps<int16_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline int16_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi16 (a, a);
        tmp = _mm_hadd_epi16 (tmp, tmp);
        tmp = _mm_hadd_epi16 (tmp, tmp);

        return static_cast<int16_t> (_mm_cvtsi128_si32 (tmp) & 0xffff);
       #else
        return SIMDFallbackOps<int16_t, __m128i>::sum (a);
       #endif
    }
};

//==============================================================================
/** Unsigned 16-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint16_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (uint16_t, kHighBit);
    DECLARE_SSE_SIMD_CONST (uint16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE vconst (const uint16_t* a) noexcept                     { return load (a); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE load (const uint16_t* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m128i v, uint16_t* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE expand (uint16_t s) noexcept                            { return _mm_set1_epi16 ((int16_t) s); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi16 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi16 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return _mm_mullo_epi16 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
   #if defined(__SSE4__)
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { return _mm_min_epu16 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { return _mm_max_epu16 (a, b); }
   #else
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
   #endif
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi16 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi16 (ssign (a), ssign (b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline uint16_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<uint16_t, __m128i>::get (v, i); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, uint16_t s) noexcept          { return SIMDFallbackOps<uint16_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline uint16_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi16 (a, a);
        tmp = _mm_hadd_epi16 (tmp, tmp);
        tmp = _mm_hadd_epi16 (tmp, tmp);

        return static_cast<uint16_t> (static_cast<uint32_t> (_mm_cvtsi128_si32 (tmp)) & 0xffffu);
       #else
        return SIMDFallbackOps<uint16_t, __m128i>::sum (a);
       #endif
    }
};

//==============================================================================
/** Signed 32-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int32_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE vconst (const int32_t* a) noexcept                      { return load (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE load (const int32_t* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m128i v, int32_t* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE expand (int32_t s) noexcept                             { return _mm_set1_epi32 (s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi32 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi32 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi32 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi32 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline int32_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<int32_t, __m128i>::get (v, i); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, int32_t s) noexcept           { return SIMDFallbackOps<int32_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline int32_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi32 (a, a);
        return _mm_cvtsi128_si32 (_mm_hadd_epi32 (tmp, tmp));
       #else
        return SIMDFallbackOps<int32_t, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_mullo_epi32 (a, b);
       #else
        __m128i even = _mm_mul_epu32 (a,b);
        __m128i odd = _mm_mul_epu32 (_mm_srli_si128 (a,4), _mm_srli_si128 (b,4));
        return _mm_unpacklo_epi32 (_mm_shuffle_epi32(even, _MM_SHUFFLE (0,0,2,0)),
                                   _mm_shuffle_epi32(odd,  _MM_SHUFFLE (0,0,2,0)));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_min_epi32 (a, b);
       #else
        __m128i lt = greaterThan (b, a);
        return bit_or (bit_and (lt, a), bit_andnot (lt, b));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_max_epi32 (a, b);
       #else
        __m128i gt = greaterThan (a, b);
        return bit_or (bit_and (gt, a), bit_andnot (gt, b));
       #endif
    }
};

//==============================================================================
/** Unsigned 32-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint32_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (uint32_t, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (uint32_t, kHighBit);

    //==============================================================================
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE vconst (const uint32_t* a) noexcept                     { return load (a); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE load (const uint32_t* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m128i v, uint32_t* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE expand (uint32_t s) noexcept                            { return _mm_set1_epi32 ((int32_t) s); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi32 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi32 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept                   { return _mm_cmpeq_epi32 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept             { return _mm_cmpgt_epi32 (ssign (a), ssign (b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline uint32_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<uint32_t, __m128i>::get (v, i); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, uint32_t s) noexcept          { return SIMDFallbackOps<uint32_t, __m128i>::set (v, i, s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline uint32_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept
    {
       #ifdef __SSSE3__
        __m128i tmp = _mm_hadd_epi32 (a, a);
        return static_cast<uint32_t> (_mm_cvtsi128_si32 (_mm_hadd_epi32 (tmp, tmp)));
       #else
        return SIMDFallbackOps<uint32_t, __m128i>::sum (a);
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_mullo_epi32 (a, b);
       #else
        __m128i even = _mm_mul_epu32 (a,b);
        __m128i odd = _mm_mul_epu32 (_mm_srli_si128 (a,4), _mm_srli_si128 (b,4));
        return _mm_unpacklo_epi32 (_mm_shuffle_epi32(even, _MM_SHUFFLE (0,0,2,0)),
                                   _mm_shuffle_epi32(odd,  _MM_SHUFFLE (0,0,2,0)));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_min_epi32 (a, b);
       #else
        __m128i lt = greaterThan (b, a);
        return bit_or (bit_and (lt, a), bit_andnot (lt, b));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_max_epi32 (a, b);
       #else
        __m128i gt = greaterThan (a, b);
        return bit_or (bit_and (gt, a), bit_andnot (gt, b));
       #endif
    }
};

//==============================================================================
/** Signed 64-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int64_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (int64_t, kAllBitsSet);

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE vconst (const int64_t* a) noexcept                      { return load (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE expand (int64_t s) noexcept                             { return _mm_set1_epi64x (s); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE load (const int64_t* a) noexcept                        { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m128i v, int64_t* p) noexcept                  { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi64 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi64 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline int64_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<int64_t, __m128i>::get (v, i); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, int64_t s) noexcept           { return SIMDFallbackOps<int64_t, __m128i>::set (v, i, s); }
    static forcedinline int64_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept                                { return SIMDFallbackOps<int64_t, __m128i>::sum (a); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return SIMDFallbackOps<int64_t, __m128i>::mul (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                           { return a; }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_cmpeq_epi64 (a, b);
       #else
        __m128i bitmask = _mm_cmpeq_epi32 (a, b);
        bitmask = _mm_and_si128 (bitmask, _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 3, 0, 1)));
        return _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 2, 0, 0));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_2__)
        return _mm_cmpgt_epi64 (a, b);
       #else
        return SIMDFallbackOps<int64_t, __m128i>::greaterThan (a, b);
       #endif
    }
};

//==============================================================================
/** Unsigned 64-bit integer SSE intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint64_t>
{
    //==============================================================================
    using vSIMDType = __m128i;

    //==============================================================================
    DECLARE_SSE_SIMD_CONST (uint64_t, kAllBitsSet);
    DECLARE_SSE_SIMD_CONST (uint64_t, kHighBit);

    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE vconst (const uint64_t* a) noexcept                     { return load (a); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE expand (uint64_t s) noexcept                            { return _mm_set1_epi64x ((int64_t) s); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE ssign (__m128i a) noexcept                              { return _mm_xor_si128 (a, vconst (kHighBit)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE load (const uint64_t* a) noexcept                       { return _mm_load_si128 (reinterpret_cast<const __m128i*> (a)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m128i v, uint64_t* p) noexcept                 { _mm_store_si128 (reinterpret_cast<__m128i*> (p), v); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE add (__m128i a, __m128i b) noexcept                     { return _mm_add_epi64 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE sub (__m128i a, __m128i b) noexcept                     { return _mm_sub_epi64 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_and (__m128i a, __m128i b) noexcept                 { return _mm_and_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_or  (__m128i a, __m128i b) noexcept                 { return _mm_or_si128  (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_xor (__m128i a, __m128i b) noexcept                 { return _mm_xor_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_andnot (__m128i a, __m128i b) noexcept              { return _mm_andnot_si128 (a, b); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE bit_not (__m128i a) noexcept                            { return _mm_andnot_si128 (a, vconst (kAllBitsSet)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE min (__m128i a, __m128i b) noexcept                     { __m128i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE max (__m128i a, __m128i b) noexcept                     { __m128i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m128i a, __m128i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m128i a, __m128i b, __m128i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE notEqual (__m128i a, __m128i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m128i a, __m128i b) noexcept                { return (_mm_movemask_epi8 (equal (a, b)) == 0xffff); }
    static forcedinline uint64_t JUCE_VECTOR_CALLTYPE get (__m128i v, size_t i) noexcept                      { return SIMDFallbackOps<uint64_t, __m128i>::get (v, i); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE set (__m128i v, size_t i, uint64_t s) noexcept          { return SIMDFallbackOps<uint64_t, __m128i>::set (v, i, s); }
    static forcedinline uint64_t JUCE_VECTOR_CALLTYPE sum (__m128i a) noexcept                                { return SIMDFallbackOps<uint64_t, __m128i>::sum (a); }
    static forcedinline __m128i  JUCE_VECTOR_CALLTYPE mul (__m128i a, __m128i b) noexcept                     { return SIMDFallbackOps<uint64_t, __m128i>::mul (a, b); }
    static forcedinline __m128i JUCE_VECTOR_CALLTYPE truncate (__m128i a) noexcept                            { return a; }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE equal (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_1__)
        return _mm_cmpeq_epi64 (a, b);
       #else
        __m128i bitmask = _mm_cmpeq_epi32 (a, b);
        bitmask = _mm_and_si128 (bitmask, _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 3, 0, 1)));
        return _mm_shuffle_epi32 (bitmask, _MM_SHUFFLE (2, 2, 0, 0));
       #endif
    }

    static forcedinline __m128i JUCE_VECTOR_CALLTYPE greaterThan (__m128i a, __m128i b) noexcept
    {
       #if defined(__SSE4_2__)
        return _mm_cmpgt_epi64 (ssign (a), ssign (b));
       #else
        return SIMDFallbackOps<uint64_t, __m128i>::greaterThan (a, b);
       #endif
    }
};

#endif

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace dsp
} // namespace juce
