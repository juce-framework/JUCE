/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_LowLevelGraphicsSoftwareRenderer.h"
#include "juce_EdgeTable.h"
#include "../imaging/juce_Image.h"
#include "../colour/juce_PixelFormats.h"
#include "../geometry/juce_PathStrokeType.h"
#include "../geometry/juce_Rectangle.h"
#include "../../../../juce_core/basics/juce_SystemStats.h"

#if ! (defined (JUCE_MAC) || (defined (JUCE_WIN32) && defined (JUCE_64BIT)))
 #define JUCE_USE_SSE_INSTRUCTIONS 1
#endif

#if defined (JUCE_DEBUG) && JUCE_MSVC
 #pragma warning (disable: 4714)
#endif

#define MINIMUM_COORD -0x3fffffff
#define MAXIMUM_COORD 0x3fffffff

#define ASSERT_COORDS_ARE_SENSIBLE_NUMBERS(x, y, w, h) \
    jassert ((int) x >= MINIMUM_COORD  \
              && (int) x <= MAXIMUM_COORD \
              && (int) y >= MINIMUM_COORD \
              && (int) y <= MAXIMUM_COORD \
              && (int) w >= 0 \
              && (int) w < MAXIMUM_COORD \
              && (int) h >= 0 \
              && (int) h < MAXIMUM_COORD);

//==============================================================================
static void replaceRectRGB (uint8* pixels, const int w, int h, const int stride, const Colour& colour) throw()
{
    const PixelARGB blendColour (colour.getPixelARGB());

    if (w < 32)
    {
        while (--h >= 0)
        {
            PixelRGB* dest = (PixelRGB*) pixels;

            for (int i = w; --i >= 0;)
                (dest++)->set (blendColour);

            pixels += stride;
        }
    }
    else
    {
        // for wider fills, it's worth using some optimisations..

        const uint8 r = blendColour.getRed();
        const uint8 g = blendColour.getGreen();
        const uint8 b = blendColour.getBlue();

        if (r == g && r == b)  // if all the component values are the same, we can cheat..
        {
            while (--h >= 0)
            {
                memset (pixels, r, w * 3);
                pixels += stride;
            }
        }
        else
        {
            PixelRGB filler [4];
            filler[0].set (blendColour);
            filler[1].set (blendColour);
            filler[2].set (blendColour);
            filler[3].set (blendColour);
            const int* const intFiller = (const int*) filler;

            while (--h >= 0)
            {
                uint8* dest = (uint8*) pixels;

                int i = w;

                while ((i > 8) && (((pointer_sized_int) dest & 7) != 0))
                {
                    ((PixelRGB*) dest)->set (blendColour);
                    dest += 3;
                    --i;
                }

                while (i >= 4)
                {
                    ((int*) dest) [0] = intFiller[0];
                    ((int*) dest) [1] = intFiller[1];
                    ((int*) dest) [2] = intFiller[2];

                    dest += 12;
                    i -= 4;
                }

                while (--i >= 0)
                {
                    ((PixelRGB*) dest)->set (blendColour);
                    dest += 3;
                }

                pixels += stride;
            }
        }
    }
}

static void replaceRectARGB (uint8* pixels, const int w, int h, const int stride, const Colour& colour) throw()
{
    const PixelARGB blendColour (colour.getPixelARGB());

    while (--h >= 0)
    {
        PixelARGB* const dest = (PixelARGB*) pixels;

        for (int i = 0; i < w; ++i)
            dest[i] = blendColour;

        pixels += stride;
    }
}

static void blendRectRGB (uint8* pixels, const int w, int h, const int stride, const Colour& colour) throw()
{
    if (colour.isOpaque())
    {
        replaceRectRGB (pixels, w, h, stride, colour);
    }
    else
    {
        const PixelARGB blendColour (colour.getPixelARGB());
        const int alpha = blendColour.getAlpha();

        if (alpha <= 0)
            return;

#if defined (JUCE_USE_SSE_INSTRUCTIONS) && ! JUCE_64BIT
        if (SystemStats::hasSSE())
        {
            int64 rgb0 = (((int64) blendColour.getRed()) << 32)
                           | (int64) ((blendColour.getGreen() << 16)
                                        | blendColour.getBlue());

            const int invAlpha = 0xff - alpha;
            int64 aaaa = (invAlpha << 16) | invAlpha;
            aaaa = (aaaa << 16) | aaaa;

#ifndef JUCE_GCC
            __asm
            {
                movq mm1, aaaa
                movq mm2, rgb0
                pxor mm7, mm7
            }

            while (--h >= 0)
            {
                __asm
                {
                    mov edx, pixels
                    mov ebx, w

                pixloop:
                    prefetchnta [edx]
                    mov ax, [edx + 1]
                    shl eax, 8
                    mov al, [edx]
                    movd mm0, eax

                    punpcklbw mm0, mm7
                    pmullw mm0, mm1
                    psrlw mm0, 8
                    paddw mm0, mm2
                    packuswb mm0, mm7

                    movd eax, mm0
                    mov [edx], al
                    inc edx
                    shr eax, 8
                    mov [edx], ax
                    add edx, 2

                    dec ebx
                    jg pixloop
                }

                pixels += stride;
            }

            __asm emms
#else
            __asm__ __volatile__ (
                "movq %[aaaa], %%mm1        \n"
                "\tmovq %[rgb0], %%mm2      \n"
                "\tpxor %%mm7, %%mm7        \n"
                ".lineLoop2:                \n"
                "\tmovl %%esi,%%edx         \n"
                "\tmovl %[w], %%ebx         \n"
                ".pixLoop2:                 \n"
                "\tprefetchnta (%%edx)      \n"
                "\tmov (%%edx), %%ax        \n"
                "\tshl $8, %%eax            \n"
                "\tmov 2(%%edx), %%al       \n"
                "\tmovd %%eax, %%mm0        \n"
                "\tpunpcklbw %%mm7, %%mm0   \n"
                "\tpmullw %%mm1, %%mm0      \n"
                "\tpsrlw $8, %%mm0          \n"
                "\tpaddw %%mm2, %%mm0       \n"
                "\tpackuswb %%mm7, %%mm0    \n"
                "\tmovd %%mm0, %%eax        \n"
                "\tmovb %%al, (%%edx)       \n"
                "\tinc %%edx                \n"
                "\tshr $8, %%eax            \n"
                "\tmovw %%ax, (%%edx)       \n"
                "\tadd $2, %%edx            \n"
                "\tdec %%ebx                \n"
                "\tjg .pixLoop2             \n"
                "\tadd %%edi, %%esi         \n"
                "\tdec %%ecx                \n"
                "\tjg .lineLoop2            \n"
                "\temms                     \n"
                : /* No output registers */
                : [aaaa]   "m" (aaaa), /* Input registers */
                  [rgb0]   "m" (rgb0),
                  [w]      "m" (w),
                           "c" (h),
                  [stride] "D" (stride),
                  [pixels] "S" (pixels)
                : "cc", "eax", "edx", "memory"    /* Clobber list */
            );
#endif
        }
        else
#endif
        {
            while (--h >= 0)
            {
                PixelRGB* dest = (PixelRGB*) pixels;

                for (int i = w; --i >= 0;)
                    (dest++)->blend (blendColour);

                pixels += stride;
            }
        }
    }
}

static void blendRectARGB (uint8* pixels, const int w, int h, const int stride, const Colour& colour) throw()
{
    if (colour.isOpaque())
    {
        replaceRectARGB (pixels, w, h, stride, colour);
    }
    else
    {
        const PixelARGB blendColour (colour.getPixelARGB());
        const int alpha = blendColour.getAlpha();

        if (alpha <= 0)
            return;

        while (--h >= 0)
        {
            PixelARGB* dest = (PixelARGB*) pixels;

            for (int i = w; --i >= 0;)
                (dest++)->blend (blendColour);

            pixels += stride;
        }
    }
}

