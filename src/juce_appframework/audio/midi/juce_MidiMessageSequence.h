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

#ifndef __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
#define __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__

#include "juce_MidiMessage.h"
#include "../../../juce_core/containers/juce_OwnedArray.h"


//==============================================================================
/**
    A sequence of timestamped midi messages.

    This allows the sequence to be manipulated, and also to be read from and
    written to a standard midi file.

    @see MidiMessage, MidiFile
*/
class JUCE_API  MidiMessageSequence
{
public:
    //==============================================================================
    /** Creates an empty midi sequence object. */
    MidiMessageSequence();

    /** Creates a copy of another sequence. */
    MidiMessageSequence (const MidiMessageSequence& other);

    /** Replaces this sequence with another one. */
    const MidiMessageSequence& operator= (const MidiMessageSequence& other);

    /** Destructor. */
    ~MidiMessageSequence();

    //==============================================================================
    /** Structure used to hold midi events in the sequence.

        These structures act as 'handles' on the events as they are moved about in
        the list, and make it quick to find the matching note-offs for note-on events.

        @see MidiMessageSequence::getEventPointer
    */
    class MidiEventHolder
    {
    public:
        //==============================================================================
        /** Destructor. */
        ~MidiEventHolder();

        /** The message itself, whose timestamp is used to specify the event's time.
        */
        MidiMessage message;

        /** The matching note-off event (if this is a note-on event).

            If this isn't a note-on, this pointer will be null.

            Use the MidiMessageSequence::updateMatchedPairs() method to keep these
            note-offs up-to-date after events have been moved around in the sequence
            or deleted.
        */
        MidiEventHolder* noteOffObject;


        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        friend class MidiMessageSequence;
        MidiEventHolder (const MidiMessage& message);
    };

    //==============================================================================
    /** Clears the sequence. */
    void clear();

    /** Returns the number of events in the sequence. */
    int getNumEvents() const;

    /** Returns a pointer to one of the events. */
    MidiEventHolder* getEventPointer (const int index) const;

    /** Returns the time of the note-up that matches the note-on at this index.

        If the event at this index isn't a note-on, it'll just return 0.

        @see MidiMessageSequence::MidiEventHolder::noteOffObject
    */
    double getTimeOfMatchingKeyUp (const int index) const;

    /** Returns the index of the note-up that matches the note-on at this index.

        If the event at this index isn't a note-on, it'll just return -1.

        @see MidiMessageSequence::MidiEventHolder::noteOffObject
    */
    int getIndexOfMatchingKeyUp (const int index) const;

    /** Returns the index of an event. */
    int getIndexOf (MidiEventHolder* const event) const;

    /** Returns the index of the first event on or after the given timestamp.

        If the time is beyond the end of the sequence, this will return the
        number of events.
    */
    int getNextIndexAtTime (const double timeStamp) const;

    //==============================================================================
    /** Returns the timestamp of the first event in the sequence.

        @see getEndTime
    */
    double getStartTime() const;

    /** Returns the timestamp of the last event in the sequence.

        @see getStartTime
    */
    double getEndTime() const;

    /** Returns the timestamp of the event at a given index.

        If the index is out-of-range, this will return 0.0
    */
    double getEventTime (const int index) const;

    //==============================================================================
    /** Inserts a midi message into the sequence.

        The index at which the new message gets inserted will depend on its timestamp,
        because the sequence is kept sorted.

        Remember to call updateMatchedPairs() after adding note-on events.

        @param newMessage       the new message to add (an internal copy will be made)
        @param timeAdjustment   an optional value to add to the timestamp of the message
                                that will be inserted
        @see updateMatchedPairs
    */
    void addEvent (const MidiMessage& newMessage,
                   double timeAdjustment = 0);

