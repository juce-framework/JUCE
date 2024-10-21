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
    Whirlpool hash class.

    The Whirlpool algorithm was developed by
    <a href="mailto:pbarreto@scopus.com.br">Paulo S. L. M. Barreto</a> and
    <a href="mailto:vincent.rijmen@cryptomathic.com">Vincent Rijmen</a>.

    See
        P.S.L.M. Barreto, V. Rijmen,
        "The Whirlpool hashing function"
        NESSIE submission, 2000 (tweaked version, 2001),
        https://www.cosic.esat.kuleuven.ac.be/nessie/workshop/submissions/whirlpool.zip

    @see SHA256, MD5

    @tags{Cryptography}
*/
class JUCE_API  Whirlpool
{
public:
    //==============================================================================
    /** Creates an empty Whirlpool object.
        The default constructor just creates a hash filled with zeros. (This is not
        equal to the hash of an empty block of data).
    */
    Whirlpool() noexcept;

    /** Destructor. */
    ~Whirlpool() noexcept;

    /** Creates a copy of another Whirlpool. */
    Whirlpool (const Whirlpool& other) noexcept;

    /** Copies another Whirlpool. */
    Whirlpool& operator= (const Whirlpool& other) noexcept;

    //==============================================================================
    /** Creates a hash from a block of raw data. */
    explicit Whirlpool (const MemoryBlock&);

    /** Creates a hash from a block of raw data. */
    Whirlpool (const void* data, size_t numBytes);

    /** Creates a hash from the contents of a stream.

        This will read from the stream until the stream is exhausted, or until
        maxBytesToRead bytes have been read. If maxBytesToRead is negative, the entire
        stream will be read.
    */
    Whirlpool (InputStream& input, int64 maxBytesToRead = -1);

    /** Reads a file and generates the hash of its contents.
        If the file can't be opened, the hash will be left uninitialised
        (i.e. full of zeros).
    */
    explicit Whirlpool (const File& file);

    /** Creates a checksum from a UTF-8 buffer.
        E.g.
        @code Whirlpool checksum (myString.toUTF8());
        @endcode
    */
    explicit Whirlpool (CharPointer_UTF8 utf8Text) noexcept;

    //==============================================================================
    /** Returns the hash as a 64-byte block of data. */
    MemoryBlock getRawData() const;

    /** Returns the checksum as a 128-digit hex string. */
    String toHexString() const;

    //==============================================================================
    bool operator== (const Whirlpool&) const noexcept;
    bool operator!= (const Whirlpool&) const noexcept;


private:
    //==============================================================================
    uint8 result [64];
    void process (const void*, size_t);

    JUCE_LEAK_DETECTOR (Whirlpool)
};

} // namespace juce
