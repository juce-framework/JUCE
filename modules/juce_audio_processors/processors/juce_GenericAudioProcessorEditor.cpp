/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

class ProcessorParameterPropertyComp   : public PropertyComponent,
                                         private AudioProcessorListener,
                                         private Timer
{
public:
    ProcessorParameterPropertyComp (const String& name, AudioProcessor& owner_, const int index_)
        : PropertyComponent (name),
          owner (owner_),
          index (index_),
          paramHasChanged (false),
          slider (owner_, index_)
    {
        startTimer (100);
        addAndMakeVisible (&slider);
        owner_.addListener (this);
    }

    ~ProcessorParameterPropertyComp()
    {
        owner.removeListener (this);
    }

    void refresh()
    {
        paramHasChanged = false;
        slider.setValue (owner.getParameter (index), dontSendNotification);
    }

    void audioProcessorChanged (AudioProcessor*)  {}

    void audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float)
    {
        if (parameterIndex == index)
            paramHasChanged = true;
    }

    void timerCallback()
    {
        if (paramHasChanged)
        {
            refresh();
            startTimer (1000 / 50);
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
        ParamSlider (AudioProcessor& owner_, const int index_)
            : owner (owner_),
              index (index_)
        {
            setRange (0.0, 1.0, 0.0);
            setSliderStyle (Slider::LinearBar);
            setTextBoxIsEditable (false);
            setScrollWheelEnabled (false);
        }

        void valueChanged()
        {
            const float newVal = (float) getValue();

            if (owner.getParameter (index) != newVal)
                owner.setParameterNotifyingHost (index, newVal);
        }

        String getTextFromValue (double /*value*/)
        {
            return owner.getParameterText (index);
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
GenericAudioProcessorEditor::GenericAudioProcessorEditor (AudioProcessor* const owner_)
    : AudioProcessorEditor (owner_)
{
    jassert (owner_ != nullptr);
    setOpaque (true);

    addAndMakeVisible (&panel);

    Array <PropertyComponent*> params;

    const int numParams = owner_->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        String name (owner_->getParameterName (i));
        if (name.trim().isEmpty())
            name = "Unnamed";

        ProcessorParameterPropertyComp* const pc = new ProcessorParameterPropertyComp (name, *owner_, i);
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
