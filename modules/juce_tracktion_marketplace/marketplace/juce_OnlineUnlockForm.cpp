/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct Spinner  : public Component, private Timer
{
    Spinner()                       { startTimer (1000 / 50); }
    void timerCallback() override   { repaint(); }

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawSpinningWaitAnimation (g, Colours::darkgrey, 0, 0, getWidth(), getHeight());
    }
};

struct OnlineUnlockForm::OverlayComp  : public Component,
                                        private Thread,
                                        private Timer
{
    OverlayComp (OnlineUnlockForm& f)  : Thread (String()), form (f)
    {
        email = form.emailBox.getText();
        password = form.passwordBox.getText();
        addAndMakeVisible (spinner);

        startThread (4);
    }

    ~OverlayComp()
    {
        stopThread (10000);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white.withAlpha (0.97f));

        g.setColour (Colours::black);
        g.setFont (15.0f);

        g.drawFittedText (TRANS("Contacting XYZ...").replace ("XYZ", form.status.getWebsiteName()),
                          getLocalBounds().reduced (20, 0).removeFromTop (proportionOfHeight (0.6f)),
                          Justification::centred, 5);
    }

    void resized() override
    {
        const int spinnerSize = 40;
        spinner.setBounds ((getWidth() - spinnerSize) / 2, proportionOfHeight (0.6f), spinnerSize, spinnerSize);
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
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              TRANS("Registration Failed"),
                                              result.errorMessage);
        }
        else if (result.informativeMessage.isNotEmpty())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                              TRANS("Registration Complete!"),
                                              result.informativeMessage);
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

    OnlineUnlockForm& form;
    Spinner spinner;
    OnlineUnlockStatus::UnlockResult result;
    String email, password;
};

static juce_wchar getDefaultPasswordChar() noexcept
{
   #if JUCE_LINUX
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

OnlineUnlockForm::OnlineUnlockForm (OnlineUnlockStatus& s,
                                    const String& userInstructions,
                                    bool hasCancelButton)
    : message (String(), userInstructions),
      passwordBox (String(), getDefaultPasswordChar()),
      registerButton (TRANS("Register")),
      cancelButton (TRANS ("Cancel")),
      status (s)
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

    Rectangle<int> r (getLocalBounds().reduced (10, 20));

    Rectangle<int> buttonArea (r.removeFromBottom (buttonHeight));
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
    Font font (Font::getDefaultTypefaceForFont (Font (Font::getDefaultSansSerifFontName(),
                                                      Font::getDefaultStyle(),
                                                      5.0f)));

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

    emailBox.setTextToShowWhenEmpty (TRANS("Email Address"), labelCol);
    passwordBox.setTextToShowWhenEmpty (TRANS("Password"), labelCol);
}

void OnlineUnlockForm::showBubbleMessage (const String& text, Component& target)
{
    bubble = new BubbleMessageComponent (500);
    addChildComponent (bubble);

    AttributedString attString;
    attString.append (text, Font (16.0f));

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

        addAndMakeVisible (unlockingOverlay = new OverlayComp (*this));
        resized();
        unlockingOverlay->enterModalState();
    }
}

void OnlineUnlockForm::dismiss()
{
    delete this;
}
