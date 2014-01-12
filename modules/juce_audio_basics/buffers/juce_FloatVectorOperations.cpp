/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace FloatVectorHelpers
{

    #define JUCE_INCREMENT_SRC_DEST    dest += 4; src += 4;
    #define JUCE_INCREMENT_DEST        dest += 4;

   #if JUCE_USE_SSE_INTRINSICS
    static bool sse2Present = false;

    static bool isSSE2Available() noexcept
    {
        if (sse2Present)
            return true;

        sse2Present = SystemStats::hasSSE2();
        return sse2Present;
    }

    inline static bool isAligned (const void* p) noexcept
    {
        return (((pointer_sized_int) p) & 15) == 0;
    }

    static inline float findMinimumOrMaximum (const float* src, int num, const bool isMinimum) noexcept
    {
        const int numLongOps = num / 4;

        if (numLongOps > 1 && FloatVectorHelpers::isSSE2Available())
        {
            __m128 val;

            #define JUCE_MINIMUMMAXIMUM_SSE_LOOP(loadOp, minMaxOp) \
                val = loadOp (src); \
                src += 4; \
                for (int i = 1; i < numLongOps; ++i) \
                { \
                    const __m128 s = loadOp (src); \
                    val = minMaxOp (val, s); \
                    src += 4; \
                }

            if (isMinimum)
            {
                if (FloatVectorHelpers::isAligned (src)) { JUCE_MINIMUMMAXIMUM_SSE_LOOP (_mm_load_ps,  _mm_min_ps) }
                else                                     { JUCE_MINIMUMMAXIMUM_SSE_LOOP (_mm_loadu_ps, _mm_min_ps) }
            }
            else
            {
                if (FloatVectorHelpers::isAligned (src)) { JUCE_MINIMUMMAXIMUM_SSE_LOOP (_mm_load_ps, _mm_max_ps) }
                else                                     { JUCE_MINIMUMMAXIMUM_SSE_LOOP (_mm_loadu_ps,_mm_max_ps) }
            }

            float localVal;

            {
                float vals[4];
                _mm_storeu_ps (vals, val);

                localVal = isMinimum ? jmin (vals[0], vals[1], vals[2], vals[3])
                                     : jmax (vals[0], vals[1], vals[2], vals[3]);
            }

            num &= 3;

            for (int i = 0; i < num; ++i)
                localVal = isMinimum ? jmin (localVal, src[i])
                                     : jmax (localVal, src[i]);

            return localVal;
        }

        return isMinimum ? juce::findMinimum (src, num)
                         : juce::findMaximum (src, num);
    }

    #define JUCE_BEGIN_SSE_OP \
        if (FloatVectorHelpers::isSSE2Available()) \
        { \
            const int numLongOps = num / 4;

    #define JUCE_FINISH_SSE_OP(normalOp) \
            num &= 3; \
            if (num == 0) return; \
        } \
        for (int i = 0; i < num; ++i) normalOp;

    #define JUCE_SSE_LOOP(sseOp, srcLoad, dstLoad, dstStore, locals, increment) \
        for (int i = 0; i < numLongOps; ++i) \
        { \
            locals (srcLoad, dstLoad); \
            dstStore (dest, sseOp); \
            increment; \
        }

    #define JUCE_LOAD_NONE(srcLoad, dstLoad)
    #define JUCE_LOAD_DEST(srcLoad, dstLoad)     const __m128 d = dstLoad (dest);
    #define JUCE_LOAD_SRC(srcLoad, dstLoad)      const __m128 s = srcLoad (src);
    #define JUCE_LOAD_SRC_DEST(srcLoad, dstLoad) const __m128 d = dstLoad (dest); const __m128 s = srcLoad (src);

    #define JUCE_PERFORM_SSE_OP_DEST(normalOp, sseOp, locals) \
        JUCE_BEGIN_SSE_OP \
        if (FloatVectorHelpers::isAligned (dest))   JUCE_SSE_LOOP (sseOp, dummy, _mm_load_ps,  _mm_store_ps,  locals, JUCE_INCREMENT_DEST) \
        else                                        JUCE_SSE_LOOP (sseOp, dummy, _mm_loadu_ps, _mm_storeu_ps, locals, JUCE_INCREMENT_DEST) \
        JUCE_FINISH_SSE_OP (normalOp)

    #define JUCE_PERFORM_SSE_OP_SRC_DEST(normalOp, sseOp, locals, increment) \
        JUCE_BEGIN_SSE_OP \
        if (FloatVectorHelpers::isAligned (dest)) \
        { \
            if (FloatVectorHelpers::isAligned (src)) JUCE_SSE_LOOP (sseOp, _mm_load_ps,  _mm_load_ps, _mm_store_ps, locals, increment) \
            else                                     JUCE_SSE_LOOP (sseOp, _mm_loadu_ps, _mm_load_ps, _mm_store_ps, locals, increment) \
        }\
        else \
        { \
            if (FloatVectorHelpers::isAligned (src)) JUCE_SSE_LOOP (sseOp, _mm_load_ps,  _mm_loadu_ps, _mm_storeu_ps, locals, increment) \
            else                                     JUCE_SSE_LOOP (sseOp, _mm_loadu_ps, _mm_loadu_ps, _mm_storeu_ps, locals, increment) \
        } \
        JUCE_FINISH_SSE_OP (normalOp)


    //==============================================================================
   #elif JUCE_USE_ARM_NEON

    static inline float findMinimumOrMaximum (const float* src, int num, const bool isMinimum) noexcept
    {
        const int numLongOps = num / 4;

        if (numLongOps > 1)
        {
            float32x4_t val;

            #define JUCE_MINIMUMMAXIMUM_NEON_LOOP(loadOp, minMaxOp) \
                val = loadOp (src); \
                src += 4; \
                for (int i = 1; i < numLongOps; ++i) \
                { \
                    const float32x4_t s = loadOp (src); \
                    val = minMaxOp (val, s); \
                    src += 4; \
                }

            if (isMinimum) { JUCE_MINIMUMMAXIMUM_NEON_LOOP (vld1q_f32, vminq_f32) }
            else           { JUCE_MINIMUMMAXIMUM_NEON_LOOP (vld1q_f32, vmaxq_f32) }

            float localVal;

            {
                float vals[4];
                vst1q_f32 (vals, val);

                localVal = isMinimum ? jmin (vals[0], vals[1], vals[2], vals[3])
                                     : jmax (vals[0], vals[1], vals[2], vals[3]);
            }

            num &= 3;

            for (int i = 0; i < num; ++i)
                localVal = isMinimum ? jmin (localVal, src[i])
                                     : jmax (localVal, src[i]);

            return localVal;
        }

        return isMinimum ? juce::findMinimum (src, num)
                         : juce::findMaximum (src, num);
    }

    #define JUCE_BEGIN_NEON_OP \
        const int numLongOps = num / 4;

    #define JUCE_FINISH_NEON_OP(normalOp) \
        num &= 3; \
        if (num == 0) return; \
        for (int i = 0; i < num; ++i) normalOp;

    #define JUCE_NEON_LOOP(neonOp, srcLoad, dstLoad, dstStore, locals, increment) \
        for (int i = 0; i < numLongOps; ++i) \
        { \
            locals (srcLoad, dstLoad); \
            dstStore (dest, neonOp); \
            increment; \
        }

    #define JUCE_LOAD_NONE(srcLoad, dstLoad)
    #define JUCE_LOAD_DEST(srcLoad, dstLoad)     const float32x4_t d = dstLoad (dest);
    #define JUCE_LOAD_SRC(srcLoad, dstLoad)      const float32x4_t s = srcLoad (src);
    #define JUCE_LOAD_SRC_DEST(srcLoad, dstLoad) const float32x4_t d = dstLoad (dest); const float32x4_t s = srcLoad (src);

    #define JUCE_PERFORM_NEON_OP_DEST(normalOp, neonOp, locals) \
        JUCE_BEGIN_NEON_OP \
        JUCE_NEON_LOOP (neonOp, dummy, vld1q_f32,  vst1q_f32,  locals, JUCE_INCREMENT_DEST) \
        JUCE_FINISH_NEON_OP (normalOp)

    #define JUCE_PERFORM_NEON_OP_SRC_DEST(normalOp, neonOp, locals) \
        JUCE_BEGIN_NEON_OP \
        JUCE_NEON_LOOP (neonOp, vld1q_f32, vld1q_f32,  vst1q_f32,  locals, JUCE_INCREMENT_SRC_DEST) \
        JUCE_FINISH_NEON_OP (normalOp)

    //==============================================================================
    #else
     #define JUCE_PERFORM_SSE_OP_DEST(normalOp, unused1, unused2)              for (int i = 0; i < num; ++i) normalOp;
     #define JUCE_PERFORM_SSE_OP_SRC_DEST(normalOp, sseOp, locals, increment)  for (int i = 0; i < num; ++i) normalOp;
    #endif
}

