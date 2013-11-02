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

namespace MidiHelpers
{
    inline uint8 initialByte (const int type, const int channel) noexcept
    {
        return (uint8) (type | jlimit (0, 15, channel - 1));
    }

    inline uint8 validVelocity (const int v) noexcept
    {
        return (uint8) jlimit (0, 127, v);
    }
}

//==============================================================================
int MidiMessage::readVariableLengthVal (const uint8* data, int& numBytesUsed) noexcept
{
    numBytesUsed = 0;
    int v = 0, i;

    do
    {
        i = (int) *data++;

        if (++numBytesUsed > 6)
            break;

        v = (v << 7) + (i & 0x7f);

    } while (i & 0x80);

    return v;
}

int MidiMessage::getMessageLengthFromFirstByte (const uint8 firstByte) noexcept
{
    // this method only works for valid starting bytes of a short midi message
    jassert (firstByte >= 0x80 && firstByte != 0xf0 && firstByte != 0xf7);

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
MidiMessage::MidiMessage() noexcept
   : timeStamp (0), size (2)
{
    preallocatedData.asBytes[0] = 0xf0;
    preallocatedData.asBytes[1] = 0xf7;
}

MidiMessage::MidiMessage (const void* const d, const int dataSize, const double t)
   : timeStamp (t),
     size (dataSize)
{
    jassert (dataSize > 0);

    if (dataSize > 4)
    {
        allocatedData.malloc (dataSize);
        memcpy (allocatedData, d, (size_t) dataSize);
    }
    else
    {
        memcpy (preallocatedData.asBytes, d, (size_t) dataSize);
    }

    // check that the length matches the data..
    jassert (size > 3 || *(uint8*)d >= 0xf0 || getMessageLengthFromFirstByte (*(uint8*)d) == size);
}

MidiMessage::MidiMessage (const int byte1, const double t) noexcept
   : timeStamp (t), size (1)
{
    preallocatedData.asBytes[0] = (uint8) byte1;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 1);
}

MidiMessage::MidiMessage (const int byte1, const int byte2, const double t) noexcept
   : timeStamp (t), size (2)
{
    preallocatedData.asBytes[0] = (uint8) byte1;
    preallocatedData.asBytes[1] = (uint8) byte2;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 2);
}

MidiMessage::MidiMessage (const int byte1, const int byte2, const int byte3, const double t) noexcept
   : timeStamp (t), size (3)
{
    preallocatedData.asBytes[0] = (uint8) byte1;
    preallocatedData.asBytes[1] = (uint8) byte2;
    preallocatedData.asBytes[2] = (uint8) byte3;

    // check that the length matches the data..
    jassert (byte1 >= 0xf0 || getMessageLengthFromFirstByte ((uint8) byte1) == 3);
}

MidiMessage::MidiMessage (const MidiMessage& other)
   : timeStamp (other.timeStamp), size (other.size)
{
    if (size > 4)
    {
        allocatedData.malloc (size);
        memcpy (allocatedData, other.allocatedData, (size_t) size);
    }
    else
    {
        preallocatedData.asInt32 = other.preallocatedData.asInt32;
    }
}

MidiMessage::MidiMessage (const MidiMessage& other, const double newTimeStamp)
   : timeStamp (newTimeStamp), size (other.size)
{
    if (size > 4)
    {
        allocatedData.malloc (size);
        memcpy (allocatedData, other.allocatedData, (size_t) size);
    }
    else
    {
        preallocatedData.asInt32 = other.preallocatedData.asInt32;
    }
}

