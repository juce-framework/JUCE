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

/** Creates a graphics context object that will render into the given OpenGL target. */
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&, int width, int height);

/** Creates a graphics context object that will render into the given OpenGL framebuffer. */
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&, OpenGLFrameBuffer&);

/** Creates a graphics context object that will render into the given OpenGL framebuffer,
    with the given size.
*/
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext&,
                                                                      unsigned int frameBufferID,
                                                                      int width, int height);


//==============================================================================
/**
    Used to create custom shaders for use with an openGL 2D rendering context.

    Given a GL-based rendering context, you can write a fragment shader that applies some
    kind of per-pixel effect.

    @tags{OpenGL}
*/
struct JUCE_API  OpenGLGraphicsContextCustomShader
{
    /** Creates a custom shader.

        The shader code will not be compiled until actually needed, so it's OK to call this
        constructor when no GL context is active.

        The code should be a normal fragment shader. As well as the usual GLSL variables, there is
        also an automatically declared varying vec2 called "pixelPos", which indicates the pixel
        position within the graphics context of the pixel being drawn. There is also a varying value
        "pixelAlpha", which indicates the alpha by which the pixel should be multiplied, so that the
        edges of any clip-region masks are anti-aliased correctly.
    */
    OpenGLGraphicsContextCustomShader (const String& fragmentShaderCode);

    /** Destructor. */
    ~OpenGLGraphicsContextCustomShader();

    /** Returns the program, if it has been linked and is active.
        This can be called when you're about to use fillRect, to set up any uniforms/textures that
        the program may require.
    */
    OpenGLShaderProgram* getProgram (LowLevelGraphicsContext&) const;

    /** Applies the shader to a rectangle within the graphics context. */
    void fillRect (LowLevelGraphicsContext&, Rectangle<int> area) const;

    /** Attempts to compile the program if necessary, and returns an error message if it fails. */
    Result checkCompilation (LowLevelGraphicsContext&);

    /** Returns the code that was used to create this object. */
    const String& getFragmentShaderCode() const noexcept           { return code; }

    /** Optional lambda that will be called when the shader is activated, to allow
        user code to do setup tasks.
    */
    std::function<void (OpenGLShaderProgram&)> onShaderActivated;

private:
    String code, hashName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLGraphicsContextCustomShader)
};

} // namespace juce
