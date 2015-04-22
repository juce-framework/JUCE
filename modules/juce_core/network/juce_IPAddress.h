/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_IPADDRESS_H_INCLUDED
#define JUCE_IPADDRESS_H_INCLUDED


//==============================================================================
/**
    An IPV4 address.
*/
class JUCE_API  IPAddress
{
public:
    //==============================================================================
    /** Populates a list of all the IP addresses that this machine is using. */
    static void findAllAddresses (Array<IPAddress>& results);

    //==============================================================================
    /** Creates a null address (0.0.0.0). */
    IPAddress() noexcept;

    /** Creates an address from 4 bytes. */
    explicit IPAddress (const uint8 bytes[4]) noexcept;

    /** Creates an address from 4 bytes. */
    IPAddress (uint8 address1, uint8 address2, uint8 address3, uint8 address4) noexcept;

    /** Creates an address from a packed 32-bit integer, where the MSB is
        the first number in the address, and the LSB is the last.
    */
    explicit IPAddress (uint32 asNativeEndian32Bit) noexcept;

    /** Parses a string IP address of the form "a.b.c.d". */
    explicit IPAddress (const String& address);

    /** Returns a dot-separated string in the form "1.2.3.4" */
    String toString() const;

    /** Returns an address meaning "any" (0.0.0.0) */
    static IPAddress any() noexcept;

    /** Returns an address meaning "broadcast" (255.255.255.255) */
    static IPAddress broadcast() noexcept;

    /** Returns an address meaning "localhost" (127.0.0.1) */
    static IPAddress local() noexcept;

    bool operator== (const IPAddress& other) const noexcept;
    bool operator!= (const IPAddress& other) const noexcept;

    /** The elements of the IP address. */
    uint8 address[4];
};


#endif   // JUCE_IPADDRESS_H_INCLUDED
