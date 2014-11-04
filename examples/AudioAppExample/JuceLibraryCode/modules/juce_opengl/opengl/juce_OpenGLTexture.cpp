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

static int getAllowedTextureSize (int x)
{
   #if JUCE_OPENGL_ALLOW_NON_POWER_OF_TWO_TEXTURES
    return x;
   #else
    return nextPowerOfTwo (x);
   #endif
}

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
    static void flip (HeapBlock<PixelARGB>& dataCopy, const uint8* srcData, const int lineStride,
                      const int w, const int h)
    {
        dataCopy.malloc ((size_t) (w * h));

        for (int y = 0; y < h; ++y)
        {
            const PixelType* src = (const PixelType*) srcData;
            PixelARGB* const dst = (PixelARGB*) (dataCopy + w * (h - 1 - y));

            for (int x = 0; x < w; ++x)
            {
               #if JUCE_ANDROID
                PixelType s (src[x]);
                dst[x].setARGB (s.getAlpha(), s.getBlue(), s.getGreen(), s.getRed());
               #else
                dst[x].set (src[x]);
               #endif
            }


            srcData += lineStride;
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
        case Image::ARGB:           Flipper<PixelARGB> ::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH); break;
        case Image::RGB:            Flipper<PixelRGB>  ::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH); break;
        case Image::SingleChannel:  Flipper<PixelAlpha>::flip (dataCopy, srcData.data, srcData.lineStride, imageW, imageH); break;
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
    Flipper<PixelARGB>::flip (flippedCopy, (const uint8*) pixels, 4 * w, w, h);

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
