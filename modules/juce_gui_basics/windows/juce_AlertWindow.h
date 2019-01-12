/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** A window that displays a message and has buttons for the user to react to it.

    For simple dialog boxes with just a couple of buttons on them, there are
    some static methods for running these.

    For more complex dialogs, an AlertWindow can be created, then it can have some
    buttons and components added to it, and its runModalLoop() method is then used to
    show it. The value returned by runModalLoop() shows which button the
    user pressed to dismiss the box.

    @see ThreadWithProgressWindow

    @tags{GUI}
*/
class JUCE_API  AlertWindow  : public TopLevelWindow
{
public:
    //==============================================================================
    /** The type of icon to show in the dialog box. */
    enum AlertIconType
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

    //==============================================================================
    /** Creates an AlertWindow.

        @param title    the headline to show at the top of the dialog box
        @param message  a longer, more descriptive message to show underneath the
                        headline
        @param iconType the type of icon to display
        @param associatedComponent   if this is non-null, it specifies the component that the
                        alert window should be associated with. Depending on the look
                        and feel, this might be used for positioning of the alert window.
    */
    AlertWindow (const String& title,
                 const String& message,
                 AlertIconType iconType,
                 Component* associatedComponent = nullptr);

    /** Destroys the AlertWindow */
    ~AlertWindow() override;

    //==============================================================================
    /** Returns the type of alert icon that was specified when the window
        was created. */
    AlertIconType getAlertType() const noexcept             { return alertIconType; }

    //==============================================================================
    /** Changes the dialog box's message.

        This will also resize the window to fit the new message if required.
    */
    void setMessage (const String& message);

    //==============================================================================
    /** Adds a button to the window.

        @param name         the text to show on the button
        @param returnValue  the value that should be returned from runModalLoop()
                            if this is the button that the user presses.
        @param shortcutKey1 an optional key that can be pressed to trigger this button
        @param shortcutKey2 a second optional key that can be pressed to trigger this button
    */
    void addButton (const String& name,
                    int returnValue,
                    const KeyPress& shortcutKey1 = KeyPress(),
                    const KeyPress& shortcutKey2 = KeyPress());

    /** Returns the number of buttons that the window currently has. */
    int getNumButtons() const;

    /** Invokes a click of one of the buttons. */
    void triggerButtonClick (const String& buttonName);

    /** If set to true and the window contains no buttons, then pressing the escape key will make
        the alert cancel its modal state.
        By default this setting is true - turn it off if you don't want the box to respond to
        the escape key. Note that it is ignored if you have any buttons, and in that case you
        should give the buttons appropriate keypresses to trigger cancelling if you want to.
    */
    void setEscapeKeyCancels (bool shouldEscapeKeyCancel);

    //==============================================================================
    /** Adds a textbox to the window for entering strings.

        @param name             an internal name for the text-box. This is the name to pass to
                                the getTextEditorContents() method to find out what the
                                user typed-in.
        @param initialContents  a string to show in the text box when it's first shown
        @param onScreenLabel    if this is non-empty, it will be displayed next to the
                                text-box to label it.
        @param isPasswordBox    if true, the text editor will display asterisks instead of
                                the actual text
        @see getTextEditorContents
    */
    void addTextEditor (const String& name,
                        const String& initialContents,
                        const String& onScreenLabel = String(),
                        bool isPasswordBox = false);

    /** Returns the contents of a named textbox.

        After showing an AlertWindow that contains a text editor, this can be
        used to find out what the user has typed into it.

        @param nameOfTextEditor     the name of the text box that you're interested in
        @see addTextEditor
    */
    String getTextEditorContents (const String& nameOfTextEditor) const;

    /** Returns a pointer to a textbox that was added with addTextEditor(). */
    TextEditor* getTextEditor (const String& nameOfTextEditor) const;

    //==============================================================================
    /** Adds a drop-down list of choices to the box.

        After the box has been shown, the getComboBoxComponent() method can
        be used to find out which item the user picked.

        @param name     the label to use for the drop-down list
        @param items    the list of items to show in it
        @param onScreenLabel    if this is non-empty, it will be displayed next to the
                                combo-box to label it.
        @see getComboBoxComponent
    */
    void addComboBox (const String& name,
                      const StringArray& items,
                      const String& onScreenLabel = String());

