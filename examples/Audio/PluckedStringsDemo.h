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

 name:             PluckedStringsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simulation of a plucked string sound.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017

 type:             Component
 mainClass:        PluckedStringsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    A very basic generator of a simulated plucked string sound, implementing
    the Karplus-Strong algorithm.

    Not performance-optimised!
*/
class StringSynthesiser
{
public:
    //==============================================================================
    /** Constructor.

        @param sampleRate      The audio sample rate to use.
        @param frequencyInHz   The fundamental frequency of the simulated string in
                               Hertz.
    */
    StringSynthesiser (double sampleRate, double frequencyInHz)
    {
        doPluckForNextBuffer.set (false);
        prepareSynthesiserState (sampleRate, frequencyInHz);
    }

    //==============================================================================
    /** Excite the simulated string by plucking it at a given position.

        @param pluckPosition The position of the plucking, relative to the length
                             of the string. Must be between 0 and 1.
    */
    void stringPlucked (float pluckPosition)
    {
        jassert (pluckPosition >= 0.0 && pluckPosition <= 1.0);

        // we choose a very simple approach to communicate with the audio thread:
        // simply tell the synth to perform the plucking excitation at the beginning
        // of the next buffer (= when generateAndAddData is called the next time).

        if (doPluckForNextBuffer.compareAndSetBool (1, 0))
        {
            // plucking in the middle gives the largest amplitude;
            // plucking at the very ends will do nothing.
            amplitude = std::sin (MathConstants<float>::pi * pluckPosition);
        }
    }

    //==============================================================================
    /** Generate next chunk of mono audio output and add it into a buffer.

        @param outBuffer  Buffer to fill (one channel only). New sound will be
                          added to existing content of the buffer (instead of
                          replacing it).
        @param numSamples Number of samples to generate (make sure that outBuffer
                          has enough space).
    */
    void generateAndAddData (float* outBuffer, int numSamples)
    {
        if (doPluckForNextBuffer.compareAndSetBool (0, 1))
            exciteInternalBuffer();

        // cycle through the delay line and apply a simple averaging filter
        for (auto i = 0; i < numSamples; ++i)
        {
            auto nextPos = (pos + 1) % delayLine.size();

            delayLine[nextPos] = (float) (decay * 0.5 * (delayLine[nextPos] + delayLine[pos]));
            outBuffer[i] += delayLine[pos];

            pos = nextPos;
        }
    }

private:
    //==============================================================================
    void prepareSynthesiserState (double sampleRate, double frequencyInHz)
    {
        auto delayLineLength = (size_t) roundToInt (sampleRate / frequencyInHz);

        // we need a minimum delay line length to get a reasonable synthesis.
        // if you hit this assert, increase sample rate or decrease frequency!
        jassert (delayLineLength > 50);

        delayLine.resize (delayLineLength);
        std::fill (delayLine.begin(), delayLine.end(), 0.0f);

        excitationSample.resize (delayLineLength);

        // as the excitation sample we use random noise between -1 and 1
        // (as a simple approximation to a plucking excitation)

        std::generate (excitationSample.begin(),
                       excitationSample.end(),
                       [] { return (Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f; } );
    }

    void exciteInternalBuffer()
    {
        // fill the buffer with the precomputed excitation sound (scaled with amplitude)

        jassert (delayLine.size() >= excitationSample.size());

        std::transform (excitationSample.begin(),
                        excitationSample.end(),
                        delayLine.begin(),
                        [this] (double sample) { return static_cast<float> (amplitude * sample); } );
    }

    //==============================================================================
    const double decay = 0.998;
    double amplitude = 0.0;

    Atomic<int> doPluckForNextBuffer;

    std::vector<float> excitationSample, delayLine;
    size_t pos = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringSynthesiser)
};

