/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MidiKeyboardState.h"
#include "../../../juce_core/basics/juce_Time.h"


//==============================================================================
MidiKeyboardState::MidiKeyboardState()
    : listeners (2)
{
    zeromem (noteStates, sizeof (noteStates));
}

MidiKeyboardState::~MidiKeyboardState()
{
}

//==============================================================================
void MidiKeyboardState::reset()
{
    const ScopedLock sl (lock);
    zeromem (noteStates, sizeof (noteStates));
    eventsToAdd.clear();
}

bool MidiKeyboardState::isNoteOn (const int midiChannel, const int n) const throw()
{
    jassert (midiChannel >= 0 && midiChannel <= 16);

    return n >= 0 && n < 128
            && (noteStates[n] & (1 << (midiChannel - 1))) != 0;
}

bool MidiKeyboardState::isNoteOnForChannels (const int midiChannelMask, const int n) const throw()
{
    return n >= 0 && n < 128
            && (noteStates[n] & midiChannelMask) != 0;
}

void MidiKeyboardState::noteOn (const int midiChannel, const int midiNoteNumber, const float velocity)
{
    jassert (midiChannel >= 0 && midiChannel <= 16);
    jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);

    const ScopedLock sl (lock);

    if (midiNoteNumber >= 0 && midiNoteNumber < 128)
    {
        const int timeNow = (int) Time::getMillisecondCounter();
        eventsToAdd.addEvent (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity), timeNow);
        eventsToAdd.clear (0, timeNow - 500);

        noteOnInternal (midiChannel, midiNoteNumber, velocity);
    }
}

void MidiKeyboardState::noteOnInternal  (const int midiChannel, const int midiNoteNumber, const float velocity)
{
    if (midiNoteNumber >= 0 && midiNoteNumber < 128)
    {
        noteStates [midiNoteNumber] |= (1 << (midiChannel - 1));

        for (int i = listeners.size(); --i >= 0;)
            ((MidiKeyboardStateListener*) listeners.getUnchecked(i))
                ->handleNoteOn (this, midiChannel, midiNoteNumber, velocity);
    }
}

void MidiKeyboardState::noteOff (const int midiChannel, const int midiNoteNumber)
{
    const ScopedLock sl (lock);

    if (isNoteOn (midiChannel, midiNoteNumber))
    {
        const int timeNow = (int) Time::getMillisecondCounter();
        eventsToAdd.addEvent (MidiMessage::noteOff (midiChannel, midiNoteNumber), timeNow);
        eventsToAdd.clear (0, timeNow - 500);

        noteOffInternal (midiChannel, midiNoteNumber);
    }
}

void MidiKeyboardState::noteOffInternal  (const int midiChannel, const int midiNoteNumber)
{
    if (isNoteOn (midiChannel, midiNoteNumber))
    {
        noteStates [midiNoteNumber] &= ~(1 << (midiChannel - 1));

        for (int i = listeners.size(); --i >= 0;)
            ((MidiKeyboardStateListener*) listeners.getUnchecked(i))
                ->handleNoteOff (this, midiChannel, midiNoteNumber);
    }
}

void MidiKeyboardState::allNotesOff (const int midiChannel)
{
    const ScopedLock sl (lock);

    if (midiChannel <= 0)
    {
        for (int i = 1; i <= 16; ++i)
            allNotesOff (i);
    }
    else
    {
        for (int i = 0; i < 128; ++i)
            noteOff (midiChannel, i);
    }
}

void MidiKeyboardState::processNextMidiEvent (const MidiMessage& message)
{
    if (message.isNoteOn())
    {
        noteOnInternal (message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }
    else if (message.isNoteOff())
    {
        noteOffInternal (message.getChannel(), message.getNoteNumber());
    }
    else if (message.isAllNotesOff())
    {
        for (int i = 0; i < 128; ++i)
            noteOffInternal (message.getChannel(), i);
    }
}

void MidiKeyboardState::processNextMidiBuffer (MidiBuffer& buffer,
                                               const int startSample,
                                               const int numSamples,
                                               const bool injectIndirectEvents)
{
    MidiBuffer::Iterator i (buffer);
    MidiMessage message (0xf4, 0.0);
    int time;

    const ScopedLock sl (lock);

    while (i.getNextEvent (message, time))
        processNextMidiEvent (message);

    if (injectIndirectEvents)
    {
        MidiBuffer::Iterator i2 (eventsToAdd);
        const int firstEventToAdd = eventsToAdd.getFirstEventTime();
        const double scaleFactor = numSamples / (double) (eventsToAdd.getLastEventTime() + 1 - firstEventToAdd);

        while (i2.getNextEvent (message, time))
        {
            const int pos = jlimit (0, numSamples - 1, roundDoubleToInt ((time - firstEventToAdd) * scaleFactor));
            buffer.addEvent (message, startSample + pos);
        }
    }

    eventsToAdd.clear();
}

//==============================================================================
void MidiKeyboardState::addListener (MidiKeyboardStateListener* const listener) throw()
{
    const ScopedLock sl (lock);
    listeners.addIfNotAlreadyThere (listener);
}

void MidiKeyboardState::removeListener (MidiKeyboardStateListener* const listener) throw()
{
    const ScopedLock sl (lock);
    listeners.removeValue (listener);
}


END_JUCE_NAMESPACE
