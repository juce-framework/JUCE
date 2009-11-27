/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_LowLevelGraphicsSoftwareRenderer.h"
#include "juce_EdgeTable.h"
#include "../imaging/juce_Image.h"
#include "../colour/juce_PixelFormats.h"
#include "../geometry/juce_PathStrokeType.h"
#include "../geometry/juce_Rectangle.h"
#include "../../../core/juce_SystemStats.h"
#include "../../../utilities/juce_DeletedAtShutdown.h"

#if (JUCE_WINDOWS || JUCE_LINUX) && ! JUCE_64BIT
 #define JUCE_USE_SSE_INSTRUCTIONS 1
#endif

#if JUCE_MSVC && JUCE_DEBUG
 #pragma warning (disable: 4714) // warning about forcedinline methods not being inlined
#endif

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4127) // "expression is constant" warning
#endif

//==============================================================================
/*static void replaceRectARGB (uint8* pixels, const int w, int h, const int stride, const Colour& colour) throw()
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

#if JUCE_USE_SSE_INSTRUCTIONS
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
                "\tpush %%ebx               \n"
                "\tmovq %[aaaa], %%mm1      \n"
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
                "\tpop %%ebx                \n"
                "\temms                     \n"
                : // No output registers
                : [aaaa]   "m" (aaaa), // Input registers
                  [rgb0]   "m" (rgb0),
                  [w]      "m" (w),
                           "c" (h),
                  [stride] "D" (stride),
                  [pixels] "S" (pixels)
                : "cc", "eax", "edx", "memory"
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
*/

//==============================================================================
template <class PixelType, bool replaceExisting = false>
class SolidColourEdgeTableRenderer
{
public:
    SolidColourEdgeTableRenderer (const Image::BitmapData& data_, const PixelARGB& colour) throw()
        : data (data_),
          sourceColour (colour)
    {
        if (sizeof (PixelType) == 3)
        {
            areRGBComponentsEqual = sourceColour.getRed() == sourceColour.getGreen()
                                        && sourceColour.getGreen() == sourceColour.getBlue();
            filler[0].set (sourceColour);
            filler[1].set (sourceColour);
            filler[2].set (sourceColour);
            filler[3].set (sourceColour);
        }
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) data.getLinePointer (y);
    }

    forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
    {
        if (replaceExisting)
            linePixels[x].set (sourceColour);
        else
            linePixels[x].blend (sourceColour, alphaLevel);
    }

    forcedinline void handleEdgeTableLine (const int x, int width, const int alphaLevel) const throw()
    {
        PixelARGB p (sourceColour);
        p.multiplyAlpha (alphaLevel);

        PixelType* dest = linePixels + x;

        if (replaceExisting || p.getAlpha() >= 0xff)
            replaceLine (dest, p, width);
        else
            blendLine (dest, p, width);
    }

private:
    const Image::BitmapData& data;
    PixelType* linePixels;
    PixelARGB sourceColour;
    PixelRGB filler [4];
    bool areRGBComponentsEqual;

    forcedinline void blendLine (PixelType* dest, const PixelARGB& colour, int width) const throw()
    {
        do
        {
            dest->blend (colour);
            ++dest;

        } while (--width > 0);
    }

    forcedinline void replaceLine (PixelRGB* dest, const PixelARGB& colour, int width) const throw()
    {
        if (areRGBComponentsEqual)  // if all the component values are the same, we can cheat..
        {
            memset (dest, colour.getRed(), width * 3);
        }
        else
        {
            if (width >> 5)
            {
                const int* const intFiller = (const int*) filler;

                while (width > 8 && (((pointer_sized_int) dest) & 7) != 0)
                {
                    dest->set (colour);
                    ++dest;
                    --width;
                }

                while (width > 4)
                {
                    ((int*) dest) [0] = intFiller[0];
                    ((int*) dest) [1] = intFiller[1];
                    ((int*) dest) [2] = intFiller[2];
                    dest = (PixelRGB*) (((uint8*) dest) + 12);
                    width -= 4;
                }
            }

            while (--width >= 0)
            {
                dest->set (colour);
                ++dest;
            }
        }
    }

    forcedinline void replaceLine (PixelAlpha* dest, const PixelARGB& colour, int width) const throw()
    {
        memset (dest, colour.getAlpha(), width);
    }

    forcedinline void replaceLine (PixelARGB* dest, const PixelARGB& colour, int width) const throw()
    {
        do
        {
            dest->set (colour);
            ++dest;

        } while (--width > 0);
    }

    SolidColourEdgeTableRenderer (const SolidColourEdgeTableRenderer&);
    const SolidColourEdgeTableRenderer& operator= (const SolidColourEdgeTableRenderer&);
};

