/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
    : data (static_cast <const char*> (sourceData)),
      dataSize (sourceDataSize),
      position (0)
{
    if (keepInternalCopy)
    {
        internalCopy.append (data, sourceDataSize);
        data = static_cast <const char*> (internalCopy.getData());
    }
}

MemoryInputStream::MemoryInputStream (const MemoryBlock& sourceData,
                                      const bool keepInternalCopy)
    : data (static_cast <const char*> (sourceData.getData())),
      dataSize (sourceData.getSize()),
      position (0)
{
    if (keepInternalCopy)
    {
        internalCopy = sourceData;
        data = static_cast <const char*> (internalCopy.getData());
    }
}

MemoryInputStream::~MemoryInputStream()
{
}

int64 MemoryInputStream::getTotalLength()
{
    return dataSize;
}

int MemoryInputStream::read (void* const buffer, const int howMany)
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

bool MemoryInputStream::setPosition (const int64 pos)
{
    position = (int) jlimit ((int64) 0, (int64) dataSize, pos);
    return true;
}

int64 MemoryInputStream::getPosition()
{
    return position;
}


#if JUCE_UNIT_TESTS

#include "../../utilities/juce_UnitTest.h"
#include "../../maths/juce_Random.h"
#include "juce_MemoryOutputStream.h"

class MemoryStreamTests  : public UnitTest
{
public:
    MemoryStreamTests() : UnitTest ("MemoryInputStream & MemoryOutputStream") {}

    void runTest()
    {
        beginTest ("Basics");

        int randomInt = Random::getSystemRandom().nextInt();
        int64 randomInt64 = Random::getSystemRandom().nextInt64();
        double randomDouble = Random::getSystemRandom().nextDouble();
        String randomString;
        for (int i = 50; --i >= 0;)
            randomString << (juce_wchar) (Random::getSystemRandom().nextInt() & 0xffff);

        MemoryOutputStream mo;
        mo.writeInt (randomInt);
        mo.writeIntBigEndian (randomInt);
        mo.writeCompressedInt (randomInt);
        mo.writeString (randomString);
        mo.writeInt64 (randomInt64);
        mo.writeInt64BigEndian (randomInt64);
        mo.writeDouble (randomDouble);
        mo.writeDoubleBigEndian (randomDouble);

        MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
        expect (mi.readInt() == randomInt);
        expect (mi.readIntBigEndian() == randomInt);
        expect (mi.readCompressedInt() == randomInt);
        expect (mi.readString() == randomString);
        expect (mi.readInt64() == randomInt64);
        expect (mi.readInt64BigEndian() == randomInt64);
        expect (mi.readDouble() == randomDouble);
        expect (mi.readDoubleBigEndian() == randomDouble);
    }
};

static MemoryStreamTests memoryInputStreamUnitTests;

#endif

END_JUCE_NAMESPACE
