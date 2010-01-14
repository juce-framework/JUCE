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

#include "juce_MidiMessage.h"
#include "../../containers/juce_MemoryBlock.h"


//==============================================================================
int MidiMessage::readVariableLengthVal (const uint8* data,
                                        int& numBytesUsed) throw()
{
    numBytesUsed = 0;
    int v = 0;
    int i;

    do
    {
        i = (int) *data++;

        if (++numBytesUsed > 6)
            break;

        v = (v << 7) + (i & 0x7f);

    } while (i & 0x80);

    return v;
}

int MidiMessage::getMessageLengthFromFirstByte (const uint8 firstByte) throw()
{
    // this method only works for valid starting bytes of a short midi message
    jassert (firstByte >= 0x80
              && firstByte != 0xf0
              && firstByte != 0xf7);

    static const char messageLengths[] =
    {
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        1, 2, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    return messageLengths [firstByte & 0x7f];
}

//==============================================================================
MidiMessage::MidiMessage (const uint8* const d,
                          const int dataSize,
                          const double t) throw()
   : timeStamp (t),
     message (0),
     size (dataSize)
{
    jassert (dataSize > 0);

    if (dataSize <= 4)
        data = (uint8*) &message;
    else
        data = (uint8*) juce_malloc (dataSize);

    memcpy (data, d, dataSize);

    // check that the length matches the data..
    jassert (size > 3 || *d >= 0xf0 || getMessageLengthFromFirstByte (*d) == size);
}

MidiMessage::MidiMessage (const int byte1,
                          const double t) throw()
   : timeStamp (t),
     data ((uint8*) &message),
     size (1)
{
    data[0] = (uint8) byte1;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 1);
}

MidiMessage::MidiMessage (const int byte1,
                          const int byte2,
                          const double t) throw()
   : timeStamp (t),
     data ((uint8*) &message),
     size (2)
{
    data[0] = (uint8) byte1;
    data[1] = (uint8) byte2;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 2);
}

MidiMessage::MidiMessage (const int byte1,
                          const int byte2,
                          const int byte3,
                          const double t) throw()
   : timeStamp (t),
     data ((uint8*) &message),
     size (3)
{
    data[0] = (uint8) byte1;
    data[1] = (uint8) byte2;
    data[2] = (uint8) byte3;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 3);
}

MidiMessage::MidiMessage (const MidiMessage& other) throw()
   : timeStamp (other.timeStamp),
     message (other.message),
     size (other.size)
{
    if (other.data != (uint8*) &other.message)
    {
        data = (uint8*) juce_malloc (size);
        memcpy (data, other.data, size);
    }
    else
    {
        data = (uint8*) &message;
    }
}

MidiMessage::MidiMessage (const MidiMessage& other,
                          const double newTimeStamp) throw()
   : timeStamp (newTimeStamp),
     message (other.message),
     size (other.size)
{
    if (other.data != (uint8*) &other.message)
    {
        data = (uint8*) juce_malloc (size);
        memcpy (data, other.data, size);
    }
    else
    {
        data = (uint8*) &message;
    }
}

