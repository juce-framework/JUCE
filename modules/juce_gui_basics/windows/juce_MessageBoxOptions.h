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

/** The type of icon to show in the dialog box. */
enum class MessageBoxIconType
{
    NoIcon,         /**< No icon will be shown on the dialog box. */
    QuestionIcon,   /**< A question-mark icon, for dialog boxes that need the
                         user to answer a question. */
    WarningIcon,    /**< An exclamation mark to indicate that the dialog is a
                         warning about something and shouldn't be ignored. */
    InfoIcon        /**< An icon that indicates that the dialog box is just
                         giving the user some information, which doesn't require
                         a response from them. */
};

/** Roles that may be applied to buttons in a dialog box. */
enum class MessageBoxButtonRole
{
    normal,                 /**< A style for standard actions. */
    destroy,                /**< An action that causes destructive changes. */
    cancel,                 /**< An action that closes the dialog without making any changes. */
};

//==============================================================================
/** Class that holds configuration information about the action buttons displayed in a message box.

    @tags{GUI}
*/
class JUCE_API  MessageBoxButtonOptions
{
public:
    /** Sets the text and result code for this action.

        Result codes should be unique for each action within the same message box.
    */
    MessageBoxButtonOptions (String textIn, int codeIn)
        : text (std::move (textIn)), code (codeIn)
    {
    }

    /** Sets the role of this button.

        If unspecified, the button will have the 'normal' role.
        Within a given message box, a maximum of one button may have the 'destroy' role,
        and a maximum of one button may have the 'cancel' role.

        The button with the 'cancel' role, if present, will be positioned as follows in a native
        message box:
        - macOS: bottom of the stack of buttons, or leftmost, depending on layout
        - Android: leftmost
        - Windows: rightmost
        - iOS: bottom of the stack of buttons, or leftmost, depending on layout

        You will get the best result across all platforms by setting both the preferred
        action and the cancel role.
    */
    MessageBoxButtonOptions& setRole (MessageBoxButtonRole roleIn)
    {
        role = roleIn;
        return *this;
    }

    /** Like setRole(), but returns a copy of this object with the new state applied. */
    [[nodiscard]] MessageBoxButtonOptions withRole (MessageBoxButtonRole roleIn) const
    {
        return MessageBoxButtonOptions { *this }.setRole (roleIn);
    }

    /** Sets whether this button is the default/primary action of the dialog, usually
        highlighted and selected with the 'return' key on the keyboard.

        If no action is specified as preferred, then the first button added to the dialog
        without the 'cancel' role will be treated as the preferred action.

        The preferred action (either explicit or implicit) will be positioned as follows in a native
        message box:
        - macOS: top of the stack of buttons, or rightmost, depending on layout
        - Android: rightmost
        - Windows: leftmost
        - iOS: top of the stack of buttons (three buttons), or rightmost (two buttons). The cancel
               role takes precedence, so if the layout has two buttons and one button has the
               cancel role, the cancel button will be leftmost.

        You will get the best result across all platforms by setting both the preferred
        action and the cancel role.
    */
    MessageBoxButtonOptions& setPreferredAction (bool shouldBeDefault = true)
    {
        preferredAction = shouldBeDefault;
        return *this;
    }

    /** Like setPreferredAction(), but returns a copy of this object with the new state applied. */
    [[nodiscard]] MessageBoxButtonOptions withPreferredAction (bool shouldBeDefault = true) const
    {
        return MessageBoxButtonOptions { *this }.setPreferredAction (shouldBeDefault);
    }

    /** Returns the text corresponding to this action. */
    String getText() const
    {
        return text;
    }

    /** Returns the result code of this action. */
    int getCode() const
    {
        return code;
    }

    /** Returns the role of this action. */
    MessageBoxButtonRole getRole() const
    {
        return role;
    }

    /** Returns true if this is the preferred action, or false otherwise. */
    bool isPreferredAction() const
    {
        return preferredAction;
    }

private:
    String text;
    int code = 0;
    MessageBoxButtonRole role = MessageBoxButtonRole::normal;
    bool preferredAction = false;
};

