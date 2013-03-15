/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

MemoryOutputStream::MemoryOutputStream (const size_t initialSize)
  : data (internalBlock),
    position (0),
    size (0)
{
    internalBlock.setSize (initialSize, false);
}

MemoryOutputStream::MemoryOutputStream (MemoryBlock& memoryBlockToWriteTo,
                                        const bool appendToExistingBlockContent)
  : data (memoryBlockToWriteTo),
    position (0),
    size (0)
{
    if (appendToExistingBlockContent)
        position = size = memoryBlockToWriteTo.getSize();
}

MemoryOutputStream::~MemoryOutputStream()
{
    trimExternalBlockSize();
}

void MemoryOutputStream::flush()
{
    trimExternalBlockSize();
}

void MemoryOutputStream::trimExternalBlockSize()
{
    if (&data != &internalBlock)
        data.setSize (size, false);
}

void MemoryOutputStream::preallocate (const size_t bytesToPreallocate)
{
    data.ensureSize (bytesToPreallocate + 1);
}

void MemoryOutputStream::reset() noexcept
{
    position = 0;
    size = 0;
}

void MemoryOutputStream::prepareToWrite (size_t numBytes)
{
    jassert ((ssize_t) numBytes >= 0);
    size_t storageNeeded = position + numBytes;

    if (storageNeeded >= data.getSize())
        data.ensureSize ((storageNeeded + jmin (storageNeeded / 2, (size_t) (1024 * 1024)) + 32) & ~31u);
}

bool MemoryOutputStream::write (const void* const buffer, size_t howMany)
{
    jassert (buffer != nullptr && ((ssize_t) howMany) >= 0);

    if (howMany > 0)
    {
        prepareToWrite (howMany);
        memcpy (static_cast<char*> (data.getData()) + position, buffer, howMany);
        position += howMany;
        size = jmax (size, position);
    }

    return true;
}

void MemoryOutputStream::writeRepeatedByte (uint8 byte, size_t howMany)
{
    if (howMany > 0)
    {
        prepareToWrite (howMany);
        memset (static_cast<char*> (data.getData()) + position, byte, howMany);
        position += howMany;
        size = jmax (size, position);
    }
}

MemoryBlock MemoryOutputStream::getMemoryBlock() const
{
    return MemoryBlock (getData(), getDataSize());
}

const void* MemoryOutputStream::getData() const noexcept
{
    if (data.getSize() > size)
        static_cast <char*> (data.getData()) [size] = 0;

    return data.getData();
}

bool MemoryOutputStream::setPosition (int64 newPosition)
{
    if (newPosition <= (int64) size)
    {
        // ok to seek backwards
        position = jlimit ((size_t) 0, size, (size_t) newPosition);
        return true;
    }

    // can't move beyond the end of the stream..
    return false;
}

int MemoryOutputStream::writeFromInputStream (InputStream& source, int64 maxNumBytesToWrite)
{
    // before writing from an input, see if we can preallocate to make it more efficient..
    int64 availableData = source.getTotalLength() - source.getPosition();

    if (availableData > 0)
    {
        if (maxNumBytesToWrite > availableData)
            maxNumBytesToWrite = availableData;

        preallocate (data.getSize() + (size_t) maxNumBytesToWrite);
    }

    return OutputStream::writeFromInputStream (source, maxNumBytesToWrite);
}

String MemoryOutputStream::toUTF8() const
{
    const char* const d = static_cast <const char*> (getData());
    return String (CharPointer_UTF8 (d), CharPointer_UTF8 (d + getDataSize()));
}

String MemoryOutputStream::toString() const
{
    return String::createStringFromData (getData(), (int) getDataSize());
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const MemoryOutputStream& streamToRead)
{
    const size_t dataSize = streamToRead.getDataSize();

    if (dataSize > 0)
        stream.write (streamToRead.getData(), dataSize);

    return stream;
}
