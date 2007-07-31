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

#include "juce_MidiMessageSequence.h"
#include "../../../juce_core/containers/juce_Array.h"


//==============================================================================
MidiMessageSequence::MidiMessageSequence()
{
}

MidiMessageSequence::MidiMessageSequence (const MidiMessageSequence& other)
{
    list.ensureStorageAllocated (other.list.size());

    for (int i = 0; i < other.list.size(); ++i)
        list.add (new MidiEventHolder (other.list.getUnchecked(i)->message));
}

const MidiMessageSequence& MidiMessageSequence::operator= (const MidiMessageSequence& other)
{
    if (this != &other)
    {
        clear();

        for (int i = 0; i < other.list.size(); ++i)
            list.add (new MidiEventHolder (other.list.getUnchecked(i)->message));
    }

    return *this;
}

MidiMessageSequence::~MidiMessageSequence()
{
}

void MidiMessageSequence::clear()
{
    list.clear();
}

int MidiMessageSequence::getNumEvents() const
{
    return list.size();
}

MidiMessageSequence::MidiEventHolder* MidiMessageSequence::getEventPointer (const int index) const
{
    return list [index];
}

double MidiMessageSequence::getTimeOfMatchingKeyUp (const int index) const
{
    const MidiEventHolder* const meh = list [index];

    if (meh != 0 && meh->noteOffObject != 0)
        return meh->noteOffObject->message.getTimeStamp();
    else
        return 0.0;
}

int MidiMessageSequence::getIndexOfMatchingKeyUp (const int index) const
{
    const MidiEventHolder* const meh = list [index];

    return (meh != 0) ? list.indexOf (meh->noteOffObject) : -1;
}

int MidiMessageSequence::getIndexOf (MidiEventHolder* const event) const
{
    return list.indexOf (event);
}

int MidiMessageSequence::getNextIndexAtTime (const double timeStamp) const
{
    const int numEvents = list.size();

    int i;
    for (i = 0; i < numEvents; ++i)
        if (list.getUnchecked(i)->message.getTimeStamp() >= timeStamp)
            break;

    return i;
}

//==============================================================================
double MidiMessageSequence::getStartTime() const
{
    if (list.size() > 0)
        return list.getUnchecked(0)->message.getTimeStamp();
    else
        return 0;
}

double MidiMessageSequence::getEndTime() const
{
    if (list.size() > 0)
        return list.getLast()->message.getTimeStamp();
    else
        return 0;
}

double MidiMessageSequence::getEventTime (const int index) const
{
    if (index < 0 || index >= list.size())
        return 0.0;
    else
        return list.getUnchecked (index)->message.getTimeStamp();
}

//==============================================================================
void MidiMessageSequence::addEvent (const MidiMessage& newMessage,
                                    double timeAdjustment)
{
    MidiEventHolder* const newOne = new MidiEventHolder (newMessage);

    timeAdjustment += newMessage.getTimeStamp();
    newOne->message.setTimeStamp (timeAdjustment);

    int i;
    for (i = list.size(); --i >= 0;)
        if (list.getUnchecked(i)->message.getTimeStamp() <= timeAdjustment)
            break;

    list.insert (i + 1, newOne);
}

void MidiMessageSequence::deleteEvent (const int index,
                                       const bool deleteMatchingNoteUp)
{
    if (index >= 0 && index < list.size())
    {
        if (deleteMatchingNoteUp)
            deleteEvent (getIndexOfMatchingKeyUp (index), false);

        list.remove (index);
    }
}

void MidiMessageSequence::addSequence (const MidiMessageSequence& other,
                                       double timeAdjustment,
                                       double firstAllowableTime,
                                       double endOfAllowableDestTimes)
{
    firstAllowableTime -= timeAdjustment;
    endOfAllowableDestTimes -= timeAdjustment;

    for (int i = 0; i < other.list.size(); ++i)
    {
        const MidiMessage& m = other.list.getUnchecked(i)->message;
        const double t = m.getTimeStamp();

        if (t >= firstAllowableTime && t < endOfAllowableDestTimes)
        {
            MidiEventHolder* const newOne = new MidiEventHolder (m);
            newOne->message.setTimeStamp (timeAdjustment + t);

            list.add (newOne);
        }
    }

    sort();
}

