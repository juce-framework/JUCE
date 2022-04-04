/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
