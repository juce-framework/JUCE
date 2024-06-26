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

//==============================================================================
class UIATransformProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ITransformProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT Move (double x, double y) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

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
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

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
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        return (HRESULT) UIA_E_NOTSUPPORTED;
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
