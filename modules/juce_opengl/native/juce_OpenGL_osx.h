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

    void initialiseOnRenderThread (OpenGLContext&) {}
    void shutdownOnRenderThread()               { deactivateCurrentContext(); }

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return static_cast <void*> (renderContext); }
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

    NSOpenGLContext* renderContext;
    NSOpenGLView* view;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;

    //==============================================================================
    struct MouseForwardingNSOpenGLViewClass  : public ObjCClass <NSOpenGLView>
    {
        MouseForwardingNSOpenGLViewClass()  : ObjCClass <NSOpenGLView> ("JUCEGLView_")
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
