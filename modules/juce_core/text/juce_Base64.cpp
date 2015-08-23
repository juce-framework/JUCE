/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

bool Base64::convertToBase64 (OutputStream& base64Result, const void* sourceData, size_t sourceDataSize)
{
    static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const uint8* source = static_cast<const uint8*> (sourceData);

    while (sourceDataSize > 0)
    {
        char frame[4];
        const uint8 byte0 = *source++;
        frame[0] = lookup [(byte0 & 0xfcu) >> 2];
        uint32 bits = (byte0 & 0x03u) << 4;

        if (sourceDataSize > 1)
        {
            const uint8 byte1 = *source++;
            frame[1] = lookup[bits | ((byte1 & 0xf0u) >> 4)];
            bits = (byte1 & 0x0fu) << 2;

            if (sourceDataSize > 2)
            {
                const uint8 byte2 = *source++;
                frame[2] = lookup[bits | ((byte2 & 0xc0u) >> 6)];
                frame[3] = lookup[byte2 & 0x3fu];
                sourceDataSize -= 3;
            }
            else
            {
                frame[2] = lookup[bits];
                frame[3] = '=';
                sourceDataSize = 0;
            }
        }
        else
        {
            frame[1] = lookup[bits];
            frame[2] = '=';
            frame[3] = '=';
            sourceDataSize = 0;
        }

        if (! base64Result.write (frame, 4))
            return false;
    }

    return true;
}

bool Base64::convertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput)
{
    for (String::CharPointerType s = base64TextInput.text; ! s.isEmpty();)
    {
        uint8 data[4];

        for (int i = 0; i < 4; ++i)
        {
            uint32 c = (uint32) s.getAndAdvance();

            if (c >= 'A' && c <= 'Z')         c -= 'A';
            else if (c >= 'a' && c <= 'z')    c -= 'a' - 26;
            else if (c >= '0' && c <= '9')    c += 52 - '0';
            else if (c == '+')                c = 62;
            else if (c == '/')                c = 63;
            else if (c == '=')                { c = 64; if (i <= 1) return false; }
            else                              return false;

            data[i] = (uint8) c;
        }

        binaryOutput.writeByte ((char) ((data[0] << 2) | (data[1] >> 4)));

        if (data[2] < 64)
        {
            binaryOutput.writeByte ((char) ((data[1] << 4) | (data[2] >> 2)));

            if (data[3] < 64)
                binaryOutput.writeByte ((char) ((data[2] << 6) | data[3]));
        }
    }

    return true;
}

String Base64::toBase64 (const void* sourceData, size_t sourceDataSize)
{
    MemoryOutputStream m ((sourceDataSize * 4) / 3 + 3);
    bool ok = convertToBase64 (m, sourceData, sourceDataSize);
    jassert (ok); // should always succeed for this simple case
    ignoreUnused (ok);
    return m.toString();
}

String Base64::toBase64 (const String& text)
{
    return toBase64 (text.toRawUTF8(), strlen (text.toRawUTF8()));
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class Base64Tests  : public UnitTest
{
public:
    Base64Tests() : UnitTest ("Base64 class") {}

    static MemoryBlock createRandomData (Random& r)
    {
        MemoryOutputStream m;

        for (int i = r.nextInt (400); --i >= 0;)
            m.writeByte ((char) r.nextInt (256));

        return m.getMemoryBlock();
    }

    void runTest() override
    {
        beginTest ("Base64");

        Random r = getRandom();

        for (int i = 1000; --i >= 0;)
        {
            const MemoryBlock original (createRandomData (r));
            String asBase64 (Base64::toBase64 (original.getData(), original.getSize()));
            MemoryOutputStream out;
            expect (Base64::convertFromBase64 (out, asBase64));
            MemoryBlock result = out.getMemoryBlock();
            expect (result == original);
        }
    }
};

static Base64Tests base64Tests;

#endif
