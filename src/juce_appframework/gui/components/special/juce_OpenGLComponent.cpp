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

#ifdef _MSC_VER
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif

#include "../../../../../juce_Config.h"

#if JUCE_OPENGL

#ifdef _WIN32
#include <windows.h>
#include <gl/gl.h>
#else
 #ifdef LINUX
  #include <GL/glx.h>
 #else
  #include <agl/agl.h>
 #endif
#endif

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#undef KeyPress
#include "juce_OpenGLComponent.h"
#include "../../graphics/geometry/juce_RectangleList.h"
#include "../../../events/juce_Timer.h"
#include "../layout/juce_ComponentMovementWatcher.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

//==============================================================================
extern void* juce_createOpenGLContext (OpenGLComponent* component, void* sharedContext);
extern void juce_deleteOpenGLContext (void* context);
extern bool juce_makeOpenGLContextCurrent (void* context);
extern void juce_swapOpenGLBuffers (void* context);
extern void juce_updateOpenGLWindowPos (void* context, Component* owner, Component* topComp);
extern void juce_repaintOpenGLWindow (void* context);

static VoidArray activeGLWindows (2);


//==============================================================================
class InternalGLContextHolder  : public ComponentMovementWatcher
{
private:
    OpenGLComponent* owner;
    void* context;
    InternalGLContextHolder* sharedContext;
    bool wasShowing;

public:
    bool needToUpdateViewport;

    //==============================================================================
    InternalGLContextHolder (OpenGLComponent* const owner_,
                             InternalGLContextHolder* const sharedContext_)
        : ComponentMovementWatcher (owner_),
          owner (owner_),
          context (0),
          sharedContext (sharedContext_),
          wasShowing (false),
          needToUpdateViewport (true)
    {
    }

    ~InternalGLContextHolder()
    {
        release();
    }

    //==============================================================================
    void release()
    {
        if (context != 0)
        {
            juce_deleteOpenGLContext (context);
            context = 0;
        }
    }

    void initialise()
    {
        jassert (context == 0);

        if (context == 0)
        {
            context = juce_createOpenGLContext (owner,
                                                sharedContext != 0 ? sharedContext->context
                                                                   : 0);

            if (context != 0)
            {
                componentMovedOrResized (true, true);

                if (makeCurrent())
                    owner->newOpenGLContextCreated();
            }
        }
    }

    //==============================================================================
    bool makeCurrent() const
    {
        return context != 0 && juce_makeOpenGLContextCurrent (context);
    }

    void swapBuffers() const
    {
        if (context != 0)
            juce_swapOpenGLBuffers (context);
    }

    void repaint() const
    {
        if (context != 0)
            juce_repaintOpenGLWindow (context);
    }

    //==============================================================================
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        if (owner->getWidth() > 0 && owner->getHeight() > 0)
        {
            Component* const topComp = owner->getTopLevelComponent();

            if (topComp->getPeer() != 0)
            {
                needToUpdateViewport = true;

                if (context == 0)
                {
                    if (owner->isShowing())
                        initialise();
                    else
                        return;
                }

                if (context != 0)
                    juce_updateOpenGLWindowPos (context, owner, topComp);
            }
        }
    }

    void componentPeerChanged()
    {
        release();

        if (owner->isShowing() && owner->getTopLevelComponent()->getPeer() != 0)
            initialise();
    }

    void componentVisibilityChanged (Component&)
    {
        if (wasShowing != owner->isShowing())
        {
            wasShowing = owner->isShowing();
            componentMovedOrResized (true, true);
        }
    }
};

//==============================================================================
OpenGLComponent::OpenGLComponent (OpenGLComponent* share)
{
    setOpaque (true);
    internalData = new InternalGLContextHolder (this, (InternalGLContextHolder*) (share != 0 ? share->internalData : 0));

    activeGLWindows.add (this);
}

OpenGLComponent::~OpenGLComponent()
{
    activeGLWindows.removeValue ((void*) this);

    InternalGLContextHolder* const context = (InternalGLContextHolder*) internalData;
    delete context;
}

bool OpenGLComponent::makeCurrentContextActive()
{
    InternalGLContextHolder* const context = (InternalGLContextHolder*) internalData;
    return context->makeCurrent();
}

void OpenGLComponent::makeCurrentContextInactive()
{
    juce_makeOpenGLContextCurrent (0);
}

void OpenGLComponent::swapBuffers()
{
    InternalGLContextHolder* const context = (InternalGLContextHolder*) internalData;

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
    if (! makeCurrentContextActive())
        return false;

    InternalGLContextHolder* const context = (InternalGLContextHolder*) internalData;

    if (context->needToUpdateViewport)
    {
        context->needToUpdateViewport = false;
        glViewport (0, 0, getWidth(), getHeight());
    }

    renderOpenGL();

    context->swapBuffers();

    return true;
}

void OpenGLComponent::internalRepaint (int x, int y, int w, int h)
{
    Component::internalRepaint (x, y, w, h);

    InternalGLContextHolder* const context = (InternalGLContextHolder*) internalData;
    context->repaint();
}


END_JUCE_NAMESPACE

#endif
