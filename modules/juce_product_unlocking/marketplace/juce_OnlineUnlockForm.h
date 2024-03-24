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
    ScopedMessageBox messageBox;
    Component::SafePointer<Component> unlockingOverlay;

    void buttonClicked (Button*) override;
    void attemptRegistration();
    void showBubbleMessage (const String&, Component&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OnlineUnlockForm)
};

} // namespace juce
