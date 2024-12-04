/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ImageConvolutionKernel::ImageConvolutionKernel (int sizeToUse)
    : values ((size_t) (sizeToUse * sizeToUse)),
      size (sizeToUse)
{
    clear();
}

ImageConvolutionKernel::~ImageConvolutionKernel() = default;

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
            auto cx = x - centre;
            auto cy = y - centre;

            values [x + y * size] = (float) std::exp (radiusFactor * (cx * cx + cy * cy));
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

    auto area = destinationArea.getIntersection (destImage.getBounds());

    if (area.isEmpty())
        return;

    auto right = area.getRight();
    auto bottom = area.getBottom();

    const Image::BitmapData destData (destImage, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                                      Image::BitmapData::writeOnly);
    uint8* line = destData.data;

    const Image::BitmapData srcData (sourceImage, Image::BitmapData::readOnly);

    const auto applyKernel = [&] (auto stride)
    {
        constexpr auto pixelStride = stride.value;

        for (int y = area.getY(); y < bottom; ++y)
        {
            uint8* dest = line;
            line += destData.lineStride;

            for (int x = area.getX(); x < right; ++x)
            {
                float sum[pixelStride]{};

                for (int yy = 0; yy < size; ++yy)
                {
                    const int sy = y + yy - (size >> 1);

                    if (sy >= srcData.height)
                        break;

                    if (sy < 0)
                        continue;

                    int sx = x - (size >> 1);
                    const uint8* src = srcData.getPixelPointer (sx, sy);

                    for (int xx = 0; xx < size; ++xx)
                    {
                        if (sx >= srcData.width)
                            break;

                        if (sx >= 0)
                        {
                            const auto kernelMult = values[xx + yy * size];

                            for (auto& s : sum)
                                s += kernelMult * *src++;
                        }
                        else
                        {
                            src += pixelStride;
                        }

                        ++sx;
                    }
                }

                for (const auto& s : sum)
                    *dest++ = (uint8) jmin (0xff, roundToInt (s));
            }
        }
    };

    switch (destData.pixelStride)
    {
        case 4:
            return applyKernel (std::integral_constant<size_t, 4>{});
        case 3:
            return applyKernel (std::integral_constant<size_t, 3>{});
        case 1:
            return applyKernel (std::integral_constant<size_t, 1>{});
    }
}

} // namespace juce
