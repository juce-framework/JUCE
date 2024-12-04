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

struct Spinner final : public Component,
                       private Timer
{
    Spinner()                       { startTimer (1000 / 50); }
    void timerCallback() override   { repaint(); }

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawSpinningWaitAnimation (g, Colours::darkgrey, 0, 0, getWidth(), getHeight());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spinner)
};

struct OnlineUnlockForm::OverlayComp final : public Component,
                                             private Thread,
                                             private Timer,
                                             private Button::Listener
{
    OverlayComp (OnlineUnlockForm& f, bool hasCancelButton = false)
        : Thread (String()), form (f)
    {
        result.succeeded = false;
        email = form.emailBox.getText();
        password = form.passwordBox.getText();
        addAndMakeVisible (spinner);

        if (hasCancelButton)
        {
            cancelButton.reset (new TextButton (TRANS ("Cancel")));
            addAndMakeVisible (cancelButton.get());
            cancelButton->addListener (this);
        }

        startThread (Priority::normal);
    }

    ~OverlayComp() override
    {
        stopThread (10000);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white.withAlpha (0.97f));

        g.setColour (Colours::black);
        g.setFont (15.0f);

        g.drawFittedText (TRANS ("Contacting XYZ...").replace ("XYZ", form.status.getWebsiteName()),
                          getLocalBounds().reduced (20, 0).removeFromTop (proportionOfHeight (0.6f)),
                          Justification::centred, 5);
    }

    void resized() override
    {
        const int spinnerSize = 40;
        spinner.setBounds ((getWidth() - spinnerSize) / 2, proportionOfHeight (0.6f), spinnerSize, spinnerSize);

        if (cancelButton != nullptr)
            cancelButton->setBounds (getLocalBounds().removeFromBottom (50).reduced (getWidth() / 4, 5));
    }

    void run() override
    {
        result = form.status.attemptWebserverUnlock (email, password);
        startTimer (100);
    }

    void timerCallback() override
    {
        spinner.setVisible (false);
        stopTimer();

        if (result.errorMessage.isNotEmpty())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             TRANS ("Registration Failed"),
                                                             result.errorMessage,
                                                             {},
                                                             &form);
            form.messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else if (result.informativeMessage.isNotEmpty())
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                             TRANS ("Registration Complete!"),
                                                             result.informativeMessage,
                                                             {},
                                                             &form);
            form.messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else if (result.urlToLaunch.isNotEmpty())
        {
            URL url (result.urlToLaunch);
            url.launchInDefaultBrowser();
        }

        // (local copies because we're about to delete this)
        const bool worked = result.succeeded;
        OnlineUnlockForm& f = form;

        delete this;

        if (worked)
            f.dismiss();
    }

    void buttonClicked (Button* button) override
    {
        if (button == cancelButton.get())
        {
            form.status.userCancelled();

            spinner.setVisible (false);
            stopTimer();

            delete this;
        }
    }

    OnlineUnlockForm& form;
    Spinner spinner;
    OnlineUnlockStatus::UnlockResult result;
    String email, password;

    std::unique_ptr<TextButton> cancelButton;

    JUCE_LEAK_DETECTOR (OnlineUnlockForm::OverlayComp)
};

static juce_wchar getDefaultPasswordChar() noexcept
{
   #if JUCE_LINUX || JUCE_BSD
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

OnlineUnlockForm::OnlineUnlockForm (OnlineUnlockStatus& s,
                                    const String& userInstructions,
                                    bool hasCancelButton,
                                    bool overlayHasCancelButton)
    : message (String(), userInstructions),
      passwordBox (String(), getDefaultPasswordChar()),
      registerButton (TRANS ("Register")),
      cancelButton (TRANS ("Cancel")),
      status (s),
      showOverlayCancelButton (overlayHasCancelButton)
{
    // Please supply a message to tell your users what to do!
    jassert (userInstructions.isNotEmpty());

    setOpaque (true);

    emailBox.setText (status.getUserEmail());
    message.setJustificationType (Justification::centred);

    addAndMakeVisible (message);
    addAndMakeVisible (emailBox);
    addAndMakeVisible (passwordBox);
    addAndMakeVisible (registerButton);

    if (hasCancelButton)
        addAndMakeVisible (cancelButton);

    emailBox.setEscapeAndReturnKeysConsumed (false);
    passwordBox.setEscapeAndReturnKeysConsumed (false);

    registerButton.addShortcut (KeyPress (KeyPress::returnKey));

    registerButton.addListener (this);
    cancelButton.addListener (this);

    lookAndFeelChanged();
    setSize (500, 250);
}

OnlineUnlockForm::~OnlineUnlockForm()
{
    unlockingOverlay.deleteAndZero();
}

void OnlineUnlockForm::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
}

