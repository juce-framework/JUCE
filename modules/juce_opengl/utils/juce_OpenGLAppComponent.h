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
