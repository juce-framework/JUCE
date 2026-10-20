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
    class WindowsTaskDialog : public ScopedMessageBoxInterface
    {
    public:
        explicit WindowsTaskDialog (const MessageBoxOptions& opts)
            : associatedComponent (opts.getAssociatedComponent()), options (opts) {}

        void runAsync (std::function<void (std::optional<int>)> recipient) override
        {
            future = std::async (std::launch::async, [showMessageBox = getShowMessageBox(), recipient]
            {
                const auto messageResult = std::invoke ([&]() -> std::optional<int>
                {
                    const auto initComResult = CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

                    if (initComResult != S_OK)
                        return {};

                    const ScopeGuard scope { [] { CoUninitialize(); } };

                    return showMessageBox != nullptr ? showMessageBox() : std::nullopt;
                });

                NullCheckedInvocation::invoke (recipient, messageResult);
            });
        }

        std::optional<int> runSync() override
        {
            if (auto showMessageBox = getShowMessageBox())
                return showMessageBox();

            return {};
        }

        void close() override
        {
            auto* toClose = std::invoke ([&]
            {
                const std::scoped_lock lock { mutex };
                closed = true;
                return std::exchange (windowHandle, nullptr);
            });

            if (toClose != nullptr)
                EndDialog (toClose, 0);
        }

        void setDialogWindowHandle (HWND dialogHandle)
        {
            const std::scoped_lock lock { mutex };
            windowHandle = dialogHandle;
        }

    private:
        std::function<std::optional<int>()> getShowMessageBox()
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
        std::function<std::optional<int>()> getShowMessageBoxForParent (const HWND parent)
        {
            JUCE_ASSERT_MESSAGE_THREAD

            return [this, parent]() -> std::optional<int>
            {
                const auto title = options.getTitle();
                const auto message = options.getMessage();

                static constexpr auto buttonIdOffset = 0x100;

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

                const auto& buttons = options.getButtons();
                const auto cancelIter = buttons.size() == 1
                                      ? buttons.begin()
                                      : std::find_if (buttons.begin(),
                                                      buttons.end(),
                                                      [] (const auto& b) { return b.getRole() == MessageBoxButtonRole::cancel; });

                if (cancelIter != buttons.end() || buttons.empty())
                    config.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;

                config.nDefaultButton = (int) std::invoke ([&]
                {
                    const auto iter = std::find_if (buttons.begin(), buttons.end(), [] (const auto& b) { return b.isPreferredAction(); });
                    return iter != buttons.end() ? buttonIdOffset + std::distance (buttons.begin(), iter) : 0;
                });

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
                    config.pszMainIcon = std::invoke ([&]() -> LPWSTR
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
                    });
                }

                std::vector<String> buttonStrings;
                std::vector<TASKDIALOG_BUTTON> buttonLabels;
                Array<int> resultCodes;

                for (const auto& buttonIndex : AlertWindowHelpers::getSuggestedButtonOrder (options))
                {
                    const auto& button = options.getButtons()[(size_t) buttonIndex];
                    if (const auto buttonText = button.getText(); buttonText.isNotEmpty())
                    {
                        buttonLabels.push_back ({ buttonIndex + buttonIdOffset,
                                                  buttonStrings.emplace_back (buttonText).toWideCharPointer() });
                        resultCodes.add (button.getCode());
                    }
                }

                config.pButtons = buttonLabels.data();
                config.cButtons = (UINT) buttonLabels.size();

                int buttonId = 0;
                TaskDialogIndirect (&config, &buttonId, nullptr, nullptr);

                const std::scoped_lock lock { mutex };

                if (closed)
                    return 0;

                if (buttonId == IDCANCEL)
                    return cancelIter != buttons.end() ? cancelIter->getCode() : 0;

                const auto buttonIndex = buttonId - buttonIdOffset;

                return isPositiveAndBelow (buttonIndex, buttons.size()) ? buttons[(size_t) buttonIndex].getCode() : 0;
            };
        }

        Component::SafePointer<Component> associatedComponent;
        std::future<void> future;
        MessageBoxOptions options;

        HWND windowHandle = nullptr;
        bool closed = false;
        std::mutex mutex;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTaskDialog)
    };

    return std::make_unique<WindowsTaskDialog> (options);
}

} // namespace juce::detail
