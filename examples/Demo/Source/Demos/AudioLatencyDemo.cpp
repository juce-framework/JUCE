/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"
#include "AudioLiveScrollingDisplay.h"


class LatencyTester  : public AudioIODeviceCallback,
                       private Timer
{
public:
    LatencyTester (TextEditor& resultsBox_)
        : playingSampleNum (0),
          recordedSampleNum (-1),
          sampleRate (0),
          testIsRunning (false),
          resultsBox (resultsBox_)
    {
        MainAppWindow::getSharedAudioDeviceManager().addAudioCallback (this);
    }

    ~LatencyTester()
    {
        MainAppWindow::getSharedAudioDeviceManager().removeAudioCallback (this);
    }

    //==============================================================================
    void beginTest()
    {
        resultsBox.moveCaretToEnd();
        resultsBox.insertTextAtCaret (newLine + newLine + "Starting test..." + newLine);
        resultsBox.moveCaretToEnd();

        startTimer (50);

        const ScopedLock sl (lock);
        createTestSound();
        recordedSound.clear();
        playingSampleNum = recordedSampleNum = 0;
        testIsRunning = true;
    }

    void timerCallback()
    {
        if (testIsRunning && recordedSampleNum >= recordedSound.getNumSamples())
        {
            testIsRunning = false;
            stopTimer();

            // Test has finished, so calculate the result..
            const int latencySamples = calculateLatencySamples();

            resultsBox.moveCaretToEnd();
            resultsBox.insertTextAtCaret (getMessageDescribingResult (latencySamples));
            resultsBox.moveCaretToEnd();
        }
    }

    String getMessageDescribingResult (int latencySamples)
    {
        String message;

        if (latencySamples >= 0)
        {
            message << newLine
                    << "Results:" << newLine
                    << latencySamples << " samples (" << String (latencySamples * 1000.0 / sampleRate, 1)
                    << " milliseconds)" << newLine
                    << "The audio device reports an input latency of "
                    << deviceInputLatency << " samples, output latency of "
                    << deviceOutputLatency << " samples." << newLine
                    << "So the corrected latency = "
                    << (latencySamples - deviceInputLatency - deviceOutputLatency)
                    << " samples (" << String ((latencySamples - deviceInputLatency - deviceOutputLatency) * 1000.0 / sampleRate, 2)
                    << " milliseconds)";
        }
        else
        {
            message << newLine
                    << "Couldn't detect the test signal!!" << newLine
                    << "Make sure there's no background noise that might be confusing it..";
        }

        return message;
    }

    //==============================================================================
    void audioDeviceAboutToStart (AudioIODevice* device)
    {
        testIsRunning = false;
        playingSampleNum = recordedSampleNum = 0;

        sampleRate = device->getCurrentSampleRate();
        deviceInputLatency = device->getInputLatencyInSamples();
        deviceOutputLatency = device->getOutputLatencyInSamples();

        recordedSound.setSize (1, (int) (0.9 * sampleRate));
        recordedSound.clear();
    }

    void audioDeviceStopped()
    {
        // (nothing to do here)
    }

    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples)
    {
        const ScopedLock sl (lock);

        if (testIsRunning)
        {
            float* const recordingBuffer = recordedSound.getWritePointer (0);
            const float* const playBuffer = testSound.getReadPointer (0);

            for (int i = 0; i < numSamples; ++i)
            {
                if (recordedSampleNum < recordedSound.getNumSamples())
                {
                    float inputSamp = 0;
                    for (int j = numInputChannels; --j >= 0;)
                        if (inputChannelData[j] != 0)
                            inputSamp += inputChannelData[j][i];

                    recordingBuffer [recordedSampleNum] = inputSamp;
                }

                ++recordedSampleNum;

                float outputSamp = (playingSampleNum < testSound.getNumSamples()) ? playBuffer [playingSampleNum] : 0;

                for (int j = numOutputChannels; --j >= 0;)
                    if (outputChannelData[j] != 0)
                        outputChannelData[j][i] = outputSamp;

                ++playingSampleNum;
            }
        }
        else
        {
            // We need to clear the output buffers, in case they're full of junk..
            for (int i = 0; i < numOutputChannels; ++i)
                if (outputChannelData[i] != 0)
                    zeromem (outputChannelData[i], sizeof (float) * (size_t) numSamples);
        }
    }

