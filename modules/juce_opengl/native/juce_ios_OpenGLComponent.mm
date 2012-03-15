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

//==============================================================================
class GLESContext   : public OpenGLContext
{
public:
    GLESContext (UIView* parentView,
                 Component* const component_,
                 const OpenGLPixelFormat& pixelFormat,
                 const GLESContext* const sharedContext,
                 const bool isGLES2_)
        : component (component_), glLayer (nil), context (nil),
          useDepthBuffer (pixelFormat.depthBufferBits > 0), isGLES2 (isGLES2_),
          frameBufferHandle (0), colorBufferHandle (0),
          depthBufferHandle (0), lastWidth (0), lastHeight (0)
    {
        view = [[JuceGLView alloc] initWithFrame: CGRectMake (0, 0, 64, 64)];
        view.opaque = YES;
        view.hidden = NO;
        view.backgroundColor = [UIColor blackColor];
        view.userInteractionEnabled = NO;

        glLayer = (CAEAGLLayer*) [view layer];
        [parentView addSubview: view];

        NSUInteger apiType = isGLES2_ ? kEAGLRenderingAPIOpenGLES2
                                      : kEAGLRenderingAPIOpenGLES1;

        if (sharedContext != nullptr)
            context = [[EAGLContext alloc] initWithAPI: apiType
                                            sharegroup: [sharedContext->context sharegroup]];
        else
            context = [[EAGLContext alloc] initWithAPI: apiType];

        createGLBuffers();
    }

    ~GLESContext()
    {
        properties.clear(); // to release any stored programs, etc that may be held in properties.
        makeInactive();
        [context release];
        context = nil;

        [view removeFromSuperview];
        [view release];
        freeGLBuffers();
    }

    bool makeActive() const noexcept
    {
        jassert (context != nil);

        [EAGLContext setCurrentContext: context];
        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
        return true;
    }

    void swapBuffers()
    {
        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);
        [context presentRenderbuffer: GL_RENDERBUFFER];
    }

    bool makeInactive() const noexcept
    {
        return [EAGLContext setCurrentContext: nil];
    }

    bool isActive() const noexcept                  { return [EAGLContext currentContext] == context; }

    void* getRawContext() const noexcept            { return glLayer; }
    unsigned int getFrameBufferID() const           { return (unsigned int) frameBufferHandle; }

    int getWidth() const                            { return lastWidth; }
    int getHeight() const                           { return lastHeight; }

    bool areShadersAvailable() const                { return isGLES2; }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        // For some strange reason, the view seems to fail unless its width is a multiple of 8...
        view.frame = CGRectMake ((CGFloat) bounds.getX(), (CGFloat) bounds.getY(),
                                 (CGFloat) (bounds.getWidth() & ~7),
                                 (CGFloat) bounds.getHeight());

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

    //==============================================================================
    void createGLBuffers()
    {
        makeActive();

        glGenFramebuffers (1, &frameBufferHandle);
        glGenRenderbuffers (1, &colorBufferHandle);
        glGenRenderbuffers (1, &depthBufferHandle);

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);
        bool ok = [context renderbufferStorage: GL_RENDERBUFFER fromDrawable: glLayer];
        jassert (ok); (void) ok;

        GLint width, height;
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        if (useDepthBuffer)
        {
            glBindRenderbuffer (GL_RENDERBUFFER, depthBufferHandle);
            glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        }

        glBindRenderbuffer (GL_RENDERBUFFER, colorBufferHandle);

        glBindFramebuffer (GL_FRAMEBUFFER, frameBufferHandle);
        glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferHandle);

        if (useDepthBuffer)
            glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferHandle);

        jassert (glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }

    void freeGLBuffers()
    {
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
    }

private:
    WeakReference<Component> component;
    JuceGLView* view;
    CAEAGLLayer* glLayer;
    EAGLContext* context;
    bool useDepthBuffer, isGLES2;
    GLuint frameBufferHandle, colorBufferHandle, depthBufferHandle;
    int numFrames;
    int lastWidth, lastHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GLESContext);
};


OpenGLContext* OpenGLComponent::createContext()
{
    JUCE_AUTORELEASEPOOL
    ComponentPeer* peer = getPeer();

    if (peer != nullptr)
        return new GLESContext ((UIView*) peer->getNativeHandle(), this, preferredPixelFormat,
                                dynamic_cast <const GLESContext*> (contextToShareListsWith),
                                (flags & openGLES2) != 0);

    return nullptr;
}

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return [EAGLContext currentContext] != nil;
}
