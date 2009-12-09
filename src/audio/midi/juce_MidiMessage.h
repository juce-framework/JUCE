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

#ifndef __JUCE_MIDIMESSAGE_JUCEHEADER__
#define __JUCE_MIDIMESSAGE_JUCEHEADER__

#include "../../text/juce_String.h"


//==============================================================================
/**
    Encapsulates a MIDI message.

    @see MidiMessageSequence, MidiOutput, MidiInput
*/
class JUCE_API  MidiMessage
{
public:
    //==============================================================================
    /** Creates a 3-byte short midi message.

        @param byte1            message byte 1
        @param byte2            message byte 2
        @param byte3            message byte 3
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (const int byte1,
                 const int byte2,
                 const int byte3,
                 const double timeStamp = 0) throw();

    /** Creates a 2-byte short midi message.

        @param byte1            message byte 1
        @param byte2            message byte 2
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (const int byte1,
                 const int byte2,
                 const double timeStamp = 0) throw();

    /** Creates a 1-byte short midi message.

        @param byte1            message byte 1
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (const int byte1,
                 const double timeStamp = 0) throw();

    /** Creates a midi message from a block of data. */
    MidiMessage (const uint8* const data,
                 const int dataSize,
                 const double timeStamp = 0) throw();

    /** Reads the next midi message from some data.

        This will read as many bytes from a data stream as it needs to make a
        complete message, and will return the number of bytes it used. This lets
        you read a sequence of midi messages from a file or stream.

        @param data             the data to read from
        @param size             the maximum number of bytes it's allowed to read
        @param numBytesUsed     returns the number of bytes that were actually needed
        @param lastStatusByte   in a sequence of midi messages, the initial byte
                                can be dropped from a message if it's the same as the
                                first byte of the previous message, so this lets you
                                supply the byte to use if the first byte of the message
                                has in fact been dropped.
        @param timeStamp        the time to give the midi message - this value doesn't
                                use any particular units, so will be application-specific
    */
    MidiMessage (const uint8* data,
                 int size,
                 int& numBytesUsed,
                 uint8 lastStatusByte,
                 double timeStamp = 0) throw();

    /** Creates a copy of another midi message. */
    MidiMessage (const MidiMessage& other) throw();

    /** Creates a copy of another midi message, with a different timestamp. */
    MidiMessage (const MidiMessage& other,
                 const double newTimeStamp) throw();

    /** Destructor. */
    ~MidiMessage() throw();

    /** Copies this message from another one. */
    const MidiMessage& operator= (const MidiMessage& other) throw();

    //==============================================================================
    /** Returns a pointer to the raw midi data.

        @see getRawDataSize
    */
    uint8* getRawData() const throw()                           { return data; }

    /** Returns the number of bytes of data in the message.

        @see getRawData
    */
    int getRawDataSize() const throw()                          { return size; }

    //==============================================================================
    /** Returns the timestamp associated with this message.

        The exact meaning of this time and its units will vary, as messages are used in
        a variety of different contexts.

        If you're getting the message from a midi file, this could be a time in seconds, or
        a number of ticks - see MidiFile::convertTimestampTicksToSeconds().

        If the message is being used in a MidiBuffer, it might indicate the number of
        audio samples from the start of the buffer.

        If the message was created by a MidiInput, see MidiInputCallback::handleIncomingMidiMessage()
        for details of the way that it initialises this value.

        @see setTimeStamp, addToTimeStamp
    */
    double getTimeStamp() const throw()                         { return timeStamp; }

    /** Changes the message's associated timestamp.

        The units for the timestamp will be application-specific - see the notes for getTimeStamp().

        @see addToTimeStamp, getTimeStamp
    */
    void setTimeStamp (const double newTimestamp) throw()       { timeStamp = newTimestamp; }

    /** Adds a value to the message's timestamp.

        The units for the timestamp will be application-specific.
    */
    void addToTimeStamp (const double delta) throw()            { timeStamp += delta; }

    //==============================================================================
    /** Returns the midi channel associated with the message.

        @returns    a value 1 to 16 if the message has a channel, or 0 if it hasn't (e.g.
                    if it's a sysex)
        @see isForChannel, setChannel
    */
    int getChannel() const throw();

