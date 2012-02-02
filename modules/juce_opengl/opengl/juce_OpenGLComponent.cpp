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

class OpenGLComponent::OpenGLCachedComponentImage  : public CachedComponentImage,
                                                     public Timer // N.B. using a Timer rather than an AsyncUpdater
                                                                  // to avoid scheduling problems on Windows
{
public:
    OpenGLCachedComponentImage (OpenGLComponent& owner_)
        : owner (owner_)
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

                if (! owner.threadStarted)
                {
                    owner.threadStarted = true;
                    owner.startRenderThread();
                }
            }
        }
        else
        {
            owner.updateContext();

            if (isTimerRunning())
                timerCallback();
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
        frameBuffer.release();
        owner.releaseAsRenderingTarget();
    }

    void timerCallback()
    {
        stopTimer();

        owner.performRender();
        owner.releaseAsRenderingTarget();
    }

    void triggerRepaint()
    {
        owner.needToRepaint = true;

        if (! owner.isUsingDedicatedThread())
            startTimer (1000 / 70);
    }

    OpenGLFrameBuffer& getFrameBuffer (int width, int height)
    {
        const int fbW = frameBuffer.getWidth();
        const int fbH = frameBuffer.getHeight();

        if (fbW != width || fbH != height || ! frameBuffer.isValid())
        {
            jassert (owner.getCurrentContext() != nullptr);
            frameBuffer.initialise (*owner.getCurrentContext(), width, height);
            validArea.clear();
        }

        return frameBuffer;
    }

    RectangleList validArea;

private:
    OpenGLComponent& owner;
    OpenGLFrameBuffer frameBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLCachedComponentImage);
};

//==============================================================================
class OpenGLComponent::OpenGLComponentWatcher  : public ComponentMovementWatcher
{
public:
    OpenGLComponentWatcher (OpenGLComponent& owner_)
        : ComponentMovementWatcher (&owner_),
          owner (owner_)
    {
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        owner.updateContextPosition();
    }

    void componentPeerChanged()
    {
        owner.recreateContextAsync();
    }

    void componentVisibilityChanged()
    {
        if (! owner.isShowing())
            owner.stopBackgroundThread();
    }

private:
    OpenGLComponent& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponentWatcher);
};

//==============================================================================
class OpenGLComponent::OpenGLComponentRenderThread  : public Thread
{
public:
    OpenGLComponentRenderThread (OpenGLComponent& owner_)
        : Thread ("OpenGL Render"),
          owner (owner_)
    {
    }

    void run()
    {
       #if JUCE_LINUX
        {
            MessageManagerLock mml (this);

            if (! mml.lockWasGained())
                return;

            owner.updateContext();
            owner.updateContextPosition();
        }
       #endif

        while (! threadShouldExit())
        {
            const uint32 startOfRendering = Time::getMillisecondCounter();

            if (! owner.performRender())
                break;

            const int elapsed = (int) (Time::getMillisecondCounter() - startOfRendering);
            Thread::sleep (jmax (1, (1000 / 60) - elapsed));
        }

       #if JUCE_LINUX
        owner.deleteContext();
       #endif
    }

private:
    OpenGLComponent& owner;

    JUCE_DECLARE_NON_COPYABLE (OpenGLComponentRenderThread);
};

void OpenGLComponent::startRenderThread()
{
    if (renderThread == nullptr)
        renderThread = new OpenGLComponentRenderThread (*this);

    renderThread->startThread (6);
}

void OpenGLComponent::stopRenderThread()
{
    if (renderThread != nullptr)
    {
        renderThread->stopThread (5000);
        renderThread = nullptr;
    }

   #if ! JUCE_LINUX
    deleteContext();
   #endif
}

//==============================================================================
OpenGLComponent::OpenGLComponent (const int flags_)
    : flags (flags_),
      contextToShareListsWith (nullptr),
      needToUpdateViewport (true),
      needToDeleteContext (false),
      threadStarted (false),
      needToRepaint (true)
{
    setOpaque (true);
    setCachedComponentImage (cachedImage = new OpenGLCachedComponentImage (*this));
    componentWatcher = new OpenGLComponentWatcher (*this);
}

