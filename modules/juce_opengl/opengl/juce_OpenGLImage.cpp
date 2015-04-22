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
        return frameBuffer.initialise (context, width, height);
    }

    LowLevelGraphicsContext* createLowLevelContext() override
    {
        sendDataChangeMessage();
        return createOpenGLGraphicsContext (context, frameBuffer);
    }

    ImageType* createType() const override     { return new OpenGLImageType(); }

    ImagePixelData* clone() override
    {
        OpenGLFrameBufferImage* im = new OpenGLFrameBufferImage (context, width, height);
        im->incReferenceCount();

        {
            Image newImage (im);
            Graphics g (newImage);
            g.drawImageAt (Image (this), 0, 0, false);
        }

        im->resetReferenceCount();
        return im;
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
            HeapBlock<PixelARGB> tempRow ((size_t) w);
            const size_t rowSize = sizeof (PixelARGB) * (size_t) w;

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
            HeapBlock<PixelARGB> invertedCopy ((size_t) (area.getWidth() * area.getHeight()));
            const size_t rowSize = sizeof (PixelARGB) * (size_t) area.getWidth();

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
            DataReleaser* r = new DataReleaser (frameBuffer, x, y, bitmapData.width, bitmapData.height);
            bitmapData.dataReleaser = r;

            bitmapData.data = (uint8*) r->data.getData();
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

    ScopedPointer<OpenGLFrameBufferImage> im (new OpenGLFrameBufferImage (*currentContext, width, height));

    if (! im->initialise())
        return ImagePixelData::Ptr();

    im->frameBuffer.clear (Colours::transparentBlack);
    return im.release();
}

OpenGLFrameBuffer* OpenGLImageType::getFrameBufferFrom (const Image& image)
{
    if (OpenGLFrameBufferImage* const glImage = dynamic_cast<OpenGLFrameBufferImage*> (image.getPixelData()))
        return &(glImage->frameBuffer);

    return nullptr;
}