//==============================================================================
/*
    This component represents a horizontal vibrating musical string of fixed height
    and variable length. The string can be excited by calling stringPlucked().
*/
class StringComponent   : public Component,
                          private Timer
{
public:
    StringComponent (int lengthInPixels, Colour stringColour)
        : length (lengthInPixels), colour (stringColour)
    {
        // ignore mouse-clicks so that our parent can get them instead.
        setInterceptsMouseClicks (false, false);
        setSize (length, height);
        startTimerHz (60);
    }

    //==============================================================================
    void stringPlucked (float pluckPositionRelative)
    {
        amplitude = maxAmplitude * std::sin (pluckPositionRelative * MathConstants<float>::pi);
        phase = MathConstants<float>::pi;
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.setColour (colour);
        g.strokePath (generateStringPath(), PathStrokeType (2.0f));
    }

    Path generateStringPath() const
    {
        auto y = height / 2.0f;

        Path stringPath;
        stringPath.startNewSubPath (0, y);
        stringPath.quadraticTo (length / 2.0f, y + (std::sin (phase) * amplitude), (float) length, y);
        return stringPath;
    }

    //==============================================================================
    void timerCallback() override
    {
        updateAmplitude();
        updatePhase();
        repaint();
    }

    void updateAmplitude()
    {
        // this determines the decay of the visible string vibration.
        amplitude *= 0.99f;
    }

    void updatePhase()
    {
        // this determines the visible vibration frequency.
        // just an arbitrary number chosen to look OK:
        auto phaseStep = 400.0f / length;

        phase += phaseStep;

        if (phase >= MathConstants<float>::twoPi)
            phase -= MathConstants<float>::twoPi;
    }

private:
    //==============================================================================
    int length;
    Colour colour;

    int height = 20;
    float amplitude = 0.0f;
    const float maxAmplitude = 12.0f;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringComponent)
};

//==============================================================================
class PluckedStringsDemo   : public AudioAppComponent
{
public:
    PluckedStringsDemo()
       #ifdef JUCE_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        createStringComponents();
        setSize (800, 560);

        // specify the number of input and output channels that we want to open
        auto audioDevice = deviceManager.getCurrentAudioDevice();
        auto numInputChannels  = (audioDevice != nullptr ? audioDevice->getActiveInputChannels() .countNumberOfSetBits() : 0);
        auto numOutputChannels = jmax (audioDevice != nullptr ? audioDevice->getActiveOutputChannels().countNumberOfSetBits() : 2, 2);

        setAudioChannels (numInputChannels, numOutputChannels);
    }

    ~PluckedStringsDemo()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        generateStringSynths (sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* channelData = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);

            if (channel == 0)
            {
                for (auto synth : stringSynths)
                    synth->generateAndAddData (channelData, bufferToFill.numSamples);
            }
            else
            {
                memcpy (channelData,
                        bufferToFill.buffer->getReadPointer (0),
                        ((size_t) bufferToFill.numSamples) * sizeof (float));
            }
        }
    }

    void releaseResources() override
    {
        stringSynths.clear();
    }

    //==============================================================================
    void paint (Graphics&) override {}

    void resized() override
    {
        auto xPos = 20;
        auto yPos = 20;
        auto yDistance = 50;

        for (auto stringLine : stringLines)
        {
            stringLine->setTopLeftPosition (xPos, yPos);
            yPos += yDistance;
            addAndMakeVisible (stringLine);
        }
    }

private:
    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        for (auto i = 0; i < stringLines.size(); ++i)
        {
            auto* stringLine = stringLines.getUnchecked (i);

            if (stringLine->getBounds().contains (e.getPosition()))
            {
                auto position = (e.position.x - stringLine->getX()) / stringLine->getWidth();

                stringLine->stringPlucked (position);
                stringSynths.getUnchecked (i)->stringPlucked (position);
            }
        }
    }

    //==============================================================================
    struct StringParameters
    {
        StringParameters (int midiNote)
            : frequencyInHz (MidiMessage::getMidiNoteInHertz (midiNote)),
              lengthInPixels ((int) (760 / (frequencyInHz / MidiMessage::getMidiNoteInHertz (42))))
        {}

        double frequencyInHz;
        int lengthInPixels;
    };

    static Array<StringParameters> getDefaultStringParameters()
    {
        return Array<StringParameters> (42, 44, 46, 49, 51, 54, 56, 58, 61, 63, 66, 68, 70);
    }

    void createStringComponents()
    {
        for (auto stringParams : getDefaultStringParameters())
        {
            stringLines.add (new StringComponent (stringParams.lengthInPixels,
                                                  Colour::fromHSV (Random().nextFloat(), 0.6f, 0.9f, 1.0f)));
        }
    }

    void generateStringSynths (double sampleRate)
    {
        stringSynths.clear();

        for (auto stringParams : getDefaultStringParameters())
        {
            stringSynths.add (new StringSynthesiser (sampleRate, stringParams.frequencyInHz));
        }
    }

    //==============================================================================
    OwnedArray<StringComponent> stringLines;
    OwnedArray<StringSynthesiser> stringSynths;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluckedStringsDemo)
};
