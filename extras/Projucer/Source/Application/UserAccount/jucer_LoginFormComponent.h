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

#pragma once


#include "../../Project/UI/jucer_UserAvatarComponent.h"

//==============================================================================
class LoginFormComponent  : public Component
{
public:
    LoginFormComponent (MainWindow& window)
        : mainWindow (window)
    {
        setTitle ("Login");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (emailBox);
        emailBox.setTextToShowWhenEmpty ("Email", Colours::black.withAlpha (0.2f));
        emailBox.setJustification (Justification::centredLeft);
        emailBox.onReturnKey = [this] { submitDetails(); };
        emailBox.setTitle ("Email");

        addAndMakeVisible (passwordBox);
        passwordBox.setTextToShowWhenEmpty ("Password", Colours::black.withAlpha (0.2f));
        passwordBox.setPasswordCharacter ((juce_wchar) 0x2022);
        passwordBox.setJustification (Justification::centredLeft);
        passwordBox.onReturnKey = [this] { submitDetails(); };
        passwordBox.setTitle ("Password");

        addAndMakeVisible (logInButton);
        logInButton.onClick = [this] { submitDetails(); };

        addAndMakeVisible (enableGPLButton);
        enableGPLButton.onClick = [this]
        {
            ProjucerApplication::getApp().getLicenseController().setState (LicenseController::getGPLState());
            mainWindow.hideLoginFormOverlay();
        };

        addAndMakeVisible (userAvatar);

        addAndMakeVisible (createAccountLabel);
        createAccountLabel.setFont (Font (14.0f, Font::underlined));
        createAccountLabel.addMouseListener (this, false);
        createAccountLabel.setMouseCursor (MouseCursor::PointingHandCursor);

        addAndMakeVisible (errorMessageLabel);
        errorMessageLabel.setMinimumHorizontalScale (1.0f);
        errorMessageLabel.setFont (12.0f);
        errorMessageLabel.setColour (Label::textColourId, Colours::red);
        errorMessageLabel.setVisible (false);

        dismissButton.setShape (getLookAndFeel().getCrossShape (1.0f), false, true, false);
        addAndMakeVisible (dismissButton);
        dismissButton.onClick = [this] { mainWindow.hideLoginFormOverlay(); };
        dismissButton.setTitle ("Dismiss");

        setWantsKeyboardFocus (true);
        setOpaque (true);

        lookAndFeelChanged();

        setSize (300, 350);
    }

    ~LoginFormComponent() override
    {
        ProjucerApplication::getApp().getLicenseController().cancelSignIn();
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (20);
        auto spacing = bounds.getHeight() / 20;

        userAvatar.setBounds (bounds.removeFromTop (iconHeight).reduced ((bounds.getWidth() / 2) - (iconHeight / 2), 0));

        errorMessageLabel.setBounds (bounds.removeFromTop (spacing));
        bounds.removeFromTop (spacing / 2);

        auto textEditorHeight = bounds.getHeight() / 5;

        emailBox.setBounds (bounds.removeFromTop (textEditorHeight));
        bounds.removeFromTop (spacing);

        passwordBox.setBounds (bounds.removeFromTop (textEditorHeight));
        bounds.removeFromTop (spacing * 2);

        emailBox.setFont (Font ((float) textEditorHeight / 2.5f));
        passwordBox.setFont (Font ((float) textEditorHeight / 2.5f));

        logInButton.setBounds (bounds.removeFromTop (textEditorHeight));

        auto slice = bounds.removeFromTop (textEditorHeight);
        createAccountLabel.setBounds (slice.removeFromLeft (createAccountLabel.getFont().getStringWidth (createAccountLabel.getText()) + 5));
        slice.removeFromLeft (15);
        enableGPLButton.setBounds (slice.reduced (0, 5));

        dismissButton.setBounds (getLocalBounds().reduced (10).removeFromTop (20).removeFromRight (20));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId).contrasting (0.1f));
    }

    void mouseUp (const MouseEvent& event) override
    {
        if (event.eventComponent == &createAccountLabel)
            URL ("https://juce.com/verification/register").launchInDefaultBrowser();
    }

    void lookAndFeelChanged() override
    {
        enableGPLButton.setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
    }

