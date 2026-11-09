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
    class MessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& opts) : options (opts) {}

        void runAsync (std::function<void (std::optional<int>)> recipient) override
        {
            auto* peerToUse = std::invoke ([&]() -> UIViewComponentPeer*
            {
                if (auto* comp = options.getAssociatedComponent())
                    if (auto* peer = comp->getPeer())
                        return static_cast<UIViewComponentPeer*> (peer);

                return iOSGlobals::currentlyFocusedPeer;
            });

            if (peerToUse == nullptr)
            {
                // Since iOS8, alert windows need to be associated with a window, so you need to
                // have at least one window on screen when you use this
                jassertfalse;
                return;
            }

            alert.reset ([[UIAlertController alertControllerWithTitle: juceStringToNS (options.getTitle())
                                                              message: juceStringToNS (options.getMessage())
                                                       preferredStyle: UIAlertControllerStyleAlert] retain]);

           #if JUCE_IOS_API_VERSION_CAN_BE_BUILT (16, 0)
            if (@available (iOS 16, *))
            {
                const auto severity = options.getIconType() == MessageBoxIconType::WarningIcon
                                    ? UIAlertControllerSeverityCritical
                                    : UIAlertControllerSeverityDefault;
                [alert.get() setSeverity: severity];
            }
           #endif

            auto buttonOrder = AlertWindowHelpers::getSuggestedButtonOrder (options);

            // The interface guidelines for iOS say that a two-button alert should have
            // the preferred action on the right, but buttons are presented left-to-right
            if (buttonOrder.size() == 2)
                std::reverse (buttonOrder.begin(), buttonOrder.end());

            const auto& buttons = options.getButtons();

            for (const auto buttonIndex : buttonOrder)
            {
                const auto& button = buttons[(size_t) buttonIndex];
                const auto text = button.getText();

                if (text.isEmpty())
                    continue;

                const auto style = std::invoke ([&]
                {
                    if (buttons.size() == 1)
                        return UIAlertActionStyleCancel;

                    switch (button.getRole())
                    {
                        case MessageBoxButtonRole::normal:
                            return UIAlertActionStyleDefault;

                        case MessageBoxButtonRole::cancel:
                            return UIAlertActionStyleCancel;

                        case MessageBoxButtonRole::destroy:
                            return UIAlertActionStyleDestructive;
                    }

                    return UIAlertActionStyleDefault;
                });

                auto* action = [UIAlertAction actionWithTitle: juceStringToNS (text)
                                                        style: style
                                                      handler: ^(UIAlertAction*)
                {
                    const auto ind = buttonIndex;
                    const auto opt = options;
                    const auto r = recipient;
                    MessageManager::callAsync ([ind, opt, r]
                    {
                        const auto code = opt.getButtons()[(size_t) ind].getCode();
                        NullCheckedInvocation::invoke (r, code);
                    });
                }];

                [alert.get() addAction: action];

                if (button.isPreferredAction())
                    [alert.get() setPreferredAction: action];
            }

            if ([alert.get() preferredAction] == nullptr)
            {
                const auto positionSelector = buttonOrder.size() == 2
                                            ? @selector (lastObject)
                                            : @selector (firstObject);
                [alert.get() setPreferredAction: [[alert.get() actions] performSelector: positionSelector]];
            }

            [peerToUse->controller presentViewController: alert.get()
                                                animated: YES
                                              completion: nil];
        }

        std::optional<int> runSync() override
        {
            bool gotResult = false;
            std::optional<int> result;

            JUCE_AUTORELEASEPOOL
            {
                runAsync ([&gotResult, &result] (std::optional<int> r)
                {
                    gotResult = true;
                    result = r;
                });

                while (! gotResult)
                {
                    JUCE_AUTORELEASEPOOL
                    {
                        [[NSRunLoop mainRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
                    }
                }
            }

            return result;
        }

        void close() override
        {
            if (auto* alertViewController = alert.get())
                [alertViewController dismissViewControllerAnimated: YES completion: nil];
        }

    private:
        const MessageBoxOptions options;
        NSUniquePtr<UIAlertController> alert;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageBox)
    };

    return std::make_unique<MessageBox> (options);
}

} // namespace juce::detail
