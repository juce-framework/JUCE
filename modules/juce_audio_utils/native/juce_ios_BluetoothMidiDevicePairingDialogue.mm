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

#if ! TARGET_IPHONE_SIMULATOR

#include <CoreAudioKit/CoreAudioKit.h>

namespace juce
{

//==============================================================================
class BluetoothMidiSelectorOverlay  : public Component
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
