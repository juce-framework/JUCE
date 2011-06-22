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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_OPENGL

#if JUCE_MAC

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

    if ([self openGLContext] == 0)
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

- (void) rightMouseDown: (NSEvent*) ev
{
    [[self superview] rightMouseDown: ev];
}

- (void) rightMouseUp: (NSEvent*) ev
{
    [[self superview] rightMouseUp: ev];
}

@end
BEGIN_JUCE_NAMESPACE

//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (Component& component,
                       const OpenGLPixelFormat& pixelFormat_,
                       NSOpenGLContext* sharedContext)
        : renderContext (nil),
          pixelFormat (pixelFormat_)
    {
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAMPSafe,
            NSOpenGLPFAClosestPolicy,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAColorSize,   (NSOpenGLPixelFormatAttribute) (pixelFormat.redBits
                                                                        + pixelFormat.greenBits
                                                                        + pixelFormat.blueBits),
            NSOpenGLPFAAlphaSize,   (NSOpenGLPixelFormatAttribute) pixelFormat.alphaBits,
            NSOpenGLPFADepthSize,   (NSOpenGLPixelFormatAttribute) pixelFormat.depthBufferBits,
            NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute) pixelFormat.stencilBufferBits,
            NSOpenGLPFAAccumSize,   (NSOpenGLPixelFormatAttribute) (pixelFormat.accumulationBufferRedBits
                                                                        + pixelFormat.accumulationBufferGreenBits
                                                                        + pixelFormat.accumulationBufferBlueBits
                                                                        + pixelFormat.accumulationBufferAlphaBits),
            NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute) 1,
            (NSOpenGLPixelFormatAttribute) 0
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

        viewHolder = new NSViewComponentInternal (view, component);
    }

    ~WindowedGLContext()
    {
        deleteContext();
        viewHolder = 0;
    }

    void deleteContext()
    {
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

    const OpenGLPixelFormat getPixelFormat() const  { return pixelFormat; }
    void* getRawContext() const noexcept            { return renderContext; }

    void updateWindowPosition (const Rectangle<int>&) {}

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

    void repaint()
    {
        // we need to invalidate the juce view that holds this gl view, to make it
        // cause a repaint callback
        NSView* v = (NSView*) viewHolder->view;
        NSRect r = [v frame];

        // bit of a bodge here.. if we only invalidate the area of the gl component,
        // it's completely covered by the NSOpenGLView, so the OS throws away the
        // repaint message, thus never causing our paint() callback, and never repainting
        // the comp. So invalidating just a little bit around the edge helps..
        [[v superview] setNeedsDisplayInRect: NSInsetRect (r, -2.0f, -2.0f)];
    }

    void* getNativeWindowHandle() const     { return viewHolder->view; }

    //==============================================================================
    NSOpenGLContext* renderContext;
    ThreadSafeNSOpenGLView* view;

private:
    OpenGLPixelFormat pixelFormat;
    ScopedPointer <NSViewComponentInternal> viewHolder;

    //==============================================================================
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
    return context != nullptr ? static_cast<WindowedGLContext*> (static_cast<OpenGLContext*> (context))->getNativeWindowHandle()
                              : nullptr;
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

static int getPixelFormatAttribute (NSOpenGLPixelFormat* p, NSOpenGLPixelFormatAttribute att)
{
    GLint val = 0;
    [p getValues: &val forAttribute: att forVirtualScreen: 0];
    return (int) val;
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* /*component*/,
                                                  OwnedArray <OpenGLPixelFormat>& results)
{
    NSOpenGLPixelFormatAttribute attributes[] =
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFADepthSize,  (NSOpenGLPixelFormatAttribute) 16,
        NSOpenGLPFAAlphaSize,  (NSOpenGLPixelFormatAttribute) 8,
        NSOpenGLPFAColorSize,  (NSOpenGLPixelFormatAttribute) 24,
        NSOpenGLPFAAccumSize,  (NSOpenGLPixelFormatAttribute) 32,
        (NSOpenGLPixelFormatAttribute) 0
    };

    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attributes];

    if (format != nil)
    {
        OpenGLPixelFormat* const pf = new OpenGLPixelFormat();

        pf->redBits = pf->greenBits = pf->blueBits = getPixelFormatAttribute (format, NSOpenGLPFAColorSize) / 3;
        pf->alphaBits = getPixelFormatAttribute (format, NSOpenGLPFAAlphaSize);
        pf->depthBufferBits = getPixelFormatAttribute (format, NSOpenGLPFADepthSize);
        pf->stencilBufferBits = getPixelFormatAttribute (format, NSOpenGLPFAStencilSize);
        pf->accumulationBufferRedBits = pf->accumulationBufferGreenBits
            = pf->accumulationBufferBlueBits = pf->accumulationBufferAlphaBits
            = getPixelFormatAttribute (format, NSOpenGLPFAAccumSize) / 4;

        [format release];
        results.add (pf);
    }
}

#else
//==============================================================================

END_JUCE_NAMESPACE

@interface JuceGLView   : UIView
{
}
+ (Class) layerClass;
@end

