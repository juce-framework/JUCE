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

class AndroidViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (jobject v, Component& comp, bool makeSiblingRatherThanChild = false)
        : ComponentMovementWatcher (&comp),
          view (v),
          owner (comp),
          embedAsSiblingRatherThanChild (makeSiblingRatherThanChild)
    {
        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl()
    {
        removeFromParent();
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        auto* topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            auto pos = topComp->getLocalPoint (&owner, Point<int>());

            Rectangle<int> r (pos.x, pos.y, owner.getWidth(), owner.getHeight());
            r *= Desktop::getInstance().getDisplays().getMainDisplay().scale;

            getEnv()->CallVoidMethod (view, AndroidView.layout, r.getX(), r.getY(),
                                      r.getRight(), r.getBottom());
        }
    }

    void componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();

            currentPeer = peer;

            addToParent();
        }

        enum
        {
            VISIBLE   = 0,
            INVISIBLE = 4
        };

        getEnv()->CallVoidMethod (view, AndroidView.setVisibility, owner.isShowing() ? VISIBLE : INVISIBLE);
     }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    void componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);

        // Ensure that the native component doesn't get obscured.
        if (embedAsSiblingRatherThanChild)
            getEnv()->CallVoidMethod (view, AndroidView.bringToFront);
    }

    Rectangle<int> getViewBounds() const
    {
        auto* env = getEnv();

        int width  = env->CallIntMethod (view, AndroidView.getWidth);
        int height = env->CallIntMethod (view, AndroidView.getHeight);

        return Rectangle<int> (width, height);
    }

    GlobalRef view;

private:
    void addToParent()
    {
        if (currentPeer != nullptr)
        {
            jobject peerView = (jobject) currentPeer->getNativeHandle();

            // NB: Assuming a parent is always of ViewGroup type
            auto* env = getEnv();

            if (embedAsSiblingRatherThanChild)
            {
                // This is a workaround for a bug in a web browser component where
                // scrolling would be very slow and occasionally would scroll in
                // opposite direction to dragging direction. In normal circumstances,
                // the native view should be a child of peerView instead.
                auto parentView = LocalRef<jobject> (env->CallObjectMethod (peerView, AndroidView.getParent));
                env->CallVoidMethod (parentView, AndroidViewGroup.addView, view.get());
            }
            else
            {
                env->CallVoidMethod (peerView, AndroidViewGroup.addView, view.get());
            }

            componentMovedOrResized (false, false);
        }
    }

    void removeFromParent()
    {
        auto* env = getEnv();
        auto parentView = env->CallObjectMethod (view, AndroidView.getParent);

        if (parentView != 0)
        {
            // Assuming a parent is always of ViewGroup type
            env->CallVoidMethod (parentView, AndroidViewGroup.removeView, view.get());
        }
    }

    Component& owner;
    bool embedAsSiblingRatherThanChild;
    ComponentPeer* currentPeer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
AndroidViewComponent::AndroidViewComponent (bool makeSiblingRatherThanChild)
    : embedAsSiblingRatherThanChild (makeSiblingRatherThanChild)
{
}

AndroidViewComponent::~AndroidViewComponent() {}

void AndroidViewComponent::setView (void* view)
{
    if (view != getView())
    {
        pimpl.reset();

        if (view != nullptr)
            pimpl.reset (new Pimpl ((jobject) view, *this, embedAsSiblingRatherThanChild));
    }
}

void* AndroidViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : (void*) pimpl->view;
}

void AndroidViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

void AndroidViewComponent::paint (Graphics&) {}

} // namespace juce
