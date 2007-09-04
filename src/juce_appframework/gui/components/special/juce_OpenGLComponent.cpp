/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

#if JUCE_OPENGL

BEGIN_JUCE_NAMESPACE

#include "juce_OpenGLComponent.h"
#include "../layout/juce_ComponentMovementWatcher.h"
#include "../../../../juce_core/threads/juce_ScopedLock.h"


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
        owner->createContext();
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
      componentToShareListsWith (0),
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

void OpenGLComponent::createContext()
{
    const ScopedLock sl (contextLock);

    jassert (context == 0);

    if (context == 0 && isShowing() && getTopLevelComponent()->getPeer() != 0)
    {
        context = OpenGLContext::createContextForWindow (this,
                                                         preferredPixelFormat,
                                                         componentToShareListsWith != 0
                                                             ? componentToShareListsWith->context
                                                             : 0);

        if (context != 0)
        {
            updateContextPosition();

            if (makeCurrentContextActive())
                newOpenGLContextCreated();
        }
    }
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

            if (context == 0)
                createContext();

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

bool OpenGLComponent::setPixelFormat (const OpenGLPixelFormat& formatToUse)
{
    if (preferredPixelFormat == formatToUse)
        return true;

    const ScopedLock sl (contextLock);
    deleteContext();
    preferredPixelFormat = formatToUse;
    createContext();

    return context != 0;
}

void OpenGLComponent::shareWith (OpenGLComponent* const comp)
{
    if (componentToShareListsWith != comp)
    {
        const ScopedLock sl (contextLock);
        deleteContext();
        componentToShareListsWith = comp;
        createContext();
    }
}

bool OpenGLComponent::makeCurrentContextActive()
{
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