MidiMessage::MidiMessage (const uint8* src,
                          int sz,
                          int& numBytesUsed,
                          const uint8 lastStatusByte,
                          double t) throw()
    : timeStamp (t),
      data ((uint8*) &message),
      message (0)
{
    unsigned int byte = (unsigned int) *src;

    if (byte < 0x80)
    {
        byte = (unsigned int) (uint8) lastStatusByte;
        numBytesUsed = -1;
    }
    else
    {
        numBytesUsed = 0;
        --sz;
        ++src;
    }

    if (byte >= 0x80)
    {
        if (byte == 0xf0)
        {
            const uint8* d = (const uint8*) src;

            while (d < src + sz)
            {
                if (*d >= 0x80) // stop if we hit a status byte, and don't include it in this message
                {
                    if (*d == 0xf7)   // include an 0xf7 if we hit one
                        ++d;

                    break;
                }

                ++d;
            }

            size = 1 + (int) (d - src);

            data = (uint8*) juce_malloc (size);
            *data = (uint8) byte;
            memcpy (data + 1, src, size - 1);
        }
        else if (byte == 0xff)
        {
            int n;
            const int bytesLeft = readVariableLengthVal (src + 1, n);
            size = jmin (sz + 1, n + 2 + bytesLeft);

            data = (uint8*) juce_malloc (size);
            *data = (uint8) byte;
            memcpy (data + 1, src, size - 1);
        }
        else
        {
            size = getMessageLengthFromFirstByte ((uint8) byte);
            *data = (uint8) byte;

            if (size > 1)
            {
                data[1] = src[0];

                if (size > 2)
                    data[2] = src[1];
            }
        }

        numBytesUsed += size;
    }
    else
    {
        message = 0;
        size = 0;
    }
}

const MidiMessage& MidiMessage::operator= (const MidiMessage& other) throw()
{
    if (this != &other)
    {
        timeStamp = other.timeStamp;
        size = other.size;
        message = other.message;

        if (data != (uint8*) &message)
            juce_free (data);

        if (other.data != (uint8*) &other.message)
        {
            data = (uint8*) juce_malloc (size);
            memcpy (data, other.data, size);
        }
        else
        {
            data = (uint8*) &message;
        }
    }

    return *this;
}

MidiMessage::~MidiMessage() throw()
{
    if (data != (uint8*) &message)
        juce_free (data);
}

int MidiMessage::getChannel() const throw()
{
    if ((data[0] & 0xf0) != 0xf0)
        return (data[0] & 0xf) + 1;
    else
        return 0;
}

bool MidiMessage::isForChannel (const int channel) const throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    return ((data[0] & 0xf) == channel - 1)
             && ((data[0] & 0xf0) != 0xf0);
}

void MidiMessage::setChannel (const int channel) throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    if ((data[0] & 0xf0) != (uint8) 0xf0)
        data[0] = (uint8) ((data[0] & (uint8)0xf0)
                            | (uint8)(channel - 1));
}

bool MidiMessage::isNoteOn (const bool returnTrueForVelocity0) const throw()
{
    return ((data[0] & 0xf0) == 0x90)
             && (returnTrueForVelocity0 || data[2] != 0);
}

bool MidiMessage::isNoteOff (const bool returnTrueForNoteOnVelocity0) const throw()
{
    return ((data[0] & 0xf0) == 0x80)
            || (returnTrueForNoteOnVelocity0 && (data[2] == 0) && ((data[0] & 0xf0) == 0x90));
}

bool MidiMessage::isNoteOnOrOff() const throw()
{
    const int d = data[0] & 0xf0;
    return (d == 0x90) || (d == 0x80);
}

int MidiMessage::getNoteNumber() const throw()
{
    return data[1];
}

void MidiMessage::setNoteNumber (const int newNoteNumber) throw()
{
    if (isNoteOnOrOff())
        data[1] = (uint8) jlimit (0, 127, newNoteNumber);
}

uint8 MidiMessage::getVelocity() const throw()
{
    if (isNoteOnOrOff())
        return data[2];
    else
        return 0;
}

float MidiMessage::getFloatVelocity() const throw()
{
    return getVelocity() * (1.0f / 127.0f);
}

void MidiMessage::setVelocity (const float newVelocity) throw()
{
    if (isNoteOnOrOff())
        data[2] = (uint8) jlimit (0, 0x7f, roundToInt (newVelocity * 127.0f));
}

void MidiMessage::multiplyVelocity (const float scaleFactor) throw()
{
    if (isNoteOnOrOff())
        data[2] = (uint8) jlimit (0, 0x7f, roundToInt (scaleFactor * data[2]));
}

bool MidiMessage::isAftertouch() const throw()
{
    return (data[0] & 0xf0) == 0xa0;
}

int MidiMessage::getAfterTouchValue() const throw()
{
    return data[2];
}

