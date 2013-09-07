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

MidiMessageSequence::MidiMessageSequence()
{
}

MidiMessageSequence::MidiMessageSequence (const MidiMessageSequence& other)
{
    list.ensureStorageAllocated (other.list.size());

    for (int i = 0; i < other.list.size(); ++i)
        list.add (new MidiEventHolder (other.list.getUnchecked(i)->message));
}

MidiMessageSequence& MidiMessageSequence::operator= (const MidiMessageSequence& other)
{
    MidiMessageSequence otherCopy (other);
    swapWith (otherCopy);
    return *this;
}

void MidiMessageSequence::swapWith (MidiMessageSequence& other) noexcept
{
    list.swapWith (other.list);
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
    if (const MidiEventHolder* const meh = list [index])
        if (meh->noteOffObject != nullptr)
            return meh->noteOffObject->message.getTimeStamp();

    return 0.0;
}

int MidiMessageSequence::getIndexOfMatchingKeyUp (const int index) const
{
    if (const MidiEventHolder* const meh = list [index])
        return list.indexOf (meh->noteOffObject);

    return -1;
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
    return getEventTime (0);
}

double MidiMessageSequence::getEndTime() const
{
    return getEventTime (list.size() - 1);
}

double MidiMessageSequence::getEventTime (const int index) const
{
    if (const MidiEventHolder* const meh = list [index])
        return meh->message.getTimeStamp();

    return 0.0;
}

//==============================================================================
MidiMessageSequence::MidiEventHolder* MidiMessageSequence::addEvent (const MidiMessage& newMessage,
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
    return newOne;
}

void MidiMessageSequence::deleteEvent (const int index,
                                       const bool deleteMatchingNoteUp)
{
    if (isPositiveAndBelow (index, list.size()))
    {
        if (deleteMatchingNoteUp)
            deleteEvent (getIndexOfMatchingKeyUp (index), false);

        list.remove (index);
    }
}

struct MidiMessageSequenceSorter
{
    static int compareElements (const MidiMessageSequence::MidiEventHolder* const first,
                                const MidiMessageSequence::MidiEventHolder* const second) noexcept
    {
        const double diff = first->message.getTimeStamp() - second->message.getTimeStamp();
        return (diff > 0) - (diff < 0);
    }
};

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
void MidiMessageSequence::sort()
{
    MidiMessageSequenceSorter sorter;
    list.sort (sorter, true);
}

void MidiMessageSequence::updateMatchedPairs()
{
    for (int i = 0; i < list.size(); ++i)
    {
        MidiEventHolder* const meh = list.getUnchecked(i);
        const MidiMessage& m1 = meh->message;

        if (m1.isNoteOn())
        {
            meh->noteOffObject = nullptr;
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
                        meh->noteOffObject = list[j];
                        break;
                    }
                    else if (m.isNoteOn())
                    {
                        MidiEventHolder* const newEvent = new MidiEventHolder (MidiMessage::noteOff (chan, note));
                        list.insert (j, newEvent);
                        newEvent->message.setTimeStamp (m.getTimeStamp());
                        meh->noteOffObject = newEvent;
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
    {
        MidiMessage& mm = list.getUnchecked(i)->message;
        mm.setTimeStamp (mm.getTimeStamp() + delta);
    }
}

//==============================================================================
void MidiMessageSequence::extractMidiChannelMessages (const int channelNumberToExtract,
                                                      MidiMessageSequence& destSequence,
                                                      const bool alsoIncludeMetaEvents) const
{
    for (int i = 0; i < list.size(); ++i)
    {
        const MidiMessage& mm = list.getUnchecked(i)->message;

        if (mm.isForChannel (channelNumberToExtract) || (alsoIncludeMetaEvents && mm.isMetaEvent()))
            destSequence.addEvent (mm);
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
    Array<int> doneControllers;
    doneControllers.ensureStorageAllocated (32);

    for (int i = list.size(); --i >= 0;)
    {
        const MidiMessage& mm = list.getUnchecked(i)->message;

        if (mm.isForChannel (channelNumber) && mm.getTimeStamp() <= time)
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
MidiMessageSequence::MidiEventHolder::MidiEventHolder (const MidiMessage& mm)
   : message (mm), noteOffObject (nullptr)
{
}

MidiMessageSequence::MidiEventHolder::~MidiEventHolder()
{
}