//==============================================================================
class LinearGradientPixelGenerator
{
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
            scale = roundDoubleToInt ((numEntries << (int) numScaleBits) / (double) (y2 - y1));
            start = roundDoubleToInt (y1 * scale);
        }
        else if (horizontal)
        {
            scale = roundDoubleToInt ((numEntries << (int) numScaleBits) / (double) (x2 - x1));
            start = roundDoubleToInt (x1 * scale);
        }
        else
        {
            grad = (y2 - y1) / (double) (x1 - x2);
            yTerm = y1 - x1 / grad;
            scale = roundDoubleToInt ((numEntries << (int) numScaleBits) / (yTerm * grad - (y2 * grad - x2)));
            grad *= scale;
        }
    }

    forcedinline void setY (const int y) throw()
    {
        if (vertical)
            linePix = lookupTable [jlimit (0, numEntries, (y * scale - start) >> (int) numScaleBits)];
        else if (! horizontal)
            start = roundDoubleToInt ((y - yTerm) * grad);
    }

    forcedinline const PixelARGB getPixel (const int x) const throw()
    {
        if (vertical)
            return linePix;

        return lookupTable [jlimit (0, numEntries, (x * scale - start) >> (int) numScaleBits)];
    }

private:
    const PixelARGB* const lookupTable;
    const int numEntries;
    PixelARGB linePix;
    int start, scale;
    double grad, yTerm;
    bool vertical, horizontal;
    enum { numScaleBits = 12 };

    LinearGradientPixelGenerator (const LinearGradientPixelGenerator&);
    const LinearGradientPixelGenerator& operator= (const LinearGradientPixelGenerator&);
};

//==============================================================================
class RadialGradientPixelGenerator
{
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

protected:
    const PixelARGB* const lookupTable;
    const int numEntries;
    const double gx1, gy1;
    double maxDist, invScale, dy;

    RadialGradientPixelGenerator (const RadialGradientPixelGenerator&);
    const RadialGradientPixelGenerator& operator= (const RadialGradientPixelGenerator&);
};

//==============================================================================
class TransformedRadialGradientPixelGenerator   : public RadialGradientPixelGenerator
{
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

private:
    double tM10, tM00, lineYM01, lineYM11;
    const AffineTransform inverseTransform;

    TransformedRadialGradientPixelGenerator (const TransformedRadialGradientPixelGenerator&);
    const TransformedRadialGradientPixelGenerator& operator= (const TransformedRadialGradientPixelGenerator&);
};

//==============================================================================
template <class PixelType, class GradientType>
class GradientEdgeTableRenderer  : public GradientType
{
public:
    GradientEdgeTableRenderer (const Image::BitmapData& destData_, const ColourGradient& gradient,
                               const PixelARGB* const lookupTable, const int numEntries) throw()
        : GradientType (gradient, lookupTable, numEntries - 1),
          destData (destData_)
    {
    }

    forcedinline void setEdgeTableYPos (const int y) throw()
    {
        linePixels = (PixelType*) destData.getLinePointer (y);
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

private:
    const Image::BitmapData& destData;
    PixelType* linePixels;

    GradientEdgeTableRenderer (const GradientEdgeTableRenderer&);
    const GradientEdgeTableRenderer& operator= (const GradientEdgeTableRenderer&);
};

//==============================================================================
template <class DestPixelType, class SrcPixelType>
class ImageFillEdgeTableRenderer
{
public:
    ImageFillEdgeTableRenderer (const Image::BitmapData& destData_,
                                const Image::BitmapData& srcData_,
                                const int extraAlpha_) throw()
        : destData (destData_),
          srcData (srcData_),
          extraAlpha (extraAlpha_ + 1)
    {
    }

    forcedinline void setEdgeTableYPos (int y) throw()
    {
        linePixels = (DestPixelType*) destData.getLinePointer (y);
        sourceLineStart = (SrcPixelType*) srcData.getLinePointer (y);
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
            copyRow (dest, sourceLineStart + x, width);
        }
    }

private:
    const Image::BitmapData& destData;
    const Image::BitmapData& srcData;
    const int extraAlpha;
    DestPixelType* linePixels;
    SrcPixelType* sourceLineStart;

    template <class PixelType1, class PixelType2>
    forcedinline static void copyRow (PixelType1* dest, PixelType2* src, int width) throw()
    {
        do
        {
            dest++ ->blend (*src++);

        } while (--width > 0);
    }

    forcedinline static void copyRow (PixelRGB* dest, PixelRGB* src, int width) throw()
    {
        memcpy (dest, src, width * sizeof (PixelRGB));
    }

