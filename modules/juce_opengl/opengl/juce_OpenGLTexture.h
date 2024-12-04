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
    Creates an openGL texture from an Image.

    @tags{OpenGL}
*/
class JUCE_API  OpenGLTexture
{
public:
    OpenGLTexture();
    ~OpenGLTexture();

    /** Creates a texture from the given image.

        Note that if the image's dimensions aren't a power-of-two, the texture may
        be created with a larger size.

        The image will be arranged so that its top-left corner is at texture
        coordinate (0, 1).
    */
    void loadImage (const Image& image);

    /** Creates a texture from a raw array of pixels.
        If width and height are not powers-of-two, the texture will be created with a
        larger size, and only the subsection (0, 0, width, height) will be initialised.
        The data is sent directly to the OpenGL driver without being flipped vertically,
        so the first pixel will be mapped onto texture coordinate (0, 0).
    */
    void loadARGB (const PixelARGB* pixels, int width, int height);

    /** Creates a texture from a raw array of pixels.
        This is like loadARGB, but will vertically flip the data so that the first
        pixel ends up at texture coordinate (0, 1), and if the width and height are
        not powers-of-two, it will compensate by using a larger texture size.
    */
    void loadARGBFlipped (const PixelARGB* pixels, int width, int height);

    /** Creates an alpha-channel texture from an array of alpha values.
        If width and height are not powers-of-two, the texture will be created with a
        larger size, and only the subsection (0, 0, width, height) will be initialised.
        The data is sent directly to the OpenGL driver without being flipped vertically,
        so the first pixel will be mapped onto texture coordinate (0, 0).
    */
    void loadAlpha (const uint8* pixels, int width, int height);

    /** Frees the texture, if there is one. */
    void release();

    /** Binds the texture to the currently active openGL context. */
    void bind() const;

    /** Unbinds the texture to the currently active openGL context. */
    void unbind() const;

    /** Returns the GL texture ID number. */
    GLuint getTextureID() const noexcept        { return textureID; }

    int getWidth() const noexcept               { return width; }
    int getHeight() const noexcept              { return height; }

    /** Returns true if a texture can be created with the given size.
        Some systems may require that the sizes are powers-of-two.
    */
    static bool isValidSize (int width, int height);

private:
    GLuint textureID;
    int width, height;
    OpenGLContext* ownerContext;

    void create (int w, int h, const void*, GLenum, bool topLeft);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLTexture)
};

} // namespace juce