    /** Returns true if the message applies to the given midi channel.

        @param channelNumber    the channel number to look for, in the range 1 to 16
        @see getChannel, setChannel
    */
    bool isForChannel (const int channelNumber) const throw();

    /** Changes the message's midi channel.

        This won't do anything for non-channel messages like sysexes.

        @param newChannelNumber    the channel number to change it to, in the range 1 to 16
    */
    void setChannel (const int newChannelNumber) throw();

    //==============================================================================
    /** Returns true if this is a system-exclusive message.
    */
    bool isSysEx() const throw();

    /** Returns a pointer to the sysex data inside the message.

        If this event isn't a sysex event, it'll return 0.

        @see getSysExDataSize
    */
    const uint8* getSysExData() const throw();

    /** Returns the size of the sysex data.

        This value excludes the 0xf0 header byte and the 0xf7 at the end.

        @see getSysExData
    */
    int getSysExDataSize() const throw();

    //==============================================================================
    /** Returns true if this message is a 'key-down' event.

        @param returnTrueForVelocity0   if true, then if this event is a note-on with
                        velocity 0, it will still be considered to be a note-on and the
                        method will return true. If returnTrueForVelocity0 is false, then
                        if this is a note-on event with velocity 0, it'll be regarded as
                        a note-off, and the method will return false

        @see isNoteOff, getNoteNumber, getVelocity, noteOn
    */
    bool isNoteOn (const bool returnTrueForVelocity0 = false) const throw();

    /** Creates a key-down message (using a floating-point velocity).

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 1.0
        @see isNoteOn
    */
    static const MidiMessage noteOn (const int channel,
                                     const int noteNumber,
                                     const float velocity) throw();

    /** Creates a key-down message (using an integer velocity).

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @param velocity     in the range 0 to 127
        @see isNoteOn
    */
    static const MidiMessage noteOn (const int channel,
                                     const int noteNumber,
                                     const uint8 velocity) throw();

    /** Returns true if this message is a 'key-up' event.

        If returnTrueForNoteOnVelocity0 is true, then his will also return true
        for a note-on event with a velocity of 0.

        @see isNoteOn, getNoteNumber, getVelocity, noteOff
    */
    bool isNoteOff (const bool returnTrueForNoteOnVelocity0 = true) const throw();

    /** Creates a key-up message.

        @param channel      the midi channel, in the range 1 to 16
        @param noteNumber   the key number, 0 to 127
        @see isNoteOff
    */
    static const MidiMessage noteOff (const int channel,
                                      const int noteNumber) throw();

    /** Returns true if this message is a 'key-down' or 'key-up' event.

        @see isNoteOn, isNoteOff
    */
    bool isNoteOnOrOff() const throw();

    /** Returns the midi note number for note-on and note-off messages.

        If the message isn't a note-on or off, the value returned will be
        meaningless.

        @see isNoteOff, getMidiNoteName, getMidiNoteInHertz, setNoteNumber
    */
    int getNoteNumber() const throw();

    /** Changes the midi note number of a note-on or note-off message.

        If the message isn't a note on or off, this will do nothing.
    */
    void setNoteNumber (const int newNoteNumber) throw();

    //==============================================================================
    /** Returns the velocity of a note-on or note-off message.

        The value returned will be in the range 0 to 127.

        If the message isn't a note-on or off event, it will return 0.

        @see getFloatVelocity
    */
    uint8 getVelocity() const throw();

    /** Returns the velocity of a note-on or note-off message.

        The value returned will be in the range 0 to 1.0

        If the message isn't a note-on or off event, it will return 0.

        @see getVelocity, setVelocity
    */
    float getFloatVelocity() const throw();

    /** Changes the velocity of a note-on or note-off message.

        If the message isn't a note on or off, this will do nothing.

        @param newVelocity  the new velocity, in the range 0 to 1.0
        @see getFloatVelocity, multiplyVelocity
    */
    void setVelocity (const float newVelocity) throw();

    /** Multiplies the velocity of a note-on or note-off message by a given amount.

        If the message isn't a note on or off, this will do nothing.

        @param scaleFactor  the value by which to multiply the velocity
        @see setVelocity
    */
    void multiplyVelocity (const float scaleFactor) throw();

    //==============================================================================
    /** Returns true if the message is a program (patch) change message.

        @see getProgramChangeNumber, getGMInstrumentName
    */
    bool isProgramChange() const throw();