MidiMessage::MidiMessage (const void* srcData, int sz, int& numBytesUsed, const uint8 lastStatusByte, double t)
    : timeStamp (t)
{
    const uint8* src = static_cast <const uint8*> (srcData);
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
            const uint8* d = src;
            bool haveReadAllLengthBytes = false;
            int numVariableLengthSysexBytes = 0;

            while (d < src + sz)
            {
                if (*d >= 0x80)
                {
                    if (*d == 0xf7)
                    {
                        ++d;  // include the trailing 0xf7 when we hit it
                        break;
                    }

                    if (haveReadAllLengthBytes) // if we see a 0x80 bit set after the initial data length
                        break;                  // bytes, assume it's the end of the sysex

                    ++numVariableLengthSysexBytes;
                }
                else if (! haveReadAllLengthBytes)
                {
                    haveReadAllLengthBytes = true;
                    ++numVariableLengthSysexBytes;
                }

                ++d;
            }

            src += numVariableLengthSysexBytes;
            size = 1 + (int) (d - src);

            allocatedData.malloc (size);
            *allocatedData = (uint8) byte;
            memcpy (allocatedData + 1, src, (size_t) (size - 1));

            numBytesUsed += numVariableLengthSysexBytes;  // (these aren't counted in the size)
        }
        else if (byte == 0xff)
        {
            int n;
            const int bytesLeft = readVariableLengthVal (src + 1, n);
            size = jmin (sz + 1, n + 2 + bytesLeft);

            allocatedData.malloc (size);
            *allocatedData = (uint8) byte;
            memcpy (allocatedData + 1, src, (size_t) size - 1);
        }
        else
        {
            preallocatedData.asInt32 = 0;
            size = getMessageLengthFromFirstByte ((uint8) byte);
            preallocatedData.asBytes[0] = (uint8) byte;

            if (size > 1)
            {
                preallocatedData.asBytes[1] = src[0];

                if (size > 2)
                    preallocatedData.asBytes[2] = src[1];
            }
        }

        numBytesUsed += size;
    }
    else
    {
        preallocatedData.asInt32 = 0;
        size = 0;
    }
}

MidiMessage& MidiMessage::operator= (const MidiMessage& other)
{
    if (this != &other)
    {
        timeStamp = other.timeStamp;
        size = other.size;

        if (size > 4)
        {
            allocatedData.malloc (size);
            memcpy (allocatedData, other.allocatedData, (size_t) size);
        }
        else
        {
            allocatedData.free();
            preallocatedData.asInt32 = other.preallocatedData.asInt32;
        }
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
MidiMessage::MidiMessage (MidiMessage&& other) noexcept
   : timeStamp (other.timeStamp), size (other.size)
{
    if (other.allocatedData != nullptr)
        allocatedData.swapWith (other.allocatedData);
    else
        preallocatedData.asInt32 = other.preallocatedData.asInt32;
}

MidiMessage& MidiMessage::operator= (MidiMessage&& other) noexcept
{
    jassert (this != &other); // shouldn't be possible

    timeStamp = other.timeStamp;
    size = other.size;
    allocatedData.swapWith (other.allocatedData);
    preallocatedData.asInt32 = other.preallocatedData.asInt32;

    return *this;
}
#endif

MidiMessage::~MidiMessage() {}

int MidiMessage::getChannel() const noexcept
{
    const uint8* const data = getRawData();

    if ((data[0] & 0xf0) != 0xf0)
        return (data[0] & 0xf) + 1;

    return 0;
}

bool MidiMessage::isForChannel (const int channel) const noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    const uint8* const data = getRawData();

    return ((data[0] & 0xf) == channel - 1)
             && ((data[0] & 0xf0) != 0xf0);
}

void MidiMessage::setChannel (const int channel) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    uint8* const data = getData();

    if ((data[0] & 0xf0) != (uint8) 0xf0)
        data[0] = (uint8) ((data[0] & (uint8) 0xf0)
                            | (uint8)(channel - 1));
}

bool MidiMessage::isNoteOn (const bool returnTrueForVelocity0) const noexcept
{
    const uint8* const data = getRawData();

    return ((data[0] & 0xf0) == 0x90)
             && (returnTrueForVelocity0 || data[2] != 0);
}

bool MidiMessage::isNoteOff (const bool returnTrueForNoteOnVelocity0) const noexcept
{
    const uint8* const data = getRawData();

    return ((data[0] & 0xf0) == 0x80)
            || (returnTrueForNoteOnVelocity0 && (data[2] == 0) && ((data[0] & 0xf0) == 0x90));
}

bool MidiMessage::isNoteOnOrOff() const noexcept
{
    const uint8* const data = getRawData();

    const int d = data[0] & 0xf0;
    return (d == 0x90) || (d == 0x80);
}

int MidiMessage::getNoteNumber() const noexcept
{
    return getRawData()[1];
}

void MidiMessage::setNoteNumber (const int newNoteNumber) noexcept
{
    if (isNoteOnOrOff())
        getData()[1] = (uint8) (newNoteNumber & 127);
}

uint8 MidiMessage::getVelocity() const noexcept
{
    if (isNoteOnOrOff())
        return getRawData()[2];

    return 0;
}

float MidiMessage::getFloatVelocity() const noexcept
{
    return getVelocity() * (1.0f / 127.0f);
}

void MidiMessage::setVelocity (const float newVelocity) noexcept
{
    if (isNoteOnOrOff())
        getData()[2] = MidiHelpers::validVelocity (roundToInt (newVelocity * 127.0f));
}

