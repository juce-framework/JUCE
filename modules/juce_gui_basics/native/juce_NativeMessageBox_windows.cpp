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

#if JUCE_MSVC
 // required to enable the newer dialog box on vista and above
 #pragma comment(linker,                             \
         "\"/MANIFESTDEPENDENCY:type='Win32' "       \
         "name='Microsoft.Windows.Common-Controls' " \
         "version='6.0.0.0' "                        \
         "processorArchitecture='*' "                \
         "publicKeyToken='6595b64144ccf1df' "        \
         "language='*'\""                            \
     )
#endif

std::unique_ptr<ScopedMessageBoxInterface> ScopedMessageBoxInterface::create (const MessageBoxOptions& options)
{
    class WindowsMessageBoxBase  : public ScopedMessageBoxInterface
    {
    public:
        explicit WindowsMessageBoxBase (Component* comp)
            : associatedComponent (comp) {}

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
        virtual std::function<int()> getShowMessageBoxForParent (HWND parent) = 0;

        Component::SafePointer<Component> associatedComponent;
        std::atomic<HWND> windowHandle { nullptr };
        std::future<void> future;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsMessageBoxBase)
    };

    class PreVistaMessageBox  : public WindowsMessageBoxBase
    {
    public:
        PreVistaMessageBox (const MessageBoxOptions& opts, UINT extraFlags)
            : WindowsMessageBoxBase (opts.getAssociatedComponent()),
              flags (extraFlags | getMessageBoxFlags (opts.getIconType())),
              title (opts.getTitle()), message (opts.getMessage()) {}

    private:
        std::function<int()> getShowMessageBoxForParent (const HWND parent) override
        {
            JUCE_ASSERT_MESSAGE_THREAD

            static std::map<DWORD, PreVistaMessageBox*> map;
            static std::mutex mapMutex;

            return [this, parent]
            {
                const auto threadId = GetCurrentThreadId();

                {
                    const std::scoped_lock scope { mapMutex };
                    map.emplace (threadId, this);
                }

                const ScopeGuard eraseFromMap { [threadId]
                {
                    const std::scoped_lock scope { mapMutex };
                    map.erase (threadId);
                } };

                const auto hookCallback = [] (int nCode, const WPARAM wParam, const LPARAM lParam)
                {
                    auto* params = reinterpret_cast<CWPSTRUCT*> (lParam);

                    if (nCode >= 0
                        && params != nullptr
                        && (params->message == WM_INITDIALOG || params->message == WM_DESTROY))
                    {
                        const auto callbackThreadId = GetCurrentThreadId();

                        const std::scoped_lock scope { mapMutex };

                        if (const auto iter = map.find (callbackThreadId); iter != map.cend())
                            iter->second->setDialogWindowHandle (params->message == WM_INITDIALOG ? params->hwnd : nullptr);
                    }

                    return CallNextHookEx ({}, nCode, wParam, lParam);
                };

                const auto hook = SetWindowsHookEx (WH_CALLWNDPROC,
                                                    hookCallback,
                                                    (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                                    threadId);
                const ScopeGuard removeHook { [hook] { UnhookWindowsHookEx (hook); } };

                const auto result = MessageBox (parent, message.toWideCharPointer(), title.toWideCharPointer(), flags);

                if (result == IDYES || result == IDOK)     return 0;
                if (result == IDNO && ((flags & 1) != 0))  return 1;

                return 2;
            };
        }

        static UINT getMessageBoxFlags (MessageBoxIconType iconType) noexcept
        {
            // this window can get lost behind JUCE windows which are set to be alwaysOnTop
            // so if there are any set it to be topmost
            const auto topmostFlag = WindowUtils::areThereAnyAlwaysOnTopWindows() ? MB_TOPMOST : 0;

            const auto iconFlags = [&]() -> decltype (topmostFlag)
            {
                switch (iconType)
                {
                    case MessageBoxIconType::QuestionIcon:  return MB_ICONQUESTION;
                    case MessageBoxIconType::WarningIcon:   return MB_ICONWARNING;
                    case MessageBoxIconType::InfoIcon:      return MB_ICONINFORMATION;
                    case MessageBoxIconType::NoIcon:        break;
                }

                return 0;
            }();

            return static_cast<UINT> (MB_TASKMODAL | MB_SETFOREGROUND | topmostFlag | iconFlags);
        }

        const UINT flags;
        const String title, message;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreVistaMessageBox)
    };

    class WindowsTaskDialog  : public WindowsMessageBoxBase
    {
        static auto getTaskDialogFunc()
        {
            using TaskDialogIndirectFunc = HRESULT (WINAPI*) (const TASKDIALOGCONFIG*, INT*, INT*, BOOL*);

            static const auto result = [&]() -> TaskDialogIndirectFunc
            {
                if (SystemStats::getOperatingSystemType() < SystemStats::WinVista)
                    return nullptr;

                const auto comctl = "Comctl32.dll";
                LoadLibraryA (comctl);
                const auto comctlModule = GetModuleHandleA (comctl);

                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type")
                if (comctlModule != nullptr)
                    return (TaskDialogIndirectFunc) GetProcAddress (comctlModule, "TaskDialogIndirect");
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE

                return nullptr;
            }();

            return result;
        }

    public:
        explicit WindowsTaskDialog (const MessageBoxOptions& opts)
            : WindowsMessageBoxBase (opts.getAssociatedComponent()),
              iconType (opts.getIconType()),
              title (opts.getTitle()), message (opts.getMessage()),
              buttons { opts.getButtonText (0), opts.getButtonText (1), opts.getButtonText (2) } {}

        static bool isAvailable()
        {
            return getTaskDialogFunc() != nullptr;
        }

    private:
        std::function<int()> getShowMessageBoxForParent (const HWND parent) override
        {
            JUCE_ASSERT_MESSAGE_THREAD

            return [this, parent]
            {
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

                if (iconType == MessageBoxIconType::QuestionIcon)
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
                        switch (iconType)
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

                std::vector<TASKDIALOG_BUTTON> buttonLabels;

                for (const auto& buttonText : buttons)
                    if (buttonText.isNotEmpty())
                        buttonLabels.push_back ({ (int) buttonLabels.size(), buttonText.toWideCharPointer() });

                config.pButtons = buttonLabels.data();
                config.cButtons = (UINT) buttonLabels.size();

                int buttonIndex = 0;

                if (auto* func = getTaskDialogFunc())
                    func (&config, &buttonIndex, nullptr, nullptr);
                else
                    jassertfalse;

                return buttonIndex;
            };
        }

        const MessageBoxIconType iconType;
        const String title, message;
        const std::array<String, 3> buttons;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsTaskDialog)
    };

    if (WindowsTaskDialog::isAvailable())
        return std::make_unique<WindowsTaskDialog> (options);

    const auto extraFlags = [&options]
    {
        const auto numButtons = options.getNumButtons();

        if (numButtons == 3)
            return MB_YESNOCANCEL;

        if (numButtons == 2)
            return options.getButtonText (0) == "OK" ? MB_OKCANCEL
                                                     : MB_YESNO;

        return MB_OK;
    }();

    return std::make_unique<PreVistaMessageBox> (options, (UINT) extraFlags);
}

} // namespace juce::detail
