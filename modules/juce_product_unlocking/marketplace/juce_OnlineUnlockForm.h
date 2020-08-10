/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

/** Acts as a GUI which asks the user for their details, and calls the appropriate
    methods on your OnlineUnlockStatus object to attempt to register the app.

    You should create one of these components and add it to your parent window,
    or use a DialogWindow to display it as a pop-up. But if you're writing a plugin,
    then DO NOT USE A DIALOG WINDOW! Add it as a child component of your plugin's editor
    component instead. Plugins that pop up external registration windows are incredibly
    annoying, and cause all sorts of headaches for hosts. Don't be the person who
    writes that plugin that irritates everyone with a dialog box every time they
    try to scan for new plugins!

    Note that after adding it, you should put the component into a modal state,
    and it will automatically delete itself when it has completed.

    Although it deletes itself, it's also OK to delete it manually yourself
    if you need to get rid of it sooner.

    @see OnlineUnlockStatus

    @tags{ProductUnlocking}
*/
class JUCE_API  OnlineUnlockForm  : public Component,
                                    private Button::Listener
{
public:
    /** Creates an unlock form that will work with the given status object.
        The userInstructions will be displayed above the email and password boxes.
    */
    OnlineUnlockForm (OnlineUnlockStatus&,
                      const String& userInstructions,
                      bool hasCancelButton = true,
                      bool overlayHasCancelButton = false);

    /** Destructor. */
    ~OnlineUnlockForm() override;

    /** This is called when the form is dismissed (either cancelled or when registration
        succeeds).
        By default it will delete this, but you can override it to do other things.
    */
    virtual void dismiss();

    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void lookAndFeelChanged() override;

    Label message;
    TextEditor emailBox, passwordBox;
    TextButton registerButton, cancelButton;

private:
    OnlineUnlockStatus& status;
    std::unique_ptr<BubbleMessageComponent> bubble;

    bool showOverlayCancelButton;

    struct OverlayComp;
    friend struct OverlayComp;
    Component::SafePointer<Component> unlockingOverlay;

    void buttonClicked (Button*) override;
    void attemptRegistration();
    void showBubbleMessage (const String&, Component&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OnlineUnlockForm)
};

} // namespace juce
