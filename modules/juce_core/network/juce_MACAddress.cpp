/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

MACAddress::MACAddress()
{
    zeromem (address, sizeof (address));
}

MACAddress::MACAddress (const MACAddress& other)
{
    memcpy (address, other.address, sizeof (address));
}

MACAddress& MACAddress::operator= (const MACAddress& other)
{
    memcpy (address, other.address, sizeof (address));
    return *this;
}

MACAddress::MACAddress (const uint8 bytes[6])
{
    memcpy (address, bytes, sizeof (address));
}

String MACAddress::toString() const
{
    String s;

    for (size_t i = 0; i < sizeof (address); ++i)
    {
        s << String::toHexString ((int) address[i]).paddedLeft ('0', 2);

        if (i < sizeof (address) - 1)
            s << '-';
    }

    return s;
}

int64 MACAddress::toInt64() const noexcept
{
    int64 n = 0;

    for (int i = (int) sizeof (address); --i >= 0;)
        n = (n << 8) | address[i];

    return n;
}

bool MACAddress::isNull() const noexcept                                { return toInt64() == 0; }

bool MACAddress::operator== (const MACAddress& other) const noexcept    { return memcmp (address, other.address, sizeof (address)) == 0; }
bool MACAddress::operator!= (const MACAddress& other) const noexcept    { return ! operator== (other); }
