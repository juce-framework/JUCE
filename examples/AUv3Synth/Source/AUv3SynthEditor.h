/*
  ==============================================================================

    Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/
#include "MaterialLookAndFeel.h"

//==============================================================================
class AUv3SynthEditor   : public AudioProcessorEditor,
                           public ButtonListener,
                           public Slider::Listener,
                           private Timer
{
public:
    //==============================================================================
    AUv3SynthEditor (AudioProcessor& processor)
        :   AudioProcessorEditor (processor),
            recordButton ("Record"),
            roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        LookAndFeel::setDefaultLookAndFeel (&materialLookAndFeel);

        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);

        recordButton.addListener (this);
        addAndMakeVisible (recordButton);

        roomSizeSlider.addListener (this);
        roomSizeSlider.setRange (0.0, 1.0);
        addAndMakeVisible (roomSizeSlider);

        Path proAudioPath;
        proAudioPath.loadPathFromData (BinaryData::proaudio_path, BinaryData::proaudio_pathSize);
        proAudioIcon.setPath (proAudioPath);
        addAndMakeVisible (proAudioIcon);

        Colour proAudioIconColour = findColour (TextButton::buttonOnColourId);
        proAudioIcon.setFill (FillType (proAudioIconColour));

        setSize (600, 400);
        startTimer (100);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();

        int guiElementAreaHeight = r.getHeight() / 3;

        proAudioIcon.setTransformToFit (r.removeFromLeft (proportionOfWidth (0.25))
                                        .withSizeKeepingCentre (guiElementAreaHeight, guiElementAreaHeight)
                                        .toFloat(),
                                        RectanglePlacement::fillDestination);

        int margin = guiElementAreaHeight / 4;
        r.reduce (margin, margin);

        int buttonHeight = guiElementAreaHeight - margin;

        recordButton.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
        roomSizeSlider.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
    }

    //==============================================================================
    void buttonClicked (Button* button) override
    {
        if (button == &recordButton)
        {
            recordButton.setEnabled (false);
            setParameterValue ("isRecording", 1.0f);
        }
    }

    void sliderValueChanged (Slider*) override
    {
        setParameterValue ("roomSize", roomSizeSlider.getValue());
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        bool isRecordingNow = (getParameterValue ("isRecording") >= 0.5f);

        recordButton.setEnabled (! isRecordingNow);
        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);
    }

    //==============================================================================
    AudioProcessorParameter* getParameter (const String& paramId)
    {
        if (AudioProcessor* processor = getAudioProcessor())
        {
            const OwnedArray<AudioProcessorParameter>& params = processor->getParameters();

            for (int i = 0; i < params.size(); ++i)
            {
                if (AudioProcessorParameterWithID* param = dynamic_cast<AudioProcessorParameterWithID*> (params[i]))
                {
                    if (param->paramID == paramId)
                        return param;
                }
            }
        }

        return nullptr;
    }

    //==============================================================================
    float getParameterValue (const String& paramId)
    {
        if (AudioProcessorParameter* param = getParameter (paramId))
            return param->getValue();

        return 0.0f;
    }

    void setParameterValue (const String& paramId, float value)
    {
        if (AudioProcessorParameter* param = getParameter (paramId))
            param->setValueNotifyingHost (value);
    }

    //==============================================================================
    MaterialLookAndFeel materialLookAndFeel;

    //==============================================================================
    TextButton recordButton;
    Slider roomSizeSlider;
    DrawablePath proAudioIcon;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthEditor)
};
