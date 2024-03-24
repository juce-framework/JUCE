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