void OnlineUnlockForm::resized()
{
    /* If you're writing a plugin, then DO NOT USE A POP-UP A DIALOG WINDOW!
       Plugins that create external windows are incredibly annoying for users, and
       cause all sorts of headaches for hosts. Don't be the person who writes that
       plugin that irritates everyone with a nagging dialog box every time they scan!
    */
    jassert (JUCEApplicationBase::isStandaloneApp() || findParentComponentOfClass<DialogWindow>() == nullptr);

    const int buttonHeight = 22;

    auto r = getLocalBounds().reduced (10, 20);

    auto buttonArea = r.removeFromBottom (buttonHeight);
    registerButton.changeWidthToFitText (buttonHeight);
    cancelButton.changeWidthToFitText (buttonHeight);

    const int gap = 20;
    buttonArea = buttonArea.withSizeKeepingCentre (registerButton.getWidth()
                                                     + (cancelButton.isVisible() ? gap + cancelButton.getWidth() : 0),
                                                   buttonHeight);
    registerButton.setBounds (buttonArea.removeFromLeft (registerButton.getWidth()));
    buttonArea.removeFromLeft (gap);
    cancelButton.setBounds (buttonArea);

    r.removeFromBottom (20);

    // (force use of a default system font to make sure it has the password blob character)
    const auto typeface = Font::getDefaultTypefaceForFont (FontOptions (Font::getDefaultSansSerifFontName(),
                                                                        Font::getDefaultStyle(),
                                                                        5.0f));
    Font font (withDefaultMetrics (FontOptions { typeface }));

    const int boxHeight = 24;
    passwordBox.setBounds (r.removeFromBottom (boxHeight));
    passwordBox.setInputRestrictions (64);
    passwordBox.setFont (font);

    r.removeFromBottom (20);
    emailBox.setBounds (r.removeFromBottom (boxHeight));
    emailBox.setInputRestrictions (512);
    emailBox.setFont (font);

    r.removeFromBottom (20);

    message.setBounds (r);

    if (unlockingOverlay != nullptr)
        unlockingOverlay->setBounds (getLocalBounds());
}

void OnlineUnlockForm::lookAndFeelChanged()
{
    Colour labelCol (findColour (TextEditor::backgroundColourId).contrasting (0.5f));

    emailBox.setTextToShowWhenEmpty (TRANS ("Email Address"), labelCol);
    passwordBox.setTextToShowWhenEmpty (TRANS ("Password"), labelCol);
}

void OnlineUnlockForm::showBubbleMessage (const String& text, Component& target)
{
    bubble.reset (new BubbleMessageComponent (500));
    addChildComponent (bubble.get());

    AttributedString attString;
    attString.append (text, withDefaultMetrics (FontOptions (16.0f)));

    bubble->showAt (getLocalArea (&target, target.getLocalBounds()),
                    attString, 500,  // numMillisecondsBeforeRemoving
                    true,  // removeWhenMouseClicked
                    false); // deleteSelfAfterUse
}

void OnlineUnlockForm::buttonClicked (Button* b)
{
    if (b == &registerButton)
        attemptRegistration();
    else if (b == &cancelButton)
        dismiss();
}

void OnlineUnlockForm::attemptRegistration()
{
    if (unlockingOverlay == nullptr)
    {
        if (emailBox.getText().trim().length() < 3)
        {
            showBubbleMessage (TRANS ("Please enter a valid email address!"), emailBox);
            return;
        }

        if (passwordBox.getText().trim().length() < 3)
        {
            showBubbleMessage (TRANS ("Please enter a valid password!"), passwordBox);
            return;
        }

        status.setUserEmail (emailBox.getText());

        addAndMakeVisible (unlockingOverlay = new OverlayComp (*this, showOverlayCancelButton));
        resized();
        unlockingOverlay->enterModalState();
    }
}

void OnlineUnlockForm::dismiss()
{
    delete this;
}

} // namespace juce
