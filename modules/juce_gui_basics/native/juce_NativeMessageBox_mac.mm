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

namespace juce::detail
{

std::unique_ptr<ScopedMessageBoxInterface> ScopedMessageBoxInterface::create (const MessageBoxOptions& options)
{
    class OSXMessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit OSXMessageBox (const MessageBoxOptions& opts)
            : options (opts) {}

        void runAsync (std::function<void (int)> recipient) override
        {
            makeAlert();

            const auto onDone = [recipient] (NSModalResponse result)
            {
                recipient (convertResult (result));
            };

            if (auto* comp = options.getAssociatedComponent())
            {
                if (auto* peer = comp->getPeer())
                {
                    if (auto* view = static_cast<NSView*> (peer->getNativeHandle()))
                    {
                        if (auto* window = [view window])
                        {
                            [alertWindow.get() beginSheetModalForWindow: window completionHandler: ^(NSModalResponse result)
                            {
                                onDone (result);
                            }];

                            return;
                        }
                    }
                }
            }

            const auto result = [alertWindow.get() runModal];
            onDone (result);
        }

        int runSync() override
        {
            makeAlert();
            return convertResult ([alertWindow.get() runModal]);
        }

        void close() override
        {
            if (auto* alert = alertWindow.get())
                [[alert window] close];
        }

    private:
        static int convertResult (NSModalResponse response)
        {
            switch (response)
            {
                case NSAlertFirstButtonReturn:   return 0;
                case NSAlertSecondButtonReturn:  return 1;
                case NSAlertThirdButtonReturn:   return 2;
                default:                         break;
            }

            jassertfalse;
            return 0;
        }

        static void addButton (NSAlert* alert, const String& button)
        {
            if (! button.isEmpty())
                [alert addButtonWithTitle: juceStringToNS (button)];
        }

        void makeAlert()
        {
            NSAlert* alert = [[NSAlert alloc] init];

            [alert setMessageText:     juceStringToNS (options.getTitle())];
            [alert setInformativeText: juceStringToNS (options.getMessage())];

            [alert setAlertStyle: options.getIconType() == MessageBoxIconType::WarningIcon ? NSAlertStyleCritical
                                                                                           : NSAlertStyleInformational];

            const auto button1Text = options.getButtonText (0);

            addButton (alert, button1Text.isEmpty() ? "OK" : button1Text);
            addButton (alert, options.getButtonText (1));
            addButton (alert, options.getButtonText (2));

            alertWindow.reset (alert);
        }

        NSUniquePtr<NSAlert> alertWindow;
        MessageBoxOptions options;
        std::unique_ptr<ModalComponentManager::Callback> callback;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSXMessageBox)
    };

    return std::make_unique<OSXMessageBox> (options);
}

} // namespace juce::detail
