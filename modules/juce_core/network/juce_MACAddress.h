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

#ifndef JUCE_MACADDRESS_H_INCLUDED
#define JUCE_MACADDRESS_H_INCLUDED


//==============================================================================
/**
    Represents a MAC network card adapter address ID.
*/
class JUCE_API  MACAddress
{
public:
    //==============================================================================
    /** Populates a list of the MAC addresses of all the available network cards. */
    static void findAllAddresses (Array<MACAddress>& results);

    //==============================================================================
    /** Creates a null address (00-00-00-00-00-00). */
    MACAddress() noexcept;

    /** Creates a copy of another address. */
    MACAddress (const MACAddress&) noexcept;

    /** Creates a copy of another address. */
    MACAddress& operator= (const MACAddress&) noexcept;

    /** Creates an address from 6 bytes. */
    explicit MACAddress (const uint8 bytes[6]) noexcept;

    /** Creates an address from a hex string.
        If the string isn't a 6-byte hex value, this will just default-initialise
        the object.
    */
    explicit MACAddress (StringRef address);

    /** Returns a pointer to the 6 bytes that make up this address. */
    const uint8* getBytes() const noexcept        { return address; }

    /** Returns a dash-separated string in the form "11-22-33-44-55-66" */
    String toString() const;

    /** Returns a hex string of this address, using a custom separator between each byte. */
    String toString (StringRef separator) const;

    /** Returns the address in the lower 6 bytes of an int64.

        This uses a little-endian arrangement, with the first byte of the address being
        stored in the least-significant byte of the result value.
    */
    int64 toInt64() const noexcept;

    /** Returns true if this address is null (00-00-00-00-00-00). */
    bool isNull() const noexcept;

    bool operator== (const MACAddress&) const noexcept;
    bool operator!= (const MACAddress&) const noexcept;

    //==============================================================================
private:
    uint8 address[6];
};


#endif   // JUCE_MACADDRESS_H_INCLUDED
