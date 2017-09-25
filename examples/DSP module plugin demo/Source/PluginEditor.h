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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

struct ParameterSlider    : public Slider,
                            public Timer
{
    ParameterSlider (AudioProcessorParameter& p)
        : Slider (p.getName (256)), param (p)
    {
        setRange (0.0, 1.0, 0.0);
        startTimerHz (30);
        updateSliderPos();
    }

    void valueChanged() override
    {
        if (isMouseButtonDown())
            param.setValueNotifyingHost ((float) Slider::getValue());
        else
            param.setValue ((float) Slider::getValue());
    }

    void timerCallback() override       { updateSliderPos(); }

    void startedDragging() override     { param.beginChangeGesture(); }
    void stoppedDragging() override     { param.endChangeGesture();   }

    double getValueFromText (const String& text) override   { return param.getValueForText (text); }
    String getTextFromValue (double value) override         { return param.getText ((float) value, 1024) + " " + param.getLabel(); }

    void updateSliderPos()
    {
        const float newValue = param.getValue();

        if (newValue != (float) Slider::getValue() && ! isMouseButtonDown())
            Slider::setValue (newValue);
    }

    AudioProcessorParameter& param;
};

//==============================================================================
/**
    This is the editor component that will be displayed.
*/
class DspModulePluginDemoAudioProcessorEditor  : public AudioProcessorEditor,
                                                 private ComboBox::Listener,
                                                 private Button::Listener
{
public:
    DspModulePluginDemoAudioProcessorEditor (DspModulePluginDemoAudioProcessor&);
    ~DspModulePluginDemoAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void comboBoxChanged (ComboBox*) override;
    void buttonClicked (Button*) override;

    //==============================================================================
    DspModulePluginDemoAudioProcessor& processor;

    ScopedPointer<ParameterSlider> inputVolumeSlider, outputVolumeSlider,
                                   lowPassFilterFreqSlider, highPassFilterFreqSlider;
    ComboBox stereoBox, slopeBox, waveshaperBox, cabinetTypeBox;
    ToggleButton cabinetSimButton, oversamplingButton;

    Label inputVolumeLabel, outputVolumeLabel, lowPassFilterFreqLabel,
          highPassFilterFreqLabel, stereoLabel, slopeLabel, waveshaperLabel,
          cabinetTypeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoAudioProcessorEditor)
};
