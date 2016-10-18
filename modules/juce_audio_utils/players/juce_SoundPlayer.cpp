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

//==============================================================================
// This is an AudioTransportSource which will own it's assigned source
struct AudioSourceOwningTransportSource  : public AudioTransportSource
{
    AudioSourceOwningTransportSource (PositionableAudioSource* s)  : source (s)
    {
        AudioTransportSource::setSource (s);
    }

    ~AudioSourceOwningTransportSource()
    {
        setSource (nullptr);
    }

private:
    ScopedPointer<PositionableAudioSource> source;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourceOwningTransportSource)
};

//==============================================================================
// An AudioSourcePlayer which will remove itself from the AudioDeviceManager's
// callback list once it finishes playing its source
struct AutoRemovingTransportSource : public AudioTransportSource, private Timer
{
    AutoRemovingTransportSource (MixerAudioSource& mixerToUse, AudioTransportSource* ts, bool ownSource,
                                 int samplesPerBlock, double sampleRate)
        : mixer (mixerToUse), transportSource (ts, ownSource)
    {
        jassert (ts != nullptr);

        setSource (transportSource);

        prepareToPlay (samplesPerBlock, sampleRate);
        start();

        mixer.addInputSource (this, true);
        startTimerHz (10);
    }

    ~AutoRemovingTransportSource()
    {
        setSource (nullptr);
    }

    void timerCallback() override
    {
        if (! transportSource->isPlaying())
            mixer.removeInputSource (this);
    }

private:
    MixerAudioSource& mixer;
    OptionalScopedPointer<AudioTransportSource> transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoRemovingTransportSource)
};

// An AudioSource which simply outputs a buffer
class AudioSampleBufferSource  : public PositionableAudioSource
{
public:
    AudioSampleBufferSource (AudioSampleBuffer* audioBuffer, bool ownBuffer, bool playOnAllChannels)
        : buffer (audioBuffer, ownBuffer),
          position (0), looping (false),
          playAcrossAllChannels (playOnAllChannels)
    {}

    //==============================================================================
    void setNextReadPosition (int64 newPosition) override
    {
        jassert (newPosition >= 0);

        if (looping)
            newPosition = newPosition % static_cast<int64> (buffer->getNumSamples());

        position = jmin (buffer->getNumSamples(), static_cast<int> (newPosition));
    }

    int64 getNextReadPosition() const override      { return static_cast<int64> (position); }
    int64 getTotalLength() const override           { return static_cast<int64> (buffer->getNumSamples()); }

    bool isLooping() const override                 { return looping; }
    void setLooping (bool shouldLoop) override      { looping = shouldLoop; }

    //==============================================================================
    void prepareToPlay (int, double) override {}
    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        const int bufferSize = buffer->getNumSamples();
        const int samplesNeeded = bufferToFill.numSamples;
        const int samplesToCopy = jmin (bufferSize - position, samplesNeeded);

        if (samplesToCopy > 0)
        {
            int maxInChannels = buffer->getNumChannels();
            int maxOutChannels = bufferToFill.buffer->getNumChannels();

            if (! playAcrossAllChannels)
                maxOutChannels = jmin (maxOutChannels, maxInChannels);

            for (int i = 0; i < maxOutChannels; ++i)
                bufferToFill.buffer->copyFrom (i, bufferToFill.startSample, *buffer,
                                               i % maxInChannels, position, samplesToCopy);
        }

        position += samplesNeeded;

        if (looping)
            position %= bufferSize;
    }

private:
    //==============================================================================
    OptionalScopedPointer<AudioSampleBuffer> buffer;
    int position;
    bool looping, playAcrossAllChannels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSampleBufferSource)
};

SoundPlayer::SoundPlayer()
    : sampleRate (44100.0), bufferSize (512)
{
    formatManager.registerBasicFormats();
    player.setSource (&mixer);
}

SoundPlayer::~SoundPlayer()
{
    mixer.removeAllInputs();
    player.setSource (nullptr);
}

void SoundPlayer::play (const File& file)
{
    if (file.existsAsFile())
        play (formatManager.createReaderFor (file), true);
}

void SoundPlayer::play (const void* resourceData, size_t resourceSize)
{
    if (resourceData != nullptr && resourceSize > 0)
    {
        MemoryInputStream* mem = new MemoryInputStream (resourceData, resourceSize, false);
        play (formatManager.createReaderFor (mem), true);
    }
}

void SoundPlayer::play (AudioFormatReader* reader, bool deleteWhenFinished)
{
    if (reader != nullptr)
        play (new AudioFormatReaderSource (reader, deleteWhenFinished), true);
}

void SoundPlayer::play (AudioSampleBuffer* buffer, bool deleteWhenFinished, bool playOnAllOutputChannels)
{
    if (buffer != nullptr)
        play (new AudioSampleBufferSource (buffer, deleteWhenFinished, playOnAllOutputChannels), true);
}

void SoundPlayer::play (PositionableAudioSource* audioSource, bool deleteWhenFinished)
{
    if (audioSource != nullptr)
    {
        AudioTransportSource* transport = dynamic_cast<AudioTransportSource*> (audioSource);

        if (transport == nullptr)
        {
            if (deleteWhenFinished)
            {
                transport = new AudioSourceOwningTransportSource (audioSource);
            }
            else
            {
                transport = new AudioTransportSource();
                transport->setSource (audioSource);
                deleteWhenFinished = true;
            }
        }

        transport->start();
        transport->prepareToPlay (bufferSize, sampleRate);

        new AutoRemovingTransportSource (mixer, transport, deleteWhenFinished, bufferSize, sampleRate);
    }
    else
    {
        if (deleteWhenFinished)
            delete audioSource;
    }
}

void SoundPlayer::playTestSound()
{
    const int soundLength = (int) sampleRate;

    const double frequency = 440.0;
    const float amplitude = 0.5f;

    const double phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);

    AudioSampleBuffer* newSound = new AudioSampleBuffer (1, soundLength);

    for (int i = 0; i < soundLength; ++i)
        newSound->setSample (0, i, amplitude * (float) std::sin (i * phasePerSample));

    newSound->applyGainRamp (0, 0, soundLength / 10, 0.0f, 1.0f);
    newSound->applyGainRamp (0, soundLength - soundLength / 4, soundLength / 4, 1.0f, 0.0f);

    play (newSound, true, true);
}

//==============================================================================
void SoundPlayer::audioDeviceIOCallback (const float** inputChannelData,
                                         int numInputChannels,
                                         float** outputChannelData,
                                         int numOutputChannels,
                                         int numSamples)
{
    player.audioDeviceIOCallback (inputChannelData, numInputChannels,
                                  outputChannelData, numOutputChannels,
                                  numSamples);
}

void SoundPlayer::audioDeviceAboutToStart (AudioIODevice* device)
{
    if (device != nullptr)
    {
        sampleRate = device->getCurrentSampleRate();
        bufferSize = device->getCurrentBufferSizeSamples();
    }

    player.audioDeviceAboutToStart (device);
}

void SoundPlayer::audioDeviceStopped()
{
    player.audioDeviceStopped();
}

void SoundPlayer::audioDeviceError (const String& errorMessage)
{
    player.audioDeviceError (errorMessage);
}
