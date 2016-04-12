    /*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
class MainContentComponent   : public AudioAppComponent,
                               private Timer
{
public:
    //==============================================================================
    MainContentComponent()
        : pos (299, 299),
          waveTableIndex (0),
          bufferIndex (0),
          sampleRate (0.0),
          expectedSamplesPerBlock (0),
          dragging (false)
    {
        setSize (600, 600);

        for (int i = 0; i < numElementsInArray (waveValues); ++i)
            zeromem (waveValues[i], sizeof (waveValues[i]));

        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
        startTimerHz (60);
    }

    ~MainContentComponent()
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

        for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            int ind = waveTableIndex;

            float* const channelData = bufferToFill.buffer->getWritePointer (chan, bufferToFill.startSample);

            for (int i = 0; i < bufferToFill.numSamples; ++i)
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
        g.fillAll (Colours::black);

        Point<float> nextPos = pos + delta;

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
        g.setColour (Colours::grey);
        g.fillEllipse (pos.x, pos.y, 20, 20);

        drawWaveform (g, 20.0f, 0);
        drawWaveform (g, 40.0f, 1);
    }

    void drawWaveform (Graphics& g, float y, int channel) const
    {
        const int pathWidth = 2000;

        Path wavePath;
        wavePath.startNewSubPath (0.0f, y);

        for (int i = 1; i < pathWidth; ++i)
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

        for (int i = 0; i < steps; ++i)
        {
            Point<float> p = start + ((finish - start) * i) / steps;

            const int index = (bufferIndex + i) % wavetableSize;
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

    Point<float> pos, delta;
    int waveTableIndex;
    int bufferIndex;
    double sampleRate;
    int expectedSamplesPerBlock;
    Point<float> lastMousePosition;
    float waveValues[2][wavetableSize];
    bool dragging;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This is called from Main.cpp)
Component* createMainContentComponent()  { return new MainContentComponent(); };


#endif  // MAINCOMPONENT_H_INCLUDED
