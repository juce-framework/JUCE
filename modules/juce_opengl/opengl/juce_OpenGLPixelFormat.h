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
    Represents the various properties of an OpenGL pixel format.

    @see OpenGLContext::setPixelFormat

    @tags{OpenGL}
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

} // namespace juce
