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
    class WindowsTaskDialog : public ScopedMessageBoxInterface
    {
    public:
        explicit WindowsTaskDialog (const MessageBoxOptions& opts)
            : associatedComponent (opts.getAssociatedComponent()), options (opts) {}

        void runAsync (std::function<void (int)> recipient) override
        {
            future = std::async (std::launch::async, [showMessageBox = getShowMessageBox(), recipient]
            {
                const auto initComResult = CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

                if (initComResult != S_OK)
                    return;

                const ScopeGuard scope { [] { CoUninitialize(); } };

                const auto messageResult = showMessageBox != nullptr ? showMessageBox() : 0;
                NullCheckedInvocation::invoke (recipient, messageResult);
            });
        }

        int runSync() override
        {
            if (auto showMessageBox = getShowMessageBox())
                return showMessageBox();

            return 0;
        }

        void close() override
        {
            if (auto* toClose = windowHandle.exchange (nullptr))
                EndDialog (toClose, 0);
        }

        void setDialogWindowHandle (HWND dialogHandle)
        {
            windowHandle = dialogHandle;
        }

    private:
        std::function<int()> getShowMessageBox()
        {
            const auto parent = associatedComponent != nullptr ? (HWND) associatedComponent->getWindowHandle() : nullptr;
            return getShowMessageBoxForParent (parent);
        }

        /*  Returns a function that should display a message box and return the result.

            getShowMessageBoxForParent() will be called on the message thread.

            The returned function will be called on a separate thread, in order to avoid blocking the
            message thread.

            'this' is guaranteed to be alive when the returned function is called.
        */
        std::function<int()> getShowMessageBoxForParent (const HWND parent)
        {
            JUCE_ASSERT_MESSAGE_THREAD

            return [this, parent]
            {
                const auto title = options.getTitle();
                const auto message = options.getMessage();

                TASKDIALOGCONFIG config{};

                config.cbSize         = sizeof (config);
                config.hwndParent     = parent;
                config.pszWindowTitle = title.toWideCharPointer();
                config.pszContent     = message.toWideCharPointer();
                config.hInstance      = (HINSTANCE) Process::getCurrentModuleInstanceHandle();
                config.lpCallbackData = reinterpret_cast<LONG_PTR> (this);
                config.pfCallback     = [] (HWND hwnd, UINT msg, WPARAM, LPARAM, LONG_PTR lpRefData)
                {
                    if (auto* t = reinterpret_cast<WindowsTaskDialog*> (lpRefData))
                    {
                        switch (msg)
                        {
                            case TDN_CREATED:
                            case TDN_DIALOG_CONSTRUCTED:
                                t->setDialogWindowHandle (hwnd);
                                break;

                            case TDN_DESTROYED:
                                t->setDialogWindowHandle (nullptr);
                                break;
                        }
                    }

                    return S_OK;
                };

                if (options.getIconType() == MessageBoxIconType::QuestionIcon)
                {
                    if (auto* questionIcon = LoadIcon (nullptr, IDI_QUESTION))
                    {
                        config.hMainIcon = questionIcon;
                        config.dwFlags |= TDF_USE_HICON_MAIN;
                    }
                }
                else
                {
                    config.pszMainIcon = [&]() -> LPWSTR
                    {
                        switch (options.getIconType())
                        {
                            case MessageBoxIconType::WarningIcon:   return TD_WARNING_ICON;
                            case MessageBoxIconType::InfoIcon:      return TD_INFORMATION_ICON;

                            case MessageBoxIconType::QuestionIcon:  JUCE_FALLTHROUGH
                            case MessageBoxIconType::NoIcon:
                                break;
                        }

                        return nullptr;
                    }();
                }

                std::vector<String> buttonStrings;
                std::vector<TASKDIALOG_BUTTON> buttonLabels;

                for (auto i = 0; i < options.getNumButtons(); ++i)
                    if (const auto buttonText = options.getButtonText (i); buttonText.isNotEmpty())
                        buttonLabels.push_back ({ (int) buttonLabels.size(), buttonStrings.emplace_back (buttonText).toWideCharPointer() });

                config.pButtons = buttonLabels.data();
                config.cButtons = (UINT) buttonLabels.size();

                int buttonIndex = 0;
                TaskDialogIndirect (&config, &buttonIndex, nullptr, nullptr);

                return buttonIndex;
            };
        }

        Component::SafePointer<Component> associatedComponent;
        std::atomic<HWND> windowHandle { nullptr };
        std::future<void> future;
        MessageBoxOptions options;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTaskDialog)
    };

    return std::make_unique<WindowsTaskDialog> (options);
}

} // namespace juce::detail
