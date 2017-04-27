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
#include "IAAEffectProcessor.h"
#include "SimpleMeter.h"


class IAAEffectEditor  : public AudioProcessorEditor,
                         private IAAEffectProcessor::MeterListener,
                         private ButtonListener,
                         private Timer
{
public:
    IAAEffectEditor (IAAEffectProcessor& p,
                     AudioProcessorValueTreeState& vts)
        : AudioProcessorEditor (p),
          processor (p),
          parameters (vts)
    {
        // Register for meter value updates.
        processor.addMeterListener (*this);

        gainSlider.setSliderStyle (Slider::SliderStyle::LinearVertical);
        gainSlider.setTextBoxStyle (Slider::TextEntryBoxPosition::TextBoxAbove, false, 60, 20);
        addAndMakeVisible (gainSlider);

        for (auto& meter : meters)
            addAndMakeVisible (meter);

        // Configure all the graphics for the transport control.

        transportText.setFont (Font (Font::getDefaultMonospacedFontName(), 18.0f, Font::plain));
        transportText.setJustificationType (Justification::topLeft);
        addChildComponent (transportText);

        Path rewindShape;
        rewindShape.addRectangle (0.0, 0.0, 5.0, buttonSize);
        rewindShape.addTriangle (0.0, buttonSize / 2, buttonSize, 0.0, buttonSize, buttonSize);
        rewindButton.setShape (rewindShape, true, true, false);
        rewindButton.addListener (this);
        addChildComponent (rewindButton);

        Path playShape;
        playShape.addTriangle (0.0, 0.0, 0.0, buttonSize, buttonSize, buttonSize / 2);
        playButton.setShape (playShape, true, true, false);
        playButton.addListener (this);
        addChildComponent (playButton);

        Path recordShape;
        recordShape.addEllipse (0.0, 0.0, buttonSize, buttonSize);
        recordButton.setShape (recordShape, true, true, false);
        recordButton.addListener (this);
        addChildComponent (recordButton);

        // Configure the switch to host button.

        switchToHostButtonLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 18.0f, Font::plain));
        switchToHostButtonLabel.setJustificationType (Justification::centredRight);
        switchToHostButtonLabel.setText ("Switch to\nhost app:", dontSendNotification);
        addChildComponent (switchToHostButtonLabel);

        switchToHostButton.addListener (this);
        addChildComponent (switchToHostButton);

        Rectangle<int> screenSize = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
        setSize (screenSize.getWidth(), screenSize.getHeight());

        resized();

        startTimerHz (60);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto area = getBounds().reduced (20);

        gainSlider.setBounds (area.removeFromLeft (60));

        for (auto& meter : meters)
        {
            area.removeFromLeft (10);
            meter.setBounds (area.removeFromLeft (20));
        }

        area.removeFromLeft (20);
        transportText.setBounds (area.removeFromTop (120));

        auto navigationArea = area.removeFromTop (buttonSize);
        rewindButton.setTopLeftPosition (navigationArea.getPosition());
        navigationArea.removeFromLeft (buttonSize + 10);
        playButton.setTopLeftPosition (navigationArea.getPosition());
        navigationArea.removeFromLeft (buttonSize + 10);
        recordButton.setTopLeftPosition (navigationArea.getPosition());

        area.removeFromTop (30);

        auto appSwitchArea = area.removeFromTop (buttonSize);
        switchToHostButtonLabel.setBounds (appSwitchArea.removeFromLeft (100));
        appSwitchArea.removeFromLeft (5);
        switchToHostButton.setBounds (appSwitchArea.removeFromLeft (buttonSize));
    }

