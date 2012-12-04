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
    ~Uuid() noexcept;

    /** Creates a copy of another UUID. */
    Uuid (const Uuid& other) noexcept;

    /** Copies another UUID. */
    Uuid& operator= (const Uuid& other) noexcept;

    //==============================================================================
    /** Returns true if the ID is zero. */
    bool isNull() const noexcept;

    bool operator== (const Uuid& other) const noexcept;
    bool operator!= (const Uuid& other) const noexcept;

    //==============================================================================
    /** Returns a stringified version of this UUID.

        A Uuid object can later be reconstructed from this string using operator= or
        the constructor that takes a string parameter.

        @returns a 32 character hex string.
    */
    String toString() const;

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
    const uint8* getRawData() const noexcept                { return uuid; }

    /** Creates a UUID from a 16-byte array.
        @see getRawData
    */
    Uuid (const uint8* rawData);

    /** Sets this UUID from 16-bytes of raw data. */
    Uuid& operator= (const uint8* rawData) noexcept;


private:
    //==============================================================================
    uint8 uuid[16];

    JUCE_LEAK_DETECTOR (Uuid)
};


#endif   // __JUCE_UUID_JUCEHEADER__
