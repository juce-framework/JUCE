/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_UUID_H_INCLUDED
#define JUCE_UUID_H_INCLUDED


//==============================================================================
/**
    A universally unique 128-bit identifier.

    This class generates very random unique numbers. It's vanishingly unlikely
    that two identical UUIDs would ever be created by chance. The values are
    formatted to meet the RFC 4122 version 4 standard.

    The class includes methods for saving the ID as a string or as raw binary data.
*/
class JUCE_API  Uuid
{
public:
    //==============================================================================
    /** Creates a new unique ID, compliant with RFC 4122 version 4. */
    Uuid();

    /** Destructor. */
    ~Uuid() noexcept;

    /** Creates a copy of another UUID. */
    Uuid (const Uuid&) noexcept;

    /** Copies another UUID. */
    Uuid& operator= (const Uuid&) noexcept;

    //==============================================================================
    /** Returns true if the ID is zero. */
    bool isNull() const noexcept;

    /** Returns a null Uuid object. */
    static Uuid null() noexcept;

    bool operator== (const Uuid&) const noexcept;
    bool operator!= (const Uuid&) const noexcept;

    //==============================================================================
    /** Returns a stringified version of this UUID.

        A Uuid object can later be reconstructed from this string using operator= or
        the constructor that takes a string parameter.

        @returns a 32 character hex string.
    */
    String toString() const;

    /** Returns a stringified version of this UUID, separating it into sections with dashes.
        @returns a string in the format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    */
    String toDashedString() const;

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
    Uuid (const uint8* rawData) noexcept;

    /** Sets this UUID from 16-bytes of raw data. */
    Uuid& operator= (const uint8* rawData) noexcept;


private:
    //==============================================================================
    uint8 uuid[16];
    String getHexRegion (int, int) const;

    JUCE_LEAK_DETECTOR (Uuid)
};


#endif   // JUCE_UUID_H_INCLUDED
