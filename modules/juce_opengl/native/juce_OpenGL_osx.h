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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare,
                   bool shouldUseMultisampling,
                   OpenGLVersion version)
    {
        NSOpenGLPixelFormatAttribute attribs[64] = { 0 };
        createAttribs (attribs, version, pixFormat, shouldUseMultisampling);

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

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

    static void createAttribs (NSOpenGLPixelFormatAttribute* attribs, OpenGLVersion version,
                               const OpenGLPixelFormat& pixFormat, bool shouldUseMultisampling)
    {
        ignoreUnused (version);
        int numAttribs = 0;

       #if JUCE_OPENGL3
        attribs[numAttribs++] = NSOpenGLPFAOpenGLProfile;
        attribs[numAttribs++] = version >= openGL3_2 ? NSOpenGLProfileVersion3_2Core
                                                     : NSOpenGLProfileVersionLegacy;
       #endif

        attribs[numAttribs++] = NSOpenGLPFADoubleBuffer;
        attribs[numAttribs++] = NSOpenGLPFAClosestPolicy;
        attribs[numAttribs++] = NSOpenGLPFANoRecovery;
        attribs[numAttribs++] = NSOpenGLPFAColorSize;
        attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) (pixFormat.redBits + pixFormat.greenBits + pixFormat.blueBits);
        attribs[numAttribs++] = NSOpenGLPFAAlphaSize;
        attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.alphaBits;
        attribs[numAttribs++] = NSOpenGLPFADepthSize;
        attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.depthBufferBits;
        attribs[numAttribs++] = NSOpenGLPFAStencilSize;
        attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.stencilBufferBits;
        attribs[numAttribs++] = NSOpenGLPFAAccumSize;
        attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) (pixFormat.accumulationBufferRedBits + pixFormat.accumulationBufferGreenBits
                                                                   + pixFormat.accumulationBufferBlueBits + pixFormat.accumulationBufferAlphaBits);

        if (shouldUseMultisampling)
        {
            attribs[numAttribs++] = NSOpenGLPFAMultisample;
            attribs[numAttribs++] = NSOpenGLPFASampleBuffers;
            attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) 1;
            attribs[numAttribs++] = NSOpenGLPFASamples;
            attribs[numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.multisamplingLevel;
        }
    }

    bool initialiseOnRenderThread (OpenGLContext&)    { return true; }
    void shutdownOnRenderThread()                     { deactivateCurrentContext(); }

    bool createdOk() const noexcept                   { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept              { return static_cast<void*> (renderContext); }
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

        if (minSwapTimeMs > 0)
        {
            // When our window is entirely occluded by other windows, flushBuffer
            // fails to wait for the swap interval, so the render loop spins at full
            // speed, burning CPU. This hack detects when things are going too fast
            // and sleeps if necessary.

            auto swapTime = Time::getMillisecondCounterHiRes() - now;
            auto frameTime = (int) (now - lastSwapTime);

            if (swapTime < 0.5 && frameTime < minSwapTimeMs - 3)
            {
                if (underrunCounter > 3)
                {
                    Thread::sleep (2 * (minSwapTimeMs - frameTime));
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

    void updateWindowPosition (Rectangle<int>) {}

    bool setSwapInterval (int numFramesPerSwap)
    {
        // The macOS OpenGL programming guide says that numFramesPerSwap
        // can only be 0 or 1.
        jassert (isPositiveAndBelow (numFramesPerSwap, 2));

        minSwapTimeMs = (numFramesPerSwap * 1000) / 60;

        [renderContext setValues: (const GLint*) &numFramesPerSwap
                   #if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
                    forParameter: NSOpenGLContextParameterSwapInterval];
                   #else
                    forParameter: NSOpenGLCPSwapInterval];
                   #endif
        return true;
    }

    int getSwapInterval() const
    {
        GLint numFrames = 0;
        [renderContext getValues: &numFrames
                   #if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
                    forParameter: NSOpenGLContextParameterSwapInterval];
                   #else
                    forParameter: NSOpenGLCPSwapInterval];
                   #endif

        return numFrames;
    }

    NSOpenGLContext* renderContext = nil;
    NSOpenGLView* view = nil;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;
    double lastSwapTime = 0;
    int minSwapTimeMs = 0, underrunCounter = 0;

    //==============================================================================
    struct MouseForwardingNSOpenGLViewClass  : public ObjCClass<NSOpenGLView>
    {
        MouseForwardingNSOpenGLViewClass()  : ObjCClass<NSOpenGLView> ("JUCEGLView_")
        {
            addMethod (@selector (rightMouseDown:),      rightMouseDown,     "v@:@");
            addMethod (@selector (rightMouseUp:),        rightMouseUp,       "v@:@");
            addMethod (@selector (acceptsFirstMouse:),   acceptsFirstMouse,  "v@:@");

            registerClass();
        }

    private:
        static void rightMouseDown (id self, SEL, NSEvent* ev)      { [[(NSOpenGLView*) self superview] rightMouseDown: ev]; }
        static void rightMouseUp   (id self, SEL, NSEvent* ev)      { [[(NSOpenGLView*) self superview] rightMouseUp:   ev]; }
        static BOOL acceptsFirstMouse (id, SEL, NSEvent*)           { return YES; }
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
