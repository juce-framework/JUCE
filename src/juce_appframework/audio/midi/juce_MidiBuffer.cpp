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

#include "juce_MidiBuffer.h"


//==============================================================================
MidiBuffer::MidiBuffer() throw()
    : ArrayAllocationBase <uint8> (32),
      bytesUsed (0)
{
}

MidiBuffer::MidiBuffer (const MidiBuffer& other) throw()
    : ArrayAllocationBase <uint8> (32),
      bytesUsed (other.bytesUsed)
{
    ensureAllocatedSize (bytesUsed);
    memcpy (elements, other.elements, bytesUsed);
}

const MidiBuffer& MidiBuffer::operator= (const MidiBuffer& other) throw()
{
    bytesUsed = other.bytesUsed;
    ensureAllocatedSize (bytesUsed);
    memcpy (elements, other.elements, bytesUsed);

    return *this;
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
    uint8* const start = findEventAfter (elements, startSample - 1);
    uint8* const end   = findEventAfter (start, startSample + numSamples - 1);

    if (end > start)
    {
        const size_t bytesToMove = (size_t) (bytesUsed - (end - elements));

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
        ensureAllocatedSize (bytesUsed + numBytes + 6);

        uint8* d = findEventAfter (elements, sampleNumber);
        const size_t bytesToMove = (size_t) (bytesUsed - (d - elements));

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

    const uint8* data;
    int size, position;

    while (i.getNextEvent (data, size, position)
            && (position < startSample + numSamples || numSamples < 0))
    {
        addEvent (data, size, position + sampleDeltaToAdd);
    }
}

bool MidiBuffer::isEmpty() const throw()
{
    return bytesUsed == 0;
}

int MidiBuffer::getNumEvents() const throw()
{
    int n = 0;
    const uint8* d = elements;
    const uint8* const end = elements + bytesUsed;

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
    return (bytesUsed > 0) ? *(const int*) elements : 0;
}

int MidiBuffer::getLastEventTime() const throw()
{
    if (bytesUsed == 0)
        return 0;

    const uint8* d = elements;
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
    const uint8* const endData = elements + bytesUsed;

    while (d < endData && *(int*) d <= samplePosition)
    {
        d += 4;
        d += 2 + *(uint16*) d;
    }

    return d;
}

//==============================================================================
MidiBuffer::Iterator::Iterator (const MidiBuffer& buffer) throw()
    : buffer (buffer),
      data (buffer.elements)
{
}

MidiBuffer::Iterator::~Iterator() throw()
{
}

//==============================================================================
void MidiBuffer::Iterator::setNextSamplePosition (const int samplePosition) throw()
{
    data = buffer.elements;
    const uint8* dataEnd = buffer.elements + buffer.bytesUsed;

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
    if (data >= buffer.elements + buffer.bytesUsed)
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
    if (data >= buffer.elements + buffer.bytesUsed)
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
