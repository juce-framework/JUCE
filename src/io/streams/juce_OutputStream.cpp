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


#include "juce_OutputStream.h"
#include "../../threads/juce_CriticalSection.h"
#include "../../containers/juce_VoidArray.h"


//==============================================================================
#if JUCE_DEBUG
static CriticalSection activeStreamLock;
static VoidArray activeStreams;

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
OutputStream::OutputStream() throw()
{
#if JUCE_DEBUG
    activeStreamLock.enter();
    activeStreams.add (this);
    activeStreamLock.exit();
#endif
}

OutputStream::~OutputStream()
{
#if JUCE_DEBUG
    activeStreamLock.enter();
    activeStreams.removeValue (this);
    activeStreamLock.exit();
#endif
}

//==============================================================================
void OutputStream::writeBool (bool b)
{
    writeByte ((b) ? (char) 1
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
    const int numBytes = text.copyToUTF8 (0);
    HeapBlock <uint8> temp (numBytes);

    text.copyToUTF8 (temp);
    write (temp, numBytes); // (numBytes includes the terminating null).
}

void OutputStream::printf (const char* pf, ...)
{
    unsigned int bufSize = 256;
    HeapBlock <char> buf (bufSize);

    for (;;)
    {
        va_list list;
        va_start (list, pf);

        const int num = CharacterFunctions::vprintf (buf, bufSize, pf, list);

        va_end (list);

        if (num > 0)
        {
            write (buf, num);
            break;
        }
        else if (num == 0)
        {
            break;
        }

        bufSize += 256;
        buf.malloc (bufSize);
    }
}

OutputStream& OutputStream::operator<< (const int number)
{
    const String s (number);
    write ((const char*) s, s.length());
    return *this;
}

OutputStream& OutputStream::operator<< (const double number)
{
    const String s (number);
    write ((const char*) s, s.length());
    return *this;
}

OutputStream& OutputStream::operator<< (const char character)
{
    writeByte (character);
    return *this;
}

OutputStream& OutputStream::operator<< (const char* const text)
{
    write (text, (int) strlen (text));
    return *this;
}

OutputStream& OutputStream::operator<< (const juce_wchar* const text)
{
    const String s (text);
    write ((const char*) s, s.length());
    return *this;
}

OutputStream& OutputStream::operator<< (const String& text)
{
    write ((const char*) text,
           text.length());

    return *this;
}

void OutputStream::writeText (const String& text,
                              const bool asUnicode,
                              const bool writeUnicodeHeaderBytes)
{
    if (asUnicode)
    {
        if (writeUnicodeHeaderBytes)
            write ("\x0ff\x0fe", 2);

        const juce_wchar* src = (const juce_wchar*) text;
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
        const char* src = (const char*) text;
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

int OutputStream::writeFromInputStream (InputStream& source,
                                        int numBytesToWrite)
{
    if (numBytesToWrite < 0)
        numBytesToWrite = 0x7fffffff;

    int numWritten = 0;

    while (numBytesToWrite > 0 && ! source.isExhausted())
    {
        char buffer [8192];

        const int num = source.read (buffer, (int) jmin ((size_t) numBytesToWrite, sizeof (buffer)));

        if (num == 0)
            break;

        write (buffer, num);

        numBytesToWrite -= num;
        numWritten += num;
    }

    return numWritten;
}


END_JUCE_NAMESPACE
