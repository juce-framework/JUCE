/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

SynthesiserSound::SynthesiserSound() {}
SynthesiserSound::~SynthesiserSound() {}

//==============================================================================
SynthesiserVoice::SynthesiserVoice()
    : currentSampleRate (44100.0),
      currentlyPlayingNote (-1),
      noteOnTime (0),
      keyIsDown (false),
      sostenutoPedalDown (false)
{
}

SynthesiserVoice::~SynthesiserVoice()
{
}

bool SynthesiserVoice::isPlayingChannel (const int midiChannel) const
{
    return currentlyPlayingSound != nullptr
            && currentlyPlayingSound->appliesToChannel (midiChannel);
}

void SynthesiserVoice::setCurrentPlaybackSampleRate (const double newRate)
{
    currentSampleRate = newRate;
}

void SynthesiserVoice::clearCurrentNote()
{
    currentlyPlayingNote = -1;
    currentlyPlayingSound = nullptr;
}

//==============================================================================
Synthesiser::Synthesiser()
    : sampleRate (0),
      lastNoteOnCounter (0),
      shouldStealNotes (true)
{
    for (int i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        lastPitchWheelValues[i] = 0x2000;
}

Synthesiser::~Synthesiser()
{
}

//==============================================================================
SynthesiserVoice* Synthesiser::getVoice (const int index) const
{
    const ScopedLock sl (lock);
    return voices [index];
}

void Synthesiser::clearVoices()
{
    const ScopedLock sl (lock);
    voices.clear();
}

void Synthesiser::addVoice (SynthesiserVoice* const newVoice)
{
    const ScopedLock sl (lock);
    voices.add (newVoice);
}

void Synthesiser::removeVoice (const int index)
{
    const ScopedLock sl (lock);
    voices.remove (index);
}

void Synthesiser::clearSounds()
{
    const ScopedLock sl (lock);
    sounds.clear();
}

void Synthesiser::addSound (const SynthesiserSound::Ptr& newSound)
{
    const ScopedLock sl (lock);
    sounds.add (newSound);
}

void Synthesiser::removeSound (const int index)
{
    const ScopedLock sl (lock);
    sounds.remove (index);
}

void Synthesiser::setNoteStealingEnabled (const bool shouldSteal)
{
    shouldStealNotes = shouldSteal;
}

//==============================================================================
void Synthesiser::setCurrentPlaybackSampleRate (const double newRate)
{
    if (sampleRate != newRate)
    {
        const ScopedLock sl (lock);

        allNotesOff (0, false);

        sampleRate = newRate;

        for (int i = voices.size(); --i >= 0;)
            voices.getUnchecked (i)->setCurrentPlaybackSampleRate (newRate);
    }
}

void Synthesiser::renderNextBlock (AudioSampleBuffer& outputBuffer, const MidiBuffer& midiData,
                                   int startSample, int numSamples)
{
    // must set the sample rate before using this!
    jassert (sampleRate != 0);

    const ScopedLock sl (lock);

    MidiBuffer::Iterator midiIterator (midiData);
    midiIterator.setNextSamplePosition (startSample);
    MidiMessage m (0xf4, 0.0);

    while (numSamples > 0)
    {
        int midiEventPos;
        const bool useEvent = midiIterator.getNextEvent (m, midiEventPos)
                                && midiEventPos < startSample + numSamples;

        const int numThisTime = useEvent ? midiEventPos - startSample
                                         : numSamples;

        if (numThisTime > 0)
        {
            for (int i = voices.size(); --i >= 0;)
                voices.getUnchecked (i)->renderNextBlock (outputBuffer, startSample, numThisTime);
        }

        if (useEvent)
            handleMidiEvent (m);

        startSample += numThisTime;
        numSamples -= numThisTime;
    }
}

void Synthesiser::handleMidiEvent (const MidiMessage& m)
{
    if (m.isNoteOn())
    {
        noteOn (m.getChannel(), m.getNoteNumber(), m.getFloatVelocity());
    }
    else if (m.isNoteOff())
    {
        noteOff (m.getChannel(), m.getNoteNumber(), true);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        allNotesOff (m.getChannel(), true);
    }
    else if (m.isPitchWheel())
    {
        const int channel = m.getChannel();
        const int wheelPos = m.getPitchWheelValue();
        lastPitchWheelValues [channel - 1] = wheelPos;

        handlePitchWheel (channel, wheelPos);
    }
    else if (m.isController())
    {
        handleController (m.getChannel(), m.getControllerNumber(), m.getControllerValue());
    }
}

//==============================================================================
void Synthesiser::noteOn (const int midiChannel,
                          const int midiNoteNumber,
                          const float velocity)
{
    const ScopedLock sl (lock);

    for (int i = sounds.size(); --i >= 0;)
    {
        SynthesiserSound* const sound = sounds.getUnchecked(i);

        if (sound->appliesToNote (midiNoteNumber)
             && sound->appliesToChannel (midiChannel))
        {
            // If hitting a note that's still ringing, stop it first (it could be
            // still playing because of the sustain or sostenuto pedal).
            for (int j = voices.size(); --j >= 0;)
            {
                SynthesiserVoice* const voice = voices.getUnchecked (j);

                if (voice->getCurrentlyPlayingNote() == midiNoteNumber
                     && voice->isPlayingChannel (midiChannel))
                    stopVoice (voice, true);
            }

            startVoice (findFreeVoice (sound, shouldStealNotes),
                        sound, midiChannel, midiNoteNumber, velocity);
        }
    }
}

void Synthesiser::startVoice (SynthesiserVoice* const voice,
                              SynthesiserSound* const sound,
                              const int midiChannel,
                              const int midiNoteNumber,
                              const float velocity)
{
    if (voice != nullptr && sound != nullptr)
    {
        if (voice->currentlyPlayingSound != nullptr)
            voice->stopNote (false);

        voice->startNote (midiNoteNumber, velocity, sound,
                          lastPitchWheelValues [midiChannel - 1]);

        voice->currentlyPlayingNote = midiNoteNumber;
        voice->noteOnTime = ++lastNoteOnCounter;
        voice->currentlyPlayingSound = sound;
        voice->keyIsDown = true;
        voice->sostenutoPedalDown = false;
    }
}

void Synthesiser::stopVoice (SynthesiserVoice* voice, const bool allowTailOff)
{
    jassert (voice != nullptr);

    voice->stopNote (allowTailOff);

    // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
    jassert (allowTailOff || (voice->getCurrentlyPlayingNote() < 0 && voice->getCurrentlyPlayingSound() == 0));
}

void Synthesiser::noteOff (const int midiChannel,
                           const int midiNoteNumber,
                           const bool allowTailOff)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
        {
            if (SynthesiserSound* const sound = voice->getCurrentlyPlayingSound())
            {
                if (sound->appliesToNote (midiNoteNumber)
                     && sound->appliesToChannel (midiChannel))
                {
                    voice->keyIsDown = false;

                    if (! (sustainPedalsDown [midiChannel] || voice->sostenutoPedalDown))
                        stopVoice (voice, allowTailOff);
                }
            }
        }
    }
}