const MidiMessage MidiMessage::aftertouchChange (const int channel,
                                                 const int noteNum,
                                                 const int aftertouchValue) throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (((unsigned int) noteNum) <= 127);
    jassert (((unsigned int) aftertouchValue) <= 127);

    return MidiMessage (0xa0 | jlimit (0, 15, channel - 1),
                        noteNum & 0x7f,
                        aftertouchValue & 0x7f);
}

bool MidiMessage::isChannelPressure() const throw()
{
    return (data[0] & 0xf0) == 0xd0;
}

int MidiMessage::getChannelPressureValue() const throw()
{
    jassert (isChannelPressure());

    return data[1];
}

const MidiMessage MidiMessage::channelPressureChange (const int channel,
                                                      const int pressure) throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (((unsigned int) pressure) <= 127);

    return MidiMessage (0xd0 | jlimit (0, 15, channel - 1),
                        pressure & 0x7f);
}

bool MidiMessage::isProgramChange() const throw()
{
    return (data[0] & 0xf0) == 0xc0;
}

int MidiMessage::getProgramChangeNumber() const throw()
{
    return data[1];
}

const MidiMessage MidiMessage::programChange (const int channel,
                                              const int programNumber) throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    return MidiMessage (0xc0 | jlimit (0, 15, channel - 1),
                        programNumber & 0x7f);
}

bool MidiMessage::isPitchWheel() const throw()
{
    return (data[0] & 0xf0) == 0xe0;
}

int MidiMessage::getPitchWheelValue() const throw()
{
    return data[1] | (data[2] << 7);
}

const MidiMessage MidiMessage::pitchWheel (const int channel,
                                           const int position) throw()
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (((unsigned int) position) <= 0x3fff);

    return MidiMessage (0xe0 | jlimit (0, 15, channel - 1),
                        position & 127,
                        (position >> 7) & 127);
}

bool MidiMessage::isController() const throw()
{
    return (data[0] & 0xf0) == 0xb0;
}

int MidiMessage::getControllerNumber() const throw()
{
    jassert (isController());

    return data[1];
}

int MidiMessage::getControllerValue() const throw()
{
    jassert (isController());

    return data[2];
}

const MidiMessage MidiMessage::controllerEvent (const int channel,
                                                const int controllerType,
                                                const int value) throw()
{
    // the channel must be between 1 and 16 inclusive
    jassert (channel > 0 && channel <= 16);

    return MidiMessage (0xb0 | jlimit (0, 15, channel - 1),
                        controllerType & 127,
                        value & 127);
}

const MidiMessage MidiMessage::noteOn (const int channel,
                                       const int noteNumber,
                                       const float velocity) throw()
{
    return noteOn (channel, noteNumber, (uint8)(velocity * 127.0f));
}

const MidiMessage MidiMessage::noteOn (const int channel,
                                       const int noteNumber,
                                       const uint8 velocity) throw()
{
    jassert (channel > 0 && channel <= 16);
    jassert (((unsigned int) noteNumber) <= 127);

    return MidiMessage (0x90 | jlimit (0, 15, channel - 1),
                        noteNumber & 127,
                        jlimit (0, 127, roundToInt (velocity)));
}

const MidiMessage MidiMessage::noteOff (const int channel,
                                        const int noteNumber) throw()
{
    jassert (channel > 0 && channel <= 16);
    jassert (((unsigned int) noteNumber) <= 127);

    return MidiMessage (0x80 | jlimit (0, 15, channel - 1), noteNumber & 127, 0);
}

const MidiMessage MidiMessage::allNotesOff (const int channel) throw()
{
    jassert (channel > 0 && channel <= 16);

    return controllerEvent (channel, 123, 0);
}

bool MidiMessage::isAllNotesOff() const throw()
{
    return (data[0] & 0xf0) == 0xb0
            && data[1] == 123;
}