//==============================================================================
static void blendAlphaMapARGB (uint8* destPixel, const int imageStride,
                               const uint8* alphaValues, const int w, int h,
                               const int pixelStride, const int lineStride,
                               const Colour& colour) throw()
{
    const PixelARGB srcPix (colour.getPixelARGB());

    while (--h >= 0)
    {
        PixelARGB* dest = (PixelARGB*) destPixel;
        const uint8* src = alphaValues;

        int i = w;
        while (--i >= 0)
        {
            unsigned int srcAlpha = *src;
            src += pixelStride;

            if (srcAlpha > 0)
                dest->blend (srcPix, srcAlpha);

            ++dest;
        }

        alphaValues += lineStride;
        destPixel += imageStride;
    }
}

static void blendAlphaMapRGB (uint8* destPixel, const int imageStride,
                              const uint8* alphaValues, int const width, int height,
                              const int pixelStride, const int lineStride,
                              const Colour& colour) throw()
{
    const PixelARGB srcPix (colour.getPixelARGB());

    while (--height >= 0)
    {
        PixelRGB* dest = (PixelRGB*) destPixel;
        const uint8* src = alphaValues;

        int i = width;
        while (--i >= 0)
        {
            unsigned int srcAlpha = *src;
            src += pixelStride;

            if (srcAlpha > 0)
                dest->blend (srcPix, srcAlpha);

            ++dest;
        }

        alphaValues += lineStride;
        destPixel += imageStride;
    }
}

//==============================================================================
template <class PixelType>
class SolidColourEdgeTableRenderer
{
    uint8* const data;
    const int stride;
    PixelType* linePixels;
    PixelARGB sourceColour;

    SolidColourEdgeTableRenderer (const SolidColourEdgeTableRenderer&);
    const SolidColourEdgeTableRenderer& operator= (const SolidColourEdgeTableRenderer&);

public:
    SolidColourEdgeTableRenderer (uint8* const data_,
                                  const int stride_,
                                  const Colour& colour) throw()
        : data (data_),
          stride (stride_),
          sourceColour (colour.getPixelARGB())
    {
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) (data + stride * y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        linePixels[x].blend (sourceColour, alphaLevel);
    }

    forcedinline void handleEdgeTableLine (const int x, int width, const int alphaLevel) const throw()
    {
        PixelARGB p (sourceColour);
        p.multiplyAlpha (alphaLevel);

        PixelType* dest = linePixels + x;

        if (p.getAlpha() < 0xff)
        {
            do
            {
                dest->blend (p);
                ++dest;

            } while (--width > 0);
        }
        else
        {
            do
            {
                dest->set (p);
                ++dest;

            } while (--width > 0);
        }
    }
};

class AlphaBitmapRenderer
{
    uint8* data;
    int stride;
    uint8* lineStart;

    AlphaBitmapRenderer (const AlphaBitmapRenderer&);
    const AlphaBitmapRenderer& operator= (const AlphaBitmapRenderer&);

public:
    AlphaBitmapRenderer (uint8* const data_,
                         const int stride_) throw()
        : data (data_),
          stride (stride_)
    {
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        lineStart = data + (stride * y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        lineStart [x] = (uint8) alphaLevel;
    }

    forcedinline void handleEdgeTableLine (const int x, int width, const int alphaLevel) const throw()
    {
        uint8* d = lineStart + x;

        while (--width >= 0)
            *d++ = (uint8) alphaLevel;
    }
};

//==============================================================================
static const int numScaleBits = 12;

class LinearGradientPixelGenerator
{
    const PixelARGB* const lookupTable;
    const int numEntries;
    PixelARGB linePix;
    int start, scale;
    double grad, yTerm;
    bool vertical, horizontal;

    LinearGradientPixelGenerator (const LinearGradientPixelGenerator&);
    const LinearGradientPixelGenerator& operator= (const LinearGradientPixelGenerator&);

public:
    LinearGradientPixelGenerator (const ColourGradient& gradient,
                                  const PixelARGB* const lookupTable_, const int numEntries_)
        : lookupTable (lookupTable_),
          numEntries (numEntries_)
    {
        jassert (numEntries_ >= 0);
        float x1 = gradient.x1;
        float y1 = gradient.y1;
        float x2 = gradient.x2;
        float y2 = gradient.y2;

        if (! gradient.transform.isIdentity())
        {
            Line l (x2, y2, x1, y1);
            const Point p3 = l.getPointAlongLine (0.0, 100.0f);
            float x3 = p3.getX();
            float y3 = p3.getY();

            gradient.transform.transformPoint (x1, y1);
            gradient.transform.transformPoint (x2, y2);
            gradient.transform.transformPoint (x3, y3);

            Line l2 (x2, y2, x3, y3);
            float prop = l2.findNearestPointTo (x1, y1);
            const Point newP2 (l2.getPointAlongLineProportionally (prop));

            x2 = newP2.getX();
            y2 = newP2.getY();
        }

        vertical = fabs (x1 - x2) < 0.001f;
        horizontal = fabs (y1 - y2) < 0.001f;

        if (vertical)
        {
            scale = roundDoubleToInt ((numEntries << numScaleBits) / (double) (y2 - y1));
            start = roundDoubleToInt (y1 * scale);
        }
        else if (horizontal)
        {
            scale = roundDoubleToInt ((numEntries << numScaleBits) / (double) (x2 - x1));
            start = roundDoubleToInt (x1 * scale);
        }
        else
        {
            grad = (y2 - y1) / (double) (x1 - x2);
            yTerm = y1 - x1 / grad;
            scale = roundDoubleToInt ((numEntries << numScaleBits) / (yTerm * grad - (y2 * grad - x2)));
            grad *= scale;
        }
    }

    forcedinline void setY (const int y) throw()
    {
        if (vertical)
            linePix = lookupTable [jlimit (0, numEntries, (y * scale - start) >> numScaleBits)];
        else if (! horizontal)
            start = roundDoubleToInt ((y - yTerm) * grad);
    }

    forcedinline const PixelARGB getPixel (const int x) const throw()
    {
        if (vertical)
            return linePix;

        return lookupTable [jlimit (0, numEntries, (x * scale - start) >> numScaleBits)];
    }
};

class RadialGradientPixelGenerator
{
protected:
    const PixelARGB* const lookupTable;
    const int numEntries;
    const double gx1, gy1;
    double maxDist, invScale;
    double dy;

    RadialGradientPixelGenerator (const RadialGradientPixelGenerator&);
    const RadialGradientPixelGenerator& operator= (const RadialGradientPixelGenerator&);

public:
    RadialGradientPixelGenerator (const ColourGradient& gradient,
                                  const PixelARGB* const lookupTable_, const int numEntries_) throw()
        : lookupTable (lookupTable_),
          numEntries (numEntries_),
          gx1 (gradient.x1),
          gy1 (gradient.y1)
    {
        jassert (numEntries_ >= 0);
        const float dx = gradient.x1 - gradient.x2;
        const float dy = gradient.y1 - gradient.y2;
        maxDist = dx * dx + dy * dy;
        invScale = (numEntries + 1) / sqrt (maxDist);
    }

    forcedinline void setY (const int y) throw()
    {
        dy = y - gy1;
        dy *= dy;
    }

    forcedinline const PixelARGB getPixel (const int px) const throw()
    {
        double x = px - gx1;
        x *= x;
        x += dy;

        if (x >= maxDist)
            return lookupTable [numEntries];
        else
            return lookupTable [jmin (numEntries, roundDoubleToInt (sqrt (x) * invScale))];
    }
};

class TransformedRadialGradientPixelGenerator   : public RadialGradientPixelGenerator
{
    double tM10, tM00, lineYM01, lineYM11;
    AffineTransform inverseTransform;

