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
    This class a container which holds all the classes pertaining to the AudioData::Pointer
    audio sample format class.

    @see AudioData::Pointer.

    @tags{Audio}
*/
class JUCE_API  AudioData
{
public:
    //==============================================================================
    // These types can be used as the SampleFormat template parameter for the AudioData::Pointer class.

    class Int8;       /**< Used as a template parameter for AudioData::Pointer. Indicates an 8-bit integer packed data format. */
    class UInt8;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 8-bit unsigned integer packed data format. */
    class Int16;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 16-bit integer packed data format. */
    class Int24;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 24-bit integer packed data format. */
    class Int32;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 32-bit integer packed data format. */
    class Float32;    /**< Used as a template parameter for AudioData::Pointer. Indicates an 32-bit float data format. */

    //==============================================================================
    // These types can be used as the Endianness template parameter for the AudioData::Pointer class.

    class BigEndian;      /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in big-endian order. */
    class LittleEndian;   /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in little-endian order. */
    class NativeEndian;   /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in the CPU's native endianness. */

    //==============================================================================
    // These types can be used as the InterleavingType template parameter for the AudioData::Pointer class.

    class NonInterleaved; /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored contiguously. */
    class Interleaved;    /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are interleaved with a number of other channels. */

    //==============================================================================
    // These types can be used as the Constness template parameter for the AudioData::Pointer class.

    class NonConst; /**< Used as a template parameter for AudioData::Pointer. Indicates that the pointer can be used for non-const data. */
    class Const;    /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples can only be used for const data.. */


    //==============================================================================
    /** @cond */
    class BigEndian
    {
    public:
        template <class SampleFormatType> static float getAsFloat (SampleFormatType& s) noexcept                         { return s.getAsFloatBE(); }
        template <class SampleFormatType> static void  setAsFloat (SampleFormatType& s, float newValue) noexcept         { s.setAsFloatBE (newValue); }
        template <class SampleFormatType> static int32 getAsInt32 (SampleFormatType& s) noexcept                         { return s.getAsInt32BE(); }
        template <class SampleFormatType> static void  setAsInt32 (SampleFormatType& s, int32 newValue) noexcept         { s.setAsInt32BE (newValue); }
        template <class SourceType, class DestType> static void copyFrom (DestType& dest, SourceType& source) noexcept   { dest.copyFromBE (source); }
        enum { isBigEndian = 1 };
    };

    class LittleEndian
    {
    public:
        template <class SampleFormatType> static float getAsFloat (SampleFormatType& s) noexcept                         { return s.getAsFloatLE(); }
        template <class SampleFormatType> static void  setAsFloat (SampleFormatType& s, float newValue) noexcept         { s.setAsFloatLE (newValue); }
        template <class SampleFormatType> static int32 getAsInt32 (SampleFormatType& s) noexcept                         { return s.getAsInt32LE(); }
        template <class SampleFormatType> static void  setAsInt32 (SampleFormatType& s, int32 newValue) noexcept         { s.setAsInt32LE (newValue); }
        template <class SourceType, class DestType> static void copyFrom (DestType& dest, SourceType& source) noexcept   { dest.copyFromLE (source); }
        enum { isBigEndian = 0 };
    };

    #if JUCE_BIG_ENDIAN
     class NativeEndian   : public BigEndian  {};
    #else
     class NativeEndian   : public LittleEndian  {};
    #endif

    //==============================================================================
    class Int8
    {
    public:
        inline Int8 (void* d) noexcept  : data (static_cast<int8*> (d))  {}

        inline void advance() noexcept                          { ++data; }
        inline void skip (int numSamples) noexcept              { data += numSamples; }
        inline float getAsFloatLE() const noexcept              { return (float) (*data * (1.0 / (1.0 + (double) maxValue))); }
        inline float getAsFloatBE() const noexcept              { return getAsFloatLE(); }
        inline void setAsFloatLE (float newValue) noexcept      { *data = (int8) jlimit ((int) -maxValue, (int) maxValue, roundToInt (newValue * (1.0 + (double) maxValue))); }
        inline void setAsFloatBE (float newValue) noexcept      { setAsFloatLE (newValue); }
        inline int32 getAsInt32LE() const noexcept              { return (int) (*((uint8*) data) << 24); }
        inline int32 getAsInt32BE() const noexcept              { return getAsInt32LE(); }
        inline void setAsInt32LE (int newValue) noexcept        { *data = (int8) (newValue >> 24); }
        inline void setAsInt32BE (int newValue) noexcept        { setAsInt32LE (newValue); }
        inline void clear() noexcept                            { *data = 0; }
        inline void clearMultiple (int num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int8& source) noexcept    { *data = *source.data; }

