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

#include <CoreAudioKit/CABTLEMIDIWindowController.h>

// This is a very basic wrapper class that can open a Bluetooth MIDI pairing dialogue
// on Mac OS X and notify it's owner when the window is being closed.
@interface OSXBluetoothMidiDialogueWrapper : NSObject {
    CABTLEMIDIWindowController* controller;
    std::function<void(void)> onWindowClose;
}

- (void) show;

@end

@implementation OSXBluetoothMidiDialogueWrapper
- (instancetype) init:(std::function<void(void)>) onWindowClose_ {
    self = [super init];
    
    if (self) {
        // This will increment the refcount on the controller.
        // We need to free the memory later.
        controller = [[CABTLEMIDIWindowController alloc] init];
        
        onWindowClose = onWindowClose_;
    }
    
    return self;
}

- (void) dealloc {
    // This will decrement the refcount on the controller. If everything is operating
    // normally, the refcount will be zero and the memory is freed.
    [controller release];
    
    [super dealloc];
}

- (void) show {
    [controller showWindow:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(receiveNotification:)
                                                 name:@"NSWindowWillCloseNotification"
                                               object:[controller window]];
}

- (void) receiveNotification:(NSNotification *) notification
{
    if ([[notification name] isEqualToString:@"NSWindowWillCloseNotification"]) {
        onWindowClose();
    }
}

@end

namespace juce
{
    
    //==============================================================================
    class BluetoothMidiSelectorOverlay  : public Component
    {
    public:
        BluetoothMidiSelectorOverlay (ModalComponentManager::Callback* exitCallbackToUse, const Rectangle<int>&) :
            exitCallback (exitCallbackToUse)
        {
            const auto windowCloseCallback = [&]
            {
                if (exitCallback) {
                    exitCallback->modalStateFinished(0);
                }
                
                // TODO: This is only called when the BLE MIDI pairing window is closed.
                //       Since the pairing window is not modal, the parent window can be closed first,
                //       and trigger the leak detector. Need to somehow make sure that can't happen.
                
                // To prevent leaking of this BluetoothMidiSelectorOverlay instance,
                // we need to trigger async deletion (that will happen some time after the modal callback)
                WeakReference<Component> target (this);
                MessageManager::callAsync ([=]
                    {
                       if (auto* c = target.get())
                           delete c;
                    });
            };
            
            nativeDialogue = [[OSXBluetoothMidiDialogueWrapper alloc] init:windowCloseCallback];
            [nativeDialogue show];
        }
        
        ~BluetoothMidiSelectorOverlay()
        {
            [nativeDialogue release];
        }
        
    private:
        OSXBluetoothMidiDialogueWrapper* nativeDialogue;
        std::unique_ptr<ModalComponentManager::Callback> exitCallback;
        
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
        return NSClassFromString ([NSString stringWithUTF8String: "CABTLEMIDIWindowController"]) != nil;
    }
    
} // namespace juce
