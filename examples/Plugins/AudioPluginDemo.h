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

 name:             AudioPluginDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Synthesiser audio plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone, androidstudio

 type:             AudioProcessor
 mainClass:        JuceDemoPluginAudioProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/** A demo synth sound that's just a basic sine wave.. */
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};

//==============================================================================
/** A simple demo synth voice that just plays a sine wave.. */
class SineWaveVoice   : public SynthesiserVoice
{
public:
    SineWaveVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound* /*sound*/,
                    int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..

            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override
    {
        // not implemented for the purposes of this demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not implemented for the purposes of this demo!
    }

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (sin (currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        // tells the synth that this voice has stopped
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (sin (currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0;
    double angleDelta   = 0.0;
    double level        = 0.0;
    double tailOff      = 0.0;
};

//==============================================================================
/** As the name suggest, this class does the actual audio processing. */
class JuceDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JuceDemoPluginAudioProcessor()
        : AudioProcessor (getBusesProperties())
    {
        lastPosInfo.resetToDefault();

        // This creates our parameters. We'll keep some raw pointers to them in this class,
        // so that we can easily access them later, but the base class will take care of
        // deleting them for us.
        addParameter (gainParam  = new AudioParameterFloat ("gain",  "Gain",           0.0f, 1.0f, 0.9f));
        addParameter (delayParam = new AudioParameterFloat ("delay", "Delay Feedback", 0.0f, 1.0f, 0.5f));

        initialiseSynth();
    }

    ~JuceDemoPluginAudioProcessor() {}

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // Only mono/stereo and input/output must have same layout
        const auto& mainOutput = layouts.getMainOutputChannelSet();
        const auto& mainInput  = layouts.getMainInputChannelSet();

        // input and output layout must either be the same or the input must be disabled altogether
        if (! mainInput.isDisabled() && mainInput != mainOutput)
            return false;

        // do not allow disabling the main buses
        if (mainOutput.isDisabled())
            return false;

        // only allow stereo and mono
        if (mainOutput.size() > 2)
            return false;

        return true;
    }

    void prepareToPlay (double newSampleRate, int /*samplesPerBlock*/) override
    {
        // Use this method as the place to do any pre-playback
        // initialisation that you need..
        synth.setCurrentPlaybackSampleRate (newSampleRate);
        keyboardState.reset();

        if (isUsingDoublePrecision())
        {
            delayBufferDouble.setSize (2, 12000);
            delayBufferFloat .setSize (1, 1);
        }
        else
        {
            delayBufferFloat .setSize (2, 12000);
            delayBufferDouble.setSize (1, 1);
        }

        reset();
    }

    void releaseResources() override
    {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
        keyboardState.reset();
    }

    void reset() override
    {
        // Use this method as the place to clear any delay lines, buffers, etc, as it
        // means there's been a break in the audio's continuity.
        delayBufferFloat .clear();
        delayBufferDouble.clear();
    }

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferFloat);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferDouble);
    }

    //==============================================================================
    bool hasEditor() const override                                   { return true; }

    AudioProcessorEditor* createEditor() override
    {
        return new JuceDemoPluginAudioProcessorEditor (*this);
    }

    //==============================================================================
    const String getName() const override                             { return JucePlugin_Name; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }
    double getTailLengthSeconds() const override                      { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return {}; }
    void changeProgramName (int, const String&) override              {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        // You should use this method to store your parameters in the memory block.
        // Here's an example of how you can use XML to make it easy and more robust:

        // Create an outer XML element..
        XmlElement xml ("MYPLUGINSETTINGS");

        // add some attributes to it..
        xml.setAttribute ("uiWidth",  lastUIWidth);
        xml.setAttribute ("uiHeight", lastUIHeight);

        // Store the values of all our parameters, using their param ID as the XML attribute
        for (auto* param : getParameters())
            if (auto* p = dynamic_cast<AudioProcessorParameterWithID*> (param))
                xml.setAttribute (p->paramID, p->getValue());

        // then use this helper function to stuff it into the binary blob and return it..
        copyXmlToBinary (xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        // You should use this method to restore your parameters from this memory block,
        // whose contents will have been created by the getStateInformation() call.

        // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
        std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

        if (xmlState.get() != nullptr)
        {
            // make sure that it's actually our type of XML object..
            if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
            {
                // ok, now pull out our last window size..
                lastUIWidth  = jmax (xmlState->getIntAttribute ("uiWidth",  lastUIWidth),  400);
                lastUIHeight = jmax (xmlState->getIntAttribute ("uiHeight", lastUIHeight), 200);

                // Now reload our parameters..
                for (auto* param : getParameters())
                    if (auto* p = dynamic_cast<AudioProcessorParameterWithID*> (param))
                        p->setValue ((float) xmlState->getDoubleAttribute (p->paramID, p->getValue()));
            }
        }
    }

    //==============================================================================
    void updateTrackProperties (const TrackProperties& properties) override
    {
        trackProperties = properties;

        if (auto* editor = dynamic_cast<JuceDemoPluginAudioProcessorEditor*> (getActiveEditor()))
            editor->updateTrackProperties ();
    }

    //==============================================================================
    // These properties are public so that our editor component can access them
    // A bit of a hacky way to do it, but it's only a demo! Obviously in your own
    // code you'll do this much more neatly..

    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    MidiKeyboardState keyboardState;

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    int lastUIWidth = 400, lastUIHeight = 200;

    // Our parameters
    AudioParameterFloat* gainParam  = nullptr;
    AudioParameterFloat* delayParam = nullptr;

    // Current track colour and name
    TrackProperties trackProperties;

private:
    //==============================================================================
    /** This is the editor component that our filter will display. */
    class JuceDemoPluginAudioProcessorEditor  : public AudioProcessorEditor,
                                                private Timer
    {
    public:
        JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor& owner)
            : AudioProcessorEditor (owner),
              midiKeyboard         (owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
        {
            // add some sliders..
            gainSlider.reset (new ParameterSlider (*owner.gainParam));
            addAndMakeVisible (gainSlider.get());
            gainSlider->setSliderStyle (Slider::Rotary);

            delaySlider.reset (new ParameterSlider (*owner.delayParam));
            addAndMakeVisible (delaySlider.get());
            delaySlider->setSliderStyle (Slider::Rotary);

            // add some labels for the sliders..
            gainLabel.attachToComponent (gainSlider.get(), false);
            gainLabel.setFont (Font (11.0f));

            delayLabel.attachToComponent (delaySlider.get(), false);
            delayLabel.setFont (Font (11.0f));

            // add the midi keyboard component..
            addAndMakeVisible (midiKeyboard);

            // add a label that will display the current timecode and status..
            addAndMakeVisible (timecodeDisplayLabel);
            timecodeDisplayLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 15.0f, Font::plain));

            // set resize limits for this plug-in
            setResizeLimits (400, 200, 1024, 700);

            // set our component's initial size to be the last one that was stored in the filter's settings
            setSize (owner.lastUIWidth,
                     owner.lastUIHeight);

            updateTrackProperties();

            // start a timer which will keep our timecode display updated
            startTimerHz (30);
        }

        ~JuceDemoPluginAudioProcessorEditor() {}

        //==============================================================================
        void paint (Graphics& g) override
        {
            g.setColour (backgroundColour);
            g.fillAll();
        }

        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced (8);

            timecodeDisplayLabel.setBounds (r.removeFromTop (26));
            midiKeyboard        .setBounds (r.removeFromBottom (70));

            r.removeFromTop (20);
            auto sliderArea = r.removeFromTop (60);
            gainSlider->setBounds  (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() / 2)));
            delaySlider->setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));

            getProcessor().lastUIWidth  = getWidth();
            getProcessor().lastUIHeight = getHeight();
        }

        void timerCallback() override
        {
            updateTimecodeDisplay (getProcessor().lastPosInfo);
        }

        void hostMIDIControllerIsAvailable (bool controllerIsAvailable) override
        {
            midiKeyboard.setVisible (! controllerIsAvailable);
        }

        void updateTrackProperties()
        {
            auto trackColour = getProcessor().trackProperties.colour;
            auto& lf = getLookAndFeel();

            backgroundColour = (trackColour == Colour() ? lf.findColour (ResizableWindow::backgroundColourId)
                                                        : trackColour.withAlpha (1.0f).withBrightness (0.266f));
            repaint();
        }

    private:
        //==============================================================================
        // This is a handy slider subclass that controls an AudioProcessorParameter
        // (may move this class into the library itself at some point in the future..)
        class ParameterSlider   : public Slider,
                                  private Timer
        {
        public:
            ParameterSlider (AudioProcessorParameter& p)
                : Slider (p.getName (256)), param (p)
            {
                setRange (0.0, 1.0, 0.0);
                startTimerHz (30);
                updateSliderPos();
            }

            void valueChanged() override        { param.setValueNotifyingHost ((float) Slider::getValue()); }

            void timerCallback() override       { updateSliderPos(); }

            void startedDragging() override     { param.beginChangeGesture(); }
            void stoppedDragging() override     { param.endChangeGesture();   }

            double getValueFromText (const String& text) override   { return param.getValueForText (text); }
            String getTextFromValue (double value) override         { return param.getText ((float) value, 1024); }

            void updateSliderPos()
            {
                auto newValue = param.getValue();

                if (newValue != (float) Slider::getValue() && ! isMouseButtonDown())
                    Slider::setValue (newValue, NotificationType::dontSendNotification);
            }

            AudioProcessorParameter& param;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
        };

        MidiKeyboardComponent midiKeyboard;

        Label timecodeDisplayLabel,
              gainLabel  { {}, "Throughput level:" },
              delayLabel { {}, "Delay:" };

        std::unique_ptr<ParameterSlider> gainSlider, delaySlider;
        Colour backgroundColour;

        //==============================================================================
        JuceDemoPluginAudioProcessor& getProcessor() const
        {
            return static_cast<JuceDemoPluginAudioProcessor&> (processor);
        }

        //==============================================================================
        // quick-and-dirty function to format a timecode string
        static String timeToTimecodeString (double seconds)
        {
            auto millisecs = roundToInt (seconds * 1000.0);
            auto absMillisecs = std::abs (millisecs);

            return String::formatted ("%02d:%02d:%02d.%03d",
                                      millisecs / 3600000,
                                      (absMillisecs / 60000) % 60,
                                      (absMillisecs / 1000)  % 60,
                                      absMillisecs % 1000);
        }

        // quick-and-dirty function to format a bars/beats string
        static String quarterNotePositionToBarsBeatsString (double quarterNotes, int numerator, int denominator)
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

        // Updates the text in our position label.
        void updateTimecodeDisplay (AudioPlayHead::CurrentPositionInfo pos)
        {
            MemoryOutputStream displayText;

            displayText << "[" << SystemStats::getJUCEVersion() << "]   "
            << String (pos.bpm, 2) << " bpm, "
            << pos.timeSigNumerator << '/' << pos.timeSigDenominator
            << "  -  " << timeToTimecodeString (pos.timeInSeconds)
            << "  -  " << quarterNotePositionToBarsBeatsString (pos.ppqPosition,
                                                                pos.timeSigNumerator,
                                                                pos.timeSigDenominator);

            if (pos.isRecording)
                displayText << "  (recording)";
            else if (pos.isPlaying)
                displayText << "  (playing)";

            timecodeDisplayLabel.setText (displayText.toString(), dontSendNotification);
        }
    };

    //==============================================================================
    template <typename FloatType>
    void process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer)
    {
        auto numSamples = buffer.getNumSamples();

        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, numSamples);

        // Now pass any incoming midi messages to our keyboard state object, and let it
        // add messages to the buffer if the user is clicking on the on-screen keys
        keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

        // and now get our synth to process these midi events and generate its output.
        synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

        // Apply our delay effect to the new output..
        applyDelay (buffer, delayBuffer);

        applyGain (buffer, delayBuffer); // apply our gain-change to the outgoing data..

        // Now ask the host for the current time so we can store it to be displayed later...
        updateCurrentTimeInfoFromHost();
    }

    template <typename FloatType>
    void applyGain (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer)
    {
        ignoreUnused (delayBuffer);
        auto gainLevel = gainParam->get();

        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
            buffer.applyGain (channel, 0, buffer.getNumSamples(), gainLevel);
    }

    template <typename FloatType>
    void applyDelay (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer)
    {
        auto numSamples = buffer.getNumSamples();
        auto delayLevel = delayParam->get();

        auto delayPos = 0;

        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        {
            auto channelData = buffer.getWritePointer (channel);
            auto delayData = delayBuffer.getWritePointer (jmin (channel, delayBuffer.getNumChannels() - 1));
            delayPos = delayPosition;

            for (auto i = 0; i < numSamples; ++i)
            {
                auto in = channelData[i];
                channelData[i] += delayData[delayPos];
                delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;

                if (++delayPos >= delayBuffer.getNumSamples())
                    delayPos = 0;
            }
        }

        delayPosition = delayPos;
    }

    AudioBuffer<float> delayBufferFloat;
    AudioBuffer<double> delayBufferDouble;

    int delayPosition = 0;

    Synthesiser synth;

    void initialiseSynth()
    {
        auto numVoices = 8;

        // Add some voices...
        for (auto i = 0; i < numVoices; ++i)
            synth.addVoice (new SineWaveVoice());

        // ..and give the synth a sound to play
        synth.addSound (new SineWaveSound());
    }

    void updateCurrentTimeInfoFromHost()
    {
        if (auto* ph = getPlayHead())
        {
            AudioPlayHead::CurrentPositionInfo newTime;

            if (ph->getCurrentPosition (newTime))
            {
                lastPosInfo = newTime;  // Successfully got the current time from the host..
                return;
            }
        }

        // If the host fails to provide the current time, we'll just reset our copy to a default..
        lastPosInfo.resetToDefault();
    }

    static BusesProperties getBusesProperties()
    {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};
