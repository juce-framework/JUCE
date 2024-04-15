/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            auto area = (peer->getAreaCoveredBy (owner).toFloat() * peer->getPlatformScaleFactor()).getSmallestIntegerContainer();

            UINT flagsToSend =  SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;

            if (! wasMoved)   flagsToSend |= SWP_NOMOVE;
            if (! wasResized) flagsToSend |= SWP_NOSIZE;

            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

            SetWindowPos (hwnd, nullptr, area.getX(), area.getY(), area.getWidth(), area.getHeight(), flagsToSend);

            invalidateHWNDAndChildren();
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

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
            InvalidateRect (hwnd, nullptr, TRUE);
     }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    void componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);
    }

    Rectangle<int> getHWNDBounds() const
    {
        if (auto* peer = owner.getPeer())
        {
            ScopedThreadDPIAwarenessSetter threadDpiAwarenessSetter { hwnd };

            RECT r;
            GetWindowRect (hwnd, &r);
            Rectangle<int> windowRectangle (r.right - r.left, r.bottom - r.top);

            return (windowRectangle.toFloat() / peer->getPlatformScaleFactor()).toNearestInt();
        }

        return {};
    }

    void invalidateHWNDAndChildren()
    {
        EnumChildWindows (hwnd, invalidateHwndCallback, 0);
    }

    static BOOL WINAPI invalidateHwndCallback (HWND hwnd, LPARAM)
    {
        InvalidateRect (hwnd, nullptr, TRUE);
        return TRUE;
    }

    HWND hwnd;

private:
    void addToParent()
    {
        if (currentPeer != nullptr)
        {
            auto windowFlags = GetWindowLongPtr (hwnd, GWL_STYLE);

            using FlagType = decltype (windowFlags);

            windowFlags &= ~(FlagType) WS_POPUP;
            windowFlags |= (FlagType) WS_CHILD;

            SetWindowLongPtr (hwnd, GWL_STYLE, windowFlags);
            SetParent (hwnd, (HWND) currentPeer->getNativeHandle());

            componentMovedOrResized (true, true);
        }
    }

    void removeFromParent()
    {
        ShowWindow (hwnd, SW_HIDE);
        SetParent (hwnd, nullptr);
    }

    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
HWNDComponent::HWNDComponent()  {}
HWNDComponent::~HWNDComponent() {}

void HWNDComponent::paint (Graphics&) {}

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

void HWNDComponent::updateHWNDBounds()
{
    if (pimpl != nullptr)
        pimpl->componentMovedOrResized (true, true);
}

} // namespace juce
