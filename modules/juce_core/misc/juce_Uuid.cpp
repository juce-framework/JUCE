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


namespace
{
    int64 getRandomSeedFromMACAddresses()
    {
        Array<MACAddress> result;
        MACAddress::findAllAddresses (result);

        Random r;
        for (int i = 0; i < result.size(); ++i)
            r.combineSeed (result[i].toInt64());

        return r.nextInt64();
    }
}

//==============================================================================
Uuid::Uuid()
{
    // The normal random seeding is pretty good, but we'll throw some MAC addresses
    // into the mix too, to make it very very unlikely that two UUIDs will ever be the same..

    static Random r1 (getRandomSeedFromMACAddresses());

    value.asInt64[0] = r1.nextInt64();
    value.asInt64[1] = r1.nextInt64();

    Random r2;

    for (int i = 4; --i >= 0;)
        value.asInt[i] ^= r2.nextInt();
}

Uuid::~Uuid() noexcept
{
}

Uuid::Uuid (const Uuid& other)
    : value (other.value)
{
}

Uuid& Uuid::operator= (const Uuid& other)
{
    value = other.value;
    return *this;
}

bool Uuid::operator== (const Uuid& other) const
{
    return value.asInt64[0] == other.value.asInt64[0]
        && value.asInt64[1] == other.value.asInt64[1];
}

bool Uuid::operator!= (const Uuid& other) const
{
    return ! operator== (other);
}

bool Uuid::isNull() const noexcept
{
    return (value.asInt64 [0] == 0) && (value.asInt64 [1] == 0);
}

//==============================================================================
String Uuid::toString() const
{
    return String::toHexString (value.asBytes, sizeof (value.asBytes), 0);
}

Uuid::Uuid (const String& uuidString)
{
    operator= (uuidString);
}

Uuid& Uuid::operator= (const String& uuidString)
{
    MemoryBlock mb;
    mb.loadFromHexString (uuidString);
    mb.ensureSize (sizeof (value.asBytes), true);
    mb.copyTo (value.asBytes, 0, sizeof (value.asBytes));
    return *this;
}

//==============================================================================
Uuid::Uuid (const uint8* const rawData)
{
    operator= (rawData);
}

Uuid& Uuid::operator= (const uint8* const rawData)
{
    if (rawData != nullptr)
        memcpy (value.asBytes, rawData, sizeof (value.asBytes));
    else
        zeromem (value.asBytes, sizeof (value.asBytes));

    return *this;
}
