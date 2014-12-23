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

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare,
                   bool shouldUseMultisampling,
                   OpenGLVersion version)
        : lastSwapTime (0), minSwapTimeMs (0), underrunCounter (0)
    {
        NSOpenGLPixelFormatAttribute attribs[64] = { 0 };
        createAttribs (attribs, version, pixFormat, shouldUseMultisampling);

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

        static MouseForwardingNSOpenGLViewClass cls;
        view = [cls.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       pixelFormat: format];

       #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
        if ([view respondsToSelector: @selector (setWantsBestResolutionOpenGLSurface:)])
            [view setWantsBestResolutionOpenGLSurface: YES];
       #endif

        [[NSNotificationCenter defaultCenter] addObserver: view
                                                 selector: @selector (_surfaceNeedsUpdate:)
                                                     name: NSViewGlobalFrameDidChangeNotification
                                                   object: view];

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
        renderContext = nil;
    }

    static void createAttribs (NSOpenGLPixelFormatAttribute* attribs, OpenGLVersion version,
                               const OpenGLPixelFormat& pixFormat, bool shouldUseMultisampling)
    {
        (void) version;
        int numAttribs = 0;

       #if JUCE_OPENGL3
        attribs [numAttribs++] = NSOpenGLPFAOpenGLProfile;
        attribs [numAttribs++] = version >= openGL3_2 ? NSOpenGLProfileVersion3_2Core
                                                      : NSOpenGLProfileVersionLegacy;
       #endif

        attribs [numAttribs++] = NSOpenGLPFADoubleBuffer;
        attribs [numAttribs++] = NSOpenGLPFAClosestPolicy;
        attribs [numAttribs++] = NSOpenGLPFANoRecovery;
        attribs [numAttribs++] = NSOpenGLPFAColorSize;
        attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) (pixFormat.redBits + pixFormat.greenBits + pixFormat.blueBits);
        attribs [numAttribs++] = NSOpenGLPFAAlphaSize;
        attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.alphaBits;
        attribs [numAttribs++] = NSOpenGLPFADepthSize;
        attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.depthBufferBits;
        attribs [numAttribs++] = NSOpenGLPFAStencilSize;
        attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.stencilBufferBits;
        attribs [numAttribs++] = NSOpenGLPFAAccumSize;
        attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) (pixFormat.accumulationBufferRedBits + pixFormat.accumulationBufferGreenBits
                                                                   + pixFormat.accumulationBufferBlueBits + pixFormat.accumulationBufferAlphaBits);

        if (shouldUseMultisampling)
        {
            attribs [numAttribs++] = NSOpenGLPFAMultisample;
            attribs [numAttribs++] = NSOpenGLPFASampleBuffers;
            attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) 1;
            attribs [numAttribs++] = NSOpenGLPFASamples;
            attribs [numAttribs++] = (NSOpenGLPixelFormatAttribute) pixFormat.multisamplingLevel;
        }
    }

    void initialiseOnRenderThread (OpenGLContext&) {}
    void shutdownOnRenderThread()               { deactivateCurrentContext(); }

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return static_cast<void*> (renderContext); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

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
        [renderContext flushBuffer];

        sleepIfRenderingTooFast();
    }

    void updateWindowPosition (const Rectangle<int>&) {}

    bool setSwapInterval (int numFramesPerSwap)
    {
        minSwapTimeMs = (numFramesPerSwap * 1000) / 60;

        [renderContext setValues: (const GLint*) &numFramesPerSwap
                    forParameter: NSOpenGLCPSwapInterval];
        return true;
    }

    int getSwapInterval() const
    {
        GLint numFrames = 0;
        [renderContext getValues: &numFrames
                    forParameter: NSOpenGLCPSwapInterval];

        return numFrames;
    }

    void sleepIfRenderingTooFast()
    {
        // When our window is entirely occluded by other windows, the system
        // fails to correctly implement the swap interval time, so the render
        // loop spins at full speed, burning CPU. This hack detects when things
        // are going too fast and slows things down if necessary.

        if (minSwapTimeMs > 0)
        {
            const double now = Time::getMillisecondCounterHiRes();
            const int elapsed = (int) (now - lastSwapTime);
            lastSwapTime = now;

            if (isPositiveAndBelow (elapsed, minSwapTimeMs - 3))
            {
                if (underrunCounter > 3)
                    Thread::sleep (minSwapTimeMs - elapsed);
                else
                    ++underrunCounter;
            }
            else
            {
                underrunCounter = 0;
            }
        }
    }

    NSOpenGLContext* renderContext;
    NSOpenGLView* view;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;
    double lastSwapTime;
    int minSwapTimeMs, underrunCounter;

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
    return CGLGetCurrentContext() != 0;
}
