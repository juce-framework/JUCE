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

OpenGLTexture::OpenGLTexture()
    : textureID (0), width (0), height (0), ownerContext (nullptr)
{
}

OpenGLTexture::~OpenGLTexture()
{
    release();
}

bool OpenGLTexture::isValidSize (int width, int height)
{
    return isPowerOfTwo (width) && isPowerOfTwo (height);
}

void OpenGLTexture::create (const int w, const int h, const void* pixels, GLenum type, bool topLeft)
{
    ownerContext = OpenGLContext::getCurrentContext();

    // Texture objects can only be created when the current thread has an active OpenGL
    // context. You'll need to create this object in one of the OpenGLContext's callbacks.
    jassert (ownerContext != nullptr);

    if (textureID == 0)
    {
        JUCE_CHECK_OPENGL_ERROR
        glGenTextures (1, &textureID);
        glBindTexture (GL_TEXTURE_2D, textureID);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        auto glMagFilter = (GLint) (ownerContext->texMagFilter == OpenGLContext::linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        JUCE_CHECK_OPENGL_ERROR
    }
    else
    {
        glBindTexture (GL_TEXTURE_2D, textureID);
        JUCE_CHECK_OPENGL_ERROR;
    }

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    JUCE_CHECK_OPENGL_ERROR

    const auto textureNpotSupported = ownerContext->isTextureNpotSupported();

    const auto getAllowedTextureSize = [&] (int n)
    {
        return textureNpotSupported ? n : nextPowerOfTwo (n);
    };

    width  = getAllowedTextureSize (w);
    height = getAllowedTextureSize (h);

    const GLint internalformat = type == GL_ALPHA ? GL_ALPHA : GL_RGBA;

    if (width != w || height != h)
    {
        glTexImage2D (GL_TEXTURE_2D, 0, internalformat,
                      width, height, 0, type, GL_UNSIGNED_BYTE, nullptr);

        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, topLeft ? (height - h) : 0, w, h,
                         type, GL_UNSIGNED_BYTE, pixels);
    }
    else
    {
        glTexImage2D (GL_TEXTURE_2D, 0, internalformat,
                      w, h, 0, type, GL_UNSIGNED_BYTE, pixels);
    }

    JUCE_CHECK_OPENGL_ERROR
}

template <class PixelType>
struct Flipper
{
    static void flip (HeapBlock<PixelARGB>& dataCopy,
                      const uint8* srcData,
                      const int lineStride,
                      const int pixelStride,
                      const int w,
                      const int h)
    {
        dataCopy.malloc (w * h);

        for (int y = 0; y < h; ++y)
        {
            auto* srcLine = srcData + lineStride * y;
            auto* dstLine = dataCopy.get() + w * (h - 1 - y);

            for (int x = 0; x < w; ++x)
            {
                auto* srcPixel = srcLine + x * pixelStride;
                dstLine[x].set (*unalignedPointerCast<const PixelType*> (srcPixel));
            }
        }
    }
};

void OpenGLTexture::loadImage (const Image& image)
{
    const int imageW = image.getWidth();
    const int imageH = image.getHeight();

    HeapBlock<PixelARGB> dataCopy;
    Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    switch (srcData.pixelFormat)
    {
        case Image::ARGB:           Flipper<PixelARGB> ::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::RGB:            Flipper<PixelRGB>  ::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::SingleChannel:  Flipper<PixelAlpha>::flip (dataCopy, srcData.data, srcData.lineStride, srcData.pixelStride, imageW, imageH); break;
        case Image::UnknownFormat:
        default: break;
    }

    create (imageW, imageH, dataCopy, JUCE_RGBA_FORMAT, true);
}

void OpenGLTexture::loadARGB (const PixelARGB* pixels, const int w, const int h)
{
    create (w, h, pixels, JUCE_RGBA_FORMAT, false);
}

void OpenGLTexture::loadAlpha (const uint8* pixels, int w, int h)
{
    create (w, h, pixels, GL_ALPHA, false);
}

void OpenGLTexture::loadARGBFlipped (const PixelARGB* pixels, int w, int h)
{
    HeapBlock<PixelARGB> flippedCopy;
    Flipper<PixelARGB>::flip (flippedCopy, (const uint8*) pixels, 4 * w, 4, w, h);

    create (w, h, flippedCopy, JUCE_RGBA_FORMAT, true);
}

void OpenGLTexture::release()
{
    if (textureID != 0)
    {
        // If the texture is deleted while the owner context is not active, it's
        // impossible to delete it, so this will be a leak until the context itself
        // is deleted.
        jassert (ownerContext == OpenGLContext::getCurrentContext());

        if (ownerContext == OpenGLContext::getCurrentContext())
        {
            glDeleteTextures (1, &textureID);

            textureID = 0;
            width = 0;
            height = 0;
        }
    }
}

void OpenGLTexture::bind() const
{
    glBindTexture (GL_TEXTURE_2D, textureID);
}

void OpenGLTexture::unbind() const
{
    glBindTexture (GL_TEXTURE_2D, 0);
}

} // namespace juce
