/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_GenericAudioProcessorEditor.h"
#include "juce_AudioProcessor.h"
#include "../../gui/components/controls/juce_Slider.h"


//==============================================================================
class ProcessorParameterPropertyComp   : public PropertyComponent,
                                         public AudioProcessorListener,
                                         public AsyncUpdater
{
public:
    ProcessorParameterPropertyComp (const String& name,
                                    AudioProcessor* const owner_,
                                    const int index_)
        : PropertyComponent (name),
          owner (owner_),
          index (index_)
    {
        addAndMakeVisible (slider = new ParamSlider (owner_, index_));
        owner_->addListener (this);
    }

    ~ProcessorParameterPropertyComp()
    {
        owner->removeListener (this);
        deleteAllChildren();
    }

    void refresh()
    {
        slider->setValue (owner->getParameter (index), false);
    }

    void audioProcessorChanged (AudioProcessor*)  {}

    void audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float)
    {
        if (parameterIndex == index)
            triggerAsyncUpdate();
    }

    void handleAsyncUpdate()
    {
        refresh();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioProcessor* const owner;
    const int index;
    Slider* slider;

    //==============================================================================
    class ParamSlider  : public Slider
    {
    public:
        ParamSlider (AudioProcessor* const owner_, const int index_)
            : Slider (String::empty),
              owner (owner_),
              index (index_)
        {
            setRange (0.0, 1.0, 0.0);
            setSliderStyle (Slider::LinearBar);
            setTextBoxIsEditable (false);
            setScrollWheelEnabled (false);
        }

        ~ParamSlider()
        {
        }

        void valueChanged()
        {
            const float newVal = (float) getValue();

            if (owner->getParameter (index) != newVal)
                owner->setParameter (index, newVal);
        }

        const String getTextFromValue (double /*value*/)
        {
            return owner->getParameterText (index);
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        AudioProcessor* const owner;
        const int index;

        ParamSlider (const ParamSlider&);
        const ParamSlider& operator= (const ParamSlider&);
    };

    ProcessorParameterPropertyComp (const ProcessorParameterPropertyComp&);
    const ProcessorParameterPropertyComp& operator= (const ProcessorParameterPropertyComp&);
};


//==============================================================================
GenericAudioProcessorEditor::GenericAudioProcessorEditor (AudioProcessor* const owner_)
    : AudioProcessorEditor (owner_)
{
    setOpaque (true);

    addAndMakeVisible (panel = new PropertyPanel());

    Array <PropertyComponent*> params;

    const int numParams = owner_->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        String name (owner_->getParameterName (i));
        if (name.trim().isEmpty())
            name = "Unnamed";

        ProcessorParameterPropertyComp* const pc = new ProcessorParameterPropertyComp (name, owner_, i);
        params.add (pc);
        totalHeight += pc->getPreferredHeight();
    }

    panel->addProperties (params);

    setSize (400, jlimit (25, 400, totalHeight));
}

GenericAudioProcessorEditor::~GenericAudioProcessorEditor()
{
    deleteAllChildren();
}

void GenericAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void GenericAudioProcessorEditor::resized()
{
    panel->setSize (getWidth(), getHeight());
}


END_JUCE_NAMESPACE
