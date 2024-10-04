/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioWorkgroupDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple audio workgroup demo application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioWorkgroupDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/AudioLiveScrollingDisplay.h"
#include "../Assets/ADSRComponent.h"

constexpr auto NumWorkerThreads = 4;

//==============================================================================
class ThreadBarrier : public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<ThreadBarrier>;

    static Ptr make (int numThreadsToSynchronise)
    {
        return { new ThreadBarrier { numThreadsToSynchronise } };
    }

    void arriveAndWait()
    {
        std::unique_lock lk { mutex };

        [[maybe_unused]] const auto c = ++blockCount;

        // You've tried to synchronise too many threads!!
        jassert (c <= threadCount);

        if (blockCount == threadCount)
        {
            blockCount = 0;
            cv.notify_all();
            return;
        }

        cv.wait (lk, [this] { return blockCount == 0; });
    }

private:
    std::mutex mutex;
    std::condition_variable cv;
    int blockCount{};
    const int threadCount{};

    explicit ThreadBarrier (int numThreadsToSynchronise)
        : threadCount (numThreadsToSynchronise) {}

    JUCE_DECLARE_NON_COPYABLE (ThreadBarrier)
    JUCE_DECLARE_NON_MOVEABLE (ThreadBarrier)
};

struct Voice
{
    struct Oscillator
    {
        float getNextSample()
        {
            const auto s = (2.f * phase - 1.f);
            phase += delta;

            if (phase >= 1.f)
                phase -= 1.f;

            return s;
        }

        float delta = 0;
        float phase = 0;
    };

    Voice (int numSamples, double newSampleRate)
        : sampleRate (newSampleRate),
          workBuffer (2, numSamples)
    {
    }

    bool isActive() const { return adsr.isActive(); }

    void startNote (int midiNoteNumber, float detuneAmount, ADSR::Parameters env)
    {
        constexpr float superSawDetuneValues[] = { -1.f, -0.8f, -0.6f, 0.f, 0.5f, 0.7f, 1.f };
        const auto freq = 440.f * std::pow (2.f, ((float) midiNoteNumber - 69.f) / 12.f);

        for (size_t i = 0; i < 7; i++)
        {
            auto& osc = oscillators[i];

            const auto detune = superSawDetuneValues[i] * detuneAmount;

            osc.delta = (freq + detune) / (float) sampleRate;
            osc.phase = wobbleGenerator.nextFloat();
        }

        currentNote = midiNoteNumber;

        adsr.setParameters (env);
        adsr.setSampleRate (sampleRate);
        adsr.noteOn();
    }

    void stopNote()
    {
        adsr.noteOff();
    }

    void run()
    {
        workBuffer.clear();

        constexpr auto oscillatorCount = 7;
        constexpr float superSawPanValues[] = { -1.f, -0.7f, -0.3f, 0.f, 0.3f, 0.7f, 1.f };

        constexpr auto spread = 0.8f;
        constexpr auto mix = 1 / 7.f;

        auto* l = workBuffer.getWritePointer (0);
        auto* r = workBuffer.getWritePointer (1);

        for (int i = 0; i < workBuffer.getNumSamples(); i++)
        {
            const auto a = adsr.getNextSample();

            float left = 0;
            float right = 0;

            for (size_t o = 0; o < oscillatorCount; o++)
            {
                auto& osc = oscillators[o];
                const auto s = a * osc.getNextSample();

                left  += s * (1.f - (superSawPanValues[o] * spread));
                right += s * (1.f + (superSawPanValues[o] * spread));
            }

            l[i] += left  * mix;
            r[i] += right * mix;
        }

        workBuffer.applyGain (0.25f);
    }

    const AudioSampleBuffer& getWorkBuffer() const { return workBuffer; }

    ADSR adsr;
    double sampleRate;
    std::array<Oscillator, 7> oscillators;
    int currentNote = 0;
    Random wobbleGenerator;

private:
    AudioSampleBuffer workBuffer;

    JUCE_DECLARE_NON_COPYABLE (Voice)
    JUCE_DECLARE_NON_MOVEABLE (Voice)
};

struct AudioWorkerThreadOptions
{
    int numChannels;
    int numSamples;
    double sampleRate;
    AudioWorkgroup workgroup;
    ThreadBarrier::Ptr completionBarrier;
};

