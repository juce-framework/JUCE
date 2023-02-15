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

namespace juce
{

template <ScopedMessageBox::Pimpl::MapFn mapFn = nullptr>
static int showNativeBoxUnmanaged (const MessageBoxOptions& opts, ModalComponentManager::Callback* cb)
{
    return ScopedMessageBox::Pimpl::showUnmanaged<mapFn> (ScopedMessageBoxInterface::create (opts), cb);
}

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (MessageBoxIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    showNativeBoxUnmanaged<ScopedMessageBox::Pimpl::messageBox> (MessageBoxOptions().withIconType (iconType)
                                                                                    .withTitle (title)
                                                                                    .withMessage (message)
                                                                                    .withButton (TRANS("OK"))
                                                                                    .withAssociatedComponent (associatedComponent),
                                                                 nullptr);
}

int JUCE_CALLTYPE NativeMessageBox::show (const MessageBoxOptions& options)
{
    return showNativeBoxUnmanaged<> (options, nullptr);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (MessageBoxIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    showNativeBoxUnmanaged<ScopedMessageBox::Pimpl::messageBox> (MessageBoxOptions().withIconType (iconType)
                                                                                    .withTitle (title)
                                                                                    .withMessage (message)
                                                                                    .withButton (TRANS("OK"))
                                                                                    .withAssociatedComponent (associatedComponent),
                                                                 callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (MessageBoxIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    return showNativeBoxUnmanaged<ScopedMessageBox::Pimpl::okCancel> (MessageBoxOptions().withIconType (iconType)
                                                                                         .withTitle (title)
                                                                                         .withMessage (message)
                                                                                         .withButton (TRANS("OK"))
                                                                                         .withButton (TRANS("Cancel"))
                                                                                         .withAssociatedComponent (associatedComponent),
                                                                      callback) != 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (MessageBoxIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    return showNativeBoxUnmanaged<ScopedMessageBox::Pimpl::yesNoCancel> (MessageBoxOptions().withIconType (iconType)
                                                                                            .withTitle (title)
                                                                                            .withMessage (message)
                                                                                            .withButton (TRANS("Yes"))
                                                                                            .withButton (TRANS("No"))
                                                                                            .withButton (TRANS("Cancel"))
                                                                                            .withAssociatedComponent (associatedComponent),
                                                                         callback);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (MessageBoxIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    return showNativeBoxUnmanaged<ScopedMessageBox::Pimpl::okCancel> (MessageBoxOptions().withIconType (iconType)
                                                                                         .withTitle (title)
                                                                                         .withMessage (message)
                                                                                         .withButton (TRANS("Yes"))
                                                                                         .withButton (TRANS("No"))
                                                                                         .withAssociatedComponent (associatedComponent),
                                                                      callback);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                ModalComponentManager::Callback* callback)
{
    showNativeBoxUnmanaged<> (options, callback);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                std::function<void (int)> callback)
{
    showAsync (options, ModalCallbackFunction::create (callback));
}

ScopedMessageBox NativeMessageBox::showScopedAsync (const MessageBoxOptions& options, std::function<void (int)> callback)
{
    return ScopedMessageBox::Pimpl::show (ScopedMessageBoxInterface::create (options), std::move (callback));
}

} // namespace juce
