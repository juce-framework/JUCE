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

class OpenGLFrameBufferImage final : public ImagePixelData
{
public:
    using Ptr = ReferenceCountedObjectPtr<OpenGLFrameBufferImage>;

    OpenGLFrameBufferImage (OpenGLContext& c, int w, int h)
        : ImagePixelData (Image::ARGB, w, h),
          context (c),
          pixelStride (4)
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
        bitmapData.pixelStride = pixelStride;

        auto releaser = std::make_unique<DataReleaser> (this, Rectangle { x, y, bitmapData.width, bitmapData.height }, mode);

        bitmapData.data = (uint8*) releaser->data.get();
        bitmapData.size = (size_t) bitmapData.width
                        * (size_t) bitmapData.height
                        * sizeof (PixelARGB);
        bitmapData.lineStride = (bitmapData.width * bitmapData.pixelStride + 3) & ~3;

        bitmapData.dataReleaser = std::move (releaser);

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    OpenGLContext& context;
    OpenGLFrameBuffer frameBuffer;

private:
    int pixelStride;

    struct DataReleaser final : public Image::BitmapData::BitmapDataReleaser
    {
        DataReleaser (Ptr selfIn, Rectangle<int> areaIn, Image::BitmapData::ReadWriteMode modeIn)
            : self (selfIn),
              data ((size_t) (areaIn.getWidth() * areaIn.getHeight())),
              area (areaIn),
              mode (modeIn)
        {
            if (mode != Image::BitmapData::writeOnly)
                self->frameBuffer.readPixels (data.get(), getArea());
        }

        ~DataReleaser() override
        {
            if (mode != Image::BitmapData::readOnly)
                self->frameBuffer.writePixels (data, getArea());
        }

        Rectangle<int> getArea() const
        {
            return area.withBottomY (self->frameBuffer.getHeight() - area.getY());
        }

        Ptr self;
        HeapBlock<PixelARGB> data;
        Rectangle<int> area;
        Image::BitmapData::ReadWriteMode mode;
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
    if (OpenGLFrameBufferImage* const glImage = dynamic_cast<OpenGLFrameBufferImage*> (image.getPixelData().get()))
        return &(glImage->frameBuffer);

    return nullptr;
}

} // namespace juce
