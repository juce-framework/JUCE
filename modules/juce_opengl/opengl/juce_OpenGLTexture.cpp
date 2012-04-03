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

OpenGLTexture::OpenGLTexture()
    : textureID (0), width (0), height (0)
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

void OpenGLTexture::create (const int w, const int h, const void* pixels, GLenum type)
{
    // Texture objects can only be created when the current thread has an active OpenGL
    // context. You'll need to create this object in one of the OpenGLContext's callbacks.
    jassert (OpenGLHelpers::isContextActive());

    jassert (isValidSize (w, h)); // Perhaps these dimensions must be a power-of-two?

    width  = w;
    height = h;

    if (textureID == 0)
    {
        JUCE_CHECK_OPENGL_ERROR
        glGenTextures (1, &textureID);
        glBindTexture (GL_TEXTURE_2D, textureID);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    glTexImage2D (GL_TEXTURE_2D, 0, type == GL_ALPHA ? GL_ALPHA : GL_RGBA,
                  w, h, 0, type, GL_UNSIGNED_BYTE, pixels);
    JUCE_CHECK_OPENGL_ERROR
}

template <class PixelType>
struct Flipper
{
    static void flip (HeapBlock<PixelARGB>& dataCopy, const uint8* srcData, const int lineStride,
                      const int w, const int h, const int textureW, const int textureH)
    {
        dataCopy.malloc (textureW * textureH);

        for (int y = 0; y < h; ++y)
        {
            const PixelType* src = (const PixelType*) srcData;
            PixelARGB* const dst = (PixelARGB*) (dataCopy + textureW * (textureH - 1 - y));

            for (int x = 0; x < w; ++x)
                dst[x].set (src[x]);

            srcData += lineStride;
        }
    }
};

void OpenGLTexture::loadImage (const Image& image)
{
    const int imageW = image.getWidth();
    const int imageH = image.getHeight();
    const int textureW = nextPowerOfTwo (imageW);
    const int textureH = nextPowerOfTwo (imageH);

    HeapBlock<PixelARGB> dataCopy;
    Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    switch (srcData.pixelFormat)
    {
        case Image::ARGB:           Flipper<PixelARGB> ::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH, textureW, textureH); break;
        case Image::RGB:            Flipper<PixelRGB>  ::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH, textureW, textureH); break;
        case Image::SingleChannel:  Flipper<PixelAlpha>::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH, textureW, textureH); break;
        default: break;
    }

    create (textureW, textureH, dataCopy, JUCE_RGBA_FORMAT);
}

void OpenGLTexture::loadARGB (const PixelARGB* pixels, const int w, const int h)
{
    jassert (isValidSize (w, h));
    create (w, h, pixels, JUCE_RGBA_FORMAT);
}

void OpenGLTexture::loadAlpha (const uint8* pixels, int w, int h)
{
    jassert (isValidSize (w, h));
    create (w, h, pixels, GL_ALPHA);
}

void OpenGLTexture::loadARGBFlipped (const PixelARGB* pixels, int w, int h)
{
    const int textureW = nextPowerOfTwo (w);
    const int textureH = nextPowerOfTwo (h);

    HeapBlock<PixelARGB> flippedCopy;
    Flipper<PixelARGB>::flip (flippedCopy, (const uint8*) pixels, 4 * w, w, h, textureW, textureH);

    loadARGB (flippedCopy, textureW, textureH);
}

void OpenGLTexture::release()
{
    if (textureID != 0)
    {
        if (OpenGLHelpers::isContextActive())
            glDeleteTextures (1, &textureID);

        textureID = 0;
        width = 0;
        height = 0;
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

#if JUCE_USE_OPENGL_FIXED_FUNCTION
void OpenGLTexture::draw2D (float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float x4, float y4,
                            const Colour& colour) const
{
    bind();
    OpenGLHelpers::drawQuad2D (x1, y1, x2, y2, x3, y3, x4, y4, colour);
    unbind();
}

void OpenGLTexture::draw3D (float x1, float y1, float z1,
                            float x2, float y2, float z2,
                            float x3, float y3, float z3,
                            float x4, float y4, float z4,
                            const Colour& colour) const
{
    bind();
    OpenGLHelpers::drawQuad3D (x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, colour);
    unbind();
}
#endif
