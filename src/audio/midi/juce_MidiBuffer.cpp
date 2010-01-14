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

#include "juce_MidiBuffer.h"


//==============================================================================
MidiBuffer::MidiBuffer() throw()
    : bytesUsed (0)
{
}

MidiBuffer::MidiBuffer (const MidiMessage& message) throw()
    : bytesUsed (0)
{
    addEvent (message, 0);
}

MidiBuffer::MidiBuffer (const MidiBuffer& other) throw()
    : data (other.data),
      bytesUsed (other.bytesUsed)
{
}

const MidiBuffer& MidiBuffer::operator= (const MidiBuffer& other) throw()
{
    if (this != &other)
    {
        bytesUsed = other.bytesUsed;
        data = other.data;
    }

    return *this;
}

void MidiBuffer::swap (MidiBuffer& other)
{
    data.swapWith (other.data);
    swapVariables <int> (bytesUsed, other.bytesUsed);
}

MidiBuffer::~MidiBuffer() throw()
{
}

void MidiBuffer::clear() throw()
{
    bytesUsed = 0;
}

void MidiBuffer::clear (const int startSample,
                        const int numSamples) throw()
{
    uint8* const start = findEventAfter (data, startSample - 1);
    uint8* const end   = findEventAfter (start, startSample + numSamples - 1);

    if (end > start)
    {
        const size_t bytesToMove = (size_t) (bytesUsed - (end - (uint8*) data.getData()));

        if (bytesToMove > 0)
            memmove (start, end, bytesToMove);

        bytesUsed -= (int) (end - start);
    }
}

void MidiBuffer::addEvent (const MidiMessage& m,
                           const int sampleNumber) throw()
{
    addEvent (m.getRawData(), m.getRawDataSize(), sampleNumber);
}

static int findActualEventLength (const uint8* const data,
                                  const int maxBytes) throw()
{
    unsigned int byte = (unsigned int) *data;

    int size = 0;

    if (byte == 0xf0 || byte == 0xf7)
    {
        const uint8* d = data + 1;

        while (d < data + maxBytes)
            if (*d++ == 0xf7)
                break;

        size = (int) (d - data);
    }
    else if (byte == 0xff)
    {
        int n;
        const int bytesLeft = MidiMessage::readVariableLengthVal (data + 1, n);
        size = jmin (maxBytes, n + 2 + bytesLeft);
    }
    else if (byte >= 0x80)
    {
        size = jmin (maxBytes, MidiMessage::getMessageLengthFromFirstByte ((uint8) byte));
    }

    return size;
}

void MidiBuffer::addEvent (const uint8* const newData,
                           const int maxBytes,
                           const int sampleNumber) throw()
{
    const int numBytes = findActualEventLength (newData, maxBytes);

    if (numBytes > 0)
    {
        int spaceNeeded = bytesUsed + numBytes + 6;
        data.ensureSize ((spaceNeeded + spaceNeeded / 2 + 8) & ~7);

        uint8* d = findEventAfter ((uint8*) data.getData(), sampleNumber);
        const size_t bytesToMove = (size_t) (bytesUsed - (d - (uint8*) data.getData()));

        if (bytesToMove > 0)
            memmove (d + numBytes + 6,
                     d,
                     bytesToMove);

        *(int*) d = sampleNumber;
        d += 4;
        *(uint16*) d = (uint16) numBytes;
        d += 2;

        memcpy (d, newData, numBytes);

        bytesUsed += numBytes + 6;
    }
}

void MidiBuffer::addEvents (const MidiBuffer& otherBuffer,
                            const int startSample,
                            const int numSamples,
                            const int sampleDeltaToAdd) throw()
{
    Iterator i (otherBuffer);
    i.setNextSamplePosition (startSample);

    const uint8* eventData;
    int eventSize, position;

    while (i.getNextEvent (eventData, eventSize, position)
            && (position < startSample + numSamples || numSamples < 0))
    {
        addEvent (eventData, eventSize, position + sampleDeltaToAdd);
    }
}

bool MidiBuffer::isEmpty() const throw()
{
    return bytesUsed == 0;
}

int MidiBuffer::getNumEvents() const throw()
{
    int n = 0;
    const uint8* d = (uint8*) data.getData();
    const uint8* const end = d + bytesUsed;

    while (d < end)
    {
        d += 4;
        d += 2 + *(const uint16*) d;
        ++n;
    }

    return n;
}

int MidiBuffer::getFirstEventTime() const throw()
{
    return (bytesUsed > 0) ? *(const int*) data.getData() : 0;
}

int MidiBuffer::getLastEventTime() const throw()
{
    if (bytesUsed == 0)
        return 0;

    const uint8* d = (uint8*) data.getData();
    const uint8* const endData = d + bytesUsed;

    for (;;)
    {
        const uint8* nextOne = d + 6 + * (const uint16*) (d + 4);

        if (nextOne >= endData)
            return *(const int*) d;

        d = nextOne;
    }
}

uint8* MidiBuffer::findEventAfter (uint8* d, const int samplePosition) const throw()
{
    const uint8* const endData = ((uint8*) data.getData()) + bytesUsed;

    while (d < endData && *(int*) d <= samplePosition)
    {
        d += 4;
        d += 2 + *(uint16*) d;
    }

    return d;
}

//==============================================================================
MidiBuffer::Iterator::Iterator (const MidiBuffer& buffer_) throw()
    : buffer (buffer_),
      data ((uint8*) buffer_.data.getData())
{
}

MidiBuffer::Iterator::~Iterator() throw()
{
}

//==============================================================================
void MidiBuffer::Iterator::setNextSamplePosition (const int samplePosition) throw()
{
    data = buffer.data;
    const uint8* dataEnd = ((uint8*) buffer.data.getData()) + buffer.bytesUsed;

    while (data < dataEnd && *(int*) data < samplePosition)
    {
        data += 4;
        data += 2 + *(uint16*) data;
    }
}

bool MidiBuffer::Iterator::getNextEvent (const uint8* &midiData,
                                         int& numBytes,
                                         int& samplePosition) throw()
{
    if (data >= ((uint8*) buffer.data.getData()) + buffer.bytesUsed)
        return false;

    samplePosition = *(int*) data;
    data += 4;
    numBytes = *(uint16*) data;
    data += 2;
    midiData = data;
    data += numBytes;

    return true;
}

bool MidiBuffer::Iterator::getNextEvent (MidiMessage& result,
                                         int& samplePosition) throw()
{
    if (data >= ((uint8*) buffer.data.getData()) + buffer.bytesUsed)
        return false;

    samplePosition = *(int*) data;
    data += 4;
    const int numBytes = *(uint16*) data;
    data += 2;
    result = MidiMessage (data, numBytes, samplePosition);
    data += numBytes;

    return true;
}


END_JUCE_NAMESPACE
