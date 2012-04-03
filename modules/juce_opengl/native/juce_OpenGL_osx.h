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

} // (juce namespace)

#define ThreadSafeNSOpenGLView MakeObjCClassName(ThreadSafeNSOpenGLView)

//==============================================================================
@interface ThreadSafeNSOpenGLView  : NSOpenGLView
{
    juce::CriticalSection* contextLock;
    bool needsUpdate;
}

- (id) initWithFrame: (NSRect) frameRect pixelFormat: (NSOpenGLPixelFormat*) format;
- (bool) makeActive;
- (void) reshape;
- (void) rightMouseDown: (NSEvent*) ev;
- (void) rightMouseUp: (NSEvent*) ev;
@end

@implementation ThreadSafeNSOpenGLView

- (id) initWithFrame: (NSRect) frameRect
         pixelFormat: (NSOpenGLPixelFormat*) format
{
    contextLock = new juce::CriticalSection();
    self = [super initWithFrame: frameRect pixelFormat: format];
    needsUpdate = true;

    if (self != nil)
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector (_surfaceNeedsUpdate:)
                                                     name: NSViewGlobalFrameDidChangeNotification
                                                   object: self];
    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    delete contextLock;
    [super dealloc];
}

- (bool) makeActive
{
    const juce::ScopedLock sl (*contextLock);

    if ([self openGLContext] == nil)
        return false;

    [[self openGLContext] makeCurrentContext];

    if (needsUpdate)
    {
        [super update];
        needsUpdate = false;
    }

    return true;
}

- (void) _surfaceNeedsUpdate: (NSNotification*) notification
{
    (void) notification;
    const juce::ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) update
{
    const juce::ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) reshape
{
    const juce::ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) rightMouseDown: (NSEvent*) ev  { [[self superview] rightMouseDown: ev]; }
- (void) rightMouseUp:   (NSEvent*) ev  { [[self superview] rightMouseUp:   ev]; }

@end

namespace juce
{

//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixelFormat,
                   const NativeContext* contextToShareWith)
    {
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAMPSafe,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   (NSOpenGLPixelFormatAttribute) (pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits),
            NSOpenGLPFAAlphaSize,   (NSOpenGLPixelFormatAttribute) pixelFormat.alphaBits,
            NSOpenGLPFADepthSize,   (NSOpenGLPixelFormatAttribute) pixelFormat.depthBufferBits,
            NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) pixelFormat.stencilBufferBits,
            NSOpenGLPFAAccumSize,   (NSOpenGLPixelFormatAttribute) (pixelFormat.accumulationBufferRedBits + pixelFormat.accumulationBufferGreenBits
                                        + pixelFormat.accumulationBufferBlueBits + pixelFormat.accumulationBufferAlphaBits),
            pixelFormat.multisamplingLevel > 0 ? NSOpenGLPFASamples : (NSOpenGLPixelFormatAttribute) 0,
            (NSOpenGLPixelFormatAttribute) pixelFormat.multisamplingLevel,
            0
        };

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

        view = [[ThreadSafeNSOpenGLView alloc] initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                 pixelFormat: format];

        NSOpenGLContext* const sharedContext
            = contextToShareWith != nullptr ? contextToShareWith->renderContext : nil;

        renderContext = [[[NSOpenGLContext alloc] initWithFormat: format
                                                    shareContext: sharedContext] autorelease];

        setSwapInterval (1);

        [view setOpenGLContext: renderContext];
        [format release];

        viewAttachment = NSViewComponent::attachViewToComponent (component, view);
    }

    ~NativeContext()
    {
        [renderContext clearDrawable];
        [renderContext setView: nil];
        [view setOpenGLContext: nil];
        renderContext = nil;
    }

    void initialiseOnRenderThread() {}
    void shutdownOnRenderThread() {}

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return static_cast <void*> (renderContext); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    bool makeActive() const noexcept
    {
        jassert (renderContext != nil);

        if ([renderContext view] != view)
            [renderContext setView: view];

        [view makeActive];
        return true;
    }

    bool isActive() const noexcept
    {
        return [NSOpenGLContext currentContext] == renderContext;
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
    ThreadSafeNSOpenGLView* view;
    ReferenceCountedObjectPtr<ReferenceCountedObject> viewAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext);
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return CGLGetCurrentContext() != 0;
}
