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

#include "../JuceDemoHeader.h"


//==============================================================================
class KeyMappingsDemo   : public Component
{
public:
    KeyMappingsDemo()
        : keyMappingEditor (*MainAppWindow::getApplicationCommandManager().getKeyMappings(), true)
    {
        setOpaque (true);
        addAndMakeVisible (keyMappingEditor);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.93f)));
    }

    void resized() override
    {
        keyMappingEditor.setBounds (getLocalBounds().reduced (4));
    }

private:
    KeyMappingEditorComponent keyMappingEditor;

    void lookAndFeelChanged() override
    {
        auto* lf = &LookAndFeel::getDefaultLookAndFeel();
        keyMappingEditor.setColours (lf->findColour (KeyMappingEditorComponent::backgroundColourId),
                                     lf->findColour (KeyMappingEditorComponent::textColourId));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappingsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<KeyMappingsDemo> demo ("01 Shortcut Keys");
