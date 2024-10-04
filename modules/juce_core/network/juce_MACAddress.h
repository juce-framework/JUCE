/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Represents a MAC network card adapter address ID.

    @tags{Core}
*/
class JUCE_API  MACAddress  final
{
public:
    //==============================================================================
    /** Returns a list of the MAC addresses of all the available network cards. */
    static Array<MACAddress> getAllAddresses();

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

} // namespace juce
