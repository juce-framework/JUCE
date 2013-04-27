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

#ifndef __JUCE_IPADDRESS_JUCEHEADER__
#define __JUCE_IPADDRESS_JUCEHEADER__


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


#endif   // __JUCE_IPADDRESS_JUCEHEADER__
