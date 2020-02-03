/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             InterAppAudioEffectPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Inter-app audio effect plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        IAAEffectProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include <array>

//==============================================================================
// A very simple decaying meter.
class SimpleMeter  : public Component,
                     private Timer
{
public:
    SimpleMeter()
    {
        startTimerHz (30);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);

        auto area = g.getClipBounds();
        g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
        g.fillRoundedRectangle (area.toFloat(), 6.0);

        auto unfilledHeight = area.getHeight() * (1.0 - level);
        g.reduceClipRegion (area.getX(), area.getY(),
                            area.getWidth(), (int) unfilledHeight);
        g.setColour (getLookAndFeel().findColour (Slider::trackColourId));
        g.fillRoundedRectangle (area.toFloat(), 6.0);
    }

    void resized() override {}

    //==============================================================================
    // Called from the audio thread.
    void update (float newLevel)
    {
        // We don't care if maxLevel gets set to zero (in timerCallback) between the
        // load and the assignment.
        maxLevel = jmax (maxLevel.load(), newLevel);
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        auto callbackLevel = maxLevel.exchange (0.0);

        float decayFactor = 0.95f;

        if (callbackLevel > level)
            level = callbackLevel;
        else if (level > 0.001)
            level *= decayFactor;
        else
            level = 0;

        repaint();
    }

    std::atomic<float> maxLevel {0.0};
    float level = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleMeter)
};

#if JUCE_PROJUCER_LIVE_BUILD

// Animate the meter in the Projucer live build.
struct MockSimpleMeter  : public Component,
                          private Timer
{
    MockSimpleMeter()
    {
        addAndMakeVisible (meter);
        resized();
        startTimerHz (100);
    }

    void paint (Graphics&) override {}

    void resized() override
    {
        meter.setBounds (getBounds());
    }

    void timerCallback() override
    {
        meter.update (std::pow (randomNumberGenerator.nextFloat(), 2));
    }

    SimpleMeter meter;
    Random randomNumberGenerator;
};

#endif

//==============================================================================
// A simple Inter-App Audio plug-in with a gain control and some meters.
class IAAEffectProcessor  : public AudioProcessor
{
public:
    IAAEffectProcessor()
         : AudioProcessor (BusesProperties()
                           .withInput  ("Input",  AudioChannelSet::stereo(), true)
                           .withOutput ("Output", AudioChannelSet::stereo(), true)),
           parameters (*this, nullptr, "InterAppAudioEffect",
                       { std::make_unique<AudioParameterFloat> ("gain", "Gain", NormalisableRange<float> (0.0f, 1.0f), 1.0f / 3.14f) })
    {
    }

    ~IAAEffectProcessor() {}

    //==============================================================================
    void prepareToPlay (double, int) override
    {
        previousGain = *parameters.getRawParameterValue ("gain");
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts.getMainInputChannels() > 2)
            return false;

        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;