    /** Deletes one of the events in the sequence.

        Remember to call updateMatchedPairs() after removing events.

        @param index                 the index of the event to delete
        @param deleteMatchingNoteUp  whether to also remove the matching note-off
                                     if the event you're removing is a note-on
    */
    void deleteEvent (const int index,
                      const bool deleteMatchingNoteUp);

    /** Merges another sequence into this one.

        Remember to call updateMatchedPairs() after using this method.

        @param other                    the sequence to add from
        @param timeAdjustmentDelta      an amount to add to the timestamps of the midi events
                                        as they are read from the other sequence
        @param firstAllowableDestTime   events will not be added if their time is earlier
                                        than this time. (This is after their time has been adjusted
                                        by the timeAdjustmentDelta)
        @param endOfAllowableDestTimes  events will not be added if their time is equal to
                                        or greater than this time. (This is after their time has
                                        been adjusted by the timeAdjustmentDelta)
    */
    void addSequence (const MidiMessageSequence& other,
                      double timeAdjustmentDelta,
                      double firstAllowableDestTime,
                      double endOfAllowableDestTimes);

    //==============================================================================
    /** Makes sure all the note-on and note-off pairs are up-to-date.

        Call this after moving messages about or deleting/adding messages, and it
        will scan the list and make sure all the note-offs in the MidiEventHolder
        structures are pointing at the correct ones.
    */
    void updateMatchedPairs();


    //==============================================================================
    /** Copies all the messages for a particular midi channel to another sequence.

        @param channelNumberToExtract   the midi channel to look for, in the range 1 to 16
        @param destSequence             the sequence that the chosen events should be copied to
        @param alsoIncludeMetaEvents    if true, any meta-events (which don't apply to a specific
                                        channel) will also be copied across.
        @see extractSysExMessages
    */
    void extractMidiChannelMessages (const int channelNumberToExtract,
                                     MidiMessageSequence& destSequence,
                                     const bool alsoIncludeMetaEvents) const;

    /** Copies all midi sys-ex messages to another sequence.

        @param destSequence     this is the sequence to which any sys-exes in this sequence
                                will be added
        @see extractMidiChannelMessages
    */
    void extractSysExMessages (MidiMessageSequence& destSequence) const;

    /** Removes any messages in this sequence that have a specific midi channel.

        @param channelNumberToRemove    the midi channel to look for, in the range 1 to 16
    */
    void deleteMidiChannelMessages (const int channelNumberToRemove);

    /** Removes any sys-ex messages from this sequence.
    */
    void deleteSysExMessages();

    /** Adds an offset to the timestamps of all events in the sequence.

        @param deltaTime    the amount to add to each timestamp.
    */
    void addTimeToMessages (const double deltaTime);

    //==============================================================================
    /** Scans through the sequence to determine the state of any midi controllers at
        a given time.

        This will create a sequence of midi controller changes that can be
        used to set all midi controllers to the state they would be in at the
        specified time within this sequence.

        As well as controllers, it will also recreate the midi program number
        and pitch bend position.

        @param channelNumber    the midi channel to look for, in the range 1 to 16. Controllers
                                for other channels will be ignored.
        @param time             the time at which you want to find out the state - there are
                                no explicit units for this time measurement, it's the same units
                                as used for the timestamps of the messages
        @param resultMessages   an array to which midi controller-change messages will be added. This
                                will be the minimum number of controller changes to recreate the
                                state at the required time.
    */
    void createControllerUpdatesForTime (const int channelNumber,
                                         const double time,
                                         OwnedArray<MidiMessage>& resultMessages);

    //==============================================================================
    juce_UseDebuggingNewOperator

    /** @internal */
    static int compareElements (const MidiMessageSequence::MidiEventHolder* const first,
                                const MidiMessageSequence::MidiEventHolder* const second) throw();

private:
    //==============================================================================
    friend class MidiComparator;
    friend class MidiFile;
    OwnedArray <MidiEventHolder> list;

    void sort();
};


#endif   // __JUCE_MIDIMESSAGESEQUENCE_JUCEHEADER__