const MidiMessage MidiMessage::allSoundOff (const int channel) throw()
{
    return controllerEvent (channel, 120, 0);
}

bool MidiMessage::isAllSoundOff() const throw()
{
    return (data[0] & 0xf0) == 0xb0
             && data[1] == 120;
}

const MidiMessage MidiMessage::allControllersOff (const int channel) throw()
{
    return controllerEvent (channel, 121, 0);
}

const MidiMessage MidiMessage::masterVolume (const float volume) throw()
{
    const int vol = jlimit (0, 0x3fff, roundToInt (volume * 0x4000));

    uint8 buf[8];
    buf[0] = 0xf0;
    buf[1] = 0x7f;
    buf[2] = 0x7f;
    buf[3] = 0x04;
    buf[4] = 0x01;
    buf[5] = (uint8) (vol & 0x7f);
    buf[6] = (uint8) (vol >> 7);
    buf[7] = 0xf7;

    return MidiMessage (buf, 8);
}

//==============================================================================
bool MidiMessage::isSysEx() const throw()
{
    return *data == 0xf0;
}

const MidiMessage MidiMessage::createSysExMessage (const uint8* sysexData,
                                                   const int dataSize) throw()
{
    MemoryBlock mm (dataSize + 2);
    uint8* const m = (uint8*) mm.getData();

    m[0] = 0xf0;
    memcpy (m + 1, sysexData, dataSize);
    m[dataSize + 1] = 0xf7;

    return MidiMessage (m, dataSize + 2);
}

const uint8* MidiMessage::getSysExData() const throw()
{
    return (isSysEx()) ? getRawData() + 1
                       : 0;
}

int MidiMessage::getSysExDataSize() const throw()
{
    return (isSysEx()) ? size - 2
                       : 0;
}

bool MidiMessage::isMetaEvent() const throw()
{
    return *data == 0xff;
}

bool MidiMessage::isActiveSense() const throw()
{
    return *data == 0xfe;
}

//==============================================================================
int MidiMessage::getMetaEventType() const throw()
{
    if (*data != 0xff)
        return -1;
    else
        return data[1];
}

int MidiMessage::getMetaEventLength() const throw()
{
    if (*data == 0xff)
    {
        int n;
        return jmin (size - 2, readVariableLengthVal (data + 2, n));
    }

    return 0;
}

const uint8* MidiMessage::getMetaEventData() const throw()
{
    int n;
    const uint8* d = data + 2;
    readVariableLengthVal (d, n);
    return d + n;
}

bool MidiMessage::isTrackMetaEvent() const throw()
{
    return getMetaEventType() == 0;
}

bool MidiMessage::isEndOfTrackMetaEvent() const throw()
{
    return getMetaEventType() == 47;
}

bool MidiMessage::isTextMetaEvent() const throw()
{
    const int t = getMetaEventType();

    return t > 0 && t < 16;
}

const String MidiMessage::getTextFromTextMetaEvent() const throw()
{
    return String ((const char*) getMetaEventData(),
                   getMetaEventLength());
}

bool MidiMessage::isTrackNameEvent() const throw()
{
    return (data[1] == 3)
            && (*data == 0xff);
}

bool MidiMessage::isTempoMetaEvent() const throw()
{
    return (data[1] == 81)
            && (*data == 0xff);
}

bool MidiMessage::isMidiChannelMetaEvent() const throw()
{
    return (data[1] == 0x20)
            && (*data == 0xff)
            && (data[2] == 1);
}

int MidiMessage::getMidiChannelMetaEventChannel() const throw()
{
    return data[3] + 1;
}

double MidiMessage::getTempoSecondsPerQuarterNote() const throw()
{
    if (! isTempoMetaEvent())
        return 0.0;

    const uint8* const d = getMetaEventData();

    return (((unsigned int) d[0] << 16)
             | ((unsigned int) d[1] << 8)
             | d[2])
            / 1000000.0;
}

