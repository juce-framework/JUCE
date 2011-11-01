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

BEGIN_JUCE_NAMESPACE


//==============================================================================
OpenGLPixelFormat::OpenGLPixelFormat (const int bitsPerRGBComponent,
                                      const int alphaBits_,
                                      const int depthBufferBits_,
                                      const int stencilBufferBits_)
    : redBits (bitsPerRGBComponent),
      greenBits (bitsPerRGBComponent),
      blueBits (bitsPerRGBComponent),
      alphaBits (alphaBits_),
      depthBufferBits (depthBufferBits_),
      stencilBufferBits (stencilBufferBits_),
      accumulationBufferRedBits (0),
      accumulationBufferGreenBits (0),
      accumulationBufferBlueBits (0),
      accumulationBufferAlphaBits (0),
      multisamplingLevel (0)
{
}

OpenGLPixelFormat::OpenGLPixelFormat (const OpenGLPixelFormat& other)
    : redBits (other.redBits),
      greenBits (other.greenBits),
      blueBits (other.blueBits),
      alphaBits (other.alphaBits),
      depthBufferBits (other.depthBufferBits),
      stencilBufferBits (other.stencilBufferBits),
      accumulationBufferRedBits (other.accumulationBufferRedBits),
      accumulationBufferGreenBits (other.accumulationBufferGreenBits),
      accumulationBufferBlueBits (other.accumulationBufferBlueBits),
      accumulationBufferAlphaBits (other.accumulationBufferAlphaBits),
      multisamplingLevel (other.multisamplingLevel)
{
}

OpenGLPixelFormat& OpenGLPixelFormat::operator= (const OpenGLPixelFormat& other)
{
    redBits = other.redBits;
    greenBits = other.greenBits;
    blueBits = other.blueBits;
    alphaBits = other.alphaBits;
    depthBufferBits = other.depthBufferBits;
    stencilBufferBits = other.stencilBufferBits;
    accumulationBufferRedBits = other.accumulationBufferRedBits;
    accumulationBufferGreenBits = other.accumulationBufferGreenBits;
    accumulationBufferBlueBits = other.accumulationBufferBlueBits;
    accumulationBufferAlphaBits = other.accumulationBufferAlphaBits;
    multisamplingLevel = other.multisamplingLevel;
    return *this;
}

bool OpenGLPixelFormat::operator== (const OpenGLPixelFormat& other) const
{
    return redBits == other.redBits
            && greenBits == other.greenBits
            && blueBits == other.blueBits
            && alphaBits == other.alphaBits
            && depthBufferBits == other.depthBufferBits
            && stencilBufferBits == other.stencilBufferBits
            && accumulationBufferRedBits == other.accumulationBufferRedBits
            && accumulationBufferGreenBits == other.accumulationBufferGreenBits
            && accumulationBufferBlueBits == other.accumulationBufferBlueBits
            && accumulationBufferAlphaBits == other.accumulationBufferAlphaBits
            && multisamplingLevel == other.multisamplingLevel;
}

//==============================================================================
static Array<OpenGLContext*> knownContexts;

OpenGLContext::OpenGLContext() noexcept
{
    knownContexts.add (this);
}

OpenGLContext::~OpenGLContext()
{
    knownContexts.removeValue (this);
}

OpenGLContext* OpenGLContext::getCurrentContext()
{
    for (int i = knownContexts.size(); --i >= 0;)
    {
        OpenGLContext* const oglc = knownContexts.getUnchecked(i);

        if (oglc->isActive())
            return oglc;
    }

    return nullptr;
}

//==============================================================================
class OpenGLComponent::OpenGLComponentWatcher  : public ComponentMovementWatcher
{
public:
    OpenGLComponentWatcher (OpenGLComponent* const owner_)
        : ComponentMovementWatcher (owner_),
          owner (owner_)
    {
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        owner->updateContextPosition();
    }

    void componentPeerChanged()
    {
        owner->recreateContextAsync();
    }

    void componentVisibilityChanged()
    {
        if (! owner->isShowing())
            owner->stopBackgroundThread();
    }

private:
    OpenGLComponent* const owner;

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

            if (! owner.renderAndSwapBuffers())
                break;

            const int elapsed = (int) (Time::getMillisecondCounter() - startOfRendering);
            Thread::sleep (jmax (1, 20 - elapsed));
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
OpenGLComponent::OpenGLComponent (const OpenGLType type_, const bool useBackgroundThread)
    : type (type_),
      contextToShareListsWith (nullptr),
      needToUpdateViewport (true),
      needToDeleteContext (false),
      threadStarted (false),
      useThread (useBackgroundThread)
{
    setOpaque (true);
    componentWatcher = new OpenGLComponentWatcher (this);
}

OpenGLComponent::~OpenGLComponent()
{
    stopBackgroundThread();
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
                if (! useThread)
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
        {
            const ScopedLock sl (contextLock);
            updateEmbeddedPosition (topComp->getLocalArea (this, getLocalBounds()));
        }
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

void OpenGLComponent::paint (Graphics&)
{
    ComponentPeer* const peer = getPeer();

    if (useThread)
    {
        if (peer != nullptr && isShowing())
        {
           #if ! JUCE_LINUX
            updateContext();
           #endif

            if (! threadStarted)
            {
                threadStarted = true;
                startRenderThread();
            }
        }
    }
    else
    {
        updateContext();

        if (! renderAndSwapBuffers())
            return;
    }

    if (peer != nullptr)
    {
        const Point<int> topLeft (getScreenPosition() - peer->getScreenPosition());
        peer->addMaskedRegion (topLeft.x, topLeft.y, getWidth(), getHeight());
    }
}

bool OpenGLComponent::renderAndSwapBuffers()
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
        swapBuffers();
    }

    return true;
}

unsigned int OpenGLComponent::getFrameBufferID() const
{
    return context != nullptr ? context->getFrameBufferID() : 0;
}

END_JUCE_NAMESPACE