    ImageFillEdgeTableRenderer (const ImageFillEdgeTableRenderer&);
    const ImageFillEdgeTableRenderer& operator= (const ImageFillEdgeTableRenderer&);
};

//==============================================================================
template <class DestPixelType, class SrcPixelType>
class TransformedImageFillEdgeTableRenderer
{
public:
    TransformedImageFillEdgeTableRenderer (const Image::BitmapData& destData_,
                                           const Image::BitmapData& srcData_,
                                           const AffineTransform& transform,
                                           const int extraAlpha_,
                                           const bool betterQuality_) throw()
        : interpolator (transform),
          destData (destData_),
          srcData (srcData_),
          extraAlpha (extraAlpha_ + 1),
          betterQuality (betterQuality_),
          pixelOffset (betterQuality_ ? 0.5f : 0.0f),
          pixelOffsetInt (betterQuality_ ? -128 : 0),
          maxX (srcData_.width - 1),
          maxY (srcData_.height - 1)
    {
    }

    forcedinline void setEdgeTableYPos (const int newY) throw()
    {
        y = newY;
        linePixels = (DestPixelType*) destData.getLinePointer (newY);
    }

    forcedinline void handleEdgeTablePixel (const int x, int alphaLevel) throw()
    {
        alphaLevel *= extraAlpha;
        alphaLevel >>= 8;

        SrcPixelType p;
        generate (&p, x, 1);

        linePixels[x].blend (p, alphaLevel);
    }

    forcedinline void handleEdgeTableLine (const int x, int width, int alphaLevel) throw()
    {
        SrcPixelType* span = (SrcPixelType*) alloca (sizeof (SrcPixelType) * width);
        generate (span, x, width);

        DestPixelType* dest = linePixels + x;
        alphaLevel *= extraAlpha;
        alphaLevel >>= 8;

        if (alphaLevel < 0xfe)
        {
            do
            {
                dest++ ->blend (*span++, alphaLevel);
            } while (--width > 0);
        }
        else
        {
            do
            {
                dest++ ->blend (*span++);
            } while (--width > 0);
        }
    }

private:
    //==============================================================================
    void generate (PixelARGB* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;

            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                uint32 c[4] = { 256 * 128, 256 * 128, 256 * 128, 256 * 128 };
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);

