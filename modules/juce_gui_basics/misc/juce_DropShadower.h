/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

    @tags{GUI}
*/
class JUCE_API  DropShadower  : private ComponentListener
{
public:
    //==============================================================================
    /** Creates a DropShadower. */
    DropShadower (const DropShadow& shadowType);

    /** Destructor. */
    ~DropShadower() override;

    /** Attaches the DropShadower to the component you want to shadow. */
    void setOwner (Component* componentToFollow);

private:
    //==============================================================================
    void componentMovedOrResized (Component&, bool, bool) override;
    void componentBroughtToFront (Component&) override;
    void componentChildrenChanged (Component&) override;
    void componentParentHierarchyChanged (Component&) override;
    void componentVisibilityChanged (Component&) override;

    void updateParent();
    void updateShadows();

    class ShadowWindow;

    WeakReference<Component> owner;
    OwnedArray<Component> shadowWindows;
    DropShadow shadow;
    bool reentrant = false;
    WeakReference<Component> lastParentComp;

    class ParentVisibilityChangedListener;
    std::unique_ptr<ParentVisibilityChangedListener> visibilityChangedListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DropShadower)
    JUCE_DECLARE_WEAK_REFERENCEABLE (DropShadower)
};

} // namespace juce
