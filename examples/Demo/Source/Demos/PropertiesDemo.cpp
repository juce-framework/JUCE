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

#include "../JuceDemoHeader.h"


//==============================================================================
class DemoButtonPropertyComponent : public ButtonPropertyComponent
{
public:
    DemoButtonPropertyComponent (const String& propertyName)
        : ButtonPropertyComponent (propertyName, true),
          counter (0)
    {
        refresh();
    }

    void buttonClicked() override
    {
        ++counter;
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Action Button Pressed",
                                          "Pressing this type of property component can trigger an action such as showing an alert window!");
        refresh();
    }

    String getButtonText() const override
    {
        String text ("Button clicked ");
        text << counter << " times";
        return text;
    }

private:
    int counter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoButtonPropertyComponent)
};

//==============================================================================
class DemoSliderPropertyComponent : public SliderPropertyComponent
{
public:
    DemoSliderPropertyComponent (const String& propertyName)
        : SliderPropertyComponent (propertyName, 0.0, 100.0, 0.001)
    {
        setValue (Random::getSystemRandom().nextDouble() * 42.0);
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
    Array<PropertyComponent*> comps;

    comps.add (new TextPropertyComponent (Value (var ("This is a single-line Text Property")), "Text 1", 200, false));
    comps.add (new TextPropertyComponent (Value (var ("Another one")), "Text 2", 200, false));

    comps.add (new TextPropertyComponent (Value (var (
        "Lorem ipsum dolor sit amet, cu mei labore admodum facilisi. Iriure iuvaret invenire ea vim, cum quod"
        "si intellegat delicatissimi an. Cetero recteque ei eos, his an scripta fastidii placerat. Nec et anc"
        "illae nominati corrumpit. Vis dictas audire accumsan ad, elit fabulas saperet mel eu.\n"
        "\n"
        "Dicam utroque ius ne, eum choro phaedrum eu. Ut mel omnes virtute appareat, semper quodsi labitur in"
        " cum. Est aeque eripuit deleniti in, amet ferri recusabo ea nec. Cu persius maiorum corrumpit mei, i"
        "n ridens perpetua mea, pri nobis tation inermis an. Vis alii autem cotidieque ut, ius harum salutatu"
        "s ut. Mel eu purto veniam dissentias, malis doctus bonorum ne vel, mundi aperiam adversarium cu eum."
        " Mei quando graeci te, dolore accusata mei te.")),
                                          "Multi-line text",
                                          1000, true));

    return comps;
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

    for (int i = 0; i < howMany; ++i)
    {
        choices.add ("Item " + String (i));
        choiceVars.add (i);
    }

    for (int i = 0; i < howMany; ++i)
        comps.add (new ChoicePropertyComponent (Value (Random::getSystemRandom().nextInt (6)), "Choice Property " + String (i + 1), choices, choiceVars));

    return comps;
}

//==============================================================================
class PropertiesDemo   : public Component
{
public:
    PropertiesDemo()
    {
        setOpaque (true);
        addAndMakeVisible (propertyPanel);

        propertyPanel.addSection ("Text Editors", createTextEditors());
        propertyPanel.addSection ("Sliders", createSliders (3));
        propertyPanel.addSection ("Choice Properties", createChoices (6));
        propertyPanel.addSection ("Buttons & Toggles", createButtons (3));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour::greyLevel (0.8f));
    }

    void resized() override
    {
        propertyPanel.setBounds (getLocalBounds().reduced (4));
    }

private:
    PropertyPanel propertyPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertiesDemo)
};

//==============================================================================
class ConcertinaDemo   : public Component,
                         private Timer
{
public:
    ConcertinaDemo()
    {
        setOpaque (true);
        addAndMakeVisible (concertinaPanel);

        {
            PropertyPanel* panel = new PropertyPanel ("Text Editors");
            panel->addProperties (createTextEditors());
            addPanel (panel);
        }

        {
            PropertyPanel* panel = new PropertyPanel ("Sliders");
            panel->addSection ("Section 1", createSliders (4), true);
            panel->addSection ("Section 2", createSliders (3), true);
            addPanel (panel);
        }

        {
            PropertyPanel* panel = new PropertyPanel ("Choice Properties");
            panel->addProperties (createChoices (12));
            addPanel (panel);
        }

        {
            PropertyPanel* panel = new PropertyPanel ("Buttons & Toggles");
            panel->addProperties (createButtons (6));
            addPanel (panel);
        }

        startTimer (300);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour::greyLevel (0.8f));
    }

    void resized() override
    {
        concertinaPanel.setBounds (getLocalBounds().reduced (4));
    }

    void timerCallback() override
    {
        stopTimer();
        concertinaPanel.expandPanelFully (concertinaPanel.getPanel(0), true);
    }

private:
    ConcertinaPanel concertinaPanel;

    void addPanel (PropertyPanel* panel)
    {
        concertinaPanel.addPanel (-1, panel, true);
        concertinaPanel.setMaximumPanelSize (panel, panel->getTotalContentHeight());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType <PropertiesDemo> demo1 ("10 Components: Property Panels");
static JuceDemoType <ConcertinaDemo> demo2 ("10 Components: Concertina Panels");