    /** Returns a drop-down list that was added to the AlertWindow.

        @param nameOfList   the name that was passed into the addComboBox() method
                            when creating the drop-down
        @returns the ComboBox component, or nullptr if none was found for the given name.
    */
    ComboBox* getComboBoxComponent (const String& nameOfList) const;

    //==============================================================================
    /** Adds a block of text.

        This is handy for adding a multi-line note next to a textbox or combo-box,
        to provide more details about what's going on.
    */
    void addTextBlock (const String& text);

    //==============================================================================
    /** Adds a progress-bar to the window.

        @param progressValue    a variable that will be repeatedly checked while the
                                dialog box is visible, to see how far the process has
                                got. The value should be in the range 0 to 1.0
    */
    void addProgressBarComponent (double& progressValue);

    //==============================================================================
    /** Adds a user-defined component to the dialog box.

        @param component    the component to add - its size should be set up correctly
                            before it is passed in. The caller is responsible for deleting
                            the component later on - the AlertWindow won't delete it.
    */
    void addCustomComponent (Component* component);

    /** Returns the number of custom components in the dialog box.
        @see getCustomComponent, addCustomComponent
    */
    int getNumCustomComponents() const;

    /** Returns one of the custom components in the dialog box.

        @param index    a value 0 to (getNumCustomComponents() - 1).
                        Out-of-range indexes will return nullptr
        @see getNumCustomComponents, addCustomComponent
    */
    Component* getCustomComponent (int index) const;

    /** Removes one of the custom components in the dialog box.
        Note that this won't delete it, it just removes the component from the window

        @param index    a value 0 to (getNumCustomComponents() - 1).
                        Out-of-range indexes will return nullptr
        @returns        the component that was removed (or null)
        @see getNumCustomComponents, addCustomComponent
    */
    Component* removeCustomComponent (int index);

    //==============================================================================
    /** Returns true if the window contains any components other than just buttons.*/
    bool containsAnyExtraComponents() const;

    //==============================================================================
    // easy-to-use message box functions:

   #if JUCE_MODAL_LOOPS_PERMITTED
    /** Shows a dialog box that just has a message and a single button to get rid of it.

        The box is shown modally, and the method will block until the user has clicked the
        button (or pressed the escape or return keys).

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param buttonText   the text to show in the button - if this string is empty, the
                            default string "OK" (or a localised version) will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
    */
    static void JUCE_CALLTYPE showMessageBox (AlertIconType iconType,
                                              const String& title,
                                              const String& message,
                                              const String& buttonText = String(),
                                              Component* associatedComponent = nullptr);
   #endif

