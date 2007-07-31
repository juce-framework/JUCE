/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__
#define __JUCE_OPENGLCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"

// this is used to disable OpenGL, and is defined in juce_Config.h
#if JUCE_OPENGL || DOXYGEN


//==============================================================================
/**
    A component that contains an OpenGL canvas.

    Override this, add it to whatever component you want to, and use the renderOpenGL()
    method to draw its contents.

*/
class JUCE_API  OpenGLComponent  : public Component
{
public:
    //==============================================================================
    /** Creates an OpenGLComponent.

        @param componentToShareContextWith  if this is another OpenGLComponent, then
                                            the two contexts will have their OpenGL contexts
                                            shared
    */
    OpenGLComponent (OpenGLComponent* componentToShareContextWith = 0);

    /** Destructor. */
    ~OpenGLComponent();

    //==============================================================================
    /** Makes this component the current openGL context.

        You might want to use this in things like your resize() method, before calling
        GL commands.

        If this returns false, then the context isn't active, so you should avoid
        making any calls.
    */
    bool makeCurrentContextActive();

    /** Stops the current component being the active OpenGL context.

        This is the opposite of makeCurrentContextActive()
    */
    void makeCurrentContextInactive();

    /** Flips the openGL buffers over. */
    void swapBuffers();

    /** This replaces the normal paint() callback - use it to draw your openGL stuff.

        When this is called, makeCurrentContextActive() will already have been called
        for you, so you just need to draw.
    */
    virtual void renderOpenGL() = 0;

    /** This method is called when the component creates a new OpenGL context.

        A new context may be created when the component is first used, or when it
        is moved to a different window, or when the window is hidden and re-shown,
        etc.

        You can use this callback as an opportunity to set up things like textures
        that your context needs.

        When this callback happens, the context will already have been made current
        using the makeCurrentContextActive() method, so there's no need to call it
        again in your code.
    */
    virtual void newOpenGLContextCreated() = 0;


    //==============================================================================
    /** Calls the rendering callback, and swaps the buffers afterwards.

        Called by paint(), this can be overridden if you need to decouple the rendering
        from the paint callback and render with a different thread.

        Returns true if the operation succeeded.
    */
    virtual bool renderAndSwapBuffers();


    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

    juce_UseDebuggingNewOperator

private:
    friend class InternalGLContextHolder;
    void* internalData;

    OpenGLComponent (const OpenGLComponent&);
    const OpenGLComponent& operator= (const OpenGLComponent&);

    void internalRepaint (int x, int y, int w, int h);
};


#endif
#endif   // __JUCE_OPENGLCOMPONENT_JUCEHEADER__
