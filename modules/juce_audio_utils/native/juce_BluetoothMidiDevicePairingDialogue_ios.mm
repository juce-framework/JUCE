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

#if ! TARGET_IPHONE_SIMULATOR

namespace juce
{

//==============================================================================
class BluetoothMidiSelectorOverlay final : public Component
{
public:
    BluetoothMidiSelectorOverlay (ModalComponentManager::Callback* exitCallbackToUse,
                                  const Rectangle<int>& boundsToUse)
        : bounds (boundsToUse)
    {
        std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackToUse);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);

        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        toFront (true);
        setOpaque (true);

        controller = [[CABTMIDICentralViewController alloc] init];
        nativeSelectorComponent.setView ([controller view]);

        addAndMakeVisible (nativeSelectorComponent);

        enterModalState (true, exitCallback.release(), true);
    }

    ~BluetoothMidiSelectorOverlay() override
    {
        nativeSelectorComponent.setView (nullptr);
        [controller release];
    }

    void paint (Graphics& g) override
    {
        g.fillAll (bounds.isEmpty() ? Colours::black.withAlpha (0.5f) : Colours::black);
    }

    void inputAttemptWhenModal() override           { close(); }
    void mouseDrag (const MouseEvent&) override     {}
    void mouseDown (const MouseEvent&) override     { close(); }
    void resized() override                         { update(); }
    void parentSizeChanged() override               { update(); }

private:
    void update()
    {
        if (bounds.isEmpty())
        {
            const int pw = getParentWidth();
            const int ph = getParentHeight();

            nativeSelectorComponent.setBounds (Rectangle<int> (pw, ph)
                                                 .withSizeKeepingCentre (jmin (400, pw),
                                                                         jmin (450, ph - 40)));
        }
        else
        {
            nativeSelectorComponent.setBounds (bounds.withZeroOrigin());
        }
    }

    void close()
    {
        exitModalState (0);
        setVisible (false);
    }

    CABTMIDICentralViewController* controller;
    UIViewComponent nativeSelectorComponent;
    Rectangle<int> bounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

bool BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                               Rectangle<int>* btBounds)
{
    std::unique_ptr<ModalComponentManager::Callback> cb (exitCallback);
    auto boundsToUse = (btBounds != nullptr ? *btBounds : Rectangle<int> {});

    if (isAvailable())
    {
        new BluetoothMidiSelectorOverlay (cb.release(), boundsToUse);
        return true;
    }

    return false;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
    return NSClassFromString (@"CABTMIDICentralViewController") != nil;
}

} // namespace juce

//==============================================================================
#else

namespace juce
{
    bool BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                                   Rectangle<int>*)
    {
        std::unique_ptr<ModalComponentManager::Callback> cb (exitCallback);
        return false;
    }

    bool BluetoothMidiDevicePairingDialogue::isAvailable()  { return false; }
}

#endif
