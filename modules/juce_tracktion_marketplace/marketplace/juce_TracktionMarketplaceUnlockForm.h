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

/** Acts as a GUI which asks the user for their details, and calls the approriate
    methods on your TracktionMarketplaceStatus object to attempt to register the app.

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

    @see TracktionMarketplaceStatus
*/
class JUCE_API  TracktionMarketplaceUnlockForm  : public Component,
                                                  private ButtonListener
{
public:
    /** Creates an unlock form that will work with the given status object.
        The userInstructions will be displayed above the email and password boxes.
    */
    TracktionMarketplaceUnlockForm (TracktionMarketplaceStatus&,
                                    const String& userInstructions);

    /** Destructor. */
    ~TracktionMarketplaceUnlockForm();

    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void lookAndFeelChanged() override;

private:
    TracktionMarketplaceStatus& status;

    Label message;
    TextEditor emailBox, passwordBox;
    TextButton registerButton, cancelButton;
    ScopedPointer<BubbleMessageComponent> bubble;

    struct OverlayComp;
    friend struct OverlayComp;
    Component::SafePointer<Component> unlockingOverlay;

    void buttonClicked (Button*) override;
    void attemptRegistration();
    void cancel();
    void showBubbleMessage (const String&, Component&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracktionMarketplaceUnlockForm)
};
