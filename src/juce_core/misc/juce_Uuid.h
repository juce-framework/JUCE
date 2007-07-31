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

#ifndef __JUCE_UUID_JUCEHEADER__
#define __JUCE_UUID_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/**
    A universally unique 128-bit identifier.

    This class generates very random unique numbers based on the system time
    and MAC addresses if any are available. It's extremely unlikely that two identical
    UUIDs would ever be created by chance.

    The class includes methods for saving the ID as a string or as raw binary data.
*/
class JUCE_API  Uuid
{
public:
    //==============================================================================
    /** Creates a new unique ID. */
    Uuid();

    /** Destructor. */
    ~Uuid() throw();

    /** Creates a copy of another UUID. */
    Uuid (const Uuid& other);

    /** Copies another UUID. */
    Uuid& operator= (const Uuid& other);

    /** Returns true if the ID is zero. */
    bool isNull() const throw();

    //==============================================================================
    /** Compares two UUIDs. */
    bool operator== (const Uuid& other) const;

    /** Compares two UUIDs. */
    bool operator!= (const Uuid& other) const;

    //==============================================================================
    /** Returns a stringified version of this UUID.

        A Uuid object can later be reconstructed from this string using operator= or
        the constructor that takes a string parameter.

        @returns a 32 character hex string.
    */
    const String toString() const;

    /** Creates an ID from an encoded string version.

        @see toString
    */
    Uuid (const String& uuidString);

    /** Copies from a stringified UUID.

        The string passed in should be one that was created with the toString() method.
    */
    Uuid& operator= (const String& uuidString);


    //==============================================================================
    /** Returns a pointer to the internal binary representation of the ID.

        This is an array of 16 bytes. To reconstruct a Uuid from its data, use
        the constructor or operator= method that takes an array of uint8s.
    */
    const uint8* getRawData() const throw()                 { return value.asBytes; }

    /** Creates a UUID from a 16-byte array.

        @see getRawData
    */
    Uuid (const uint8* const rawData);

    /** Sets this UUID from 16-bytes of raw data. */
    Uuid& operator= (const uint8* const rawData);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    union
    {
        uint8 asBytes [16];
        int asInt[4];
        int64 asInt64[2];

    } value;
};


#endif   // __JUCE_UUID_JUCEHEADER__
