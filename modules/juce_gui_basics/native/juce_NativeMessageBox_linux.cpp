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
    // On Linux, we re-use the AlertWindow rather than using a platform-specific dialog.
    // For consistency with the NativeMessageBox on other platforms, the result code must
    // match the button index, hence this adapter.
    class MessageBox : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& options)
            : inner (detail::AlertWindowHelpers::create (options)),
              numButtons (options.getNumButtons()) {}

        void runAsync (std::function<void (int)> fn) override
        {
            inner->runAsync ([fn, n = numButtons] (int result)
                             {
                                 fn (map (result, n));
                             });
        }

        int runSync() override
        {
            return map (inner->runSync(), numButtons);
        }

        void close() override
        {
            inner->close();
        }

    private:
        static int map (int button, int numButtons) { return (button + numButtons - 1) % numButtons; }

        std::unique_ptr<ScopedMessageBoxInterface> inner;
        int numButtons = 0;
    };

    return std::make_unique<MessageBox> (options);
}

} // namespace juce::detail
