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

#if JUCE_ANDROID
 void triggerAndroidOpenGLRepaint (OpenGLContext*);
#endif

class OpenGLComponent::OpenGLCachedComponentImage  : public CachedComponentImage,
                                                     public Thread,
                                                     public Timer // N.B. using a Timer rather than an AsyncUpdater
                                                                  // to avoid scheduling problems on Windows
{
public:
    OpenGLCachedComponentImage (OpenGLComponent& owner_, bool renderComponents_)
        : Thread ("OpenGL Rendering"),
          owner (owner_), needToRepaint (true),
          renderComponents (renderComponents_)
    {}

    void paint (Graphics&)
    {
        ComponentPeer* const peer = owner.getPeer();

        if (peer != nullptr)
            peer->addMaskedRegion (owner.getScreenBounds() - peer->getScreenPosition());

        if (owner.isUsingDedicatedThread())
        {
            if (peer != nullptr && owner.isShowing())
            {
               #if ! JUCE_LINUX
                owner.updateContext();
               #endif

                owner.startRenderThread();
            }
        }
        else
        {
            owner.updateContext();

           #if JUCE_ANDROID
            triggerAndroidOpenGLRepaint (owner.getCurrentContext());
           #else
            if (isTimerRunning())
                timerCallback();
           #endif
        }
    }

    void invalidateAll()
    {
        validArea.clear();
        triggerRepaint();
    }

    void invalidate (const Rectangle<int>& area)
    {
        validArea.subtract (area);
        triggerRepaint();
    }

    void releaseResources()
    {
        owner.makeCurrentRenderingTarget();
        cachedImageFrameBuffer.release();
        owner.releaseAsRenderingTarget();
    }

    //==============================================================================
    void timerCallback()
    {
        stopTimer();

        renderFrame();
        owner.releaseAsRenderingTarget();
    }

    void triggerRepaint()
    {
        needToRepaint = true;

       #if JUCE_ANDROID
        triggerAndroidOpenGLRepaint (owner.getCurrentContext());
       #else
        if (! owner.isUsingDedicatedThread())
            startTimer (1000 / 70);
       #endif
    }

    void updateContextPosition()
    {
        if (owner.getWidth() > 0 && owner.getHeight() > 0)
        {
            Component* const topComp = owner.getTopLevelComponent();

            if (topComp->getPeer() != nullptr)
            {
                const Rectangle<int> bounds (topComp->getLocalArea (&owner, owner.getLocalBounds()));

                const ScopedLock sl (owner.contextLock);

                if (owner.context != nullptr)
                    owner.context->updateWindowPosition (bounds);
            }
        }
    }

    //==============================================================================
    void ensureFrameBufferSize (int width, int height)
    {
        const int fbW = cachedImageFrameBuffer.getWidth();
        const int fbH = cachedImageFrameBuffer.getHeight();

        if (fbW != width || fbH != height || ! cachedImageFrameBuffer.isValid())
        {
            jassert (owner.getCurrentContext() != nullptr);
            cachedImageFrameBuffer.initialise (*owner.getCurrentContext(), width, height);
            validArea.clear();
            JUCE_CHECK_OPENGL_ERROR
        }
    }

    void clearRegionInFrameBuffer (const RectangleList& list)
    {
        glClearColor (0, 0, 0, 0);
        glEnable (GL_SCISSOR_TEST);

        const GLuint previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();
        cachedImageFrameBuffer.makeCurrentRenderingTarget();

        for (RectangleList::Iterator i (list); i.next();)
        {
            const Rectangle<int>& r = *i.getRectangle();
            glScissor (r.getX(), owner.getHeight() - r.getBottom(), r.getWidth(), r.getHeight());
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        glDisable (GL_SCISSOR_TEST);
        owner.getCurrentContext()->extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
        JUCE_CHECK_OPENGL_ERROR
    }

    bool renderFrame()
    {
        const ScopedLock sl (owner.contextLock);

       #if JUCE_LINUX
        owner.updateContext();
       #endif

        OpenGLContext* const context = owner.getCurrentContext();

        if (context != nullptr)
        {
            if (! context->makeActive())
                return false;

            JUCE_CHECK_OPENGL_ERROR
            glViewport (0, 0, owner.getWidth(), owner.getHeight());
            owner.renderOpenGL();
            JUCE_CHECK_OPENGL_ERROR

            if (renderComponents)
                paintComponent (context);

            context->swapBuffers();
        }

        return true;
    }

    void paintComponent (OpenGLContext* const context)
    {
        jassert (context != nullptr);

        owner.contextLock.exit(); // (MM must be locked before the context lock)
        MessageManagerLock mmLock (this);
        owner.contextLock.enter();

        if (! mmLock.lockWasGained())
            return;

        // you mustn't set your own cached image object for an OpenGLComponent!
        jassert (dynamic_cast<OpenGLCachedComponentImage*> (owner.getCachedComponentImage()) == this);

        const Rectangle<int> bounds (owner.getLocalBounds());
        ensureFrameBufferSize (bounds.getWidth(), bounds.getHeight());

        if (needToRepaint)
        {
            needToRepaint = false;

            RectangleList invalid (bounds);
            invalid.subtract (validArea);
            validArea = bounds;

            if (! invalid.isEmpty())
            {
                clearRegionInFrameBuffer (invalid);

                {
                    ScopedPointer<LowLevelGraphicsContext> g (createOpenGLGraphicsContext (*context, cachedImageFrameBuffer));
                    g->clipToRectangleList (invalid);
                    paintOwner (*g);
                    JUCE_CHECK_OPENGL_ERROR
                }

                context->makeActive();
            }
        }

        JUCE_CHECK_OPENGL_ERROR
       #if ! JUCE_ANDROID
        glEnable (GL_TEXTURE_2D);
       #endif
        context->extensions.glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, cachedImageFrameBuffer.getTextureID());

        jassert (bounds.getPosition() == Point<int>());
        context->copyTexture (bounds, bounds, context->getWidth(), context->getHeight());
        glBindTexture (GL_TEXTURE_2D, 0);
        JUCE_CHECK_OPENGL_ERROR
    }

    void paintOwner (LowLevelGraphicsContext& context)
    {
        Graphics g (&context);

       #if JUCE_ENABLE_REPAINT_DEBUGGING
        g.saveState();
       #endif

        JUCE_TRY
        {
            owner.paintEntireComponent (g, false);
        }
        JUCE_CATCH_EXCEPTION

       #if JUCE_ENABLE_REPAINT_DEBUGGING
        // enabling this code will fill all areas that get repainted with a colour overlay, to show
        // clearly when things are being repainted.
        g.restoreState();

        static Random rng;
        g.fillAll (Colour ((uint8) rng.nextInt (255),
                           (uint8) rng.nextInt (255),
                           (uint8) rng.nextInt (255),
                           (uint8) 0x50));
       #endif
    }

    //==============================================================================
    void run()
    {
        initialise();

        while (! threadShouldExit())
        {
            const uint32 frameRenderStartTime = Time::getMillisecondCounter();

            if (renderFrame())
                waitForNextFrame (frameRenderStartTime);
        }

        shutdown();
    }

    void initialise()
    {
       #if JUCE_LINUX
        MessageManagerLock mml (this);

        if (mml.lockWasGained())
        {
            owner.updateContext();
            updateContextPosition();
        }
       #endif
    }

    void shutdown()
    {
       #if JUCE_LINUX
        owner.deleteContext();
       #endif
    }

    void waitForNextFrame (const uint32 frameRenderStartTime)
    {
        const int defaultFPS = 60;

        const int elapsed = (int) (Time::getMillisecondCounter() - frameRenderStartTime);
        Thread::sleep (jmax (1, (1000 / defaultFPS) - elapsed));
    }

    //==============================================================================
    RectangleList validArea;
    OpenGLComponent& owner;
    OpenGLFrameBuffer cachedImageFrameBuffer;
    bool needToRepaint;
    const bool renderComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLCachedComponentImage);
};

