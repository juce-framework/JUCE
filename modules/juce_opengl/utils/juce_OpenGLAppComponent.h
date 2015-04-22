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

#ifndef JUCE_OPENGLAPPCOMPONENT_H_INCLUDED
#define JUCE_OPENGLAPPCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A base class for writing simple one-page graphical apps.

    A subclass can inherit from this and implement just a few methods such as
    paint() and mouse-handling. The base class provides some simple abstractions
    to take care of continuously repainting itself.
*/
class OpenGLAppComponent   : public Component,
                             private OpenGLRenderer
{
public:
    OpenGLAppComponent();
    ~OpenGLAppComponent();

    /** Returns the number of times that the render method has been called since
        the component started running.
    */
    int getFrameCounter() const noexcept        { return frameCounter; }

    void shutdownOpenGL();

    /** Implement this method to set up any GL objects that you need for rendering.
        The GL context will be active when this method is called.
    */
    virtual void initialise() = 0;

    /** Implement this method to free any GL objects that you created during rendering.
        The GL context will still be active when this method is called.
    */
    virtual void shutdown() = 0;

    /**
    */
    virtual void render() = 0;

    OpenGLContext openGLContext;

private:
    //==============================================================================

    int frameCounter;

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLAppComponent)
};


#endif   // JUCE_OPENGLAPPCOMPONENT_H_INCLUDED
