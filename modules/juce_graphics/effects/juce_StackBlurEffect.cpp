/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class StackBlur
{
public:
    explicit StackBlur (int radius)
        : blurRadius {radius}
    {
    }

    void setNumPixels (int newNumPixels)
    {
        numPixels = newNumPixels;
    }

    void setStride (int newStride)
    {
        stride = newStride;
    }

    void blur (uint8* pixels)
    {
        Queue queue {blurRadius, pixels[0]};

        const auto stackSize = std::pow(blurRadius + 1.0, 2.0);
        auto stack = static_cast<double>(pixels[0]) * stackSize;

        for (auto sourceIndex = stride; sourceIndex < (numPixels + blurRadius) * stride; sourceIndex += stride)
        {
            const auto sourcePixel = pixels[jlimit(0, (numPixels - 1) * stride, sourceIndex)];
            queue.add (sourcePixel);

            stack += queue.calculateStackDifference();

            const auto targetIndex = sourceIndex - (blurRadius * stride);

            if (targetIndex >= 0)
            {
                const auto alpha = jlimit (0.0, static_cast<double> (std::numeric_limits<uint8>::max()), std::round (stack / stackSize));
                const auto targetPixel = static_cast<uint8> (alpha);
                pixels[targetIndex] = targetPixel;
            }
        }
    }

private:
    //==============================================================================
    class Queue
    {
    public:
        Queue (int radius, uint8 initialValue)
            : size {static_cast<uint64> (radius) * 2 + 1}
            , buffer (size + 1, initialValue)
            , inBuffer {&buffer[static_cast<std::size_t> (radius) + 1], radius + 1}
            , outBuffer {buffer.data(), radius + 1}
        {
        }

        void add (uint8 valueToAdd)
        {
            outBuffer.write (inBuffer.front());
            inBuffer.write (valueToAdd);
        }

        double calculateStackDifference() const
        {
            return inBuffer.sum() - outBuffer.sum();
        }

        const uint64 size;

    private:
        //==============================================================================
        class RingBuffer
        {
        public:
            RingBuffer (uint8* sourceData, int bufferSize)
                : data {sourceData}
                , size {bufferSize}
            {
            }

            void write (uint8 value)
            {
                data[writeIndex] = value;

                if (++writeIndex >= size)
                    writeIndex = 0;
            }

            uint8 front() const
            {
                return data[writeIndex];
            }

            double sum() const
            {
                return std::accumulate (data, data + size, 0.0);
            }

        private:
            int writeIndex {0};
            uint8* data;
            const int size;
        };

        //==============================================================================
        std::vector<uint8> buffer;

        RingBuffer inBuffer;
        RingBuffer outBuffer;
    };

    //==============================================================================
    const int blurRadius;

    int numPixels {0};
    int stride {0};
};

//==============================================================================
void StackBlurEffect::setBlurRadius (int newBlurRadius)
{
    jassert (newBlurRadius > 0);
    blurRadius = newBlurRadius;
}

void StackBlurEffect::setUseThreadPool (bool shouldUseThreadPool, int jobsPerThread)
{
    threadPoolEnabled = shouldUseThreadPool;
    numJobsPerThread = jobsPerThread;
}

int getNumColourChannels (const Image& image)
{
    switch (image.getFormat())
    {
        case Image::PixelFormat::RGB:
            return 3;
        case Image::PixelFormat::ARGB:
            return 4;
        case Image::PixelFormat::SingleChannel:
            return 1;
        case Image::UnknownFormat:
        default:
            return 0;
    }
}

void StackBlurEffect::applyEffect (Image& sourceImage, Graphics& g, float scale, float alpha)
{
    // Create a copy of the image
    auto blurredImage = sourceImage.createCopy();

    // Blur each channel in the image
    for (auto channel = 0; channel < getNumColourChannels (blurredImage); channel++)
    {
        blurHorizontally (blurredImage, channel, scale);
        blurVertically (blurredImage, channel, scale);
    }

    // Draw the blurred image to the provided context
    blurredImage.multiplyAllAlphas (alpha);
    g.drawImageAt (blurredImage, 0, 0);
}

//==============================================================================
void StackBlurEffect::blurHorizontally (juce::Image& image, int channel, float scale)
{
    Image::BitmapData bitmapData { image, Image::BitmapData::readWrite };

    const auto numRows = image.getHeight();

    StackBlur blur {roundToInt (blurRadius * scale)};
    blur.setNumPixels (bitmapData.width);
    blur.setStride (bitmapData.pixelStride);

    for (auto i = 0; i < numRows; i += numJobsPerThread)
    {
        auto blurRow = [this, &bitmapData, i, channel, numRows, &blur]() {
            for (auto row = i; row < i + numJobsPerThread && row < numRows; row++)
            {
                auto* const pixels = bitmapData.getPixelPointer (0, row) + channel;
                blur.blur (pixels);
            }
        };

        if (threadPoolEnabled)
            threadPool.addJob (blurRow);
        else
            blurRow();
    }

    waitForAllThreadPoolJobsToFinish();
}

void StackBlurEffect::blurVertically (juce::Image& image, int channel, float scale)
{
    Image::BitmapData bitmapData { image, Image::BitmapData::readWrite };

    const auto numColumns = image.getWidth();

    StackBlur blur {roundToInt (blurRadius * scale)};
    blur.setNumPixels (bitmapData.height);
    blur.setStride (bitmapData.lineStride);

    for (auto i = 0; i < numColumns; i += numJobsPerThread)
    {
        auto blurColumn = [this, &bitmapData, i, channel, numColumns, &blur]() {
            for (auto column = i; column < i + numJobsPerThread && column < numColumns; column++)
            {
                auto* const pixels = bitmapData.getPixelPointer (column, 0) + channel;
                blur.blur (pixels);
            }
        };

        if (threadPoolEnabled)
            threadPool.addJob (blurColumn);
        else
            blurColumn();
    }

    waitForAllThreadPoolJobsToFinish();
}

void StackBlurEffect::waitForAllThreadPoolJobsToFinish() const
{
    while (threadPool.getNumJobs() > 0) {}
}

} // namespace juce
