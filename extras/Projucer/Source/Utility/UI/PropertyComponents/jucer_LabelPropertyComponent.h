/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class LabelPropertyComponent  : public PropertyComponent
{
public:
    LabelPropertyComponent (const String& labelText, int propertyHeight = 25,
                            Font labelFont = Font (16.0f, Font::bold),
                            Justification labelJustification = Justification::centred)
        : PropertyComponent (labelText),
          labelToDisplay ({}, labelText)
    {
        setPreferredHeight (propertyHeight);

        labelToDisplay.setJustificationType (labelJustification);
        labelToDisplay.setFont (labelFont);

        addAndMakeVisible (labelToDisplay);
        setLookAndFeel (&lf);
    }

    ~LabelPropertyComponent() override    { setLookAndFeel (nullptr); }

    //==============================================================================
    void refresh() override {}

    void resized() override
    {
        labelToDisplay.setBounds (getLocalBounds());
    }

private:
    //==============================================================================
    struct LabelLookAndFeel : public ProjucerLookAndFeel
    {
        void drawPropertyComponentLabel (Graphics&, int, int, PropertyComponent&) {}
    };

    void lookAndFeelChanged() override
    {
        labelToDisplay.setColour (Label::textColourId, ProjucerApplication::getApp().lookAndFeel.findColour (defaultTextColourId));
    }

    //==============================================================================
    LabelLookAndFeel lf;
    Label labelToDisplay;
};