private:
    class ProgressButton  : public TextButton,
                            private Timer
    {
    public:
        ProgressButton (const String& buttonName)
            : TextButton (buttonName), text (buttonName)
        {
        }

        void setBusy (bool shouldBeBusy)
        {
            isInProgress = shouldBeBusy;

            if (isInProgress)
            {
                setEnabled (false);
                setButtonText ({});
                startTimerHz (30);
            }
            else
            {
                setEnabled (true);
                setButtonText (text);
                stopTimer();
            }
        }

        void paint (Graphics& g) override
        {
            TextButton::paint (g);

            if (isInProgress)
            {
                auto size = getHeight() - 10;
                auto halfSize = size / 2;

                getLookAndFeel().drawSpinningWaitAnimation (g, Colours::white,
                                                            (getWidth() / 2) - halfSize, (getHeight() / 2) - halfSize,
                                                            size, size);
            }
        }

    private:
        void timerCallback() override
        {
            repaint();
        }

        String text;
        bool isInProgress = false;
    };

    //==============================================================================
    void updateLoginButtonStates (bool isLoggingIn)
    {
        logInButton.setBusy (isLoggingIn);

        emailBox.setEnabled (! isLoggingIn);
        passwordBox.setEnabled (! isLoggingIn);
    }

    void submitDetails()
    {
        auto loginFormError = checkLoginFormsAreValid();

        if (loginFormError.isNotEmpty())
        {
            showErrorMessage (loginFormError);
            return;
        }

        updateLoginButtonStates (true);

        auto completionCallback = [weakThis = SafePointer<LoginFormComponent> { this }] (const String& errorMessage)
        {
            if (weakThis == nullptr)
                return;

            weakThis->updateLoginButtonStates (false);

            if (errorMessage.isNotEmpty())
            {
                weakThis->showErrorMessage (errorMessage);
            }
            else
            {
                weakThis->hideErrorMessage();
                weakThis->mainWindow.hideLoginFormOverlay();
                ProjucerApplication::getApp().getCommandManager().commandStatusChanged();
            }
        };

        ProjucerApplication::getApp().getLicenseController().signIn (emailBox.getText(), passwordBox.getText(),
                                                                     std::move (completionCallback));
    }

    String checkLoginFormsAreValid() const
    {
        auto email = emailBox.getText();

        if (email.isEmpty() || email.indexOfChar ('@') < 0)
            return "Please enter a valid email.";

        auto password = passwordBox.getText();

        if (password.isEmpty() || password.length() < 8)
            return "Please enter a valid password.";

        return {};
    }

    void showErrorMessage (const String& errorMessage)
    {
        errorMessageLabel.setText (errorMessage, dontSendNotification);
        errorMessageLabel.setVisible (true);
    }

    void hideErrorMessage()
    {
        errorMessageLabel.setText ({}, dontSendNotification);
        errorMessageLabel.setVisible (false);
    }

    //==============================================================================
    static constexpr int iconHeight = 50;

    MainWindow& mainWindow;

    TextEditor emailBox, passwordBox;
    ProgressButton logInButton { "Sign In" };
    TextButton enableGPLButton { "Enable GPL Mode" };
    ShapeButton dismissButton { {},
                                findColour (treeIconColourId),
                                findColour (treeIconColourId).overlaidWith (findColour (defaultHighlightedTextColourId).withAlpha (0.2f)),
                                findColour (treeIconColourId).overlaidWith (findColour (defaultHighlightedTextColourId).withAlpha (0.4f)) };
    UserAvatarComponent userAvatar { false };
    Label createAccountLabel { {}, "Create an account" },
          errorMessageLabel { {}, {} };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoginFormComponent)
};
