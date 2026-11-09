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

namespace juce
{

static int showNativeBoxUnmanaged (const MessageBoxOptions& opts,
                                   ModalComponentManager::Callback* cb)
{
    // If this is hit, your alert window has duplicate result codes!
    jassert (opts.areAllResultCodesUnique());

    auto implementation = detail::ScopedMessageBoxInterface::create (opts);
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
                                               .withButton (TRANS ("OK"), 0)
                                               .withAssociatedComponent (associatedComponent),
                            nullptr);
}

int JUCE_CALLTYPE NativeMessageBox::showForOptions (const MessageBoxOptions& options)
{
    return showNativeBoxUnmanaged (options, nullptr);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (MessageBoxIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions{}.withIconType (iconType)
                                      .withTitle (title)
                                      .withMessage (message)
                                      .withButton (TRANS ("OK"), 0)
                                      .withAssociatedComponent (associatedComponent);
    showNativeBoxUnmanaged (options, callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (MessageBoxIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions{}.withIconType (iconType)
                                      .withTitle (title)
                                      .withMessage (message)
                                      .withButton (TRANS ("OK"), 1)
                                      .withButton (TRANS ("Cancel"), 0)
                                      .withAssociatedComponent (associatedComponent);
    return showNativeBoxUnmanaged (options, callback) != 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (MessageBoxIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions{}.withIconType (iconType)
                                      .withTitle (title)
                                      .withMessage (message)
                                      .withButton (TRANS ("Yes"), 1)
                                      .withButton (TRANS ("No"), 2)
                                      .withButton (TRANS ("Cancel"), 0)
                                      .withAssociatedComponent (associatedComponent);
    return showNativeBoxUnmanaged (options, callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (MessageBoxIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    auto options = MessageBoxOptions{}.withIconType (iconType)
                                      .withTitle (title)
                                      .withMessage (message)
                                      .withButton (TRANS ("Yes"), 1)
                                      .withButton (TRANS ("No"), 0)
                                      .withAssociatedComponent (associatedComponent);
    return showNativeBoxUnmanaged (options, callback);
}

void JUCE_CALLTYPE NativeMessageBox::showForOptionsAsync (const MessageBoxOptions& options,
                                                          ModalComponentManager::Callback* callback)
{
    showNativeBoxUnmanaged (options, callback);
}

void JUCE_CALLTYPE NativeMessageBox::showForOptionsAsync (const MessageBoxOptions& options,
                                                          std::function<void (int)> callback)
{
    showForOptionsAsync (options, ModalCallbackFunction::create (callback));
}

ScopedMessageBox NativeMessageBox::showScopedAsync (const MessageBoxOptions& options, std::function<void (int)> callback)
{
    // If this is hit, your alert window has duplicate result codes!
    jassert (options.areAllResultCodesUnique());

    auto implementation = detail::ScopedMessageBoxInterface::create (options);
    return detail::ConcreteScopedMessageBoxImpl::show (std::move (implementation), std::move (callback));
}

} // namespace juce