        int8* data;
        enum { bytesPerSample = 1, maxValue = 0x7f, resolution = (1 << 24), isFloat = 0 };
    };

    class UInt8
    {
    public:
        inline UInt8 (void* d) noexcept  : data (static_cast<uint8*> (d))  {}

        inline void advance() noexcept                          { ++data; }
        inline void skip (int numSamples) noexcept              { data += numSamples; }
        inline float getAsFloatLE() const noexcept              { return (float) ((*data - 128) * (1.0 / (1.0 + (double) maxValue))); }
        inline float getAsFloatBE() const noexcept              { return getAsFloatLE(); }
        inline void setAsFloatLE (float newValue) noexcept      { *data = (uint8) jlimit (0, 255, 128 + roundToInt (newValue * (1.0 + (double) maxValue))); }
        inline void setAsFloatBE (float newValue) noexcept      { setAsFloatLE (newValue); }
        inline int32 getAsInt32LE() const noexcept              { return (int) (((uint8) (*data - 128)) << 24); }
        inline int32 getAsInt32BE() const noexcept              { return getAsInt32LE(); }
        inline void setAsInt32LE (int newValue) noexcept        { *data = (uint8) (128 + (newValue >> 24)); }
        inline void setAsInt32BE (int newValue) noexcept        { setAsInt32LE (newValue); }
        inline void clear() noexcept                            { *data = 128; }
        inline void clearMultiple (int num) noexcept            { memset (data, 128, (size_t) num) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (UInt8& source) noexcept   { *data = *source.data; }

        uint8* data;
        enum { bytesPerSample = 1, maxValue = 0x7f, resolution = (1 << 24), isFloat = 0 };
    };

    class Int16
    {
    public:
        inline Int16 (void* d) noexcept  : data (static_cast<uint16*> (d))  {}