    /** Returns the new program number of a program change message.

        If the message isn't a program change, the value returned will be
        nonsense.

        @see isProgramChange, getGMInstrumentName
    */
    int getProgramChangeNumber() const throw();

    /** Creates a program-change message.

        @param channel          the midi channel, in the range 1 to 16
        @param programNumber    the midi program number, 0 to 127
        @see isProgramChange, getGMInstrumentName
    */
    static const MidiMessage programChange (const int channel,
                                            const int programNumber) throw();

    //==============================================================================
    /** Returns true if the message is a pitch-wheel move.

        @see getPitchWheelValue, pitchWheel
    */
    bool isPitchWheel() const throw();

    /** Returns the pitch wheel position from a pitch-wheel move message.

        The value returned is a 14-bit number from 0 to 0x3fff, indicating the wheel position.
        If called for messages which aren't pitch wheel events, the number returned will be
        nonsense.

        @see isPitchWheel
    */
    int getPitchWheelValue() const throw();

    /** Creates a pitch-wheel move message.

        @param channel      the midi channel, in the range 1 to 16
        @param position     the wheel position, in the range 0 to 16383
        @see isPitchWheel
    */
    static const MidiMessage pitchWheel (const int channel,
                                         const int position) throw();

    //==============================================================================
    /** Returns true if the message is an aftertouch event.

        For aftertouch events, use the getNoteNumber() method to find out the key
        that it applies to, and getAftertouchValue() to find out the amount. Use
        getChannel() to find out the channel.

        @see getAftertouchValue, getNoteNumber
    */
    bool isAftertouch() const throw();

    /** Returns the amount of aftertouch from an aftertouch messages.

        The value returned is in the range 0 to 127, and will be nonsense for messages
        other than aftertouch messages.

        @see isAftertouch
    */
    int getAfterTouchValue() const throw();

    /** Creates an aftertouch message.

        @param channel              the midi channel, in the range 1 to 16
        @param noteNumber           the key number, 0 to 127
        @param aftertouchAmount     the amount of aftertouch, 0 to 127
        @see isAftertouch
    */
    static const MidiMessage aftertouchChange (const int channel,
                                               const int noteNumber,
                                               const int aftertouchAmount) throw();

    /** Returns true if the message is a channel-pressure change event.

        This is like aftertouch, but common to the whole channel rather than a specific
        note. Use getChannelPressureValue() to find out the pressure, and getChannel()
        to find out the channel.

        @see channelPressureChange
    */
    bool isChannelPressure() const throw();

    /** Returns the pressure from a channel pressure change message.

        @returns the pressure, in the range 0 to 127
        @see isChannelPressure, channelPressureChange
    */
    int getChannelPressureValue() const throw();

    /** Creates a channel-pressure change event.

        @param channel              the midi channel: 1 to 16
        @param pressure             the pressure, 0 to 127
        @see isChannelPressure
    */
    static const MidiMessage channelPressureChange (const int channel,
                                                    const int pressure) throw();

    //==============================================================================
    /** Returns true if this is a midi controller message.

        @see getControllerNumber, getControllerValue, controllerEvent
    */
    bool isController() const throw();

    /** Returns the controller number of a controller message.

        The name of the controller can be looked up using the getControllerName() method.

        Note that the value returned is invalid for messages that aren't controller changes.

        @see isController, getControllerName, getControllerValue
    */
    int getControllerNumber() const throw();

    /** Returns the controller value from a controller message.

        A value 0 to 127 is returned to indicate the new controller position.

        Note that the value returned is invalid for messages that aren't controller changes.

        @see isController, getControllerNumber
    */
    int getControllerValue() const throw();

    /** Creates a controller message.

        @param channel          the midi channel, in the range 1 to 16
        @param controllerType   the type of controller
        @param value            the controller value
        @see isController
    */
    static const MidiMessage controllerEvent (const int channel,
                                              const int controllerType,
                                              const int value) throw();

    /** Checks whether this message is an all-notes-off message.

        @see allNotesOff
    */
    bool isAllNotesOff() const throw();

    /** Checks whether this message is an all-sound-off message.

        @see allSoundOff
    */
    bool isAllSoundOff() const throw();

    /** Creates an all-notes-off message.

        @param channel              the midi channel, in the range 1 to 16
        @see isAllNotesOff
    */
    static const MidiMessage allNotesOff (const int channel) throw();

