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

#include "juce_Uuid.h"
#include "../basics/juce_Random.h"
#include "../basics/juce_Time.h"
#include "../basics/juce_SystemStats.h"


//==============================================================================
Uuid::Uuid()
{
    // do some serious mixing up of our MAC addresses and different types of time info,
    // plus a couple of passes of pseudo-random numbers over the whole thing.
    SystemStats::getMACAddresses (value.asInt64, 2);

    int i;
    for (i = 16; --i >= 0;)
    {
        Random r (Time::getHighResolutionTicks()
                    + Random::getSystemRandom().nextInt()
                    + value.asInt [i & 3]);

        value.asBytes[i] ^= (uint8) r.nextInt();
    }

    value.asInt64 [0] ^= Time::getHighResolutionTicks();
    value.asInt64 [1] ^= Time::currentTimeMillis();

    for (i = 4; --i >= 0;)
    {
        Random r (Time::getHighResolutionTicks() ^ value.asInt[i]);
        value.asInt[i] ^= r.nextInt();
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