void Synthesiser::allNotesOff (const int midiChannel, const bool allowTailOff)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->stopNote (allowTailOff);
    }

    sustainPedalsDown.clear();
}

void Synthesiser::handlePitchWheel (const int midiChannel, const int wheelValue)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->pitchWheelMoved (wheelValue);
    }
}

void Synthesiser::handleController (const int midiChannel,
                                    const int controllerNumber,
                                    const int controllerValue)
{
    switch (controllerNumber)
    {
        case 0x40:  handleSustainPedal   (midiChannel, controllerValue >= 64); break;
        case 0x42:  handleSostenutoPedal (midiChannel, controllerValue >= 64); break;
        case 0x43:  handleSoftPedal      (midiChannel, controllerValue >= 64); break;
        default:    break;
    }

    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->controllerMoved (controllerNumber, controllerValue);
    }
}

void Synthesiser::handleSustainPedal (int midiChannel, bool isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    if (isDown)
    {
        sustainPedalsDown.setBit (midiChannel);
    }
    else
    {
        for (int i = voices.size(); --i >= 0;)
        {
            SynthesiserVoice* const voice = voices.getUnchecked (i);

            if (voice->isPlayingChannel (midiChannel) && ! voice->keyIsDown)
                stopVoice (voice, true);
        }

        sustainPedalsDown.clearBit (midiChannel);
    }
}

void Synthesiser::handleSostenutoPedal (int midiChannel, bool isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (voice->isPlayingChannel (midiChannel))
        {
            if (isDown)
                voice->sostenutoPedalDown = true;
            else if (voice->sostenutoPedalDown)
                stopVoice (voice, true);
        }
    }
}

void Synthesiser::handleSoftPedal (int midiChannel, bool /*isDown*/)
{
    (void) midiChannel;
    jassert (midiChannel > 0 && midiChannel <= 16);
}

//==============================================================================
SynthesiserVoice* Synthesiser::findFreeVoice (SynthesiserSound* soundToPlay,
                                              const bool stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (voice->getCurrentlyPlayingNote() < 0  && voice->canPlaySound (soundToPlay))
            return voice;
    }

    if (stealIfNoneAvailable)
    {
        // currently this just steals the one that's been playing the longest, but could be made a bit smarter..
        SynthesiserVoice* oldest = nullptr;

        for (int i = voices.size(); --i >= 0;)
        {
            SynthesiserVoice* const voice = voices.getUnchecked (i);

            if (voice->canPlaySound (soundToPlay)
                 && (oldest == nullptr || oldest->noteOnTime > voice->noteOnTime))
                oldest = voice;
        }

        jassert (oldest != nullptr);
        return oldest;
    }

    return nullptr;
}
