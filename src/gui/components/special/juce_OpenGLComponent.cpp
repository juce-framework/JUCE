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

#include "../../../core/juce_StandardHeader.h"

#if JUCE_OPENGL

BEGIN_JUCE_NAMESPACE

#include "juce_OpenGLComponent.h"
#include "../layout/juce_ComponentMovementWatcher.h"
#include "../../../threads/juce_ScopedLock.h"


//==============================================================================
extern void juce_glViewport (const int w, const int h);


//==============================================================================
OpenGLPixelFormat::OpenGLPixelFormat (const int bitsPerRGBComponent,
                                      const int alphaBits_,
                                      const int depthBufferBits_,
                                      const int stencilBufferBits_) throw()
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

bool OpenGLPixelFormat::operator== (const OpenGLPixelFormat& other) const throw()
{
    return memcmp (this, &other, sizeof (other)) == 0;
}

//==============================================================================
static VoidArray knownContexts;

OpenGLContext::OpenGLContext() throw()
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
        OpenGLContext* const oglc = (OpenGLContext*) knownContexts.getUnchecked(i);

        if (oglc->isActive())
            return oglc;
    }

    return 0;
}


//==============================================================================
class OpenGLComponentWatcher  : public ComponentMovementWatcher
{
public:
    //==============================================================================
    OpenGLComponentWatcher (OpenGLComponent* const owner_)
        : ComponentMovementWatcher (owner_),
          owner (owner_),
          wasShowing (false)
    {
    }

    ~OpenGLComponentWatcher() {}

    //==============================================================================
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        owner->updateContextPosition();
    }

    void componentPeerChanged()
    {
        const ScopedLock sl (owner->getContextLock());
        owner->deleteContext();
    }

    void componentVisibilityChanged (Component&)
    {
        const bool isShowingNow = owner->isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;
            owner->updateContextPosition();
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OpenGLComponent* const owner;
    bool wasShowing;
};

//==============================================================================
OpenGLComponent::OpenGLComponent()
    : context (0),
      contextToShareListsWith (0),
      needToUpdateViewport (true)
{
    setOpaque (true);
    componentWatcher = new OpenGLComponentWatcher (this);
}

OpenGLComponent::~OpenGLComponent()
{
    deleteContext();
    delete componentWatcher;
}

void OpenGLComponent::deleteContext()
{
    const ScopedLock sl (contextLock);
    deleteAndZero (context);
}

void OpenGLComponent::updateContextPosition()
{
    needToUpdateViewport = true;

    if (getWidth() > 0 && getHeight() > 0)
    {
        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != 0)
        {
            const ScopedLock sl (contextLock);

            if (context != 0)
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
    if (context != 0)
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
    if (context == 0)
    {
        const ScopedLock sl (contextLock);

        if (isShowing() && getTopLevelComponent()->getPeer() != 0)
        {
            context = OpenGLContext::createContextForWindow (this,
                                                             preferredPixelFormat,
                                                             contextToShareListsWith);

            if (context != 0)
            {
                updateContextPosition();

                if (context->makeActive())
                    newOpenGLContextCreated();
            }
        }
    }

    return context != 0 && context->makeActive();
}

void OpenGLComponent::makeCurrentContextInactive()
{
    if (context != 0)
        context->makeInactive();
}

bool OpenGLComponent::isActiveContext() const throw()
{
    return context != 0 && context->isActive();
}

void OpenGLComponent::swapBuffers()
{
    if (context != 0)
        context->swapBuffers();
}

void OpenGLComponent::paint (Graphics&)
{
    if (renderAndSwapBuffers())
    {
        ComponentPeer* const peer = getPeer();

        if (peer != 0)
        {
            peer->addMaskedRegion (getScreenX() - peer->getScreenX(),
                                   getScreenY() - peer->getScreenY(),
                                   getWidth(), getHeight());
        }
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

void OpenGLComponent::internalRepaint (int x, int y, int w, int h)
{
    Component::internalRepaint (x, y, w, h);

    if (context != 0)
        context->repaint();
}


END_JUCE_NAMESPACE

#endif
