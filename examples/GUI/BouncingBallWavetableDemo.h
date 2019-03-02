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

 name:             BouncingBallWavetableDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Wavetable synthesis with a bouncing ball.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        BouncingBallWavetableDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class BouncingBallWavetableDemo   : public AudioAppComponent,
                                    private Timer
{
public:
    //==============================================================================
    BouncingBallWavetableDemo()
       #ifdef JUCE_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        setSize (600, 600);

        for (auto i = 0; i < numElementsInArray (waveValues); ++i)
            zeromem (waveValues[i], sizeof (waveValues[i]));

        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
        startTimerHz (60);
    }

    ~BouncingBallWavetableDemo()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        sampleRate = newSampleRate;
        expectedSamplesPerBlock = samplesPerBlockExpected;
    }

    /*  This method generates the actual audio samples.
        In this example the buffer is filled with a sine wave whose frequency and
        amplitude are controlled by the mouse position.
     */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            auto ind = waveTableIndex;

            auto* channelData = bufferToFill.buffer->getWritePointer (chan, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                if (isPositiveAndBelow (chan, numElementsInArray (waveValues)))
                {
                    channelData[i] = waveValues[chan][ind % wavetableSize];
                    ++ind;
                }
            }
        }

        waveTableIndex = (int) (waveTableIndex + bufferToFill.numSamples) % wavetableSize;
    }

    void releaseResources() override
    {
        // This gets automatically called when audio device parameters change
        // or device is restarted.
        stopTimer();
    }


    //==============================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        auto nextPos = pos + delta;

        if (nextPos.x < 10 || nextPos.x + 10 > getWidth())
        {
            delta.x = -delta.x;
            nextPos.x = pos.x + delta.x;
        }

        if (nextPos.y < 50 || nextPos.y + 10 > getHeight())
        {
            delta.y = -delta.y;
            nextPos.y = pos.y + delta.y;
        }

        if (! dragging)
        {
            writeInterpolatedValue (pos, nextPos);
            pos = nextPos;
        }
        else
        {
            pos = lastMousePosition;
        }

        // draw a circle
        g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
        g.fillEllipse (pos.x, pos.y, 20, 20);

        drawWaveform (g, 20.0f, 0);
        drawWaveform (g, 40.0f, 1);
    }

    void drawWaveform (Graphics& g, float y, int channel) const
    {
        auto pathWidth = 2000;

        Path wavePath;
        wavePath.startNewSubPath (0.0f, y);

        for (auto i = 1; i < pathWidth; ++i)
            wavePath.lineTo ((float) i, (1.0f + waveValues[channel][i * numElementsInArray (waveValues[0]) / pathWidth]) * 10.0f);

        g.strokePath (wavePath, PathStrokeType (1.0f),
                      wavePath.getTransformToScaleToFit (Rectangle<float> (0.0f, y, (float) getWidth(), 20.0f), false));
    }

    // Mouse handling..
    void mouseDown (const MouseEvent& e) override
    {
        lastMousePosition = e.position;
        mouseDrag (e);
        dragging = true;
    }

    void mouseDrag (const MouseEvent& e) override
    {
        dragging = true;

        if (e.position != lastMousePosition)
        {
            // calculate movement vector
            delta = e.position - lastMousePosition;

            waveValues[0][bufferIndex % wavetableSize] = xToAmplitude (e.position.x);
            waveValues[1][bufferIndex % wavetableSize] = yToAmplitude (e.position.y);

            ++bufferIndex;
            lastMousePosition = e.position;
        }
    }

    void mouseUp (const MouseEvent&) override
    {
        dragging = false;
    }

    void writeInterpolatedValue (Point<float> lastPosition,
                                 Point<float> currentPosition)
    {
        Point<float> start, finish;

        if (lastPosition.getX() > currentPosition.getX())
        {
            finish = lastPosition;
            start  = currentPosition;
        }
        else
        {
            start  = lastPosition;
            finish = currentPosition;
        }

        for (auto i = 0; i < steps; ++i)
        {
            auto p = start + ((finish - start) * i) / steps;

            auto index = (bufferIndex + i) % wavetableSize;
            waveValues[1][index] = yToAmplitude (p.y);
            waveValues[0][index] = xToAmplitude (p.x);
        }

        bufferIndex = (bufferIndex + steps) % wavetableSize;
    }

    float indexToX (int indexValue) const noexcept
    {
        return (float) indexValue;
    }

    float amplitudeToY (float amp) const noexcept
    {
        return getHeight() - (amp + 1.0f) * getHeight() / 2.0f;
    }

    float xToAmplitude (float x) const noexcept
    {
        return jlimit (-1.0f, 1.0f, 2.0f * (getWidth() - x) / getWidth() - 1.0f);
    }

    float yToAmplitude (float y) const noexcept
    {
        return jlimit (-1.0f, 1.0f, 2.0f * (getHeight() - y) / getHeight() - 1.0f);
    }

    void timerCallback() override
    {
        repaint();
    }

private:
    //==============================================================================
    enum
    {
        wavetableSize = 36000,
        steps = 10
    };

    Point<float> pos   = { 299.0f, 299.0f };
    Point<float> delta = { 0.0f, 0.0f };
    int waveTableIndex = 0;
    int bufferIndex    = 0;
    double sampleRate  = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;
    float waveValues[2][wavetableSize];
    bool dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBallWavetableDemo)
};
