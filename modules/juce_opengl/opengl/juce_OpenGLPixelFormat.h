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

#ifndef __JUCE_OPENGLPIXELFORMAT_JUCEHEADER__
#define __JUCE_OPENGLPIXELFORMAT_JUCEHEADER__


//==============================================================================
/**
    Represents the various properties of an OpenGL pixel format.

    @see OpenGLContext::setPixelFormat
*/
class JUCE_API  OpenGLPixelFormat
{
public:
    //==============================================================================
    /** Creates an OpenGLPixelFormat.

        The default constructor just initialises the object as a simple 8-bit
        RGBA format.
    */
    OpenGLPixelFormat (int bitsPerRGBComponent = 8,
                       int alphaBits = 8,
                       int depthBufferBits = 16,
                       int stencilBufferBits = 0) noexcept;

    bool operator== (const OpenGLPixelFormat&) const noexcept;
    bool operator!= (const OpenGLPixelFormat&) const noexcept;

    //==============================================================================
    int redBits;          /**< The number of bits per pixel to use for the red channel. */
    int greenBits;        /**< The number of bits per pixel to use for the green channel. */
    int blueBits;         /**< The number of bits per pixel to use for the blue channel. */
    int alphaBits;        /**< The number of bits per pixel to use for the alpha channel. */

    int depthBufferBits;      /**< The number of bits per pixel to use for a depth buffer. */
    int stencilBufferBits;    /**< The number of bits per pixel to use for a stencil buffer. */

    int accumulationBufferRedBits;    /**< The number of bits per pixel to use for an accumulation buffer's red channel. */
    int accumulationBufferGreenBits;  /**< The number of bits per pixel to use for an accumulation buffer's green channel. */
    int accumulationBufferBlueBits;   /**< The number of bits per pixel to use for an accumulation buffer's blue channel. */
    int accumulationBufferAlphaBits;  /**< The number of bits per pixel to use for an accumulation buffer's alpha channel. */

    uint8 multisamplingLevel;         /**< The number of samples to use for full-scene multisampled anti-aliasing (if available). */
};


#endif   // __JUCE_OPENGLPIXELFORMAT_JUCEHEADER__
