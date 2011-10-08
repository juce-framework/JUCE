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

BEGIN_JUCE_NAMESPACE

OpenGLFrameBufferImage::OpenGLFrameBufferImage (Image::PixelFormat format, int width, int height)
    : Image::SharedImage (format, width, height),
      pixelStride (4),
      lineStride (width * pixelStride)
{
    frameBuffer.initialise (width, height);
    frameBuffer.clear (Colours::transparentBlack);
}

OpenGLFrameBufferImage::~OpenGLFrameBufferImage() {}

LowLevelGraphicsContext* OpenGLFrameBufferImage::createLowLevelContext()
{
    return new LowLevelGraphicsSoftwareRenderer (Image (this));
}

Image::SharedImage* OpenGLFrameBufferImage::clone()
{
    OpenGLFrameBufferImage* im = new OpenGLFrameBufferImage (getPixelFormat(), getWidth(), getHeight());
    im->incReferenceCount();

    {
        Image newImage (im);
        Graphics g (newImage);
        g.drawImageAt (Image (this), 0, 0, false);
    }

    im->resetReferenceCount();
    return im;
}

Image::ImageType OpenGLFrameBufferImage::getType() const
{
    return Image::NativeImage;
}

namespace OpenGLImageHelpers
{
    struct Dummy
    {
        Dummy (OpenGLFrameBuffer&, int, int, int, int) noexcept {}
        static void read (OpenGLFrameBuffer&, Image::BitmapData& , int, int) noexcept {}
        static void write (const void*) noexcept {}
    };

    struct Reader
    {
        static void read (OpenGLFrameBuffer& frameBuffer, Image::BitmapData& bitmapData, int x, int y)
        {
            frameBuffer.readPixels (bitmapData.data, Rectangle<int> (x, y, bitmapData.width, bitmapData.height));
        }
    };

    struct Writer
    {
        Writer (OpenGLFrameBuffer& frameBuffer_, int x, int y, int w, int h) noexcept
            : frameBuffer (frameBuffer_), area (x, y, w, h)
        {}

        void write (const void* const data) const noexcept
        {
            frameBuffer.writePixels (data, area);
        }

        OpenGLFrameBuffer& frameBuffer;
        const Rectangle<int> area;

        JUCE_DECLARE_NON_COPYABLE (Writer);
    };

    template <class ReaderType, class WriterType>
    struct DataReleaser  : public Image::BitmapData::BitmapDataReleaser
    {
        DataReleaser (OpenGLFrameBuffer& frameBuffer, size_t dataSize, int x, int y, int w, int h)
            : data (dataSize),
              writer (frameBuffer, x, y, w, h)
        {}

        ~DataReleaser()
        {
            writer.write (data);
        }

        static void initialise (OpenGLFrameBuffer& frameBuffer, Image::BitmapData& bitmapData, int x, int y)
        {
            DataReleaser* r = new DataReleaser (frameBuffer, bitmapData.lineStride * bitmapData.height,
                                                x, y, bitmapData.width, bitmapData.height);
            bitmapData.dataReleaser = r;
            bitmapData.data = r->data + x * bitmapData.pixelStride + y * bitmapData.lineStride;

            ReaderType::read (frameBuffer, bitmapData, x, y);
        }

        HeapBlock<uint8> data;
        WriterType writer;
    };
}

void OpenGLFrameBufferImage::initialiseBitmapData (Image::BitmapData& bitmapData, int x, int y,
                                                   Image::BitmapData::ReadWriteMode mode)
{
    using namespace OpenGLImageHelpers;

    bitmapData.pixelFormat = format;
    bitmapData.lineStride  = lineStride;
    bitmapData.pixelStride = pixelStride;

    switch (mode)
    {
        case Image::BitmapData::writeOnly:  DataReleaser<Dummy,  Writer>::initialise (frameBuffer, bitmapData, x, y); break;
        case Image::BitmapData::readOnly:   DataReleaser<Reader, Dummy> ::initialise (frameBuffer, bitmapData, x, y); break;
        case Image::BitmapData::readWrite:  DataReleaser<Reader, Writer>::initialise (frameBuffer, bitmapData, x, y); break;
        default:                            jassertfalse; break;
    }
}

END_JUCE_NAMESPACE
