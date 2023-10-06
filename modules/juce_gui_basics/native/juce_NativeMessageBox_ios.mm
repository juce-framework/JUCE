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
    class MessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& opts) : options (opts) {}

        void runAsync (std::function<void (int)> recipient) override
        {
            if (iOSGlobals::currentlyFocusedPeer == nullptr)
            {
                // Since iOS8, alert windows need to be associated with a window, so you need to
                // have at least one window on screen when you use this
                jassertfalse;
                return;
            }

            alert.reset ([[UIAlertController alertControllerWithTitle: juceStringToNS (options.getTitle())
                                                              message: juceStringToNS (options.getMessage())
                                                       preferredStyle: UIAlertControllerStyleAlert] retain]);

            for (auto i = 0; i < options.getNumButtons(); ++i)
            {
                const auto text = options.getButtonText (i);

                if (text.isEmpty())
                    continue;

                auto* action = [UIAlertAction actionWithTitle: juceStringToNS (text)
                                                        style: UIAlertActionStyleDefault
                                                      handler: ^(UIAlertAction*)
                {
                    MessageManager::callAsync ([recipient, i] { NullCheckedInvocation::invoke (recipient, i); });
                }];

                [alert.get() addAction: action];

                if (i == 0)
                    [alert.get() setPreferredAction: action];
            }

            [iOSGlobals::currentlyFocusedPeer->controller presentViewController: alert.get()
                                                                       animated: YES
                                                                     completion: nil];
        }

        int runSync() override
        {
            int result = -1;

            JUCE_AUTORELEASEPOOL
            {
                runAsync ([&result] (int r) { result = r; });

                while (result < 0)
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
