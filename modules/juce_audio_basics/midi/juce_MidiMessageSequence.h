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

#ifndef JUCE_MIDIMESSAGESEQUENCE_H_INCLUDED
#define JUCE_MIDIMESSAGESEQUENCE_H_INCLUDED


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
    MidiMessageSequence (const MidiMessageSequence&);

    /** Replaces this sequence with another one. */
    MidiMessageSequence& operator= (const MidiMessageSequence&);

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

        /** The message itself, whose timestamp is used to specify the event's time. */
        MidiMessage message;

        /** The matching note-off event (if this is a note-on event).

            If this isn't a note-on, this pointer will be nullptr.

            Use the MidiMessageSequence::updateMatchedPairs() method to keep these
            note-offs up-to-date after events have been moved around in the sequence
            or deleted.
        */
        MidiEventHolder* noteOffObject;

    private:
        //==============================================================================
        friend class MidiMessageSequence;
        MidiEventHolder (const MidiMessage&);
        JUCE_LEAK_DETECTOR (MidiEventHolder)
    };

    //==============================================================================
    /** Clears the sequence. */
    void clear();

    /** Returns the number of events in the sequence. */
    int getNumEvents() const noexcept;

    /** Returns a pointer to one of the events. */
    MidiEventHolder* getEventPointer (int index) const noexcept;

    /** Returns the time of the note-up that matches the note-on at this index.
        If the event at this index isn't a note-on, it'll just return 0.
        @see MidiMessageSequence::MidiEventHolder::noteOffObject
    */
    double getTimeOfMatchingKeyUp (int index) const noexcept;

    /** Returns the index of the note-up that matches the note-on at this index.
        If the event at this index isn't a note-on, it'll just return -1.
        @see MidiMessageSequence::MidiEventHolder::noteOffObject
    */
    int getIndexOfMatchingKeyUp (int index) const noexcept;

    /** Returns the index of an event. */
    int getIndexOf (MidiEventHolder* event) const noexcept;

    /** Returns the index of the first event on or after the given timestamp.
        If the time is beyond the end of the sequence, this will return the
        number of events.
    */
    int getNextIndexAtTime (double timeStamp) const noexcept;

    //==============================================================================
    /** Returns the timestamp of the first event in the sequence.
        @see getEndTime
    */
    double getStartTime() const noexcept;

    /** Returns the timestamp of the last event in the sequence.
        @see getStartTime
    */
    double getEndTime() const noexcept;

    /** Returns the timestamp of the event at a given index.
        If the index is out-of-range, this will return 0.0
    */
    double getEventTime (int index) const noexcept;

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
    MidiEventHolder* addEvent (const MidiMessage& newMessage,
                               double timeAdjustment = 0);

    /** Deletes one of the events in the sequence.

        Remember to call updateMatchedPairs() after removing events.

        @param index                 the index of the event to delete
        @param deleteMatchingNoteUp  whether to also remove the matching note-off
                                     if the event you're removing is a note-on
    */
    void deleteEvent (int index, bool deleteMatchingNoteUp);

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

    /** Merges another sequence into this one.
        Remember to call updateMatchedPairs() after using this method.

        @param other                    the sequence to add from
        @param timeAdjustmentDelta      an amount to add to the timestamps of the midi events
                                        as they are read from the other sequence
    */
    void addSequence (const MidiMessageSequence& other,
                      double timeAdjustmentDelta);

    //==============================================================================
    /** Makes sure all the note-on and note-off pairs are up-to-date.

        Call this after re-ordering messages or deleting/adding messages, and it
        will scan the list and make sure all the note-offs in the MidiEventHolder
        structures are pointing at the correct ones.
    */
    void updateMatchedPairs() noexcept;

    /** Forces a sort of the sequence.
        You may need to call this if you've manually modified the timestamps of some
        events such that the overall order now needs updating.
    */
    void sort() noexcept;

    //==============================================================================
    /** Copies all the messages for a particular midi channel to another sequence.

        @param channelNumberToExtract   the midi channel to look for, in the range 1 to 16
        @param destSequence             the sequence that the chosen events should be copied to
        @param alsoIncludeMetaEvents    if true, any meta-events (which don't apply to a specific
                                        channel) will also be copied across.
        @see extractSysExMessages
    */
    void extractMidiChannelMessages (int channelNumberToExtract,
                                     MidiMessageSequence& destSequence,
                                     bool alsoIncludeMetaEvents) const;

    /** Copies all midi sys-ex messages to another sequence.
        @param destSequence     this is the sequence to which any sys-exes in this sequence
                                will be added
        @see extractMidiChannelMessages
    */
    void extractSysExMessages (MidiMessageSequence& destSequence) const;

    /** Removes any messages in this sequence that have a specific midi channel.
        @param channelNumberToRemove    the midi channel to look for, in the range 1 to 16
    */
    void deleteMidiChannelMessages (int channelNumberToRemove);

    /** Removes any sys-ex messages from this sequence. */
    void deleteSysExMessages();

    /** Adds an offset to the timestamps of all events in the sequence.
        @param deltaTime    the amount to add to each timestamp.
    */
    void addTimeToMessages (double deltaTime) noexcept;

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
    void createControllerUpdatesForTime (int channelNumber, double time,
                                         Array<MidiMessage>& resultMessages);

    //==============================================================================
    /** Swaps this sequence with another one. */
    void swapWith (MidiMessageSequence&) noexcept;

private:
    //==============================================================================
    friend class MidiFile;
    OwnedArray<MidiEventHolder> list;

    JUCE_LEAK_DETECTOR (MidiMessageSequence)
};


#endif   // JUCE_MIDIMESSAGESEQUENCE_H_INCLUDED