        return true;
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        float gain = *parameters.getRawParameterValue ("gain");

        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        auto numSamples = buffer.getNumSamples();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());

        // Apply the gain to the samples using a ramp to avoid discontinuities in
        // the audio between processed buffers.
        for (auto channel = 0; channel < totalNumInputChannels; ++channel)
        {
            buffer.applyGainRamp (channel, 0, numSamples, previousGain, gain);
            auto newLevel = buffer.getMagnitude (channel, 0, numSamples);

            meterListeners.call ([=] (MeterListener& l) { l.handleNewMeterValue (channel, newLevel); });
        }

        previousGain = gain;

        // Now ask the host for the current time so we can store it to be displayed later.
        updateCurrentTimeInfoFromHost (lastPosInfo);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        return new IAAEffectEditor (*this, parameters);
    }

    bool hasEditor() const override                                    { return true; }

    //==============================================================================
    const String getName() const override                              { return JucePlugin_Name; }
    bool acceptsMidi() const override                                  { return false; }
    bool producesMidi() const override                                 { return false; }
    double getTailLengthSeconds() const override                       { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                      { return 1; }
    int getCurrentProgram() override                                   { return 0; }
    void setCurrentProgram (int) override                              {}
    const String getProgramName (int) override                         { return {}; }
    void changeProgramName (int, const String&) override               {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        if (auto xml = parameters.state.createXml())
            copyXmlToBinary (*xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
            if (xmlState->hasTagName (parameters.state.getType()))
                parameters.state = ValueTree::fromXml (*xmlState);
    }

    //==============================================================================
    bool updateCurrentTimeInfoFromHost (AudioPlayHead::CurrentPositionInfo& posInfo)
    {
        if (auto* ph = getPlayHead())
        {
            AudioPlayHead::CurrentPositionInfo newTime;

            if (ph->getCurrentPosition (newTime))
            {
                posInfo = newTime;  // Successfully got the current time from the host.
                return true;
            }
        }

        // If the host fails to provide the current time, we'll just reset our copy to a default.
        lastPosInfo.resetToDefault();

        return false;
    }

    // Allow an IAAAudioProcessorEditor to register as a listener to receive new
    // meter values directly from the audio thread.
    struct MeterListener
    {
        virtual ~MeterListener() {}

        virtual void handleNewMeterValue (int, float) = 0;
    };

    void addMeterListener    (MeterListener& listener) { meterListeners.add    (&listener); }
    void removeMeterListener (MeterListener& listener) { meterListeners.remove (&listener); }


private:
    //==============================================================================
    class IAAEffectEditor  : public AudioProcessorEditor,
                             private IAAEffectProcessor::MeterListener,
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
            rewindButton.onClick = [this]
            {
                if (transportControllable())
                    processor.getPlayHead()->transportRewind();
            };
            addChildComponent (rewindButton);

            Path playShape;
            playShape.addTriangle (0.0, 0.0, 0.0, buttonSize, buttonSize, buttonSize / 2);
            playButton.setShape (playShape, true, true, false);
            playButton.onClick = [this]
            {
                if (transportControllable())
                    processor.getPlayHead()->transportPlay (! lastPosInfo.isPlaying);
            };
            addChildComponent (playButton);

            Path recordShape;
            recordShape.addEllipse (0.0, 0.0, buttonSize, buttonSize);
            recordButton.setShape (recordShape, true, true, false);
            recordButton.onClick = [this]
            {
                if (transportControllable())
                    processor.getPlayHead()->transportRecord (! lastPosInfo.isRecording);
            };
            addChildComponent (recordButton);

            // Configure the switch to host button.

            switchToHostButtonLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 18.0f, Font::plain));
            switchToHostButtonLabel.setJustificationType (Justification::centredRight);
            switchToHostButtonLabel.setText ("Switch to\nhost app:", dontSendNotification);
            addChildComponent (switchToHostButtonLabel);

            switchToHostButton.onClick = [this]
            {
                if (transportControllable())
                {
                    PluginHostType hostType;
                    hostType.switchToHostApplication();
                }
            };
            addChildComponent (switchToHostButton);

            auto screenSize = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
            setSize (screenSize.getWidth(), screenSize.getHeight());

            resized();

            startTimerHz (60);
        }

        ~IAAEffectEditor()
        {
            processor.removeMeterListener (*this);
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
        bool transportControllable()
        {
            auto playHead = processor.getPlayHead();
            return playHead != nullptr && playHead->canControlTransport();
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
                auto playColour = lastPosInfo.isPlaying ? Colours::green : defaultButtonColour;
                playButton.setColours (playColour, playColour, playColour);
                playButton.repaint();

                auto recordColour = lastPosInfo.isRecording ? Colours::red : defaultButtonColour;
                recordButton.setColours (recordColour, recordColour, recordColour);
                recordButton.repaint();
            }
        }

        void updateSwitchToHostDisplay()
        {
            PluginHostType hostType;
            auto visible = hostType.isInterAppAudioConnected();

            if (switchToHostButtonLabel.isVisible() != visible)
            {
                switchToHostButtonLabel.setVisible (visible);
                switchToHostButton.setVisible (visible);

                if (visible)
                {
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
        AudioProcessorValueTreeState::SliderAttachment gainAttachment = { parameters, "gain", gainSlider };

        std::array<SimpleMeter, 2> meters;

        ImageButton switchToHostButton;
        Label transportText, switchToHostButtonLabel;

        AudioPlayHead::CurrentPositionInfo lastPosInfo;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IAAEffectEditor)
    };

    //==============================================================================
    AudioProcessorValueTreeState parameters;
    float previousGain = 0.0f;
    std::array<float, 2> meterValues = { { 0, 0 } };

    // This keeps a copy of the last set of timing info that was acquired during an
    // audio callback - the UI component will display this.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    ListenerList<MeterListener> meterListeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IAAEffectProcessor)
};