//==============================================================================
int MidiMessageSequence::compareElements (const MidiMessageSequence::MidiEventHolder* const first,
                                          const MidiMessageSequence::MidiEventHolder* const second) throw()
{
    const double diff = first->message.getTimeStamp()
                         - second->message.getTimeStamp();

    return (diff == 0) ? 0
                       : ((diff > 0) ? 1
                                     : -1);
}

void MidiMessageSequence::sort()
{
    list.sort (*this, true);
}

//==============================================================================
void MidiMessageSequence::updateMatchedPairs()
{
    for (int i = 0; i < list.size(); ++i)
    {
        const MidiMessage& m1 = list.getUnchecked(i)->message;

        if (m1.isNoteOn())
        {
            list.getUnchecked(i)->noteOffObject = 0;
            const int note = m1.getNoteNumber();
            const int chan = m1.getChannel();
            const int len = list.size();

            for (int j = i + 1; j < len; ++j)
            {
                const MidiMessage& m = list.getUnchecked(j)->message;

                if (m.getNoteNumber() == note && m.getChannel() == chan)
                {
                    if (m.isNoteOff())
                    {
                        list.getUnchecked(i)->noteOffObject = list[j];
                        break;
                    }
                    else if (m.isNoteOn())
                    {
                        list.insert (j, new MidiEventHolder (MidiMessage::noteOff (chan, note)));
                        list.getUnchecked(j)->message.setTimeStamp (m.getTimeStamp());
                        list.getUnchecked(i)->noteOffObject = list[j];
                        break;
                    }
                }
            }
        }
    }
}

void MidiMessageSequence::addTimeToMessages (const double delta)
{
    for (int i = list.size(); --i >= 0;)
        list.getUnchecked (i)->message.setTimeStamp (list.getUnchecked (i)->message.getTimeStamp()
                                                      + delta);
}

//==============================================================================
void MidiMessageSequence::extractMidiChannelMessages (const int channelNumberToExtract,
                                                      MidiMessageSequence& destSequence,
                                                      const bool alsoIncludeMetaEvents) const
{
    for (int i = 0; i < list.size(); ++i)
    {
        const MidiMessage& mm = list.getUnchecked(i)->message;

        if (mm.isForChannel (channelNumberToExtract)
             || (alsoIncludeMetaEvents && mm.isMetaEvent()))
        {
            destSequence.addEvent (mm);
        }
    }
}

void MidiMessageSequence::extractSysExMessages (MidiMessageSequence& destSequence) const
{
    for (int i = 0; i < list.size(); ++i)
    {
        const MidiMessage& mm = list.getUnchecked(i)->message;

        if (mm.isSysEx())
            destSequence.addEvent (mm);
    }
}

void MidiMessageSequence::deleteMidiChannelMessages (const int channelNumberToRemove)
{
    for (int i = list.size(); --i >= 0;)
        if (list.getUnchecked(i)->message.isForChannel (channelNumberToRemove))
            list.remove(i);
}

void MidiMessageSequence::deleteSysExMessages()
{
    for (int i = list.size(); --i >= 0;)
        if (list.getUnchecked(i)->message.isSysEx())
            list.remove(i);
}

//==============================================================================
void MidiMessageSequence::createControllerUpdatesForTime (const int channelNumber,
                                                          const double time,
                                                          OwnedArray<MidiMessage>& dest)
{
    bool doneProg = false;
    bool donePitchWheel = false;
    Array <int> doneControllers (32);

    for (int i = list.size(); --i >= 0;)
    {
        const MidiMessage& mm = list.getUnchecked(i)->message;

        if (mm.isForChannel (channelNumber)
             && mm.getTimeStamp() <= time)
        {
            if (mm.isProgramChange())
            {
                if (! doneProg)
                {
                    dest.add (new MidiMessage (mm, 0.0));
                    doneProg = true;
                }
            }
            else if (mm.isController())
            {
                if (! doneControllers.contains (mm.getControllerNumber()))
                {
                    dest.add (new MidiMessage (mm, 0.0));
                    doneControllers.add (mm.getControllerNumber());
                }
            }
            else if (mm.isPitchWheel())
            {
                if (! donePitchWheel)
                {
                    dest.add (new MidiMessage (mm, 0.0));
                    donePitchWheel = true;
                }
            }
        }
    }
}


//==============================================================================
MidiMessageSequence::MidiEventHolder::MidiEventHolder (const MidiMessage& message_)
   : message (message_),
     noteOffObject (0)
{
}

MidiMessageSequence::MidiEventHolder::~MidiEventHolder()
{
}

END_JUCE_NAMESPACE
