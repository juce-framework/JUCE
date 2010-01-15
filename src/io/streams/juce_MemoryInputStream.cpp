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


#include "juce_MemoryInputStream.h"


//==============================================================================
MemoryInputStream::MemoryInputStream (const void* const sourceData,
                                      const size_t sourceDataSize,
                                      const bool keepInternalCopy)
    : data ((const char*) sourceData),
      dataSize (sourceDataSize),
      position (0)
{
    if (keepInternalCopy)
    {
        internalCopy.append (data, sourceDataSize);
        data = (const char*) internalCopy.getData();
    }
}

MemoryInputStream::~MemoryInputStream()
{
}

int64 MemoryInputStream::getTotalLength()
{
    return dataSize;
}

int MemoryInputStream::read (void* buffer, int howMany)
{
    jassert (howMany >= 0);
    const int num = jmin (howMany, (int) (dataSize - position));
    memcpy (buffer, data + position, num);
    position += num;
    return (int) num;
}

bool MemoryInputStream::isExhausted()
{
    return (position >= dataSize);
}

bool MemoryInputStream::setPosition (int64 pos)
{
    position = (int) jlimit ((int64) 0, (int64) dataSize, pos);

    return true;
}

int64 MemoryInputStream::getPosition()
{
    return position;
}

END_JUCE_NAMESPACE