class AudioWorkerThread final : private Thread
{
public:
    using Ptr = std::unique_ptr<AudioWorkerThread>;
    using Options = AudioWorkerThreadOptions;

    explicit AudioWorkerThread (const Options& workerOptions)
        : Thread ("AudioWorkerThread"),
          options (workerOptions)
    {
        jassert (options.completionBarrier != nullptr);

       #if defined (JUCE_MAC)
        jassert (options.workgroup);
       #endif

        startRealtimeThread (RealtimeOptions{}.withApproximateAudioProcessingTime (options.numSamples, options.sampleRate));
    }

    ~AudioWorkerThread() override { stop(); }

    using Thread::notify;
    using Thread::signalThreadShouldExit;
    using Thread::isThreadRunning;

    int getJobCount() const { return lastJobCount; }

    int queueAudioJobs (Span<Voice*> jobs)
    {
        size_t spanIndex = 0;

        const auto write = jobQueueFifo.write ((int) jobs.size());
        write.forEach ([&, jobs] (int dstIndex)
        {
            jobQueue[(size_t) dstIndex] = jobs[spanIndex++];
        });
        return write.blockSize1 + write.blockSize2;
    }

private:
    void stop()
    {
        signalThreadShouldExit();
        stopThread (-1);
    }

    void run() override
    {
        WorkgroupToken token;

        options.workgroup.join (token);

        while (wait (-1) && ! threadShouldExit())
        {
            const auto numReady = jobQueueFifo.getNumReady();
            lastJobCount = numReady;

            if (numReady > 0)
            {
                jobQueueFifo.read (jobQueueFifo.getNumReady())
                            .forEach ([this] (int srcIndex)
                            {
                                jobQueue[(size_t) srcIndex]->run();
                            });
            }

            // Wait for all our threads to get to this point.
            options.completionBarrier->arriveAndWait();
        }
    }

    static constexpr auto numJobs = 128;

    Options options;
    std::array<Voice*, numJobs> jobQueue;
    AbstractFifo jobQueueFifo { numJobs };
    std::atomic<int> lastJobCount = 0;

private:
    JUCE_DECLARE_NON_COPYABLE (AudioWorkerThread)
    JUCE_DECLARE_NON_MOVEABLE (AudioWorkerThread)
};

template <typename ValueType, typename LockType>
struct SharedThreadValue
{
    SharedThreadValue (LockType& lockRef, ValueType initialValue = {})
        : lock (lockRef),
          preSyncValue (initialValue),
          postSyncValue (initialValue)
    {
    }

    void set (const ValueType& newValue)
    {
        const typename LockType::ScopedLockType sl { lock };
        preSyncValue = newValue;
    }

    ValueType get() const
    {
        {
            const typename LockType::ScopedTryLockType sl { lock, true };

            if (sl.isLocked())
                postSyncValue = preSyncValue;
        }

        return postSyncValue;
    }

private:
    LockType& lock;
    ValueType preSyncValue{};
    mutable ValueType postSyncValue{};

    JUCE_DECLARE_NON_COPYABLE (SharedThreadValue)
    JUCE_DECLARE_NON_MOVEABLE (SharedThreadValue)
};

//==============================================================================
class SuperSynth
{
public:
    SuperSynth() = default;

    void setEnvelope (ADSR::Parameters params)
    {
        envelope.set (params);
    }

    void setThickness (float newThickness)
    {
        thickness.set (newThickness);
    }

    void prepareToPlay (int numSamples, double sampleRate)
    {
        activeVoices.reserve (128);

        for (auto& voice : voices)
            voice.reset (new Voice { numSamples, sampleRate });
    }

