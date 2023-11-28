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
                            if (@available (macOS 10.9, *))
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
