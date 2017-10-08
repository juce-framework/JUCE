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
/** Single-precision floating point AVX intrinsics. */
template <>
struct SIMDNativeOps<float>
{
    typedef __m256 vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int32_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (int32_t, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (float, kOne);

    //==============================================================================
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE vconst (const float* a) noexcept                     { return *reinterpret_cast<const __m256*> (a); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE vconst (const int32_t* a) noexcept                   { return *reinterpret_cast<const __m256*> (a); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE expand (float s) noexcept                            { return _mm256_broadcast_ss (&s); }
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
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE multiplyAdd (__m256 a, __m256 b, __m256 c) noexcept  { return _mm256_fmadd_ps (b, c, a); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE dupeven (__m256 a) noexcept                          { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 2, 0, 0)); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE dupodd (__m256 a) noexcept                           { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (3, 3, 1, 1)); }
    static forcedinline __m256 JUCE_VECTOR_CALLTYPE swapevenodd (__m256 a) noexcept                      { return _mm256_shuffle_ps (a, a, _MM_SHUFFLE (2, 3, 0, 1)); }
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
       return ((float*) &retval)[0];
    }
};

//==============================================================================
/** Double-precision floating point AVX intrinsics. */
template <>
struct SIMDNativeOps<double>
{
    typedef __m256d vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int64_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (int64_t, kEvenHighBit);
    DECLARE_AVX_SIMD_CONST (double, kOne);

