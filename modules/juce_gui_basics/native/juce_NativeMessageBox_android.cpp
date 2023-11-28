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
    class AndroidMessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit AndroidMessageBox (const MessageBoxOptions& o) : opts (o) {}

        void runAsync (std::function<void (int)> recipient) override
        {
            const auto makeDialogListener = [&recipient] (int result)
            {
                return new DialogListener ([recipient, result] { recipient (result); });
            };

            auto* env = getEnv();

            LocalRef<jobject> builder (env->NewObject (AndroidAlertDialogBuilder, AndroidAlertDialogBuilder.construct, getMainActivity().get()));

            const auto setText = [&] (auto method, const String& text)
            {
                builder = LocalRef<jobject> (env->CallObjectMethod (builder, method, javaString (text).get()));
            };

            setText (AndroidAlertDialogBuilder.setTitle,   opts.getTitle());
            setText (AndroidAlertDialogBuilder.setMessage, opts.getMessage());
            builder = LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.setCancelable, true));

            builder = LocalRef<jobject> (env->CallObjectMethod (builder, AndroidAlertDialogBuilder.setOnCancelListener,
                                                                CreateJavaInterface (makeDialogListener (0),
                                                                                     "android/content/DialogInterface$OnCancelListener").get()));

            const auto addButton = [&] (auto method, int index)
            {
                builder = LocalRef<jobject> (env->CallObjectMethod (builder,
                                                                    method,
                                                                    javaString (opts.getButtonText (index)).get(),
                                                                    CreateJavaInterface (makeDialogListener (index),
                                                                                         "android/content/DialogInterface$OnClickListener").get()));
            };

            addButton (AndroidAlertDialogBuilder.setPositiveButton, 0);

            if (opts.getButtonText (1).isNotEmpty())
                addButton (AndroidAlertDialogBuilder.setNegativeButton, 1);

            if (opts.getButtonText (2).isNotEmpty())
                addButton (AndroidAlertDialogBuilder.setNeutralButton, 2);

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

        int runSync() override
        {
            // Not implemented on this platform.
            jassertfalse;
            return 0;
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
