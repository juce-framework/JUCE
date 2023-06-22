/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

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

        auto size = 10.0f + (float) random.nextInt (30);

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

        if (ballBounds.getX() < 0)                              direction.x =  std::abs (direction.x);
        if (ballBounds.getY() < 0)                              direction.y =  std::abs (direction.y);
        if (ballBounds.getRight()  > (float) getParentWidth())  direction.x = -std::abs (direction.x);
        if (ballBounds.getBottom() > (float) getParentHeight()) direction.y = -std::abs (direction.y);

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

        addAndMakeVisible (alertWindowResult);
        alertWindowResult.setJustificationType (Justification::centred);

        setSize (250, 250);
    }

    ~WindowsDemo() override
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
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.justifyContent = FlexBox::JustifyContent::center;

        constexpr auto buttonWidth = 108.0f;
        constexpr auto componentHeight = 24.0f;
        constexpr auto gap = 4.0f;

        flexBox.items.add (FlexItem { showWindowsButton }.withHeight (componentHeight)
                                                         .withMinWidth (buttonWidth)
                                                         .withAlignSelf (FlexItem::AlignSelf::center));

        flexBox.items.add (FlexItem{}.withHeight (gap));
        flexBox.items.add (FlexItem { closeWindowsButton }.withHeight (componentHeight)
                                                          .withMinWidth (buttonWidth)
                                                          .withAlignSelf (FlexItem::AlignSelf::center));

        flexBox.items.add (FlexItem{}.withHeight (gap));
        flexBox.items.add (FlexItem { alertWindowResult }.withHeight (componentHeight));

        flexBox.performLayout (getLocalBounds());
    }

private:
    // Because in this demo the windows delete themselves, we'll use the
    // Component::SafePointer class to point to them, which automatically becomes
    // null when the component that it points to is deleted.
    Array<Component::SafePointer<Component>> windows;
    SafePointer<DialogWindow> dialogWindow;

    TextButton showWindowsButton   { "Show Windows" },
               closeWindowsButton  { "Close Windows" };
    Label alertWindowResult { "Alert Window result" };

    void showAllWindows()
    {
        closeAllWindows();

        showDocumentWindow (false);
        showDocumentWindow (true);
        showTransparentWindow();
        showAlertWindow();
        showDialogWindow();
    }

    void closeAllWindows()
    {
        for (auto& window : windows)
            window.deleteAndZero();

        windows.clear();
        alertWindowResult.setText ("", dontSendNotification);
    }

    static auto getDisplayArea()
    {
        return Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced (20);
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

        auto result = placement.appliedTo (area, getDisplayArea());
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

        auto result = placement.appliedTo (area, getDisplayArea());
        balls->setBounds (result);

        balls->setVisible (true);
    }

    void showAlertWindow()
    {
        auto* alertWindow = new AlertWindow ("Alert Window",
                                             "For more complex dialogs, you can easily add components to an AlertWindow, such as...",
                                             MessageBoxIconType::InfoIcon);
        windows.add (alertWindow);

        alertWindow->addTextBlock ("Text block");
        alertWindow->addComboBox ("Combo box", {"Combo box", "Item 2", "Item 3"});
        alertWindow->addTextEditor ("Text editor", "Text editor");
        alertWindow->addTextEditor ("Password", "password", "including for passwords", true);
        alertWindowCustomComponent.emplace();
        alertWindow->addCustomComponent (&(*alertWindowCustomComponent));
        alertWindow->addTextBlock ("Progress bar");
        alertWindow->addProgressBarComponent (alertWindowCustomComponent->value, ProgressBar::Style::linear);
        alertWindow->addProgressBarComponent (alertWindowCustomComponent->value, ProgressBar::Style::circular);
        alertWindow->addTextBlock ("Press any button, or the escape key, to close the window");

        enum AlertWindowResult
        {
            noButtonPressed,
            button1Pressed,
            button2Pressed
        };

        alertWindow->addButton ("Button 1", AlertWindowResult::button1Pressed);
        alertWindow->addButton ("Button 2", AlertWindowResult::button2Pressed);

        RectanglePlacement placement { RectanglePlacement::yMid
                                       | RectanglePlacement::xLeft
                                       | RectanglePlacement::doNotResize };

        alertWindow->setBounds (placement.appliedTo (alertWindow->getBounds(), getDisplayArea()));

        alertWindowResult.setText ("", dontSendNotification);
        alertWindow->enterModalState (false, ModalCallbackFunction::create ([ref = SafePointer { this }] (int result)
        {
            if (ref == nullptr)
                return;

            const auto text = [&]
            {
                switch (result)
                {
                    case noButtonPressed:
                        return "Dismissed the Alert Window without pressing a button";
                    case button1Pressed:
                        return "Dismissed the Alert Window using Button 1";
                    case button2Pressed:
                        return "Dismissed the Alert Window using Button 2";
                }

                return "Unhandled event when dismissing the Alert Window";
            }();

            ref->alertWindowResult.setText (text, dontSendNotification);
        }), true);
    }

    class AlertWindowCustomComponent : public Component,
                                       private Slider::Listener
    {
    public:
        AlertWindowCustomComponent()
        {
            slider.setRange (0.0, 1.0);
            slider.setValue (0.5, NotificationType::dontSendNotification);
            slider.addListener (this);

            addAndMakeVisible (label);
            addAndMakeVisible (slider);

            setSize (200, 50);
        }

        ~AlertWindowCustomComponent() override
        {
            slider.removeListener (this);
        }

        void resized() override
        {
            auto bounds =  getLocalBounds();
            label.setBounds (bounds.removeFromTop (getHeight() / 2));
            slider.setBounds (bounds);
        }

        void sliderValueChanged (Slider*) override
        {
            value = slider.getValue();
        }

        double value { -1.0 };

    private:
        Label label { "Label", "Custom component" };
        Slider slider { Slider::SliderStyle::LinearHorizontal,
                        Slider::TextEntryBoxPosition::NoTextBox };
    };

    std::optional<AlertWindowCustomComponent> alertWindowCustomComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDemo)
};
