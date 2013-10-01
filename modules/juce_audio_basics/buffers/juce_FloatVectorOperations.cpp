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

#if JUCE_USE_SSE_INTRINSICS

namespace FloatVectorHelpers
{
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
       #if JUCE_USE_SSE_INTRINSICS
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
       #endif

        return isMinimum ? juce::findMinimum (src, num)
                         : juce::findMaximum (src, num);
    }
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

#define JUCE_INCREMENT_SRC_DEST    dest += 4; src += 4;
#define JUCE_INCREMENT_DEST        dest += 4;

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


#else
 #define JUCE_PERFORM_SSE_OP_DEST(normalOp, unused1, unused2)              for (int i = 0; i < num; ++i) normalOp;
 #define JUCE_PERFORM_SSE_OP_SRC_DEST(normalOp, sseOp, locals, increment)  for (int i = 0; i < num; ++i) normalOp;
#endif

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
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                  _mm_mul_ps (mult, s),
                                  JUCE_LOAD_SRC, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::add (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vadd (src, 1, dest, 1, dest, 1, num);
   #else
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i],
                                  _mm_add_ps (d, s),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::add (float* dest, float amount, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 amountToAdd = _mm_load1_ps (&amount);
   #endif

    JUCE_PERFORM_SSE_OP_DEST (dest[i] += amount,
                              _mm_add_ps (d, amountToAdd),
                              JUCE_LOAD_DEST)
}

void JUCE_CALLTYPE FloatVectorOperations::addWithMultiply (float* dest, const float* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i] * multiplier,
                                  _mm_add_ps (d, _mm_mul_ps (mult, s)),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
}

void JUCE_CALLTYPE FloatVectorOperations::multiply (float* dest, const float* src, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vmul (src, 1, dest, 1, dest, 1, num);
   #else
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] *= src[i],
                                  _mm_mul_ps (d, s),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
   #endif
}

void JUCE_CALLTYPE FloatVectorOperations::multiply (float* dest, float multiplier, int num) noexcept
{
   #if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vsmul (dest, 1, &multiplier, dest, 1, num);
   #else
    #if JUCE_USE_SSE_INTRINSICS
     const __m128 mult = _mm_load1_ps (&multiplier);
    #endif

    JUCE_PERFORM_SSE_OP_DEST (dest[i] *= multiplier,
                              _mm_mul_ps (d, mult),
                              JUCE_LOAD_DEST)
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
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                  _mm_mul_ps (mult, _mm_cvtepi32_ps (_mm_loadu_si128 ((const __m128i*) src))),
                                  JUCE_LOAD_NONE, JUCE_INCREMENT_SRC_DEST)
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
   #endif

    juce::findMinAndMax (src, num, minResult, maxResult);
}

float JUCE_CALLTYPE FloatVectorOperations::findMinimum (const float* src, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    return FloatVectorHelpers::findMinimumOrMaximum (src, num, true);
   #else
    return juce::findMinimum (src, num);
   #endif
}

float JUCE_CALLTYPE FloatVectorOperations::findMaximum (const float* src, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
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
