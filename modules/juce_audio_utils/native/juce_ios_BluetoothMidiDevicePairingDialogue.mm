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

// Note: for the Bluetooth Midi selector overlay, we need the class
// UIViewComponent from the juce_gui_extra module. If this module is not
// included in your app, BluetoothMidiDevicePairingDialogue::open() will fail
// and return false.
// It is also not available in the iPhone/iPad simulator.
#if JUCE_MODULE_AVAILABLE_juce_gui_extra && ! TARGET_IPHONE_SIMULATOR

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
        ScopedPointer<ModalComponentManager::Callback> exitCallback (exitCallbackToUse);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);

        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        toFront (true);
        setOpaque (! bounds.isEmpty());

        controller = [[CABTMIDICentralViewController alloc] init];
        nativeSelectorComponent.setView ([controller view]);

        addAndMakeVisible (nativeSelectorComponent);

        enterModalState (true, exitCallback.release(), true);
    }

    ~BluetoothMidiSelectorOverlay()
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
    ScopedPointer<ModalComponentManager::Callback> cb (exitCallback);
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
    return NSClassFromString ([NSString stringWithUTF8String: "CABTMIDICentralViewController"]) != nil;
}

} // namespace juce

//==============================================================================
#else

namespace juce
{
    bool BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                                   Rectangle<int>*)
    {
        ScopedPointer<ModalComponentManager::Callback> cb (exitCallback);
        return false;
    }

    bool BluetoothMidiDevicePairingDialogue::isAvailable()  { return false; }
}

#endif
