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
class BluetoothMidiPairingWindowClass final : public ObjCClass<NSObject>
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
            auto nsBounds = makeCGRect (*bounds);

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

class BluetoothMidiSelectorWindowHelper final : public DeletedAtShutdown
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
    new BluetoothMidiSelectorWindowHelper (exitCallback, bounds);
    return true;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
    return true;
}

} // namespace juce
