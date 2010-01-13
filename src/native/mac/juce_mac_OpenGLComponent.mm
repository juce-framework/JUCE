/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

@end
BEGIN_JUCE_NAMESPACE

//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (Component* const component,
                       const OpenGLPixelFormat& pixelFormat_,
                       NSOpenGLContext* sharedContext)
        : renderContext (0),
          pixelFormat (pixelFormat_)
    {
        jassert (component != 0);

        NSOpenGLPixelFormatAttribute attribs [64];
        int n = 0;
        attribs[n++] = NSOpenGLPFADoubleBuffer;
        attribs[n++] = NSOpenGLPFAAccelerated;
        attribs[n++] = NSOpenGLPFAMPSafe; // NSOpenGLPFAAccelerated, NSOpenGLPFAMultiScreen, NSOpenGLPFASingleRenderer
        attribs[n++] = NSOpenGLPFAColorSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) jmax (pixelFormat.redBits,
                                                            pixelFormat.greenBits,
                                                            pixelFormat.blueBits);
        attribs[n++] = NSOpenGLPFAAlphaSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) pixelFormat.alphaBits;
        attribs[n++] = NSOpenGLPFADepthSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) pixelFormat.depthBufferBits;
        attribs[n++] = NSOpenGLPFAStencilSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) pixelFormat.stencilBufferBits;
        attribs[n++] = NSOpenGLPFAAccumSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) jmax (pixelFormat.accumulationBufferRedBits,
                                                            pixelFormat.accumulationBufferGreenBits,
                                                            pixelFormat.accumulationBufferBlueBits,
                                                            pixelFormat.accumulationBufferAlphaBits);

        // xxx not sure how to do fullSceneAntiAliasingNumSamples..
        attribs[n++] = NSOpenGLPFASampleBuffers;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) 1;
        attribs[n++] = NSOpenGLPFAClosestPolicy;
        attribs[n++] = NSOpenGLPFANoRecovery;
        attribs[n++] = (NSOpenGLPixelFormatAttribute) 0;

        NSOpenGLPixelFormat* format
            = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

        view = [[ThreadSafeNSOpenGLView alloc] initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                 pixelFormat: format];

        renderContext = [[[NSOpenGLContext alloc] initWithFormat: format
                                                    shareContext: sharedContext] autorelease];

        const GLint swapInterval = 1;
        [renderContext setValues: &swapInterval forParameter: NSOpenGLCPSwapInterval];

        [view setOpenGLContext: renderContext];
        [renderContext setView: view];

        [format release];

        viewHolder = new NSViewComponentInternal (view, component);
    }

    ~WindowedGLContext()
    {
        makeInactive();
        [renderContext clearDrawable];
        viewHolder = 0;
    }

    bool makeActive() const throw()
    {
        jassert (renderContext != 0);
        [view makeActive];
        return isActive();
    }

    bool makeInactive() const throw()
    {
        [view makeInactive];
        return true;
    }

    bool isActive() const throw()
    {
        return [NSOpenGLContext currentContext] == renderContext;
    }

    const OpenGLPixelFormat getPixelFormat() const  { return pixelFormat; }
    void* getRawContext() const throw()             { return renderContext; }

    void updateWindowPosition (int x, int y, int w, int h, int outerWindowHeight)
    {
    }

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
    juce_UseDebuggingNewOperator

    NSOpenGLContext* renderContext;
    ThreadSafeNSOpenGLView* view;

private:
    OpenGLPixelFormat pixelFormat;
    ScopedPointer <NSViewComponentInternal> viewHolder;

    //==============================================================================
    WindowedGLContext (const WindowedGLContext&);
    const WindowedGLContext& operator= (const WindowedGLContext&);
};

//==============================================================================
OpenGLContext* OpenGLContext::createContextForWindow (Component* const component,
                                                      const OpenGLPixelFormat& pixelFormat,
                                                      const OpenGLContext* const contextToShareWith)
{
    ScopedPointer <WindowedGLContext> c (new WindowedGLContext (component, pixelFormat,
                                                                contextToShareWith != 0 ? (NSOpenGLContext*) contextToShareWith->getRawContext() : 0));

    return (c->renderContext != 0) ? c.release() : 0;
}

void* OpenGLComponent::getNativeWindowHandle() const
{
    return context != 0 ? ((WindowedGLContext*) context)->getNativeWindowHandle()
                        : 0;
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* /*component*/,
                                                  OwnedArray <OpenGLPixelFormat>& results)
{
/*    GLint attribs [64];
    int n = 0;
    attribs[n++] = AGL_RGBA;
    attribs[n++] = AGL_DOUBLEBUFFER;
    attribs[n++] = AGL_ACCELERATED;
    attribs[n++] = AGL_NO_RECOVERY;
    attribs[n++] = AGL_NONE;

    AGLPixelFormat p = aglChoosePixelFormat (0, 0, attribs);

    while (p != 0)
    {
        OpenGLPixelFormat* const pf = new OpenGLPixelFormat();
        pf->redBits = getAGLAttribute (p, AGL_RED_SIZE);
        pf->greenBits = getAGLAttribute (p, AGL_GREEN_SIZE);
        pf->blueBits = getAGLAttribute (p, AGL_BLUE_SIZE);
        pf->alphaBits = getAGLAttribute (p, AGL_ALPHA_SIZE);
        pf->depthBufferBits = getAGLAttribute (p, AGL_DEPTH_SIZE);
        pf->stencilBufferBits = getAGLAttribute (p, AGL_STENCIL_SIZE);
        pf->accumulationBufferRedBits = getAGLAttribute (p, AGL_ACCUM_RED_SIZE);
        pf->accumulationBufferGreenBits = getAGLAttribute (p, AGL_ACCUM_GREEN_SIZE);
        pf->accumulationBufferBlueBits = getAGLAttribute (p, AGL_ACCUM_BLUE_SIZE);
        pf->accumulationBufferAlphaBits = getAGLAttribute (p, AGL_ACCUM_ALPHA_SIZE);

        results.add (pf);

        p = aglNextPixelFormat (p);
    }*/

    //jassertfalse  //xxx can't see how you do this in cocoa!
}

#else
//==============================================================================

OpenGLContext* OpenGLContext::createContextForWindow (Component* const component,
                                                      const OpenGLPixelFormat& pixelFormat,
                                                      const OpenGLContext* const contextToShareWith)
{
    return 0;
}

void juce_glViewport (const int w, const int h)
{
    //glViewport (0, 0, w, h);
}

#endif

#endif