void MidiMessage::multiplyVelocity (const float scaleFactor) noexcept
{
    if (isNoteOnOrOff())
    {
        uint8* const data = getData();
        data[2] = MidiHelpers::validVelocity (roundToInt (scaleFactor * data[2]));
    }
}

bool MidiMessage::isAftertouch() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xa0;
}

int MidiMessage::getAfterTouchValue() const noexcept
{
    jassert (isAftertouch());
    return getRawData()[2];
}

MidiMessage MidiMessage::aftertouchChange (const int channel,
                                           const int noteNum,
                                           const int aftertouchValue) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (noteNum, (int) 128));
    jassert (isPositiveAndBelow (aftertouchValue, (int) 128));

    return MidiMessage (MidiHelpers::initialByte (0xa0, channel),
                        noteNum & 0x7f,
                        aftertouchValue & 0x7f);
}

bool MidiMessage::isChannelPressure() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xd0;
}

int MidiMessage::getChannelPressureValue() const noexcept
{
    jassert (isChannelPressure());
    return getRawData()[1];
}

MidiMessage MidiMessage::channelPressureChange (const int channel, const int pressure) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (pressure, (int) 128));

    return MidiMessage (MidiHelpers::initialByte (0xd0, channel), pressure & 0x7f);
}

bool MidiMessage::isSustainPedalOn() const noexcept     { return isControllerOfType (0x40) && getRawData()[2] >= 64; }
bool MidiMessage::isSustainPedalOff() const noexcept    { return isControllerOfType (0x40) && getRawData()[2] <  64; }

bool MidiMessage::isSostenutoPedalOn() const noexcept   { return isControllerOfType (0x42) && getRawData()[2] >= 64; }
bool MidiMessage::isSostenutoPedalOff() const noexcept  { return isControllerOfType (0x42) && getRawData()[2] <  64; }

bool MidiMessage::isSoftPedalOn() const noexcept        { return isControllerOfType (0x43) && getRawData()[2] >= 64; }
bool MidiMessage::isSoftPedalOff() const noexcept       { return isControllerOfType (0x43) && getRawData()[2] <  64; }


bool MidiMessage::isProgramChange() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xc0;
}

int MidiMessage::getProgramChangeNumber() const noexcept
{
    jassert (isProgramChange());
    return getRawData()[1];
}

MidiMessage MidiMessage::programChange (const int channel, const int programNumber) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

    return MidiMessage (MidiHelpers::initialByte (0xc0, channel), programNumber & 0x7f);
}

bool MidiMessage::isPitchWheel() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xe0;
}

int MidiMessage::getPitchWheelValue() const noexcept
{
    jassert (isPitchWheel());
    const uint8* const data = getRawData();
    return data[1] | (data[2] << 7);
}