                uint32 weight = (256 - hiResX) * (256 - hiResY);
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];
                c[3] += weight * src[3];

                weight = hiResX * (256 - hiResY);
                c[0] += weight * src[4];
                c[1] += weight * src[5];
                c[2] += weight * src[6];
                c[3] += weight * src[7];

                src += this->srcData.lineStride;

                weight = (256 - hiResX) * hiResY;
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];
                c[3] += weight * src[3];

                weight = hiResX * hiResY;
                c[0] += weight * src[4];
                c[1] += weight * src[5];
                c[2] += weight * src[6];
                c[3] += weight * src[7];

                dest->setARGB ((uint8) (c[PixelARGB::indexA] >> 16),
                               (uint8) (c[PixelARGB::indexR] >> 16),
                               (uint8) (c[PixelARGB::indexG] >> 16),
                               (uint8) (c[PixelARGB::indexB] >> 16));
            }
            else
            {
                // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                if (loResX < 0)     loResX = 0;
                if (loResY < 0)     loResY = 0;
                if (loResX > maxX)  loResX = maxX;
                if (loResY > maxY)  loResY = maxY;

                dest->set (*(const PixelARGB*) this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    void generate (PixelRGB* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;
            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                uint32 c[3] = { 256 * 128, 256 * 128, 256 * 128 };
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);

                unsigned int weight = (256 - hiResX) * (256 - hiResY);
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];

                weight = hiResX * (256 - hiResY);
                c[0] += weight * src[3];
                c[1] += weight * src[4];
                c[2] += weight * src[5];

                src += this->srcData.lineStride;

                weight = (256 - hiResX) * hiResY;
                c[0] += weight * src[0];
                c[1] += weight * src[1];
                c[2] += weight * src[2];

                weight = hiResX * hiResY;
                c[0] += weight * src[3];
                c[1] += weight * src[4];
                c[2] += weight * src[5];

                dest->setARGB ((uint8) 255,
                               (uint8) (c[PixelRGB::indexR] >> 16),
                               (uint8) (c[PixelRGB::indexG] >> 16),
                               (uint8) (c[PixelRGB::indexB] >> 16));
            }
            else
            {
                // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                if (loResX < 0)     loResX = 0;
                if (loResY < 0)     loResY = 0;
                if (loResX > maxX)  loResX = maxX;
                if (loResY > maxY)  loResY = maxY;

                dest->set (*(const PixelRGB*) this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    void generate (PixelAlpha* dest, const int x, int numPixels) throw()
    {
        this->interpolator.setStartOfLine (x + pixelOffset, y + pixelOffset, numPixels);

        do
        {
            int hiResX, hiResY;
            this->interpolator.next (hiResX, hiResY);
            hiResX += pixelOffsetInt;
            hiResY += pixelOffsetInt;
            int loResX = hiResX >> 8;
            int loResY = hiResY >> 8;

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                hiResX &= 255;
                hiResY &= 255;

                uint32 c = 256 * 128;
                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);
                c += src[0] * ((256 - hiResX) * (256 - hiResY));
                c += src[1] * (hiResX * (256 - hiResY));
                src += this->srcData.lineStride;
                c += src[0] * ((256 - hiResX) * hiResY);
                c += src[1] * (hiResX * hiResY);

                *((uint8*) dest) = (uint8) c;
            }
            else
            {
                // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                if (loResX < 0)     loResX = 0;
                if (loResY < 0)     loResY = 0;
                if (loResX > maxX)  loResX = maxX;
                if (loResY > maxY)  loResY = maxY;

                *((uint8*) dest) = *(this->srcData.getPixelPointer (loResX, loResY));
            }

            ++dest;

        } while (--numPixels > 0);
    }

    //==============================================================================
    class TransformedImageSpanInterpolator
    {
    public:
        TransformedImageSpanInterpolator (const AffineTransform& transform) throw()
            : inverseTransform (transform.inverted())
        {}

        void setStartOfLine (float x, float y, const int numPixels) throw()
        {
            float x1 = x, y1 = y;
            inverseTransform.transformPoint (x1, y1);
            x += numPixels;
            inverseTransform.transformPoint (x, y);

            xBresenham.set ((int) (x1 * 256.0f), (int) (x * 256.0f), numPixels);
            yBresenham.set ((int) (y1 * 256.0f), (int) (y * 256.0f), numPixels);
        }

        void next (int& x, int& y) throw()
        {
            x = xBresenham.n;
            xBresenham.stepToNext();
            y = yBresenham.n;
            yBresenham.stepToNext();
        }

    private:
        class BresenhamInterpolator
        {
        public:
            BresenhamInterpolator() throw() {}

            void set (const int n1, const int n2, const int numSteps_) throw()
            {
                numSteps = jmax (1, numSteps_);
                step = (n2 - n1) / numSteps;
                remainder = modulo = (n2 - n1) % numSteps;
                n = n1;

                if (modulo <= 0)
                {
                    modulo += numSteps;
                    remainder += numSteps;
                    --step;
                }

                modulo -= numSteps;
            }

            forcedinline void stepToNext() throw()
            {
                modulo += remainder;
                n += step;

                if (modulo > 0)
                {
                    modulo -= numSteps;
                    ++n;
                }
            }

            int n;

        private:
            int numSteps, step, modulo, remainder;
        };

        const AffineTransform inverseTransform;
        BresenhamInterpolator xBresenham, yBresenham;

        TransformedImageSpanInterpolator (const TransformedImageSpanInterpolator&);
        const TransformedImageSpanInterpolator& operator= (const TransformedImageSpanInterpolator&);
    };

    //==============================================================================
    TransformedImageSpanInterpolator interpolator;
    const Image::BitmapData& destData;
    const Image::BitmapData& srcData;
    const int extraAlpha;
    const bool betterQuality;
    const float pixelOffset;
    const int pixelOffsetInt, maxX, maxY;
    int y;
    DestPixelType* linePixels;

    TransformedImageFillEdgeTableRenderer (const TransformedImageFillEdgeTableRenderer&);
    const TransformedImageFillEdgeTableRenderer& operator= (const TransformedImageFillEdgeTableRenderer&);
};

//==============================================================================
class LLGCSavedState
{
public:
    LLGCSavedState (const Rectangle& clip_,
                    const int xOffset_, const int yOffset_,
                    const Font& font_, const Colour& colour_, ColourGradient* const gradient_,
                    const Graphics::ResamplingQuality interpolationQuality_) throw()
        : edgeTable (new EdgeTableHolder (EdgeTable (clip_))),
          xOffset (xOffset_),
          yOffset (yOffset_),
          font (font_),
          colour (colour_),
          gradient (gradient_),
          interpolationQuality (interpolationQuality_)
    {
    }

