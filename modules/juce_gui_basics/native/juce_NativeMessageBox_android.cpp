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
    class AndroidMessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit AndroidMessageBox (const MessageBoxOptions& o) : opts (o) {}

        void runAsync (std::function<void (std::optional<int>)> recipient) override
        {
            const auto makeDialogListener = [&recipient] (std::optional<int> result)
            {
                return new DialogListener ([recipient, result]
                {
                    recipient (result);
                });
            };

            auto* env = getEnv();

            LocalRef<jobject> builder (env->NewObject (AndroidAlertDialogBuilder, AndroidAlertDialogBuilder.construct, getMainActivity().get()));

            const auto setText = [&] (auto method, const String& text)
            {
                builder = LocalRef<jobject> (env->CallObjectMethod (builder, method, javaString (text).get()));
            };

            setText (AndroidAlertDialogBuilder.setTitle,   opts.getTitle());
            setText (AndroidAlertDialogBuilder.setMessage, opts.getMessage());

            const auto& buttons = opts.getButtons();
            const auto cancelIter = std::find_if (buttons.begin(),
                                                  buttons.end(),
                                                  [] (const auto& b) { return b.getRole() == MessageBoxButtonRole::cancel; });

            jboolean cancelable = cancelIter != buttons.end() || buttons.empty();
            builder = LocalRef<jobject> (env->CallObjectMethod (builder,
                                                                AndroidAlertDialogBuilder.setCancelable,
                                                                cancelable));

            builder = LocalRef<jobject> (env->CallObjectMethod (builder,
                                                                AndroidAlertDialogBuilder.setOnCancelListener,
                                                                CreateJavaInterface (makeDialogListener (cancelIter != buttons.end() ? std::make_optional (cancelIter->getCode()) : std::nullopt),
                                                                                     "android/content/DialogInterface$OnCancelListener").get()));

            const jmethodID buttonTextSetters[] { AndroidAlertDialogBuilder.setPositiveButton,
                                                  AndroidAlertDialogBuilder.setNegativeButton,
                                                  AndroidAlertDialogBuilder.setNeutralButton };

            const auto buttonOrder = AlertWindowHelpers::getSuggestedButtonOrder (opts);

            for (const auto [index, method] : enumerate (buttonTextSetters, size_t{}))
            {
                if (buttonOrder.size() <= index)
                    break;

                const auto buttonIndex = buttonOrder[index];
                const auto& button = buttons[(size_t) buttonIndex];

                if (button.getText().isEmpty())
                    continue;

                builder = LocalRef<jobject> (env->CallObjectMethod (builder,
                                                                    method,
                                                                    javaString (button.getText()).get(),
                                                                    CreateJavaInterface (makeDialogListener (button.getCode()),
                                                                                         "android/content/DialogInterface$OnClickListener").get()));
            }

            dialog = GlobalRef (LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.create)));

            LocalRef<jobject> window (env->CallObjectMethod (dialog, AndroidDialog.getWindow));

            if (Desktop::getInstance().getKioskModeComponent() != nullptr)
            {
                env->CallVoidMethod (window, AndroidWindow.setFlags, FLAG_NOT_FOCUSABLE, FLAG_NOT_FOCUSABLE);
                LocalRef<jobject> decorView (env->CallObjectMethod (window, AndroidWindow.getDecorView));
                env->CallVoidMethod (decorView, AndroidView.setSystemUiVisibility, fullScreenFlags);
            }

            env->CallVoidMethod (dialog, AndroidDialog.show);

            if (Desktop::getInstance().getKioskModeComponent() != nullptr)
                env->CallVoidMethod (window, AndroidWindow.clearFlags, FLAG_NOT_FOCUSABLE);
        }

        std::optional<int> runSync() override
        {
            // Not implemented on this platform.
            jassertfalse;
            return {};
        }

        void close() override
        {
            if (dialog != nullptr)
                getEnv()->CallVoidMethod (dialog, AndroidDialogInterface.dismiss);
        }

    private:
        const MessageBoxOptions opts;
        GlobalRef dialog;
    };

    return std::make_unique<AndroidMessageBox> (options);
}

} // namespace juce::detail