    //==============================================================================
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE vconst (const double* a) noexcept                       { return *reinterpret_cast<const __m256d*> (a); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE vconst (const int64_t* a) noexcept                      { return *reinterpret_cast<const __m256d*> (a); }
    static forcedinline __m256d expand (double s) noexcept                              { return _mm256_broadcast_sd (&s); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE add (__m256d a, __m256d b) noexcept                     { return _mm256_add_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE sub (__m256d a, __m256d b) noexcept                     { return _mm256_sub_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE mul (__m256d a, __m256d b) noexcept                     { return _mm256_mul_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_and (__m256d a, __m256d b) noexcept                 { return _mm256_and_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_or  (__m256d a, __m256d b) noexcept                 { return _mm256_or_pd  (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_xor (__m256d a, __m256d b) noexcept                 { return _mm256_xor_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE bit_notand (__m256d a, __m256d b) noexcept              { return _mm256_andnot_pd (a, b); }
    static forcedinline __m256d bit_not (__m256d a) noexcept                            { return bit_notand (a, vconst (kAllBitsSet)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE min (__m256d a, __m256d b) noexcept                     { return _mm256_min_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE max (__m256d a, __m256d b) noexcept                     { return _mm256_max_pd (a, b); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE equal (__m256d a, __m256d b) noexcept                   { return _mm256_cmp_pd (a, b, _CMP_EQ_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE notEqual (__m256d a, __m256d b) noexcept                { return _mm256_cmp_pd (a, b, _CMP_NEQ_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE greaterThan (__m256d a, __m256d b) noexcept             { return _mm256_cmp_pd (a, b, _CMP_GT_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256d a, __m256d b) noexcept      { return _mm256_cmp_pd (a, b, _CMP_GE_OQ); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE multiplyAdd (__m256d a, __m256d b, __m256d c) noexcept  { return _mm256_add_pd (a, _mm256_mul_pd (b, c)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE dupeven (__m256d a) noexcept                            { return _mm256_shuffle_pd (a, a, 0); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE dupodd (__m256d a) noexcept                             { return _mm256_shuffle_pd (a, a, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE swapevenodd (__m256d a) noexcept                        { return _mm256_shuffle_pd (a, a, (1 << 0) | (0 << 1) | (1 << 2) | (0 << 3)); }
    static forcedinline __m256d JUCE_VECTOR_CALLTYPE oddevensum (__m256d a) noexcept                         { return _mm256_add_pd (_mm256_permute2f128_pd (a, a, 1), a); }

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
        return ((double*) &retval)[0];
    }
};

//==============================================================================
/** Signed 8-bit integer AVX intrinsics */
template <>
struct SIMDNativeOps<int8_t>
{
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int8_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const int8_t* a) noexcept                       { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int8_t s) noexcept                              { return _mm256_set1_epi8 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

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

        const int8_t* lo_ptr = reinterpret_cast<const int8_t*> (&lo);
        const int8_t* hi_ptr = reinterpret_cast<const int8_t*> (&hi);

        return (int8_t) (lo_ptr[0] + hi_ptr[0] + lo_ptr[16] + hi_ptr[16]);
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
/** Unsigned 8-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<uint8_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint8_t, kHighBit);
    DECLARE_AVX_SIMD_CONST (uint8_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const uint8_t* a) noexcept                      { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, vconst (kHighBit)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (uint8_t s) noexcept                             { return _mm256_set1_epi8 ((int8_t) s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi8 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi8 (ssign (a), ssign (b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

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

        const uint8_t* lo_ptr = reinterpret_cast<const uint8_t*> (&lo);
        const uint8_t* hi_ptr = reinterpret_cast<const uint8_t*> (&hi);

        return (uint8_t) (lo_ptr[0] + hi_ptr[0] + lo_ptr[16] + hi_ptr[16]);
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
/** Signed 16-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<int16_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const int16_t* a) noexcept                      { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int16_t s) noexcept                             { return _mm256_set1_epi16 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline int16_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        int16_t* ptr = reinterpret_cast<int16_t*> (&tmp);
        return (int16_t) (ptr[0] + ptr[8]);
    }
};

//==============================================================================
/** Unsigned 16-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<uint16_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint16_t, kHighBit);
    DECLARE_AVX_SIMD_CONST (uint16_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const uint16_t* a) noexcept                     { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, vconst (kHighBit)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (uint16_t s) noexcept                            { return _mm256_set1_epi16 ((int16_t) s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi16 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi16 (ssign (a), ssign (b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline uint16_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi16 (a, a);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        tmp = _mm256_hadd_epi16 (tmp, tmp);
        uint16_t* ptr = reinterpret_cast<uint16_t*> (&tmp);
        return (uint16_t) (ptr[0] + ptr[8]);
    }
};

//==============================================================================
/** Signed 32-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<int32_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int32_t, kAllBitsSet);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const int32_t* a) noexcept                      { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int32_t s) noexcept                             { return _mm256_set1_epi32 (s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline int32_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);
        int32_t* ptr = reinterpret_cast<int32_t*> (&tmp);
        return ptr[0] + ptr[4];
    }
};

//==============================================================================
/** Unsigned 32-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<uint32_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint32_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (uint32_t, kHighBit);

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const uint32_t* a) noexcept                     { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, vconst (kHighBit)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (uint32_t s) noexcept                            { return _mm256_set1_epi32 ((int32_t) s); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept                     { return _mm256_mullo_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { return _mm256_min_epu32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { return _mm256_max_epu32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi32 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi32 (ssign (a), ssign (b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline uint32_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        __m256i tmp = _mm256_hadd_epi32 (a, a);
        tmp = _mm256_hadd_epi32 (tmp, tmp);
        uint32_t* ptr = reinterpret_cast<uint32_t*> (&tmp);
        return ptr[0] + ptr[4];
    }
};

//==============================================================================
/** Signed 64-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<int64_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (int64_t, kAllBitsSet);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const int64_t* a) noexcept                      { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (int64_t s) noexcept
    {
       #ifdef _MSC_VER
        __m256d tmp = _mm256_broadcast_sd (reinterpret_cast<const double*> (&s));
        return *reinterpret_cast<const __m256i*> (&tmp);
       #else
        return _mm256_set1_epi64x ((int64_t) s);
       #endif
    }

    static forcedinline int64_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        const int64_t* ptr = reinterpret_cast<const int64_t*> (&a);
        return ptr[0] + ptr[1] + ptr[2] + ptr[3];
    }

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept
    {
        __m256i retval;

        const int64_t* aptr = reinterpret_cast<const int64_t*> (&a);
        const int64_t* bptr = reinterpret_cast<const int64_t*> (&b);
        int64_t* dst =  reinterpret_cast<int64_t*> (&retval);

        for (int i = 0; i < 4; ++i)
            dst[i] = aptr[i] * bptr[i];

        return retval;
    }
};

//==============================================================================
/** Unsigned 64-bit integer AVX intrinsics. */
template <>
struct SIMDNativeOps<uint64_t>
{
    //==============================================================================
    typedef __m256i vSIMDType;

    //==============================================================================
    DECLARE_AVX_SIMD_CONST (uint64_t, kAllBitsSet);
    DECLARE_AVX_SIMD_CONST (uint64_t, kHighBit);

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE vconst (const uint64_t* a) noexcept                     { return *reinterpret_cast<const __m256i*> (a); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE ssign (__m256i a) noexcept                              { return _mm256_xor_si256 (a, vconst (kHighBit)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE add (__m256i a, __m256i b) noexcept                     { return _mm256_add_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE sub (__m256i a, __m256i b) noexcept                     { return _mm256_sub_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_and (__m256i a, __m256i b) noexcept                 { return _mm256_and_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_or  (__m256i a, __m256i b) noexcept                 { return _mm256_or_si256  (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_xor (__m256i a, __m256i b) noexcept                 { return _mm256_xor_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_andnot (__m256i a, __m256i b) noexcept              { return _mm256_andnot_si256 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE bit_not (__m256i a) noexcept                            { return _mm256_andnot_si256 (a, vconst (kAllBitsSet)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE min (__m256i a, __m256i b) noexcept                     { __m256i lt = greaterThan (b, a); return bit_or (bit_and (lt, a), bit_andnot (lt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE max (__m256i a, __m256i b) noexcept                     { __m256i gt = greaterThan (a, b); return bit_or (bit_and (gt, a), bit_andnot (gt, b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE equal (__m256i a, __m256i b) noexcept                   { return _mm256_cmpeq_epi64 (a, b); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThan (__m256i a, __m256i b) noexcept             { return _mm256_cmpgt_epi64 (ssign (a), ssign (b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE greaterThanOrEqual (__m256i a, __m256i b) noexcept      { return bit_or (greaterThan (a, b), equal (a,b)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE multiplyAdd (__m256i a, __m256i b, __m256i c) noexcept  { return add (a, mul (b, c)); }
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE notEqual (__m256i a, __m256i b) noexcept                { return bit_not (equal (a, b)); }

    //==============================================================================
    static forcedinline __m256i JUCE_VECTOR_CALLTYPE expand (uint64_t s) noexcept
    {
       #ifdef _MSC_VER
        __m256d tmp = _mm256_broadcast_sd (reinterpret_cast<const double*> (&s));
        return *reinterpret_cast<const __m256i*> (&tmp);
       #else
        return _mm256_set1_epi64x ((int64_t) s);
       #endif
    }

    static forcedinline uint64_t JUCE_VECTOR_CALLTYPE sum (__m256i a) noexcept
    {
        const uint64_t* ptr = reinterpret_cast<const uint64_t*> (&a);
        return ptr[0] + ptr[1] + ptr[2] + ptr[3];
    }

    static forcedinline __m256i JUCE_VECTOR_CALLTYPE mul (__m256i a, __m256i b) noexcept
    {
        __m256i retval;

        const uint64_t* aptr = reinterpret_cast<const uint64_t*> (&a);
        const uint64_t* bptr = reinterpret_cast<const uint64_t*> (&b);
        uint64_t* dst =  reinterpret_cast<uint64_t*> (&retval);

        for (int i = 0; i < 4; ++i)
            dst[i] = aptr[i] * bptr[i];

        return retval;
    }
};

#endif

} // namespace dsp
} // namespace juce
