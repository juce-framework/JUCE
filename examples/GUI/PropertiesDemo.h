/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PropertiesDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays various property components.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        PropertiesDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class DemoButtonPropertyComponent final : public ButtonPropertyComponent
{
public:
    DemoButtonPropertyComponent (const String& propertyName)
        : ButtonPropertyComponent (propertyName, true)
    {
        refresh();
    }

    void buttonClicked() override
    {
        ++counter;
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         "Action Button Pressed",
                                                         "Pressing this type of property component can trigger an action such as showing an alert window!");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
        refresh();
    }

    String getButtonText() const override
    {
        return "Button clicked " + String (counter) + " times";
    }

private:
    int counter = 0;
    ScopedMessageBox messageBox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoButtonPropertyComponent)
};

//==============================================================================
class DemoSliderPropertyComponent final : public SliderPropertyComponent
{
public:
    DemoSliderPropertyComponent (const String& propertyName)
        : SliderPropertyComponent (propertyName, 0.0, 100.0, 0.001)
    {
        slider.setValue (Random::getSystemRandom().nextDouble() * 42.0);
    }

    void setValue (double newValue) override
    {
        slider.setValue (newValue);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoSliderPropertyComponent)
};

//==============================================================================
static Array<PropertyComponent*> createTextEditors()
{
    return { new TextPropertyComponent (Value (var ("This is a single-line Text Property")), "Text 1", 200, false),
             new TextPropertyComponent (Value (var ("Another one")), "Text 2", 200, false),
             new TextPropertyComponent (Value (var ( "Lorem ipsum dolor sit amet, cu mei labore admodum facilisi. Iriure iuvaret invenire ea vim, cum quod"
                                                     "si intellegat delicatissimi an. Cetero recteque ei eos, his an scripta fastidii placerat. Nec et anc"
                                                     "illae nominati corrumpit. Vis dictas audire accumsan ad, elit fabulas saperet mel eu.\n"
                                                     "\n"
                                                     "Dicam utroque ius ne, eum choro phaedrum eu. Ut mel omnes virtute appareat, semper quodsi labitur in"
                                                     " cum. Est aeque eripuit deleniti in, amet ferri recusabo ea nec. Cu persius maiorum corrumpit mei, i"
                                                     "n ridens perpetua mea, pri nobis tation inermis an. Vis alii autem cotidieque ut, ius harum salutatu"
                                                     "s ut. Mel eu purto veniam dissentias, malis doctus bonorum ne vel, mundi aperiam adversarium cu eum."
                                                     " Mei quando graeci te, dolore accusata mei te.")),
                                        "Multi-line text",
                                        1000, true) };
}

static Array<PropertyComponent*> createSliders (int howMany)
{
    Array<PropertyComponent*> comps;

    for (int i = 0; i < howMany; ++i)
        comps.add (new DemoSliderPropertyComponent ("Slider " + String (i + 1)));

    return comps;
}

static Array<PropertyComponent*> createButtons (int howMany)
{
    Array<PropertyComponent*> comps;

    for (int i = 0; i < howMany; ++i)
        comps.add (new DemoButtonPropertyComponent ("Button " + String (i + 1)));

    for (int i = 0; i < howMany; ++i)
        comps.add (new BooleanPropertyComponent (Value (Random::getSystemRandom().nextBool()), "Toggle " + String (i + 1), "Description of toggleable thing"));

    return comps;
}

static Array<PropertyComponent*> createChoices (int howMany)
{
    Array<PropertyComponent*> comps;

    StringArray choices;
    Array<var> choiceVars;

    for (int i = 0; i < 12; ++i)
    {
        choices.add ("Item " + String (i));
        choiceVars.add (i);
    }

    for (int i = 0; i < howMany; ++i)
        comps.add (new ChoicePropertyComponent (Value (Random::getSystemRandom().nextInt (12)), "Choice Property " + String (i + 1), choices, choiceVars));

    for (int i = 0; i < howMany; ++i)
        comps.add (new MultiChoicePropertyComponent (Value (Array<var>()), "Multi-Choice Property " + String (i + 1), choices, choiceVars));

    return comps;
}

//==============================================================================
class PropertiesDemo final : public Component,
                             private Timer
{
public:
    PropertiesDemo()
    {
        setOpaque (true);
        addAndMakeVisible (concertinaPanel);

        {
            auto* panel = new PropertyPanel ("Text Editors");
            panel->addProperties (createTextEditors());
            addPanel (panel);
        }

        {
            auto* panel = new PropertyPanel ("Sliders");
            panel->addSection ("Section 1", createSliders (4), true);
            panel->addSection ("Section 2", createSliders (3), true);
            addPanel (panel);
        }

        {
            auto* panel = new PropertyPanel ("Choice Properties");
            panel->addProperties (createChoices (3));
            addPanel (panel);
        }

        {
            auto* panel = new PropertyPanel ("Buttons & Toggles");
            panel->addProperties (createButtons (6));
            addPanel (panel);
        }


        setSize (750, 650);
        startTimer (300);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.8f)));
    }

    void resized() override
    {
        concertinaPanel.setBounds (getLocalBounds().reduced (4));
    }

    void timerCallback() override
    {
        stopTimer();
        concertinaPanel.expandPanelFully (concertinaPanel.getPanel (0), true);
    }

private:
    ConcertinaPanel concertinaPanel;

    void addPanel (PropertyPanel* panel)
    {
        concertinaPanel.addPanel (-1, panel, true);
        concertinaPanel.setMaximumPanelSize (panel, panel->getTotalContentHeight());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertiesDemo)
};
