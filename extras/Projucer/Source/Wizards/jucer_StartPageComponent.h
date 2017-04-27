/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

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
