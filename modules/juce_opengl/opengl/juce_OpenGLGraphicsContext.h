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

#ifndef JUCE_OPENGLGRAPHICSCONTEXT_H_INCLUDED
#define JUCE_OPENGLGRAPHICSCONTEXT_H_INCLUDED


/** Creates a graphics context object that will render into the given OpenGL target.
    The caller is responsible for deleting this object when no longer needed.
*/
LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& target,
                                                      int width, int height);

/** Creates a graphics context object that will render into the given OpenGL target.
    The caller is responsible for deleting this object when no longer needed.
*/
LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context,
                                                      OpenGLFrameBuffer& target);

/** Creates a graphics context object that will render into the given OpenGL target.
    The caller is responsible for deleting this object when no longer needed.
*/
LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context,
                                                      unsigned int frameBufferID,
                                                      int width, int height);


//==============================================================================
/**
    Used to create custom shaders for use with an openGL 2D rendering context.

    Given a GL-based rendering context, you can write a fragment shader that applies some
    kind of per-pixel effect.
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
    void fillRect (LowLevelGraphicsContext&, const Rectangle<int>& area) const;

    /** Attempts to compile the program if necessary, and returns an error message if it fails. */
    Result checkCompilation (LowLevelGraphicsContext&);

    /** Returns the code that was used to create this object. */
    const String& getFragmentShaderCode() const noexcept           { return code; }

private:
    String code, hashName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLGraphicsContextCustomShader)
};


#endif   // JUCE_OPENGLGRAPHICSCONTEXT_H_INCLUDED
