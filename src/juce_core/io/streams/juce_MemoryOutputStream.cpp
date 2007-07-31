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

#include "../../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_MemoryOutputStream.h"


//==============================================================================
MemoryOutputStream::MemoryOutputStream (const int initialSize,
                                        const int blockSizeToIncreaseBy,
                                        MemoryBlock* const memoryBlockToWriteTo) throw()
  : data (memoryBlockToWriteTo),
    position (0),
    size (0),
    blockSize (jmax (16, blockSizeToIncreaseBy)),
    ownsMemoryBlock (memoryBlockToWriteTo == 0)
{
    if (memoryBlockToWriteTo == 0)
        data = new MemoryBlock (initialSize);
    else
        memoryBlockToWriteTo->setSize (initialSize, false);
}

MemoryOutputStream::~MemoryOutputStream() throw()
{
    if (ownsMemoryBlock)
        delete data;
    else
        flush();
}

void MemoryOutputStream::flush()
{
    if (! ownsMemoryBlock)
        data->setSize (size, false);
}

void MemoryOutputStream::reset() throw()
{
    position = 0;
    size = 0;
}

bool MemoryOutputStream::write (const void* buffer, int howMany)
{
    int storageNeeded = position + howMany + 1;
    storageNeeded = storageNeeded - (storageNeeded % blockSize) + blockSize;

    data->ensureSize (storageNeeded);
    data->copyFrom (buffer, position, howMany);
    position += howMany;
    size = jmax (size, position);

    return true;
}

const char* MemoryOutputStream::getData() throw()
{
    if (data->getSize() > size)
        ((char*) data->getData()) [size] = 0;

    return (const char*) data->getData();
}

int MemoryOutputStream::getDataSize() const throw()
{
    return size;
}

int64 MemoryOutputStream::getPosition()
{
    return position;
}

bool MemoryOutputStream::setPosition (int64 newPosition)
{
    if (newPosition <= size)
    {
        // ok to seek backwards
        position = jlimit (0, size, (int) newPosition);
        return true;
    }
    else
    {
        // trying to make it bigger isn't a good thing to do..
        return false;
    }
}

END_JUCE_NAMESPACE
