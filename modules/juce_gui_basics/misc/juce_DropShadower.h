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

#ifndef __JUCE_DROPSHADOWER_JUCEHEADER__
#define __JUCE_DROPSHADOWER_JUCEHEADER__

#include "../components/juce_Component.h"


//==============================================================================
/**
    Adds a drop-shadow to a component.

    This object creates and manages a set of components which sit around a
    component, creating a gaussian shadow around it. The components will track
    the position of the component and if it's brought to the front they'll also
    follow this.

    For desktop windows you don't need to use this class directly - just
    set the Component::windowHasDropShadow flag when calling
    Component::addToDesktop(), and the system will create one of these if it's
    needed (which it obviously isn't on the Mac, for example).
*/
class JUCE_API  DropShadower  : private ComponentListener
{
public:
    //==============================================================================
    /** Creates a DropShadower. */
    DropShadower (const DropShadow& shadowType);

    /** Destructor. */
    ~DropShadower();

    /** Attaches the DropShadower to the component you want to shadow. */
    void setOwner (Component* componentToFollow);


private:
    //==============================================================================
    class ShadowWindow;

    Component* owner;
    OwnedArray<Component> shadowWindows;
    DropShadow shadow;
    bool reentrant;
    WeakReference<Component> lastParentComp;

    void componentMovedOrResized (Component&, bool, bool);
    void componentBroughtToFront (Component&);
    void componentChildrenChanged (Component&);
    void componentParentHierarchyChanged (Component&);
    void componentVisibilityChanged (Component&);

    void updateParent();
    void updateShadows();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DropShadower)
};


#endif   // __JUCE_DROPSHADOWER_JUCEHEADER__
