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

#include "../../../core/juce_StandardHeader.h"

#if JUCE_OPENGL

BEGIN_JUCE_NAMESPACE

#include "juce_OpenGLComponent.h"
#include "../windows/juce_ComponentPeer.h"
#include "../layout/juce_ComponentMovementWatcher.h"
#include "../../../threads/juce_Thread.h"


//==============================================================================
extern void juce_glViewport (const int w, const int h);


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
      fullSceneAntiAliasingNumSamples (0)
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
      fullSceneAntiAliasingNumSamples (other.fullSceneAntiAliasingNumSamples)
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
    fullSceneAntiAliasingNumSamples = other.fullSceneAntiAliasingNumSamples;
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
            && fullSceneAntiAliasingNumSamples == other.fullSceneAntiAliasingNumSamples;
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
class OpenGLComponent::OpenGLComponentRenderThread  : public Thread,
                                                      public AsyncUpdater
{
public:
    //==============================================================================
    OpenGLComponentRenderThread (OpenGLComponent& owner_)
        : Thread ("OpenGL Render"),
          owner (owner_)
    {
    }

    void run()
    {
        // Context will get created and callback triggered on first render
        while (owner.renderAndSwapBuffers() && ! threadShouldExit())
            owner.waitAfterSwapping();

        triggerAsyncUpdate();
    }

    void handleAsyncUpdate()
    {
        owner.stopRendering();
    }

    //==============================================================================
private:
    OpenGLComponent& owner;

    JUCE_DECLARE_NON_COPYABLE (OpenGLComponentRenderThread);
};



//==============================================================================
class OpenGLComponent::OpenGLComponentWatcher  : public ComponentMovementWatcher
{
public:
    //==============================================================================
    OpenGLComponentWatcher (OpenGLComponent* const owner_)
        : ComponentMovementWatcher (owner_),
          owner (owner_)
    {
    }

    //==============================================================================
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        owner->updateContextPosition();
    }

    void componentPeerChanged()
    {
        const ScopedLock sl (owner->getContextLock());
        owner->stopRendering();
    }

    void componentVisibilityChanged()
    {
        if (! owner->isShowing())
        {
            const ScopedLock sl (owner->getContextLock());
            owner->stopRendering();
        }
    }

    //==============================================================================
private:
    OpenGLComponent* const owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponentWatcher);
};

//==============================================================================
OpenGLComponent::OpenGLComponent (const OpenGLType type_)
    : type (type_),
      contextToShareListsWith (nullptr),
      needToUpdateViewport (true)
{
    setOpaque (true);
    componentWatcher = new OpenGLComponentWatcher (this);
}

OpenGLComponent::~OpenGLComponent()
{
    deleteContext();
    componentWatcher = nullptr;
}

void OpenGLComponent::deleteContext()
{
    const ScopedLock sl (contextLock);
    context = nullptr;
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

            if (context != nullptr)
                context->updateWindowPosition (getScreenX() - topComp->getScreenX(),
                                               getScreenY() - topComp->getScreenY(),
                                               getWidth(),
                                               getHeight(),
                                               topComp->getHeight());
        }
    }
}

const OpenGLPixelFormat OpenGLComponent::getPixelFormat() const
{
    OpenGLPixelFormat pf;

    const ScopedLock sl (contextLock);
    if (context != nullptr)
        pf = context->getPixelFormat();

    return pf;
}

void OpenGLComponent::setPixelFormat (const OpenGLPixelFormat& formatToUse)
{
    if (! (preferredPixelFormat == formatToUse))
    {
        const ScopedLock sl (contextLock);
        deleteContext();
        preferredPixelFormat = formatToUse;
    }
}

void OpenGLComponent::shareWith (OpenGLContext* c)
{
    if (contextToShareListsWith != c)
    {
        const ScopedLock sl (contextLock);
        deleteContext();
        contextToShareListsWith = c;
    }
}

bool OpenGLComponent::makeCurrentContextActive()
{
    if (context == nullptr)
    {
        const ScopedLock sl (contextLock);

        if (isShowing() && getTopLevelComponent()->getPeer() != nullptr)
        {
            context = createContext();

            if (context != nullptr)
            {
                updateContextPosition();

                if (context->makeActive())
                    newOpenGLContextCreated();
            }
        }
    }

    return context != nullptr && context->makeActive();
}

void OpenGLComponent::makeCurrentContextInactive()
{
    if (context != nullptr)
        context->makeInactive();
}

bool OpenGLComponent::isActiveContext() const noexcept
{
    return context != nullptr && context->isActive();
}

void OpenGLComponent::swapBuffers()
{
    if (context != nullptr)
        context->swapBuffers();
}

void OpenGLComponent::setUsingDedicatedThread (bool useDedicatedThread) noexcept
{
    useThread = useDedicatedThread;
}

void OpenGLComponent::paint (Graphics&)
{
    if (useThread)
    {
        if (renderThread == nullptr)
            renderThread = new OpenGLComponentRenderThread (*this);

        if (! renderThread->isThreadRunning())
            renderThread->startThread (6);

        // fall-through and update the masking region
    }
    else
    {
        if (renderThread != nullptr && renderThread->isThreadRunning())
            renderThread->stopThread (5000);

        if (! renderAndSwapBuffers())
            return;
    }

    ComponentPeer* const peer = getPeer();

    if (peer != nullptr)
    {
        const Point<int> topLeft (getScreenPosition() - peer->getScreenPosition());
        peer->addMaskedRegion (topLeft.getX(), topLeft.getY(), getWidth(), getHeight());
    }
}

bool OpenGLComponent::renderAndSwapBuffers()
{
    const ScopedLock sl (contextLock);

    if (! makeCurrentContextActive())
        return false;

    if (needToUpdateViewport)
    {
        needToUpdateViewport = false;
        juce_glViewport (getWidth(), getHeight());
    }

    renderOpenGL();
    swapBuffers();

    return true;
}

void OpenGLComponent::waitAfterSwapping()
{
    jassert (renderThread != nullptr && Thread::getCurrentThread() == renderThread);

    Thread::sleep (20);
}

bool OpenGLComponent::stopRendering()
{
    const ScopedLock sl (contextLock);

    if (! makeCurrentContextActive())
        return false;

    releaseOpenGLContext(); // callback to allow for shutdown

    if (renderThread != nullptr && Thread::getCurrentThread() == renderThread)
    {
        // make the context inactive - if we're on a thread, this will release the context,
        // so the main thread can take it and do shutdown

        makeCurrentContextInactive();
    }
    else if (context != nullptr)
        context->deleteContext();

    return true;
}

void OpenGLComponent::internalRepaint (int x, int y, int w, int h)
{
    Component::internalRepaint (x, y, w, h);

    if (context != nullptr)
        context->repaint();
}


END_JUCE_NAMESPACE

#endif