@implementation JuceGLView
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class GLESContext   : public OpenGLContext
{
public:
    GLESContext (UIViewComponentPeer* peer,
                 Component* const component_,
                 const OpenGLPixelFormat& pixelFormat_,
                 const GLESContext* const sharedContext,
                 NSUInteger apiType)
        : component (component_), pixelFormat (pixelFormat_), glLayer (nil), context (nil),
          useDepthBuffer (pixelFormat_.depthBufferBits > 0), frameBufferHandle (0), colorBufferHandle (0),
          depthBufferHandle (0), lastWidth (0), lastHeight (0)
    {
        view = [[JuceGLView alloc] initWithFrame: CGRectMake (0, 0, 64, 64)];
        view.opaque = YES;
        view.hidden = NO;
        view.backgroundColor = [UIColor blackColor];
        view.userInteractionEnabled = NO;

        glLayer = (CAEAGLLayer*) [view layer];
        [peer->view addSubview: view];

        if (sharedContext != nullptr)
            context = [[EAGLContext alloc] initWithAPI: apiType
                                            sharegroup: [sharedContext->context sharegroup]];
        else
            context = [[EAGLContext alloc] initWithAPI: apiType];

        createGLBuffers();
    }

    ~GLESContext()
    {
        deleteContext();

        [view removeFromSuperview];
        [view release];
        freeGLBuffers();
    }

    void deleteContext()
    {
        makeInactive();
        [context release];
        context = nil;
    }

    bool makeActive() const noexcept
    {
        jassert (context != nil);

        [EAGLContext setCurrentContext: context];
        glBindFramebufferOES (GL_FRAMEBUFFER_OES, frameBufferHandle);
        return true;
    }

    void swapBuffers()
    {
        glBindRenderbufferOES (GL_RENDERBUFFER_OES, colorBufferHandle);
        [context presentRenderbuffer: GL_RENDERBUFFER_OES];
    }

    bool makeInactive() const noexcept
    {
        return [EAGLContext setCurrentContext: nil];
    }

    bool isActive() const noexcept
    {
        return [EAGLContext currentContext] == context;
    }

    const OpenGLPixelFormat getPixelFormat() const  { return pixelFormat; }
    void* getRawContext() const noexcept            { return glLayer; }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        view.frame = CGRectMake ((CGFloat) bounds.getX(), (CGFloat) bounds.getY(),
                                 (CGFloat) bounds.getWidth(), (CGFloat) bounds.getHeight());

        if (lastWidth != bounds.getWidth() || lastHeight != bounds.getHeight())
        {
            lastWidth = bounds.getWidth();
            lastHeight = bounds.getHeight();
            freeGLBuffers();
            createGLBuffers();
        }
    }

    bool setSwapInterval (const int numFramesPerSwap)
    {
        numFrames = numFramesPerSwap;
        return true;
    }

    int getSwapInterval() const
    {
        return numFrames;
    }

    void repaint()
    {
    }

    //==============================================================================
    void createGLBuffers()
    {
        makeActive();

        glGenFramebuffersOES (1, &frameBufferHandle);
        glGenRenderbuffersOES (1, &colorBufferHandle);
        glGenRenderbuffersOES (1, &depthBufferHandle);

        glBindRenderbufferOES (GL_RENDERBUFFER_OES, colorBufferHandle);
        [context renderbufferStorage: GL_RENDERBUFFER_OES fromDrawable: glLayer];

        GLint width, height;
        glGetRenderbufferParameterivOES (GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
        glGetRenderbufferParameterivOES (GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);

        if (useDepthBuffer)
        {
            glBindRenderbufferOES (GL_RENDERBUFFER_OES, depthBufferHandle);
            glRenderbufferStorageOES (GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
        }

        glBindRenderbufferOES (GL_RENDERBUFFER_OES, colorBufferHandle);

        glBindFramebufferOES (GL_FRAMEBUFFER_OES, frameBufferHandle);
        glFramebufferRenderbufferOES (GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorBufferHandle);

        if (useDepthBuffer)
            glFramebufferRenderbufferOES (GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthBufferHandle);

        jassert (glCheckFramebufferStatusOES (GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES);
    }

    void freeGLBuffers()
    {
        if (frameBufferHandle != 0)
        {
            glDeleteFramebuffersOES (1, &frameBufferHandle);
            frameBufferHandle = 0;
        }

        if (colorBufferHandle != 0)
        {
            glDeleteRenderbuffersOES (1, &colorBufferHandle);
            colorBufferHandle = 0;
        }

        if (depthBufferHandle != 0)
        {
            glDeleteRenderbuffersOES (1, &depthBufferHandle);
            depthBufferHandle = 0;
        }
    }

    //==============================================================================
private:
    WeakReference<Component> component;
    OpenGLPixelFormat pixelFormat;
    JuceGLView* view;
    CAEAGLLayer* glLayer;
    EAGLContext* context;
    bool useDepthBuffer;
    GLuint frameBufferHandle, colorBufferHandle, depthBufferHandle;
    int numFrames;
    int lastWidth, lastHeight;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GLESContext);
};


OpenGLContext* OpenGLComponent::createContext()
{
    JUCE_AUTORELEASEPOOL
    UIViewComponentPeer* peer = dynamic_cast <UIViewComponentPeer*> (getPeer());

    if (peer != nullptr)
        return new GLESContext (peer, this, preferredPixelFormat,
                                dynamic_cast <const GLESContext*> (contextToShareListsWith),
                                type == openGLES2 ? kEAGLRenderingAPIOpenGLES2 : kEAGLRenderingAPIOpenGLES1);

    return nullptr;
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* /*component*/,
                                                  OwnedArray <OpenGLPixelFormat>& /*results*/)
{
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

#endif

#endif