double MidiMessage::getTempoMetaEventTickLength (const short timeFormat) const throw()
{
    if (timeFormat > 0)
    {
        if (! isTempoMetaEvent())
            return 0.5 / timeFormat;

        return getTempoSecondsPerQuarterNote() / timeFormat;
    }
    else
    {
        const int frameCode = (-timeFormat) >> 8;
        double framesPerSecond;

        switch (frameCode)
        {
            case 24: framesPerSecond = 24.0;   break;
            case 25: framesPerSecond = 25.0;   break;
            case 29: framesPerSecond = 29.97;  break;
            case 30: framesPerSecond = 30.0;   break;
            default: framesPerSecond = 30.0;   break;
        }

        return (1.0 / framesPerSecond) / (timeFormat & 0xff);
    }
}

const MidiMessage MidiMessage::tempoMetaEvent (int microsecondsPerQuarterNote) throw()
{
    uint8 d[8];
    d[0] = 0xff;
    d[1] = 81;
    d[2] = 3;
    d[3] = (uint8) (microsecondsPerQuarterNote >> 16);
    d[4] = (uint8) ((microsecondsPerQuarterNote >> 8) & 0xff);
    d[5] = (uint8) (microsecondsPerQuarterNote & 0xff);

    return MidiMessage (d, 6, 0.0);
}

bool MidiMessage::isTimeSignatureMetaEvent() const throw()
{
    return (data[1] == 0x58)
             && (*data == (uint8) 0xff);
}

void MidiMessage::getTimeSignatureInfo (int& numerator,
                                        int& denominator) const throw()
{
    if (isTimeSignatureMetaEvent())
    {
        const uint8* const d = getMetaEventData();
        numerator = d[0];
        denominator = 1 << d[1];
    }
    else
    {
        numerator = 4;
        denominator = 4;
    }
}

const MidiMessage MidiMessage::timeSignatureMetaEvent (const int numerator,
                                                       const int denominator) throw()
{
    uint8 d[8];
    d[0] = 0xff;
    d[1] = 0x58;
    d[2] = 0x04;
    d[3] = (uint8) numerator;

    int n = 1;
    int powerOfTwo = 0;

    while (n < denominator)
    {
        n <<= 1;
        ++powerOfTwo;
    }

    d[4] = (uint8) powerOfTwo;
    d[5] = 0x01;
    d[6] = 96;

    return MidiMessage (d, 7, 0.0);
}

const MidiMessage MidiMessage::midiChannelMetaEvent (const int channel) throw()
{
    uint8 d[8];
    d[0] = 0xff;
    d[1] = 0x20;
    d[2] = 0x01;
    d[3] = (uint8) jlimit (0, 0xff, channel - 1);

    return MidiMessage (d, 4, 0.0);
}

bool MidiMessage::isKeySignatureMetaEvent() const throw()
{
    return getMetaEventType() == 89;
}

int MidiMessage::getKeySignatureNumberOfSharpsOrFlats() const throw()
{
    return (int) *getMetaEventData();
}

const MidiMessage MidiMessage::endOfTrack() throw()
{
    return MidiMessage (0xff, 0x2f, 0, 0.0);
}

//==============================================================================
bool MidiMessage::isSongPositionPointer() const throw()
{
    return *data == 0xf2;
}

int MidiMessage::getSongPositionPointerMidiBeat() const throw()
{
    return data[1] | (data[2] << 7);
}

const MidiMessage MidiMessage::songPositionPointer (const int positionInMidiBeats) throw()
{
    return MidiMessage (0xf2,
                        positionInMidiBeats & 127,
                        (positionInMidiBeats >> 7) & 127);
}

bool MidiMessage::isMidiStart() const throw()
{
    return *data == 0xfa;
}

const MidiMessage MidiMessage::midiStart() throw()
{
    return MidiMessage (0xfa);
}

bool MidiMessage::isMidiContinue() const throw()
{
    return *data == 0xfb;
}

const MidiMessage MidiMessage::midiContinue() throw()
{
    return MidiMessage (0xfb);
}