//==============================================================================
void JUCE_CALLTYPE FloatVectorOperations::clear (float* dest, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vclr (dest, 1, (size_t) num);
   #else
    zeromem (dest, num * sizeof (float));
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::fill (float* dest, float valueToFill, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vfill (&valueToFill, dest, 1, (size_t) num);
   #elif JUCE_USE_ARM_NEON
    const float32x4_t val = vld1q_dup_f32 (&valueToFill);
    JUCE_PERFORM_NEON_OP_DEST (dest[i] = valueToFill, val, JUCE_LOAD_NONE)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 val = _mm_load1_ps (&valueToFill);
    #endif
    JUCE_PERFORM_SSE_OP_DEST (dest[i] = valueToFill, val, JUCE_LOAD_NONE)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::copy (float* dest, const float* src, int num) noexcept
{
    memcpy (dest, src, (size_t) num * sizeof (float));
}

void JUCE_CALLTYPE FloatVectorOperations::copyWithMultiply (float* dest, const float* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vsmul (src, 1, &multiplier, dest, 1, num);
   #elif JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] += src[i], vmulq_n_f32(s, multiplier), JUCE_LOAD_SRC)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier, _mm_mul_ps (mult, s),
                                  JUCE_LOAD_SRC, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::add (float* dest, float amount, int num) noexcept
{
   #if JUCE_USE_ARM_NEON
    const float32x4_t amountToAdd = vld1q_dup_f32(&amount);
    JUCE_PERFORM_NEON_OP_DEST (dest[i] += amount, vaddq_f32 (d, amountToAdd), JUCE_LOAD_DEST)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 amountToAdd = _mm_load1_ps (&amount);
    #endif
    JUCE_PERFORM_SSE_OP_DEST (dest[i] += amount, _mm_add_ps (d, amountToAdd), JUCE_LOAD_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::add (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vadd (src, 1, dest, 1, dest, 1, num);
   #elif JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] += src[i], vaddq_f32 (d, s), JUCE_LOAD_SRC_DEST)
   #else
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i], _mm_add_ps (d, s), JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::subtract (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vsub (src, 1, dest, 1, dest, 1, num);
   #elif JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] -= src[i], vsubq_f32 (d, s), JUCE_LOAD_SRC_DEST)
   #else
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] -= src[i], _mm_sub_ps (d, s), JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::addWithMultiply (float* dest, const float* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] += src[i] * multiplier,
                                   vmlaq_n_f32 (d, s, multiplier),
                                   JUCE_LOAD_SRC_DEST)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif

     JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i] * multiplier,
                                   _mm_add_ps (d, _mm_mul_ps (mult, s)),
                                   JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif

}

void JUCE_CALLTYPE FloatVectorOperations::multiply (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vmul (src, 1, dest, 1, dest, 1, num);
   #elif JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] *= src[i], vmulq_f32 (d, s), JUCE_LOAD_SRC_DEST)
   #else
    JUCE_PERFORM_SSE_OP_SRC_DEST  (dest[i] *= src[i], _mm_mul_ps (d, s), JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::multiply (float* dest, float multiplier, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vsmul (dest, 1, &multiplier, dest, 1, num);
   #elif JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_DEST (dest[i] *= multiplier, vmulq_n_f32 (d, multiplier), JUCE_LOAD_DEST)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif
    JUCE_PERFORM_SSE_OP_DEST (dest[i] *= multiplier, _mm_mul_ps (d, mult), JUCE_LOAD_DEST)
   #endif
}

void FloatVectorOperations::negate (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vneg ((float*) src, 1, dest, 1, (vDSP_Length) num);
   #else
    copyWithMultiply (dest, src, -1.0f, num);
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::convertFixedToFloat (float* dest, const int* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_ARM_NEON
    JUCE_PERFORM_NEON_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                   vmulq_n_f32 (vcvtq_f32_s32 (vld1q_s32 (src)), multiplier),
                                   JUCE_LOAD_NONE)
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif

     JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                   _mm_mul_ps (mult, _mm_cvtepi32_ps (_mm_loadu_si128 ((const __m128i*) src))),
                                   JUCE_LOAD_NONE, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::findMinAndMax (const float* src, int num, float& minResult, float& maxResult) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const int numLongOps = num / 4;

    if (numLongOps > 1 && FloatVectorHelpers::isSSE2Available())
    {
        __m128 mn, mx;

        #define JUCE_MINMAX_SSE_LOOP(loadOp) \
            mn = loadOp (src); \
            mx = mn; \
            src += 4; \
            for (int i = 1; i < numLongOps; ++i) \
            { \
                const __m128 s = loadOp (src); \
                mn = _mm_min_ps (mn, s); \
                mx = _mm_max_ps (mx, s); \
                src += 4; \
            }

        if (FloatVectorHelpers::isAligned (src)) { JUCE_MINMAX_SSE_LOOP (_mm_load_ps) }
        else                                     { JUCE_MINMAX_SSE_LOOP (_mm_loadu_ps) }

        float localMin, localMax;

        {
            float mns[4], mxs[4];
            _mm_storeu_ps (mns, mn);
            _mm_storeu_ps (mxs, mx);

            localMin = jmin (mns[0], mns[1], mns[2], mns[3]);
            localMax = jmax (mxs[0], mxs[1], mxs[2], mxs[3]);
        }

        num &= 3;

        for (int i = 0; i < num; ++i)
        {
            const float s = src[i];
            localMin = jmin (localMin, s);
            localMax = jmax (localMax, s);
        }

        minResult = localMin;
        maxResult = localMax;
        return;
    }
   #elif JUCE_USE_ARM_NEON
    const int numLongOps = num / 4;

    if (numLongOps > 1)
    {
        float32x4_t mn, mx;

        #define JUCE_MINMAX_NEON_LOOP(loadOp) \
            mn = loadOp (src); \
            mx = mn; \
            src += 4; \
            for (int i = 1; i < numLongOps; ++i) \
            { \
                const float32x4_t s = loadOp (src); \
                mn = vminq_f32 (mn, s); \
                mx = vmaxq_f32 (mx, s); \
                src += 4; \
            }

        JUCE_MINMAX_NEON_LOOP (vld1q_f32);

        float localMin, localMax;

        {
            float mns[4], mxs[4];
            vst1q_f32 (mns, mn);
            vst1q_f32 (mxs, mx);

            localMin = jmin (mns[0], mns[1], mns[2], mns[3]);
            localMax = jmax (mxs[0], mxs[1], mxs[2], mxs[3]);
        }

        num &= 3;

        for (int i = 0; i < num; ++i)
        {
            const float s = src[i];
            localMin = jmin (localMin, s);
            localMax = jmax (localMax, s);
        }

        minResult = localMin;
        maxResult = localMax;
        return;
    }
   #endif

    juce::findMinAndMax (src, num, minResult, maxResult);
}

float JUCE_CALLTYPE FloatVectorOperations::findMinimum (const float* src, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS || JUCE_USE_ARM_NEON
    return FloatVectorHelpers::findMinimumOrMaximum (src, num, true);
   #else
    return juce::findMinimum (src, num);
   #endif
}

float JUCE_CALLTYPE FloatVectorOperations::findMaximum (const float* src, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS || JUCE_USE_ARM_NEON
    return FloatVectorHelpers::findMinimumOrMaximum (src, num, false);
   #else
    return juce::findMaximum (src, num);
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::enableFlushToZeroMode (bool shouldEnable) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    if (FloatVectorHelpers::isSSE2Available())
        _MM_SET_FLUSH_ZERO_MODE (shouldEnable ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF);
   #endif
    (void) shouldEnable;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class FloatVectorOperationsTests  : public UnitTest
{
public:
    FloatVectorOperationsTests() : UnitTest ("FloatVectorOperations") {}

    void runTest()
    {
        beginTest ("FloatVectorOperations");

        for (int i = 100; --i >= 0;)
        {
            const int num = getRandom().nextInt (500) + 1;

            HeapBlock<float> buffer1 (num + 16), buffer2 (num + 16);
            HeapBlock<int> buffer3 (num + 16);

           #if JUCE_ARM
            float* const data1 = buffer1;
            float* const data2 = buffer2;
            int* const int1 = buffer3;
           #else
            float* const data1 = addBytesToPointer (buffer1.getData(), getRandom().nextInt (16));
            float* const data2 = addBytesToPointer (buffer2.getData(), getRandom().nextInt (16));
            int* const int1 = addBytesToPointer (buffer3.getData(), getRandom().nextInt (16));
           #endif

            fillRandomly (data1, num);
            fillRandomly (data2, num);

            float mn1, mx1, mn2, mx2;
            FloatVectorOperations::findMinAndMax (data1, num, mn1, mx1);
            juce::findMinAndMax (data1, num, mn2, mx2);
            expect (mn1 == mn2);
            expect (mx1 == mx2);

            expect (FloatVectorOperations::findMinimum (data1, num) == juce::findMinimum (data1, num));
            expect (FloatVectorOperations::findMaximum (data1, num) == juce::findMaximum (data1, num));

            expect (FloatVectorOperations::findMinimum (data2, num) == juce::findMinimum (data2, num));
            expect (FloatVectorOperations::findMaximum (data2, num) == juce::findMaximum (data2, num));

            FloatVectorOperations::clear (data1, num);
            expect (areAllValuesEqual (data1, num, 0));

            FloatVectorOperations::fill (data1, 2.0f, num);
            expect (areAllValuesEqual (data1, num, 2.0f));

            FloatVectorOperations::add (data1, 2.0f, num);
            expect (areAllValuesEqual (data1, num, 4.0f));

            FloatVectorOperations::copy (data2, data1, num);
            expect (areAllValuesEqual (data2, num, 4.0f));

            FloatVectorOperations::add (data2, data1, num);
            expect (areAllValuesEqual (data2, num, 8.0f));

            FloatVectorOperations::copyWithMultiply (data2, data1, 4.0f, num);
            expect (areAllValuesEqual (data2, num, 16.0f));

            FloatVectorOperations::addWithMultiply (data2, data1, 4.0f, num);
            expect (areAllValuesEqual (data2, num, 32.0f));

            FloatVectorOperations::multiply (data1, 2.0f, num);
            expect (areAllValuesEqual (data1, num, 8.0f));

            FloatVectorOperations::multiply (data1, data2, num);
            expect (areAllValuesEqual (data1, num, 256.0f));

            FloatVectorOperations::negate (data2, data1, num);
            expect (areAllValuesEqual (data2, num, -256.0f));

            FloatVectorOperations::subtract (data1, data2, num);
            expect (areAllValuesEqual (data1, num, 512.0f));

            fillRandomly (int1, num);
            FloatVectorOperations::convertFixedToFloat (data1, int1, 2.0f, num);
            convertFixed (data2, int1, 2.0f, num);
            expect (buffersMatch (data1, data2, num));
        }
    }

    void fillRandomly (float* d, int num)
    {
        while (--num >= 0)
            *d++ = getRandom().nextFloat() * 1000.0f;
    }

    void fillRandomly (int* d, int num)
    {
        while (--num >= 0)
            *d++ = getRandom().nextInt();
    }

    static void convertFixed (float* d, const int* s, float multiplier, int num)
    {
        while (--num >= 0)
            *d++ = *s++ * multiplier;
    }

    static bool areAllValuesEqual (const float* d, int num, float target)
    {
        while (--num >= 0)
            if (*d++ != target)
                return false;

        return true;
    }

    static bool buffersMatch (const float* d1, const float* d2, int num)
    {
        while (--num >= 0)
            if (std::abs (*d1++ - *d2++) > std::numeric_limits<float>::epsilon())
                return false;

        return true;
    }
};

static FloatVectorOperationsTests vectorOpTests;

#endif
