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

namespace juce
{

//==============================================================================
class API_AVAILABLE (macos (10.11)) BluetoothMidiPairingWindowClass : public ObjCClass<NSObject>
{
public:
    struct Callbacks
    {
        std::unique_ptr<ModalComponentManager::Callback> modalExit;
        std::function<void()> windowClosed;
    };

    BluetoothMidiPairingWindowClass()   : ObjCClass<NSObject> ("JUCEBluetoothMidiPairingWindowClass_")
    {
        addIvar<Callbacks*> ("callbacks");
        addIvar<CABTLEMIDIWindowController*> ("controller");

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (initWithCallbacks:),       initWithCallbacks);
        addMethod (@selector (show:),                    show);
        addMethod (@selector (receivedWindowWillClose:), receivedWindowWillClose);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        addMethod (@selector (dealloc), dealloc);

        registerClass();
    }

private:
    static CABTLEMIDIWindowController* getController (id self)
    {
        return getIvar<CABTLEMIDIWindowController*> (self, "controller");
    }

    static id initWithCallbacks (id self, SEL, Callbacks* cbs)
    {
        self = sendSuperclassMessage<id> (self, @selector (init));

        object_setInstanceVariable (self, "callbacks", cbs);
        object_setInstanceVariable (self, "controller", [CABTLEMIDIWindowController new]);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector (receivedWindowWillClose:)
                                                     name: @"NSWindowWillCloseNotification"
                                                   object: [getController (self) window]];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return self;
    }

    static void dealloc (id self, SEL)
    {
        [getController (self) release];
        sendSuperclassMessage<void> (self, @selector (dealloc));
    }

    static void show (id self, SEL, Rectangle<int>* bounds)
    {
        if (bounds != nullptr)
        {
            auto nsBounds = makeNSRect (*bounds);

            auto mainScreenHeight = []
            {
                if ([[NSScreen screens] count] == 0)
                    return (CGFloat) 0.0f;

                return [[[NSScreen screens] objectAtIndex: 0] frame].size.height;
            }();

            nsBounds.origin.y = mainScreenHeight - (nsBounds.origin.y + nsBounds.size.height);

            [getController (self).window setFrame: nsBounds
                                          display: YES];
        }

        [getController (self) showWindow: nil];
    }

    static void receivedWindowWillClose (id self, SEL, NSNotification*)
    {
        [[NSNotificationCenter defaultCenter] removeObserver: self];

        auto* cbs = getIvar<Callbacks*> (self, "callbacks");

        if (cbs->modalExit != nullptr)
            cbs->modalExit->modalStateFinished (0);

        cbs->windowClosed();
    }
};

class API_AVAILABLE (macos (10.11)) BluetoothMidiSelectorWindowHelper : public DeletedAtShutdown
{
public:
    BluetoothMidiSelectorWindowHelper (ModalComponentManager::Callback* exitCallback,
                                       Rectangle<int>* bounds)
    {
        std::unique_ptr<ModalComponentManager::Callback> exitCB (exitCallback);

        static BluetoothMidiPairingWindowClass cls;
        window.reset (cls.createInstance());

        auto deletionCB = [safeThis = WeakReference<BluetoothMidiSelectorWindowHelper> { this }]
        {
            if (safeThis != nullptr)
                delete safeThis.get();
        };

        callbacks.reset (new BluetoothMidiPairingWindowClass::Callbacks { std::move (exitCB),
                                                                          std::move (deletionCB) });

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [window.get() performSelector: @selector (initWithCallbacks:)
                           withObject: (id) callbacks.get()];
        [window.get() performSelector: @selector (show:)
                           withObject: (id) bounds];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

private:
    std::unique_ptr<NSObject, NSObjectDeleter> window;
    std::unique_ptr<BluetoothMidiPairingWindowClass::Callbacks> callbacks;

    JUCE_DECLARE_WEAK_REFERENCEABLE (BluetoothMidiSelectorWindowHelper)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorWindowHelper)
};

//==============================================================================
bool BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                               Rectangle<int>* bounds)
{
    if (@available (macOS 10.11, *))
    {
        new BluetoothMidiSelectorWindowHelper (exitCallback, bounds);
        return true;
    }

    std::unique_ptr<ModalComponentManager::Callback> cb (exitCallback);
    // This functionality is unavailable when targetting OSX < 10.11. Instead,
    // you should pair Bluetooth MIDI devices using the "Audio MIDI Setup" app
    // (located in /Applications/Utilities).
    jassertfalse;
    return false;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
    if (@available (macOS 10.11, *))
        return true;

    return false;
}

} // namespace juce
