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

class HWNDComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (HWND h, Component& comp)
        : ComponentMovementWatcher (&comp),
          hwnd (h),
          owner (comp)
    {
        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl() override
    {
        removeFromParent();
        DestroyWindow (hwnd);
    }

    void componentMovedOrResized (bool wasMoved, bool wasResized) override
    {
        auto* topComponent = owner.getTopLevelComponent();

        if (auto* peer = owner.getPeer())
        {
            auto pos = topComponent->getLocalPoint (&owner, Point<int>());

            auto scaled = (Rectangle<int> (pos.x, pos.y, owner.getWidth(), owner.getHeight()).toDouble()
                            * peer->getPlatformScaleFactor()).getSmallestIntegerContainer();

            DWORD windowFlags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;
            if (! wasMoved)    windowFlags |= SWP_NOMOVE;
            if (! wasResized)  windowFlags |= SWP_NOSIZE;

            SetWindowPos (hwnd, 0, scaled.getX(), scaled.getY(), scaled.getWidth(), scaled.getHeight(), windowFlags);
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

        auto isShowing = owner.isShowing();

        ShowWindow (hwnd, isShowing ? SW_SHOWNA : SW_HIDE);

        if (isShowing)
            InvalidateRect (hwnd, 0, 0);
     }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    void componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);
    }

    Rectangle<int> getHWNDBounds() const
    {
        if (auto* peer = owner.getPeer())
        {
            RECT r;
            GetWindowRect (hwnd, &r);

            return (Rectangle<int>::leftTopRightBottom (r.left, r.top, r.right, r.bottom).toDouble()
                     / peer->getPlatformScaleFactor()).getSmallestIntegerContainer();
        }

        return {};
    }

    HWND hwnd;

private:
    void addToParent()
    {
        if (currentPeer != nullptr)
        {
            auto windowFlags = GetWindowLongPtr (hwnd, -16);

            windowFlags &= ~(WS_POPUP | WS_CHILD);

            SetWindowLongPtr (hwnd, -16, windowFlags);
            SetParent (hwnd, (HWND) currentPeer->getNativeHandle());

            componentMovedOrResized (true, true);
        }
    }

    void removeFromParent()
    {
        SetParent (hwnd, NULL);
    }

    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
HWNDComponent::HWNDComponent()
{
}

HWNDComponent::~HWNDComponent() {}

void HWNDComponent::setHWND (void* hwnd)
{
    if (hwnd != getHWND())
    {
        pimpl.reset();

        if (hwnd != nullptr)
            pimpl.reset (new Pimpl ((HWND) hwnd, *this));
    }
}

void* HWNDComponent::getHWND() const
{
    return pimpl == nullptr ? nullptr : (void*) pimpl->hwnd;
}

void HWNDComponent::resizeToFit()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getHWNDBounds());
}

} // namespace juce
