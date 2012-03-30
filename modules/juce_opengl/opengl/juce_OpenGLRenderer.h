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

#ifndef __JUCE_OPENGLRENDERER_JUCEHEADER__
#define __JUCE_OPENGLRENDERER_JUCEHEADER__

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
        Note that this callback will be made on a background thread, so make sure
        that your implementation is thread-safe.
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


#endif   // __JUCE_OPENGLRENDERER_JUCEHEADER__
