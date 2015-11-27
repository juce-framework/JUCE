/*
  ==============================================================================

    NoiseGateEditor.h
    Created: 23 Nov 2015 3:08:33pm
    Author:  Fabian Renn

  ==============================================================================
*/

#ifndef NOISEGATEEDITOR_H_INCLUDED
#define NOISEGATEEDITOR_H_INCLUDED


class NoiseGateEditor : public AudioProcessorEditor,
                        public SliderListener,
                        private Timer
{
public:
    enum
    {
        kParamSliderHeight = 40,
        kParamLabelWidth = 80,
        kParamSliderWidth = 300
    };

    NoiseGateEditor (AudioProcessor& parent)
        : AudioProcessorEditor (parent)
    {
        const OwnedArray<AudioProcessorParameter>& params = parent.getParameters();
        for (int i = 0; i < params.size(); ++i)
        {
            if (const AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*>(params[i]))
            {
                Slider* aSlider;

                paramSliders.add (aSlider = new Slider (param->name));
                aSlider->setRange (param->range.start, param->range.end);
                aSlider->setSliderStyle (Slider::LinearHorizontal);
                aSlider->setValue (dynamic_cast<const AudioProcessorParameter*>(param)->getValue());

                aSlider->addListener (this);
                addAndMakeVisible (aSlider);

                Label* aLabel;
                paramLabels.add (aLabel = new Label (param->name, param->name));
                addAndMakeVisible (aLabel);
            }
        }

        setSize (kParamSliderWidth + kParamLabelWidth, kParamSliderHeight * paramSliders.size());
        startTimer (100);
    }

    ~NoiseGateEditor()
    {
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();

        for (int i = 0; i < paramSliders.size(); ++i)
        {
            Rectangle<int> paramBounds = r.removeFromTop (kParamSliderHeight);
            Rectangle<int> labelBounds = paramBounds.removeFromLeft (kParamLabelWidth);

            paramLabels[i]->setBounds (labelBounds);
            paramSliders[i]->setBounds (paramBounds);
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void sliderValueChanged (Slider* slider) override
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();

        int paramIndex = paramSliders.indexOf (slider);
        if (paramIndex >= 0 && paramIndex < params.size())
            params[paramIndex]->setValueNotifyingHost ((float) slider->getValue());
    }

private:
    void timerCallback() override
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
        for (int i = 0; i < params.size(); ++i)
        {
            if (const AudioProcessorParameter* param = params[i])
            {
                if (i < paramSliders.size())
                    paramSliders[i]->setValue (param->getValue());
            }
        }
    }

    OwnedArray<Slider> paramSliders;
    OwnedArray<Label> paramLabels;
};


#endif  // NOISEGATEEDITOR_H_INCLUDED