MidiMessage MidiMessage::pitchWheel (const int channel, const int position) noexcept
{
    jassert (channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
    jassert (isPositiveAndBelow (position, (int) 0x4000));

    return MidiMessage (MidiHelpers::initialByte (0xe0, channel),
                        position & 127, (position >> 7) & 127);
}

bool MidiMessage::isController() const noexcept
{
    return (getRawData()[0] & 0xf0) == 0xb0;
}

bool MidiMessage::isControllerOfType (const int controllerType) const noexcept
{
    const uint8* const data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == controllerType;
}

int MidiMessage::getControllerNumber() const noexcept
{
    jassert (isController());
    return getRawData()[1];
}

int MidiMessage::getControllerValue() const noexcept
{
    jassert (isController());
    return getRawData()[2];
}

MidiMessage MidiMessage::controllerEvent (const int channel, const int controllerType, const int value) noexcept
{
    // the channel must be between 1 and 16 inclusive
    jassert (channel > 0 && channel <= 16);

    return MidiMessage (MidiHelpers::initialByte (0xb0, channel),
                        controllerType & 127, value & 127);
}

MidiMessage MidiMessage::noteOn (const int channel, const int noteNumber, const float velocity) noexcept
{
    return noteOn (channel, noteNumber, (uint8) (velocity * 127.0f + 0.5f));
}

MidiMessage MidiMessage::noteOn (const int channel, const int noteNumber, const uint8 velocity) noexcept
{
    jassert (channel > 0 && channel <= 16);
    jassert (isPositiveAndBelow (noteNumber, (int) 128));

    return MidiMessage (MidiHelpers::initialByte (0x90, channel),
                        noteNumber & 127, MidiHelpers::validVelocity (velocity));
}

MidiMessage MidiMessage::noteOff (const int channel, const int noteNumber, uint8 velocity) noexcept
{
    jassert (channel > 0 && channel <= 16);
    jassert (isPositiveAndBelow (noteNumber, (int) 128));

    return MidiMessage (MidiHelpers::initialByte (0x80, channel),
                        noteNumber & 127, MidiHelpers::validVelocity (velocity));
}

MidiMessage MidiMessage::allNotesOff (const int channel) noexcept
{
    return controllerEvent (channel, 123, 0);
}

bool MidiMessage::isAllNotesOff() const noexcept
{
    const uint8* const data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == 123;
}

MidiMessage MidiMessage::allSoundOff (const int channel) noexcept
{
    return controllerEvent (channel, 120, 0);
}

bool MidiMessage::isAllSoundOff() const noexcept
{
    const uint8* const data = getRawData();
    return (data[0] & 0xf0) == 0xb0 && data[1] == 120;
}

MidiMessage MidiMessage::allControllersOff (const int channel) noexcept
{
    return controllerEvent (channel, 121, 0);
}

MidiMessage MidiMessage::masterVolume (const float volume)
{
    const int vol = jlimit (0, 0x3fff, roundToInt (volume * 0x4000));

    const uint8 buf[] = { 0xf0, 0x7f, 0x7f, 0x04, 0x01,
                          (uint8) (vol & 0x7f),
                          (uint8) (vol >> 7),
                          0xf7 };

    return MidiMessage (buf, 8);
}

//==============================================================================
bool MidiMessage::isSysEx() const noexcept
{
    return *getRawData() == 0xf0;
}

MidiMessage MidiMessage::createSysExMessage (const void* sysexData, const int dataSize)
{
    HeapBlock<uint8> m ((size_t) dataSize + 2);

    m[0] = 0xf0;
    memcpy (m + 1, sysexData, (size_t) dataSize);
    m[dataSize + 1] = 0xf7;

    return MidiMessage (m, dataSize + 2);
}

const uint8* MidiMessage::getSysExData() const noexcept
{
    return isSysEx() ? getRawData() + 1 : nullptr;
}

int MidiMessage::getSysExDataSize() const noexcept
{
    return isSysEx() ? size - 2 : 0;
}

//==============================================================================
bool MidiMessage::isMetaEvent() const noexcept      { return *getRawData() == 0xff; }
bool MidiMessage::isActiveSense() const noexcept    { return *getRawData() == 0xfe; }

int MidiMessage::getMetaEventType() const noexcept
{
    const uint8* const data = getRawData();
    return *data != 0xff ? -1 : data[1];
}

int MidiMessage::getMetaEventLength() const noexcept
{
    const uint8* const data = getRawData();
    if (*data == 0xff)
    {
        int n;
        return jmin (size - 2, readVariableLengthVal (data + 2, n));
    }

    return 0;
}

const uint8* MidiMessage::getMetaEventData() const noexcept
{
    jassert (isMetaEvent());

    int n;
    const uint8* d = getRawData() + 2;
    readVariableLengthVal (d, n);
    return d + n;
}

bool MidiMessage::isTrackMetaEvent() const noexcept         { return getMetaEventType() == 0; }
bool MidiMessage::isEndOfTrackMetaEvent() const noexcept    { return getMetaEventType() == 47; }

bool MidiMessage::isTextMetaEvent() const noexcept
{
    const int t = getMetaEventType();
    return t > 0 && t < 16;
}

String MidiMessage::getTextFromTextMetaEvent() const
{
    const char* const textData = reinterpret_cast <const char*> (getMetaEventData());
    return String (CharPointer_UTF8 (textData),
                   CharPointer_UTF8 (textData + getMetaEventLength()));
}

bool MidiMessage::isTrackNameEvent() const noexcept         { const uint8* data = getRawData(); return (data[1] == 3)    && (*data == 0xff); }
bool MidiMessage::isTempoMetaEvent() const noexcept         { const uint8* data = getRawData(); return (data[1] == 81)   && (*data == 0xff); }
bool MidiMessage::isMidiChannelMetaEvent() const noexcept   { const uint8* data = getRawData(); return (data[1] == 0x20) && (*data == 0xff) && (data[2] == 1); }

int MidiMessage::getMidiChannelMetaEventChannel() const noexcept
{
    jassert (isMidiChannelMetaEvent());
    return getRawData()[3] + 1;
}

double MidiMessage::getTempoSecondsPerQuarterNote() const noexcept
{
    if (! isTempoMetaEvent())
        return 0.0;

    const uint8* const d = getMetaEventData();

    return (((unsigned int) d[0] << 16)
             | ((unsigned int) d[1] << 8)
             | d[2])
            / 1000000.0;
}

double MidiMessage::getTempoMetaEventTickLength (const short timeFormat) const noexcept
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

MidiMessage MidiMessage::tempoMetaEvent (int microsecondsPerQuarterNote) noexcept
{
    const uint8 d[] = { 0xff, 81, 3,
                        (uint8) (microsecondsPerQuarterNote >> 16),
                        (uint8) (microsecondsPerQuarterNote >> 8),
                        (uint8) microsecondsPerQuarterNote };

    return MidiMessage (d, 6, 0.0);
}

bool MidiMessage::isTimeSignatureMetaEvent() const noexcept
{
    const uint8* const data = getRawData();
    return (data[1] == 0x58) && (*data == (uint8) 0xff);
}

void MidiMessage::getTimeSignatureInfo (int& numerator, int& denominator) const noexcept
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

MidiMessage MidiMessage::timeSignatureMetaEvent (const int numerator, const int denominator)
{
    int n = 1;
    int powerOfTwo = 0;

    while (n < denominator)
    {
        n <<= 1;
        ++powerOfTwo;
    }

    const uint8 d[] = { 0xff, 0x58, 0x04, (uint8) numerator, (uint8) powerOfTwo, 1, 96 };
    return MidiMessage (d, 7, 0.0);
}

MidiMessage MidiMessage::midiChannelMetaEvent (const int channel) noexcept
{
    const uint8 d[] = { 0xff, 0x20, 0x01, (uint8) jlimit (0, 0xff, channel - 1) };
    return MidiMessage (d, 4, 0.0);
}

bool MidiMessage::isKeySignatureMetaEvent() const noexcept
{
    return getMetaEventType() == 0x59;
}

int MidiMessage::getKeySignatureNumberOfSharpsOrFlats() const noexcept
{
    return (int) getMetaEventData()[0];
}

bool MidiMessage::isKeySignatureMajorKey() const noexcept
{
    return getMetaEventData()[1] == 0;
}

MidiMessage MidiMessage::keySignatureMetaEvent (int numberOfSharpsOrFlats, bool isMinorKey)
{
    jassert (numberOfSharpsOrFlats >= -7 && numberOfSharpsOrFlats <= 7);

    const uint8 d[] = { 0xff, 0x59, 0x02, (uint8) numberOfSharpsOrFlats, isMinorKey ? (uint8) 1 : (uint8) 0 };
    return MidiMessage (d, 5, 0.0);
}

MidiMessage MidiMessage::endOfTrack() noexcept
{
    return MidiMessage (0xff, 0x2f, 0, 0.0);
}

//==============================================================================
bool MidiMessage::isSongPositionPointer() const noexcept         { return *getRawData() == 0xf2; }
int MidiMessage::getSongPositionPointerMidiBeat() const noexcept { const uint8* data = getRawData(); return data[1] | (data[2] << 7); }

MidiMessage MidiMessage::songPositionPointer (const int positionInMidiBeats) noexcept
{
    return MidiMessage (0xf2,
                        positionInMidiBeats & 127,
                        (positionInMidiBeats >> 7) & 127);
}

bool MidiMessage::isMidiStart() const noexcept            { return *getRawData() == 0xfa; }
MidiMessage MidiMessage::midiStart() noexcept             { return MidiMessage (0xfa); }

bool MidiMessage::isMidiContinue() const noexcept         { return *getRawData() == 0xfb; }
MidiMessage MidiMessage::midiContinue() noexcept          { return MidiMessage (0xfb); }

bool MidiMessage::isMidiStop() const noexcept             { return *getRawData() == 0xfc; }
MidiMessage MidiMessage::midiStop() noexcept              { return MidiMessage (0xfc); }

bool MidiMessage::isMidiClock() const noexcept            { return *getRawData() == 0xf8; }
MidiMessage MidiMessage::midiClock() noexcept             { return MidiMessage (0xf8); }

bool MidiMessage::isQuarterFrame() const noexcept               { return *getRawData() == 0xf1; }
int MidiMessage::getQuarterFrameSequenceNumber() const noexcept { return ((int) getRawData()[1]) >> 4; }
int MidiMessage::getQuarterFrameValue() const noexcept          { return ((int) getRawData()[1]) & 0x0f; }

MidiMessage MidiMessage::quarterFrame (const int sequenceNumber, const int value) noexcept
{
    return MidiMessage (0xf1, (sequenceNumber << 4) | value);
}

bool MidiMessage::isFullFrame() const noexcept
{
    const uint8* const data = getRawData();

    return data[0] == 0xf0
            && data[1] == 0x7f
            && size >= 10
            && data[3] == 0x01
            && data[4] == 0x01;
}

void MidiMessage::getFullFrameParameters (int& hours, int& minutes, int& seconds, int& frames,
                                          MidiMessage::SmpteTimecodeType& timecodeType) const noexcept
{
    jassert (isFullFrame());

    const uint8* const data = getRawData();
    timecodeType = (SmpteTimecodeType) (data[5] >> 5);
    hours   = data[5] & 0x1f;
    minutes = data[6];
    seconds = data[7];
    frames  = data[8];
}

MidiMessage MidiMessage::fullFrame (const int hours, const int minutes,
                                    const int seconds, const int frames,
                                    MidiMessage::SmpteTimecodeType timecodeType)
{
    const uint8 d[] = { 0xf0, 0x7f, 0x7f, 0x01, 0x01,
                        (uint8) ((hours & 0x01f) | (timecodeType << 5)),
                        (uint8) minutes,
                        (uint8) seconds,
                        (uint8) frames,
                        0xf7 };

    return MidiMessage (d, 10, 0.0);
}

bool MidiMessage::isMidiMachineControlMessage() const noexcept
{
    const uint8* const data = getRawData();
    return data[0] == 0xf0
        && data[1] == 0x7f
        && data[3] == 0x06
        && size > 5;
}

MidiMessage::MidiMachineControlCommand MidiMessage::getMidiMachineControlCommand() const noexcept
{
    jassert (isMidiMachineControlMessage());

    return (MidiMachineControlCommand) getRawData()[4];
}

MidiMessage MidiMessage::midiMachineControlCommand (MidiMessage::MidiMachineControlCommand command)
{
    const uint8 d[] = { 0xf0, 0x7f, 0, 6, (uint8) command, 0xf7 };

    return MidiMessage (d, 6, 0.0);
}

//==============================================================================
bool MidiMessage::isMidiMachineControlGoto (int& hours, int& minutes, int& seconds, int& frames) const noexcept
{
    const uint8* const data = getRawData();
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

MidiMessage MidiMessage::midiMachineControlGoto (int hours, int minutes, int seconds, int frames)
{
    const uint8 d[] = { 0xf0, 0x7f, 0, 6, 0x44, 6, 1,
                        (uint8) hours,
                        (uint8) minutes,
                        (uint8) seconds,
                        (uint8) frames,
                        0xf7 };

    return MidiMessage (d, 12, 0.0);
}

//==============================================================================
String MidiMessage::getMidiNoteName (int note, bool useSharps, bool includeOctaveNumber, int octaveNumForMiddleC)
{
    static const char* const sharpNoteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static const char* const flatNoteNames[]  = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    if (isPositiveAndBelow (note, (int) 128))
    {
        String s (useSharps ? sharpNoteNames [note % 12]
                            : flatNoteNames  [note % 12]);

        if (includeOctaveNumber)
            s << (note / 12 + (octaveNumForMiddleC - 5));

        return s;
    }

    return String::empty;
}

double MidiMessage::getMidiNoteInHertz (int noteNumber, const double frequencyOfA) noexcept
{
    return frequencyOfA * pow (2.0, (noteNumber - 69) / 12.0);
}

const char* MidiMessage::getGMInstrumentName (const int n)
{
    static const char* names[] =
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

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}

const char* MidiMessage::getGMInstrumentBankName (const int n)
{
    static const char* names[] =
    {
        "Piano", "Chromatic Percussion", "Organ", "Guitar",
        "Bass", "Strings", "Ensemble", "Brass",
        "Reed", "Pipe", "Synth Lead", "Synth Pad",
        "Synth Effects", "Ethnic", "Percussive", "Sound Effects"
    };

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}

const char* MidiMessage::getRhythmInstrumentName (const int n)
{
    static const char* names[] =
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

    return (n >= 35 && n <= 81) ? names [n - 35] : nullptr;
}

const char* MidiMessage::getControllerName (const int n)
{
    static const char* names[] =
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

    return isPositiveAndBelow (n, numElementsInArray (names)) ? names[n] : nullptr;
}
