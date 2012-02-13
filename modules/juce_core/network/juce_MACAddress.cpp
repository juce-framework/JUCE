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


MACAddress::MACAddress()
    : asInt64 (0)
{
}

MACAddress::MACAddress (const MACAddress& other)
    : asInt64 (other.asInt64)
{
}

MACAddress& MACAddress::operator= (const MACAddress& other)
{
    asInt64 = other.asInt64;
    return *this;
}

MACAddress::MACAddress (const uint8 bytes[6])
    : asInt64 (0)
{
    memcpy (asBytes, bytes, sizeof (asBytes));
}

String MACAddress::toString() const
{
    String s;

    for (int i = 0; i < numElementsInArray (asBytes); ++i)
    {
        s << String::toHexString ((int) asBytes[i]).paddedLeft ('0', 2);

        if (i < numElementsInArray (asBytes) - 1)
            s << '-';
    }

    return s;
}

int64 MACAddress::toInt64() const noexcept
{
    int64 n = 0;

    for (int i = numElementsInArray (asBytes); --i >= 0;)
        n = (n << 8) | asBytes[i];

    return n;
}

bool MACAddress::isNull() const noexcept                                { return asInt64 == 0; }

bool MACAddress::operator== (const MACAddress& other) const noexcept    { return asInt64 == other.asInt64; }
bool MACAddress::operator!= (const MACAddress& other) const noexcept    { return asInt64 != other.asInt64; }
