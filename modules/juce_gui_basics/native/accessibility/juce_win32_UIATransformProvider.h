/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
class UIATransformProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ITransformProvider>
{
public:
    explicit UIATransformProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT Move (double x, double y) override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            RECT rect;
            GetWindowRect ((HWND) peer->getNativeHandle(), &rect);

            rect.left = roundToInt (x);
            rect.top  = roundToInt (y);

            auto bounds = Rectangle<int>::leftTopRightBottom (rect.left, rect.top, rect.right, rect.bottom);

            peer->setBounds (Desktop::getInstance().getDisplays().physicalToLogical (bounds),
                             peer->isFullScreen());
        }

        return S_OK;
    }

    JUCE_COMRESULT Resize (double width, double height) override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            auto scale = peer->getPlatformScaleFactor();

            peer->getComponent().setSize (roundToInt (width  / scale),
                                          roundToInt (height / scale));
        }

        return S_OK;
    }

    JUCE_COMRESULT Rotate (double) override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        return UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT get_CanMove (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = true;
            return S_OK;
        });
    }

    JUCE_COMRESULT get_CanResize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            if (auto* peer = getPeer())
                *pRetVal = ((peer->getStyleFlags() & ComponentPeer::windowIsResizable) != 0);

            return S_OK;
        });
    }

    JUCE_COMRESULT get_CanRotate (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = false;
            return S_OK;
        });
    }

private:
    ComponentPeer* getPeer() const
    {
        return getHandler().getComponent().getPeer();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATransformProvider)
};

} // namespace juce
