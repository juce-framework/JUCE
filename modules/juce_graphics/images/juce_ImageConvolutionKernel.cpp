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

ImageConvolutionKernel::ImageConvolutionKernel (const int size_)
    : values ((size_t) (size_ * size_)),
      size (size_)
{
    clear();
}

ImageConvolutionKernel::~ImageConvolutionKernel()
{
}

//==============================================================================
float ImageConvolutionKernel::getKernelValue (const int x, const int y) const noexcept
{
    if (isPositiveAndBelow (x, size) && isPositiveAndBelow (y, size))
        return values [x + y * size];

    jassertfalse;
    return 0;
}

void ImageConvolutionKernel::setKernelValue (const int x, const int y, const float value) noexcept
{
    if (isPositiveAndBelow (x, size) && isPositiveAndBelow (y, size))
    {
        values [x + y * size] = value;
    }
    else
    {
        jassertfalse;
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
                                           const Image& sourceImage,
                                           const Rectangle<int>& destinationArea) const
{
    if (sourceImage == destImage)
    {
        destImage.duplicateIfShared();
    }
    else
    {
        if (sourceImage.getWidth() != destImage.getWidth()
             || sourceImage.getHeight() != destImage.getHeight()
             || sourceImage.getFormat() != destImage.getFormat())
        {
            jassertfalse;
            return;
        }
    }

    const Rectangle<int> area (destinationArea.getIntersection (destImage.getBounds()));

    if (area.isEmpty())
        return;

    const int right = area.getRight();
    const int bottom = area.getBottom();

    const Image::BitmapData destData (destImage, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                                      Image::BitmapData::writeOnly);
    uint8* line = destData.data;

    const Image::BitmapData srcData (sourceImage, Image::BitmapData::readOnly);

    if (destData.pixelStride == 4)
    {
        for (int y = area.getY(); y < bottom; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = area.getX(); x < right; ++x)
            {
                float c1 = 0;
                float c2 = 0;
                float c3 = 0;
                float c4 = 0;

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= srcData.height)
                        break;

                    if (sy >= 0)
                    {
                        int sx = x - (size >> 1);
                        const uint8* src = srcData.getPixelPointer (sx, sy);

                        for (int xx = 0; xx < size; ++xx)
                        {
                            if (sx >= srcData.width)
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
        for (int y = area.getY(); y < bottom; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = area.getX(); x < right; ++x)
            {
                float c1 = 0;
                float c2 = 0;
                float c3 = 0;

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= srcData.height)
                        break;

                    if (sy >= 0)
                    {
                        int sx = x - (size >> 1);
                        const uint8* src = srcData.getPixelPointer (sx, sy);

                        for (int xx = 0; xx < size; ++xx)
                        {
                            if (sx >= srcData.width)
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
    else if (destData.pixelStride == 1)
    {
        for (int y = area.getY(); y < bottom; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = area.getX(); x < right; ++x)
            {
                float c1 = 0;

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= srcData.height)
                        break;

                    if (sy >= 0)
                    {
                        int sx = x - (size >> 1);
                        const uint8* src = srcData.getPixelPointer (sx, sy);

                        for (int xx = 0; xx < size; ++xx)
                        {
                            if (sx >= srcData.width)
                                break;

                            if (sx >= 0)
                            {
                                const float kernelMult = values [xx + yy * size];
                                c1 += kernelMult * *src++;
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
            }
        }
    }
}
