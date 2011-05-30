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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Uuid.h"
#include "../maths/juce_Random.h"
#include "juce_Time.h"
#include "../io/network/juce_MACAddress.h"
#include "../memory/juce_MemoryBlock.h"


//==============================================================================
Uuid::Uuid()
{
    // Mix up any available MAC addresses with some time-based pseudo-random numbers
    // to make it very very unlikely that two UUIDs will ever be the same..

    static int64 macAddresses[2];
    static bool hasCheckedMacAddresses = false;

    if (! hasCheckedMacAddresses)
    {
        hasCheckedMacAddresses = true;

        Array<MACAddress> result;
        MACAddress::findAllAddresses (result);

        for (int i = 0; i < numElementsInArray (macAddresses); ++i)
            macAddresses[i] = result[i].toInt64();
    }

    value.asInt64[0] = macAddresses[0];
    value.asInt64[1] = macAddresses[1];

    // We'll use both a local RNG that is re-seeded, plus the shared RNG,
    // whose seed will carry over between calls to this method.

    Random r (macAddresses[0] ^ macAddresses[1]
                ^ Random::getSystemRandom().nextInt64());

    for (int i = 4; --i >= 0;)
    {
        r.setSeedRandomly(); // calling this repeatedly improves randomness
        value.asInt[i] ^= r.nextInt();
        value.asInt[i] ^= Random::getSystemRandom().nextInt();
    }
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


END_JUCE_NAMESPACE
