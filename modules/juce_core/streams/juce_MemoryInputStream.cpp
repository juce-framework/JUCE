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

MemoryInputStream::MemoryInputStream (const void* const sourceData,
                                      const size_t sourceDataSize,
                                      const bool keepInternalCopy)
    : data (sourceData),
      dataSize (sourceDataSize),
      position (0)
{
    if (keepInternalCopy)
        createInternalCopy();
}

MemoryInputStream::MemoryInputStream (const MemoryBlock& sourceData,
                                      const bool keepInternalCopy)
    : data (sourceData.getData()),
      dataSize (sourceData.getSize()),
      position (0)
{
    if (keepInternalCopy)
        createInternalCopy();
}

void MemoryInputStream::createInternalCopy()
{
    internalCopy.malloc (dataSize);
    memcpy (internalCopy, data, dataSize);
    data = internalCopy;
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
    jassert (buffer != nullptr && howMany >= 0);

    const int num = jmin (howMany, (int) (dataSize - position));
    if (num <= 0)
        return 0;

    memcpy (buffer, addBytesToPointer (data, position), (size_t) num);
    position += (unsigned int) num;
    return num;
}

bool MemoryInputStream::isExhausted()
{
    return position >= dataSize;
}

bool MemoryInputStream::setPosition (const int64 pos)
{
    position = (size_t) jlimit ((int64) 0, (int64) dataSize, pos);
    return true;
}

int64 MemoryInputStream::getPosition()
{
    return position;
}


//==============================================================================
#if JUCE_UNIT_TESTS

class MemoryStreamTests  : public UnitTest
{
public:
    MemoryStreamTests() : UnitTest ("MemoryInputStream & MemoryOutputStream") {}

    void runTest()
    {
        beginTest ("Basics");
        Random r;

        int randomInt = r.nextInt();
        int64 randomInt64 = r.nextInt64();
        double randomDouble = r.nextDouble();
        String randomString (createRandomWideCharString());

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
        expectEquals (mi.readString(), randomString);
        expect (mi.readInt64() == randomInt64);
        expect (mi.readInt64BigEndian() == randomInt64);
        expect (mi.readDouble() == randomDouble);
        expect (mi.readDoubleBigEndian() == randomDouble);
    }

    static String createRandomWideCharString()
    {
        juce_wchar buffer [50] = { 0 };
        Random r;

        for (int i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            if (r.nextBool())
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (juce_wchar) (1 + r.nextInt (0xff));
        }

        return CharPointer_UTF32 (buffer);
    }
};

static MemoryStreamTests memoryInputStreamUnitTests;

#endif