OpenGLComponent::~OpenGLComponent()
{
    if (isUsingDedicatedThread())
        stopBackgroundThread();
    else
        deleteContext();

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
                    updateContextPosition();

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
            setCachedComponentImage (nullptr);
            releaseOpenGLContext();
            context->makeInactive();
        }

        context = nullptr;
    }

    needToDeleteContext = false;
}

void OpenGLComponent::updateContextPosition()
{
    needToUpdateViewport = true;

    if (getWidth() > 0 && getHeight() > 0)
    {
        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
            updateEmbeddedPosition (topComp->getLocalArea (this, getLocalBounds()));
    }
}

void OpenGLComponent::stopBackgroundThread()
{
    if (threadStarted)
    {
        stopRenderThread();
        threadStarted = false;
    }
}

void OpenGLComponent::triggerRepaint()
{
    // you mustn't set your own cached image object for an OpenGLComponent!
    jassert (dynamic_cast<OpenGLCachedComponentImage*> (getCachedComponentImage()) == cachedImage);

    cachedImage->triggerRepaint();
}

void OpenGLComponent::paint (Graphics&)
{
}

unsigned int OpenGLComponent::getFrameBufferID() const
{
    return context != nullptr ? context->getFrameBufferID() : 0;
}

bool OpenGLComponent::performRender()
{
    const ScopedLock sl (contextLock);

   #if JUCE_LINUX
    updateContext();
   #endif

    if (context != nullptr)
    {
        if (! makeCurrentRenderingTarget())
            return false;

        if (needToUpdateViewport)
        {
            needToUpdateViewport = false;
            glViewport (0, 0, getWidth(), getHeight());
        }

        renderOpenGL();

        if ((flags & allowSubComponents) != 0)
        {
            contextLock.exit(); // (MM must be locked before the context lock)
            MessageManagerLock mmLock (renderThread);
            contextLock.enter();

            if (! mmLock.lockWasGained())
                return false;

            // you mustn't set your own cached image object for an OpenGLComponent!
            jassert (dynamic_cast<OpenGLCachedComponentImage*> (getCachedComponentImage()) == cachedImage);

            const Rectangle<int> bounds (getLocalBounds());
            OpenGLFrameBuffer& frameBuffer = cachedImage->getFrameBuffer (bounds.getWidth(), bounds.getHeight());

            if (needToRepaint)
            {
                needToRepaint = false;

                RectangleList invalid (bounds);
                invalid.subtract (cachedImage->validArea);
                cachedImage->validArea = bounds;

                if (! invalid.isEmpty())
                {
                    jassert (getCurrentContext() != nullptr);

                    {
                        OpenGLGraphicsContext g (*getCurrentContext(), frameBuffer);
                        g.clipToRectangleList (invalid);

                        g.setFill (Colours::transparentBlack);
                        g.fillRect (bounds, true);
                        g.setFill (Colours::black);

                        paintSelf (g);
                    }

                    makeCurrentRenderingTarget();
                }
            }

            glEnable (GL_TEXTURE_2D);
            context->extensions.glActiveTexture (GL_TEXTURE0);
            glBindTexture (GL_TEXTURE_2D, frameBuffer.getTextureID());

            jassert (bounds.getPosition() == Point<int>());
            context->copyTexture (bounds, bounds);
            glBindTexture (GL_TEXTURE_2D, 0);
        }

        swapBuffers();
    }

    return true;
}

void OpenGLComponent::paintSelf (OpenGLGraphicsContext& glRenderer)
{
    Graphics g (&glRenderer);

   #if JUCE_ENABLE_REPAINT_DEBUGGING
    g.saveState();
   #endif

    JUCE_TRY
    {
        paintEntireComponent (g, false);
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
