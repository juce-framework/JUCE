/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
struct OutlineWindowComponent  : public Component
{
    OutlineWindowComponent (Component* c, FocusOutline::OutlineWindowProperties& p)
      : target (c), props (p)
    {
        setVisible (true);
        setInterceptsMouseClicks (false, false);

        if (target->isOnDesktop())
        {
            setSize (1, 1);
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (auto* parent = target->getParentComponent())
        {
            auto targetIndex = parent->getIndexOfChildComponent (target);
            parent->addChildComponent (this, targetIndex + 1);
        }
    }

    void paint (Graphics& g) override
    {
        if (target != nullptr)
            props.drawOutline (g, getWidth(), getHeight());
    }

    void resized() override
    {
        repaint();
    }

    float getDesktopScaleFactor() const override
    {
        return target != nullptr ? target->getDesktopScaleFactor()
                                 : Component::getDesktopScaleFactor();
    }

private:
    WeakReference<Component> target;
    FocusOutline::OutlineWindowProperties& props;

    JUCE_DECLARE_NON_COPYABLE (OutlineWindowComponent)
};

//==============================================================================
FocusOutline::FocusOutline (std::unique_ptr<OutlineWindowProperties> props)
    : properties (std::move (props))
{
}

FocusOutline::~FocusOutline()
{
    if (owner != nullptr)
        owner->removeComponentListener (this);

    if (lastParentComp != nullptr)
        lastParentComp->removeComponentListener (this);
}

void FocusOutline::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != nullptr)
            owner->removeComponentListener (this);

        owner = componentToFollow;

        if (owner != nullptr)
            owner->addComponentListener (this);

        updateParent();
        updateOutlineWindow();
    }
}

void FocusOutline::componentMovedOrResized (Component& c, bool, bool)
{
    if (owner == &c)
        updateOutlineWindow();
}

void FocusOutline::componentBroughtToFront (Component& c)
{
    if (owner == &c)
        updateOutlineWindow();
}

void FocusOutline::componentParentHierarchyChanged (Component& c)
{
    if (owner == &c)
    {
        updateParent();
        updateOutlineWindow();
    }
}

void FocusOutline::componentVisibilityChanged (Component& c)
{
    if (owner == &c)
        updateOutlineWindow();
}

void FocusOutline::updateParent()
{
    lastParentComp = (owner != nullptr ? owner->getParentComponent()
                                       : nullptr);
}

void FocusOutline::updateOutlineWindow()
{
    if (reentrant)
        return;

    const ScopedValueSetter<bool> setter (reentrant, true);

    if (owner == nullptr)
    {
        outlineWindow = nullptr;
        return;
    }

    if (owner->isShowing()
         && owner->getWidth() > 0 && owner->getHeight() > 0)
    {
        if (outlineWindow == nullptr)
            outlineWindow = std::make_unique<OutlineWindowComponent> (owner, *properties);

        WeakReference<Component> deletionChecker (outlineWindow.get());

        outlineWindow->setAlwaysOnTop (owner->isAlwaysOnTop());

        if (deletionChecker == nullptr)
            return;

        const auto windowBounds = [this]
        {
            const auto bounds = properties->getOutlineBounds (*owner);

            if (lastParentComp != nullptr)
                return lastParentComp->getLocalArea (nullptr, bounds);

            return bounds;
        }();

        outlineWindow->setBounds (windowBounds);
    }
    else
    {
        outlineWindow = nullptr;
    }
}

} // namespace juce
