/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

} // (juce namespace)

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

namespace juce
{

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare,
                   bool multisampling,
                   OpenGLVersion version)
        : context (nil), openGLversion (version),
          frameBufferHandle (0), colorBufferHandle (0),
          depthBufferHandle (0), msaaColorHandle (0), msaaBufferHandle (0),
          lastWidth (0), lastHeight (0), needToRebuildBuffers (false),
          swapFrames (0), useDepthBuffer (pixFormat.depthBufferBits > 0),
          useMSAA (multisampling)
    {
        JUCE_AUTORELEASEPOOL
        {
            ComponentPeer* const peer = component.getPeer();
            jassert (peer != nullptr);

            const Rectangle<int> bounds (peer->getComponent().getLocalArea (&component, component.getLocalBounds()));
            lastWidth  = bounds.getWidth();
            lastHeight = bounds.getHeight();

            view = [[JuceGLView alloc] initWithFrame: convertToCGRect (bounds)];
            view.opaque = YES;
            view.hidden = NO;
            view.backgroundColor = [UIColor blackColor];
            view.userInteractionEnabled = NO;

            glLayer = (CAEAGLLayer*) [view layer];
            glLayer.contentsScale = (CGFloat) Desktop::getInstance().getDisplays().getMainDisplay().scale;
            glLayer.opaque = true;

            [((UIView*) peer->getNativeHandle()) addSubview: view];

           #if defined (__IPHONE_7_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_7_0
            if (version == openGL3_2 && [[UIDevice currentDevice].systemVersion floatValue] >= 7.0)
            {
                if (! createContext (kEAGLRenderingAPIOpenGLES3, contextToShare))
                {
                    releaseContext();
                    createContext (kEAGLRenderingAPIOpenGLES2, contextToShare);
                }
            }
            else
           #endif
            {
                createContext (kEAGLRenderingAPIOpenGLES2, contextToShare);
            }

            jassert (context != nil);

            // I'd prefer to put this stuff in the initialiseOnRenderThread() call, but doing
            // so causes myserious timing-related failures.
            [EAGLContext setCurrentContext: context];
            createGLBuffers();
            deactivateCurrentContext();
        }
    }

    ~NativeContext()
    {
        releaseContext();
        [view removeFromSuperview];
        [view release];
    }

    void initialiseOnRenderThread (OpenGLContext&) {}

    void shutdownOnRenderThread()
    {
        JUCE_CHECK_OPENGL_ERROR
        freeGLBuffers();
        deactivateCurrentContext();
    }

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return context; }
    GLuint getFrameBufferID() const noexcept    { return useMSAA ? msaaBufferHandle : frameBufferHandle; }

    bool makeActive() const noexcept
    {
        if (! [EAGLContext setCurrentContext: context])
            return false;

        glBindFramebuffer (GL_FRAMEBUFFER, useMSAA ? msaaBufferHandle
                                                   : frameBufferHandle);
        return true;
    }

    bool isActive() const noexcept
    {
        return [EAGLContext currentContext] == context;
    }

    static void deactivateCurrentContext()
    {
        [EAGLContext setCurrentContext: nil];
    }

    void swapBuffers()
    {
        if (useMSAA)
        {
            glBindFramebuffer (GL_DRAW_FRAMEBUFFER, frameBufferHandle);
            glBindFramebuffer (GL_READ_FRAMEBUFFER, msaaBufferHandle);

           #if defined (__IPHONE_7_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_7_0
            if (openGLversion >= openGL3_2)
            {
                glBlitFramebuffer (0, 0, lastWidth, lastHeight, 0, 0, lastWidth, lastHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            }
            else
           #endif
            {
               #ifdef GL_APPLE_framebuffer_multisample
                glResolveMultisampleFramebufferAPPLE();
               #endif
            }
        }

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);
        [context presentRenderbuffer: GL_RENDERBUFFER];

        if (needToRebuildBuffers)
        {
            needToRebuildBuffers = false;

            freeGLBuffers();
            createGLBuffers();
            makeActive();
        }
    }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        view.frame = convertToCGRect (bounds);

        if (lastWidth != bounds.getWidth() || lastHeight != bounds.getHeight())
        {
            lastWidth  = bounds.getWidth();
            lastHeight = bounds.getHeight();
            needToRebuildBuffers = true;
        }
    }

    bool setSwapInterval (const int numFramesPerSwap) noexcept
    {
        swapFrames = numFramesPerSwap;
        return false;
    }

    int getSwapInterval() const noexcept    { return swapFrames; }

    struct Locker { Locker (NativeContext&) {} };

private:
    JuceGLView* view;
    CAEAGLLayer* glLayer;
    EAGLContext* context;
    const OpenGLVersion openGLversion;
    GLuint frameBufferHandle, colorBufferHandle, depthBufferHandle,
           msaaColorHandle, msaaBufferHandle;

    int volatile lastWidth, lastHeight;
    bool volatile needToRebuildBuffers;
    int swapFrames;
    bool useDepthBuffer, useMSAA;

    bool createContext (EAGLRenderingAPI type, void* contextToShare)
    {
        jassert (context == nil);
        context = [EAGLContext alloc];

        context = contextToShare != nullptr
                    ? [context initWithAPI: type  sharegroup: [(EAGLContext*) contextToShare sharegroup]]
                    : [context initWithAPI: type];

        return context != nil;
    }

    void releaseContext()
    {
        [context release];
        context = nil;
    }

    //==============================================================================
    void createGLBuffers()
    {
        glGenFramebuffers (1, &frameBufferHandle);
        glGenRenderbuffers (1, &colorBufferHandle);

        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);

        glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferHandle);

        bool ok = [context renderbufferStorage: GL_RENDERBUFFER fromDrawable: glLayer];
        jassert (ok); ignoreUnused (ok);

        GLint width, height;
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        if (useMSAA)
        {
            glGenFramebuffers (1, &msaaBufferHandle);
            glGenRenderbuffers (1, &msaaColorHandle);

            glBindFramebuffer (GL_FRAMEBUFFER, msaaBufferHandle);
            glBindRenderbuffer (GL_RENDERBUFFER, msaaColorHandle);

            glRenderbufferStorageMultisample (GL_RENDERBUFFER, 4, GL_RGBA8, width, height);

            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorHandle);
        }

        if (useDepthBuffer)
        {
            glGenRenderbuffers (1, &depthBufferHandle);
            glBindRenderbuffer (GL_RENDERBUFFER, depthBufferHandle);

            if (useMSAA)
                glRenderbufferStorageMultisample (GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, width, height);
            else
                glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferHandle);
        }

        jassert (glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        JUCE_CHECK_OPENGL_ERROR
    }

    void freeGLBuffers()
    {
        JUCE_CHECK_OPENGL_ERROR
        [context renderbufferStorage: GL_RENDERBUFFER fromDrawable: nil];

        deleteFrameBuffer (frameBufferHandle);
        deleteFrameBuffer (msaaBufferHandle);
        deleteRenderBuffer (colorBufferHandle);
        deleteRenderBuffer (depthBufferHandle);
        deleteRenderBuffer (msaaColorHandle);

        JUCE_CHECK_OPENGL_ERROR
    }

    static void deleteFrameBuffer  (GLuint& i)   { if (i != 0) glDeleteFramebuffers  (1, &i); i = 0; }
    static void deleteRenderBuffer (GLuint& i)   { if (i != 0) glDeleteRenderbuffers (1, &i); i = 0; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return [EAGLContext currentContext] != nil;
}
