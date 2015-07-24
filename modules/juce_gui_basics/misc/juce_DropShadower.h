/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_DROPSHADOWER_H_INCLUDED
#define JUCE_DROPSHADOWER_H_INCLUDED


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

    void componentMovedOrResized (Component&, bool, bool) override;
    void componentBroughtToFront (Component&) override;
    void componentChildrenChanged (Component&) override;
    void componentParentHierarchyChanged (Component&) override;
    void componentVisibilityChanged (Component&) override;

    void updateParent();
    void updateShadows();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DropShadower)
};


#endif   // JUCE_DROPSHADOWER_H_INCLUDED
