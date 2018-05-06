/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