bool MidiMessage::isMidiStop() const throw()
{
    return *data == 0xfc;
}

const MidiMessage MidiMessage::midiStop() throw()
{
    return MidiMessage (0xfc);
}

bool MidiMessage::isMidiClock() const throw()
{
    return *data == 0xf8;
}

const MidiMessage MidiMessage::midiClock() throw()
{
    return MidiMessage (0xf8);
}

bool MidiMessage::isQuarterFrame() const throw()
{
    return *data == 0xf1;
}

int MidiMessage::getQuarterFrameSequenceNumber() const throw()
{
    return ((int) data[1]) >> 4;
}

int MidiMessage::getQuarterFrameValue() const throw()
{
    return ((int) data[1]) & 0x0f;
}

const MidiMessage MidiMessage::quarterFrame (const int sequenceNumber,
                                             const int value) throw()
{
    return MidiMessage (0xf1, (sequenceNumber << 4) | value);
}

bool MidiMessage::isFullFrame() const throw()
{
    return data[0] == 0xf0
            && data[1] == 0x7f
            && size >= 10
            && data[3] == 0x01
            && data[4] == 0x01;
}

void MidiMessage::getFullFrameParameters (int& hours,
                                          int& minutes,
                                          int& seconds,
                                          int& frames,
                                          MidiMessage::SmpteTimecodeType& timecodeType) const throw()
{
    jassert (isFullFrame());

    timecodeType = (SmpteTimecodeType) (data[5] >> 5);
    hours   = data[5] & 0x1f;
    minutes = data[6];
    seconds = data[7];
    frames  = data[8];
}

const MidiMessage MidiMessage::fullFrame (const int hours,
                                          const int minutes,
                                          const int seconds,
                                          const int frames,
                                          MidiMessage::SmpteTimecodeType timecodeType)
{
    uint8 d[10];
    d[0] = 0xf0;
    d[1] = 0x7f;
    d[2] = 0x7f;
    d[3] = 0x01;
    d[4] = 0x01;
    d[5] = (uint8) ((hours & 0x01f) | (timecodeType << 5));
    d[6] = (uint8) minutes;
    d[7] = (uint8) seconds;
    d[8] = (uint8) frames;
    d[9] = 0xf7;

    return MidiMessage (d, 10, 0.0);
}

bool MidiMessage::isMidiMachineControlMessage() const throw()
{
    return data[0] == 0xf0
        && data[1] == 0x7f
        && data[3] == 0x06
        && size > 5;
}

MidiMessage::MidiMachineControlCommand MidiMessage::getMidiMachineControlCommand() const throw()
{
    jassert (isMidiMachineControlMessage());

    return (MidiMachineControlCommand) data[4];
}

const MidiMessage MidiMessage::midiMachineControlCommand (MidiMessage::MidiMachineControlCommand command)
{
    uint8 d[6];
    d[0] = 0xf0;
    d[1] = 0x7f;
    d[2] = 0x00;
    d[3] = 0x06;
    d[4] = (uint8) command;
    d[5] = 0xf7;

    return MidiMessage (d, 6, 0.0);
}

//==============================================================================
bool MidiMessage::isMidiMachineControlGoto (int& hours,
                                            int& minutes,
                                            int& seconds,
                                            int& frames) const throw()
{
    if (size >= 12
         && data[0] == 0xf0
         && data[1] == 0x7f
         && data[3] == 0x06
         && data[4] == 0x44
         && data[5] == 0x06
         && data[6] == 0x01)
    {
        hours = data[7] % 24;   // (that some machines send out hours > 24)
        minutes = data[8];
        seconds = data[9];
        frames = data[10];

        return true;
    }

    return false;
}

