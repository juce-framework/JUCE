/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

extern XContext windowHandleXContext;

//==============================================================================
// Defined juce_linux_Windowing.cpp
void juce_LinuxAddRepaintListener (ComponentPeer*, Component* dummy);
void juce_LinuxRemoveRepaintListener (ComponentPeer*, Component* dummy);

//==============================================================================
class OpenGLContext::NativeContext
{
private:
    struct DummyComponent  : public Component
    {
        DummyComponent (OpenGLContext::NativeContext& nativeParentContext)
            : native (nativeParentContext)
        {
        }

        void handleCommandMessage (int commandId) override
        {
            if (commandId == 0)
                native.triggerRepaint();
        }

        OpenGLContext::NativeContext& native;
    };

public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& cPixelFormat,
                   void* shareContext,
                   bool useMultisamplingIn,
                   OpenGLVersion)
        : component (comp), contextToShareWith (shareContext), dummy (*this)
    {
        display = XWindowSystem::getInstance()->getDisplay();

        XWindowSystemUtilities::ScopedXLock xLock;

        X11Symbols::getInstance()->xSync (display, False);

        const std::vector<GLint> optionalAttribs
        {
            GLX_SAMPLE_BUFFERS, useMultisamplingIn ? 1 : 0,
            GLX_SAMPLES,        cPixelFormat.multisamplingLevel
        };

        if (! tryChooseVisual (cPixelFormat, optionalAttribs) && ! tryChooseVisual (cPixelFormat, {}))
            return;

        auto* peer = component.getPeer();
        jassert (peer != nullptr);

        auto windowH = (Window) peer->getNativeHandle();
        auto colourMap = X11Symbols::getInstance()->xCreateColormap (display, windowH, bestVisual->visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = embeddedWindowEventMask;

        auto glBounds = component.getTopLevelComponent()
                           ->getLocalArea (&component, component.getLocalBounds());

        glBounds = Desktop::getInstance().getDisplays().logicalToPhysical (glBounds);

        embeddedWindow = X11Symbols::getInstance()->xCreateWindow (display, windowH,
                                                                   glBounds.getX(), glBounds.getY(),
                                                                   (unsigned int) jmax (1, glBounds.getWidth()),
                                                                   (unsigned int) jmax (1, glBounds.getHeight()),
                                                                   0, bestVisual->depth,
                                                                   InputOutput,
                                                                   bestVisual->visual,
                                                                   CWBorderPixel | CWColormap | CWEventMask,
                                                                   &swa);

        X11Symbols::getInstance()->xSaveContext (display, (XID) embeddedWindow, windowHandleXContext, (XPointer) peer);

        X11Symbols::getInstance()->xMapWindow (display, embeddedWindow);
        X11Symbols::getInstance()->xFreeColormap (display, colourMap);

        X11Symbols::getInstance()->xSync (display, False);

        juce_LinuxAddRepaintListener (peer, &dummy);
    }

    ~NativeContext()
    {
        if (auto* peer = component.getPeer())
        {
            juce_LinuxRemoveRepaintListener (peer, &dummy);

            if (embeddedWindow != 0)
            {
                XWindowSystemUtilities::ScopedXLock xLock;

                X11Symbols::getInstance()->xUnmapWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xDestroyWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xSync (display, False);

                XEvent event;
                while (X11Symbols::getInstance()->xCheckWindowEvent (display,
                                                                     embeddedWindow,
                                                                     embeddedWindowEventMask,
                                                                     &event) == True)
                {
                }
            }
        }

        if (bestVisual != nullptr)
            X11Symbols::getInstance()->xFree (bestVisual);
    }

    bool initialiseOnRenderThread (OpenGLContext& c)
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        renderContext = glXCreateContext (display, bestVisual, (GLXContext) contextToShareWith, GL_TRUE);
        c.makeActive();
        context = &c;

        return true;
    }

    void shutdownOnRenderThread()
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        context = nullptr;
        deactivateCurrentContext();
        glXDestroyContext (display, renderContext);
        renderContext = nullptr;
    }

    bool makeActive() const noexcept
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        return renderContext != nullptr
                 && glXMakeCurrent (display, embeddedWindow, renderContext);
    }

    bool isActive() const noexcept
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        return glXGetCurrentContext() == renderContext && renderContext != nullptr;
    }

    static void deactivateCurrentContext()
    {
        if (auto* display = XWindowSystem::getInstance()->getDisplay())
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            glXMakeCurrent (display, None, nullptr);
        }
    }

    void swapBuffers()
    {
        XWindowSystemUtilities::ScopedXLock xLock;
        glXSwapBuffers (display, embeddedWindow);
    }

    void updateWindowPosition (Rectangle<int> newBounds)
    {
        bounds = newBounds;
        auto physicalBounds = Desktop::getInstance().getDisplays().logicalToPhysical (bounds);

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xMoveResizeWindow (display, embeddedWindow,
                                                      physicalBounds.getX(), physicalBounds.getY(),
                                                      (unsigned int) jmax (1, physicalBounds.getWidth()),
                                                      (unsigned int) jmax (1, physicalBounds.getHeight()));
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        if (numFramesPerSwap == swapFrames)
            return true;

        if (auto GLXSwapIntervalSGI
              = (PFNGLXSWAPINTERVALSGIPROC) OpenGLHelpers::getExtensionFunction ("glXSwapIntervalSGI"))
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            swapFrames = numFramesPerSwap;
            GLXSwapIntervalSGI (numFramesPerSwap);
            return true;
        }

        return false;
    }

    int getSwapInterval() const                 { return swapFrames; }
    bool createdOk() const noexcept             { return true; }
    void* getRawContext() const noexcept        { return renderContext; }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    void triggerRepaint()
    {
        if (context != nullptr)
            context->triggerRepaint();
    }

    struct Locker { Locker (NativeContext&) {} };

private:
    bool tryChooseVisual (const OpenGLPixelFormat& format, const std::vector<GLint>& optionalAttribs)
    {
        std::vector<GLint> allAttribs
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,         format.redBits,
            GLX_GREEN_SIZE,       format.greenBits,
            GLX_BLUE_SIZE,        format.blueBits,
            GLX_ALPHA_SIZE,       format.alphaBits,
            GLX_DEPTH_SIZE,       format.depthBufferBits,
            GLX_STENCIL_SIZE,     format.stencilBufferBits,
            GLX_ACCUM_RED_SIZE,   format.accumulationBufferRedBits,
            GLX_ACCUM_GREEN_SIZE, format.accumulationBufferGreenBits,
            GLX_ACCUM_BLUE_SIZE,  format.accumulationBufferBlueBits,
            GLX_ACCUM_ALPHA_SIZE, format.accumulationBufferAlphaBits
        };

        allAttribs.insert (allAttribs.end(), optionalAttribs.begin(), optionalAttribs.end());

        allAttribs.push_back (None);

        bestVisual = glXChooseVisual (display, X11Symbols::getInstance()->xDefaultScreen (display), allAttribs.data());

        return bestVisual != nullptr;
    }

    static constexpr int embeddedWindowEventMask = ExposureMask | StructureNotifyMask;

    Component& component;
    GLXContext renderContext = {};
    Window embeddedWindow = {};

    int swapFrames = 1;
    Rectangle<int> bounds;
    XVisualInfo* bestVisual = nullptr;
    void* contextToShareWith;

    OpenGLContext* context = nullptr;
    DummyComponent dummy;

    ::Display* display = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    XWindowSystemUtilities::ScopedXLock xLock;
    return glXGetCurrentContext() != nullptr;
}

} // namespace juce
