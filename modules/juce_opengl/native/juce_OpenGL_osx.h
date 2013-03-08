/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

struct ThreadSafeNSOpenGLViewClass  : public ObjCClass <NSOpenGLView>
{
    ThreadSafeNSOpenGLViewClass()  : ObjCClass <NSOpenGLView> ("JUCEGLView_")
    {
        addIvar <CriticalSection*> ("lock");
        addIvar <BOOL> ("needsUpdate");

        addMethod (@selector (update),               update,             "v@:");
        addMethod (@selector (reshape),              reshape,            "v@:");
        addMethod (@selector (_surfaceNeedsUpdate:), surfaceNeedsUpdate, "v@:@");
        addMethod (@selector (rightMouseDown:),      rightMouseDown,     "v@:@");
        addMethod (@selector (rightMouseUp:),        rightMouseUp,       "v@:@");
        addMethod (@selector (acceptsFirstMouse:),   acceptsFirstMouse,  "v@:@");

        registerClass();
    }

    static void init (id self)
    {
        object_setInstanceVariable (self, "lock", new CriticalSection());

       #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
        if ([self respondsToSelector: @selector (setWantsBestResolutionOpenGLSurface:)])
            [self setWantsBestResolutionOpenGLSurface: YES];
       #endif

        setNeedsUpdate (self, YES);
    }

    static bool makeActive (id self)
    {
        const ScopedLock sl (*getLock (self));

        if ([(NSOpenGLView*) self openGLContext] == nil)
            return false;

        [[(NSOpenGLView*) self openGLContext] makeCurrentContext];

        if (getIvar<void*> (self, "needsUpdate"))
        {
            sendSuperclassMessage (self, @selector (update));
            setNeedsUpdate (self, NO);
        }

        return true;
    }

private:
    static CriticalSection* getLock (id self)
    {
        return getIvar<CriticalSection*> (self, "lock");
    }

    static void setNeedsUpdate (id self, BOOL b)
    {
        object_setInstanceVariable (self, "needsUpdate", (void*) b);
    }

    static void setNeedsUpdateLocked (id self, BOOL b)
    {
        const ScopedLock sl (*getLock (self));
        setNeedsUpdate (self, b);
    }

    static void dealloc (id self, SEL)
    {
        delete getLock (self);
        sendSuperclassMessage (self, @selector (dealloc));
    }

    static BOOL acceptsFirstMouse (id, SEL, NSEvent*)               { return YES; }
    static void surfaceNeedsUpdate (id self, SEL, NSNotification*)  { setNeedsUpdateLocked (self, YES); }
    static void update (id self, SEL)                               { setNeedsUpdateLocked (self, YES); }
    static void reshape (id self, SEL)                              { setNeedsUpdateLocked (self, YES); }

    static void rightMouseDown (id self, SEL, NSEvent* ev)  { [[(NSOpenGLView*) self superview] rightMouseDown: ev]; }
    static void rightMouseUp   (id self, SEL, NSEvent* ev)  { [[(NSOpenGLView*) self superview] rightMouseUp:   ev]; }
};


//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare)
    {
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAMPSafe,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   (NSOpenGLPixelFormatAttribute) (pixFormat.redBits + pixFormat.greenBits + pixFormat.blueBits),
            NSOpenGLPFAAlphaSize,   (NSOpenGLPixelFormatAttribute) pixFormat.alphaBits,
            NSOpenGLPFADepthSize,   (NSOpenGLPixelFormatAttribute) pixFormat.depthBufferBits,
            NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) pixFormat.stencilBufferBits,
            NSOpenGLPFAAccumSize,   (NSOpenGLPixelFormatAttribute) (pixFormat.accumulationBufferRedBits + pixFormat.accumulationBufferGreenBits
                                        + pixFormat.accumulationBufferBlueBits + pixFormat.accumulationBufferAlphaBits),
            pixFormat.multisamplingLevel > 0 ? NSOpenGLPFASamples : (NSOpenGLPixelFormatAttribute) 0,
            (NSOpenGLPixelFormatAttribute) pixFormat.multisamplingLevel,
            0
        };

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

        static ThreadSafeNSOpenGLViewClass cls;
        view = [cls.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       pixelFormat: format];
        ThreadSafeNSOpenGLViewClass::init (view);

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

    void initialiseOnRenderThread() {}
    void shutdownOnRenderThread()               { deactivateCurrentContext(); }

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return static_cast <void*> (renderContext); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    bool makeActive() const noexcept
    {
        jassert (renderContext != nil);

        if ([renderContext view] != view)
            [renderContext setView: view];

        ThreadSafeNSOpenGLViewClass::makeActive (view);
        return true;
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
    }

    void updateWindowPosition (const Rectangle<int>&) {}

    bool setSwapInterval (int numFramesPerSwap)
    {
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

private:
    NSOpenGLContext* renderContext;
    NSOpenGLView* view;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return CGLGetCurrentContext() != 0;
}
