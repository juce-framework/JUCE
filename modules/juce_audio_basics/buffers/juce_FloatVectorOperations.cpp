/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if JUCE_USE_SSE_INTRINSICS

namespace FloatVectorHelpers
{
    static bool sse2Present = false;

    static bool isSSE2Available()
    {
        if (sse2Present)
            return true;

        sse2Present = SystemStats::hasSSE2();
        return sse2Present;
    }

    inline static bool isAligned (const void* p)
    {
        return (((pointer_sized_int) p) & 15) == 0;
    }
}

#define JUCE_BEGIN_SSE_OP \
    if (FloatVectorHelpers::isSSE2Available()) \
    { \
        const int numLongOps = num / 4;

#define JUCE_FINISH_SSE_OP(normalOp) \
        _mm_empty(); \
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

#define JUCE_PERFORM_SSE_OP_DEST(normalOp, sseOp) \
    JUCE_BEGIN_SSE_OP \
    if (FloatVectorHelpers::isAligned (dest))   JUCE_SSE_LOOP (sseOp, dummy, _mm_load_ps,  _mm_store_ps,  JUCE_LOAD_DEST, JUCE_INCREMENT_DEST) \
    else                                        JUCE_SSE_LOOP (sseOp, dummy, _mm_loadu_ps, _mm_storeu_ps, JUCE_LOAD_DEST, JUCE_INCREMENT_DEST) \
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
 #define JUCE_PERFORM_SSE_OP_DEST(normalOp, unused1)                       for (int i = 0; i < num; ++i) normalOp;
 #define JUCE_PERFORM_SSE_OP_SRC_DEST(normalOp, sseOp, locals, increment)  for (int i = 0; i < num; ++i) normalOp;
#endif

void FloatVectorOperations::clear (float* dest, const int num) noexcept
{
    zeromem (dest, num * sizeof (float));
}

void FloatVectorOperations::copy (float* dest, const float* src, const int num) noexcept
{
    memcpy (dest, src, num * sizeof (float));
}

void FloatVectorOperations::copyWithMultiply (float* dest, const float* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                  _mm_mul_ps (mult, s),
                                  JUCE_LOAD_SRC, JUCE_INCREMENT_SRC_DEST)
}

void FloatVectorOperations::add (float* dest, const float* src, int num) noexcept
{
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i],
                                  _mm_add_ps (d, s),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
}

void FloatVectorOperations::add (float* dest, float amount, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 amountToAdd = _mm_load1_ps (&amount);
   #endif

    JUCE_PERFORM_SSE_OP_DEST (dest[i] += amount,
                              _mm_add_ps (d, amountToAdd))
}

void FloatVectorOperations::addWithMultiply (float* dest, const float* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] += src[i] * multiplier,
                                  _mm_add_ps (d, _mm_mul_ps (mult, s)),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
}

void FloatVectorOperations::multiply (float* dest, const float* src, int num) noexcept
{
    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] *= src[i],
                                  _mm_mul_ps (d, s),
                                  JUCE_LOAD_SRC_DEST, JUCE_INCREMENT_SRC_DEST)
}

void FloatVectorOperations::multiply (float* dest, float multiplier, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_DEST (dest[i] *= multiplier,
                              _mm_mul_ps (d, mult))
}

void FloatVectorOperations::convertFixedToFloat (float* dest, const int* src, float multiplier, int num) noexcept
{
   #if JUCE_USE_SSE_INTRINSICS
    const __m128 mult = _mm_load1_ps (&multiplier);
   #endif

    JUCE_PERFORM_SSE_OP_SRC_DEST (dest[i] = src[i] * multiplier,
                                  _mm_mul_ps (mult, _mm_movelh_ps (_mm_cvt_pi2ps (_mm_setzero_ps(), ((const __m64*) src)[0]),
                                                                   _mm_cvt_pi2ps (_mm_setzero_ps(), ((const __m64*) src)[1]))),
                                  JUCE_LOAD_NONE, JUCE_INCREMENT_SRC_DEST)
}

void FloatVectorOperations::findMinAndMax (const float* src, int num, float& minResult, float& maxResult) noexcept
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
            _mm_empty();

            localMin = jmin (mns[0], mns[1], mns[2], mns[3]);
            localMax = jmax (mxs[0], mxs[1], mxs[2], mxs[3]);
        }

        num &= 3;

        if (num != 0)
        {
            for (int i = 0; i < num; ++i)
            {
                const float s = src[i];
                localMin = jmin (localMin, s);
                localMax = jmax (localMax, s);
            }
        }

        minResult = localMin;
        maxResult = localMax;
        return;
    }
   #endif

    juce::findMinAndMax (src, num, minResult, maxResult);
}
