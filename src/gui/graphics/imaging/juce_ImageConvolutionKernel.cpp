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

#include "juce_ImageConvolutionKernel.h"


//==============================================================================
ImageConvolutionKernel::ImageConvolutionKernel (const int size_)
    : values (size_ * size_),
      size (size_)
{
    clear();
}

ImageConvolutionKernel::~ImageConvolutionKernel()
{
}

//==============================================================================
void ImageConvolutionKernel::setKernelValue (const int x,
                                             const int y,
                                             const float value)
{
    if (((unsigned int) x) < (unsigned int) size
         && ((unsigned int) y) < (unsigned int) size)
    {
        values [x + y * size] = value;
    }
    else
    {
        jassertfalse
    }
}

void ImageConvolutionKernel::clear()
{
    for (int i = size * size; --i >= 0;)
        values[i] = 0;
}

void ImageConvolutionKernel::setOverallSum (const float desiredTotalSum)
{
    double currentTotal = 0.0;

    for (int i = size * size; --i >= 0;)
        currentTotal += values[i];

    rescaleAllValues ((float) (desiredTotalSum / currentTotal));
}

void ImageConvolutionKernel::rescaleAllValues (const float multiplier)
{
    for (int i = size * size; --i >= 0;)
        values[i] *= multiplier;
}

//==============================================================================
void ImageConvolutionKernel::createGaussianBlur (const float radius)
{
    const double radiusFactor = -1.0 / (radius * radius * 2);
    const int centre = size >> 1;

    for (int y = size; --y >= 0;)
    {
        for (int x = size; --x >= 0;)
        {
            const int cx = x - centre;
            const int cy = y - centre;

            values [x + y * size] = (float) exp (radiusFactor * (cx * cx + cy * cy));
        }
    }

    setOverallSum (1.0f);
}

//==============================================================================
void ImageConvolutionKernel::applyToImage (Image& destImage,
                                           const Image* sourceImage,
                                           int dx,
                                           int dy,
                                           int dw,
                                           int dh) const
{
    ScopedPointer <Image> imageCreated;

    if (sourceImage == 0)
    {
        sourceImage = imageCreated = destImage.createCopy();
    }
    else
    {
        jassert (sourceImage->getWidth() == destImage.getWidth()
                  && sourceImage->getHeight() == destImage.getHeight()
                  && sourceImage->getFormat() == destImage.getFormat());

        if (sourceImage->getWidth() != destImage.getWidth()
             || sourceImage->getHeight() != destImage.getHeight()
             || sourceImage->getFormat() != destImage.getFormat())
            return;
    }

    const int imageWidth = destImage.getWidth();
    const int imageHeight = destImage.getHeight();

    if (dx >= imageWidth || dy >= imageHeight)
        return;

    if (dx + dw > imageWidth)
        dw = imageWidth - dx;

    if (dy + dh > imageHeight)
        dh = imageHeight - dy;

    const int dx2 = dx + dw;
    const int dy2 = dy + dh;

    const Image::BitmapData destData (destImage, dx, dy, dw, dh, true);
    uint8* line = destData.data;

    const Image::BitmapData srcData (*sourceImage, 0, 0, sourceImage->getWidth(), sourceImage->getHeight());

    if (destData.pixelStride == 4)
    {
        for (int y = dy; y < dy2; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = dx; x < dx2; ++x)
            {
                float c1 = 0;
                float c2 = 0;
                float c3 = 0;
                float c4 = 0;

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= imageHeight)
                        break;

                    if (sy >= 0)
                    {
                        int sx = x - (size >> 1);
                        const uint8* src = srcData.getPixelPointer (sx, sy);

                        for (int xx = 0; xx < size; ++xx)
                        {
                            if (sx >= imageWidth)
                                break;

                            if (sx >= 0)
                            {
                                const float kernelMult = values [xx + yy * size];
                                c1 += kernelMult * *src++;
                                c2 += kernelMult * *src++;
                                c3 += kernelMult * *src++;
                                c4 += kernelMult * *src++;
                            }
                            else
                            {
                                src += 4;
                            }

                            ++sx;
                        }
                    }
                }

                *dest++ = (uint8) jmin (0xff, roundToInt (c1));
                *dest++ = (uint8) jmin (0xff, roundToInt (c2));
                *dest++ = (uint8) jmin (0xff, roundToInt (c3));
                *dest++ = (uint8) jmin (0xff, roundToInt (c4));
            }
        }
    }
    else if (destData.pixelStride == 3)
    {
        for (int y = dy; y < dy2; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = dx; x < dx2; ++x)
            {
                float c1 = 0;
                float c2 = 0;
                float c3 = 0;

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= imageHeight)
                        break;

                    if (sy >= 0)
                    {
                        int sx = x - (size >> 1);
                        const uint8* src = srcData.getPixelPointer (sx, sy);

                        for (int xx = 0; xx < size; ++xx)
                        {
                            if (sx >= imageWidth)
                                break;

                            if (sx >= 0)
                            {
                                const float kernelMult = values [xx + yy * size];
                                c1 += kernelMult * *src++;
                                c2 += kernelMult * *src++;
                                c3 += kernelMult * *src++;
                            }
                            else
                            {
                                src += 3;
                            }

                            ++sx;
                        }
                    }
                }

                *dest++ = (uint8) roundToInt (c1);
                *dest++ = (uint8) roundToInt (c2);
                *dest++ = (uint8) roundToInt (c3);
            }
        }
    }
}

END_JUCE_NAMESPACE
