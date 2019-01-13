/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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

} // namespace juce