    /** Shows a dialog box that just has a message and a single button to get rid of it.

        The box will be displayed and placed into a modal state, but this method will
        return immediately, and if a callback was supplied, it will be invoked later
        when the user dismisses the box.

        @param iconType     the type of icon to show
        @param title        the headline to show at the top of the box
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param buttonText   the text to show in the button - if this string is empty, the
                            default string "OK" (or a localised version) will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the callback will receive a call to its
                            modalStateFinished() when the box is dismissed. The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
    */
    static void JUCE_CALLTYPE showMessageBoxAsync (AlertIconType iconType,
                                                   const String& title,
                                                   const String& message,
                                                   const String& buttonText = String(),
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
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param button1Text  the text to show in the first button - if this string is
                            empty, the default string "OK" (or a localised version of it)
                            will be used.
        @param button2Text  the text to show in the second button - if this string is
                            empty, the default string "cancel" (or a localised version of it)
                            will be used.
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the menu will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the ok button was pressed, or 0 for cancel. The callback object
                            will be owned and deleted by the system, so make sure that it works
                            safely and doesn't keep any references to objects that might be deleted
                            before it gets called.
        @returns true if button 1 was clicked, false if it was button 2. If the callback parameter
                 is not null, the method always returns false, and the user's choice is delivered
                 later by the callback.
    */
    static bool JUCE_CALLTYPE showOkCancelBox (AlertIconType iconType,
                                               const String& title,
                                               const String& message,
                                            #if JUCE_MODAL_LOOPS_PERMITTED
                                               const String& button1Text = String(),
                                               const String& button2Text = String(),
                                               Component* associatedComponent = nullptr,
                                               ModalComponentManager::Callback* callback = nullptr);
                                            #else
                                               const String& button1Text,
                                               const String& button2Text,
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
        @param message      a longer, more descriptive message to show underneath the
                            headline
        @param button1Text  the text to show in the first button - if an empty string, then
                            "yes" will be used (or a localised version of it)
        @param button2Text  the text to show in the first button - if an empty string, then
                            "no" will be used (or a localised version of it)
        @param button3Text  the text to show in the first button - if an empty string, then
                            "cancel" will be used (or a localised version of it)
        @param associatedComponent   if this is non-null, it specifies the component that the
                            alert window should be associated with. Depending on the look
                            and feel, this might be used for positioning of the alert window.
        @param callback     if this is non-null, the menu will be launched asynchronously,
                            returning immediately, and the callback will receive a call to its
                            modalStateFinished() when the box is dismissed, with its parameter
                            being 1 if the "yes" button was pressed, 2 for the "no" button, or 0
                            if it was cancelled. The callback object will be owned and deleted by the
                            system, so make sure that it works safely and doesn't keep any references
                            to objects that might be deleted before it gets called.

        @returns If the callback parameter has been set, this returns 0. Otherwise, it
                 returns one of the following values:
                 - 0 if the third button was pressed (normally used for 'cancel')
                 - 1 if the first button was pressed (normally used for 'yes')
                 - 2 if the middle button was pressed (normally used for 'no')
    */
    static int JUCE_CALLTYPE showYesNoCancelBox (AlertIconType iconType,
                                                 const String& title,
                                                 const String& message,
                                               #if JUCE_MODAL_LOOPS_PERMITTED
                                                 const String& button1Text = String(),
                                                 const String& button2Text = String(),
                                                 const String& button3Text = String(),
                                                 Component* associatedComponent = nullptr,
                                                 ModalComponentManager::Callback* callback = nullptr);
                                               #else
                                                 const String& button1Text,
                                                 const String& button2Text,
                                                 const String& button3Text,
                                                 Component* associatedComponent,
                                                 ModalComponentManager::Callback* callback);
                                               #endif

    //==============================================================================
    /** Shows an operating-system native dialog box.

        @param title        the title to use at the top
        @param bodyText     the longer message to show
        @param isOkCancel   if true, this will show an ok/cancel box, if false,
                            it'll show a box with just an ok button
        @returns true if the ok button was pressed, false if they pressed cancel.
    */
   #if JUCE_MODAL_LOOPS_PERMITTED
    static bool JUCE_CALLTYPE showNativeDialogBox (const String& title,
                                                   const String& bodyText,
                                                   bool isOkCancel);
   #endif


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the alert box.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1001800,  /**< The background colour for the window. */
        textColourId                = 0x1001810,  /**< The colour for the text. */
        outlineColourId             = 0x1001820   /**< An optional colour to use to draw a border around the window. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        alert-window drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

        virtual AlertWindow* createAlertWindow (const String& title, const String& message,
                                                const String& button1,
                                                const String& button2,
                                                const String& button3,
                                                AlertWindow::AlertIconType iconType,
                                                int numButtons,
                                                Component* associatedComponent) = 0;

        virtual void drawAlertBox (Graphics&, AlertWindow&, const Rectangle<int>& textArea, TextLayout&) = 0;

        virtual int getAlertBoxWindowFlags() = 0;

        virtual Array<int> getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>&) = 0;
        virtual int getAlertWindowButtonHeight() = 0;

        virtual Font getAlertWindowTitleFont() = 0;
        virtual Font getAlertWindowMessageFont() = 0;
        virtual Font getAlertWindowFont() = 0;
    };

protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    void userTriedToCloseWindow() override;
    /** @internal */
    int getDesktopWindowStyleFlags() const override;

private:
    //==============================================================================
    String text;
    TextLayout textLayout;
    AlertIconType alertIconType;
    ComponentBoundsConstrainer constrainer;
    ComponentDragger dragger;
    Rectangle<int> textArea;
    OwnedArray<TextButton> buttons;
    OwnedArray<TextEditor> textBoxes;
    OwnedArray<ComboBox> comboBoxes;
    OwnedArray<ProgressBar> progressBars;
    Array<Component*> customComps;
    OwnedArray<Component> textBlocks;
    Array<Component*> allComps;
    StringArray textboxNames, comboBoxNames;
    Component* const associatedComponent;
    bool escapeKeyCancels = true;

    void exitAlert (Button* button);
    void updateLayout (bool onlyIncreaseSize);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertWindow)
};

} // namespace juce
