/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

//==============================================================================
/**
    A type of ImagePixelData that stores its image data in an OpenGL
    framebuffer, allowing a JUCE Image object to wrap a framebuffer.

    By creating an Image from an instance of an OpenGLFrameBufferImage,
    you can then use a Graphics object to draw into the framebuffer using normal
    JUCE 2D operations.

    @see Image, ImageType, ImagePixelData, OpenGLFrameBuffer

    @tags{OpenGL}
*/
class JUCE_API  OpenGLImageType     : public ImageType
{
public:
    OpenGLImageType();
    ~OpenGLImageType() override;

    ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool shouldClearImage) const override;
    int getTypeID() const override;

    static OpenGLFrameBuffer* getFrameBufferFrom (const Image&);
};

} // namespace juce
