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

extern "C" GLvoid glResolveMultisampleFramebufferAPPLE();

namespace juce
{

class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& c,
                   const OpenGLPixelFormat& pixFormat,
                   void* contextToShare,
                   bool multisampling,
                   OpenGLVersion version)
        : component (c), openGLversion (version),
          useDepthBuffer (pixFormat.depthBufferBits > 0),
          useMSAA (multisampling)
    {
        JUCE_AUTORELEASEPOOL
        {
            if (auto* peer = component.getPeer())
            {
                auto bounds = peer->getAreaCoveredBy (component);

                view = [[JuceGLView alloc] initWithFrame: convertToCGRect (bounds)];
                view.opaque = YES;
                view.hidden = NO;
                view.backgroundColor = [UIColor blackColor];
                view.userInteractionEnabled = NO;

                glLayer = (CAEAGLLayer*) [view layer];
                glLayer.opaque = true;

                updateWindowPosition (bounds);

                [((UIView*) peer->getNativeHandle()) addSubview: view];

                if (version == openGL3_2 && [[UIDevice currentDevice].systemVersion floatValue] >= 7.0)
                {
                    if (! createContext (kEAGLRenderingAPIOpenGLES3, contextToShare))
                    {
                        releaseContext();
                        createContext (kEAGLRenderingAPIOpenGLES2, contextToShare);
                    }
                }
                else
                {
                    createContext (kEAGLRenderingAPIOpenGLES2, contextToShare);
                }

                if (context != nil)
                {
                    // I'd prefer to put this stuff in the initialiseOnRenderThread() call, but doing
                    // so causes mysterious timing-related failures.
                    [EAGLContext setCurrentContext: context];
                    gl::loadFunctions();
                    createGLBuffers();
                    deactivateCurrentContext();
                }
                else
                {
                    jassertfalse;
                }
            }
            else
            {
                jassertfalse;
            }
        }
    }

    ~NativeContext()
    {
        releaseContext();
        [view removeFromSuperview];
        [view release];
    }

    bool initialiseOnRenderThread (OpenGLContext&)    { return true; }

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

            if (openGLversion >= openGL3_2)
            {
                auto w = roundToInt (lastBounds.getWidth()  * glLayer.contentsScale);
                auto h = roundToInt (lastBounds.getHeight() * glLayer.contentsScale);

                glBlitFramebuffer (0, 0, w, h,
                                   0, 0, w, h,
                                   GL_COLOR_BUFFER_BIT,
                                   GL_NEAREST);
            }
            else
            {
                ::glResolveMultisampleFramebufferAPPLE();
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

    void updateWindowPosition (Rectangle<int> bounds)
    {
        view.frame = convertToCGRect (bounds);
        glLayer.contentsScale = (CGFloat) (Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale
                                            / component.getDesktopScaleFactor());

        if (lastBounds != bounds)
        {
            lastBounds = bounds;
            needToRebuildBuffers = true;
        }
    }

    bool setSwapInterval (int numFramesPerSwap) noexcept
    {
        swapFrames = numFramesPerSwap;
        return false;
    }

    int getSwapInterval() const noexcept    { return swapFrames; }

    struct Locker { Locker (NativeContext&) {} };

private:
    Component& component;
    JuceGLView* view = nil;
    CAEAGLLayer* glLayer = nil;
    EAGLContext* context = nil;
    const OpenGLVersion openGLversion;
    const bool useDepthBuffer, useMSAA;

    GLuint frameBufferHandle = 0, colorBufferHandle = 0, depthBufferHandle = 0,
           msaaColorHandle = 0, msaaBufferHandle = 0;

    Rectangle<int> lastBounds;
    int swapFrames = 0;
    bool needToRebuildBuffers = false;

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

} // namespace juce
