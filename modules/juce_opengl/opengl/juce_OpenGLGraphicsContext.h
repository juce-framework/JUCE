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
    /** Destructor. */
    ~OpenGLGraphicsContextCustomShader();

    /** Attempts to compile and return a new shader object. 
        This must be called only when an openGL context is active. It'll return nullptr
        if the code fails to compile or some other error occurs.
     
        The shader code should be a normal fragment shader. As well as the usual variables, there
        will be some extra ones: "frontColour", which is the colour that gets passed into the fillRect
        method, and "pixelPos", which is a vec2 indicating the pixel position within the graphics context
        of the pixel being drawn.
    */
    static OpenGLGraphicsContextCustomShader* create (LowLevelGraphicsContext&,
                                                      StringRef fragmentShaderCode);

    /** Applies the shader to a rectangle within the graphics context.
        NB: This will ignore any clip region that is active.
    */
    void fillRect (LowLevelGraphicsContext&, const Rectangle<int>& area, Colour colour) const;

private:
    struct Pimpl;
    friend struct Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;

    OpenGLGraphicsContextCustomShader (Pimpl*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLGraphicsContextCustomShader)
};


#endif   // JUCE_OPENGLGRAPHICSCONTEXT_H_INCLUDED
