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

class GenericEditor : public AudioProcessorEditor,
                      public Slider::Listener,
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
                const bool isLevelMeter = (((param->category & 0xffff0000) >> 16) == 2);
                if (isLevelMeter)
                    continue;

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
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
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