    TransformedRadialGradientPixelGenerator (const TransformedRadialGradientPixelGenerator&);
    const TransformedRadialGradientPixelGenerator& operator= (const TransformedRadialGradientPixelGenerator&);

public:
    TransformedRadialGradientPixelGenerator (const ColourGradient& gradient,
                                             const PixelARGB* const lookupTable_, const int numEntries_) throw()
        : RadialGradientPixelGenerator (gradient, lookupTable_, numEntries_),
          inverseTransform (gradient.transform.inverted())
    {
        tM10 = inverseTransform.mat10;
        tM00 = inverseTransform.mat00;
    }

    forcedinline void setY (const int y) throw()
    {
        lineYM01 = inverseTransform.mat01 * y + inverseTransform.mat02 - gx1;
        lineYM11 = inverseTransform.mat11 * y + inverseTransform.mat12 - gy1;
    }

    forcedinline const PixelARGB getPixel (const int px) const throw()
    {
        double x = px;
        const double y = tM10 * x + lineYM11;
        x = tM00 * x + lineYM01;
        x *= x;
        x += y * y;

        if (x >= maxDist)
            return lookupTable [numEntries];
        else
            return lookupTable [jmin (numEntries, roundDoubleToInt (sqrt (x) * invScale))];
    }
};

template <class PixelType, class GradientType>
class GradientEdgeTableRenderer  : public GradientType
{
    uint8* const data;
    const int stride;
    PixelType* linePixels;

    GradientEdgeTableRenderer (const GradientEdgeTableRenderer&);
    const GradientEdgeTableRenderer& operator= (const GradientEdgeTableRenderer&);

public:
    GradientEdgeTableRenderer (uint8* const data_,
                               const int stride_,
                               const ColourGradient& gradient,
                               const PixelARGB* const lookupTable, const int numEntries) throw()
        : GradientType (gradient, lookupTable, numEntries - 1),
          data (data_),
          stride (stride_)
    {
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) (data + stride * y);
        GradientType::setY (y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        linePixels[x].blend (GradientType::getPixel (x), alphaLevel);
    }

    forcedinline void handleEdgeTableLine (int x, int width, const int alphaLevel) const throw()
    {
        PixelType* dest = linePixels + x;

        if (alphaLevel < 0xff)
        {
            do
            {
                (dest++)->blend (GradientType::getPixel (x++), alphaLevel);

            } while (--width > 0);
        }
        else
        {
            do
            {
                (dest++)->blend (GradientType::getPixel (x++));

            } while (--width > 0);
        }
    }
};

//==============================================================================
template <class DestPixelType, class SrcPixelType>
class ImageFillEdgeTableRenderer
{
    uint8* const destImageData;
    const uint8* srcImageData;
    int stride, srcStride, extraAlpha;

    DestPixelType* linePixels;
    SrcPixelType* sourceLineStart;

    ImageFillEdgeTableRenderer (const ImageFillEdgeTableRenderer&);
    const ImageFillEdgeTableRenderer& operator= (const ImageFillEdgeTableRenderer&);

public:
    ImageFillEdgeTableRenderer (uint8* const destImageData_,
                                const int stride_,
                                const uint8* srcImageData_,
                                const int srcStride_,
                                int extraAlpha_,
                                SrcPixelType*) throw() // dummy param to avoid compiler error
        : destImageData (destImageData_),
          srcImageData (srcImageData_),
          stride (stride_),
          srcStride (srcStride_),
          extraAlpha (extraAlpha_)
    {
    }

    forcedinline void setEdgeTableYPos (int y) throw()
    {
        linePixels = (DestPixelType*) (destImageData + stride * y);
        sourceLineStart = (SrcPixelType*) (srcImageData + srcStride * y);
    }

    forcedinline void handleEdgeTablePixel (const int x, int alphaLevel) const throw()
    {
        alphaLevel = (alphaLevel * extraAlpha) >> 8;

        linePixels[x].blend (sourceLineStart [x], alphaLevel);
    }