//==============================================================================
/** Class used to create a set of options to pass to the AlertWindow and NativeMessageBox
    methods for showing dialog boxes.

    You can chain together a series of calls to this class's methods to create
    a set of whatever options you want to specify.

    E.g. @code
    AlertWindow::showAsync (MessageBoxOptions()
                              .withIconType (MessageBoxIconType::InfoIcon)
                              .withTitle ("A Title")
                              .withMessage ("A message.")
                              .withButton ("OK", 1)
                              .withButton ("Cancel", 0)
                              .withAssociatedComponent (myComp),
                            myCallback);
    @endcode

    @tags{GUI}
*/
class JUCE_API  MessageBoxOptions
{
public:
    MessageBoxOptions() = default;
    MessageBoxOptions (const MessageBoxOptions&) = default;
    MessageBoxOptions& operator= (const MessageBoxOptions&) = default;

    //==============================================================================
    /** Sets the type of icon that should be used for the dialog box. */
    [[nodiscard]] MessageBoxOptions withIconType (MessageBoxIconType type) const          { return withMember (*this, &MessageBoxOptions::iconType, type); }

    /** Sets the title of the dialog box. */
    [[nodiscard]] MessageBoxOptions withTitle (const String& boxTitle) const              { return withMember (*this, &MessageBoxOptions::title, boxTitle); }

    /** Sets the message that should be displayed in the dialog box. */
    [[nodiscard]] MessageBoxOptions withMessage (const String& boxMessage) const          { return withMember (*this, &MessageBoxOptions::message, boxMessage); }

    /** This will add a button to the dialog box with the specified text and result code.

        Generally up to 3 buttons are supported for dialog boxes, so adding any more
        than this may have no effect.

        Message-boxes with zero or one buttons, or that hold a button with the 'cancel' role, may
        be cancelled. For example, on Windows, a native message box can be dismissed by pressing
        alt+f4 or escape, and on Android, a native message box can be dismissed by tapping outside
        the box or pressing the back button.

        On macOS, a preferred "OK" button will be added automatically to native dialogs with no
        buttons. Such dialogs can be dismissed with the return key, but not with the
        escape key.

        Cancelling a zero-button message box will return "0".
        Cancelling a single-button message box will return the result code of that button.
        Cancelling a message box with a different number of buttons will return the code of
        whichever button has the 'cancel' role.
    */
    [[nodiscard]] MessageBoxOptions withButton (const String& text, int resultCode) const
    {
        auto copy = *this;
        copy.buttons.push_back ({ text, resultCode });
        return copy;
    }

    /** This will add a button to the dialog box with the specified properties.

        Generally up to 3 buttons are supported for dialog boxes, so adding any more
        than this may have no effect.

        Message-boxes with zero or one buttons, or that hold a button with the 'cancel' role, may
        be cancelled. For example, on Windows, a native message box can be dismissed by pressing
        alt+f4 or escape, and on Android, a native message box can be dismissed by tapping outside
        the box or pressing the back button.

        On macOS, a preferred "OK" button will be added automatically to native dialogs with no
        buttons. Such dialogs can be dismissed with the return key, but not with the
        escape key.

        Cancelling a zero-button message box will return "0".
        Cancelling a single-button message box will return the result code of that button.
        Cancelling a message box with a different number of buttons will return the code of
        whichever button has the 'cancel' role.
    */
    [[nodiscard]] MessageBoxOptions withButton (MessageBoxButtonOptions opt) const;

    /** The component that the dialog box should be associated with.

        An AlertWindow is kept above the window containing this component.
        See TopLevelWindow::setAssociatedComponent().
    */
    [[nodiscard]] MessageBoxOptions withAssociatedComponent (Component* component) const  { return withMember (*this, &MessageBoxOptions::associatedComponent, component); }

    /** The component that will contain the message box (e.g. the AudioProcessorEditor in a plugin).

        This will only affect JUCE AlertWindows. It won't affect the drawing of native message boxes.
        This is mainly intended for use in AU plugins, where opening additional windows can be problematic.
    */
    [[nodiscard]] MessageBoxOptions withParentComponent (Component* component) const      { return withMember (*this, &MessageBoxOptions::parentComponent, component); }

    //==============================================================================
    /** Returns the icon type of the dialog box.

        @see withIconType
    */
    MessageBoxIconType getIconType() const noexcept          { return iconType; }

