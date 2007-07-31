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


#include "juce_MemoryInputStream.h"


//==============================================================================
MemoryInputStream::MemoryInputStream (const void* const sourceData,
                                      const int sourceDataSize,
                                      const bool keepInternalCopy) throw()
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

MemoryInputStream::~MemoryInputStream() throw()
{
}

int64 MemoryInputStream::getTotalLength()
{
    return dataSize;
}

int MemoryInputStream::read (void* buffer, int howMany)
{
    const int num = jmin (howMany, dataSize - position);
    memcpy (buffer, data + position, num);
    position += num;
    return num;
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
