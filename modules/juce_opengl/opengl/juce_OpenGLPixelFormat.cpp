/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

OpenGLPixelFormat::OpenGLPixelFormat (const int bitsPerRGBComponent,
                                      const int alphaBits_,
                                      const int depthBufferBits_,
                                      const int stencilBufferBits_) noexcept
    : redBits (bitsPerRGBComponent),
      greenBits (bitsPerRGBComponent),
      blueBits (bitsPerRGBComponent),
      alphaBits (alphaBits_),
      depthBufferBits (depthBufferBits_),
      stencilBufferBits (stencilBufferBits_),
      accumulationBufferRedBits (0),
      accumulationBufferGreenBits (0),
      accumulationBufferBlueBits (0),
      accumulationBufferAlphaBits (0),
      multisamplingLevel (0)
{
}

bool OpenGLPixelFormat::operator== (const OpenGLPixelFormat& other) const noexcept
{
    return redBits == other.redBits
            && greenBits == other.greenBits
            && blueBits  == other.blueBits
            && alphaBits == other.alphaBits
            && depthBufferBits == other.depthBufferBits
            && stencilBufferBits == other.stencilBufferBits
            && accumulationBufferRedBits   == other.accumulationBufferRedBits
            && accumulationBufferGreenBits == other.accumulationBufferGreenBits
            && accumulationBufferBlueBits  == other.accumulationBufferBlueBits
            && accumulationBufferAlphaBits == other.accumulationBufferAlphaBits
            && multisamplingLevel == other.multisamplingLevel;
}

bool OpenGLPixelFormat::operator!= (const OpenGLPixelFormat& other) const noexcept
{
    return ! operator== (other);
}
