/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

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

 name:             KeyMappingsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases key mapping features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        KeyMappingsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

/** A list of the command IDs that this demo can perform. */
enum KeyPressCommandIDs
{
    buttonMoveUp = 1,
    buttonMoveRight,
    buttonMoveDown,
    buttonMoveLeft,
    nextButtonColour,
    previousButtonColour,
    nextBackgroundColour,
    previousBackgroundColour
};

//==============================================================================
/**
    This is a simple target for the key-presses which will live inside the demo component
    and contains a button that can be moved around with the arrow keys.
*/
class KeyPressTarget : public Component,
                       public ApplicationCommandTarget
{
public:
    KeyPressTarget()
    {
        Array<Colour> coloursToUse { Colours::darkblue, Colours::darkgrey, Colours::red,
                                     Colours::green, Colours::blue, Colours::hotpink };
        colours.addArray (coloursToUse);

        addAndMakeVisible (button);
    }

    //==============================================================================
    void resized() override
    {
        auto bounds = getLocalBounds();

        // keep the button on-screen
        if (buttonX < -150 || buttonX > bounds.getWidth()
            || buttonY < -30 || buttonY > bounds.getHeight())
        {
            buttonX = bounds.getCentreX() - 75;
            buttonY = bounds.getCentreY() - 15;
        }

        button.setBounds (buttonX, buttonY, 150, 30);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (colours.getUnchecked (backgroundColourIndex));
    }

    //==============================================================================
    /** No other command targets in this simple example so just return nullptr. */
    ApplicationCommandTarget* getNextCommandTarget() override   { return nullptr; }

    void getAllCommands (Array<CommandID>& commands) override
    {
        Array<CommandID> ids { KeyPressCommandIDs::buttonMoveUp, KeyPressCommandIDs::buttonMoveRight,
                               KeyPressCommandIDs::buttonMoveDown, KeyPressCommandIDs::buttonMoveLeft,
                               KeyPressCommandIDs::nextButtonColour, KeyPressCommandIDs::previousButtonColour,
                               KeyPressCommandIDs::nextBackgroundColour, KeyPressCommandIDs::previousBackgroundColour };

        commands.addArray (ids);
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case KeyPressCommandIDs::buttonMoveUp:
                result.setInfo ("Move up", "Move the button up", "Button", 0);
                result.addDefaultKeypress (KeyPress::upKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveRight:
                result.setInfo ("Move right", "Move the button right", "Button", 0);
                result.addDefaultKeypress (KeyPress::rightKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveDown:
                result.setInfo ("Move down", "Move the button down", "Button", 0);
                result.addDefaultKeypress (KeyPress::downKey, 0);
                break;
            case KeyPressCommandIDs::buttonMoveLeft:
                result.setInfo ("Move left", "Move the button left", "Button", 0);
                result.addDefaultKeypress (KeyPress::leftKey, 0);
                break;
            case KeyPressCommandIDs::nextButtonColour:
                result.setInfo ("Next colour", "Change the colour of the button to the next in the list", "Button", 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::shiftModifier);
                break;
            case KeyPressCommandIDs::previousButtonColour:
                result.setInfo ("Previous colour", "Change the colour of the button to the previous in the list", "Button", 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::shiftModifier);
                break;
            case KeyPressCommandIDs::nextBackgroundColour:
                result.setInfo ("Next colour", "Change the colour of the background to the next in the list", "Other", 0);
                result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::commandModifier);
                break;
            case KeyPressCommandIDs::previousBackgroundColour:
                result.setInfo ("Previous colour", "Change the colour of the background to the previous in the list", "Other", 0);
                result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::commandModifier);
                break;
            default:
                break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case KeyPressCommandIDs::buttonMoveUp:
                buttonY -= 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveRight:
                buttonX += 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveDown:
                buttonY += 5;
                resized();
                break;
            case KeyPressCommandIDs::buttonMoveLeft:
                buttonX -= 5;
                resized();
                break;
            case KeyPressCommandIDs::nextButtonColour:
                ++buttonColourIndex %= colours.size();
                button.setColour (TextButton::buttonColourId, colours.getUnchecked (buttonColourIndex));
                break;
            case KeyPressCommandIDs::previousButtonColour:
                --buttonColourIndex;
                if (buttonColourIndex < 0)
                    buttonColourIndex = colours.size() - 1;
                button.setColour (TextButton::buttonColourId, colours.getUnchecked (buttonColourIndex));
                break;
            case KeyPressCommandIDs::nextBackgroundColour:
                ++backgroundColourIndex %= colours.size();
                repaint();
                break;
            case KeyPressCommandIDs::previousBackgroundColour:
                --backgroundColourIndex;
                if (backgroundColourIndex < 0)
                    backgroundColourIndex = colours.size() - 1;
                repaint();
                break;
            default:
                return false;
        }

        return true;
    }

private:
    TextButton button;
    int buttonX = -200, buttonY = -200;

    Array<Colour> colours;

    int buttonColourIndex     = 0;
    int backgroundColourIndex = 1;
};

//==============================================================================
class KeyMappingsDemo   : public Component
{
public:
    KeyMappingsDemo()
    {
        // register the commands that the target component can perform
        commandManager.registerAllCommandsForTarget (&keyTarget);

        setOpaque (true);
        addAndMakeVisible (keyMappingEditor);
        addAndMakeVisible (keyTarget);

        // add command manager key mappings as a KeyListener to the top-level component
        // so it is notified of key presses
        getTopLevelComponent()->addKeyListener (commandManager.getKeyMappings());

        setSize (500, 500);

        Timer::callAfterDelay (300, [this] { keyTarget.grabKeyboardFocus(); }); // ensure that key presses are sent to the KeyPressTarget object
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.93f)));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        keyTarget       .setBounds (bounds.removeFromTop (bounds.getHeight() / 2).reduced (4));
        keyMappingEditor.setBounds (bounds.reduced (4));
    }

private:
   #if JUCE_DEMO_RUNNER
    ApplicationCommandManager& commandManager = getGlobalCommandManager();
   #else
    ApplicationCommandManager commandManager;
   #endif

    KeyMappingEditorComponent keyMappingEditor  { *commandManager.getKeyMappings(), true};

    KeyPressTarget keyTarget;

    void lookAndFeelChanged() override
    {
        auto* lf = &LookAndFeel::getDefaultLookAndFeel();

        keyMappingEditor.setColours (lf->findColour (KeyMappingEditorComponent::backgroundColourId),
                                     lf->findColour (KeyMappingEditorComponent::textColourId));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingsDemo)
};