    forcedinline void handleEdgeTableLine (int x, int width, int alphaLevel) const throw()
    {
        DestPixelType* dest = linePixels + x;
        alphaLevel = (alphaLevel * extraAlpha) >> 8;

        if (alphaLevel < 0xfe)
        {
            do
            {
                dest++ ->blend (sourceLineStart [x++], alphaLevel);

            } while (--width > 0);
        }
        else
        {
            do
            {
                dest++ ->blend (sourceLineStart [x++]);

            } while (--width > 0);
        }
    }
};

//==============================================================================
static void blendRowOfPixels (PixelARGB* dst,
                              const PixelRGB* src,
                              int width) throw()
{
    while (--width >= 0)
        (dst++)->set (*src++);
}

static void blendRowOfPixels (PixelRGB* dst,
                              const PixelRGB* src,
                              int width) throw()
{
    memcpy (dst, src, 3 * width);
}

static void blendRowOfPixels (PixelRGB* dst,
                              const PixelARGB* src,
                              int width) throw()
{
    while (--width >= 0)
        (dst++)->blend (*src++);
}

static void blendRowOfPixels (PixelARGB* dst,
                              const PixelARGB* src,
                              int width) throw()
{
    while (--width >= 0)
        (dst++)->blend (*src++);
}

static void blendRowOfPixels (PixelARGB* dst,
                              const PixelRGB* src,
                              int width,
                              const uint8 alpha) throw()
{
    while (--width >= 0)
        (dst++)->blend (*src++, alpha);
}

static void blendRowOfPixels (PixelRGB* dst,
                              const PixelRGB* src,
                              int width,
                              const uint8 alpha) throw()
{
    uint8* d = (uint8*) dst;
    const uint8* s = (const uint8*) src;
    const int inverseAlpha = 0xff - alpha;

    while (--width >= 0)
    {
        d[0] = (uint8) (s[0] + (((d[0] - s[0]) * inverseAlpha) >> 8));
        d[1] = (uint8) (s[1] + (((d[1] - s[1]) * inverseAlpha) >> 8));
        d[2] = (uint8) (s[2] + (((d[2] - s[2]) * inverseAlpha) >> 8));

        d += 3;
        s += 3;
    }
}

static void blendRowOfPixels (PixelRGB* dst,
                              const PixelARGB* src,
                              int width,
                              const uint8 alpha) throw()
{
    while (--width >= 0)
        (dst++)->blend (*src++, alpha);
}

static void blendRowOfPixels (PixelARGB* dst,
                              const PixelARGB* src,
                              int width,
                              const uint8 alpha) throw()
{
    while (--width >= 0)
        (dst++)->blend (*src++, alpha);
}

template <class DestPixelType, class SrcPixelType>
static void overlayImage (DestPixelType* dest,
                          const int destStride,
                          const SrcPixelType* src,
                          const int srcStride,
                          const int width,
                          int height,
                          const uint8 alpha) throw()
{
    if (alpha < 0xff)
    {
        while (--height >= 0)
        {
            blendRowOfPixels (dest, src, width, alpha);

            dest = (DestPixelType*) (((uint8*) dest) + destStride);
            src = (const SrcPixelType*) (((const uint8*) src) + srcStride);
        }
    }
    else
    {
        while (--height >= 0)
        {
            blendRowOfPixels (dest, src, width);

            dest = (DestPixelType*) (((uint8*) dest) + destStride);
            src = (const SrcPixelType*) (((const uint8*) src) + srcStride);
        }
    }
}

template <class DestPixelType, class SrcPixelType>
static void transformedImageRender (Image& destImage,
                                    const Image& sourceImage,
                                    const int destClipX, const int destClipY,
                                    const int destClipW, const int destClipH,
                                    const int srcClipX, const int srcClipY,
                                    const int srcClipRight, const int srcClipBottom,
                                    double srcX, double srcY,
                                    const double lineDX, const double lineDY,
                                    const double pixelDX, const double pixelDY,
                                    const uint8 alpha,
                                    const Graphics::ResamplingQuality quality,
                                    DestPixelType*,
                                    SrcPixelType*) throw() // forced by a compiler bug to include dummy
                                                           // parameters of the templated classes to
                                                           // make it use the correct instance of this function..
{
    int destStride, destPixelStride;
    uint8* const destPixels = destImage.lockPixelDataReadWrite (destClipX, destClipY, destClipW, destClipH, destStride, destPixelStride);

    int srcStride, srcPixelStride;
    const uint8* const srcPixels = sourceImage.lockPixelDataReadOnly (srcClipX, srcClipY, srcClipRight - srcClipX, srcClipBottom - srcClipY, srcStride, srcPixelStride);

    if (quality == Graphics::lowResamplingQuality) // nearest-neighbour..
    {
        for (int y = 0; y < destClipH; ++y)
        {
            double sx = srcX;
            double sy = srcY;

            DestPixelType* dest = (DestPixelType*) (destPixels + destStride * y);

            for (int x = 0; x < destClipW; ++x)
            {
                const int ix = roundDoubleToInt (floor (sx));
                const int iy = roundDoubleToInt (floor (sy));

                if (ix >= srcClipX && iy >= srcClipY
                     && ix < srcClipRight && iy < srcClipBottom)
                {
                    const SrcPixelType* const src = (const SrcPixelType*) (srcPixels + srcStride * (iy - srcClipY) + srcPixelStride * (ix - srcClipX));

                    dest->blend (*src, alpha);
                }

                ++dest;
                sx += pixelDX;
                sy += pixelDY;
            }

            srcX += lineDX;
            srcY += lineDY;
        }
    }
    else
    {
        jassert (quality == Graphics::mediumResamplingQuality); // (only bilinear is implemented, so that's what you'll get here..)

        for (int y = 0; y < destClipH; ++y)
        {
            double sx = srcX;
            double sy = srcY;
            DestPixelType* dest = (DestPixelType*) (destPixels + destStride * y);

            for (int x = 0; x < destClipW; ++x)
            {
                const double fx = floor (sx);
                const double fy = floor (sy);
                int ix = roundDoubleToInt (fx);
                int iy = roundDoubleToInt (fy);

                if (ix < srcClipRight && iy < srcClipBottom)
                {
                    const SrcPixelType* src = (const SrcPixelType*) (srcPixels + srcStride * (iy - srcClipY) + srcPixelStride * (ix - srcClipX));

                    SrcPixelType p1 (0);
                    const int dx = roundDoubleToInt ((sx - fx) * 255.0);

                    if (iy >= srcClipY)
                    {
                        if (ix >= srcClipX)
                            p1 = src[0];

                        ++ix;

                        if (ix >= srcClipX && ix < srcClipRight)
                            p1.tween (src[1], dx);

                        --ix;
                    }

                    ++iy;

                    if (iy >= srcClipY && iy < srcClipBottom)
                    {
                        SrcPixelType p2 (0);
                        src = (const SrcPixelType*) (((const uint8*) src) + srcStride);

                        if (ix >= srcClipX)
                            p2 = src[0];

                        ++ix;

                        if (ix >= srcClipX && ix < srcClipRight)
                            p2.tween (src[1], dx);

                        p1.tween (p2, roundDoubleToInt ((sy - fy) * 255.0));
                    }

                    if (p1.getAlpha() > 0)
                        dest->blend (p1, alpha);
                }

                ++dest;
                sx += pixelDX;
                sy += pixelDY;
            }

            srcX += lineDX;
            srcY += lineDY;
        }
    }

    destImage.releasePixelDataReadWrite (destPixels);
    sourceImage.releasePixelDataReadOnly (srcPixels);
}

template <class SrcPixelType, class DestPixelType>
static void renderAlphaMap (DestPixelType* destPixels,
                            int destStride,
                            SrcPixelType* srcPixels,
                            int srcStride,
                            const uint8* alphaValues,
                            const int lineStride, const int pixelStride,
                            int width, int height,
                            const int extraAlpha) throw()
{
    while (--height >= 0)
    {
        SrcPixelType* srcPix = srcPixels;
        srcPixels = (SrcPixelType*) (((const uint8*) srcPixels) + srcStride);

        DestPixelType* destPix = destPixels;
        destPixels = (DestPixelType*) (((uint8*) destPixels) + destStride);

        const uint8* alpha = alphaValues;
        alphaValues += lineStride;

        if (extraAlpha < 0x100)
        {
            for (int i = width; --i >= 0;)
            {
                destPix++ ->blend (*srcPix++, (extraAlpha * *alpha) >> 8);
                alpha += pixelStride;
            }
        }
        else
        {
            for (int i = width; --i >= 0;)
            {
                destPix++ ->blend (*srcPix++, *alpha);
                alpha += pixelStride;
            }
        }
    }
}

//==============================================================================
LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (Image& image_)
    : image (image_),
      xOffset (0),
      yOffset (0),
      stateStack (20)
{
    clip = new RectangleList (Rectangle (0, 0, image_.getWidth(), image_.getHeight()));
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer()
{
    delete clip;
}

bool LowLevelGraphicsSoftwareRenderer::isVectorDevice() const
{
    return false;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setOrigin (int x, int y)
{
    xOffset += x;
    yOffset += y;
}

bool LowLevelGraphicsSoftwareRenderer::reduceClipRegion (int x, int y, int w, int h)
{
    return clip->clipTo (Rectangle (x + xOffset, y + yOffset, w, h));
}

bool LowLevelGraphicsSoftwareRenderer::reduceClipRegion (const RectangleList& clipRegion)
{
    RectangleList temp (clipRegion);
    temp.offsetAll (xOffset, yOffset);

    return clip->clipTo (temp);
}

void LowLevelGraphicsSoftwareRenderer::excludeClipRegion (int x, int y, int w, int h)
{
    clip->subtract (Rectangle (x + xOffset, y + yOffset, w, h));
}

bool LowLevelGraphicsSoftwareRenderer::clipRegionIntersects (int x, int y, int w, int h)
{
    return clip->intersectsRectangle (Rectangle (x + xOffset, y + yOffset, w, h));
}

const Rectangle LowLevelGraphicsSoftwareRenderer::getClipBounds() const
{
    return clip->getBounds().translated (-xOffset, -yOffset);
}

bool LowLevelGraphicsSoftwareRenderer::isClipEmpty() const
{
    return clip->isEmpty();
}

//==============================================================================
LowLevelGraphicsSoftwareRenderer::SavedState::SavedState (RectangleList* const clip_,
                                                          const int xOffset_, const int yOffset_)
    : clip (clip_),
      xOffset (xOffset_),
      yOffset (yOffset_)
{
}

LowLevelGraphicsSoftwareRenderer::SavedState::~SavedState()
{
    delete clip;
}

void LowLevelGraphicsSoftwareRenderer::saveState()
{
    stateStack.add (new SavedState (new RectangleList (*clip), xOffset, yOffset));
}

void LowLevelGraphicsSoftwareRenderer::restoreState()
{
    SavedState* const top = stateStack.getLast();

    if (top != 0)
    {
        clip->swapWith (*top->clip);

        xOffset = top->xOffset;
        yOffset = top->yOffset;

        stateStack.removeLast();
    }
    else
    {
        jassertfalse // trying to pop with an empty stack!
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillRectWithColour (int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents)
{
    x += xOffset;
    y += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        clippedFillRectWithColour (*i.getRectangle(), x, y, w, h, colour, replaceExistingContents);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillRectWithColour (const Rectangle& clipRect,
                                                                  int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents)
{
    if (clipRect.intersectRectangle (x, y, w, h))
    {
        int stride, pixelStride;
        uint8* const pixels = (uint8*) image.lockPixelDataReadWrite (x, y, w, h, stride, pixelStride);

        if (image.getFormat() == Image::RGB)
        {
            if (replaceExistingContents)
                replaceRectRGB (pixels, w, h, stride, colour);
            else
                blendRectRGB (pixels, w, h, stride, colour);
        }
        else if (image.getFormat() == Image::ARGB)
        {
            if (replaceExistingContents)
                replaceRectARGB (pixels, w, h, stride, colour);
            else
                blendRectARGB (pixels, w, h, stride, colour);
        }
        else
        {
            jassertfalse // not done!
        }

        image.releasePixelDataReadWrite (pixels);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillRectWithGradient (int x, int y, int w, int h, const ColourGradient& gradient)
{
    Path p;
    p.addRectangle ((float) x, (float) y, (float) w, (float) h);
    fillPathWithGradient (p, AffineTransform::identity, gradient, EdgeTable::Oversampling_none);
}

//==============================================================================
bool LowLevelGraphicsSoftwareRenderer::getPathBounds (int clipX, int clipY, int clipW, int clipH,
                                                      const Path& path, const AffineTransform& transform,
                                                      int& x, int& y, int& w, int& h) const
{
    float tx, ty, tw, th;
    path.getBoundsTransformed (transform, tx, ty, tw, th);

    x = roundDoubleToInt (tx) - 1;
    y = roundDoubleToInt (ty) - 1;
    w = roundDoubleToInt (tw) + 2;
    h = roundDoubleToInt (th) + 2;

    // seems like this operation is using some crazy out-of-range numbers..
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, w, h);

    return Rectangle::intersectRectangles (x, y, w, h, clipX, clipY, clipW, clipH);
}

void LowLevelGraphicsSoftwareRenderer::fillPathWithColour (const Path& path, const AffineTransform& t,
                                                           const Colour& colour, EdgeTable::OversamplingLevel quality)
{
    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillPathWithColour (r.getX(), r.getY(), r.getWidth(), r.getHeight(), path, t, colour, quality);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillPathWithColour (int clipX, int clipY, int clipW, int clipH, const Path& path, const AffineTransform& t,
                                                                  const Colour& colour, EdgeTable::OversamplingLevel quality)
{
    const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));
    int cx, cy, cw, ch;

    if (getPathBounds (clipX, clipY, clipW, clipH, path, transform, cx, cy, cw, ch))
    {
        EdgeTable edgeTable (0, ch, quality);

        edgeTable.addPath (path, transform.translated ((float) -cx, (float) -cy));

        int stride, pixelStride;
        uint8* const pixels = (uint8*) image.lockPixelDataReadWrite (cx, cy, cw, ch, stride, pixelStride);

        if (image.getFormat() == Image::RGB)
        {
            jassert (pixelStride == 3);
            SolidColourEdgeTableRenderer <PixelRGB> renderer (pixels, stride, colour);
            edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
        }
        else if (image.getFormat() == Image::ARGB)
        {
            jassert (pixelStride == 4);
            SolidColourEdgeTableRenderer <PixelARGB> renderer (pixels, stride, colour);
            edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
        }
        else if (image.getFormat() == Image::SingleChannel)
        {
            jassert (pixelStride == 1);
            AlphaBitmapRenderer renderer (pixels, stride);
            edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
        }

        image.releasePixelDataReadWrite (pixels);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillPathWithGradient (const Path& path, const AffineTransform& t, const ColourGradient& gradient, EdgeTable::OversamplingLevel quality)
{
    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillPathWithGradient (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                     path, t, gradient, quality);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillPathWithGradient (int clipX, int clipY, int clipW, int clipH, const Path& path, const AffineTransform& t,
                                                                    const ColourGradient& gradient, EdgeTable::OversamplingLevel quality)
{
    const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));
    int cx, cy, cw, ch;

    if (getPathBounds (clipX, clipY, clipW, clipH, path, transform, cx, cy, cw, ch))
    {
        int stride, pixelStride;
        uint8* const pixels = (uint8*) image.lockPixelDataReadWrite (cx, cy, cw, ch, stride, pixelStride);

        ColourGradient g2 (gradient);

        const bool isIdentity = g2.transform.isIdentity();
        if (isIdentity)
        {
            g2.x1 += xOffset - cx;
            g2.x2 += xOffset - cx;
            g2.y1 += yOffset - cy;
            g2.y2 += yOffset - cy;
        }
        else
        {
            g2.transform = g2.transform.translated ((float) (xOffset - cx),
                                                    (float) (yOffset - cy));
        }

        int numLookupEntries;
        PixelARGB* const lookupTable = g2.createLookupTable (numLookupEntries);
        jassert (numLookupEntries > 0);

        EdgeTable edgeTable (0, ch, quality);

        edgeTable.addPath (path, transform.translated ((float) -cx, (float) -cy));

        if (image.getFormat() == Image::RGB)
        {
            jassert (pixelStride == 3);

            if (g2.isRadial)
            {
                if (isIdentity)
                {
                    GradientEdgeTableRenderer <PixelRGB, RadialGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                    edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
                }
                else
                {
                    GradientEdgeTableRenderer <PixelRGB, TransformedRadialGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                    edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
                }
            }
            else
            {
                GradientEdgeTableRenderer <PixelRGB, LinearGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
            }
        }
        else if (image.getFormat() == Image::ARGB)
        {
            jassert (pixelStride == 4);

            if (g2.isRadial)
            {
                if (isIdentity)
                {
                    GradientEdgeTableRenderer <PixelARGB, RadialGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                    edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
                }
                else
                {
                    GradientEdgeTableRenderer <PixelARGB, TransformedRadialGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                    edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
                }
            }
            else
            {
                GradientEdgeTableRenderer <PixelARGB, LinearGradientPixelGenerator> renderer (pixels, stride, g2, lookupTable, numLookupEntries);
                edgeTable.iterate (renderer, 0, 0, cw, ch, 0);
            }
        }
        else if (image.getFormat() == Image::SingleChannel)
        {
            jassertfalse // not done!
        }

        juce_free (lookupTable);
        image.releasePixelDataReadWrite (pixels);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillPathWithImage (const Path& path, const AffineTransform& transform,
                                                          const Image& sourceImage, int imageX, int imageY, float opacity, EdgeTable::OversamplingLevel quality)
{
    imageX += xOffset;
    imageY += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillPathWithImage (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                  path, transform, sourceImage, imageX, imageY, opacity, quality);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillPathWithImage (int x, int y, int w, int h, const Path& path, const AffineTransform& transform,
                                                                 const Image& sourceImage, int imageX, int imageY, float opacity, EdgeTable::OversamplingLevel quality)
{
    if (Rectangle::intersectRectangles (x, y, w, h, imageX, imageY, sourceImage.getWidth(), sourceImage.getHeight()))
    {
        EdgeTable edgeTable (0, h, quality);
        edgeTable.addPath (path, transform.translated ((float) (xOffset - x), (float) (yOffset - y)));

        int stride, pixelStride;
        uint8* const pixels = (uint8*) image.lockPixelDataReadWrite (x, y, w, h, stride, pixelStride);

        int srcStride, srcPixelStride;
        const uint8* const srcPix = (const uint8*) sourceImage.lockPixelDataReadOnly (x - imageX, y - imageY, w, h, srcStride, srcPixelStride);

        const int alpha = jlimit (0, 255, roundDoubleToInt (opacity * 255.0f));

        if (image.getFormat() == Image::RGB)
        {
            if (sourceImage.getFormat() == Image::RGB)
            {
                ImageFillEdgeTableRenderer <PixelRGB, PixelRGB> renderer (pixels, stride,
                                                                          srcPix, srcStride,
                                                                          alpha, (PixelRGB*) 0);
                edgeTable.iterate (renderer, 0, 0, w, h, 0);
            }
            else if (sourceImage.getFormat() == Image::ARGB)
            {
                ImageFillEdgeTableRenderer <PixelRGB, PixelARGB> renderer (pixels, stride,
                                                                           srcPix, srcStride,
                                                                           alpha, (PixelARGB*) 0);
                edgeTable.iterate (renderer, 0, 0, w, h, 0);
            }
            else
            {
                jassertfalse // not done!
            }
        }
        else if (image.getFormat() == Image::ARGB)
        {
            if (sourceImage.getFormat() == Image::RGB)
            {
                ImageFillEdgeTableRenderer <PixelARGB, PixelRGB> renderer (pixels, stride,
                                                                           srcPix, srcStride,
                                                                           alpha, (PixelRGB*) 0);
                edgeTable.iterate (renderer, 0, 0, w, h, 0);
            }
            else if (sourceImage.getFormat() == Image::ARGB)
            {
                ImageFillEdgeTableRenderer <PixelARGB, PixelARGB> renderer (pixels, stride,
                                                                            srcPix, srcStride,
                                                                            alpha, (PixelARGB*) 0);
                edgeTable.iterate (renderer, 0, 0, w, h, 0);
            }
            else
            {
                jassertfalse // not done!
            }
        }
        else
        {
            jassertfalse // not done!
        }

        sourceImage.releasePixelDataReadOnly (srcPix);
        image.releasePixelDataReadWrite (pixels);
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillAlphaChannelWithColour (const Image& clipImage, int x, int y, const Colour& colour)
{
    x += xOffset;
    y += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillAlphaChannelWithColour (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                           clipImage, x, y, colour);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillAlphaChannelWithColour (int clipX, int clipY, int clipW, int clipH, const Image& clipImage, int x, int y, const Colour& colour)
{
    int w = clipImage.getWidth();
    int h = clipImage.getHeight();
    int sx = 0;
    int sy = 0;

    if (x < clipX)
    {
        sx = clipX - x;
        w -= clipX - x;
        x = clipX;
    }

    if (y < clipY)
    {
        sy = clipY - y;
        h -= clipY - y;
        y = clipY;
    }

    if (x + w > clipX + clipW)
        w = clipX + clipW - x;

    if (y + h > clipY + clipH)
        h = clipY + clipH - y;

    if (w > 0 && h > 0)
    {
        int stride, alphaStride, pixelStride;
        uint8* const pixels = (uint8*) image.lockPixelDataReadWrite (x, y, w, h, stride, pixelStride);

        const uint8* const alphaValues
            = clipImage.lockPixelDataReadOnly (sx, sy, w, h, alphaStride, pixelStride);

#if JUCE_MAC
        const uint8* const alphas = alphaValues;
#else
        const uint8* const alphas = alphaValues + (clipImage.getFormat() == Image::ARGB ? 3 : 0);
#endif

        if (image.getFormat() == Image::RGB)
        {
            blendAlphaMapRGB (pixels, stride,
                              alphas, w, h,
                              pixelStride, alphaStride,
                              colour);
        }
        else if (image.getFormat() == Image::ARGB)
        {
            blendAlphaMapARGB (pixels, stride,
                               alphas, w, h,
                               pixelStride, alphaStride,
                               colour);
        }
        else
        {
            jassertfalse // not done!
        }

        clipImage.releasePixelDataReadOnly (alphaValues);
        image.releasePixelDataReadWrite (pixels);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillAlphaChannelWithGradient (const Image& alphaChannelImage, int imageX, int imageY, const ColourGradient& gradient)
{
    imageX += xOffset;
    imageY += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillAlphaChannelWithGradient (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                             alphaChannelImage, imageX, imageY, gradient);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillAlphaChannelWithGradient (int x, int y, int w, int h,
                                                                            const Image& alphaChannelImage,
                                                                            int imageX, int imageY, const ColourGradient& gradient)
{
    if (Rectangle::intersectRectangles (x, y, w, h, imageX, imageY, alphaChannelImage.getWidth(), alphaChannelImage.getHeight()))
    {
        ColourGradient g2 (gradient);
        g2.x1 += xOffset - x;
        g2.x2 += xOffset - x;
        g2.y1 += yOffset - y;
        g2.y2 += yOffset - y;

        Image temp (g2.isOpaque() ? Image::RGB : Image::ARGB, w, h, true);
        LowLevelGraphicsSoftwareRenderer tempG (temp);
        tempG.fillRectWithGradient (0, 0, w, h, g2);

        clippedFillAlphaChannelWithImage (x, y, w, h,
                                          alphaChannelImage, imageX, imageY,
                                          temp, x, y, 1.0f);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                                                  const Image& fillerImage, int fillerImageX, int fillerImageY, float opacity)
{
    alphaImageX += xOffset;
    alphaImageY += yOffset;

    fillerImageX += xOffset;
    fillerImageY += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedFillAlphaChannelWithImage (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                          alphaImage, alphaImageX, alphaImageY,
                                          fillerImage, fillerImageX, fillerImageY, opacity);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedFillAlphaChannelWithImage (int x, int y, int w, int h, const Image& alphaImage, int alphaImageX, int alphaImageY,
                                                                         const Image& fillerImage, int fillerImageX, int fillerImageY, float opacity)
{
    if (Rectangle::intersectRectangles (x, y, w, h, alphaImageX, alphaImageY, alphaImage.getWidth(), alphaImage.getHeight())
         && Rectangle::intersectRectangles (x, y, w, h, fillerImageX, fillerImageY, fillerImage.getWidth(), fillerImage.getHeight()))
    {
        int dstStride, dstPixStride;
        uint8* const dstPix = image.lockPixelDataReadWrite (x, y, w, h, dstStride, dstPixStride);

        int srcStride, srcPixStride;
        const uint8* const srcPix = fillerImage.lockPixelDataReadOnly (x - fillerImageX, y - fillerImageY, w, h, srcStride, srcPixStride);

        int maskStride, maskPixStride;
        const uint8* const alpha
            = alphaImage.lockPixelDataReadOnly (x - alphaImageX, y - alphaImageY, w, h, maskStride, maskPixStride);

#if JUCE_MAC
        const uint8* const alphaValues = alpha;
#else
        const uint8* const alphaValues = alpha + (alphaImage.getFormat() == Image::ARGB ? 3 : 0);
#endif

        const int extraAlpha = jlimit (0, 0x100, roundDoubleToInt (opacity * 256.0f));

        if (image.getFormat() == Image::RGB)
        {
            if (fillerImage.getFormat() == Image::RGB)
            {
                renderAlphaMap ((PixelRGB*) dstPix, dstStride, (const PixelRGB*) srcPix, srcStride, alphaValues, maskStride, maskPixStride, w, h, extraAlpha);
            }
            else if (fillerImage.getFormat() == Image::ARGB)
            {
                renderAlphaMap ((PixelRGB*) dstPix, dstStride, (const PixelARGB*) srcPix, srcStride, alphaValues, maskStride, maskPixStride, w, h, extraAlpha);
            }
            else
            {
                jassertfalse // not done!
            }
        }
        else if (image.getFormat() == Image::ARGB)
        {
            if (fillerImage.getFormat() == Image::RGB)
            {
                renderAlphaMap ((PixelARGB*) dstPix, dstStride, (const PixelRGB*) srcPix, srcStride, alphaValues, maskStride, maskPixStride, w, h, extraAlpha);
            }
            else if (fillerImage.getFormat() == Image::ARGB)
            {
                renderAlphaMap ((PixelARGB*) dstPix, dstStride, (const PixelARGB*) srcPix, srcStride, alphaValues, maskStride, maskPixStride, w, h, extraAlpha);
            }
            else
            {
                jassertfalse // not done!
            }
        }
        else
        {
            jassertfalse // not done!
        }

        alphaImage.releasePixelDataReadOnly (alphaValues);
        fillerImage.releasePixelDataReadOnly (srcPix);
        image.releasePixelDataReadWrite (dstPix);
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::blendImage (const Image& sourceImage, int dx, int dy, int dw, int dh, int sx, int sy, float opacity)
{
    dx += xOffset;
    dy += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedBlendImage (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                           sourceImage, dx, dy, dw, dh, sx, sy, opacity);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedBlendImage (int clipX, int clipY, int clipW, int clipH,
                                                          const Image& sourceImage, int dx, int dy, int dw, int dh, int sx, int sy, float opacity)
{
    if (dx < clipX)
    {
        sx += clipX - dx;
        dw -= clipX - dx;
        dx = clipX;
    }

    if (dy < clipY)
    {
        sy += clipY - dy;
        dh -= clipY - dy;
        dy = clipY;
    }

    if (dx + dw > clipX + clipW)
        dw = clipX + clipW - dx;

    if (dy + dh > clipY + clipH)
        dh = clipY + clipH - dy;

    if (dw <= 0 || dh <= 0)
        return;

    const uint8 alpha = (uint8) jlimit (0, 0xff, roundDoubleToInt (opacity * 256.0f));

    if (alpha == 0)
        return;

    int dstStride, dstPixelStride;
    uint8* const dstPixels = image.lockPixelDataReadWrite (dx, dy, dw, dh, dstStride, dstPixelStride);

    int srcStride, srcPixelStride;
    const uint8* const srcPixels = sourceImage.lockPixelDataReadOnly (sx, sy, dw, dh, srcStride, srcPixelStride);

    if (image.getFormat() == Image::ARGB)
    {
        if (sourceImage.getFormat() == Image::ARGB)
        {
            overlayImage ((PixelARGB*) dstPixels, dstStride,
                          (PixelARGB*) srcPixels, srcStride,
                          dw, dh, alpha);
        }
        else if (sourceImage.getFormat() == Image::RGB)
        {
            overlayImage ((PixelARGB*) dstPixels, dstStride,
                          (PixelRGB*) srcPixels, srcStride,
                          dw, dh, alpha);
        }
        else
        {
            jassertfalse
        }
    }
    else if (image.getFormat() == Image::RGB)
    {
        if (sourceImage.getFormat() == Image::ARGB)
        {
            overlayImage ((PixelRGB*) dstPixels, dstStride,
                          (PixelARGB*) srcPixels, srcStride,
                          dw, dh, alpha);
        }
        else if (sourceImage.getFormat() == Image::RGB)
        {
            overlayImage ((PixelRGB*) dstPixels, dstStride,
                          (PixelRGB*) srcPixels, srcStride,
                          dw, dh, alpha);
        }
        else
        {
            jassertfalse
        }
    }
    else
    {
        jassertfalse
    }

    image.releasePixelDataReadWrite (dstPixels);
    sourceImage.releasePixelDataReadOnly (srcPixels);
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::blendImageRescaling (const Image& sourceImage,
                                                            int dx, int dy, int dw, int dh,
                                                            int sx, int sy, int sw, int sh,
                                                            float alpha,
                                                            const Graphics::ResamplingQuality quality)
{
    if (sw > 0 && sh > 0)
    {
        if (sw == dw && sh == dh)
        {
            blendImage (sourceImage,
                        dx, dy, dw, dh,
                        sx, sy, alpha);
        }
        else
        {
            blendImageWarping (sourceImage,
                               sx, sy, sw, sh,
                               AffineTransform::translation ((float) -sx,
                                                             (float) -sy)
                                               .scaled (dw / (float) sw,
                                                        dh / (float) sh)
                                               .translated ((float) dx,
                                                            (float) dy),
                               alpha,
                               quality);
        }
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::blendImageWarping (const Image& sourceImage,
                                                          int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                                          const AffineTransform& t,
                                                          float opacity,
                                                          const Graphics::ResamplingQuality quality)
{
    const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedBlendImageWarping (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                  sourceImage, srcClipX, srcClipY, srcClipW, srcClipH,
                                  transform, opacity, quality);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedBlendImageWarping (int destClipX, int destClipY, int destClipW, int destClipH,
                                                                 const Image& sourceImage,
                                                                 int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                                                 const AffineTransform& transform,
                                                                 float opacity,
                                                                 const Graphics::ResamplingQuality quality)
{
    if (opacity > 0 && destClipW > 0 && destClipH > 0 && ! transform.isSingularity())
    {
        Rectangle::intersectRectangles (srcClipX, srcClipY, srcClipW, srcClipH,
                                        0, 0, sourceImage.getWidth(), sourceImage.getHeight());

        if (srcClipW <= 0 || srcClipH <= 0)
            return;

        jassert (srcClipX >= 0 && srcClipY >= 0);

        Path imageBounds;
        imageBounds.addRectangle ((float) srcClipX, (float) srcClipY, (float) srcClipW, (float) srcClipH);
        imageBounds.applyTransform (transform);
        float imX, imY, imW, imH;
        imageBounds.getBounds (imX, imY, imW, imH);

        if (Rectangle::intersectRectangles (destClipX, destClipY, destClipW, destClipH,
                                            (int) floorf (imX),
                                            (int) floorf (imY),
                                            1 + roundDoubleToInt (imW),
                                            1 + roundDoubleToInt (imH)))
        {
            const int srcClipRight  = srcClipX + srcClipW;
            const int srcClipBottom = srcClipY + srcClipH;

            const uint8 alpha = (uint8) jlimit (0, 0xff, roundDoubleToInt (opacity * 256.0f));

            float srcX1 = (float) destClipX;
            float srcY1 = (float) destClipY;
            float srcX2 = (float) (destClipX + destClipW);
            float srcY2 = srcY1;
            float srcX3 = srcX1;
            float srcY3 = (float) (destClipY + destClipH);

            AffineTransform inverse (transform.inverted());
            inverse.transformPoint (srcX1, srcY1);
            inverse.transformPoint (srcX2, srcY2);
            inverse.transformPoint (srcX3, srcY3);

            const double lineDX  = (double) (srcX3 - srcX1) / destClipH;
            const double lineDY  = (double) (srcY3 - srcY1) / destClipH;
            const double pixelDX = (double) (srcX2 - srcX1) / destClipW;
            const double pixelDY = (double) (srcY2 - srcY1) / destClipW;

            if (image.getFormat() == Image::ARGB)
            {
                if (sourceImage.getFormat() == Image::ARGB)
                {
                    transformedImageRender (image, sourceImage,
                                            destClipX, destClipY, destClipW, destClipH,
                                            srcClipX, srcClipY, srcClipRight, srcClipBottom,
                                            srcX1, srcY1, lineDX, lineDY, pixelDX, pixelDY,
                                            alpha, quality, (PixelARGB*)0, (PixelARGB*)0);
                }
                else if (sourceImage.getFormat() == Image::RGB)
                {
                    transformedImageRender (image, sourceImage,
                                            destClipX, destClipY, destClipW, destClipH,
                                            srcClipX, srcClipY, srcClipRight, srcClipBottom,
                                            srcX1, srcY1, lineDX, lineDY, pixelDX, pixelDY,
                                            alpha, quality, (PixelARGB*)0, (PixelRGB*)0);
                }
                else
                {
                    jassertfalse
                }
            }
            else if (image.getFormat() == Image::RGB)
            {
                if (sourceImage.getFormat() == Image::ARGB)
                {
                    transformedImageRender (image, sourceImage,
                                            destClipX, destClipY, destClipW, destClipH,
                                            srcClipX, srcClipY, srcClipRight, srcClipBottom,
                                            srcX1, srcY1, lineDX, lineDY, pixelDX, pixelDY,
                                            alpha, quality, (PixelRGB*)0, (PixelARGB*)0);
                }
                else if (sourceImage.getFormat() == Image::RGB)
                {
                    transformedImageRender (image, sourceImage,
                                            destClipX, destClipY, destClipW, destClipH,
                                            srcClipX, srcClipY, srcClipRight, srcClipBottom,
                                            srcX1, srcY1, lineDX, lineDY, pixelDX, pixelDY,
                                            alpha, quality, (PixelRGB*)0, (PixelRGB*)0);
                }
                else
                {
                    jassertfalse
                }
            }
            else
            {
                jassertfalse
            }
        }
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::drawLine (double x1, double y1, double x2, double y2, const Colour& colour)
{
    x1 += xOffset;
    y1 += yOffset;
    x2 += xOffset;
    y2 += yOffset;

    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedDrawLine (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                         x1, y1, x2, y2, colour);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedDrawLine (int clipX, int clipY, int clipW, int clipH, double x1, double y1, double x2, double y2, const Colour& colour)
{
    if (clipW > 0 && clipH > 0)
    {
        if (x1 == x2)
        {
            if (y2 < y1)
                swapVariables (y1, y2);

            clippedDrawVerticalLine (clipX, clipY, clipW, clipH, roundDoubleToInt (x1), y1, y2, colour);
        }
        else if (y1 == y2)
        {
            if (x2 < x1)
                swapVariables (x1, x2);

            clippedDrawHorizontalLine (clipX, clipY, clipW, clipH, roundDoubleToInt (y1), x1, x2, colour);
        }
        else
        {
            double gradient = (y2 - y1) / (x2 - x1);

            if (fabs (gradient) > 1.0)
            {
                gradient = 1.0 / gradient;

                int y = roundDoubleToInt (y1);
                const int startY = y;
                int endY = roundDoubleToInt (y2);

                if (y > endY)
                    swapVariables (y, endY);

                while (y < endY)
                {
                    const double x = x1 + gradient * (y - startY);
                    clippedDrawHorizontalLine (clipX, clipY, clipW, clipH, y, x, x + 1.0, colour);
                    ++y;
                }
            }
            else
            {
                int x = roundDoubleToInt (x1);
                const int startX = x;
                int endX = roundDoubleToInt (x2);

                if (x > endX)
                    swapVariables (x, endX);

                while (x < endX)
                {
                    const double y = y1 + gradient * (x - startX);
                    clippedDrawVerticalLine (clipX, clipY, clipW, clipH, x, y, y + 1.0, colour);
                    ++x;
                }
            }
        }
    }
}

void LowLevelGraphicsSoftwareRenderer::drawVerticalLine (const int x, double top, double bottom, const Colour& col)
{
    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedDrawVerticalLine (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                 x + xOffset, top + yOffset, bottom + yOffset, col);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedDrawVerticalLine (int clipX, int clipY, int clipW, int clipH,
                                                                const int x, double top, double bottom, const Colour& col)
{
    jassert (top <= bottom);

    if (x >= clipX
         && x < clipX + clipW
         && top < clipY + clipH
         && bottom > clipY
         && clipW > 0)
    {
        if (top < clipY)
            top = clipY;

        if (bottom > clipY + clipH)
            bottom = clipY + clipH;

        if (bottom > top)
            drawVertical (x, top, bottom, col);
    }
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontalLine (const int y, double left, double right, const Colour& col)
{
    for (RectangleList::Iterator i (*clip); i.next();)
    {
        const Rectangle& r = *i.getRectangle();

        clippedDrawHorizontalLine (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                   y + yOffset, left + xOffset, right + xOffset, col);
    }
}

void LowLevelGraphicsSoftwareRenderer::clippedDrawHorizontalLine (int clipX, int clipY, int clipW, int clipH,
                                                                  const int y, double left, double right, const Colour& col)
{
    jassert (left <= right);

    if (y >= clipY
         && y < clipY + clipH
         && left < clipX + clipW
         && right > clipX
         && clipW > 0)
    {
        if (left < clipX)
            left = clipX;

        if (right > clipX + clipW)
            right = clipX + clipW;

        if (right > left)
            drawHorizontal (y, left, right, col);
    }
}

void LowLevelGraphicsSoftwareRenderer::drawVertical (const int x,
                                                     const double top,
                                                     const double bottom,
                                                     const Colour& col)
{
    int wholeStart = (int) top;
    const int wholeEnd = (int) bottom;

    const int lastAlpha = roundDoubleToInt (255.0 * (bottom - wholeEnd));
    const int totalPixels = (wholeEnd - wholeStart) + (lastAlpha > 0 ? 1 : 0);

    if (totalPixels <= 0)
        return;

    int lineStride, dstPixelStride;
    uint8* const dstPixels = image.lockPixelDataReadWrite (x, wholeStart, 1, totalPixels, lineStride, dstPixelStride);
    uint8* dest = dstPixels;

    PixelARGB colour (col.getPixelARGB());

    if (wholeEnd == wholeStart)
    {
        if (image.getFormat() == Image::ARGB)
            ((PixelARGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (bottom - top)));
        else if (image.getFormat() == Image::RGB)
            ((PixelRGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (bottom - top)));
        else
        {
            jassertfalse
        }
    }
    else
    {
        if (image.getFormat() == Image::ARGB)
        {
            ((PixelARGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (1.0 - (top - wholeStart))));
            ++wholeStart;
            dest += lineStride;

            if (colour.getAlpha() == 0xff)
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelARGB*) dest)->set (colour);
                    ++wholeStart;
                    dest += lineStride;
                }
            }
            else
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelARGB*) dest)->blend (colour);
                    ++wholeStart;
                    dest += lineStride;
                }
            }

            if (lastAlpha > 0)
            {
                ((PixelARGB*) dest)->blend (colour, lastAlpha);
            }
        }
        else if (image.getFormat() == Image::RGB)
        {
            ((PixelRGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (1.0 - (top - wholeStart))));
            ++wholeStart;
            dest += lineStride;

            if (colour.getAlpha() == 0xff)
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelRGB*) dest)->set (colour);
                    ++wholeStart;
                    dest += lineStride;
                }
            }
            else
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelRGB*) dest)->blend (colour);
                    ++wholeStart;
                    dest += lineStride;
                }
            }

            if (lastAlpha > 0)
            {
                ((PixelRGB*) dest)->blend (colour, lastAlpha);
            }
        }
        else
        {
            jassertfalse
        }
    }

    image.releasePixelDataReadWrite (dstPixels);
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontal (const int y,
                                                       const double top,
                                                       const double bottom,
                                                       const Colour& col)
{
    int wholeStart = (int) top;
    const int wholeEnd = (int) bottom;

    const int lastAlpha = roundDoubleToInt (255.0 * (bottom - wholeEnd));
    const int totalPixels = (wholeEnd - wholeStart) + (lastAlpha > 0 ? 1 : 0);

    if (totalPixels <= 0)
        return;

    int lineStride, dstPixelStride;
    uint8* const dstPixels = image.lockPixelDataReadWrite (wholeStart, y, totalPixels, 1, lineStride, dstPixelStride);
    uint8* dest = dstPixels;

    PixelARGB colour (col.getPixelARGB());

    if (wholeEnd == wholeStart)
    {
        if (image.getFormat() == Image::ARGB)
            ((PixelARGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (bottom - top)));
        else if (image.getFormat() == Image::RGB)
            ((PixelRGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (bottom - top)));
        else
        {
            jassertfalse
        }
    }
    else
    {
        if (image.getFormat() == Image::ARGB)
        {
            ((PixelARGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (1.0 - (top - wholeStart))));
            dest += dstPixelStride;
            ++wholeStart;

            if (colour.getAlpha() == 0xff)
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelARGB*) dest)->set (colour);
                    dest += dstPixelStride;
                    ++wholeStart;
                }
            }
            else
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelARGB*) dest)->blend (colour);
                    dest += dstPixelStride;
                    ++wholeStart;
                }
            }

            if (lastAlpha > 0)
            {
                ((PixelARGB*) dest)->blend (colour, lastAlpha);
            }
        }
        else if (image.getFormat() == Image::RGB)
        {
            ((PixelRGB*) dest)->blend (colour, roundDoubleToInt (255.0 * (1.0 - (top - wholeStart))));
            dest += dstPixelStride;
            ++wholeStart;

            if (colour.getAlpha() == 0xff)
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelRGB*) dest)->set (colour);
                    dest += dstPixelStride;
                    ++wholeStart;
                }
            }
            else
            {
                while (wholeEnd > wholeStart)
                {
                    ((PixelRGB*) dest)->blend (colour);
                    dest += dstPixelStride;
                    ++wholeStart;
                }
            }

            if (lastAlpha > 0)
            {
                ((PixelRGB*) dest)->blend (colour, lastAlpha);
            }
        }
        else
        {
            jassertfalse
        }
    }

    image.releasePixelDataReadWrite (dstPixels);
}



END_JUCE_NAMESPACE
