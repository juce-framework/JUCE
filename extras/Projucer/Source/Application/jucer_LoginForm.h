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

#ifndef PROJUCER_LOGINFORM_H_INCLUDED
#define PROJUCER_LOGINFORM_H_INCLUDED


class LoginForm  : public Component,
                   private ButtonListener,
                   private TextEditor::Listener,
                   private ProjucerLicenses::LoginCallback
{
public:
    LoginForm()
        : cancelButton (TRANS("Cancel")),
          loginButton (TRANS("Login")),
          registerButton (TRANS("Register")),
          userIDEditor ("User ID text editor"),
          passwordEditor ("Password TextEditor", juce_wchar (0x2022)),
          userIDLabel ("User-ID Label", TRANS("Username")),
          passwordLabel ("Password Label", TRANS("Password")),
          errorLabel ("Error Label", String()),
          rememberLoginCheckbox (TRANS("Remember login")),
          forgotPasswordButton (TRANS("Forgotten your password?"),
                                URL ("https://auth.roli.com/forgot-password?referer=projucer")),
          rememberLogin (true)
    {
        setLookAndFeel (&lookAndFeel);

        ScopedPointer<XmlElement> svg (XmlDocument::parse (BinaryData::projucer_login_bg_svg));
        backgroundImage = Drawable::createFromSVG (*svg);

        initialiseTextField (passwordEditor, passwordLabel);
        addAndMakeVisible (passwordEditor);
        initialiseTextField (userIDEditor, userIDLabel);
        addAndMakeVisible (userIDEditor);

        String userName = ProjucerLicenses::getInstance()->getLoginName();
        userIDEditor.setText (userName.isEmpty() ? getLastUserName() : userName);

        initialiseLabel (errorLabel, Font::plain, ProjucerDialogLookAndFeel::getErrorTextColour());
        addChildComponent (errorLabel);

        addChildComponent (spinner);

        rememberLoginCheckbox.setColour (ToggleButton::textColourId, Colours::white);
        rememberLoginCheckbox.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
        rememberLoginCheckbox.setToggleState (rememberLogin, dontSendNotification);
        addAndMakeVisible (rememberLoginCheckbox);
        rememberLoginCheckbox.addListener (this);

        forgotPasswordButton.setColour (HyperlinkButton::textColourId, Colours::white);
        forgotPasswordButton.setFont (ProjucerDialogLookAndFeel::getDialogFont().withHeight (lookAndFeel.labelFontSize), false, Justification::topLeft);
        addAndMakeVisible (forgotPasswordButton);

        initialiseButton (loginButton, KeyPress::returnKey);
        addAndMakeVisible (loginButton);
        initialiseButton (registerButton);
        addAndMakeVisible (registerButton);
        initialiseButton (cancelButton, KeyPress::escapeKey);
        addAndMakeVisible (cancelButton);
        cancelButton.getProperties().set ("isSecondaryButton", true);

        centreWithSize (425, 685);
    }

    ~LoginForm()
    {
        ProjucerApplication::getApp().hideLoginForm();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xff4d4d4d));

        g.setColour (Colours::black);
        backgroundImage->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0f);
    }

    void resized() override
    {
        const int xMargin = 81;
        const int yMargin = 132;

        const int labelHeight = 24;
        const int textFieldHeight = 33;

        Rectangle<int> r = getLocalBounds().reduced (xMargin, yMargin);
        r.setWidth (r.getWidth() + 1);

        Point<int> labelOffset = Point<int> (-6, 4);

        userIDLabel.setBounds (r.removeFromTop (labelHeight) + labelOffset);
        userIDEditor.setBounds (r.removeFromTop (textFieldHeight));

        passwordLabel.setBounds (r.removeFromTop (labelHeight) + labelOffset);
        passwordEditor.setBounds (r.removeFromTop (textFieldHeight));

        r.removeFromTop (6);
        rememberLoginCheckbox.setBounds (r.removeFromTop (labelHeight) + Point<int> (-4, 0));

        r.removeFromTop (8);
        errorLabel.setBounds (r.removeFromTop (43).withTrimmedLeft (15));
        spinner.setBounds (errorLabel.getBounds().withSizeKeepingCentre (20, 20) + Point<int> (-7, -10));

        const int buttonHeight = 40;
        const int buttonMargin = 13;

        loginButton.setBounds (r.removeFromTop (buttonHeight));
        r.removeFromTop (buttonMargin);

        registerButton.setBounds (r.withHeight (buttonHeight).removeFromLeft ((r.getWidth() - buttonMargin) / 2));
        cancelButton.setBounds (r.withHeight (buttonHeight).removeFromRight ((r.getWidth() - buttonMargin) / 2));

        r.removeFromTop (45);
        forgotPasswordButton.setBounds (r.withHeight (labelHeight) + Point<int> (-2, 0));
    }

