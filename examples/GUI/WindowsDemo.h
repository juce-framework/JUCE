/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             WindowsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays various types of windows.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WindowsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Just a simple window that deletes itself when closed. */
class BasicWindow   : public DocumentWindow
{
public:
    BasicWindow (const String& name, Colour backgroundColour, int buttonsNeeded)
        : DocumentWindow (name, backgroundColour, buttonsNeeded)
    {}

    void closeButtonPressed()
    {
        delete this;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicWindow)
};

//==============================================================================
/** This window contains a ColourSelector which can be used to change the window's colour. */
class ColourSelectorWindow   : public DocumentWindow,
                               private ChangeListener
{
public:
    ColourSelectorWindow (const String& name, Colour backgroundColour, int buttonsNeeded)
        : DocumentWindow (name, backgroundColour, buttonsNeeded)
    {
        selector.setCurrentColour (backgroundColour);
        selector.setColour (ColourSelector::backgroundColourId, Colours::transparentWhite);
        selector.addChangeListener (this);
        setContentOwned (&selector, false);
    }

    ~ColourSelectorWindow()
    {
        selector.removeChangeListener (this);
    }

    void closeButtonPressed()
    {
        delete this;
    }

private:
    ColourSelector selector  { ColourSelector::showColourAtTop
                             | ColourSelector::showSliders
                             | ColourSelector::showColourspace };

    void changeListenerCallback (ChangeBroadcaster* source)
    {
        if (source == &selector)
            setBackgroundColour (selector.getCurrentColour());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourSelectorWindow)
};

//==============================================================================
class BouncingBallComponent : public Component,
                              public Timer
{
public:
    BouncingBallComponent()
    {
        setInterceptsMouseClicks (false, false);

        Random random;

        auto size = 10.0f + random.nextInt (30);

        ballBounds.setBounds (random.nextFloat() * 100.0f,
                              random.nextFloat() * 100.0f,
                              size, size);

        direction.x = random.nextFloat() * 8.0f - 4.0f;
        direction.y = random.nextFloat() * 8.0f - 4.0f;

        colour = Colour ((juce::uint32) random.nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        startTimer (60);
    }

    void paint (Graphics& g) override
    {
        g.setColour (colour);
        g.fillEllipse (ballBounds - getPosition().toFloat());
    }

    void timerCallback() override
    {
        ballBounds += direction;

        if (ballBounds.getX() < 0)                      direction.x =  std::abs (direction.x);
        if (ballBounds.getY() < 0)                      direction.y =  std::abs (direction.y);
        if (ballBounds.getRight()  > getParentWidth())  direction.x = -std::abs (direction.x);
        if (ballBounds.getBottom() > getParentHeight()) direction.y = -std::abs (direction.y);

        setBounds (ballBounds.getSmallestIntegerContainer());
    }

private:
    Colour colour;
    Rectangle<float> ballBounds;
    Point<float> direction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallComponent)
};

//==============================================================================
class BouncingBallsContainer : public Component
{
public:
    BouncingBallsContainer (int numBalls)
    {
        for (int i = 0; i < numBalls; ++i)
        {
            auto* newBall = new BouncingBallComponent();
            balls.add (newBall);
            addAndMakeVisible (newBall);
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        dragger.startDraggingComponent (this, e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        // as there's no titlebar we have to manage the dragging ourselves
        dragger.dragComponent (this, e, nullptr);
    }

    void paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colours::white);
        else
            g.fillAll (Colours::blue.withAlpha (0.2f));

        g.setFont (16.0f);
        g.setColour (Colours::black);
        g.drawFittedText ("This window has no titlebar and a transparent background.",
                          getLocalBounds().reduced (8, 0),
                          Justification::centred, 5);

        g.drawRect (getLocalBounds());
    }

private:
    ComponentDragger dragger;
    OwnedArray<BouncingBallComponent> balls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallsContainer)
};

//==============================================================================
class WindowsDemo   : public Component
{
public:
    enum Windows
    {
        dialog,
        document,
        alert,
        numWindows
    };

    WindowsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (showWindowsButton);
        showWindowsButton.onClick = [this] { showAllWindows(); };

        addAndMakeVisible (closeWindowsButton);
        closeWindowsButton.onClick = [this] { closeAllWindows(); };

        setSize (250, 250);
    }

    ~WindowsDemo()
    {
        if (dialogWindow != nullptr)
        {
            dialogWindow->exitModalState (0);

            // we are shutting down: can't wait for the message manager
            // to eventually delete this
            delete dialogWindow;
        }

        closeAllWindows();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::grey));
    }

    void resized() override
    {
        Rectangle<int> buttonSize (0, 0, 108, 28);

        Rectangle<int> area ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                             (getHeight() / 2) -  buttonSize.getHeight(),
                             buttonSize.getWidth(), buttonSize.getHeight());

        showWindowsButton .setBounds (area.reduced (2));
        closeWindowsButton.setBounds (area.translated (0, buttonSize.getHeight()).reduced (2));
    }

private:
    // Because in this demo the windows delete themselves, we'll use the
    // Component::SafePointer class to point to them, which automatically becomes
    // null when the component that it points to is deleted.
    Array<Component::SafePointer<Component>> windows;
    SafePointer<DialogWindow> dialogWindow;

    TextButton showWindowsButton   { "Show Windows" },
               closeWindowsButton  { "Close Windows" };

    void showAllWindows()
    {
        closeAllWindows();

        showDocumentWindow (false);
        showDocumentWindow (true);
        showTransparentWindow();
        showDialogWindow();
    }

    void closeAllWindows()
    {
        for (auto& window : windows)
            window.deleteAndZero();

        windows.clear();
    }

    void showDialogWindow()
    {
        String m;

        m << "Dialog Windows can be used to quickly show a component, usually blocking mouse input to other windows." << newLine
          << newLine
          << "They can also be quickly closed with the escape key, try it now.";

        DialogWindow::LaunchOptions options;
        auto* label = new Label();
        label->setText (m, dontSendNotification);
        label->setColour (Label::textColourId, Colours::whitesmoke);
        options.content.setOwned (label);

        Rectangle<int> area (0, 0, 300, 200);

        options.content->setSize (area.getWidth(), area.getHeight());

        options.dialogTitle                   = "Dialog Window";
        options.dialogBackgroundColour        = Colour (0xff0e345a);
        options.escapeKeyTriggersCloseButton  = true;
        options.useNativeTitleBar             = false;
        options.resizable                     = true;

        dialogWindow = options.launchAsync();

        if (dialogWindow != nullptr)
            dialogWindow->centreWithSize (300, 200);
    }

    void showDocumentWindow (bool native)
    {
        auto* dw = new ColourSelectorWindow ("Document Window", getRandomBrightColour(), DocumentWindow::allButtons);
        windows.add (dw);

        Rectangle<int> area (0, 0, 300, 400);

        RectanglePlacement placement ((native ? RectanglePlacement::xLeft
                                              : RectanglePlacement::xRight)
                                       | RectanglePlacement::yTop
                                       | RectanglePlacement::doNotResize);

        auto result = placement.appliedTo (area, Desktop::getInstance().getDisplays()
                                                         .getMainDisplay().userArea.reduced (20));
        dw->setBounds (result);

        dw->setResizable (true, ! native);
        dw->setUsingNativeTitleBar (native);
        dw->setVisible (true);
    }

    void showTransparentWindow()
    {
        auto* balls = new BouncingBallsContainer (3);
        balls->addToDesktop (ComponentPeer::windowIsTemporary);
        windows.add (balls);

        Rectangle<int> area (0, 0, 200, 200);

        RectanglePlacement placement (RectanglePlacement::xLeft
                                       | RectanglePlacement::yBottom
                                       | RectanglePlacement::doNotResize);

        auto result = placement.appliedTo (area, Desktop::getInstance().getDisplays()
                                                         .getMainDisplay().userArea.reduced (20));
        balls->setBounds (result);

        balls->setVisible (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDemo)
};