    void process (ThreadBarrier::Ptr barrier, Span<AudioWorkerThread*> workers,
                  AudioSampleBuffer& buffer, MidiBuffer& midiBuffer)
    {
        const auto blockThickness = thickness.get();
        const auto blockEnvelope = envelope.get();

        // We're not trying to be sample accurate.. handle the on/off events in a single block.
        for (auto event : midiBuffer)
        {
            const auto message = event.getMessage();

            if (message.isNoteOn())
            {
                for (auto& voice : voices)
                {
                    if (! voice->isActive())
                    {
                        voice->startNote (message.getNoteNumber(), blockThickness, blockEnvelope);
                        break;
                    }
                }

                continue;
            }

            if (message.isNoteOff())
            {
                for (auto& voice : voices)
                {
                    if (voice->currentNote == message.getNoteNumber())
                        voice->stopNote();
                }

                continue;
            }
        }

        // Queue up all active voices
        for (auto& voice : voices)
            if (voice->isActive())
                activeVoices.push_back (voice.get());

        constexpr auto jobsPerThread = 1;

        // Try and split the voices evenly just for demonstration purposes.
        // You could also do some of the work on this thread instead of waiting.
        for (int i = 0; i < (int) activeVoices.size();)
        {
            for (auto worker : workers)
            {
                if (i >= (int) activeVoices.size())
                    break;

                const auto jobCount = jmin (jobsPerThread, (int) activeVoices.size() - i);
                i += worker->queueAudioJobs ({ activeVoices.data() + i, (size_t) jobCount });
            }
        }

        // kick off the work.
        for (auto& worker : workers)
            worker->notify();

        // Wait for our jobs to complete.
        barrier->arriveAndWait();

        // mix the jobs into the main audio thread buffer.
        for (auto* voice : activeVoices)
        {
            buffer.addFrom (0, 0, voice->getWorkBuffer(), 0, 0, buffer.getNumSamples());
            buffer.addFrom (1, 0, voice->getWorkBuffer(), 1, 0, buffer.getNumSamples());
        }

        // Abuse std::vector not reallocating on clear.
        activeVoices.clear();
    }

private:
    std::array<std::unique_ptr<Voice>, 128> voices;
    std::vector<Voice*> activeVoices;

    template <typename T>
    using ThreadValue = SharedThreadValue<T, SpinLock>;

    SpinLock paramLock;
    ThreadValue<ADSR::Parameters> envelope  { paramLock, { 0.f, 0.3f, 1.f, 0.3f } };
    ThreadValue<float>            thickness { paramLock, 1.f };

    JUCE_DECLARE_NON_COPYABLE (SuperSynth)
    JUCE_DECLARE_NON_MOVEABLE (SuperSynth)
};

