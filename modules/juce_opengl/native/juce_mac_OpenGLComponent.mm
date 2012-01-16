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

END_JUCE_NAMESPACE

#define ThreadSafeNSOpenGLView MakeObjCClassName(ThreadSafeNSOpenGLView)

//==============================================================================
@interface ThreadSafeNSOpenGLView  : NSOpenGLView
{
    CriticalSection* contextLock;
    bool needsUpdate;
}

- (id) initWithFrame: (NSRect) frameRect pixelFormat: (NSOpenGLPixelFormat*) format;
- (bool) makeActive;
- (void) makeInactive;
- (void) reshape;
- (void) rightMouseDown: (NSEvent*) ev;
- (void) rightMouseUp: (NSEvent*) ev;
@end

@implementation ThreadSafeNSOpenGLView

- (id) initWithFrame: (NSRect) frameRect
         pixelFormat: (NSOpenGLPixelFormat*) format
{
    contextLock = new CriticalSection();
    self = [super initWithFrame: frameRect pixelFormat: format];

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
    const ScopedLock sl (*contextLock);

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

- (void) makeInactive
{
    const ScopedLock sl (*contextLock);
    [NSOpenGLContext clearCurrentContext];
}

- (void) _surfaceNeedsUpdate: (NSNotification*) notification
{
    (void) notification;
    const ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) update
{
    const ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) reshape
{
    const ScopedLock sl (*contextLock);
    needsUpdate = true;
}

- (void) rightMouseDown: (NSEvent*) ev  { [[self superview] rightMouseDown: ev]; }
- (void) rightMouseUp:   (NSEvent*) ev  { [[self superview] rightMouseUp:   ev]; }

@end
BEGIN_JUCE_NAMESPACE

//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (OpenGLComponent& component,
                       const OpenGLPixelFormat& pixelFormat,
                       NSOpenGLContext* sharedContext)
        : renderContext (nil)
    {
        extensions.initialise();

        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAMPSafe,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits,
            NSOpenGLPFAAlphaSize,   pixelFormat.alphaBits,
            NSOpenGLPFADepthSize,   pixelFormat.depthBufferBits,
            NSOpenGLPFAStencilSize, pixelFormat.stencilBufferBits,
            NSOpenGLPFAAccumSize,   pixelFormat.accumulationBufferRedBits + pixelFormat.accumulationBufferGreenBits
                                        + pixelFormat.accumulationBufferBlueBits + pixelFormat.accumulationBufferAlphaBits,
            pixelFormat.multisamplingLevel > 0 ? NSOpenGLPFASamples : 0, pixelFormat.multisamplingLevel,
            0
        };

        NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

        view = [[ThreadSafeNSOpenGLView alloc] initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                 pixelFormat: format];

        renderContext = [[[NSOpenGLContext alloc] initWithFormat: format
                                                    shareContext: sharedContext] autorelease];

        const GLint swapInterval = 1;
        [renderContext setValues: &swapInterval forParameter: NSOpenGLCPSwapInterval];

        [view setOpenGLContext: renderContext];
        [format release];

        component.setView (view);
    }

    ~WindowedGLContext()
    {
        properties.clear(); // to release any stored programs, etc that may be held in properties.

        makeInactive();
        [renderContext clearDrawable];
        [renderContext setView: nil];
        [view setOpenGLContext: nil];
        renderContext = nil;
    }

    bool makeActive() const noexcept
    {
        jassert (renderContext != nil);

        if ([renderContext view] != view)
            [renderContext setView: view];

        [view makeActive];
        return isActive();
    }

    bool makeInactive() const noexcept
    {
        [view makeInactive];
        return true;
    }

    bool isActive() const noexcept
    {
        return [NSOpenGLContext currentContext] == renderContext;
    }

    void* getRawContext() const noexcept            { return renderContext; }
    unsigned int getFrameBufferID() const           { return 0; }

    int getWidth() const                            { return [view frame].size.width; }
    int getHeight() const                           { return [view frame].size.height; }

    void swapBuffers()
    {
        [renderContext flushBuffer];
    }

    bool setSwapInterval (const int numFramesPerSwap)
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

    void* getNativeWindowHandle() const     { return view; }

    //==============================================================================
    NSOpenGLContext* renderContext;
    ThreadSafeNSOpenGLView* view;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowedGLContext);
};

//==============================================================================
OpenGLContext* OpenGLComponent::createContext()
{
    ScopedPointer<WindowedGLContext> c (new WindowedGLContext (*this, preferredPixelFormat,
                                                               contextToShareListsWith != nullptr ? (NSOpenGLContext*) contextToShareListsWith->getRawContext() : nil));

    return (c->renderContext != nil) ? c.release() : nullptr;
}

void* OpenGLComponent::getNativeWindowHandle() const
{
    return context != nullptr ? static_cast<WindowedGLContext*> (context.get())->getNativeWindowHandle()
                              : nullptr;
}

void OpenGLComponent::updateEmbeddedPosition (const Rectangle<int>&)
{
}

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return CGLGetCurrentContext() != 0;
}
