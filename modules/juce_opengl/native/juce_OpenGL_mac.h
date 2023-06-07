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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare,
                   bool shouldUseMultisampling,
                   OpenGLVersion version)
        : owner (component)
    {
        const auto attribs = createAttribs (version, pixFormat, shouldUseMultisampling);

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs.data()];

        static MouseForwardingNSOpenGLViewClass cls;
        view = [cls.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       pixelFormat: format];

        if ([view respondsToSelector: @selector (setWantsBestResolutionOpenGLSurface:)])
            [view setWantsBestResolutionOpenGLSurface: YES];

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter] addObserver: view
                                                 selector: @selector (_surfaceNeedsUpdate:)
                                                     name: NSViewGlobalFrameDidChangeNotification
                                                   object: view];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        renderContext = [[[NSOpenGLContext alloc] initWithFormat: format
                                                    shareContext: (NSOpenGLContext*) contextToShare] autorelease];

        [view setOpenGLContext: renderContext];
        [format release];

        viewAttachment = NSViewComponent::attachViewToComponent (component, view);
    }

    ~NativeContext()
    {
        [[NSNotificationCenter defaultCenter] removeObserver: view];
        [renderContext clearDrawable];
        [renderContext setView: nil];
        [view setOpenGLContext: nil];
        [view release];
    }

    static std::vector<NSOpenGLPixelFormatAttribute> createAttribs (OpenGLVersion version,
                                                                    const OpenGLPixelFormat& pixFormat,
                                                                    bool shouldUseMultisampling)
    {
        std::vector<NSOpenGLPixelFormatAttribute> attribs
        {
            NSOpenGLPFAOpenGLProfile, [version]
            {
                if (version == openGL3_2)
                    return NSOpenGLProfileVersion3_2Core;

                if (version != defaultGLVersion)
                    if (@available (macOS 10.10, *))
                        return NSOpenGLProfileVersion4_1Core;

                return NSOpenGLProfileVersionLegacy;
            }(),
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.redBits + pixFormat.greenBits + pixFormat.blueBits),
            NSOpenGLPFAAlphaSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.alphaBits),
            NSOpenGLPFADepthSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.depthBufferBits),
            NSOpenGLPFAStencilSize, static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.stencilBufferBits),
            NSOpenGLPFAAccumSize,   static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.accumulationBufferRedBits  + pixFormat.accumulationBufferGreenBits
                                                                             + pixFormat.accumulationBufferBlueBits + pixFormat.accumulationBufferAlphaBits)
        };

        if (shouldUseMultisampling)
        {
            attribs.insert (attribs.cend(),
            {
                NSOpenGLPFAMultisample,
                NSOpenGLPFASampleBuffers,   static_cast<NSOpenGLPixelFormatAttribute> (1),
                NSOpenGLPFASamples,         static_cast<NSOpenGLPixelFormatAttribute> (pixFormat.multisamplingLevel)
            });
        }

        attribs.push_back (0);

        return attribs;
    }

    InitResult initialiseOnRenderThread (OpenGLContext&)  { return InitResult::success; }
    void shutdownOnRenderThread()                         { deactivateCurrentContext(); }

    bool createdOk() const noexcept                   { return getRawContext() != nullptr; }
    NSOpenGLView* getNSView() const noexcept          { return view; }
    NSOpenGLContext* getRawContext() const noexcept   { return renderContext; }
    GLuint getFrameBufferID() const noexcept          { return 0; }

    bool makeActive() const noexcept
    {
        jassert (renderContext != nil);

        if ([renderContext view] != view)
            [renderContext setView: view];

        if (NSOpenGLContext* context = [view openGLContext])
        {
            [context makeCurrentContext];
            return true;
        }

        return false;
    }

    bool isActive() const noexcept
    {
        return [NSOpenGLContext currentContext] == renderContext;
    }

    static void deactivateCurrentContext()
    {
        [NSOpenGLContext clearCurrentContext];
    }

    struct Locker
    {
        Locker (NativeContext& nc) : cglContext ((CGLContextObj) [nc.renderContext CGLContextObj])
        {
            CGLLockContext (cglContext);
        }

        ~Locker()
        {
            CGLUnlockContext (cglContext);
        }

    private:
        CGLContextObj cglContext;
    };

    void swapBuffers()
    {
        auto now = Time::getMillisecondCounterHiRes();
        [renderContext flushBuffer];

        if (const auto minSwapTime = minSwapTimeMs.get(); minSwapTime > 0)
        {
            // When our window is entirely occluded by other windows, flushBuffer
            // fails to wait for the swap interval, so the render loop spins at full
            // speed, burning CPU. This hack detects when things are going too fast
            // and sleeps if necessary.

            auto swapTime = Time::getMillisecondCounterHiRes() - now;
            auto frameTime = (int) std::min ((uint64_t) std::numeric_limits<int>::max(),
                                             (uint64_t) now - (uint64_t) lastSwapTime);

            if (swapTime < 0.5 && frameTime < minSwapTime - 3)
            {
                if (underrunCounter > 3)
                {
                    Thread::sleep (2 * (minSwapTime - frameTime));
                    now = Time::getMillisecondCounterHiRes();
                }
                else
                {
                    ++underrunCounter;
                }
            }
            else
            {
                if (underrunCounter > 0)
                    --underrunCounter;
            }
        }

        lastSwapTime = now;
    }

    void updateWindowPosition (Rectangle<int>)
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            const auto newArea = peer->getAreaCoveredBy (owner);

            if (convertToRectInt ([view frame]) != newArea)
                [view setFrame: makeNSRect (newArea)];
        }
    }

    bool setSwapInterval (int numFramesPerSwapIn)
    {
        // The macOS OpenGL programming guide says that numFramesPerSwap
        // can only be 0 or 1.
        jassert (isPositiveAndBelow (numFramesPerSwapIn, 2));

        [renderContext setValues: (const GLint*) &numFramesPerSwapIn
                    forParameter: getSwapIntervalParameter()];

        minSwapTimeMs.setFramesPerSwap (numFramesPerSwapIn);

        return true;
    }

    int getSwapInterval() const
    {
        GLint numFrames = 0;
        [renderContext getValues: &numFrames
                    forParameter: getSwapIntervalParameter()];

        return numFrames;
    }

    void setNominalVideoRefreshPeriodS (double periodS)
    {
        jassert (periodS > 0.0);
        minSwapTimeMs.setVideoRefreshPeriodS (periodS);
    }

    static NSOpenGLContextParameter getSwapIntervalParameter()
    {
        if (@available (macOS 10.12, *))
            return NSOpenGLContextParameterSwapInterval;

        return NSOpenGLCPSwapInterval;
    }

    class MinSwapTimeMs
    {
    public:
        int get() const
        {
            return minSwapTimeMs;
        }

        void setFramesPerSwap (int n)
        {
            const std::scoped_lock lock { mutex };
            numFramesPerSwap = n;
            updateMinSwapTime();
        }

        void setVideoRefreshPeriodS (double n)
        {
            const std::scoped_lock lock { mutex };
            videoRefreshPeriodS = n;
            updateMinSwapTime();
        }

    private:
        void updateMinSwapTime()
        {
            minSwapTimeMs = static_cast<int> (numFramesPerSwap * 1000 * videoRefreshPeriodS);
        }

        std::mutex mutex;
        std::atomic<int> minSwapTimeMs { 0 };
        int numFramesPerSwap = 0;
        double videoRefreshPeriodS = 1.0 / 60.0;
    };

    Component& owner;
    NSOpenGLContext* renderContext = nil;
    NSOpenGLView* view = nil;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;
    double lastSwapTime = 0;
    int underrunCounter = 0;
    MinSwapTimeMs minSwapTimeMs;

    //==============================================================================
    struct MouseForwardingNSOpenGLViewClass  : public ObjCClass<NSOpenGLView>
    {
        MouseForwardingNSOpenGLViewClass()  : ObjCClass ("JUCEGLView_")
        {
            addMethod (@selector (rightMouseDown:),       [] (id self, SEL, NSEvent* ev)     { [[(NSOpenGLView*) self superview] rightMouseDown: ev]; });
            addMethod (@selector (rightMouseUp:),         [] (id self, SEL, NSEvent* ev)     { [[(NSOpenGLView*) self superview] rightMouseUp:   ev]; });
            addMethod (@selector (acceptsFirstMouse:),    [] (id, SEL, NSEvent*) -> BOOL     { return YES; });
            addMethod (@selector (accessibilityHitTest:), [] (id self, SEL, NSPoint p) -> id { return [[(NSOpenGLView*) self superview] accessibilityHitTest: p]; });

            registerClass();
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return CGLGetCurrentContext() != CGLContextObj();
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
