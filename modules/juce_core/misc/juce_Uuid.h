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
    A universally unique 128-bit identifier.

    This class generates very random unique numbers. It's vanishingly unlikely
    that two identical UUIDs would ever be created by chance. The values are
    formatted to meet the RFC 4122 version 4 standard.

    The class includes methods for saving the ID as a string or as raw binary data.

    @tags{Core}
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
    bool operator<  (const Uuid&) const noexcept;
    bool operator>  (const Uuid&) const noexcept;
    bool operator<= (const Uuid&) const noexcept;
    bool operator>= (const Uuid&) const noexcept;

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
    /** Returns the time-low section of the UUID. */
    uint32 getTimeLow() const noexcept;
    /** Returns the time-mid section of the UUID. */
    uint16 getTimeMid() const noexcept;
    /** Returns the time-high-and-version section of the UUID. */
    uint16 getTimeHighAndVersion() const noexcept;
    /** Returns the clock-seq-and-reserved section of the UUID. */
    uint8  getClockSeqAndReserved() const noexcept;
    /** Returns the clock-seq-low section of the UUID. */
    uint8  getClockSeqLow() const noexcept;
    /** Returns the node section of the UUID. */
    uint64 getNode() const noexcept;

    /** Returns a hash of the UUID. */
    uint64 hash() const noexcept;

    //==============================================================================
    /** Returns a pointer to the internal binary representation of the ID.

        This is an array of 16 bytes. To reconstruct a Uuid from its data, use
        the constructor or operator= method that takes an array of uint8s.
    */
    const uint8* getRawData() const noexcept                { return uuid; }

    /** Returns the size in bytes of the raw data making up this UUID. */
    static constexpr size_t size() { return sizeInBytes; }

    /** Creates a UUID from a 16-byte array.
        @see getRawData
    */
    Uuid (const uint8* rawData) noexcept;

    /** Sets this UUID from 16-bytes of raw data. */
    Uuid& operator= (const uint8* rawData) noexcept;

private:
    //==============================================================================
    static constexpr size_t sizeInBytes = 16;
    uint8 uuid[sizeInBytes];
    String getHexRegion (int, int) const;
    int compare (Uuid) const noexcept;

    JUCE_LEAK_DETECTOR (Uuid)
};

} // namespace juce

/** @cond */
namespace std
{
    template <> struct hash<juce::Uuid>
    {
        size_t operator() (const juce::Uuid& u) const noexcept   { return (size_t) u.hash(); }
    };
}
/** @endcond */
