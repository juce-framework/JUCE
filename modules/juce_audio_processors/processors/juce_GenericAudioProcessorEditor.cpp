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

namespace juce
{

class ProcessorParameterPropertyComp   : public PropertyComponent,
                                         private AudioProcessorListener,
                                         private Timer
{
public:
    ProcessorParameterPropertyComp (const String& name, AudioProcessor& p, int paramIndex)
        : PropertyComponent (name),
          owner (p),
          index (paramIndex),
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
            auto steps = owner.getParameterNumSteps (index);
            auto category = p.getParameterCategory (index);
            bool isLevelMeter = (((category & 0xffff0000) >> 16) == 2);

            if (steps > 1 && steps < 0x7fffffff)
                setRange (0.0, 1.0, 1.0 / (steps - 1.0));
            else
                setRange (0.0, 1.0);

            setEnabled (! isLevelMeter);
            setSliderStyle (Slider::LinearBar);
            setTextBoxIsEditable (false);
            setScrollWheelEnabled (true);
        }

        void valueChanged() override
        {
            auto newVal = static_cast<float> (getValue());

            if (owner.getParameter (index) != newVal)
            {
                owner.setParameterNotifyingHost (index, newVal);
                updateText();
            }
        }

        void startedDragging() override
        {
            owner.beginParameterChangeGesture(index);
        }

        void stoppedDragging() override
        {
            owner.endParameterChangeGesture(index);
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
    bool volatile paramHasChanged = false;
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

    Array<PropertyComponent*> params;

    auto numParams = p->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        auto name = p->getParameterName (i);

        if (name.trim().isEmpty())
            name = "Unnamed";

        auto* pc = new ProcessorParameterPropertyComp (name, *p, i);
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

} // namespace juce
