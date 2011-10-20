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

#ifndef __JUCE_OPENGLIMAGE_JUCEHEADER__
#define __JUCE_OPENGLIMAGE_JUCEHEADER__


//==============================================================================
/**
    A type of Image::SharedImage that stores its image data in an OpenGL
    framebuffer, allowing a JUCE Image object to wrap a framebuffer.

    By creating an Image from an instance of an OpenGLFrameBufferImage,
    you can then use a Graphics object to draw into the framebuffer using normal
    JUCE 2D operations.

    @see Image, Image::SharedImage, OpenGLFrameBuffer
*/
class JUCE_API  OpenGLFrameBufferImage   : public Image::SharedImage
{
public:
    OpenGLFrameBufferImage (int width, int height);

    /** Destructor. */
    ~OpenGLFrameBufferImage();

    /** The underlying framebuffer.
        Although this is exposed to allow access to use it as a texture, etc, be
        careful not to change its size while the image is using it.
    */
    OpenGLFrameBuffer frameBuffer;

    /** @internal */
    LowLevelGraphicsContext* createLowLevelContext();
    /** @internal */
    SharedImage* clone();
    /** @internal */
    Image::ImageType getType() const;
    /** @internal */
    void initialiseBitmapData (Image::BitmapData&, int, int, Image::BitmapData::ReadWriteMode);

private:
    int pixelStride, lineStride;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBufferImage);
};

#endif   // __JUCE_OPENGLIMAGE_JUCEHEADER__
