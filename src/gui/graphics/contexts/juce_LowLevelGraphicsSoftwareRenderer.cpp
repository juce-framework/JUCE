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
#include "../../../core/juce_Singleton.h"
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

    forcedinline void blendLine (PixelType* dest, const PixelARGB& colour, int width) const
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
    LinearGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform& transform, const PixelARGB* const lookupTable_, const int numEntries_)
        : lookupTable (lookupTable_), numEntries (numEntries_)
    {
        jassert (numEntries_ >= 0);
        float x1 = gradient.x1;
        float y1 = gradient.y1;
        float x2 = gradient.x2;
        float y2 = gradient.y2;

        if (! transform.isIdentity())
        {
            const Line l (x2, y2, x1, y1);
            const Point p3 = l.getPointAlongLine (0.0f, 100.0f);
            float x3 = p3.getX();
            float y3 = p3.getY();

            transform.transformPoint (x1, y1);
            transform.transformPoint (x2, y2);
            transform.transformPoint (x3, y3);

            const Line l2 (x2, y2, x3, y3);
            const float prop = l2.findNearestPointTo (x1, y1);
            const Point newP2 (l2.getPointAlongLineProportionally (prop));

            x2 = newP2.getX();
            y2 = newP2.getY();
        }

        vertical = fabs (x1 - x2) < 0.001f;
        horizontal = fabs (y1 - y2) < 0.001f;

        if (vertical)
        {
            scale = roundToInt ((numEntries << (int) numScaleBits) / (double) (y2 - y1));
            start = roundToInt (y1 * scale);
        }
        else if (horizontal)
        {
            scale = roundToInt ((numEntries << (int) numScaleBits) / (double) (x2 - x1));
            start = roundToInt (x1 * scale);
        }
        else
        {
            grad = (y2 - y1) / (double) (x1 - x2);
            yTerm = y1 - x1 / grad;
            scale = roundToInt ((numEntries << (int) numScaleBits) / (yTerm * grad - (y2 * grad - x2)));
            grad *= scale;
        }
    }

    forcedinline void setY (const int y) throw()
    {
        if (vertical)
            linePix = lookupTable [jlimit (0, numEntries, (y * scale - start) >> (int) numScaleBits)];
        else if (! horizontal)
            start = roundToInt ((y - yTerm) * grad);
    }

    forcedinline const PixelARGB getPixel (const int x) const throw()
    {
        return vertical ? linePix
                        : lookupTable [jlimit (0, numEntries, (x * scale - start) >> (int) numScaleBits)];
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
    RadialGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform&,
                                  const PixelARGB* const lookupTable_, const int numEntries_) throw()
        : lookupTable (lookupTable_),
          numEntries (numEntries_),
          gx1 (gradient.x1),
          gy1 (gradient.y1)
    {
        jassert (numEntries_ >= 0);
        const float gdx = gradient.x1 - gradient.x2;
        const float gdy = gradient.y1 - gradient.y2;
        maxDist = gdx * gdx + gdy * gdy;
        invScale = numEntries / sqrt (maxDist);
        jassert (roundToInt (sqrt (maxDist) * invScale) <= numEntries);
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

        return lookupTable [x >= maxDist ? numEntries : roundToInt (sqrt (x) * invScale)];
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
    TransformedRadialGradientPixelGenerator (const ColourGradient& gradient, const AffineTransform& transform,
                                             const PixelARGB* const lookupTable_, const int numEntries_) throw()
        : RadialGradientPixelGenerator (gradient, transform, lookupTable_, numEntries_),
          inverseTransform (transform.inverted())
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
            return lookupTable [jmin (numEntries, roundToInt (sqrt (x) * invScale))];
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
    GradientEdgeTableRenderer (const Image::BitmapData& destData_, const ColourGradient& gradient, const AffineTransform& transform,
                               const PixelARGB* const lookupTable_, const int numEntries_) throw()
        : GradientType (gradient, transform, lookupTable_, numEntries_ - 1),
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
static forcedinline int safeModulo (int n, const int divisor) throw()
{
    jassert (divisor > 0);
    n %= divisor;
    return (n < 0) ? (n + divisor) : n;
}

//==============================================================================
template <class DestPixelType, class SrcPixelType, bool repeatPattern>
class ImageFillEdgeTableRenderer
{
public:
    ImageFillEdgeTableRenderer (const Image::BitmapData& destData_,
                                const Image::BitmapData& srcData_,
                                const int extraAlpha_,
                                const int x, const int y) throw()
        : destData (destData_),
          srcData (srcData_),
          extraAlpha (extraAlpha_ + 1),
          xOffset (repeatPattern ? safeModulo (x, srcData_.width) - srcData_.width : x),
          yOffset (repeatPattern ? safeModulo (y, srcData_.height) - srcData_.height : y)
    {
    }

    forcedinline void setEdgeTableYPos (int y) throw()
    {
        linePixels = (DestPixelType*) destData.getLinePointer (y);

        y -= yOffset;
        if (repeatPattern)
        {
            jassert (y >= 0);
            y %= srcData.height;
        }

        sourceLineStart = (SrcPixelType*) srcData.getLinePointer (y);
    }

    forcedinline void handleEdgeTablePixel (int x, int alphaLevel) const throw()
    {
        alphaLevel = (alphaLevel * extraAlpha) >> 8;

        linePixels[x].blend (sourceLineStart [repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)], alphaLevel);
    }

    forcedinline void handleEdgeTableLine (int x, int width, int alphaLevel) const throw()
    {
        DestPixelType* dest = linePixels + x;
        alphaLevel = (alphaLevel * extraAlpha) >> 8;
        x -= xOffset;

        jassert (repeatPattern || (x >= 0 && x + width <= srcData.width));

        if (alphaLevel < 0xfe)
        {
            do
            {
                dest++ ->blend (sourceLineStart [repeatPattern ? (x++ % srcData.width) : x++], alphaLevel);
            } while (--width > 0);
        }
        else
        {
            if (repeatPattern)
            {
                do
                {
                    dest++ ->blend (sourceLineStart [x++ % srcData.width]);
                } while (--width > 0);
            }
            else
            {
                copyRow (dest, sourceLineStart + x, width);
            }
        }
    }

    void clipEdgeTableLine (EdgeTable& et, int x, int y, int width) throw()
    {
        jassert (x - xOffset >= 0 && x + width - xOffset <= srcData.width);
        SrcPixelType* s = (SrcPixelType*) srcData.getLinePointer (y - yOffset);
        uint8* mask = (uint8*) (s + x - xOffset);

        if (sizeof (SrcPixelType) == sizeof (PixelARGB))
            mask += PixelARGB::indexA;

        et.clipLineToMask (x, y, mask, sizeof (SrcPixelType), width);
    }

private:
    const Image::BitmapData& destData;
    const Image::BitmapData& srcData;
    const int extraAlpha, xOffset, yOffset;
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
template <class DestPixelType, class SrcPixelType, bool repeatPattern>
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
          maxY (srcData_.height - 1),
          scratchSize (2048)
    {
        scratchBuffer.malloc (scratchSize);
    }

    ~TransformedImageFillEdgeTableRenderer() throw()
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
        if (width > scratchSize)
        {
            scratchSize = width;
            scratchBuffer.malloc (scratchSize);
        }

        SrcPixelType* span = scratchBuffer;
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

    void clipEdgeTableLine (EdgeTable& et, int x, int y_, int width) throw()
    {
        if (width > scratchSize)
        {
            scratchSize = width;
            scratchBuffer.malloc (scratchSize);
        }

        uint8* mask = (uint8*) scratchBuffer;
        y = y_;
        generate ((SrcPixelType*) mask, x, width);

        if (sizeof (SrcPixelType) == sizeof (PixelARGB))
            mask += PixelARGB::indexA;

        et.clipLineToMask (x, y_, mask, sizeof (SrcPixelType), width);
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

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

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
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

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

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

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
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

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

            if (repeatPattern)
            {
                loResX = safeModulo (loResX, srcData.width);
                loResY = safeModulo (loResY, srcData.height);
            }

            if (betterQuality
                 && ((unsigned int) loResX) < (unsigned int) maxX
                 && ((unsigned int) loResY) < (unsigned int) maxY)
            {
                hiResX &= 255;
                hiResY &= 255;

                const uint8* src = this->srcData.getPixelPointer (loResX, loResY);
                uint32 c = 256 * 128;
                c += src[0] * ((256 - hiResX) * (256 - hiResY));
                c += src[1] * (hiResX * (256 - hiResY));
                src += this->srcData.lineStride;
                c += src[0] * ((256 - hiResX) * hiResY);
                c += src[1] * (hiResX * hiResY);

                *((uint8*) dest) = (uint8) c;
            }
            else
            {
                if (! repeatPattern)
                {
                    // Beyond the edges, just repeat the edge pixels and leave the anti-aliasing to be handled by the edgetable
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

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
    HeapBlock <SrcPixelType> scratchBuffer;
    int scratchSize;

    TransformedImageFillEdgeTableRenderer (const TransformedImageFillEdgeTableRenderer&);
    const TransformedImageFillEdgeTableRenderer& operator= (const TransformedImageFillEdgeTableRenderer&);
};

//==============================================================================
class LLGCSavedState
{
public:
    LLGCSavedState (const Rectangle& clip_, const int xOffset_, const int yOffset_,
                    const Font& font_, const FillType& fillType_,
                    const Graphics::ResamplingQuality interpolationQuality_) throw()
        : edgeTable (new EdgeTableHolder (EdgeTable (clip_))),
          xOffset (xOffset_), yOffset (yOffset_),
          font (font_), fillType (fillType_),
          interpolationQuality (interpolationQuality_)
    {
    }

    LLGCSavedState (const LLGCSavedState& other) throw()
        : edgeTable (other.edgeTable), xOffset (other.xOffset),
          yOffset (other.yOffset), font (other.font),
          fillType (other.fillType), interpolationQuality (other.interpolationQuality)
    {
    }

    ~LLGCSavedState() throw()
    {
    }

    bool clipToRectangle (const Rectangle& r) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        edgeTable->edgeTable.clipToRectangle (r.translated (xOffset, yOffset));
        return ! edgeTable->edgeTable.isEmpty();
    }

    bool clipToRectangleList (const RectangleList& r) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        RectangleList offsetList (r);
        offsetList.offsetAll (xOffset, yOffset);
        EdgeTable e2 (offsetList);
        edgeTable->edgeTable.clipToEdgeTable (e2);

        return ! edgeTable->edgeTable.isEmpty();
    }

    bool excludeClipRectangle (const Rectangle& r) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        edgeTable->edgeTable.excludeRectangle (r.translated (xOffset, yOffset));
        return ! edgeTable->edgeTable.isEmpty();
    }

    void clipToPath (const Path& p, const AffineTransform& transform) throw()
    {
        dupeEdgeTableIfMultiplyReferenced();
        EdgeTable et (edgeTable->edgeTable.getMaximumBounds(), p, transform.translated ((float) xOffset, (float) yOffset));
        edgeTable->edgeTable.clipToEdgeTable (et);
    }

    //==============================================================================
    void fillEdgeTable (Image& image, EdgeTable& et, const bool replaceContents = false) throw()
    {
        et.clipToEdgeTable (edgeTable->edgeTable);

        Image::BitmapData destData (image, 0, 0, image.getWidth(), image.getHeight(), true);

        if (fillType.isGradient())
        {
            jassert (! replaceContents); // that option is just for solid colours

            ColourGradient g2 (*(fillType.gradient));
            g2.multiplyOpacity (fillType.getOpacity());
            g2.x1 -= 0.5f; g2.y1 -= 0.5f;
            g2.x2 -= 0.5f; g2.y2 -= 0.5f;
            AffineTransform transform (fillType.transform.translated ((float) xOffset, (float) yOffset));
            const bool isIdentity = transform.isOnlyTranslation();

            if (isIdentity)
            {
                // If our translation doesn't involve any distortion, we can speed it up..
                transform.transformPoint (g2.x1, g2.y1);
                transform.transformPoint (g2.x2, g2.y2);
                transform = AffineTransform::identity;
            }

            HeapBlock <PixelARGB> lookupTable;
            const int numLookupEntries = g2.createLookupTable (transform, lookupTable);
            jassert (numLookupEntries > 0);

            switch (image.getFormat())
            {
                case Image::ARGB:   renderGradient (et, destData, g2, transform, lookupTable, numLookupEntries, isIdentity, (PixelARGB*) 0); break;
                case Image::RGB:    renderGradient (et, destData, g2, transform, lookupTable, numLookupEntries, isIdentity, (PixelRGB*) 0); break;
                default:            renderGradient (et, destData, g2, transform, lookupTable, numLookupEntries, isIdentity, (PixelAlpha*) 0); break;
            }
        }
        else if (fillType.isTiledImage())
        {
            renderImage (image, *(fillType.image), fillType.image->getBounds(), fillType.transform, &et);
        }
        else
        {
            const PixelARGB fillColour (fillType.colour.getPixelARGB());

            switch (image.getFormat())
            {
                case Image::ARGB:   renderSolidFill (et, destData, fillColour, replaceContents, (PixelARGB*) 0); break;
                case Image::RGB:    renderSolidFill (et, destData, fillColour, replaceContents, (PixelRGB*) 0); break;
                default:            renderSolidFill (et, destData, fillColour, replaceContents, (PixelAlpha*) 0); break;
            }
        }
    }

    //==============================================================================
    void renderImage (Image& destImage, const Image& sourceImage, const Rectangle& srcClip,
                      const AffineTransform& t, const EdgeTable* const tiledFillClipRegion) throw()
    {
        const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));

        const Image::BitmapData destData (destImage, 0, 0, destImage.getWidth(), destImage.getHeight(), true);
        const Image::BitmapData srcData (sourceImage, srcClip.getX(), srcClip.getY(), srcClip.getWidth(), srcClip.getHeight());
        const int alpha = fillType.colour.getAlpha();
        const bool betterQuality = (interpolationQuality != Graphics::lowResamplingQuality);

        if (transform.isOnlyTranslation())
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            int tx = (int) (transform.getTranslationX() * 256.0f);
            int ty = (int) (transform.getTranslationY() * 256.0f);

            if ((! betterQuality) || ((tx | ty) & 224) == 0)
            {
                tx = ((tx + 128) >> 8);
                ty = ((ty + 128) >> 8);

                if (tiledFillClipRegion != 0)
                {
                    blittedRenderImage (sourceImage, destImage, *tiledFillClipRegion, destData, srcData, alpha, tx, ty, true);
                }
                else
                {
                    EdgeTable et (Rectangle (tx, ty, srcClip.getWidth(), srcClip.getHeight()).getIntersection (destImage.getBounds()));
                    et.clipToEdgeTable (edgeTable->edgeTable);

                    if (! et.isEmpty())
                        blittedRenderImage (sourceImage, destImage, et, destData, srcData, alpha, tx, ty, false);
                }

                return;
            }
        }

        if (transform.isSingularity())
            return;

        if (tiledFillClipRegion != 0)
        {
            transformedRenderImage (sourceImage, destImage, *tiledFillClipRegion, destData, srcData, alpha, transform, betterQuality, true);
        }
        else
        {
            Path p;
            p.addRectangle (0.0f, 0.0f, (float) srcClip.getWidth(), (float) srcClip.getHeight());

            EdgeTable et (edgeTable->edgeTable.getMaximumBounds(), p, transform);
            et.clipToEdgeTable (edgeTable->edgeTable);

            if (! et.isEmpty())
                transformedRenderImage (sourceImage, destImage, et, destData, srcData, alpha, transform, betterQuality, false);
        }
    }

    //==============================================================================
    void clipToImageAlpha (const Image& image, const Rectangle& srcClip, const AffineTransform& t) throw()
    {
        if (! image.hasAlphaChannel())
        {
            Path p;
            p.addRectangle (srcClip);
            clipToPath (p, t);
            return;
        }

        dupeEdgeTableIfMultiplyReferenced();

        const AffineTransform transform (t.translated ((float) xOffset, (float) yOffset));
        const Image::BitmapData srcData (image, srcClip.getX(), srcClip.getY(), srcClip.getWidth(), srcClip.getHeight());
        const bool betterQuality = (interpolationQuality != Graphics::lowResamplingQuality);
        EdgeTable& et = edgeTable->edgeTable;

        if (transform.isOnlyTranslation())
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            const int tx = (int) (transform.getTranslationX() * 256.0f);
            const int ty = (int) (transform.getTranslationY() * 256.0f);

            if ((! betterQuality) || ((tx | ty) & 224) == 0)
            {
                const int imageX = ((tx + 128) >> 8);
                const int imageY = ((ty + 128) >> 8);

                if (image.getFormat() == Image::ARGB)
                    straightClipImage (et, srcData, imageX, imageY, (PixelARGB*)0);
                else
                    straightClipImage (et, srcData, imageX, imageY, (PixelAlpha*)0);

                return;
            }
        }

        if (transform.isSingularity())
        {
            et.clipToRectangle (Rectangle());
            return;
        }

        {
            Path p;
            p.addRectangle (0, 0, (float) srcData.width, (float) srcData.height);
            EdgeTable et2 (et.getMaximumBounds(), p, transform);
            et.clipToEdgeTable (et2);
        }

        if (! et.isEmpty())
        {
            if (image.getFormat() == Image::ARGB)
                transformedClipImage (et, srcData, transform, betterQuality, (PixelARGB*)0);
            else
                transformedClipImage (et, srcData, transform, betterQuality, (PixelAlpha*)0);
        }
    }

    template <class SrcPixelType>
    void transformedClipImage (EdgeTable& et, const Image::BitmapData& srcData, const AffineTransform& transform, const bool betterQuality, const SrcPixelType *) throw()
    {
        TransformedImageFillEdgeTableRenderer <SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, transform, 255, betterQuality);

        for (int y = 0; y < et.getMaximumBounds().getHeight(); ++y)
            renderer.clipEdgeTableLine (et, et.getMaximumBounds().getX(), y + et.getMaximumBounds().getY(),
                                        et.getMaximumBounds().getWidth());
    }

    template <class SrcPixelType>
    void straightClipImage (EdgeTable& et, const Image::BitmapData& srcData, int imageX, int imageY, const SrcPixelType *) throw()
    {
        Rectangle r (imageX, imageY, srcData.width, srcData.height);
        et.clipToRectangle (r);

        ImageFillEdgeTableRenderer <SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, 255, imageX, imageY);

        for (int y = 0; y < r.getHeight(); ++y)
            renderer.clipEdgeTableLine (et, r.getX(), y + r.getY(), r.getWidth());
    }

    //==============================================================================
    class EdgeTableHolder  : public ReferenceCountedObject
    {
    public:
        EdgeTableHolder (const EdgeTable& e) throw() : edgeTable (e) {}

        EdgeTable edgeTable;
    };

    ReferenceCountedObjectPtr<EdgeTableHolder> edgeTable;
    int xOffset, yOffset;
    Font font;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;