        inline void advance() noexcept                          { ++data; }
        inline void skip (int numSamples) noexcept              { data += numSamples; }
        inline float getAsFloatLE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int16) ByteOrder::swapIfBigEndian    (*data)); }
        inline float getAsFloatBE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int16) ByteOrder::swapIfLittleEndian (*data)); }
        inline void setAsFloatLE (float newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint16) jlimit ((int) -maxValue, (int) maxValue, roundToInt (newValue * (1.0 + (double) maxValue)))); }
        inline void setAsFloatBE (float newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint16) jlimit ((int) -maxValue, (int) maxValue, roundToInt (newValue * (1.0 + (double) maxValue)))); }
        inline int32 getAsInt32LE() const noexcept              { return (int32) (ByteOrder::swapIfBigEndian    ((uint16) *data) << 16); }
        inline int32 getAsInt32BE() const noexcept              { return (int32) (ByteOrder::swapIfLittleEndian ((uint16) *data) << 16); }
        inline void setAsInt32LE (int32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint16) (newValue >> 16)); }
        inline void setAsInt32BE (int32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint16) (newValue >> 16)); }
        inline void clear() noexcept                            { *data = 0; }
        inline void clearMultiple (int num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int16& source) noexcept   { *data = *source.data; }

        uint16* data;
        enum { bytesPerSample = 2, maxValue = 0x7fff, resolution = (1 << 16), isFloat = 0 };
    };

    class Int24
    {
    public:
        inline Int24 (void* d) noexcept  : data (static_cast<char*> (d))  {}

        inline void advance() noexcept                          { data += 3; }
        inline void skip (int numSamples) noexcept              { data += 3 * numSamples; }
        inline float getAsFloatLE() const noexcept              { return (float) (ByteOrder::littleEndian24Bit (data) * (1.0 / (1.0 + (double) maxValue))); }
        inline float getAsFloatBE() const noexcept              { return (float) (ByteOrder::bigEndian24Bit    (data) * (1.0 / (1.0 + (double) maxValue))); }
        inline void setAsFloatLE (float newValue) noexcept      { ByteOrder::littleEndian24BitToChars (jlimit ((int) -maxValue, (int) maxValue, roundToInt (newValue * (1.0 + (double) maxValue))), data); }
        inline void setAsFloatBE (float newValue) noexcept      { ByteOrder::bigEndian24BitToChars (jlimit    ((int) -maxValue, (int) maxValue, roundToInt (newValue * (1.0 + (double) maxValue))), data); }
        inline int32 getAsInt32LE() const noexcept              { return (int32) (((unsigned int) ByteOrder::littleEndian24Bit (data)) << 8); }
        inline int32 getAsInt32BE() const noexcept              { return (int32) (((unsigned int) ByteOrder::bigEndian24Bit    (data)) << 8); }
        inline void setAsInt32LE (int32 newValue) noexcept      { ByteOrder::littleEndian24BitToChars (newValue >> 8, data); }
        inline void setAsInt32BE (int32 newValue) noexcept      { ByteOrder::bigEndian24BitToChars    (newValue >> 8, data); }
        inline void clear() noexcept                            { data[0] = 0; data[1] = 0; data[2] = 0; }
        inline void clearMultiple (int num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int24& source) noexcept   { data[0] = source.data[0]; data[1] = source.data[1]; data[2] = source.data[2]; }

        char* data;
        enum { bytesPerSample = 3, maxValue = 0x7fffff, resolution = (1 << 8), isFloat = 0 };
    };

    class Int32
    {
    public:
        inline Int32 (void* d) noexcept  : data (static_cast<uint32*> (d))  {}

        inline void advance() noexcept                          { ++data; }
        inline void skip (int numSamples) noexcept              { data += numSamples; }
        inline float getAsFloatLE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int32) ByteOrder::swapIfBigEndian    (*data)); }
        inline float getAsFloatBE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int32) ByteOrder::swapIfLittleEndian (*data)); }
        inline void setAsFloatLE (float newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint32) (int32) ((double) maxValue * jlimit (-1.0, 1.0, (double) newValue))); }
        inline void setAsFloatBE (float newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint32) (int32) ((double) maxValue * jlimit (-1.0, 1.0, (double) newValue))); }
        inline int32 getAsInt32LE() const noexcept              { return (int32) ByteOrder::swapIfBigEndian    (*data); }
        inline int32 getAsInt32BE() const noexcept              { return (int32) ByteOrder::swapIfLittleEndian (*data); }
        inline void setAsInt32LE (int32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint32) newValue); }
        inline void setAsInt32BE (int32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint32) newValue); }
        inline void clear() noexcept                            { *data = 0; }
        inline void clearMultiple (int num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int32& source) noexcept   { *data = *source.data; }

        uint32* data;
        enum { bytesPerSample = 4, maxValue = 0x7fffffff, resolution = 1, isFloat = 0 };
    };

    /** A 32-bit integer type, of which only the bottom 24 bits are used. */
    class Int24in32  : public Int32
    {
    public:
        inline Int24in32 (void* d) noexcept  : Int32 (d)  {}

        inline float getAsFloatLE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int32) ByteOrder::swapIfBigEndian (*data)); }
        inline float getAsFloatBE() const noexcept              { return (float) ((1.0 / (1.0 + (double) maxValue)) * (int32) ByteOrder::swapIfLittleEndian (*data)); }
        inline void setAsFloatLE (float newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint32) ((double) maxValue * jlimit (-1.0, 1.0, (double) newValue))); }
        inline void setAsFloatBE (float newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint32) ((double) maxValue * jlimit (-1.0, 1.0, (double) newValue))); }
        inline int32 getAsInt32LE() const noexcept              { return (int32) ByteOrder::swapIfBigEndian    (*data) << 8; }
        inline int32 getAsInt32BE() const noexcept              { return (int32) ByteOrder::swapIfLittleEndian (*data) << 8; }
        inline void setAsInt32LE (int32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((uint32) newValue >> 8); }
        inline void setAsInt32BE (int32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((uint32) newValue >> 8); }
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int24in32& source) noexcept { *data = *source.data; }

        enum { bytesPerSample = 4, maxValue = 0x7fffff, resolution = (1 << 8), isFloat = 0 };
    };

    class Float32
    {
    public:
        inline Float32 (void* d) noexcept  : data (static_cast<float*> (d))  {}

        inline void advance() noexcept                          { ++data; }
        inline void skip (int numSamples) noexcept              { data += numSamples; }
       #if JUCE_BIG_ENDIAN
        inline float getAsFloatBE() const noexcept              { return *data; }
        inline void setAsFloatBE (float newValue) noexcept      { *data = newValue; }
        inline float getAsFloatLE() const noexcept              { union { uint32 asInt; float asFloat; } n; n.asInt = ByteOrder::swap (*(uint32*) data); return n.asFloat; }
        inline void setAsFloatLE (float newValue) noexcept      { union { uint32 asInt; float asFloat; } n; n.asFloat = newValue; *(uint32*) data = ByteOrder::swap (n.asInt); }
       #else
        inline float getAsFloatLE() const noexcept              { return *data; }
        inline void setAsFloatLE (float newValue) noexcept      { *data = newValue; }
        inline float getAsFloatBE() const noexcept              { union { uint32 asInt; float asFloat; } n; n.asInt = ByteOrder::swap (*(uint32*) data); return n.asFloat; }
        inline void setAsFloatBE (float newValue) noexcept      { union { uint32 asInt; float asFloat; } n; n.asFloat = newValue; *(uint32*) data = ByteOrder::swap (n.asInt); }
       #endif
        inline int32 getAsInt32LE() const noexcept              { return (int32) roundToInt (jlimit (-1.0, 1.0, (double) getAsFloatLE()) * (double) maxValue); }
        inline int32 getAsInt32BE() const noexcept              { return (int32) roundToInt (jlimit (-1.0, 1.0, (double) getAsFloatBE()) * (double) maxValue); }
        inline void setAsInt32LE (int32 newValue) noexcept      { setAsFloatLE ((float) (newValue * (1.0 / (1.0 + (double) maxValue)))); }
        inline void setAsInt32BE (int32 newValue) noexcept      { setAsFloatBE ((float) (newValue * (1.0 / (1.0 + (double) maxValue)))); }
        inline void clear() noexcept                            { *data = 0; }
        inline void clearMultiple (int num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline void copyFromLE (SourceType& source) noexcept    { setAsFloatLE (source.getAsFloat()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) noexcept    { setAsFloatBE (source.getAsFloat()); }
        inline void copyFromSameType (Float32& source) noexcept { *data = *source.data; }

        float* data;
        enum { bytesPerSample = 4, maxValue = 0x7fffffff, resolution = (1 << 8), isFloat = 1 };
    };

    //==============================================================================
    class NonInterleaved
    {
    public:
        inline NonInterleaved() = default;
        inline NonInterleaved (const NonInterleaved&) = default;
        inline NonInterleaved (int) noexcept {}
        inline void copyFrom (const NonInterleaved&) noexcept {}
        template <class SampleFormatType> inline void advanceData (SampleFormatType& s) noexcept                    { s.advance(); }
        template <class SampleFormatType> inline void advanceDataBy (SampleFormatType& s, int numSamples) noexcept  { s.skip (numSamples); }
        template <class SampleFormatType> inline void clear (SampleFormatType& s, int numSamples) noexcept          { s.clearMultiple (numSamples); }
        template <class SampleFormatType> static int getNumBytesBetweenSamples (const SampleFormatType&) noexcept   { return SampleFormatType::bytesPerSample; }

        enum { isInterleavedType = 0, numInterleavedChannels = 1 };
    };

    class Interleaved
    {
    public:
        inline Interleaved() noexcept  {}
        inline Interleaved (const Interleaved& other) = default;
        inline Interleaved (const int numInterleavedChans) noexcept : numInterleavedChannels (numInterleavedChans) {}
        inline void copyFrom (const Interleaved& other) noexcept { numInterleavedChannels = other.numInterleavedChannels; }
        template <class SampleFormatType> inline void advanceData (SampleFormatType& s) noexcept                    { s.skip (numInterleavedChannels); }
        template <class SampleFormatType> inline void advanceDataBy (SampleFormatType& s, int numSamples) noexcept  { s.skip (numInterleavedChannels * numSamples); }
        template <class SampleFormatType> inline void clear (SampleFormatType& s, int numSamples) noexcept          { while (--numSamples >= 0) { s.clear(); s.skip (numInterleavedChannels); } }
        template <class SampleFormatType> inline int getNumBytesBetweenSamples (const SampleFormatType&) const noexcept { return numInterleavedChannels * SampleFormatType::bytesPerSample; }
        int numInterleavedChannels = 1;
        enum { isInterleavedType = 1 };
    };

    //==============================================================================
    class NonConst
    {
    public:
        using VoidType = void;
        static void* toVoidPtr (VoidType* v) noexcept { return v; }
        enum { isConst = 0 };
    };

    class Const
    {
    public:
        using VoidType = const void;
        static void* toVoidPtr (VoidType* v) noexcept { return const_cast<void*> (v); }
        enum { isConst = 1 };
    };
    /** @endcond */

    //==============================================================================
    /**
        A pointer to a block of audio data with a particular encoding.

        This object can be used to read and write from blocks of encoded audio samples. To create one, you specify
        the audio format as a series of template parameters, e.g.
        @code
        // this creates a pointer for reading from a const array of 16-bit little-endian packed samples.
        AudioData::Pointer <AudioData::Int16,
                            AudioData::LittleEndian,
                            AudioData::NonInterleaved,
                            AudioData::Const> pointer (someRawAudioData);

        // These methods read the sample that is being pointed to
        float firstSampleAsFloat = pointer.getAsFloat();
        int32 firstSampleAsInt = pointer.getAsInt32();
        ++pointer; // moves the pointer to the next sample.
        pointer += 3; // skips the next 3 samples.
        @endcode

        The convertSamples() method lets you copy a range of samples from one format to another, automatically
        converting its format.

        @see AudioData::Converter
    */
    template <typename SampleFormat,
              typename Endianness,
              typename InterleavingType,
              typename Constness>
    class Pointer  : private InterleavingType  // (inherited for EBCO)
    {
    public:
        //==============================================================================
        /** Creates a non-interleaved pointer from some raw data in the appropriate format.
            This constructor is only used if you've specified the AudioData::NonInterleaved option -
            for interleaved formats, use the constructor that also takes a number of channels.
        */
        Pointer (typename Constness::VoidType* sourceData) noexcept
            : data (Constness::toVoidPtr (sourceData))
        {
            // If you're using interleaved data, call the other constructor! If you're using non-interleaved data,
            // you should pass NonInterleaved as the template parameter for the interleaving type!
            static_assert (InterleavingType::isInterleavedType == 0, "Incorrect constructor for interleaved data");
        }

        /** Creates a pointer from some raw data in the appropriate format with the specified number of interleaved channels.
            For non-interleaved data, use the other constructor.
        */
        Pointer (typename Constness::VoidType* sourceData, int numInterleaved) noexcept
            : InterleavingType (numInterleaved), data (Constness::toVoidPtr (sourceData))
        {
        }

        /** Creates a copy of another pointer. */
        Pointer (const Pointer& other) noexcept
            : InterleavingType (other), data (other.data)
        {
        }

        Pointer& operator= (const Pointer& other) noexcept
        {
            InterleavingType::operator= (other);
            data = other.data;
            return *this;
        }

        //==============================================================================
        /** Returns the value of the first sample as a floating point value.
            The value will be in the range -1.0 to 1.0 for integer formats. For floating point
            formats, the value could be outside that range, although -1 to 1 is the standard range.
        */
        inline float getAsFloat() const noexcept                { return Endianness::getAsFloat (data); }

        /** Sets the value of the first sample as a floating point value.

            (This method can only be used if the AudioData::NonConst option was used).
            The value should be in the range -1.0 to 1.0 - for integer formats, values outside that
            range will be clipped. For floating point formats, any value passed in here will be
            written directly, although -1 to 1 is the standard range.
        */
        inline void setAsFloat (float newValue) noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");
            Endianness::setAsFloat (data, newValue);
        }

        /** Returns the value of the first sample as a 32-bit integer.
            The value returned will be in the range 0x80000000 to 0x7fffffff, and shorter values will be
            shifted to fill this range (e.g. if you're reading from 24-bit data, the values will be shifted up
            by 8 bits when returned here). If the source data is floating point, values beyond -1.0 to 1.0 will
            be clipped so that -1.0 maps onto -0x7fffffff and 1.0 maps to 0x7fffffff.
        */
        inline int32 getAsInt32() const noexcept                { return Endianness::getAsInt32 (data); }

        /** Sets the value of the first sample as a 32-bit integer.
            This will be mapped to the range of the format that is being written - see getAsInt32().
        */
        inline void setAsInt32 (int32 newValue) noexcept
        {
             // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");
            Endianness::setAsInt32 (data, newValue);
        }

        /** Moves the pointer along to the next sample. */
        inline Pointer& operator++() noexcept                   { advance(); return *this; }

        /** Moves the pointer back to the previous sample. */
        inline Pointer& operator--() noexcept                   { this->advanceDataBy (data, -1); return *this; }

        /** Adds a number of samples to the pointer's position. */
        Pointer& operator+= (int samplesToJump) noexcept        { this->advanceDataBy (data, samplesToJump); return *this; }

        /** Returns a new pointer with the specified offset from this pointer's position. */
        Pointer operator+ (int samplesToJump) const             { return Pointer { *this } += samplesToJump; }

        /** Writes a stream of samples into this pointer from another pointer.
            This will copy the specified number of samples, converting between formats appropriately.
        */
        void convertSamples (Pointer source, int numSamples) const noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");

            for (Pointer dest (*this); --numSamples >= 0;)
            {
                dest.data.copyFromSameType (source.data);
                dest.advance();
                source.advance();
            }
        }

        /** Writes a stream of samples into this pointer from another pointer.
            This will copy the specified number of samples, converting between formats appropriately.
        */
        template <class OtherPointerType>
        void convertSamples (OtherPointerType source, int numSamples) const noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");

            Pointer dest (*this);

            if (source.getRawData() != getRawData() || source.getNumBytesBetweenSamples() >= getNumBytesBetweenSamples())
            {
                while (--numSamples >= 0)
                {
                    Endianness::copyFrom (dest.data, source);
                    dest.advance();
                    ++source;
                }
            }
            else // copy backwards if we're increasing the sample width..
            {
                dest += numSamples;
                source += numSamples;

                while (--numSamples >= 0)
                    Endianness::copyFrom ((--dest).data, --source);
            }
        }

        /** Sets a number of samples to zero. */
        void clearSamples (int numSamples) const noexcept
        {
            Pointer dest (*this);
            dest.clear (dest.data, numSamples);
        }

        /** Scans a block of data, returning the lowest and highest levels as floats */
        Range<float> findMinAndMax (size_t numSamples) const noexcept
        {
            if (numSamples == 0)
                return Range<float>();

            Pointer dest (*this);

            if (isFloatingPoint())
            {
                float mn = dest.getAsFloat();
                dest.advance();
                float mx = mn;

                while (--numSamples > 0)
                {
                    const float v = dest.getAsFloat();
                    dest.advance();

                    if (mx < v)  mx = v;
                    if (v < mn)  mn = v;
                }

                return Range<float> (mn, mx);
            }

            int32 mn = dest.getAsInt32();
            dest.advance();
            int32 mx = mn;

            while (--numSamples > 0)
            {
                const int v = dest.getAsInt32();
                dest.advance();

                if (mx < v)  mx = v;
                if (v < mn)  mn = v;
            }

            return Range<float> ((float) mn * (float) (1.0 / (1.0 + (double) Int32::maxValue)),
                                 (float) mx * (float) (1.0 / (1.0 + (double) Int32::maxValue)));
        }

        /** Scans a block of data, returning the lowest and highest levels as floats */
        void findMinAndMax (size_t numSamples, float& minValue, float& maxValue) const noexcept
        {
            Range<float> r (findMinAndMax (numSamples));
            minValue = r.getStart();
            maxValue = r.getEnd();
        }

        /** Returns true if the pointer is using a floating-point format. */
        static bool isFloatingPoint() noexcept                  { return (bool) SampleFormat::isFloat; }

        /** Returns true if the format is big-endian. */
        static bool isBigEndian() noexcept                      { return (bool) Endianness::isBigEndian; }

        /** Returns the number of bytes in each sample (ignoring the number of interleaved channels). */
        static int getBytesPerSample() noexcept                 { return (int) SampleFormat::bytesPerSample; }

        /** Returns the number of interleaved channels in the format. */
        int getNumInterleavedChannels() const noexcept          { return (int) this->numInterleavedChannels; }

        /** Returns the number of bytes between the start address of each sample. */
        int getNumBytesBetweenSamples() const noexcept          { return InterleavingType::getNumBytesBetweenSamples (data); }

        /** Returns the accuracy of this format when represented as a 32-bit integer.
            This is the smallest number above 0 that can be represented in the sample format, converted to
            a 32-bit range. E,g. if the format is 8-bit, its resolution is 0x01000000; if the format is 24-bit,
            its resolution is 0x100.
        */
        static int get32BitResolution() noexcept                { return (int) SampleFormat::resolution; }

        /** Returns a pointer to the underlying data. */
        const void* getRawData() const noexcept                 { return data.data; }

    private:
        //==============================================================================
        SampleFormat data;

        inline void advance() noexcept                          { this->advanceData (data); }

        Pointer operator++ (int); // private to force you to use the more efficient pre-increment!
        Pointer operator-- (int);
    };

    //==============================================================================
    /** A base class for objects that are used to convert between two different sample formats.

        The AudioData::ConverterInstance implements this base class and can be templated, so
        you can create an instance that converts between two particular formats, and then
        store this in the abstract base class.

        @see AudioData::ConverterInstance
    */
    class Converter
    {
    public:
        virtual ~Converter() = default;

        /** Converts a sequence of samples from the converter's source format into the dest format. */
        virtual void convertSamples (void* destSamples, const void* sourceSamples, int numSamples) const = 0;

        /** Converts a sequence of samples from the converter's source format into the dest format.
            This method takes sub-channel indexes, which can be used with interleaved formats in order to choose a
            particular sub-channel of the data to be used.
        */
        virtual void convertSamples (void* destSamples, int destSubChannel,
                                     const void* sourceSamples, int sourceSubChannel, int numSamples) const = 0;
    };

    //==============================================================================
    /**
        A class that converts between two templated AudioData::Pointer types, and which
        implements the AudioData::Converter interface.

        This can be used as a concrete instance of the AudioData::Converter abstract class.

        @see AudioData::Converter
    */
    template <class SourceSampleType, class DestSampleType>
    class ConverterInstance  : public Converter
    {
    public:
        ConverterInstance (int numSourceChannels = 1, int numDestChannels = 1)
            : sourceChannels (numSourceChannels), destChannels (numDestChannels)
        {}

        void convertSamples (void* dest, const void* source, int numSamples) const override
        {
            SourceSampleType s (source, sourceChannels);
            DestSampleType d (dest, destChannels);
            d.convertSamples (s, numSamples);
        }

        void convertSamples (void* dest, int destSubChannel,
                             const void* source, int sourceSubChannel, int numSamples) const override
        {
            jassert (destSubChannel < destChannels && sourceSubChannel < sourceChannels);

            SourceSampleType s (addBytesToPointer (source, sourceSubChannel * SourceSampleType::getBytesPerSample()), sourceChannels);
            DestSampleType d (addBytesToPointer (dest, destSubChannel * DestSampleType::getBytesPerSample()), destChannels);
            d.convertSamples (s, numSamples);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (ConverterInstance)

        const int sourceChannels, destChannels;
    };

    //==============================================================================
    /** A struct that contains a SampleFormat and Endianness to be used with the source and
        destination types when calling the interleaveSamples() and deinterleaveSamples() helpers.

        @see interleaveSamples, deinterleaveSamples
    */
    template <typename DataFormatIn, typename EndiannessIn>
    struct Format
    {
        using DataFormat = DataFormatIn;
        using Endianness = EndiannessIn;
    };

private:
    template <bool IsInterleaved, bool IsConst, typename...>
    struct ChannelDataSubtypes;

    template <bool IsInterleaved, bool IsConst, typename DataFormat, typename Endianness>
    struct ChannelDataSubtypes<IsInterleaved, IsConst, DataFormat, Endianness>
    {
        using ElementType = std::remove_pointer_t<decltype (DataFormat::data)>;
        using ChannelType = std::conditional_t<IsConst, const ElementType*, ElementType*>;
        using DataType = std::conditional_t<IsInterleaved, ChannelType, ChannelType const*>;
        using PointerType = Pointer<DataFormat,
                                    Endianness,
                                    std::conditional_t<IsInterleaved, Interleaved, NonInterleaved>,
                                    std::conditional_t<IsConst, Const, NonConst>>;
    };

    template <bool IsInterleaved, bool IsConst, typename DataFormat, typename Endianness>
    struct ChannelDataSubtypes<IsInterleaved, IsConst, Format<DataFormat, Endianness>>
    {
        using Subtypes    = ChannelDataSubtypes<IsInterleaved, IsConst, DataFormat, Endianness>;
        using DataType    = typename Subtypes::DataType;
        using PointerType = typename Subtypes::PointerType;
    };

    template <bool IsInterleaved, bool IsConst, typename... Format>
    struct ChannelData
    {
        using Subtypes    = ChannelDataSubtypes<IsInterleaved, IsConst, Format...>;
        using DataType    = typename Subtypes::DataType;
        using PointerType = typename Subtypes::PointerType;

        DataType data;
        int channels;
    };

public:
    //==============================================================================
    /** A sequence of interleaved samples used as the source for the deinterleaveSamples() method. */
    template <typename... Format> using InterleavedSource     = ChannelData<true,  true,   Format...>;
    /** A sequence of interleaved samples used as the destination for the interleaveSamples() method. */
    template <typename... Format> using InterleavedDest       = ChannelData<true,  false,  Format...>;
    /** A sequence of non-interleaved samples used as the source for the interleaveSamples() method. */
    template <typename... Format> using NonInterleavedSource  = ChannelData<false, true,   Format...>;
    /** A sequence of non-interleaved samples used as the destination for the deinterleaveSamples() method. */
    template <typename... Format> using NonInterleavedDest    = ChannelData<false, false,  Format...>;

    /** A helper function for converting a sequence of samples from a non-interleaved source
        to an interleaved destination.

        When calling this method you need to specify the source and destination data format and endianness
        from the AudioData SampleFormat and Endianness types and provide the data and number of channels
        for each. For example, to convert a floating-point stream of big endian samples to an interleaved,
        native endian stream of 16-bit integer samples you would do the following:

        @code
        using SourceFormat = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;
        using DestFormat   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;

        AudioData::interleaveSamples (AudioData::NonInterleavedSource<SourceFormat> { sourceData, numSourceChannels },
                                      AudioData::InterleavedDest<DestFormat>        { destData,   numDestChannels },
                                      numSamples);
        @endcode
    */
    template <typename... SourceFormat, typename... DestFormat>
    static void interleaveSamples (NonInterleavedSource<SourceFormat...> source,
                                   InterleavedDest<DestFormat...> dest,
                                   int numSamples)
    {
        using SourceType = typename decltype (source)::PointerType;
        using DestType   = typename decltype (dest)  ::PointerType;

        for (int i = 0; i < dest.channels; ++i)
        {
            const DestType destType (addBytesToPointer (dest.data, i * DestType::getBytesPerSample()), dest.channels);

            if (i < source.channels)
            {
                if (*source.data != nullptr)
                {
                    destType.convertSamples (SourceType { *source.data }, numSamples);
                    ++source.data;
                }
            }
            else
            {
                destType.clearSamples (numSamples);
            }
        }
    }

    /** A helper function for converting a sequence of samples from an interleaved source
        to a non-interleaved destination.

        When calling this method you need to specify the source and destination data format and endianness
        from the AudioData SampleFormat and Endianness types and provide the data and number of channels
        for each. For example, to convert a floating-point stream of big endian samples to an non-interleaved,
        native endian stream of 16-bit integer samples you would do the following:

        @code
        using SourceFormat = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;
        using DestFormat   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;

        AudioData::deinterleaveSamples (AudioData::InterleavedSource<SourceFormat> { sourceData, numSourceChannels },
                                        AudioData::NonInterleavedDest<DestFormat>  { destData,   numDestChannels },
                                        numSamples);
        @endcode
    */
    template <typename... SourceFormat, typename... DestFormat>
    static void deinterleaveSamples (InterleavedSource<SourceFormat...> source,
                                     NonInterleavedDest<DestFormat...> dest,
                                     int numSamples)
    {
        using SourceType = typename decltype (source)::PointerType;
        using DestType   = typename decltype (dest)  ::PointerType;

        for (int i = 0; i < dest.channels; ++i)
        {
            if (auto* targetChan = dest.data[i])
            {
                const DestType destType (targetChan);

                if (i < source.channels)
                    destType.convertSamples (SourceType (addBytesToPointer (source.data, i * SourceType::getBytesPerSample()), source.channels), numSamples);
                else
                    destType.clearSamples (numSamples);
            }
        }
    }
};