const MidiMessage MidiMessage::midiMachineControlGoto (int hours,
                                                       int minutes,
                                                       int seconds,
                                                       int frames)
{
    uint8 d[12];
    d[0] = 0xf0;
    d[1] = 0x7f;
    d[2] = 0x00;
    d[3] = 0x06;
    d[4] = 0x44;
    d[5] = 0x06;
    d[6] = 0x01;
    d[7] = (uint8) hours;
    d[8] = (uint8) minutes;
    d[9] = (uint8) seconds;
    d[10] = (uint8) frames;
    d[11] = 0xf7;

    return MidiMessage (d, 12, 0.0);
}

//==============================================================================
const String MidiMessage::getMidiNoteName (int note,
                                           bool useSharps,
                                           bool includeOctaveNumber,
                                           int octaveNumForMiddleC) throw()
{
    static const char* const sharpNoteNames[] = { "C", "C#", "D", "D#", "E",
                                                  "F", "F#", "G", "G#", "A",
                                                  "A#", "B" };

    static const char* const flatNoteNames[]  = { "C", "Db", "D", "Eb", "E",
                                                  "F", "Gb", "G", "Ab", "A",
                                                  "Bb", "B" };

    if (((unsigned int) note) < 128)
    {
        const String s ((useSharps) ? sharpNoteNames [note % 12]
                                    : flatNoteNames [note % 12]);

        if (includeOctaveNumber)
            return s + String (note / 12 + (octaveNumForMiddleC - 5));
        else
            return s;
    }

    return String::empty;
}

const double MidiMessage::getMidiNoteInHertz (int noteNumber) throw()
{
    noteNumber -= 12 * 6 + 9; // now 0 = A440
    return 440.0 * pow (2.0, noteNumber / 12.0);
}

const String MidiMessage::getGMInstrumentName (int n) throw()
{
    const char *names[] =
    {
        "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano",
        "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavinet", "Celesta", "Glockenspiel",
        "Music Box", "Vibraphone", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer", "Drawbar Organ",
        "Percussive Organ", "Rock Organ", "Church Organ", "Reed Organ", "Accordion", "Harmonica",
        "Tango Accordion", "Acoustic Guitar (nylon)", "Acoustic Guitar (steel)", "Electric Guitar (jazz)",
        "Electric Guitar (clean)", "Electric Guitar (mute)", "Overdriven Guitar", "Distortion Guitar",
        "Guitar Harmonics", "Acoustic Bass", "Electric Bass (finger)", "Electric Bass (pick)",
        "Fretless Bass", "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2", "Violin",
        "Viola", "Cello", "Contrabass", "Tremolo Strings", "Pizzicato Strings", "Orchestral Harp",
        "Timpani", "String Ensemble 1", "String Ensemble 2", "SynthStrings 1", "SynthStrings 2",
        "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit", "Trumpet", "Trombone", "Tuba",
        "Muted Trumpet", "French Horn", "Brass Section", "SynthBrass 1", "SynthBrass 2", "Soprano Sax",
        "Alto Sax", "Tenor Sax", "Baritone Sax", "Oboe", "English Horn", "Bassoon", "Clarinet",
        "Piccolo", "Flute", "Recorder", "Pan Flute", "Blown Bottle", "Shakuhachi", "Whistle",
        "Ocarina", "Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
        "Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass+lead)", "Pad 1 (new age)",
        "Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)", "Pad 5 (bowed)", "Pad 6 (metallic)",
        "Pad 7 (halo)", "Pad 8 (sweep)", "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)",
        "FX 4 (atmosphere)", "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
        "Sitar", "Banjo", "Shamisen", "Koto", "Kalimba", "Bag pipe", "Fiddle", "Shanai", "Tinkle Bell",
        "Agogo", "Steel Drums", "Woodblock", "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal",
        "Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet", "Telephone Ring", "Helicopter",
        "Applause", "Gunshot"
    };

    return (((unsigned int) n) < 128) ? names[n]
                                      : (const char*)0;
}

