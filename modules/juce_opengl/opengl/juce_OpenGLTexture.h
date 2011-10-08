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

#ifndef __JUCE_OPENGLTEXTURE_JUCEHEADER__
#define __JUCE_OPENGLTEXTURE_JUCEHEADER__

/**
    Creates an openGL texture from an Image.
*/
class JUCE_API  OpenGLTexture
{
public:
    OpenGLTexture();
    ~OpenGLTexture();

    /** Creates a texture from the given image.
        Note that the image's width and height must both be a power-of-two.
    */
    void load (const Image& image);

    /** Creates a texture from a raw array of pixels. */
    void load (const PixelARGB* pixels, int width, int height);

    /** Frees the texture, if there is one. */
    void release();

    /** Binds the texture to the currently selected openGL context. */
    void bind() const;

    /** Unbinds the texture to the currently selected openGL context. */
    void unbind() const;

    /** Draws this texture into the current context, with the specified corner positions. */
    void draw2D (float x1, float y1,
                 float x2, float y2,
                 float x3, float y3,
                 float x4, float y4,
                 const Colour& colour) const;

    /** Draws this texture into the current context, with the specified corner positions. */
    void draw3D (float x1, float y1, float z1,
                 float x2, float y2, float z2,
                 float x3, float y3, float z3,
                 float x4, float y4, float z4,
                 const Colour& colour) const;

private:
    unsigned int textureID;
    int width, height;

    void create (int w, int h);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLTexture);
};

#endif   // __JUCE_OPENGLTEXTURE_JUCEHEADER__