//==============================================================================
class OpenGLComponent::OpenGLComponentWatcher  : public ComponentMovementWatcher
{
public:
    OpenGLComponentWatcher (OpenGLComponent& owner_)
        : ComponentMovementWatcher (&owner_), owner (owner_)
    {
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        if (owner.cachedImage != nullptr)
            owner.cachedImage->updateContextPosition();
    }

    void componentPeerChanged()
    {
        owner.recreateContextAsync();
    }

    void componentVisibilityChanged()
    {
        if (owner.isShowing())
            owner.triggerRepaint();
        else
            owner.stopRenderThread();
    }

private:
    OpenGLComponent& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponentWatcher);
};

//==============================================================================
OpenGLComponent::OpenGLComponent (const int flags_)
   #if JUCE_ANDROID
    : flags (flags_ & ~useBackgroundThread),
   #else
    : flags (flags_),
   #endif
      contextToShareListsWith (nullptr),
      needToDeleteContext (false),
      cachedImage (nullptr)
{
    setOpaque (true);
    triggerRepaint();
    componentWatcher = new OpenGLComponentWatcher (*this);
}

OpenGLComponent::~OpenGLComponent()
{
    if (isUsingDedicatedThread())
    {
        /* If you're using a background thread, then your sub-class MUST call
           stopRenderThread() in its destructor! Otherwise, the thread could still
           be running while your sub-class isbeing destroyed, and so may make a call
           to your subclass's renderOpenGL() method when it no longer exists!
        */
        jassert (! getGLCachedImage()->isThreadRunning());

        stopRenderThread();
    }
    else
    {
        deleteContext();
    }

    componentWatcher = nullptr;
}

