/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class ProcessorParameterPropertyComp   : public PropertyComponent,
                                         private AudioProcessorListener,
                                         private Timer
{
public:
    ProcessorParameterPropertyComp (const String& name, AudioProcessor& p, int paramIndex)
        : PropertyComponent (name),
          owner (p),
          index (paramIndex),
          paramHasChanged (false),
          slider (p, paramIndex)
    {
        startTimer (100);
        addAndMakeVisible (slider);
        owner.addListener (this);
    }

    ~ProcessorParameterPropertyComp()
    {
        owner.removeListener (this);
    }

    void refresh() override
    {
        paramHasChanged = false;

        if (slider.getThumbBeingDragged() < 0)
            slider.setValue (owner.getParameter (index), dontSendNotification);

        slider.updateText();
    }

    void audioProcessorChanged (AudioProcessor*) override  {}

    void audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float) override
    {
        if (parameterIndex == index)
            paramHasChanged = true;
    }

    void timerCallback() override
    {
        if (paramHasChanged)
        {
            refresh();
            startTimerHz (50);
        }
        else
        {
            startTimer (jmin (1000 / 4, getTimerInterval() + 10));
        }
    }

private:
    //==============================================================================
    class ParamSlider  : public Slider
    {
    public:
        ParamSlider (AudioProcessor& p, int paramIndex)  : owner (p), index (paramIndex)
        {
            const int steps = owner.getParameterNumSteps (index);

            if (steps > 1 && steps < 0x7fffffff)
                setRange (0.0, 1.0, 1.0 / (steps - 1.0));
            else
                setRange (0.0, 1.0);

            setSliderStyle (Slider::LinearBar);
            setTextBoxIsEditable (false);
            setScrollWheelEnabled (true);
        }

        void valueChanged() override
        {
            const float newVal = (float) getValue();

            if (owner.getParameter (index) != newVal)
            {
                owner.setParameterNotifyingHost (index, newVal);
                updateText();
            }
        }

        String getTextFromValue (double /*value*/) override
        {
            return owner.getParameterText (index) + " " + owner.getParameterLabel (index).trimEnd();
        }

    private:
        //==============================================================================
        AudioProcessor& owner;
        const int index;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamSlider)
    };

    AudioProcessor& owner;
    const int index;
    bool volatile paramHasChanged;
    ParamSlider slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorParameterPropertyComp)
};


//==============================================================================
GenericAudioProcessorEditor::GenericAudioProcessorEditor (AudioProcessor* const p)
    : AudioProcessorEditor (p)
{
    jassert (p != nullptr);
    setOpaque (true);

    addAndMakeVisible (panel);

    Array <PropertyComponent*> params;

    const int numParams = p->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        String name (p->getParameterName (i));
        if (name.trim().isEmpty())
            name = "Unnamed";

        ProcessorParameterPropertyComp* const pc = new ProcessorParameterPropertyComp (name, *p, i);
        params.add (pc);
        totalHeight += pc->getPreferredHeight();
    }

    panel.addProperties (params);

    setSize (400, jlimit (25, 400, totalHeight));
}

GenericAudioProcessorEditor::~GenericAudioProcessorEditor()
{
}

void GenericAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void GenericAudioProcessorEditor::resized()
{
    panel.setBounds (getLocalBounds());
}