//==============================================================================
class AudioWorkgroupDemo  : public Component,
                            private Timer,
                            private AudioSource,
                            private MidiInputCallback
{
public:
    AudioWorkgroupDemo()
    {
        addAndMakeVisible (keyboardComponent);
        addAndMakeVisible (liveAudioDisplayComp);
        addAndMakeVisible (envelopeComponent);
        addAndMakeVisible (keyboardComponent);
        addAndMakeVisible (thicknessSlider);
        addAndMakeVisible (voiceCountLabel);

        std::generate (threadLabels.begin(), threadLabels.end(), &std::make_unique<Label>);

        for (auto& label : threadLabels)
        {
            addAndMakeVisible (*label);
            label->setEditable (false);
        }

        thicknessSlider.textFromValueFunction = [] (double) { return "Phatness"; };
        thicknessSlider.onValueChange = [this] { synthesizer.setThickness ((float) thicknessSlider.getValue()); };
        thicknessSlider.setRange (0.5, 15, 0.1);
        thicknessSlider.setValue (7, dontSendNotification);
        thicknessSlider.setTextBoxIsEditable (false);

        envelopeComponent.onChange  = [this] { synthesizer.setEnvelope (envelopeComponent.getParameters()); };

        voiceCountLabel.setEditable (false);

        audioSourcePlayer.setSource (this);

       #ifndef JUCE_DEMO_RUNNER
        audioDeviceManager.initialise (0, 2, nullptr, true, {}, nullptr);
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioDeviceManager.addMidiInputDeviceCallback ({}, this);

        setOpaque (true);
        setSize (640, 480);
        startTimerHz (10);
    }

    ~AudioWorkgroupDemo() override
    {
        audioSourcePlayer.setSource (nullptr);
        audioDeviceManager.removeMidiInputDeviceCallback ({}, this);
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        liveAudioDisplayComp.setBounds (bounds.removeFromTop (60));
        keyboardComponent.setBounds (bounds.removeFromBottom (150));
        envelopeComponent.setBounds (bounds.removeFromBottom (150));

        thicknessSlider.setBounds (bounds.removeFromTop (30));
        voiceCountLabel.setBounds (bounds.removeFromTop (30));

        const auto maxLabelWidth = bounds.getWidth() / 4;
        auto currentBounds = bounds.removeFromLeft (maxLabelWidth);

        for (auto& l : threadLabels)
        {
            if (currentBounds.getHeight() < 30)
                currentBounds = bounds.removeFromLeft (maxLabelWidth);

            l->setBounds (currentBounds.removeFromTop (30));
        }
    }

    void timerCallback() override
    {
        String text;
        int totalVoices = 0;

        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (size_t i = 0; i < NumWorkerThreads; i++)
            {
                const auto& thread = workerThreads[i];
                auto& label        = threadLabels[i];

                if (thread != nullptr)
                {
                    const auto count = thread->getJobCount();

                    text = "Thread ";
                    text << (int) i << ": " << count << " jobs";
                    label->setText (text, dontSendNotification);
                    totalVoices += count;
                }
            }
        }

        text = {};
        text << "Voices: " << totalVoices << " (" << totalVoices * 7 << " oscs)";
        voiceCountLabel.setText (text, dontSendNotification);
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        completionBarrier = ThreadBarrier::make ((int) NumWorkerThreads + 1);

        const auto numChannels = 2;
        const auto workerOptions = AudioWorkerThreadOptions
        {
            numChannels,
            samplesPerBlockExpected,
            sampleRate,
            audioDeviceManager.getDeviceAudioWorkgroup(),
            completionBarrier,
        };

        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (auto& worker : workerThreads)
                 worker.reset (new AudioWorkerThread { workerOptions });
        }

        synthesizer.prepareToPlay (samplesPerBlockExpected, sampleRate);
        liveAudioDisplayComp.audioDeviceAboutToStart (audioDeviceManager.getCurrentAudioDevice());
        waveformBuffer.setSize (1, samplesPerBlockExpected);
    }

    void releaseResources() override
    {
        {
            const SpinLock::ScopedLockType sl { threadArrayUiLock };

            for (auto& thread : workerThreads)
                thread.reset();
        }

        liveAudioDisplayComp.audioDeviceStopped();
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        midiBuffer.clear();

        bufferToFill.clearActiveBufferRegion();
        keyboardState.processNextMidiBuffer (midiBuffer, bufferToFill.startSample, bufferToFill.numSamples, true);

        AudioWorkerThread* workers[NumWorkerThreads]{};
        std::transform (workerThreads.begin(), workerThreads.end(), workers,
                        [] (auto& worker) { return worker.get(); });

        synthesizer.process (completionBarrier, Span { workers }, *bufferToFill.buffer, midiBuffer);

        // LiveAudioScrollingDisplay applies a 10x gain to the input signal, we need to reduce the gain on our signal.
        waveformBuffer.copyFrom (0, 0,
                                 bufferToFill.buffer->getReadPointer (0),
                                 bufferToFill.numSamples,
                                 1 / 10.f);
        liveAudioDisplayComp.audioDeviceIOCallbackWithContext (waveformBuffer.getArrayOfReadPointers(), 1,
                                                               nullptr, 0, bufferToFill.numSamples, {});
    }

    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        if (message.isNoteOn())
            keyboardState.noteOn (message.getChannel(), message.getNoteNumber(), 1);
        else if (message.isNoteOff())
            keyboardState.noteOff (message.getChannel(), message.getNoteNumber(), 1);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    MidiBuffer                midiBuffer;
    MidiKeyboardState         keyboardState;
    AudioSourcePlayer         audioSourcePlayer;
    SuperSynth                synthesizer;
    AudioSampleBuffer         waveformBuffer;

    MidiKeyboardComponent     keyboardComponent { keyboardState, MidiKeyboardComponent::horizontalKeyboard };
    LiveScrollingAudioDisplay liveAudioDisplayComp;
    ADSRComponent             envelopeComponent;
    Slider                    thicknessSlider { Slider::SliderStyle::LinearHorizontal, Slider::TextBoxLeft };
    Label                     voiceCountLabel;

    SpinLock                  threadArrayUiLock;
    ThreadBarrier::Ptr        completionBarrier;

    std::array<std::unique_ptr<Label>, NumWorkerThreads> threadLabels;
    std::array<AudioWorkerThread::Ptr, NumWorkerThreads> workerThreads;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWorkgroupDemo)
};