void OpenGLComponent::setPixelFormat (const OpenGLPixelFormat& formatToUse)
{
    if (! (preferredPixelFormat == formatToUse))
    {
        const ScopedLock sl (contextLock);
        preferredPixelFormat = formatToUse;
        recreateContextAsync();
    }
}

void OpenGLComponent::shareWith (OpenGLContext* c)
{
    if (contextToShareListsWith != c)
    {
        const ScopedLock sl (contextLock);
        contextToShareListsWith = c;
        recreateContextAsync();
    }
}

void OpenGLComponent::startRenderThread()
{
    getGLCachedImage()->startThread (6);
}

void OpenGLComponent::stopRenderThread()
{
    getGLCachedImage()->stopThread (5000);

   #if ! JUCE_LINUX
    deleteContext();
   #endif
}

void OpenGLComponent::recreateContextAsync()
{
    const ScopedLock sl (contextLock);
    needToDeleteContext = true;
    repaint();
}

bool OpenGLComponent::makeCurrentRenderingTarget()
{
    return context != nullptr && context->makeActive();
}

void OpenGLComponent::releaseAsRenderingTarget()
{
    if (context != nullptr)
        context->makeInactive();
}

void OpenGLComponent::swapBuffers()
{
    if (context != nullptr)
        context->swapBuffers();
}

void OpenGLComponent::updateContext()
{
    if (needToDeleteContext)
        deleteContext();

    if (context == nullptr)
    {
        const ScopedLock sl (contextLock);

        if (context == nullptr)
        {
            context = createContext();

            if (context != nullptr)
            {
               #if JUCE_LINUX
                if (! isUsingDedicatedThread())
               #endif
                    getGLCachedImage()->updateContextPosition();

                if (context->makeActive())
                {
                    newOpenGLContextCreated();
                    context->makeInactive();
                }
            }
        }
    }
}

void OpenGLComponent::deleteContext()
{
    const ScopedLock sl (contextLock);

    if (context != nullptr)
    {
        if (context->makeActive())
        {
            cachedImage = nullptr;
            setCachedComponentImage (nullptr);
            releaseOpenGLContext();
            context->makeInactive();
        }

        context = nullptr;
    }

    needToDeleteContext = false;
}

bool OpenGLComponent::rebuildContext()
{
    needToDeleteContext = true;
    updateContext();

    return context != nullptr && context->makeActive();
}

OpenGLComponent::OpenGLCachedComponentImage* OpenGLComponent::getGLCachedImage()
{
    // you mustn't set your own cached image object for an OpenGLComponent!
    jassert (cachedImage == nullptr
              || dynamic_cast<OpenGLCachedComponentImage*> (getCachedComponentImage()) == cachedImage);

    if (cachedImage == nullptr)
        setCachedComponentImage (cachedImage = new OpenGLCachedComponentImage (*this, (flags & allowSubComponents) != 0));

    return cachedImage;
}

void OpenGLComponent::triggerRepaint()
{
    getGLCachedImage()->triggerRepaint();
}

void OpenGLComponent::newOpenGLContextCreated() {}
void OpenGLComponent::releaseOpenGLContext() {}
void OpenGLComponent::paint (Graphics&) {}
