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

namespace juce
{

enum class ResultCodeMappingMode
{
    plainIndex,     // The result code is equal to the index of the selected button.
                    // This is used for NativeMessageBox::show, showAsync, and showMessageBox.
    alertWindow,    // The result code is mapped in the same way as AlertWindow, i.e. if there
                    // are N buttons then button X will return ((X + 1) % N).
};

static std::unique_ptr<detail::ScopedMessageBoxInterface> makeNativeMessageBoxWithMappedResult (const MessageBoxOptions& opts,
                                                                                                ResultCodeMappingMode mode)
{
    class Adapter final : public detail::ScopedMessageBoxInterface
    {
    public:
        explicit Adapter (const MessageBoxOptions& options)
            : inner (detail::ScopedMessageBoxInterface::create (options)),
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
        static int map (int button, int numButtons) { return (button + 1) % numButtons; }

        std::unique_ptr<detail::ScopedMessageBoxInterface> inner;
        int numButtons = 0;
    };

    return mode == ResultCodeMappingMode::plainIndex ? detail::ScopedMessageBoxInterface::create (opts)
                                                     : std::make_unique<Adapter> (opts);
}

static int showNativeBoxUnmanaged (const MessageBoxOptions& opts,
                                   ModalComponentManager::Callback* cb,
                                   ResultCodeMappingMode mode)
{
    auto implementation = makeNativeMessageBoxWithMappedResult (opts, mode);
    return detail::ConcreteScopedMessageBoxImpl::showUnmanaged (std::move (implementation), cb);
}

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (MessageBoxIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    showNativeBoxUnmanaged (MessageBoxOptions().withIconType (iconType)
                                               .withTitle (title)
                                               .withMessage (message)
                                               .withButton (TRANS ("OK"))
                                               .withAssociatedComponent (associatedComponent),
                            nullptr,
                            ResultCodeMappingMode::plainIndex);
}

int JUCE_CALLTYPE NativeMessageBox::show (const MessageBoxOptions& options)
{
    return showNativeBoxUnmanaged (options, nullptr, ResultCodeMappingMode::plainIndex);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (MessageBoxIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOk (iconType, title, message, {}, associatedComponent);
    showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (MessageBoxIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsOkCancel (iconType, title, message, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow) != 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (MessageBoxIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsYesNoCancel (iconType, title, message, {}, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (MessageBoxIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions::makeOptionsYesNo (iconType, title, message, {}, {}, associatedComponent);
    return showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::alertWindow);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                ModalComponentManager::Callback* callback)
{
    showNativeBoxUnmanaged (options, callback, ResultCodeMappingMode::plainIndex);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                std::function<void (int)> callback)
{
    showAsync (options, ModalCallbackFunction::create (callback));
}

ScopedMessageBox NativeMessageBox::showScopedAsync (const MessageBoxOptions& options, std::function<void (int)> callback)
{
    auto implementation = makeNativeMessageBoxWithMappedResult (options, ResultCodeMappingMode::alertWindow);
    return detail::ConcreteScopedMessageBoxImpl::show (std::move (implementation), std::move (callback));
}

} // namespace juce