const String MidiMessage::getGMInstrumentBankName (int n) throw()
{
    const char* names[] =
    {
        "Piano", "Chromatic Percussion", "Organ", "Guitar",
        "Bass", "Strings", "Ensemble", "Brass",
        "Reed", "Pipe", "Synth Lead", "Synth Pad",
        "Synth Effects", "Ethnic", "Percussive", "Sound Effects"
    };

    return (((unsigned int) n) <= 15) ? names[n]
                                      : (const char*)0;
}

const String MidiMessage::getRhythmInstrumentName (int n) throw()
{
    const char* names[] =
    {
        "Acoustic Bass Drum", "Bass Drum 1", "Side Stick", "Acoustic Snare",
        "Hand Clap", "Electric Snare", "Low Floor Tom", "Closed Hi-Hat", "High Floor Tom",
        "Pedal Hi-Hat", "Low Tom", "Open Hi-Hat", "Low-Mid Tom", "Hi-Mid Tom", "Crash Cymbal 1",
        "High Tom", "Ride Cymbal 1", "Chinese Cymbal", "Ride Bell", "Tambourine", "Splash Cymbal",
        "Cowbell", "Crash Cymbal 2", "Vibraslap", "Ride Cymbal 2", "Hi Bongo", "Low Bongo",
        "Mute Hi Conga", "Open Hi Conga", "Low Conga", "High Timbale", "Low Timbale", "High Agogo",
        "Low Agogo", "Cabasa", "Maracas", "Short Whistle", "Long Whistle", "Short Guiro",
        "Long Guiro", "Claves", "Hi Wood Block", "Low Wood Block", "Mute Cuica", "Open Cuica",
        "Mute Triangle", "Open Triangle"
    };

    return (n >= 35 && n <= 81) ? names [n - 35]
                                : (const char*)0;
}

const String MidiMessage::getControllerName (int n) throw()
{
    const char* names[] =
    {
        "Bank Select", "Modulation Wheel (coarse)", "Breath controller (coarse)",
        0, "Foot Pedal (coarse)", "Portamento Time (coarse)",
        "Data Entry (coarse)", "Volume (coarse)", "Balance (coarse)",
        0, "Pan position (coarse)", "Expression (coarse)", "Effect Control 1 (coarse)",
        "Effect Control 2 (coarse)", 0, 0, "General Purpose Slider 1", "General Purpose Slider 2",
        "General Purpose Slider 3", "General Purpose Slider 4", 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, "Bank Select (fine)", "Modulation Wheel (fine)", "Breath controller (fine)",
        0, "Foot Pedal (fine)", "Portamento Time (fine)", "Data Entry (fine)", "Volume (fine)",
        "Balance (fine)", 0, "Pan position (fine)", "Expression (fine)", "Effect Control 1 (fine)",
        "Effect Control 2 (fine)", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        "Hold Pedal (on/off)", "Portamento (on/off)", "Sustenuto Pedal (on/off)", "Soft Pedal (on/off)",
        "Legato Pedal (on/off)", "Hold 2 Pedal (on/off)", "Sound Variation", "Sound Timbre",
        "Sound Release Time", "Sound Attack Time", "Sound Brightness", "Sound Control 6",
        "Sound Control 7", "Sound Control 8", "Sound Control 9", "Sound Control 10",
        "General Purpose Button 1 (on/off)", "General Purpose Button 2 (on/off)",
        "General Purpose Button 3 (on/off)", "General Purpose Button 4 (on/off)",
        0, 0, 0, 0, 0, 0, 0, "Reverb Level", "Tremolo Level",  "Chorus Level", "Celeste Level",
        "Phaser Level", "Data Button increment", "Data Button decrement", "Non-registered Parameter (fine)",
        "Non-registered Parameter (coarse)", "Registered Parameter (fine)", "Registered Parameter (coarse)",
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "All Sound Off", "All Controllers Off",
        "Local Keyboard (on/off)", "All Notes Off", "Omni Mode Off", "Omni Mode On", "Mono Operation",
        "Poly Operation"
    };

    return (((unsigned int) n) < 128) ? names[n]
                                      : (const char*)0;
}

END_JUCE_NAMESPACE
