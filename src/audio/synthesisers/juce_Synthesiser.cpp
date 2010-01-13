/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Synthesiser.h"
#include "../../threads/juce_ScopedLock.h"


//==============================================================================
SynthesiserSound::SynthesiserSound()
{
}

SynthesiserSound::~SynthesiserSound()
{
}

//==============================================================================
SynthesiserVoice::SynthesiserVoice()
    : currentSampleRate (44100.0),
      currentlyPlayingNote (-1),
      noteOnTime (0),
      currentlyPlayingSound (0)
{
}

SynthesiserVoice::~SynthesiserVoice()
{
}

bool SynthesiserVoice::isPlayingChannel (const int midiChannel) const
{
    return currentlyPlayingSound != 0
            && currentlyPlayingSound->appliesToChannel (midiChannel);
}

void SynthesiserVoice::setCurrentPlaybackSampleRate (const double newRate)
{
    currentSampleRate = newRate;
}

void SynthesiserVoice::clearCurrentNote()
{
    currentlyPlayingNote = -1;
    currentlyPlayingSound = 0;
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

void Synthesiser::setNoteStealingEnabled (const bool shouldStealNotes_)
{
    shouldStealNotes = shouldStealNotes_;
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

void Synthesiser::renderNextBlock (AudioSampleBuffer& outputBuffer,
                                   const MidiBuffer& midiData,
                                   int startSample,
                                   int numSamples)
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
        {
            if (m.isNoteOn())
            {
                const int channel = m.getChannel();

                noteOn (channel,
                        m.getNoteNumber(),
                        m.getFloatVelocity());
            }
            else if (m.isNoteOff())
            {
                noteOff (m.getChannel(),
                         m.getNoteNumber(),
                         true);
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
                handleController (m.getChannel(),
                                  m.getControllerNumber(),
                                  m.getControllerValue());
            }
        }

        startSample += numThisTime;
        numSamples -= numThisTime;
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
    if (voice != 0 && sound != 0)
    {
        if (voice->currentlyPlayingSound != 0)
            voice->stopNote (false);

        voice->startNote (midiNoteNumber,
                          velocity,
                          sound,
                          lastPitchWheelValues [midiChannel - 1]);

        voice->currentlyPlayingNote = midiNoteNumber;
        voice->noteOnTime = ++lastNoteOnCounter;
        voice->currentlyPlayingSound = sound;
    }
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
            SynthesiserSound* const sound = voice->getCurrentlyPlayingSound();

            if (sound != 0
                 && sound->appliesToNote (midiNoteNumber)
                 && sound->appliesToChannel (midiChannel))
            {
                voice->stopNote (allowTailOff);

                // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
                jassert (allowTailOff || (voice->getCurrentlyPlayingNote() < 0 && voice->getCurrentlyPlayingSound() == 0));
            }
        }
    }
}

void Synthesiser::allNotesOff (const int midiChannel,
                               const bool allowTailOff)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->stopNote (allowTailOff);
    }
}

void Synthesiser::handlePitchWheel (const int midiChannel,
                                    const int wheelValue)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
        {
            voice->pitchWheelMoved (wheelValue);
        }
    }
}

void Synthesiser::handleController (const int midiChannel,
                                    const int controllerNumber,
                                    const int controllerValue)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        SynthesiserVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->controllerMoved (controllerNumber, controllerValue);
    }
}

//==============================================================================
SynthesiserVoice* Synthesiser::findFreeVoice (SynthesiserSound* soundToPlay,
                                              const bool stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
        if (voices.getUnchecked (i)->getCurrentlyPlayingNote() < 0
             && voices.getUnchecked (i)->canPlaySound (soundToPlay))
            return voices.getUnchecked (i);

    if (stealIfNoneAvailable)
    {
        // currently this just steals the one that's been playing the longest, but could be made a bit smarter..
        SynthesiserVoice* oldest = 0;

        for (int i = voices.size(); --i >= 0;)
        {
            SynthesiserVoice* const voice = voices.getUnchecked (i);

            if (voice->canPlaySound (soundToPlay)
                 && (oldest == 0 || oldest->noteOnTime > voice->noteOnTime))
                oldest = voice;
        }

        jassert (oldest != 0);
        return oldest;
    }

    return 0;
}


END_JUCE_NAMESPACE
