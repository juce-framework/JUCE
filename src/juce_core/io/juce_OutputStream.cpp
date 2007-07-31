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

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_OutputStream.h"


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
    const unsigned short v = swapIfBigEndian ((unsigned short) value);
    write (&v, 2);
}

void OutputStream::writeShortBigEndian (short value)
{
    const unsigned short v = swapIfLittleEndian ((unsigned short) value);
    write (&v, 2);
}

void OutputStream::writeInt (int value)
{
    const unsigned int v = swapIfBigEndian ((unsigned int) value);
    write (&v, 4);
}

void OutputStream::writeIntBigEndian (int value)
{
    const unsigned int v = swapIfLittleEndian ((unsigned int) value);
    write (&v, 4);
}

void OutputStream::writeCompressedInt (int value)
{
    unsigned int un = (value < 0) ? (unsigned int) -value
                                  : (unsigned int) value;
    unsigned int tn = un;

    int numSigBytes = 0;

    do
    {
        tn >>= 8;
        numSigBytes++;

    } while (tn & 0xff);

    if (value < 0)
        numSigBytes |= 0x80;

    writeByte ((char) numSigBytes);
    write (&un, numSigBytes);
}

void OutputStream::writeInt64 (int64 value)
{
    writeInt ((int) (value & 0xffffffff));
    writeInt ((int) (value >> 32));
}

void OutputStream::writeInt64BigEndian (int64 value)
{
    writeInt ((int) (value >> 32));
    writeInt ((int) (value & 0xffffffff));
}

void OutputStream::writeFloat (float value)
{
    writeInt (*(int*) &value);
}

void OutputStream::writeDouble (double value)
{
    writeInt64 (*(int64*) &value);
}

void OutputStream::writeString (const String& text)
{
    const int numBytes = text.copyToUTF8 (0);
    uint8* const temp = (uint8*) juce_malloc (numBytes);

    text.copyToUTF8 (temp);
    write (temp, numBytes); // (numBytes includes the terminating null).

    juce_free (temp);
}

void OutputStream::printf (const char* pf, ...)
{
    unsigned int bufSize = 256;
    char* buf = (char*) juce_malloc (bufSize);

    for (;;)
    {
        va_list list;
        va_start (list, pf);

        const int num = CharacterFunctions::vprintf (buf, bufSize, pf, list);

        if (num > 0)
        {
            write (buf, num);
            break;
        }
        else if (num == 0)
        {
            break;
        }

        juce_free (buf);
        bufSize += 256;
        buf = (char*) juce_malloc (bufSize);
    }

    juce_free (buf);
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

        const int num = source.read (buffer, jmin (numBytesToWrite, sizeof (buffer)));

        if (num == 0)
            break;

        write (buffer, num);

        numBytesToWrite -= num;
        numWritten += num;
    }

    return numWritten;
}


END_JUCE_NAMESPACE
