/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
 #define DECLARE_AVX_SIMD_CONST(type, name) \
    static __declspec(align(32)) const type name[32 / sizeof (type)]

 #define DEFINE_AVX_SIMD_CONST(type, class_type, name) \
    __declspec(align(32)) const type SIMDNativeOps<class_type>:: name[32 / sizeof (type)]

#else
 #define DECLARE_AVX_SIMD_CONST(type, name) \
    static const type name[32 / sizeof (type)] __attribute__((aligned(32)))

 #define DEFINE_AVX_SIMD_CONST(type, class_type, name) \
    const type SIMDNativeOps<class_type>:: name[32 / sizeof (type)] __attribute__((aligned(32)))

#endif

template <typename type>
struct SIMDNativeOps;

//==============================================================================
/** Single-precision floating point AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<float>
{
    using vSIMDType = __m256;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int32_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (int32_t, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (float, kOne);

    //==============================================================================
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE vconst (const float* a) noexcept                     { return load (a); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE vconst (const int32_t* a) noexcept                   { return _mm256_castsi256_ps (_mm256_load_si256 (reinterpret_cast <const __m256i*> (a))); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE expand (float s) noexcept                            { return _mm256_broadcast_ss (&s); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE load (const float* a) noexcept                       { return _mm256_load_ps (a); }
    static forcedinline void   JUCE_VECTOR_CALLTYPE store (__m256 value, float* dest) noexcept           { _mm256_store_ps (dest, value); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE add (__m256 a, __m256 b) noexcept                    { return _mm256_add_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE sub (__m256 a, __m256 b) noexcept                    { return _mm256_sub_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE mul (__m256 a, __m256 b) noexcept                    { return _mm256_mul_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE bit_and (__m256 a, __m256 b) noexcept                { return _mm256_and_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE bit_or  (__m256 a, __m256 b) noexcept                { return _mm256_or_ps  (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE bit_xor (__m256 a, __m256 b) noexcept                { return _mm256_xor_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE bit_notand (__m256 a, __m256 b) noexcept             { return _mm256_andnot_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE bit_not (__m256 a) noexcept                          { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE min (__m256 a, __m256 b) noexcept                    { return _mm256_min_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE max (__m256 a, __m256 b) noexcept                    { return _mm256_max_ps (a, b); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE equal (__m256 a, __m256 b) noexcept                  { return _mm256_cmp_ps (a, b, _CMP_EQ_OQ); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE notEqual (__m256 a, __m256 b) noexcept               { return _mm256_cmp_ps (a, b, _CMP_NEQ_OQ); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE greaterThan (__m256 a, __m256 b) noexcept            { return _mm256_cmp_ps (a, b, _CMP_GT_OQ); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256 a, __m256 b) noexcept     { return _mm256_cmp_ps (a, b, _CMP_GE_OQ); }
    static forcedinline bool   JUCE_VECTOR_CALLTYPE allEqual (__m256 a, __m256 b) noexcept               { return (_mm256_movemask_ps (equal (a, b)) == 0xff); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE dupeven (__m256 a) noexcept                          { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 2, 0, 0)); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE dupodd (__m256 a) noexcept                           { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (3, 3, 1, 1)); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE swapevenodd (__m256 a) noexcept                      { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 3, 0, 1)); }
    static forcedinline float  JUCE_VECTOR_CALLTYPE get (__m256 v, size_t i) noexcept                    { return SIMDFallbackOps<float, __m256>::get (v, i); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE set (__m256 v, size_t i, float s) noexcept           { return SIMDFallbackOps<float, __m256>::set (v, i, s); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE truncate (__m256 a) noexcept                         { return _mm256_cvtepi32_ps (_mm256_cvttps_epi32 (a)); }

    static forcedinline __m256 JUCE_VECTOR_CALLTYPE multiplyAdd (__m256 a, __m256 b, __m256 c) noexcept
    {
       #if __FMA__
        return _mm256_fmadd_ps (b, c, a);
       #else
        return add (a, mul (b, c));
       #endif
    }

    static forcedinline __m256 JUCE_VECTOR_CALLTYPE oddevensum (__m256 a) noexcept
    {
        a = _mm256_add_ps (_mm256_shuffle_ps (a, a, _MM_SHUFFLE (1, 0, 3, 2)), a);
        return add (_mm256_permute2f128_ps (a, a, 1), a);
    }

    //==============================================================================
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE cmplxmul (__m256 a, __m256 b) noexcept
    {
        __m256 rr_ir = mul (a, dupeven (b));
        __m256 ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline float JUCE_VECTOR_CALLTYPE sum (__m256 a) noexcept
    {
       __m256 retval = _mm256_dp_ps (a, vconst (kOne), 0xff);
       __m256 tmp = _mm256_permute2f128_ps (retval, retval, 1);
       retval = _mm256_add_ps (retval, tmp);

      #if JUCE_GCC
       return retval[0];
      #else
       return _mm256_cvtss_f32 (retval);
      #endif
    }
};

//==============================================================================
/** Double-precision floating point AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<double>
{
    using vSIMDType = __m256d;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int64_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (int64_t, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (double, kOne);

    //==============================================================================
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE vconst (const double* a) noexcept                      { return load (a); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE vconst (const int64_t* a) noexcept                     { return _mm256_castsi256_pd (_mm256_load_si256 (reinterpret_cast <const __m256i*> (a))); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE expand (double s) noexcept                             { return _mm256_broadcast_sd (&s); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE load (const double* a) noexcept                        { return _mm256_load_pd (a); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m256d value, double* dest) noexcept              { _mm256_store_pd (dest, value); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE add (__m256d a, __m256d b) noexcept                    { return _mm256_add_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE sub (__m256d a, __m256d b) noexcept                    { return _mm256_sub_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE mul (__m256d a, __m256d b) noexcept                    { return _mm256_mul_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_and (__m256d a, __m256d b) noexcept                { return _mm256_and_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_or  (__m256d a, __m256d b) noexcept                { return _mm256_or_pd  (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_xor (__m256d a, __m256d b) noexcept                { return _mm256_xor_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_notand (__m256d a, __m256d b) noexcept             { return _mm256_andnot_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_not (__m256d a) noexcept                           { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE min (__m256d a, __m256d b) noexcept                    { return _mm256_min_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE max (__m256d a, __m256d b) noexcept                    { return _mm256_max_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE equal (__m256d a, __m256d b) noexcept                  { return _mm256_cmp_pd (a, b, _CMP_EQ_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE notEqual (__m256d a, __m256d b) noexcept               { return _mm256_cmp_pd (a, b, _CMP_NEQ_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE greaterThan (__m256d a, __m256d b) noexcept            { return _mm256_cmp_pd (a, b, _CMP_GT_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256d a, __m256d b) noexcept     { return _mm256_cmp_pd (a, b, _CMP_GE_OQ); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256d a, __m256d b) noexcept               { return (_mm256_movemask_pd (equal (a, b)) == 0xf); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE multiplyAdd (__m256d a, __m256d b, __m256d c) noexcept { return _mm256_add_pd (a, _mm256_mul_pd (b, c)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE dupeven (__m256d a) noexcept                           { return _mm256_shuffle_pd (a, a, 0); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE dupodd (__m256d a) noexcept                            { return _mm256_shuffle_pd (a, a, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE swapevenodd (__m256d a) noexcept                       { return _mm256_shuffle_pd (a, a, (1 << 0) | (0 << 1) | (1 << 2) | (0 << 3)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE oddevensum (__m256d a) noexcept                        { return _mm256_add_pd (_mm256_permute2f128_pd (a, a, 1), a); }
    static forcedinline double  JUCE_VECTOR_CALLTYPE get (__m256d v, size_t i) noexcept                     { return SIMDFallbackOps<double, __m256d>::get (v, i); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE set (__m256d v, size_t i, double s) noexcept           { return SIMDFallbackOps<double, __m256d>::set (v, i, s); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE truncate (__m256d a) noexcept                          { return _mm256_cvtepi32_pd (_mm256_cvttpd_epi32 (a)); }

    //==============================================================================
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE cmplxmul (__m256d a, __m256d b) noexcept
    {
        __m256d rr_ir = mul (a, dupeven (b));
        __m256d ii_ri = mul (swapevenodd (a), dupodd (b));
        return add (rr_ir, bit_xor (ii_ri, vconst (kEvenHighBit)));
    }

    static forcedinline double JUCE_VECTOR_CALLTYPE sum (__m256d a) noexcept
    {
        __m256d retval = _mm256_hadd_pd (a, a);
        __m256d tmp = _mm256_permute2f128_pd (retval, retval, 1);
        retval = _mm256_add_pd (retval, tmp);

       #if JUCE_GCC
        return retval[0];
       #else
        return _mm256_cvtsd_f64 (retval);
       #endif
    }
};

//==============================================================================
/** Signed 8-bit integer AVX intrinsics

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int8_t>
{
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int8_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int8_t s) noexcept                             { return _mm256_set1_epi8 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE load (const int8_t* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m256i value, int8_t* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                    { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                    { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept             { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                           { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                    { return _mm256_min_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                    { return _mm256_max_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                  { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept            { return _mm256_cmpgt_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept     { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept               { return _mm256_movemask_epi8 (equal (a, b)) == -1; }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept               { return bit_not (equal (a, b)); }
    static forcedinline int8_t  JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                     { return SIMDFallbackOps<int8_t, __m256i>::get (v, i); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, int8_t s) noexcept           { return SIMDFallbackOps<int8_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                          { return a; }

    //==============================================================================
    static forcedinline int8_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i lo = _mm256_unpacklo_epi8 (a, _mm256_setzero_si256());
        __m256i hi = _mm256_unpackhi_epi8 (a, _mm256_setzero_si256());

        for (int i = 0; i < 3; ++i)
        {
            lo = _mm256_hadd_epi16 (lo, lo);
            hi = _mm256_hadd_epi16 (hi, hi);
        }

       #if JUCE_GCC
        return (int8_t) ((lo[0] & 0xff) +
                         (hi[0] & 0xff) +
                         (lo[2] & 0xff) +
                         (hi[2] & 0xff));
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (int8_t) ((_mm256_cvtsi256_si32 (lo) & 0xff) +
                         (_mm256_cvtsi256_si32 (hi) & 0xff) +
                         (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (lo, mask)) & 0xff) +
                         (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (hi, mask)) & 0xff));
       #endif
    }

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b)
    {
        // unpack and multiply
        __m256i even = _mm256_mullo_epi16 (a, b);
        __m256i odd  = _mm256_mullo_epi16 (_mm256_srli_epi16 (a, 8), _mm256_srli_epi16 (b, 8));

        return _mm256_or_si256 (_mm256_slli_epi16 (odd, 8),
                             _mm256_srli_epi16 (_mm256_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Unsigned 8-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint8_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint8_t, kHighBit);
    DECLARE_AVX_SIMD_CONST (uint8_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (uint8_t s) noexcept                             { return _mm256_set1_epi8 ((int8_t) s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE load (const uint8_t* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m256i value, uint8_t* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi8 (ssign (a), ssign (b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline uint8_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<uint8_t, __m256i>::get (v, i); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, uint8_t s) noexcept           { return SIMDFallbackOps<uint8_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline uint8_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i lo = _mm256_unpacklo_epi8 (a, _mm256_setzero_si256());
        __m256i hi = _mm256_unpackhi_epi8 (a, _mm256_setzero_si256());

        for (int i = 0; i < 3; ++i)
        {
            lo = _mm256_hadd_epi16 (lo, lo);
            hi = _mm256_hadd_epi16 (hi, hi);
        }

       #if JUCE_GCC
        return (uint8_t) ((static_cast<uint32_t> (lo[0]) & 0xffu) +
                          (static_cast<uint32_t> (hi[0]) & 0xffu) +
                          (static_cast<uint32_t> (lo[2]) & 0xffu) +
                          (static_cast<uint32_t> (hi[2]) & 0xffu));
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (uint8_t) ((static_cast<uint32_t> (_mm256_cvtsi256_si32 (lo)) & 0xffu) +
                          (static_cast<uint32_t> (_mm256_cvtsi256_si32 (hi)) & 0xffu) +
                          (static_cast<uint32_t> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (lo, mask))) & 0xffu) +
                          (static_cast<uint32_t> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (hi, mask))) & 0xffu));
       #endif
    }

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b)
    {
        // unpack and multiply
        __m256i even = _mm256_mullo_epi16 (a, b);
        __m256i odd  = _mm256_mullo_epi16 (_mm256_srli_epi16 (a, 8), _mm256_srli_epi16 (b, 8));

        return _mm256_or_si256 (_mm256_slli_epi16 (odd, 8),
                             _mm256_srli_epi16 (_mm256_slli_epi16 (even, 8), 8));
    }
};

//==============================================================================
/** Signed 16-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int16_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int16_t s) noexcept                             { return _mm256_set1_epi16 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE load (const int16_t* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void JUCE_VECTOR_CALLTYPE store (__m256i value, int16_t* dest) noexcept              { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline int16_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<int16_t, __m256i>::get (v, i); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, int16_t s) noexcept           { return SIMDFallbackOps<int16_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline int16_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);

       #if JUCE_GCC
        return (int16_t) ((tmp[0] & 0xffff) + (tmp[2] & 0xffff));
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (int16_t) ((_mm256_cvtsi256_si32 (tmp) & 0xffff) +
                          (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask)) & 0xffff));
       #endif
    }
};

//==============================================================================
/** Unsigned 16-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint16_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint16_t, kHighBit);
    DECLARE_AVX_SIMD_CONST (uint16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE expand (uint16_t s) noexcept                            { return _mm256_set1_epi16 ((int16_t) s); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE load (const uint16_t* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m256i value, uint16_t* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (ssign (a), ssign (b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline uint16_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<uint16_t, __m256i>::get (v, i); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, uint16_t s) noexcept          { return SIMDFallbackOps<uint16_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline uint16_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);

       #if JUCE_GCC
        return (uint16_t) ((static_cast<uint32_t> (tmp[0]) & 0xffffu) +
                           (static_cast<uint32_t> (tmp[2]) & 0xffffu));
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return (uint16_t) ((static_cast<uint32_t> (_mm256_cvtsi256_si32 (tmp)) & 0xffffu) +
                           (static_cast<uint32_t> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask))) & 0xffffu));
       #endif
    }
};

//==============================================================================
/** Signed 32-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int32_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int32_t s) noexcept                             { return _mm256_set1_epi32 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE load (const int32_t* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m256i value, int32_t* dest) noexcept           { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline int32_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<int32_t, __m256i>::get (v, i); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, int32_t s) noexcept           { return SIMDFallbackOps<int32_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }

    //==============================================================================
    static forcedinline int32_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);

       #if JUCE_GCC
        return (int32_t) (tmp[0] + tmp[2]);
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return _mm256_cvtsi256_si32 (tmp) + _mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask));
       #endif
    }
};

//==============================================================================
/** Unsigned 32-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint32_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint32_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (uint32_t, kHighBit);

    //==============================================================================
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE expand (uint32_t s) noexcept                            { return _mm256_set1_epi32 ((int32_t) s); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE load (const uint32_t* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m256i value, uint32_t* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (ssign (a), ssign (b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline uint32_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<uint32_t, __m256i>::get (v, i); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, uint32_t s) noexcept          { return SIMDFallbackOps<uint32_t, __m256i>::set (v, i, s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }

    //==============================================================================
    static forcedinline uint32_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);

       #if JUCE_GCC
        return static_cast<uint32_t> (tmp[0]) + static_cast<uint32_t> (tmp[2]);
       #else
        constexpr int mask = (2 << 0) | (3 << 2) | (0 << 4) | (1 << 6);

        return static_cast<uint32_t> (_mm256_cvtsi256_si32 (tmp))
            + static_cast<uint32_t> (_mm256_cvtsi256_si32 (_mm256_permute4x64_epi64 (tmp, mask)));
       #endif
    }
};

//==============================================================================
/** Signed 64-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<int64_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int64_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int64_t s) noexcept                             { return _mm256_set1_epi64x ((int64_t) s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE load (const int64_t* p) noexcept                        { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void    JUCE_VECTOR_CALLTYPE store (__m256i value, int64_t* dest) noexcept           { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool    JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline int64_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<int64_t, __m256i>::get (v, i); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, int64_t s) noexcept           { return SIMDFallbackOps<int64_t, __m256i>::set (v, i, s); }
    static forcedinline int64_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept                                { return SIMDFallbackOps<int64_t, __m256i>::sum (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return SIMDFallbackOps<int64_t, __m256i>::mul (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                           { return a; }
};

//==============================================================================
/** Unsigned 64-bit integer AVX intrinsics.

    @tags{DSP}
*/
template <>
struct SIMDNativeOps<uint64_t>
{
    //==============================================================================
    using vSIMDType = __m256i;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint64_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (uint64_t, kHighBit);

    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE expand (uint64_t s) noexcept                            { return _mm256_set1_epi64x ((int64_t) s); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE load (const uint64_t* p) noexcept                       { return _mm256_load_si256 (reinterpret_cast<const __m256i*> (p)); }
    static forcedinline void     JUCE_VECTOR_CALLTYPE store (__m256i value, uint64_t* dest) noexcept          { _mm256_store_si256 (reinterpret_cast<__m256i*> (dest), value); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, load (kHighBit)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, load (kAllBitsSet)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (ssign (a), ssign (b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }
    static forcedinline bool     JUCE_VECTOR_CALLTYPE allEqual (__m256i a, __m256i b) noexcept                { return (_mm256_movemask_epi8 (equal (a, b)) == -1); }
    static forcedinline uint64_t JUCE_VECTOR_CALLTYPE get (__m256i v, size_t i) noexcept                      { return SIMDFallbackOps<uint64_t, __m256i>::get (v, i); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE set (__m256i v, size_t i, uint64_t s) noexcept          { return SIMDFallbackOps<uint64_t, __m256i>::set (v, i, s); }
    static forcedinline uint64_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept                                { return SIMDFallbackOps<uint64_t, __m256i>::sum (a); }
    static forcedinline __m256i  JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return SIMDFallbackOps<uint64_t, __m256i>::mul (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE truncate (__m256i a) noexcept                            { return a; }
};

#endif

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace dsp
} // namespace juce