private:
    AudioSampleBuffer testSound, recordedSound;
    Array<int> spikePositions;
    int playingSampleNum, recordedSampleNum;
    CriticalSection lock;
    double sampleRate;
    bool testIsRunning;
    TextEditor& resultsBox;
    int deviceInputLatency, deviceOutputLatency;

    // create a test sound which consists of a series of randomly-spaced audio spikes..
    void createTestSound()
    {
        const int length = ((int) sampleRate) / 4;
        testSound.setSize (1, length);
        testSound.clear();

        Random rand;

        for (int i = 0; i < length; ++i)
            testSound.setSample (0, i, (rand.nextFloat() - rand.nextFloat() + rand.nextFloat() - rand.nextFloat()) * 0.06f);

        spikePositions.clear();

        int spikePos = 0;
        int spikeDelta = 50;

        while (spikePos < length - 1)
        {
            spikePositions.add (spikePos);

            testSound.setSample (0, spikePos,      0.99f);
            testSound.setSample (0, spikePos + 1, -0.99f);

            spikePos += spikeDelta;
            spikeDelta += spikeDelta / 6 + rand.nextInt (5);
        }
    }

    // Searches a buffer for a set of spikes that matches those in the test sound
    int findOffsetOfSpikes (const AudioSampleBuffer& buffer) const
    {
        const float minSpikeLevel = 5.0f;
        const double smooth = 0.975;
        const float* s = buffer.getReadPointer (0);
        const int spikeDriftAllowed = 5;

        Array<int> spikesFound;
        spikesFound.ensureStorageAllocated (100);
        double runningAverage = 0;
        int lastSpike = 0;

        for (int i = 0; i < buffer.getNumSamples() - 10; ++i)
        {
            const float samp = std::abs (s[i]);

            if (samp > runningAverage * minSpikeLevel && i > lastSpike + 20)
            {
                lastSpike = i;
                spikesFound.add (i);
            }

            runningAverage = runningAverage * smooth + (1.0 - smooth) * samp;
        }

        int bestMatch = -1;
        int bestNumMatches = spikePositions.size() / 3; // the minimum number of matches required

        if (spikesFound.size() < bestNumMatches)
            return -1;

        for (int offsetToTest = 0; offsetToTest < buffer.getNumSamples() - 2048; ++offsetToTest)
        {
            int numMatchesHere = 0;
            int foundIndex = 0;

            for (int refIndex = 0; refIndex < spikePositions.size(); ++refIndex)
            {
                const int referenceSpike = spikePositions.getUnchecked (refIndex) + offsetToTest;
                int spike = 0;

                while ((spike = spikesFound.getUnchecked (foundIndex)) < referenceSpike - spikeDriftAllowed
                         && foundIndex < spikesFound.size() - 1)
                    ++foundIndex;

                if (spike >= referenceSpike - spikeDriftAllowed && spike <= referenceSpike + spikeDriftAllowed)
                    ++numMatchesHere;
            }

            if (numMatchesHere > bestNumMatches)
            {
                bestNumMatches = numMatchesHere;
                bestMatch = offsetToTest;

                if (numMatchesHere == spikePositions.size())
                    break;
            }
        }

        return bestMatch;
    }

    int calculateLatencySamples() const
    {
        // Detect the sound in both our test sound and the recording of it, and measure the difference
        // in their start times..
        const int referenceStart = findOffsetOfSpikes (testSound);
        jassert (referenceStart >= 0);

        const int recordedStart = findOffsetOfSpikes (recordedSound);

        return (recordedStart < 0) ? -1
                                   : (recordedStart - referenceStart);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyTester)
};

//==============================================================================
class AudioLatencyDemo  : public Component,
                          private Button::Listener
{
public:
    AudioLatencyDemo()
    {
        setOpaque (true);

        addAndMakeVisible (liveAudioScroller = new LiveScrollingAudioDisplay());

        addAndMakeVisible (resultsBox);
        resultsBox.setMultiLine (true);
        resultsBox.setReturnKeyStartsNewLine (true);
        resultsBox.setReadOnly (true);
        resultsBox.setScrollbarsShown (true);
        resultsBox.setCaretVisible (false);
        resultsBox.setPopupMenuEnabled (true);
        resultsBox.setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
        resultsBox.setColour (TextEditor::outlineColourId, Colour (0x1c000000));
        resultsBox.setColour (TextEditor::shadowColourId, Colour (0x16000000));
        resultsBox.setText ("Running this test measures the round-trip latency between the audio output and input "
                            "devices you\'ve got selected.\n\n"
                            "It\'ll play a sound, then try to measure the time at which the sound arrives "
                            "back at the audio input. Obviously for this to work you need to have your "
                            "microphone somewhere near your speakers...");

        addAndMakeVisible (startTestButton);
        startTestButton.addListener (this);
        startTestButton.setButtonText ("Test Latency");

        MainAppWindow::getSharedAudioDeviceManager().addAudioCallback (liveAudioScroller);
    }

    ~AudioLatencyDemo()
    {
        MainAppWindow::getSharedAudioDeviceManager().removeAudioCallback (liveAudioScroller);
        startTestButton.removeListener (this);
        latencyTester = nullptr;
        liveAudioScroller = nullptr;
    }

    void startTest()
    {
        if (latencyTester == nullptr)
            latencyTester = new LatencyTester (resultsBox);

        latencyTester->beginTest();
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        liveAudioScroller->setBounds (8, 8, getWidth() - 16, 64);
        startTestButton.setBounds (8, getHeight() - 41, 168, 32);
        resultsBox.setBounds (8, 88, getWidth() - 16, getHeight() - 137);
    }

private:
    ScopedPointer<LatencyTester> latencyTester;

    ScopedPointer<LiveScrollingAudioDisplay> liveAudioScroller;
    TextButton startTestButton;
    TextEditor resultsBox;

    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == &startTestButton)
            startTest();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioLatencyDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<AudioLatencyDemo> demo ("31 Audio: Latency Detector");
