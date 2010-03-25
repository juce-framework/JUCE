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


#include "juce_MemoryOutputStream.h"


//==============================================================================
MemoryOutputStream::MemoryOutputStream (const size_t initialSize,
                                        const size_t blockSizeToIncreaseBy,
                                        MemoryBlock* const memoryBlockToWriteTo)
  : data (memoryBlockToWriteTo),
    position (0),
    size (0),
    blockSize (jmax ((size_t) 16, blockSizeToIncreaseBy))
{
    if (data == 0)
        dataToDelete = data = new MemoryBlock (initialSize);
    else
        data->setSize (initialSize, false);
}

MemoryOutputStream::~MemoryOutputStream()
{
    flush();
}

void MemoryOutputStream::flush()
{
    if (dataToDelete == 0)
        data->setSize (size, false);
}

void MemoryOutputStream::reset() throw()
{
    position = 0;
    size = 0;
}

bool MemoryOutputStream::write (const void* const buffer, int howMany)
{
    if (howMany > 0)
    {
        size_t storageNeeded = position + howMany;

        if (storageNeeded >= data->getSize())
        {
            // if we need more space, increase the block by at least 10%..
            storageNeeded += jmax (blockSize, storageNeeded / 10);
            storageNeeded = storageNeeded - (storageNeeded % blockSize) + blockSize;

            data->ensureSize (storageNeeded);
        }

        data->copyFrom (buffer, (int) position, howMany);
        position += howMany;
        size = jmax (size, position);
    }

    return true;
}

const char* MemoryOutputStream::getData() const throw()
{
    char* const d = static_cast <char*> (data->getData());

    if (data->getSize() > size)
        d [size] = 0;

    return d;
}

bool MemoryOutputStream::setPosition (int64 newPosition)
{
    if (newPosition <= (int64) size)
    {
        // ok to seek backwards
        position = jlimit ((size_t) 0, size, (size_t) newPosition);
        return true;
    }
    else
    {
        // trying to make it bigger isn't a good thing to do..
        return false;
    }
}

const String MemoryOutputStream::toUTF8() const
{
    return String (getData(), getDataSize());
}

END_JUCE_NAMESPACE
