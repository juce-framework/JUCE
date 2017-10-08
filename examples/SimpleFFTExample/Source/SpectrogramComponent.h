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

class SpectrogramComponent   : public AudioAppComponent,
                               private Timer
{
public:
    SpectrogramComponent()
        : forwardFFT (fftOrder),
          spectrogramImage (Image::RGB, 512, 512, true),
          fifoIndex (0),
          nextFFTBlockReady (false)
    {
        setOpaque (true);
        setAudioChannels (2, 0);  // we want a couple of input channels but no outputs
        startTimerHz (60);
        setSize (700, 500);
    }

    ~SpectrogramComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int /*samplesPerBlockExpected*/, double /*newSampleRate*/) override
    {
        // (nothing to do here)
    }

    void releaseResources() override
    {
        // (nothing to do here)
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            const float* channelData = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);

            for (int i = 0; i < bufferToFill.numSamples; ++i)
                pushNextSampleIntoFifo (channelData[i]);
        }
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        g.setOpacity (1.0f);
        g.drawImage (spectrogramImage, getLocalBounds().toFloat());
    }

    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;
    }

    void drawNextLineOfSpectrogram()
    {
        const int rightHandEdge = spectrogramImage.getWidth() - 1;
        const int imageHeight = spectrogramImage.getHeight();

        // first, shuffle our image leftwards by 1 pixel..
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform (fftData);

        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
        Range<float> maxLevel = FloatVectorOperations::findMinAndMax (fftData, fftSize / 2);

        for (int y = 1; y < imageHeight; ++y)
        {
            const float skewedProportionY = 1.0f - std::exp (std::log (y / (float) imageHeight) * 0.2f);
            const int fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            const float level = jmap (fftData[fftDataIndex], 0.0f, jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

            spectrogramImage.setPixelAt (rightHandEdge, y, Colour::fromHSV (level, 1.0f, level, 1.0f));
        }
    }

    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder
    };

private:
    dsp::FFT forwardFFT;
    Image spectrogramImage;

    float fifo [fftSize];
    float fftData [2 * fftSize];
    int fifoIndex;
    bool nextFFTBlockReady;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};