    /** Creates an all-sound-off message.

        @param channel              the midi channel, in the range 1 to 16
        @see isAllSoundOff
    */
    static const MidiMessage allSoundOff (const int channel) throw();

    /** Creates an all-controllers-off message.

        @param channel              the midi channel, in the range 1 to 16
    */
    static const MidiMessage allControllersOff (const int channel) throw();

    //==============================================================================
    /** Returns true if this event is a meta-event.

        Meta-events are things like tempo changes, track names, etc.

        @see getMetaEventType, isTrackMetaEvent, isEndOfTrackMetaEvent,
             isTextMetaEvent, isTrackNameEvent, isTempoMetaEvent, isTimeSignatureMetaEvent,
             isKeySignatureMetaEvent, isMidiChannelMetaEvent
    */
    bool isMetaEvent() const throw();

    /** Returns a meta-event's type number.

        If the message isn't a meta-event, this will return -1.

        @see isMetaEvent, isTrackMetaEvent, isEndOfTrackMetaEvent,
             isTextMetaEvent, isTrackNameEvent, isTempoMetaEvent, isTimeSignatureMetaEvent,
             isKeySignatureMetaEvent, isMidiChannelMetaEvent
    */
    int getMetaEventType() const throw();

    /** Returns a pointer to the data in a meta-event.

        @see isMetaEvent, getMetaEventLength
    */
    const uint8* getMetaEventData() const throw();

    /** Returns the length of the data for a meta-event.

        @see isMetaEvent, getMetaEventData
    */
    int getMetaEventLength() const throw();

    //==============================================================================
    /** Returns true if this is a 'track' meta-event. */
    bool isTrackMetaEvent() const throw();

    /** Returns true if this is an 'end-of-track' meta-event. */
    bool isEndOfTrackMetaEvent() const throw();

    /** Creates an end-of-track meta-event.

        @see isEndOfTrackMetaEvent
    */
    static const MidiMessage endOfTrack() throw();

    /** Returns true if this is an 'track name' meta-event.

        You can use the getTextFromTextMetaEvent() method to get the track's name.
    */
    bool isTrackNameEvent() const throw();

    /** Returns true if this is a 'text' meta-event.

        @see getTextFromTextMetaEvent
    */
    bool isTextMetaEvent() const throw();

    /** Returns the text from a text meta-event.

        @see isTextMetaEvent
    */
    const String getTextFromTextMetaEvent() const throw();

    //==============================================================================
    /** Returns true if this is a 'tempo' meta-event.

        @see getTempoMetaEventTickLength, getTempoSecondsPerQuarterNote
    */
    bool isTempoMetaEvent() const throw();

    /** Returns the tick length from a tempo meta-event.

        @param timeFormat   the 16-bit time format value from the midi file's header.
        @returns the tick length (in seconds).
        @see isTempoMetaEvent
    */
    double getTempoMetaEventTickLength (const short timeFormat) const throw();

    /** Calculates the seconds-per-quarter-note from a tempo meta-event.

        @see isTempoMetaEvent, getTempoMetaEventTickLength
    */
    double getTempoSecondsPerQuarterNote() const throw();

    /** Creates a tempo meta-event.

        @see isTempoMetaEvent
    */
    static const MidiMessage tempoMetaEvent (const int microsecondsPerQuarterNote) throw();

    //==============================================================================
    /** Returns true if this is a 'time-signature' meta-event.

        @see getTimeSignatureInfo
    */
    bool isTimeSignatureMetaEvent() const throw();

    /** Returns the time-signature values from a time-signature meta-event.

        @see isTimeSignatureMetaEvent
    */
    void getTimeSignatureInfo (int& numerator,
                               int& denominator) const throw();

    /** Creates a time-signature meta-event.

        @see isTimeSignatureMetaEvent
    */
    static const MidiMessage timeSignatureMetaEvent (const int numerator,
                                                     const int denominator) throw();

    //==============================================================================
    /** Returns true if this is a 'key-signature' meta-event.

        @see getKeySignatureNumberOfSharpsOrFlats
    */
    bool isKeySignatureMetaEvent() const throw();

    /** Returns the key from a key-signature meta-event.

        @see isKeySignatureMetaEvent
    */
    int getKeySignatureNumberOfSharpsOrFlats() const throw();