//==============================================================================
/** @cond */
/**
    A set of routines to convert buffers of 32-bit floating point data to and from
    various integer formats.

    Note that these functions are deprecated - the AudioData class provides a much more
    flexible set of conversion classes now.

    @tags{Audio}
*/
class [[deprecated]] JUCE_API  AudioDataConverters
{
public:
    //==============================================================================
    static void convertFloatToInt16LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 2);
    static void convertFloatToInt16BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 2);

    static void convertFloatToInt24LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 3);
    static void convertFloatToInt24BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 3);

    static void convertFloatToInt32LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);
    static void convertFloatToInt32BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);

    static void convertFloatToFloat32LE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);
    static void convertFloatToFloat32BE (const float* source, void* dest, int numSamples, int destBytesPerSample = 4);

    //==============================================================================
    static void convertInt16LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 2);
    static void convertInt16BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 2);

    static void convertInt24LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 3);
    static void convertInt24BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 3);

    static void convertInt32LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);
    static void convertInt32BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);

    static void convertFloat32LEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);
    static void convertFloat32BEToFloat (const void* source, float* dest, int numSamples, int srcBytesPerSample = 4);

    //==============================================================================
    enum DataFormat
    {
        int16LE,
        int16BE,
        int24LE,
        int24BE,
        int32LE,
        int32BE,
        float32LE,
        float32BE,
    };

    static void convertFloatToFormat (DataFormat destFormat,
                                      const float* source, void* dest, int numSamples);

    static void convertFormatToFloat (DataFormat sourceFormat,
                                      const void* source, float* dest, int numSamples);

    //==============================================================================
    static void interleaveSamples (const float** source, float* dest,
                                   int numSamples, int numChannels);

    static void deinterleaveSamples (const float* source, float** dest,
                                     int numSamples, int numChannels);

private:
    AudioDataConverters();
};
/** @endcond */

} // namespace juce
