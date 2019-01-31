/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    A base class for writing simple one-page graphical apps.

    A subclass can inherit from this and implement just a few methods such as
    paint() and mouse-handling. The base class provides some simple abstractions
    to take care of continuously repainting itself.

    @tags{OpenGL}
*/
class JUCE_API  OpenGLAppComponent   : public Component,
                                       private OpenGLRenderer
{
public:
    OpenGLAppComponent();

    /** Destructor. */
    ~OpenGLAppComponent() override;

    /** Returns the number of times that the render method has been called since
        the component started running.
    */
    int getFrameCounter() const noexcept        { return frameCounter; }

    /** This must be called from your subclass's destructor, to shut down
        the GL system and stop it calling render() before your class is destroyed.
    */
    void shutdownOpenGL();

    /** Implement this method to set up any GL objects that you need for rendering.
        The GL context will be active when this method is called.

        Note that because the GL context could be destroyed and re-created ad-hoc by
        the underlying platform, the shutdown() and initialise() calls could be called
        multiple times while your app is running. So don't make your code assume that
        this will only be called once!
    */
    virtual void initialise() = 0;

    /** Implement this method to free any GL objects that you created during rendering.
        The GL context will still be active when this method is called.

        Note that because the GL context could be destroyed and re-created ad-hoc by
        the underlying platform, the shutdown() and initialise() calls could be called
        multiple times while your app is running. So don't make your code assume that
        this will only be called once!
    */
    virtual void shutdown() = 0;

    /** Called to render your openGL.
        @see OpenGLRenderer::render()
    */
    virtual void render() = 0;

    /** The GL context */
    OpenGLContext openGLContext;

private:
    //==============================================================================
    int frameCounter = 0;

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLAppComponent)
};

} // namespace juce
