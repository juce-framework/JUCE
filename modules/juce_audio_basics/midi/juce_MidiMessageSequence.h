/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A sequence of timestamped midi messages.

    This allows the sequence to be manipulated, and also to be read from and
    written to a standard midi file.

    @see MidiMessage, MidiFile

    @tags{Audio}
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

    /** Move constructor */
    MidiMessageSequence (MidiMessageSequence&&) noexcept;

    /** Move assignment operator */
    MidiMessageSequence& operator= (MidiMessageSequence&&) noexcept;

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
        /** The message itself, whose timestamp is used to specify the event's time. */
        MidiMessage message;

        /** The matching note-off event (if this is a note-on event).

            If this isn't a note-on, this pointer will be nullptr.

            Use the MidiMessageSequence::updateMatchedPairs() method to keep these
            note-offs up-to-date after events have been moved around in the sequence
            or deleted.
        */
        MidiEventHolder* noteOffObject = nullptr;

    private:
        //==============================================================================
        friend class MidiMessageSequence;
        MidiEventHolder (const MidiMessage&);
        MidiEventHolder (MidiMessage&&);
        JUCE_LEAK_DETECTOR (MidiEventHolder)
    };

    //==============================================================================
    /** Clears the sequence. */
    void clear();

    /** Returns the number of events in the sequence. */
    int getNumEvents() const noexcept;

    /** Returns a pointer to one of the events. */
    MidiEventHolder* getEventPointer (int index) const noexcept;

    /** Iterator for the list of MidiEventHolders */
    MidiEventHolder** begin() noexcept;

    /** Iterator for the list of MidiEventHolders */
    MidiEventHolder* const* begin() const noexcept;

    /** Iterator for the list of MidiEventHolders */
    MidiEventHolder** end() noexcept;

    /** Iterator for the list of MidiEventHolders */
    MidiEventHolder* const* end() const noexcept;

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
    int getIndexOf (const MidiEventHolder* event) const noexcept;

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
    MidiEventHolder* addEvent (const MidiMessage& newMessage, double timeAdjustment = 0);

    /** Inserts a midi message into the sequence.

        The index at which the new message gets inserted will depend on its timestamp,
        because the sequence is kept sorted.

        Remember to call updateMatchedPairs() after adding note-on events.

        @param newMessage       the new message to add (an internal copy will be made)
        @param timeAdjustment   an optional value to add to the timestamp of the message
                                that will be inserted
        @see updateMatchedPairs
    */
    MidiEventHolder* addEvent (MidiMessage&& newMessage, double timeAdjustment = 0);

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

        @param regardNoteOnEventWithVel0AsNoteOff   if true, note-on events with velocity 0
                                                    will be regarded as note-off
    */
    void updateMatchedPairs(bool regardNoteOnEventsWithVel0AsNoteOff = true) noexcept;

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

        This function has special handling for the "bank select" and "data entry"
        controllers (0x00, 0x20, 0x06, 0x26, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65).

        If the sequence contains multiple bank select and program change messages,
        only the bank select messages immediately preceding the final program change
        message will be kept.

        All "data increment" and "data decrement" messages will be retained. Some hardware will
        ignore the requested increment/decrement values, so retaining all messages is the only
        way to ensure compatibility with all hardware.

        "Parameter number" changes will be slightly condensed. Only the parameter number
        events immediately preceding each data entry event will be kept. The parameter number
        will also be set to its final value at the end of the sequence, if necessary.

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

    MidiEventHolder* addEvent (MidiEventHolder*, double);

    JUCE_LEAK_DETECTOR (MidiMessageSequence)
};

} // namespace juce