private:
    //==============================================================================
    struct Spinner  : public Component,
                      private Timer
    {
        Spinner()
        {
            setInterceptsMouseClicks (false, false);
        }

        void paint (Graphics& g) override
        {
            getLookAndFeel().drawSpinningWaitAnimation (g, Colours::white, 0, 0, getWidth(), getHeight());
            startTimer (50);
        }

        void timerCallback() override
        {
            if (isVisible())
                repaint();
            else
                stopTimer();
        }
    };

    //==============================================================================
    void initialiseTextField (TextEditor& textField, Label& associatedLabel)
    {
        textField.setColour (TextEditor::focusedOutlineColourId, Colours::transparentWhite);
        textField.setColour (TextEditor::highlightColourId, ProjucerDialogLookAndFeel::getErrorTextColour());
        textField.setFont (ProjucerDialogLookAndFeel::getDialogFont().withHeight (17));
        textField.addListener (this);
        associatedLabel.setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (associatedLabel);
    }

    void initialiseButton (TextButton& button, const int associatedKeyPress = 0)
    {
        if (associatedKeyPress != 0)
            button.addShortcut (KeyPress (associatedKeyPress));

        button.addListener (this);
    }

    void initialiseLabel (Label& label, Font::FontStyleFlags fontFlags, Colour textColour)
    {
        label.setFont (Font (15.0f, fontFlags));
        label.setJustificationType (Justification::topLeft);
        label.setColour (Label::textColourId, textColour);
    }

    //==============================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &cancelButton)            cancelButtonClicked();
        if (button == &loginButton)             loginButtonClicked();
        if (button == &registerButton)          registerButtonClicked();
        if (button == &rememberLoginCheckbox)   rememberLoginCheckboxClicked();
    }

    void cancelButtonClicked()
    {
        if (DialogWindow* parentDialog = findParentComponentOfClass<DialogWindow>())
            parentDialog->exitModalState (-1);
    }

    void loginButtonClicked()
    {
        loginName = userIDEditor.getText();
        getGlobalProperties().setValue ("lastUserName", loginName);

        password = passwordEditor.getText();

        if (! isValidEmail (loginName) || password.isEmpty())
        {
            handleInvalidLogin();
            return;
        }

        loginButton.setEnabled (false);
        cancelButton.setEnabled (false);
        registerButton.setEnabled (false);
        errorLabel.setVisible (false);
        spinner.setVisible (true);

        ProjucerLicenses::getInstance()->login (loginName, password, rememberLogin, this);
    }

    void registerButtonClicked()
    {
        URL (getServerURL() + "projucer_trial").launchInDefaultBrowser();
    }

    void textEditorReturnKeyPressed (TextEditor&) override
    {
        loginButtonClicked();
    }

    void rememberLoginCheckboxClicked()
    {
        rememberLogin = rememberLoginCheckbox.getToggleState();
    }

    String getLastUserName() const
    {
        return getGlobalProperties().getValue ("lastUserName");
    }

    void handleInvalidLogin()
    {
        if (!isValidEmail (loginName))
            loginError (TRANS ("Please enter a valid e-mail address"), true);

        if (password.isEmpty())
            loginError (TRANS ("Please specify a valid password"), false);
    }

    bool isValidEmail (const String& email)
    {
        return email.isNotEmpty();
    }

    void loginError (const String& errorMessage, bool hiliteUserID) override
    {
        spinner.setVisible (false);
        errorLabel.setText (errorMessage, dontSendNotification);
        errorLabel.setVisible (true);

        TextEditor& field = hiliteUserID ? userIDEditor : passwordEditor;
        field.setColour (TextEditor::focusedOutlineColourId, Colour (0x84F08080));
        field.toFront (true);

        loginButton.setEnabled (true);
        cancelButton.setEnabled (true);
        registerButton.setEnabled (true);

        ProjucerApplication::getApp().updateAllBuildTabs();
    }

    void loginSuccess (const String& username, const String& apiKey) override
    {
        ignoreUnused (username, apiKey);

        spinner.setVisible (false);
        if (DialogWindow* parentDialog = findParentComponentOfClass<DialogWindow>())
        {
            parentDialog->exitModalState (0);
            ProjucerApplication::getApp().updateAllBuildTabs();

            if (ProjucerLicenses::getInstance()->hasFreeToUseLicense())
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                  "Free to use license info",
                                                  "The free-to-use license expires on 31st January 2017 Midnight GMT");
            }
        }
    }

    TextButton cancelButton, loginButton, registerButton;
    TextEditor userIDEditor, passwordEditor;
    Label userIDLabel, passwordLabel, errorLabel;
    ToggleButton rememberLoginCheckbox;
    HyperlinkButton forgotPasswordButton;
    Spinner spinner;
    String loginName, password;
    bool rememberLogin;
    ScopedPointer<Drawable> backgroundImage;

    ProjucerDialogLookAndFeel lookAndFeel;

    static String getServerURL()       { return "https://my.roli.com/"; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoginForm)
};


#endif  // PROJUCER_LOGINFORM_H_INCLUDED
