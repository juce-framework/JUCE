/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_NATIVEMESSAGEBOX_H_INCLUDED
#define JUCE_NATIVEMESSAGEBOX_H_INCLUDED

//==============================================================================
/**
    This class contains some static methods for showing native alert windows.
*/
class NativeMessageBox
{
public:
    /** Shows a dialog box that just has a message and a single 'ok' button to close it.

        The box is shown modally, and the method will block until the user has clicked its
        button (or pressed the escape or return keys).

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the title
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
    */
   #if JUCE_MODAL_LOOPS_PERMITTED
    static void JUCE_CALLTYPE showMessageBox (AlertWindow::AlertIconType iconType,
                                              const String& title,
                                              const String& message,
                                              Component* associatedComponent = nullptr);
   #endif

    /** Shows a dialog box that just has a message and a single 'ok' button to close it.

        The box will be displayed and placed into a modal state, but this method will return
        immediately, and the callback will be invoked later when the user dismisses the box.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the title
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the callback will receive a call to its
                            modalStateFinished() when the box is dismissed. The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
    */
    static void JUCE_CALLTYPE showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                    const String& title,
                                                    const String& message,
                                                    Component* associatedComponent = nullptr,
                                                    ModalComponentManager::Callback* callback = nullptr);

    /** Shows a dialog box with two buttons.

        Ideal for ok/cancel or yes/no choices. The return key can also be used
        to trigger the first button, and the escape key for the second button.

        If the callback parameter is null, the box is shown modally, and the method will
        block until the user has clicked the button (or pressed the escape or return keys).
        If the callback parameter is non-null, the box will be displayed and placed into a
        modal state, but this method will return immediately, and the callback will be invoked
        later when the user dismisses the box.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the title
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the box will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the ok button was pressed, or 0 for cancel, The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
        @returns true if button 1 was clicked, false if it was button 2. If the callback parameter
                 is not null, the method always returns false, and the user's choice is delivered
                 later by the callback.
    */
    static bool JUCE_CALLTYPE showOkCancelBox (AlertWindow::AlertIconType iconType,
                                               const String& title,
                                               const String& message,
                                            #if JUCE_MODAL_LOOPS_PERMITTED
                                               Component* associatedComponent = nullptr,
                                               ModalComponentManager::Callback* callback = nullptr);
                                            #else
                                               Component* associatedComponent,
                                               ModalComponentManager::Callback* callback);
                                            #endif

    /** Shows a dialog box with three buttons.

        Ideal for yes/no/cancel boxes.

        The escape key can be used to trigger the third button.

        If the callback parameter is null, the box is shown modally, and the method will
        block until the user has clicked the button (or pressed the escape or return keys).
        If the callback parameter is non-null, the box will be displayed and placed into a
        modal state, but this method will return immediately, and the callback will be invoked
        later when the user dismisses the box.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the title
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the box will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the "yes" button was pressed, 2 for the "no" button, or 0
                            if it was cancelled, The callback object will be owned and deleted by the
                            system, so make sure that it works safely and doesn't keep any references
                            to objects that might be deleted before it gets called.

        @returns If the callback parameter has been set, this returns 0. Otherwise, it returns one
                 of the following values:
                 - 0 if 'cancel' was pressed
                 - 1 if 'yes' was pressed
                 - 2 if 'no' was pressed
    */
    static int JUCE_CALLTYPE showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                 const String& title,
                                                 const String& message,
                                               #if JUCE_MODAL_LOOPS_PERMITTED
                                                 Component* associatedComponent = nullptr,
                                                 ModalComponentManager::Callback* callback = nullptr);
                                               #else
                                                 Component* associatedComponent,
                                                 ModalComponentManager::Callback* callback);
                                               #endif

private:
    NativeMessageBox() JUCE_DELETED_FUNCTION;
    JUCE_DECLARE_NON_COPYABLE (NativeMessageBox)
};

#endif   // JUCE_NATIVEMESSAGEBOX_H_INCLUDED
