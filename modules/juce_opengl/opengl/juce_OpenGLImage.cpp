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

class OpenGLFrameBufferImage   : public ImagePixelData
{
public:
    OpenGLFrameBufferImage (OpenGLContext& c, int w, int h)
        : ImagePixelData (Image::ARGB, w, h),
          context (c),
          pixelStride (4),
          lineStride (width * pixelStride)
    {
    }

    bool initialise()
    {
        if (! frameBuffer.initialise (context, width, height))
            return false;

        frameBuffer.clear (Colours::transparentBlack);
        return true;
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return createOpenGLGraphicsContext (context, frameBuffer);
    }

    std::unique_ptr<ImageType> createType() const override     { return std::make_unique<OpenGLImageType>(); }

    ImagePixelData::Ptr clone() override
    {
        std::unique_ptr<OpenGLFrameBufferImage> im (new OpenGLFrameBufferImage (context, width, height));

        if (! im->initialise())
            return ImagePixelData::Ptr();

        Image newImage (im.release());
        Graphics g (newImage);
        g.drawImageAt (Image (*this), 0, 0, false);

        return ImagePixelData::Ptr (newImage.getPixelData());
    }

    void initialiseBitmapData (Image::BitmapData& bitmapData, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        bitmapData.pixelFormat = pixelFormat;
        bitmapData.lineStride  = lineStride;
        bitmapData.pixelStride = pixelStride;

        switch (mode)
        {
            case Image::BitmapData::writeOnly:  DataReleaser<Dummy,  Writer>::initialise (frameBuffer, bitmapData, x, y); break;
            case Image::BitmapData::readOnly:   DataReleaser<Reader, Dummy> ::initialise (frameBuffer, bitmapData, x, y); break;
            case Image::BitmapData::readWrite:  DataReleaser<Reader, Writer>::initialise (frameBuffer, bitmapData, x, y); break;
            default:                            jassertfalse; break;
        }

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    OpenGLContext& context;
    OpenGLFrameBuffer frameBuffer;

private:
    int pixelStride, lineStride;

    struct Dummy
    {
        Dummy (OpenGLFrameBuffer&, int, int, int, int) noexcept {}
        static void read (OpenGLFrameBuffer&, Image::BitmapData& , int, int) noexcept {}
        static void write (const PixelARGB*) noexcept {}
    };

    struct Reader
    {
        static void read (OpenGLFrameBuffer& frameBuffer, Image::BitmapData& bitmapData, int x, int y)
        {
            frameBuffer.readPixels ((PixelARGB*) bitmapData.data,
                                    Rectangle<int> (x, frameBuffer.getHeight() - (y + bitmapData.height), bitmapData.width, bitmapData.height));

            verticalRowFlip ((PixelARGB*) bitmapData.data, bitmapData.width, bitmapData.height);
        }

        static void verticalRowFlip (PixelARGB* const data, const int w, const int h)
        {
            HeapBlock<PixelARGB> tempRow (w);
            auto rowSize = (size_t) w * sizeof (PixelARGB);

            for (int y = 0; y < h / 2; ++y)
            {
                PixelARGB* const row1 = data + y * w;
                PixelARGB* const row2 = data + (h - 1 - y) * w;
                memcpy (tempRow, row1, rowSize);
                memcpy (row1, row2, rowSize);
                memcpy (row2, tempRow, rowSize);
            }
        }
    };

    struct Writer
    {
        Writer (OpenGLFrameBuffer& fb, int x, int y, int w, int h) noexcept
            : frameBuffer (fb), area (x, y, w, h)
        {}

        void write (const PixelARGB* const data) const noexcept
        {
            HeapBlock<PixelARGB> invertedCopy (area.getWidth() * area.getHeight());
            auto rowSize = (size_t) area.getWidth() * sizeof (PixelARGB);

            for (int y = 0; y < area.getHeight(); ++y)
                memcpy (invertedCopy + area.getWidth() * y,
                        data + area.getWidth() * (area.getHeight() - 1 - y), rowSize);

            frameBuffer.writePixels (invertedCopy, area);
        }

        OpenGLFrameBuffer& frameBuffer;
        const Rectangle<int> area;

        JUCE_DECLARE_NON_COPYABLE (Writer)
    };

    template <class ReaderType, class WriterType>
    struct DataReleaser  : public Image::BitmapData::BitmapDataReleaser
    {
        DataReleaser (OpenGLFrameBuffer& fb, int x, int y, int w, int h)
            : data ((size_t) (w * h)),
              writer (fb, x, y, w, h)
        {}

        ~DataReleaser()
        {
            writer.write (data);
        }

        static void initialise (OpenGLFrameBuffer& frameBuffer, Image::BitmapData& bitmapData, int x, int y)
        {
            auto* r = new DataReleaser (frameBuffer, x, y, bitmapData.width, bitmapData.height);
            bitmapData.dataReleaser.reset (r);

            bitmapData.data = (uint8*) r->data.get();
            bitmapData.lineStride = (bitmapData.width * bitmapData.pixelStride + 3) & ~3;

            ReaderType::read (frameBuffer, bitmapData, x, y);
        }

        HeapBlock<PixelARGB> data;
        WriterType writer;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBufferImage)
};


//==============================================================================
OpenGLImageType::OpenGLImageType() {}
OpenGLImageType::~OpenGLImageType() {}

int OpenGLImageType::getTypeID() const
{
    return 3;
}

ImagePixelData::Ptr OpenGLImageType::create (Image::PixelFormat, int width, int height, bool /*shouldClearImage*/) const
{
    OpenGLContext* currentContext = OpenGLContext::getCurrentContext();
    jassert (currentContext != nullptr); // an OpenGL image can only be created when a valid context is active!

    std::unique_ptr<OpenGLFrameBufferImage> im (new OpenGLFrameBufferImage (*currentContext, width, height));

    if (! im->initialise())
        return ImagePixelData::Ptr();

    return *im.release();
}

OpenGLFrameBuffer* OpenGLImageType::getFrameBufferFrom (const Image& image)
{
    if (OpenGLFrameBufferImage* const glImage = dynamic_cast<OpenGLFrameBufferImage*> (image.getPixelData()))
        return &(glImage->frameBuffer);

    return nullptr;
}

} // namespace juce
