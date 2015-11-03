/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

// Note: for the Bluetooth Midi selector overlay, we need the class
// UIViewComponent from the juce_gui_extra module. If this module is not
// included in your app, BluetoothMidiDevicePairingDialogue::open() will fail
// and return false.
// It is also not available in the iPhone/iPad simulator.
#if JUCE_MODULE_AVAILABLE_juce_gui_extra && ! TARGET_IPHONE_SIMULATOR

} // (juce namespace)

#include <CoreAudioKit/CoreAudioKit.h>

//==============================================================================
@interface BluetoothSelectorView : NSObject

@property CABTMIDICentralViewController *central;
- (UIView*) getView;

@end

//==============================================================================
@implementation BluetoothSelectorView

- (instancetype) init
{
    self = [super init];
    self.central = [[CABTMIDICentralViewController alloc] init];
    return self;
}

- (UIView*) getView
{
    return self.central.view;
}

@end

namespace juce
{

//==============================================================================
class BluetoothMidiSelectorOverlay  : public Component
{
public:
    BluetoothMidiSelectorOverlay()
    {
        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);
        setBounds (0, 0, getParentWidth(), getParentHeight());
        toFront (true);

        nativeSelectorComponent.setView ([[[BluetoothSelectorView alloc] init] getView]);
        addAndMakeVisible (nativeSelectorComponent);

        enterModalState (true, nullptr, true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black.withAlpha (0.5f));
    }

    void inputAttemptWhenModal() override           { close(); }
    void mouseDrag (const MouseEvent&) override     {}
    void mouseDown (const MouseEvent&) override     { close(); }
    void resized () override                        { update(); }
    void parentSizeChanged() override               { update(); }

private:
    void update()
    {
        const int pw = getParentWidth();
        const int ph = getParentHeight();

        nativeSelectorComponent.setBounds (Rectangle<int> (pw, ph)
                                             .withSizeKeepingCentre (jmin (400, pw - 14),
                                                                     jmin (500, ph - 40)));
    }

    void close()
    {
        exitModalState (0);
        setVisible (false);
    }

    UIViewComponent nativeSelectorComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

#endif // JUCE_MODULE_AVAILABLE_juce_gui_extra && ! TARGET_IPHONE_SIMULATOR

//==============================================================================
bool BluetoothMidiDevicePairingDialogue::open()
{
   #if JUCE_MODULE_AVAILABLE_juce_gui_extra && ! TARGET_IPHONE_SIMULATOR
    if (isAvailable())
    {
        new BluetoothMidiSelectorOverlay();
        return true;
    }
   #endif

    return false;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
   #if JUCE_MODULE_AVAILABLE_juce_gui_extra && ! TARGET_IPHONE_SIMULATOR
    return NSClassFromString ([NSString stringWithUTF8String: "CABTMIDICentralViewController"]) != nil;
   #else
    return false;
   #endif
}