    //==============================================================================
    /** Returns true if this is a 'channel' meta-event.

        A channel meta-event specifies the midi channel that should be used
        for subsequent meta-events.

        @see getMidiChannelMetaEventChannel
    */
    bool isMidiChannelMetaEvent() const throw();

    /** Returns the channel number from a channel meta-event.

        @returns the channel, in the range 1 to 16.
        @see isMidiChannelMetaEvent
    */
    int getMidiChannelMetaEventChannel() const throw();

    /** Creates a midi channel meta-event.

        @param channel              the midi channel, in the range 1 to 16
        @see isMidiChannelMetaEvent
    */
    static const MidiMessage midiChannelMetaEvent (const int channel) throw();

    //==============================================================================
    /** Returns true if this is an active-sense message. */
    bool isActiveSense() const throw();

    //==============================================================================
    /** Returns true if this is a midi start event.

        @see midiStart
    */
    bool isMidiStart() const throw();

    /** Creates a midi start event. */
    static const MidiMessage midiStart() throw();

    /** Returns true if this is a midi continue event.

        @see midiContinue
    */
    bool isMidiContinue() const throw();

    /** Creates a midi continue event. */
    static const MidiMessage midiContinue() throw();

    /** Returns true if this is a midi stop event.

        @see midiStop
    */
    bool isMidiStop() const throw();

    /** Creates a midi stop event. */
    static const MidiMessage midiStop() throw();

    /** Returns true if this is a midi clock event.

        @see midiClock, songPositionPointer
    */
    bool isMidiClock() const throw();

    /** Creates a midi clock event. */
    static const MidiMessage midiClock() throw();

    /** Returns true if this is a song-position-pointer message.

        @see getSongPositionPointerMidiBeat, songPositionPointer
    */
    bool isSongPositionPointer() const throw();

    /** Returns the midi beat-number of a song-position-pointer message.

        @see isSongPositionPointer, songPositionPointer
    */
    int getSongPositionPointerMidiBeat() const throw();

    /** Creates a song-position-pointer message.

        The position is a number of midi beats from the start of the song, where 1 midi
        beat is 6 midi clocks, and there are 24 midi clocks in a quarter-note. So there
        are 4 midi beats in a quarter-note.

        @see isSongPositionPointer, getSongPositionPointerMidiBeat
    */
    static const MidiMessage songPositionPointer (const int positionInMidiBeats) throw();

    //==============================================================================
    /** Returns true if this is a quarter-frame midi timecode message.

        @see quarterFrame, getQuarterFrameSequenceNumber, getQuarterFrameValue
    */
    bool isQuarterFrame() const throw();

    /** Returns the sequence number of a quarter-frame midi timecode message.

        This will be a value between 0 and 7.

        @see isQuarterFrame, getQuarterFrameValue, quarterFrame
    */
    int getQuarterFrameSequenceNumber() const throw();

    /** Returns the value from a quarter-frame message.

        This will be the lower nybble of the message's data-byte, a value
        between 0 and 15
    */
    int getQuarterFrameValue() const throw();

    /** Creates a quarter-frame MTC message.

        @param sequenceNumber   a value 0 to 7 for the upper nybble of the message's data byte
        @param value            a value 0 to 15 for the lower nybble of the message's data byte
    */
    static const MidiMessage quarterFrame (const int sequenceNumber,
                                           const int value) throw();

    /** SMPTE timecode types.

        Used by the getFullFrameParameters() and fullFrame() methods.
    */
    enum SmpteTimecodeType
    {
        fps24       = 0,
        fps25       = 1,
        fps30drop   = 2,
        fps30       = 3
    };

    /** Returns true if this is a full-frame midi timecode message.
    */
    bool isFullFrame() const throw();

    /** Extracts the timecode information from a full-frame midi timecode message.

        You should only call this on messages where you've used isFullFrame() to
        check that they're the right kind.
    */
    void getFullFrameParameters (int& hours,
                                 int& minutes,
                                 int& seconds,
                                 int& frames,
                                 SmpteTimecodeType& timecodeType) const throw();

    /** Creates a full-frame MTC message.
    */
    static const MidiMessage fullFrame (const int hours,
                                        const int minutes,
                                        const int seconds,
                                        const int frames,
                                        SmpteTimecodeType timecodeType);

