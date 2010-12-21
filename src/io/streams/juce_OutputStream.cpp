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

#include "juce_OutputStream.h"
#include "../../threads/juce_ScopedLock.h"
#include "../../containers/juce_Array.h"
#include "../../memory/juce_ScopedPointer.h"
#include "../files/juce_FileInputStream.h"


//==============================================================================
#if JUCE_DEBUG
static Array<void*, CriticalSection> activeStreams;

void juce_CheckForDanglingStreams()
{
    /*
        It's always a bad idea to leak any object, but if you're leaking output
        streams, then there's a good chance that you're failing to flush a file
        to disk properly, which could result in corrupted data and other similar
        nastiness..
    */
    jassert (activeStreams.size() == 0);
};
#endif

//==============================================================================
OutputStream::OutputStream()
    : newLineString (NewLine::getDefault())
{
  #if JUCE_DEBUG
    activeStreams.add (this);
  #endif
}

OutputStream::~OutputStream()
{
  #if JUCE_DEBUG
    activeStreams.removeValue (this);
  #endif
}

//==============================================================================
void OutputStream::writeBool (const bool b)
{
    writeByte (b ? (char) 1
                 : (char) 0);
}

void OutputStream::writeByte (char byte)
{
    write (&byte, 1);
}

void OutputStream::writeShort (short value)
{
    const unsigned short v = ByteOrder::swapIfBigEndian ((unsigned short) value);
    write (&v, 2);
}

void OutputStream::writeShortBigEndian (short value)
{
    const unsigned short v = ByteOrder::swapIfLittleEndian ((unsigned short) value);
    write (&v, 2);
}

void OutputStream::writeInt (int value)
{
    const unsigned int v = ByteOrder::swapIfBigEndian ((unsigned int) value);
    write (&v, 4);
}

void OutputStream::writeIntBigEndian (int value)
{
    const unsigned int v = ByteOrder::swapIfLittleEndian ((unsigned int) value);
    write (&v, 4);
}

void OutputStream::writeCompressedInt (int value)
{
    unsigned int un = (value < 0) ? (unsigned int) -value
                                  : (unsigned int) value;

    uint8 data[5];
    int num = 0;

    while (un > 0)
    {
        data[++num] = (uint8) un;
        un >>= 8;
    }

    data[0] = (uint8) num;

    if (value < 0)
        data[0] |= 0x80;

    write (data, num + 1);
}

void OutputStream::writeInt64 (int64 value)
{
    const uint64 v = ByteOrder::swapIfBigEndian ((uint64) value);
    write (&v, 8);
}

void OutputStream::writeInt64BigEndian (int64 value)
{
    const uint64 v = ByteOrder::swapIfLittleEndian ((uint64) value);
    write (&v, 8);
}

void OutputStream::writeFloat (float value)
{
    union { int asInt; float asFloat; } n;
    n.asFloat = value;
    writeInt (n.asInt);
}

void OutputStream::writeFloatBigEndian (float value)
{
    union { int asInt; float asFloat; } n;
    n.asFloat = value;
    writeIntBigEndian (n.asInt);
}

void OutputStream::writeDouble (double value)
{
    union { int64 asInt; double asDouble; } n;
    n.asDouble = value;
    writeInt64 (n.asInt);
}

void OutputStream::writeDoubleBigEndian (double value)
{
    union { int64 asInt; double asDouble; } n;
    n.asDouble = value;
    writeInt64BigEndian (n.asInt);
}

void OutputStream::writeString (const String& text)
{
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    const int numBytes = text.getNumBytesAsUTF8() + 1;
    HeapBlock<char> temp (numBytes);
    text.copyToUTF8 (temp, numBytes);
    write (temp, numBytes);
}

void OutputStream::writeText (const String& text, const bool asUnicode,
                              const bool writeUnicodeHeaderBytes)
{
    if (asUnicode)
    {
        if (writeUnicodeHeaderBytes)
            write ("\x0ff\x0fe", 2);

        const juce_wchar* src = text;
        bool lastCharWasReturn = false;

        while (*src != 0)
        {
            if (*src == L'\n' && ! lastCharWasReturn)
                writeShort ((short) L'\r');

            lastCharWasReturn = (*src == L'\r');
            writeShort ((short) *src++);
        }
    }
    else
    {
        const char* src = text.toUTF8();
        const char* t = src;

        for (;;)
        {
            if (*t == '\n')
            {
                if (t > src)
                    write (src, (int) (t - src));

                write ("\r\n", 2);
                src = t + 1;
            }
            else if (*t == '\r')
            {
                if (t[1] == '\n')
                    ++t;
            }
            else if (*t == 0)
            {
                if (t > src)
                    write (src, (int) (t - src));

                break;
            }

            ++t;
        }
    }
}

int OutputStream::writeFromInputStream (InputStream& source, int64 numBytesToWrite)
{
    if (numBytesToWrite < 0)
        numBytesToWrite = std::numeric_limits<int64>::max();

    int numWritten = 0;

    while (numBytesToWrite > 0 && ! source.isExhausted())
    {
        char buffer [8192];
        const int num = source.read (buffer, (int) jmin (numBytesToWrite, (int64) sizeof (buffer)));

        if (num <= 0)
            break;

        write (buffer, num);

        numBytesToWrite -= num;
        numWritten += num;
    }

    return numWritten;
}

//==============================================================================
void OutputStream::setNewLineString (const String& newLineString_)
{
    newLineString = newLineString_;
}

//==============================================================================
OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const int number)
{
    return stream << String (number);
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const double number)
{
    return stream << String (number);
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const char character)
{
    stream.writeByte (character);
    return stream;
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const char* const text)
{
    stream.write (text, (int) strlen (text));
    return stream;
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const MemoryBlock& data)
{
    stream.write (data.getData(), (int) data.getSize());
    return stream;
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const File& fileToRead)
{
    const ScopedPointer<FileInputStream> in (fileToRead.createInputStream());

    if (in != 0)
        stream.writeFromInputStream (*in, -1);

    return stream;
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const NewLine&)
{
    return stream << stream.getNewLineString();
}


END_JUCE_NAMESPACE
