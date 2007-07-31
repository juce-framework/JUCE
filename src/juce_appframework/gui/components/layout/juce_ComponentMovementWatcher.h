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

#ifndef __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
#define __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__

#include "../juce_Component.h"
#include "../juce_ComponentDeletionWatcher.h"


//==============================================================================
/** An object that watches for any movement of a component or any of its parent components.

    This makes it easy to check when a component is moved relative to its top-level
    peer window. The normal Component::moved() method is only called when a component
    moves relative to its immediate parent, and sometimes you want to know if any of
    components higher up the tree have moved (which of course will affect the overall
    position of all their sub-components).

    It also includes a callback that lets you know when the top-level peer is changed.

    This class is used by specialised components like OpenGLComponent or QuickTimeComponent
    because they need to keep their custom windows in the right place and respond to
    changes in the peer.
*/
class JUCE_API  ComponentMovementWatcher    : public ComponentListener
{
public:
    //==============================================================================
    /** Creates a ComponentMovementWatcher to watch a given target component. */
    ComponentMovementWatcher (Component* const component);

    /** Destructor. */
    ~ComponentMovementWatcher();

    //==============================================================================
    /** This callback happens when the component that is being watched is moved
        relative to its top-level peer window, or when it is resized.
    */
    virtual void componentMovedOrResized (bool wasMoved, bool wasResized) = 0;

    /** This callback happens when the component's top-level peer is changed.
    */
    virtual void componentPeerChanged() = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator

    /** @internal */
    void componentParentHierarchyChanged (Component& component);
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);


private:
    //==============================================================================
    Component* const component;
    ComponentPeer* lastPeer;
    VoidArray registeredParentComps;
    bool reentrant;
    int lastX, lastY, lastWidth, lastHeight;
#ifdef JUCE_DEBUG
    ComponentDeletionWatcher* deletionWatcher;
#endif

    void unregister() throw();
    void registerWithParentComps() throw();

    ComponentMovementWatcher (const ComponentMovementWatcher&);
    const ComponentMovementWatcher& operator= (const ComponentMovementWatcher&);
};


#endif   // __JUCE_COMPONENTMOVEMENTWATCHER_JUCEHEADER__