    //==============================================================================
    /** Types of MMC command.

        @see isMidiMachineControlMessage, getMidiMachineControlCommand, midiMachineControlCommand
    */
    enum MidiMachineControlCommand
    {
        mmc_stop            = 1,
        mmc_play            = 2,
        mmc_deferredplay    = 3,
        mmc_fastforward     = 4,
        mmc_rewind          = 5,
        mmc_recordStart     = 6,
        mmc_recordStop      = 7,
        mmc_pause           = 9
    };

    /** Checks whether this is an MMC message.

        If it is, you can use the getMidiMachineControlCommand() to find out its type.
    */
    bool isMidiMachineControlMessage() const throw();

    /** For an MMC message, this returns its type.

        Make sure it's actually an MMC message with isMidiMachineControlMessage() before
        calling this method.
    */
    MidiMachineControlCommand getMidiMachineControlCommand() const throw();

    /** Creates an MMC message.
    */
    static const MidiMessage midiMachineControlCommand (MidiMachineControlCommand command);

    /** Checks whether this is an MMC "goto" message.

        If it is, the parameters passed-in are set to the time that the message contains.

        @see midiMachineControlGoto
    */
    bool isMidiMachineControlGoto (int& hours,
                                   int& minutes,
                                   int& seconds,
                                   int& frames) const throw();

    /** Creates an MMC "goto" message.

        This messages tells the device to go to a specific frame.

        @see isMidiMachineControlGoto
    */
    static const MidiMessage midiMachineControlGoto (int hours,
                                                     int minutes,
                                                     int seconds,
                                                     int frames);

    //==============================================================================
    /** Creates a master-volume change message.

        @param volume   the volume, 0 to 1.0
    */
    static const MidiMessage masterVolume (const float volume) throw();

    //==============================================================================
    /** Creates a system-exclusive message.

        The data passed in is wrapped with header and tail bytes of 0xf0 and 0xf7.
    */
    static const MidiMessage createSysExMessage (const uint8* sysexData,
                                                 const int dataSize) throw();


    //==============================================================================
    /** Reads a midi variable-length integer.

        @param data             the data to read the number from
        @param numBytesUsed     on return, this will be set to the number of bytes that were read
    */
    static int readVariableLengthVal (const uint8* data,
                                      int& numBytesUsed) throw();

    /** Based on the first byte of a short midi message, this uses a lookup table
        to return the message length (either 1, 2, or 3 bytes).

        The value passed in must be 0x80 or higher.
    */
    static int getMessageLengthFromFirstByte (const uint8 firstByte) throw();

    //==============================================================================
    /** Returns the name of a midi note number.

        E.g "C", "D#", etc.

        @param noteNumber           the midi note number, 0 to 127
        @param useSharps            if true, sharpened notes are used, e.g. "C#", otherwise
                                    they'll be flattened, e.g. "Db"
        @param includeOctaveNumber  if true, the octave number will be appended to the string,
                                    e.g. "C#4"
        @param octaveNumForMiddleC  if an octave number is being appended, this indicates the
                                    number that will be used for middle C's octave

        @see getMidiNoteInHertz
    */
    static const String getMidiNoteName (int noteNumber,
                                         bool useSharps,
                                         bool includeOctaveNumber,
                                         int octaveNumForMiddleC) throw();

    /** Returns the frequency of a midi note number.

        @see getMidiNoteName
    */
    static const double getMidiNoteInHertz (int noteNumber) throw();

    /** Returns the standard name of a GM instrument.

        @param midiInstrumentNumber     the program number 0 to 127
        @see getProgramChangeNumber
    */
    static const String getGMInstrumentName (int midiInstrumentNumber) throw();

    /** Returns the name of a bank of GM instruments.

        @param midiBankNumber   the bank, 0 to 15
    */
    static const String getGMInstrumentBankName (int midiBankNumber) throw();

    /** Returns the standard name of a channel 10 percussion sound.

        @param midiNoteNumber   the key number, 35 to 81
    */
    static const String getRhythmInstrumentName (int midiNoteNumber) throw();

    /** Returns the name of a controller type number.

        @see getControllerNumber
    */
    static const String getControllerName (int controllerNumber) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    double timeStamp;
    uint8* data;
    int message, size;
};


#endif   // __JUCE_MIDIMESSAGE_JUCEHEADER__