private:
    //==============================================================================
    // Called from the audio thread.
    void handleNewMeterValue (int channel, float value) override
    {
        meters[(size_t) channel].update (value);
    }

    //==============================================================================
    void timerCallback () override
    {
        auto timeInfoSuccess = processor.updateCurrentTimeInfoFromHost (lastPosInfo);
        transportText.setVisible (timeInfoSuccess);
        if (timeInfoSuccess)
            updateTransportTextDisplay();

        updateTransportButtonsDisplay();

        updateSwitchToHostDisplay();
    }

    //==============================================================================
    void buttonClicked (Button* b) override
    {
        auto playHead = processor.getPlayHead();
        if (playHead != nullptr && playHead->canControlTransport())
        {
            if (b == &rewindButton)
            {
                playHead->transportRewind();
            }
            else if (b == &playButton)
            {
                playHead->transportPlay(! lastPosInfo.isPlaying);
            }
            else if (b == &recordButton)
            {
                playHead->transportRecord (! lastPosInfo.isRecording);
            }
            else if (b == &switchToHostButton)
            {
                PluginHostType hostType;

                hostType.switchToHostApplication();
            }
        }
    }

    //==============================================================================
    // quick-and-dirty function to format a timecode string
    String timeToTimecodeString (double seconds)
    {
        auto millisecs = roundToInt (seconds * 1000.0);
        auto absMillisecs = std::abs (millisecs);

        return String::formatted ("%02d:%02d:%02d.%03d",
                                  millisecs / 360000,
                                  (absMillisecs / 60000) % 60,
                                  (absMillisecs / 1000) % 60,
                                  absMillisecs % 1000);
    }

    // A quick-and-dirty function to format a bars/beats string.
    String quarterNotePositionToBarsBeatsString (double quarterNotes, int numerator, int denominator)
    {
        if (numerator == 0 || denominator == 0)
            return "1|1|000";

        auto quarterNotesPerBar = (numerator * 4 / denominator);
        auto beats  = (fmod (quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * numerator;

        auto bar    = ((int) quarterNotes) / quarterNotesPerBar + 1;
        auto beat   = ((int) beats) + 1;
        auto ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

        return String::formatted ("%d|%d|%03d", bar, beat, ticks);
    }

    void updateTransportTextDisplay()
    {
        MemoryOutputStream displayText;

        displayText << "[" << SystemStats::getJUCEVersion() << "]\n"
        << String (lastPosInfo.bpm, 2) << " bpm\n"
        << lastPosInfo.timeSigNumerator << '/' << lastPosInfo.timeSigDenominator << "\n"
        << timeToTimecodeString (lastPosInfo.timeInSeconds) << "\n"
        << quarterNotePositionToBarsBeatsString (lastPosInfo.ppqPosition,
                                                 lastPosInfo.timeSigNumerator,
                                                 lastPosInfo.timeSigDenominator) << "\n";

        if (lastPosInfo.isRecording)
            displayText << "(recording)";
        else if (lastPosInfo.isPlaying)
            displayText << "(playing)";

        transportText.setText (displayText.toString(), dontSendNotification);
    }

    void updateTransportButtonsDisplay()
    {
        auto visible = processor.getPlayHead() != nullptr
                    && processor.getPlayHead()->canControlTransport();

        if (rewindButton.isVisible() != visible)
        {
            rewindButton.setVisible (visible);
            playButton.setVisible   (visible);
            recordButton.setVisible (visible);
        }

        if (visible)
        {
            Colour playColour = lastPosInfo.isPlaying ? Colours::green : defaultButtonColour;
            playButton.setColours (playColour, playColour, playColour);
            playButton.repaint();

            Colour recordColour = lastPosInfo.isRecording ? Colours::red : defaultButtonColour;
            recordButton.setColours (recordColour, recordColour, recordColour);
            recordButton.repaint();
        }
    }

    void updateSwitchToHostDisplay()
    {
        PluginHostType hostType;
        const bool visible = hostType.isInterAppAudioConnected();

        if (switchToHostButtonLabel.isVisible() != visible)
        {
            switchToHostButtonLabel.setVisible (visible);
            switchToHostButton.setVisible (visible);
            if (visible) {
                auto icon = hostType.getHostIcon (buttonSize);
                switchToHostButton.setImages(false, true, true,
                                             icon, 1.0, Colours::transparentBlack,
                                             icon, 1.0, Colours::transparentBlack,
                                             icon, 1.0, Colours::transparentBlack);
            }
        }
    }

    IAAEffectProcessor& processor;
    AudioProcessorValueTreeState& parameters;

    const int buttonSize = 30;
    const Colour defaultButtonColour = Colours::darkgrey;
    ShapeButton rewindButton {"Rewind", defaultButtonColour, defaultButtonColour, defaultButtonColour};
    ShapeButton playButton   {"Play",   defaultButtonColour, defaultButtonColour, defaultButtonColour};
    ShapeButton recordButton {"Record", defaultButtonColour, defaultButtonColour, defaultButtonColour};

    Slider gainSlider;
    AudioProcessorValueTreeState::SliderAttachment gainAttachment = {parameters, "gain", gainSlider};

    std::array<SimpleMeter, 2> meters;

    ImageButton switchToHostButton;
    Label transportText, switchToHostButtonLabel;
    Image hostImage;

    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IAAEffectEditor)
};
