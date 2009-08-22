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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Uuid.h"
#include "juce_Random.h"
#include "juce_Time.h"
#include "juce_SystemStats.h"


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
        SystemStats::getMACAddresses (macAddresses, 2);
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

Uuid::~Uuid() throw()
{
}

Uuid::Uuid (const Uuid& other)
    : value (other.value)
{
}

Uuid& Uuid::operator= (const Uuid& other)
{
    if (this != &other)
        value = other.value;

    return *this;
}

bool Uuid::operator== (const Uuid& other) const
{
    return memcmp (value.asBytes, other.value.asBytes, 16) == 0;
}

bool Uuid::operator!= (const Uuid& other) const
{
    return ! operator== (other);
}

bool Uuid::isNull() const throw()
{
    return (value.asInt64 [0] == 0) && (value.asInt64 [1] == 0);
}

//==============================================================================
const String Uuid::toString() const
{
    return String::toHexString (value.asBytes, 16, 0);
}

Uuid::Uuid (const String& uuidString)
{
    operator= (uuidString);
}

Uuid& Uuid::operator= (const String& uuidString)
{
    int destIndex = 0;
    int i = 0;

    for (;;)
    {
        int byte = 0;

        for (int loop = 2; --loop >= 0;)
        {
            byte <<= 4;

            for (;;)
            {
                const tchar c = uuidString [i++];

                if (c >= T('0') && c <= T('9'))
                {
                    byte |= c - T('0');
                    break;
                }
                else if (c >= T('a') && c <= T('z'))
                {
                    byte |= c - (T('a') - 10);
                    break;
                }
                else if (c >= T('A') && c <= T('Z'))
                {
                    byte |= c - (T('A') - 10);
                    break;
                }
                else if (c == 0)
                {
                    while (destIndex < 16)
                        value.asBytes [destIndex++] = 0;

                    return *this;
                }
            }
        }

        value.asBytes [destIndex++] = (uint8) byte;
    }
}

//==============================================================================
Uuid::Uuid (const uint8* const rawData)
{
    operator= (rawData);
}

Uuid& Uuid::operator= (const uint8* const rawData)
{
    if (rawData != 0)
        memcpy (value.asBytes, rawData, 16);
    else
        zeromem (value.asBytes, 16);

    return *this;
}


END_JUCE_NAMESPACE
