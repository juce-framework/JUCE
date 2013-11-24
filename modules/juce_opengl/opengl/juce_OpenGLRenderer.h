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

#ifndef JUCE_OPENGLRENDERER_H_INCLUDED
#define JUCE_OPENGLRENDERER_H_INCLUDED


//==============================================================================
/**
    A base class that should be implemented by classes which want to render openGL
    on a background thread.

    @see OpenGLContext
*/
class JUCE_API  OpenGLRenderer
{
public:
    OpenGLRenderer() {}
    virtual ~OpenGLRenderer() {}

    /** Called when a new GL context has been created.
        You can use this as an opportunity to create your textures, shaders, etc.
        When the method is invoked, the new GL context will be active.
        Note that this callback will be made on a background thread, so make sure
        that your implementation is thread-safe.
    */
    virtual void newOpenGLContextCreated() = 0;

    /** Called when you should render the next openGL frame.
        Note that this callback will be made on a background thread, not the message
        thread, so make sure that your implementation is thread-safe.
        For information about how to trigger a render callback, see
        OpenGLContext::triggerRepaint() and OpenGLContext::setContinuousRepainting().
    */
    virtual void renderOpenGL() = 0;

    /** Called when the current openGL context is about to close.
        You can use this opportunity to release any GL resources that you may have
        created.

        Note that this callback will be made on a background thread, so make sure
        that your implementation is thread-safe.

        (Also note that on Android, this callback won't happen, because there's currently
        no way to implement it..)
    */
    virtual void openGLContextClosing() = 0;
};


#endif   // JUCE_OPENGLRENDERER_H_INCLUDED
