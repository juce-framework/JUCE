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
                   const OpenGLPixelFormat& pixelFormat,
                   void* contextToShareWith)
        : frameBufferHandle (0), colorBufferHandle (0), depthBufferHandle (0),
          lastWidth (0), lastHeight (0), needToRebuildBuffers (false),
          swapFrames (0), useDepthBuffer (pixelFormat.depthBufferBits > 0)
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
            glLayer.contentsScale = Desktop::getInstance().getDisplays().getMainDisplay().scale;

            [((UIView*) peer->getNativeHandle()) addSubview: view];

            context = [EAGLContext alloc];

            const NSUInteger type = kEAGLRenderingAPIOpenGLES2;

            if (contextToShareWith != nullptr)
                [context initWithAPI: type  sharegroup: [(EAGLContext*) contextToShareWith sharegroup]];
            else
                [context initWithAPI: type];

            // I'd prefer to put this stuff in the initialiseOnRenderThread() call, but doing
            // so causes myserious timing-related failures.
            [EAGLContext setCurrentContext: context];
            createGLBuffers();
            deactivateCurrentContext();
        }
    }

    ~NativeContext()
    {
        [context release];
        context = nil;

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
    GLuint getFrameBufferID() const noexcept    { return frameBufferHandle; }

    bool makeActive() const noexcept
    {
        if (! [EAGLContext setCurrentContext: context])
            return false;

        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
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
    GLuint frameBufferHandle, colorBufferHandle, depthBufferHandle;
    int volatile lastWidth, lastHeight;
    bool volatile needToRebuildBuffers;
    int swapFrames;
    bool useDepthBuffer;

    //==============================================================================
    void createGLBuffers()
    {
        glGenFramebuffers (1, &frameBufferHandle);
        glGenRenderbuffers (1, &colorBufferHandle);

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);
        bool ok = [context renderbufferStorage: GL_RENDERBUFFER fromDrawable: glLayer];
        jassert (ok); (void) ok;

        if (useDepthBuffer)
        {
            GLint width, height;
            glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
            glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

            glGenRenderbuffers (1, &depthBufferHandle);
            glBindRenderbuffer (GL_RENDERBUFFER, depthBufferHandle);
            glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        }

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);

        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
        glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferHandle);

        if (useDepthBuffer)
            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferHandle);

        jassert (glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        JUCE_CHECK_OPENGL_ERROR
    }

    void freeGLBuffers()
    {
        JUCE_CHECK_OPENGL_ERROR
        [context renderbufferStorage: GL_RENDERBUFFER fromDrawable: nil];

        if (frameBufferHandle != 0)
        {
            glDeleteFramebuffers (1, &frameBufferHandle);
            frameBufferHandle = 0;
        }

        if (colorBufferHandle != 0)
        {
            glDeleteRenderbuffers (1, &colorBufferHandle);
            colorBufferHandle = 0;
        }

        if (depthBufferHandle != 0)
        {
            glDeleteRenderbuffers (1, &depthBufferHandle);
            depthBufferHandle = 0;
        }

        JUCE_CHECK_OPENGL_ERROR
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return [EAGLContext currentContext] != nil;
}