private:
    const LLGCSavedState& operator= (const LLGCSavedState&);

    void dupeEdgeTableIfMultiplyReferenced() throw()
    {
        if (edgeTable->getReferenceCount() > 1)
            edgeTable = new EdgeTableHolder (edgeTable->edgeTable);
    }

    //==============================================================================
    template <class DestPixelType>
    void renderGradient (EdgeTable& et, const Image::BitmapData& destData, const ColourGradient& g, const AffineTransform& transform,
                         const PixelARGB* const lookupTable, const int numLookupEntries, const bool isIdentity, DestPixelType*) throw()
    {
        jassert (destData.pixelStride == sizeof (DestPixelType));

        if (g.isRadial)
        {
            if (isIdentity)
            {
                GradientEdgeTableRenderer <DestPixelType, RadialGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
                et.iterate (renderer);
            }
            else
            {
                GradientEdgeTableRenderer <DestPixelType, TransformedRadialGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
                et.iterate (renderer);
            }
        }
        else
        {
            GradientEdgeTableRenderer <DestPixelType, LinearGradientPixelGenerator> renderer (destData, g, transform, lookupTable, numLookupEntries);
            et.iterate (renderer);
        }
    }

    //==============================================================================
    template <class DestPixelType>
    void renderSolidFill (EdgeTable& et, const Image::BitmapData& destData, const PixelARGB& fillColour, const bool replaceContents, DestPixelType*) throw()
    {
        jassert (destData.pixelStride == sizeof (DestPixelType));
        if (replaceContents)
        {
            SolidColourEdgeTableRenderer <DestPixelType, true> r (destData, fillColour);
            et.iterate (r);
        }
        else
        {
            SolidColourEdgeTableRenderer <DestPixelType, false> r (destData, fillColour);
            et.iterate (r);
        }
    }

    //==============================================================================
    void transformedRenderImage (const Image& srcImage, Image& destImage, const EdgeTable& et, const Image::BitmapData& destData, const Image::BitmapData& srcData,
                                 const int alpha, const AffineTransform& transform, const bool betterQuality, const bool repeatPattern) throw()
    {
        switch (destImage.getFormat())
        {
        case Image::ARGB:
            switch (srcImage.getFormat())
            {
            case Image::ARGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            case Image::RGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            default:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            }
            break;
        case Image::RGB:
            switch (srcImage.getFormat())
            {
            case Image::ARGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            case Image::RGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            default:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            }
            break;
        default:
            switch (srcImage.getFormat())
            {
            case Image::ARGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            case Image::RGB:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            default:
                if (repeatPattern) { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, true> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                else               { TransformedImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, false> r (destData, srcData, transform, alpha, betterQuality); et.iterate (r); }
                break;
            }
            break;
        }
    }

    //==============================================================================
    void blittedRenderImage (const Image& srcImage, Image& destImage, const EdgeTable& et, const Image::BitmapData& destData,
                             const Image::BitmapData& srcData, const int alpha, int x, int y, const bool repeatPattern) throw()
    {
        switch (destImage.getFormat())
        {
            case Image::ARGB:
                switch (srcImage.getFormat())
                {
                case Image::ARGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelARGB, PixelARGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelARGB, PixelARGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                case Image::RGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelARGB, PixelRGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelARGB, PixelRGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                default:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelARGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                }
            break;
            case Image::RGB:
                switch (srcImage.getFormat())
                {
                case Image::ARGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelRGB, PixelARGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelRGB, PixelARGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                case Image::RGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelRGB, PixelRGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelRGB, PixelRGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                default:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelRGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                }
            break;
            default:
                switch (srcImage.getFormat())
                {
                case Image::ARGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelAlpha, PixelARGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                case Image::RGB:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelAlpha, PixelRGB, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                default:
                    if (repeatPattern) { ImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, true> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    else               { ImageFillEdgeTableRenderer <PixelAlpha, PixelAlpha, false> r (destData, srcData, alpha, x, y); et.iterate (r); }
                    break;
                }
            break;
        }
    }
};


//==============================================================================
LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (Image& image_)
    : image (image_)
{
    currentState = new LLGCSavedState (image_.getBounds(), 0, 0, Font(),
                                       FillType(), Graphics::mediumResamplingQuality);
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer()
{
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

bool LowLevelGraphicsSoftwareRenderer::clipToRectangle (const Rectangle& r)
{
    return currentState->clipToRectangle (r);
}

bool LowLevelGraphicsSoftwareRenderer::clipToRectangleList (const RectangleList& clipRegion)
{
    return currentState->clipToRectangleList (clipRegion);
}

void LowLevelGraphicsSoftwareRenderer::excludeClipRectangle (const Rectangle& r)
{
    currentState->excludeClipRectangle (r);
}

void LowLevelGraphicsSoftwareRenderer::clipToPath (const Path& path, const AffineTransform& transform)
{
    currentState->clipToPath (path, transform);
}

void LowLevelGraphicsSoftwareRenderer::clipToImageAlpha (const Image& sourceImage, const Rectangle& srcClip, const AffineTransform& transform)
{
    currentState->clipToImageAlpha (sourceImage, srcClip, transform);
}

bool LowLevelGraphicsSoftwareRenderer::clipRegionIntersects (const Rectangle& r)
{
    return currentState->edgeTable->edgeTable.getMaximumBounds()
                .intersects (r.translated (currentState->xOffset, currentState->yOffset));
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
        currentState = top;
        stateStack.removeLast (1, false);
    }
    else
    {
        jassertfalse // trying to pop with an empty stack!
    }
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setFill (const FillType& fillType)
{
    currentState->fillType = fillType;
}

void LowLevelGraphicsSoftwareRenderer::setOpacity (float newOpacity)
{
    currentState->fillType.setOpacity (newOpacity);
}

void LowLevelGraphicsSoftwareRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    currentState->interpolationQuality = quality;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillRect (const Rectangle& r, const bool replaceExistingContents)
{
    const Rectangle& totalClip = currentState->edgeTable->edgeTable.getMaximumBounds();
    const Rectangle clipped (totalClip.getIntersection (r.translated (currentState->xOffset, currentState->yOffset)));

    if (! clipped.isEmpty())
    {
        EdgeTable et (clipped);
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

void LowLevelGraphicsSoftwareRenderer::drawImage (const Image& sourceImage, const Rectangle& srcClip,
                                                  const AffineTransform& transform, const bool fillEntireClipAsTiles)
{
    jassert (sourceImage.getBounds().contains (srcClip));

    currentState->renderImage (image, sourceImage, srcClip, transform,
                               fillEntireClipAsTiles ? &(currentState->edgeTable->edgeTable) : 0);
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
    if (bottom > top)
    {
        EdgeTable et ((float) (x + currentState->xOffset), (float) (top + currentState->yOffset), 1.0f, (float) (bottom - top));
        currentState->fillEdgeTable (image, et);
    }
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontalLine (const int y, double left, double right)
{
    if (right > left)
    {
        EdgeTable et ((float) (left + currentState->xOffset), (float) (y + currentState->yOffset),
                      (float) (right - left), 1.0f);
        currentState->fillEdgeTable (image, et);
    }
}

//==============================================================================
class GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache() throw()
        : accessCounter (0), hits (0), misses (0)
    {
        for (int i = 120; --i >= 0;)
            glyphs.add (new CachedGlyph());
    }

    ~GlyphCache() throw()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (GlyphCache);

    //==============================================================================
    void drawGlyph (LLGCSavedState& state, Image& image, const Font& font, const int glyphNumber, float x, float y) throw()
    {
        ++accessCounter;
        int oldestCounter = INT_MAX;
        CachedGlyph* oldest = 0;

        for (int i = glyphs.size(); --i >= 0;)
        {
            CachedGlyph* const glyph = glyphs.getUnchecked (i);

            if (glyph->glyph == glyphNumber && glyph->font == font)
            {
                ++hits;
                glyph->lastAccessCount = accessCounter;
                glyph->draw (state, image, x, y);
                return;
            }

            if (glyph->lastAccessCount <= oldestCounter)
            {
                oldestCounter = glyph->lastAccessCount;
                oldest = glyph;
            }
        }

        if (hits + ++misses > (glyphs.size() << 4))
        {
            if (misses * 2 > hits)
            {
                for (int i = 32; --i >= 0;)
                    glyphs.add (new CachedGlyph());
            }

            hits = misses = 0;
            oldest = glyphs.getLast();
        }

        jassert (oldest != 0);
        oldest->lastAccessCount = accessCounter;
        oldest->generate (font, glyphNumber);
        oldest->draw (state, image, x, y);
    }

    //==============================================================================
    class CachedGlyph
    {
    public:
        CachedGlyph() : glyph (0), lastAccessCount (0) {}
        ~CachedGlyph()  {}

        void draw (LLGCSavedState& state, Image& image, const float x, const float y) const throw()
        {
            if (edgeTable != 0)
            {
                EdgeTable et (*edgeTable);
                et.translate (x, roundToInt (y));
                state.fillEdgeTable (image, et, false);
            }
        }

        void generate (const Font& newFont, const int glyphNumber) throw()
        {
            font = newFont;
            glyph = glyphNumber;
            edgeTable = 0;

            Path glyphPath;
            font.getTypeface()->getOutlineForGlyph (glyphNumber, glyphPath);

            if (! glyphPath.isEmpty())
            {
                const float fontHeight = font.getHeight();
                const AffineTransform transform (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight));

                float px, py, pw, ph;
                glyphPath.getBoundsTransformed (transform.translated (0.0f, -0.5f), px, py, pw, ph);

                Rectangle clip ((int) floorf (px), (int) floorf (py),
                                roundToInt (pw) + 2, roundToInt (ph) + 2);

                edgeTable = new EdgeTable (clip, glyphPath, transform);
            }
        }

        int glyph, lastAccessCount;
        Font font;

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        ScopedPointer <EdgeTable> edgeTable;

        CachedGlyph (const CachedGlyph&);
        const CachedGlyph& operator= (const CachedGlyph&);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <CachedGlyph> glyphs;
    int accessCounter, hits, misses;

    GlyphCache (const GlyphCache&);
    const GlyphCache& operator= (const GlyphCache&);
};

juce_ImplementSingleton_SingleThreaded (GlyphCache);


void LowLevelGraphicsSoftwareRenderer::setFont (const Font& newFont)
{
    currentState->font = newFont;
}

const Font LowLevelGraphicsSoftwareRenderer::getFont()
{
    return currentState->font;
}

void LowLevelGraphicsSoftwareRenderer::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    Font& f = currentState->font;

    if (transform.isOnlyTranslation())
    {
        GlyphCache::getInstance()->drawGlyph (*currentState, image, f, glyphNumber,
                                              transform.getTranslationX() + (float) currentState->xOffset,
                                              transform.getTranslationY() + (float) currentState->yOffset);
    }
    else
    {
        Path p;
        f.getTypeface()->getOutlineForGlyph (glyphNumber, p);
        fillPath (p, AffineTransform::scale (f.getHeight() * f.getHorizontalScale(), f.getHeight()).followedBy (transform));
    }
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

END_JUCE_NAMESPACE
