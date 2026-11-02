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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

        void runAsync (std::function<void (std::optional<int>)> recipient) override
        {
            const auto resultCodes = makeAlert();

            const auto onDone = [resultCodes, recipient, opt = options] (NSModalResponse result)
            {
                const auto index = convertResult (result);
                const auto code = index.has_value() ? std::make_optional (resultCodes[*index]) : std::nullopt;

                recipient (code);
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

            const NSUniquePtr<NSAlert> alert { [alertWindow.get() retain] };
            onDone ([alert.get() runModal]);
        }

        std::optional<int> runSync() override
        {
            const auto resultCodes = makeAlert();
            const auto index = convertResult ([alertWindow.get() runModal]);

            return index.has_value() ? std::make_optional (resultCodes[*index]) : std::nullopt;
        }

        void close() override
        {
            if (alertWindow == nullptr)
                return;

            auto* window = [alertWindow.get() window];

            if ([NSApp modalWindow] == window)
                [NSApp stopModalWithCode: NSModalResponseAbort];
            else if (auto* parent = [window sheetParent])
                [parent endSheet: window returnCode: NSModalResponseAbort];

            [window close];
        }

    private:
        static std::optional<int> convertResult (NSModalResponse response)
        {
            switch (response)
            {
                case NSAlertFirstButtonReturn:   return 0;
                case NSAlertSecondButtonReturn:  return 1;
                case NSAlertThirdButtonReturn:   return 2;
                default:                         break;
            }

            return {};
        }

        Array<int> makeAlert()
        {
            NSAlert* alert = [[NSAlert alloc] init];

            [alert setMessageText:     juceStringToNS (options.getTitle())];
            [alert setInformativeText: juceStringToNS (options.getMessage())];

            [alert setAlertStyle: options.getIconType() == MessageBoxIconType::WarningIcon ? NSAlertStyleCritical
                                                                                           : NSAlertStyleInformational];

            const auto buttonOrder = AlertWindowHelpers::getSuggestedButtonOrder (options);

            const String defaultButtonStrings[] { TRANS ("OK"), String{}, String{} };
            Array<int> resultCodes;
            NSButton* cancelButton = nullptr;

            for (const auto [index, fallback] : enumerate (defaultButtonStrings, size_t{}))
            {
                const auto* button = index < buttonOrder.size()
                                   ? &options.getButtons()[(size_t) buttonOrder[index]]
                                   : nullptr;
                const auto text = button != nullptr ? button->getText() : fallback;

                if (text.isEmpty())
                    continue;

                [alert addButtonWithTitle: juceStringToNS (text)];
                resultCodes.add (button != nullptr ? button->getCode() : resultCodes.size());

                if (button == nullptr)
                    continue;

                if (button->getRole() == MessageBoxButtonRole::destroy)
                {
                    if (@available (macOS 11, *))
                    {
                        [[[alert buttons] lastObject] setHasDestructiveAction: true];
                    }
                }
                else if (button->getRole() == MessageBoxButtonRole::cancel)
                {
                    cancelButton = [[alert buttons] lastObject];
                }
            }

            if (cancelButton != nullptr && cancelButton != [[alert buttons] firstObject])
                [cancelButton setKeyEquivalent: @"\x1b"]; // The escape code from Events.h

            alertWindow.reset (alert);

            return resultCodes;
        }

        NSUniquePtr<NSAlert> alertWindow;
        MessageBoxOptions options;
        std::unique_ptr<ModalComponentManager::Callback> callback;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSXMessageBox)
    };

    return std::make_unique<OSXMessageBox> (options);
}

} // namespace juce::detail
