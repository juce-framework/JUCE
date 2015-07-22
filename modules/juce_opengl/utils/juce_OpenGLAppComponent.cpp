/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

OpenGLAppComponent::OpenGLAppComponent()   : frameCounter (0)
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
