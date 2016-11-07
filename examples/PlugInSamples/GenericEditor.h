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

class GenericEditor : public AudioProcessorEditor,
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

    GenericEditor (AudioProcessor& parent)
        : AudioProcessorEditor (parent),
          noParameterLabel ("noparam", "No parameters available")
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

        noParameterLabel.setJustificationType (Justification::horizontallyCentred | Justification::verticallyCentred);
        noParameterLabel.setFont (noParameterLabel.getFont().withStyle (Font::italic));

        setSize (kParamSliderWidth + kParamLabelWidth,
                 jmax (1, kParamSliderHeight * paramSliders.size()));

        if (paramSliders.size() == 0)
            addAndMakeVisible (noParameterLabel);
        else
            startTimer (100);
    }

    ~GenericEditor()
    {
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();
        noParameterLabel.setBounds (r);

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

    //==============================================================================
    void sliderValueChanged (Slider* slider) override
    {
        if (AudioProcessorParameter* param = getParameterForSlider (slider))
        {
            if (slider->isMouseButtonDown())
                param->setValueNotifyingHost ((float) slider->getValue());
            else
                param->setValue ((float) slider->getValue());
        }
    }

    void sliderDragStarted (Slider* slider) override
    {
        if (AudioProcessorParameter* param = getParameterForSlider (slider))
            param->beginChangeGesture();
    }

    void sliderDragEnded (Slider* slider) override
    {
        if (AudioProcessorParameter* param = getParameterForSlider (slider))
            param->endChangeGesture();
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

    AudioProcessorParameter* getParameterForSlider (Slider* slider)
    {
        const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
        return params[paramSliders.indexOf (slider)];
    }

    Label noParameterLabel;
    OwnedArray<Slider> paramSliders;
    OwnedArray<Label> paramLabels;
};