    LLGCSavedState (const LLGCSavedState& other) throw()
        : edgeTable (other.edgeTable),
          xOffset (other.xOffset),
          yOffset (other.yOffset),
          font (other.font),
          colour (other.colour),
          gradient (other.gradient),
          interpolationQuality (other.interpolationQuality)
    {
        if (gradient != 0)
            gradient = new ColourGradient (*gradient);
    }

    ~LLGCSavedState() throw()
    {
        delete gradient;
    }

    bool reduce (int x, int y, int w, int h) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        edgeTable->edgeTable.clipToRectangle (Rectangle (x, y, w, h));
        return ! edgeTable->edgeTable.isEmpty();
    }

    bool reduce (const RectangleList& r) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        RectangleList totalArea (edgeTable->edgeTable.getMaximumBounds());
        totalArea.subtract (r);

        for (RectangleList::Iterator i (totalArea); i.next();)
            edgeTable->edgeTable.excludeRectangle (*i.getRectangle());

        return ! edgeTable->edgeTable.isEmpty();
    }

    bool exclude (int x, int y, int w, int h) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        edgeTable->edgeTable.excludeRectangle (Rectangle (x, y, w, h));
        return ! edgeTable->edgeTable.isEmpty();
    }

    bool reduce (const Path& p, const AffineTransform& transform) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        EdgeTable et (edgeTable->edgeTable.getMaximumBounds(), p, transform);
        edgeTable->edgeTable.clipToEdgeTable (et);
        return ! edgeTable->edgeTable.isEmpty();
    }

    bool reduce (const Image& image, int x, int y) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        edgeTable->edgeTable.clipToImageAlpha (image, x, y);
        return ! edgeTable->edgeTable.isEmpty();
    }

    bool reduce (const Image& image, const AffineTransform& transform) throw()
    {
        jassertfalse
        return true;
    }

    void fillEdgeTable (Image& image, EdgeTable& et, const bool replaceContents = false) throw()
    {
        et.clipToEdgeTable (edgeTable->edgeTable);

        Image::BitmapData destData (image, 0, 0, image.getWidth(), image.getHeight(), true);

        if (gradient != 0)
        {
            jassert (! replaceContents); // that option is just for solid colours

            ColourGradient g2 (*gradient);

            const bool isIdentity = g2.transform.isOnlyTranslation();
            if (isIdentity)
            {
                // If our translation doesn't involve any distortion, we can speed it up..
                const float tx = g2.transform.getTranslationX() + xOffset;
                const float ty = g2.transform.getTranslationY() + yOffset;

                g2.x1 += tx;
                g2.x2 += tx;
                g2.y1 += ty;
                g2.y2 += ty;
            }
            else
            {
                g2.transform = g2.transform.translated ((float) xOffset,
                                                        (float) yOffset);
            }

            int numLookupEntries;
            PixelARGB* const lookupTable = g2.createLookupTable (numLookupEntries);
            jassert (numLookupEntries > 0);

            if (image.getFormat() == Image::RGB)
            {
                jassert (destData.pixelStride == 3);

                if (g2.isRadial)
                {
                    if (isIdentity)
                    {
                        GradientEdgeTableRenderer <PixelRGB, RadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                    else
                    {
                        GradientEdgeTableRenderer <PixelRGB, TransformedRadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                }
                else
                {
                    GradientEdgeTableRenderer <PixelRGB, LinearGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                    et.iterate (renderer);
                }
            }
            else if (image.getFormat() == Image::ARGB)
            {
                jassert (destData.pixelStride == 4);

                if (g2.isRadial)
                {
                    if (isIdentity)
                    {
                        GradientEdgeTableRenderer <PixelARGB, RadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                    else
                    {
                        GradientEdgeTableRenderer <PixelARGB, TransformedRadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                }
                else
                {
                    GradientEdgeTableRenderer <PixelARGB, LinearGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                    et.iterate (renderer);
                }
            }
            else if (image.getFormat() == Image::SingleChannel)
            {
                jassert (destData.pixelStride == 4);

                if (g2.isRadial)
                {
                    if (isIdentity)
                    {
                        GradientEdgeTableRenderer <PixelAlpha, RadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                    else
                    {
                        GradientEdgeTableRenderer <PixelAlpha, TransformedRadialGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                        et.iterate (renderer);
                    }
                }
                else
                {
                    GradientEdgeTableRenderer <PixelAlpha, LinearGradientPixelGenerator> renderer (destData, g2, lookupTable, numLookupEntries);
                    et.iterate (renderer);
                }
            }

            juce_free (lookupTable);
        }
        else
        {
            const PixelARGB fillColour (colour.getPixelARGB());

            if (replaceContents)
            {
                if (image.getFormat() == Image::RGB)
                {
                    jassert (destData.pixelStride == 3);
                    SolidColourEdgeTableRenderer <PixelRGB, true> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
                else if (image.getFormat() == Image::ARGB)
                {
                    jassert (destData.pixelStride == 4);
                    SolidColourEdgeTableRenderer <PixelARGB, true> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
                else if (image.getFormat() == Image::SingleChannel)
                {
                    jassert (destData.pixelStride == 1);
                    SolidColourEdgeTableRenderer <PixelAlpha, true> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
            }
            else
            {
                if (image.getFormat() == Image::RGB)
                {
                    jassert (destData.pixelStride == 3);
                    SolidColourEdgeTableRenderer <PixelRGB, false> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
                else if (image.getFormat() == Image::ARGB)
                {
                    jassert (destData.pixelStride == 4);
                    SolidColourEdgeTableRenderer <PixelARGB, false> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
                else if (image.getFormat() == Image::SingleChannel)
                {
                    jassert (destData.pixelStride == 1);
                    SolidColourEdgeTableRenderer <PixelAlpha, false> renderer (destData, fillColour);
                    et.iterate (renderer);
                }
            }
        }
    }

    void renderImage (Image& destImage, const Image& sourceImage,
                      int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                      const AffineTransform& t) throw()
    {
        if (t.isSingularity())
            return;

        const bool betterQuality = (interpolationQuality != Graphics::lowResamplingQuality);
        const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));

        if (transform.isOnlyTranslation())
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            const int tx = (int) (t.getTranslationX() * 256.0f);
            const int ty = (int) (t.getTranslationY() * 256.0f);

            if ((! betterQuality) || ((tx | ty) & 224) == 0)
            {
                renderImage (destImage, sourceImage, (tx + 128) >> 8, (ty + 128) >> 8);
                return;
            }
        }

        Path p;
        p.addRectangle ((float) srcClipX, (float) srcClipY, (float) srcClipW, (float) srcClipH);

        EdgeTable et (edgeTable->edgeTable.getMaximumBounds(), p, transform);
        et.clipToEdgeTable (edgeTable->edgeTable);

        Image::BitmapData destData (destImage, 0, 0, destImage.getWidth(), destImage.getHeight(), true);
        Image::BitmapData srcData (sourceImage, 0, 0, sourceImage.getWidth(), sourceImage.getHeight());

        const int extraAlpha = colour.getAlpha();

        switch (sourceImage.getFormat())
        {
        case Image::ARGB:
            if (destImage.getFormat() == Image::ARGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelARGB, PixelARGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else if (destImage.getFormat() == Image::RGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelRGB, PixelARGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else
            {
                jassert (destImage.getFormat() == Image::SingleChannel)
                TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelARGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            break;

        case Image::RGB:
            if (destImage.getFormat() == Image::ARGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelARGB, PixelRGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else if (destImage.getFormat() == Image::RGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelRGB, PixelRGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else
            {
                jassert (destImage.getFormat() == Image::SingleChannel)
                TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelRGB> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            break;

        default:
            jassert (sourceImage.getFormat() == Image::SingleChannel);

            if (destImage.getFormat() == Image::ARGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelARGB, PixelAlpha> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else if (destImage.getFormat() == Image::RGB)
            {
                TransformedImageFillEdgeTableRenderer <PixelRGB, PixelAlpha> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            else
            {
                jassert (destImage.getFormat() == Image::SingleChannel)
                TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha> renderer (destData, srcData, transform, extraAlpha, betterQuality);
                et.iterate (renderer);
            }
            break;
        }
    }

    void renderImage (Image& destImage, const Image& sourceImage, int imageX, int imageY) throw()
    {
        EdgeTable et (Rectangle (imageX, imageY, sourceImage.getWidth(), sourceImage.getHeight())
                        .getIntersection (Rectangle (0, 0, destImage.getWidth(), destImage.getHeight())));
        et.clipToEdgeTable (edgeTable->edgeTable);

        if (et.isEmpty())
            return;

        Image::BitmapData destData (destImage, 0, 0, destImage.getWidth(), destImage.getHeight(), true);
        Image::BitmapData srcData (sourceImage, 0, 0, sourceImage.getWidth(), sourceImage.getHeight());
        srcData.data = srcData.getPixelPointer (-imageX, -imageY);

        const int alpha = colour.getAlpha();

        switch (destImage.getFormat())
        {
        case Image::RGB:
            if (sourceImage.getFormat() == Image::RGB)
            {
                ImageFillEdgeTableRenderer <PixelRGB, const PixelRGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else if (sourceImage.getFormat() == Image::ARGB)
            {
                ImageFillEdgeTableRenderer <PixelRGB, const PixelARGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else
            {
                ImageFillEdgeTableRenderer <PixelRGB, const PixelAlpha> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            break;

        case Image::ARGB:
            if (sourceImage.getFormat() == Image::RGB)
            {
                ImageFillEdgeTableRenderer <PixelARGB, const PixelRGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else if (sourceImage.getFormat() == Image::ARGB)
            {
                ImageFillEdgeTableRenderer <PixelARGB, const PixelARGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else
            {
                ImageFillEdgeTableRenderer <PixelARGB, const PixelAlpha> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            break;

        default:
            jassert (destImage.getFormat() == Image::SingleChannel);
            if (sourceImage.getFormat() == Image::RGB)
            {
                ImageFillEdgeTableRenderer <PixelAlpha, const PixelRGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else if (sourceImage.getFormat() == Image::ARGB)
            {
                ImageFillEdgeTableRenderer <PixelAlpha, const PixelARGB> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            else
            {
                ImageFillEdgeTableRenderer <PixelAlpha, const PixelAlpha> renderer (destData, srcData, alpha);
                et.iterate (renderer);
            }
            break;
        }
    }

    class EdgeTableHolder  : public ReferenceCountedObject
    {
    public:
        EdgeTableHolder (const EdgeTable& e) throw()
            : edgeTable (e)
        {}

        EdgeTable edgeTable;
    };

    ReferenceCountedObjectPtr<EdgeTableHolder> edgeTable;
    int xOffset, yOffset;
    Font font;
    Colour colour;
    ColourGradient* gradient;
    Graphics::ResamplingQuality interpolationQuality;

private:
    const LLGCSavedState& operator= (const LLGCSavedState&);

    void dupeEdgeTableIfMultiplyReferenced() throw()
    {
        if (edgeTable->getReferenceCount() > 1)
            edgeTable = new EdgeTableHolder (edgeTable->edgeTable);
    }
};


//==============================================================================
LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (Image& image_)
    : image (image_),
      stateStack (20)
{
    currentState = new LLGCSavedState (Rectangle (0, 0, image_.getWidth(), image_.getHeight()),
                                       0, 0, Font(), Colours::black, 0, Graphics::mediumResamplingQuality);
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer()
{
    delete currentState;
}

bool LowLevelGraphicsSoftwareRenderer::isVectorDevice() const
{
    return false;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setOrigin (int x, int y)
{
    currentState->xOffset += x;
    currentState->yOffset += y;
}

bool LowLevelGraphicsSoftwareRenderer::reduceClipRegion (int x, int y, int w, int h)
{
    return currentState->reduce (x + currentState->xOffset, y + currentState->yOffset, w, h);
}

bool LowLevelGraphicsSoftwareRenderer::reduceClipRegion (const RectangleList& clipRegion)
{
    RectangleList temp (clipRegion);
    temp.offsetAll (currentState->xOffset, currentState->yOffset);

    return currentState->reduce (temp);
}

void LowLevelGraphicsSoftwareRenderer::excludeClipRegion (int x, int y, int w, int h)
{
    currentState->exclude (x + currentState->xOffset, y + currentState->yOffset, w, h);
}

void LowLevelGraphicsSoftwareRenderer::clipToPath (const Path& path, const AffineTransform& transform)
{
}

void LowLevelGraphicsSoftwareRenderer::clipToImage (Image& image, int imageX, int imageY)
{
}

bool LowLevelGraphicsSoftwareRenderer::clipRegionIntersects (int x, int y, int w, int h)
{
    return currentState->edgeTable->edgeTable.getMaximumBounds()
                .intersects (Rectangle (x + currentState->xOffset, y + currentState->yOffset, w, h));
}

const Rectangle LowLevelGraphicsSoftwareRenderer::getClipBounds() const
{
    return currentState->edgeTable->edgeTable.getMaximumBounds().translated (-currentState->xOffset, -currentState->yOffset);
}

bool LowLevelGraphicsSoftwareRenderer::isClipEmpty() const
{
    return currentState->edgeTable->edgeTable.isEmpty();
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::saveState()
{
    stateStack.add (new LLGCSavedState (*currentState));
}

void LowLevelGraphicsSoftwareRenderer::restoreState()
{
    LLGCSavedState* const top = stateStack.getLast();

    if (top != 0)
    {
        delete currentState;
        currentState = top;
        stateStack.removeLast (1, false);
    }
    else
    {
        jassertfalse // trying to pop with an empty stack!
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setColour (const Colour& colour_)
{
    deleteAndZero (currentState->gradient);
    currentState->colour = colour_;
}

void LowLevelGraphicsSoftwareRenderer::setGradient (const ColourGradient& gradient_)
{
    delete currentState->gradient;
    currentState->gradient = new ColourGradient (gradient_);
}

void LowLevelGraphicsSoftwareRenderer::setOpacity (float opacity)
{
    currentState->colour = currentState->colour.withAlpha (opacity);
}

void LowLevelGraphicsSoftwareRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    currentState->interpolationQuality = quality;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillRect (int x, int y, int w, int h, const bool replaceExistingContents)
{
    x += currentState->xOffset;
    y += currentState->yOffset;

    if (Rectangle::intersectRectangles (x, y, w, h, 0, 0, image.getWidth(), image.getHeight()))
    {
        EdgeTable et (Rectangle (x, y, w, h));
        currentState->fillEdgeTable (image, et, replaceExistingContents);
    }
}

void LowLevelGraphicsSoftwareRenderer::fillPath (const Path& path, const AffineTransform& transform)
{
    EdgeTable et (currentState->edgeTable->edgeTable.getMaximumBounds(),
                  path, transform.translated ((float) currentState->xOffset,
                                              (float) currentState->yOffset));

    currentState->fillEdgeTable (image, et);
}

void LowLevelGraphicsSoftwareRenderer::fillPathWithImage (const Path& path, const AffineTransform& transform,
                                                          const Image& sourceImage, int imageX, int imageY)
{
    imageX += currentState->xOffset;
    imageY += currentState->yOffset;

    saveState();
    currentState->reduce (path, transform.translated ((float) currentState->xOffset, (float) currentState->yOffset));
    currentState->renderImage (image, sourceImage, imageX, imageY);
    restoreState();
}

void LowLevelGraphicsSoftwareRenderer::fillAlphaChannel (const Image& clipImage, int x, int y)
{
    x += currentState->xOffset;
    y += currentState->yOffset;

    Rectangle maxBounds (currentState->edgeTable->edgeTable.getMaximumBounds());
    EdgeTable et (maxBounds.getIntersection (Rectangle (x, y, clipImage.getWidth(), clipImage.getHeight())));
    et.clipToImageAlpha (clipImage, x, y);

    currentState->fillEdgeTable (image, et);
}

void LowLevelGraphicsSoftwareRenderer::fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                                                  const Image& fillerImage, int fillerImageX, int fillerImageY)
{
    alphaImageX += currentState->xOffset;
    alphaImageY += currentState->yOffset;
    fillerImageX += currentState->xOffset;
    fillerImageY += currentState->yOffset;

    saveState();
    currentState->reduce (alphaImage, alphaImageX, alphaImageY);
    currentState->renderImage (image, fillerImage, fillerImageX, fillerImageY);
    restoreState();
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::blendImage (const Image& sourceImage, int dx, int dy, int dw, int dh, int sx, int sy)
{
    dx += currentState->xOffset;
    dy += currentState->yOffset;

    saveState();
    currentState->reduce (dx, dy, dw, dh);
    currentState->renderImage (image, sourceImage, dx - sx, dy - sy);
    restoreState();
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::blendImageWarping (const Image& sourceImage,
                                                          int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                                          const AffineTransform& t)
{
    currentState->renderImage (image, sourceImage, srcClipX, srcClipY, srcClipW, srcClipH, t);
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::drawLine (double x1, double y1, double x2, double y2)
{
    Path p;
    p.addLineSegment ((float) x1, (float) y1, (float) x2, (float) y2, 1.0f);
    fillPath (p, AffineTransform::identity);
}

void LowLevelGraphicsSoftwareRenderer::drawVerticalLine (const int x, double top, double bottom)
{
    EdgeTable et ((float) (x + currentState->xOffset), (float) (top + currentState->yOffset), 1.0f, (float) (bottom - top));
    currentState->fillEdgeTable (image, et);
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontalLine (const int y, double left, double right)
{
    EdgeTable et ((float) (left + currentState->xOffset), (float) (y + currentState->yOffset),
                  (float) (right - left), 1.0f);
    currentState->fillEdgeTable (image, et);
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setFont (const Font& newFont)
{
    currentState->font = newFont;
}

void LowLevelGraphicsSoftwareRenderer::drawGlyph (int glyphNumber, float x, float y)
{
    currentState->font.renderGlyphIndirectly (*this, glyphNumber, x, y);
}

void LowLevelGraphicsSoftwareRenderer::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    currentState->font.renderGlyphIndirectly (*this, glyphNumber, transform);
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

END_JUCE_NAMESPACE