    /** Returns the title of the dialog box.

        @see withTitle
    */
    String getTitle() const                                  { return title; }

    /** Returns the message of the dialog box.

        @see withMessage
    */
    String getMessage() const                                { return message; }

    /** Returns the number of buttons that have been added to the dialog box.

        @see withButton
    */
    int getNumButtons() const noexcept                       { return (int) buttons.size(); }

    /** Returns the text that has been set for one of the buttons of the dialog box.

        @see withButton, getNumButtons
    */
    [[deprecated ("Prefer to index into the result of getButtons()")]]
    String getButtonText (int buttonIndex) const
    {
        if (isPositiveAndBelow (buttonIndex, buttons.size()))
            return buttons[(size_t) buttonIndex].getText();

        // Button index does not refer to a known button!
        jassertfalse;
        return {};
    }

    /** Returns the result code associated with the button at the given index.

        @see withButton, getNumButtons
    */
    [[deprecated ("Prefer to index into the result of getButtons()")]]
    int getButtonResultCode (int buttonIndex) const
    {
        if (isPositiveAndBelow (buttonIndex, buttons.size()))
            return buttons[(size_t) buttonIndex].getCode();

        // Button index does not refer to a known button!
        jassertfalse;
        return {};
    }

    /** Returns the properties of all buttons in the message box. */
    Span<const MessageBoxButtonOptions> getButtons() const&
    {
        return buttons;
    }

    /** Deleted to avoid dangling references. */
    Span<const MessageBoxButtonOptions> getButtons() const&& = delete;

    /** Returns true if all buttons have a unique result code, or false otherwise. */
    bool areAllResultCodesUnique() const
    {
        std::set<int> codes;

        for (const auto& opt : buttons)
            codes.insert (opt.getCode());

        return (int) codes.size() == getNumButtons();
    }

    /** Returns the component that the dialog box is associated with.

        @see withAssociatedComponent
    */
    Component* getAssociatedComponent() const noexcept       { return associatedComponent; }

    /** Returns the component that will be used as the parent of the dialog box.

        @see withParentComponent
    */
    Component* getParentComponent() const noexcept           { return parentComponent; }

    /** Creates options suitable for a message box with a single button.

        If no button text is supplied, "OK" will be used.
    */
    static MessageBoxOptions makeOptionsOk (MessageBoxIconType iconType,
                                            const String& title,
                                            const String& message,
                                            const String& buttonText = String(),
                                            Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with two buttons.

        If no button text is supplied, "OK" and "Cancel" will be used.
    */
    [[deprecated ("Prefer to use MessageBoxOptions{}.withButton()..., so that result codes for each button are explicit")]]
    static MessageBoxOptions makeOptionsOkCancel (MessageBoxIconType iconType,
                                                  const String& title,
                                                  const String& message,
                                                  const String& button1Text = String(),
                                                  const String& button2Text = String(),
                                                  Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with two buttons.

        If no button text is supplied, "Yes" and "No" will be used.
    */
    [[deprecated ("Prefer to use MessageBoxOptions{}.withButton()..., so that result codes for each button are explicit")]]
    static MessageBoxOptions makeOptionsYesNo (MessageBoxIconType iconType,
                                               const String& title,
                                               const String& message,
                                               const String& button1Text = String(),
                                               const String& button2Text = String(),
                                               Component* associatedComponent = nullptr);

    /** Creates options suitable for a message box with three buttons.
     *
        If no button text is supplied, "Yes", "No", and "Cancel" will be used.
    */
    [[deprecated ("Prefer to use MessageBoxOptions{}.withButton()..., so that result codes for each button are explicit")]]
    static MessageBoxOptions makeOptionsYesNoCancel (MessageBoxIconType iconType,
                                                     const String& title,
                                                     const String& message,
                                                     const String& button1Text = String(),
                                                     const String& button2Text = String(),
                                                     const String& button3Text = String(),
                                                     Component* associatedComponent = nullptr);

private:
    //==============================================================================
    MessageBoxIconType iconType = MessageBoxIconType::InfoIcon;
    String title, message;
    std::vector<MessageBoxButtonOptions> buttons;
    WeakReference<Component> associatedComponent;
    WeakReference<Component> parentComponent;
};

} // namespace juce
