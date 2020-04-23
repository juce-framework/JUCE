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
class StartPageComponent    : public Component
{
public:
    StartPageComponent()
    {
        setSize (900, 650);

        WizardComp* projectWizard = new WizardComp();

        panel.addTab ("Create New Project", new TemplateTileBrowser (projectWizard), true);
        panel.addTab ("New Project Options", projectWizard, true);

        addAndMakeVisible (panel);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

    void resized() override
    {
        panel.setBounds (getLocalBounds());
    }

private:
    SlidingPanelComponent panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartPageComponent)
};
