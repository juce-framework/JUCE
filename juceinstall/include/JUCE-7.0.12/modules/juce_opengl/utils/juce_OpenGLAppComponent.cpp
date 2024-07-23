/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

OpenGLAppComponent::OpenGLAppComponent()
{
    setOpaque (true);
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true);
}

OpenGLAppComponent::~OpenGLAppComponent()
{
    // Before your subclass's destructor has completed, you must call
    // shutdownOpenGL() to release the GL context. (Otherwise there's
    // a danger that it may invoke a GL callback on your class while
    // it's in the process of being deleted.
    jassert (! openGLContext.isAttached());

    shutdownOpenGL();
}

void OpenGLAppComponent::shutdownOpenGL()
{
    openGLContext.detach();
}

void OpenGLAppComponent::newOpenGLContextCreated()
{
    initialise();
}

void OpenGLAppComponent::renderOpenGL()
{
    ++frameCounter;
    render();
}

void OpenGLAppComponent::openGLContextClosing()
{
    shutdown();
}

} // namespace juce
