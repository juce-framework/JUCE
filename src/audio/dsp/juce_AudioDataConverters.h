/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
#define __JUCE_AUDIODATACONVERTERS_JUCEHEADER__

//==============================================================================
/**

*/
/*
struct JUCE_API  AudioData
{
xxx need to add int range limiting..
    //==============================================================================
    struct BigEndian
    {
        template <class SampleFormatType> static inline float getAsFloat (SampleFormatType& s) throw()                          { return s.getAsFloatBE(); }
        template <class SampleFormatType> static inline void setAsFloat (SampleFormatType& s, float newValue) throw()           { s.setAsFloatBE (newValue); }
        template <class SampleFormatType> static inline int32 getAsInt32 (SampleFormatType& s) throw()                          { return s.getAsInt32BE(); }
        template <class SampleFormatType> static inline void setAsInt32 (SampleFormatType& s, int32 newValue) throw()           { s.setAsInt32BE (newValue); }
        template <class SourceType, class DestType> static inline void copyFrom (DestType& dest, SourceType& source) throw()    { dest.copyFromBE (source); }
    };

    struct LittleEndian
    {
        template <class SampleFormatType> static inline float getAsFloat (SampleFormatType& s) throw()                          { return s.getAsFloatLE(); }
        template <class SampleFormatType> static inline void setAsFloat (SampleFormatType& s, float newValue) throw()           { s.setAsFloatLE (newValue); }
        template <class SampleFormatType> static inline int32 getAsInt32 (SampleFormatType& s) throw()                          { return s.getAsInt32LE(); }
        template <class SampleFormatType> static inline void setAsInt32 (SampleFormatType& s, int32 newValue) throw()           { s.setAsInt32LE (newValue); }
        template <class SourceType, class DestType> static inline void copyFrom (DestType& dest, SourceType& source) throw()    { dest.copyFromLE (source); }
    };

    #if JUCE_BIG_ENDIAN
      typedef BigEndian     NativeEndian;
    #else
      typedef LittleEndian  NativeEndian;
    #endif

    //==============================================================================
    struct Int8
    {
        inline Int8 (void* data_)   : data (static_cast <int8*> (data_))  {}

        inline void advance() throw()                           { ++data; }
        inline void skip (int numSamples) throw()               { data += numSamples; }
        inline float getAsFloatLE() const throw()               { return (float) (*data * (1.0 / maxValue)); }
        inline float getAsFloatBE() const throw()               { return (float) (*data * (1.0 / maxValue)); }
        inline void setAsFloatLE (float newValue) throw()       { *data = (int8) roundToInt (newValue * (double) maxValue); }
        inline void setAsFloatBE (float newValue) throw()       { *data = (int8) roundToInt (newValue * (double) maxValue); }
        inline int32 getAsInt32LE() const throw()               { return (int) (*data << 24); }
        inline int32 getAsInt32BE() const throw()               { return (int) (*data << 24); }
        inline void setAsInt32LE (int newValue) throw()         { *data = (int8) (newValue >> 24); }
        inline void setAsInt32BE (int newValue) throw()         { *data = (int8) (newValue >> 24); }

        template <class SourceType> inline void copyFromLE (SourceType& source) throw()     { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) throw()     { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int8& source) throw()     { *data = *source.data; }

        int8* data;
        enum { maxValue = 0x80, resolution = 0x1000000, isFloat = 0 };
    };

    struct Int16
    {
        inline Int16 (void* data_)   : data (static_cast <uint16*> (data_))  {}

        inline void advance() throw()                           { ++data; }
        inline void skip (int numSamples) throw()               { data += numSamples; }
        inline float getAsFloatLE() const throw()               { return (float) ((1.0 / maxValue) * (int16) ByteOrder::swapIfBigEndian (*data)); }
        inline float getAsFloatBE() const throw()               { return (float) ((1.0 / maxValue) * (int16) ByteOrder::swapIfLittleEndian (*data)); }
        inline void setAsFloatLE (float newValue) throw()       { *data = ByteOrder::swapIfBigEndian ((uint16) roundToInt (newValue * (double) maxValue)); }
        inline void setAsFloatBE (float newValue) throw()       { *data = ByteOrder::swapIfLittleEndian ((uint16) roundToInt (newValue * (double) maxValue)); }
        inline int32 getAsInt32LE() const throw()               { return (int32) (ByteOrder::swapIfBigEndian ((uint16) *data) << 16); }
        inline int32 getAsInt32BE() const throw()               { return (int32) (ByteOrder::swapIfLittleEndian ((uint16) *data) << 16); }
        inline void setAsInt32LE (int32 newValue) throw()       { *data = ByteOrder::swapIfBigEndian ((uint16) (newValue >> 16)); }
        inline void setAsInt32BE (int32 newValue) throw()       { *data = ByteOrder::swapIfLittleEndian ((uint16) (newValue >> 16)); }

        template <class SourceType> inline void copyFromLE (SourceType& source) throw()     { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) throw()     { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int16& source) throw()     { *data = *source.data; }

        uint16* data;
        enum { maxValue = 0x8000, resolution = 0x10000, isFloat = 0 };
    };

    struct Int24
    {
        inline Int24 (void* data_)   : data (static_cast <char*> (data_))  {}

        inline void advance() throw()                           { data += 3; }
        inline void skip (int numSamples) throw()               { data += 3 * numSamples; }

        inline float getAsFloatLE() const throw()               { return (float) (ByteOrder::littleEndian24Bit (data) * (1.0 / maxValue)); }
        inline float getAsFloatBE() const throw()               { return (float) (ByteOrder::bigEndian24Bit (data) * (1.0 / maxValue)); }
        inline void setAsFloatLE (float newValue) throw()       { ByteOrder::littleEndian24BitToChars (roundToInt (newValue * (double) maxValue), data); }
        inline void setAsFloatBE (float newValue) throw()       { ByteOrder::bigEndian24BitToChars (roundToInt (newValue * (double) maxValue), data); }
        inline int32 getAsInt32LE() const throw()               { return (int32) ByteOrder::littleEndian24Bit (data) << 8; }
        inline int32 getAsInt32BE() const throw()               { return (int32) ByteOrder::bigEndian24Bit (data) << 8; }
        inline void setAsInt32LE (int32 newValue) throw()       { ByteOrder::littleEndian24BitToChars (newValue >> 8, data); }
        inline void setAsInt32BE (int32 newValue) throw()       { ByteOrder::bigEndian24BitToChars (newValue >> 8, data); }

        template <class SourceType> inline void copyFromLE (SourceType& source) throw()     { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) throw()     { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int24& source) throw()    { data[0] = source.data[0]; data[1] = source.data[1]; data[2] = source.data[2]; }

        char* data;
        enum { maxValue = 0x800000, resolution = 0x100, isFloat = 0 };
    };

    struct Int32
    {
        inline Int32 (void* data_)   : data (static_cast <uint32*> (data_))  {}

        inline void advance() throw()                           { ++data; }
        inline void skip (int numSamples) throw()               { data += numSamples; }
        inline float getAsFloatLE() const throw()               { return (float) ((1.0 / (1.0 + maxValue)) * (int32) ByteOrder::swapIfBigEndian (*data)); }
        inline float getAsFloatBE() const throw()               { return (float) ((1.0 / (1.0 + maxValue)) * (int32) ByteOrder::swapIfLittleEndian (*data)); }
        inline void setAsFloatLE (float newValue) throw()       { *data = ByteOrder::swapIfBigEndian ((uint32) roundToInt (newValue * (1.0 + maxValue))); }
        inline void setAsFloatBE (float newValue) throw()       { *data = ByteOrder::swapIfLittleEndian ((uint32) roundToInt (newValue * (1.0 + maxValue))); }
        inline int32 getAsInt32LE() const throw()               { return (int32) ByteOrder::swapIfBigEndian (*data); }
        inline int32 getAsInt32BE() const throw()               { return (int32) ByteOrder::swapIfLittleEndian (*data); }
        inline void setAsInt32LE (int32 newValue) throw()       { *data = ByteOrder::swapIfBigEndian ((uint32) newValue); }
        inline void setAsInt32BE (int32 newValue) throw()       { *data = ByteOrder::swapIfLittleEndian ((uint32) newValue); }

        template <class SourceType> inline void copyFromLE (SourceType& source) throw()     { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) throw()     { setAsInt32BE (source.getAsInt32()); }
        inline void copyFromSameType (Int32& source) throw()    { *data = *source.data; }

        uint32* data;
        enum { maxValue = 0x7fffffff, resolution = 1, isFloat = 0 };
    };

    struct Float32
    {
        inline Float32 (void* data_)   : data (static_cast <float*> (data_))  {}

        inline void advance() throw()                           { ++data; }
        inline void skip (int numSamples) throw()               { data += numSamples; }

       #if JUCE_BIG_ENDIAN
        inline float getAsFloatBE() const throw()               { return *data; }
        inline void setAsFloatBE (float newValue) throw()       { *data = newValue; }
        inline float getAsFloatLE() const throw()               { union { uint32 asInt; float asFloat; } n; n.asInt = ByteOrder::swap (*(uint32*) data); return n.asFloat; }
        inline void setAsFloatLE (float newValue) throw()       { union { uint32 asInt; float asFloat; } n; n.asFloat = newValue; *(uint32*) data = ByteOrder::swap (n.asInt); }
       #else
        inline float getAsFloatLE() const throw()               { return *data; }
        inline void setAsFloatLE (float newValue) throw()       { *data = newValue; }
        inline float getAsFloatBE() const throw()               { union { uint32 asInt; float asFloat; } n; n.asInt = ByteOrder::swap (*(uint32*) data); return n.asFloat; }
        inline void setAsFloatBE (float newValue) throw()       { union { uint32 asInt; float asFloat; } n; n.asFloat = newValue; *(uint32*) data = ByteOrder::swap (n.asInt); }
       #endif

        inline int32 getAsInt32LE() const throw()               { return (int32) roundToInt (getAsFloatLE() * (1.0 + maxValue)); }
        inline int32 getAsInt32BE() const throw()               { return (int32) roundToInt (getAsFloatBE() * (1.0 + maxValue)); }
        inline void setAsInt32LE (int32 newValue) throw()       { setAsFloatLE ((float) (newValue * (1.0 / (1.0 + maxValue)))); }
        inline void setAsInt32BE (int32 newValue) throw()       { setAsFloatBE ((float) (newValue * (1.0 / (1.0 + maxValue)))); }

        template <class SourceType> inline void copyFromLE (SourceType& source) throw()     { setAsFloatLE (source.getAsFloat()); }
        template <class SourceType> inline void copyFromBE (SourceType& source) throw()     { setAsFloatBE (source.getAsFloat()); }
        inline void copyFromSameType (Float32& source) throw()  { *data = *source.data; }

        float* data;
        enum { maxValue = 0x7fffffff, resolution = 0x100, isFloat = 1 };
    };

    //==============================================================================
    struct NonInterleaved
    {
        inline NonInterleaved () throw() {}
        inline NonInterleaved (const int numChannels) throw()
        { jassert (numChannels == 1); } // If you hit this assert, you're trying to create a non-interleaved pointer with more than one interleaved channel..

        template <class SampleFormatType> inline void advance (SampleFormatType& s) throw()     { s.advance(); }
    };

    struct Interleaved
    {
        inline Interleaved () throw() : numChannels (1) {}
        inline Interleaved (const int numChannels_) throw() : numChannels (numChannels_) {}
        template <class SampleFormatType> inline void advance (SampleFormatType& s) throw()     { s.skip (numChannels); }

        const int numChannels;
    };


    //==============================================================================
    template <class SampleFormat,
              class Endianness,
              class InterleavingType>
    class Pointer
    {
    public:
        Pointer (const void* sourceData)
            : data (const_cast <void*> (sourceData))
        {
        }

        Pointer (const void* sourceData, int numInterleavedChannels)
            : data (const_cast <void*> (sourceData)),
              interleaving (numInterleavedChannels)
        {
        }

        //==============================================================================
        inline float getAsFloat() const throw()                { return Endianness::getAsFloat (data); }
        inline void setAsFloat (float newValue) throw()        { Endianness::setAsFloat (data, newValue); }

        inline int32 getAsInt32() const throw()                { return Endianness::getAsInt32 (data); }
        inline void setAsInt32 (int32 newValue) throw()        { Endianness::setAsInt32 (data, newValue); }

        inline void advance() throw()                          { interleaving.advance (data); }

        void copySamples (Pointer& source, int numSamples)
        {
            while (--numSamples >= 0)
            {
                data.copyFromSameType (source.data);
                advance();
                source.advance();
            }
        }

        template <class OtherPointerType>
        void copySamples (OtherPointerType& source, int numSamples)
        {
            while (--numSamples >= 0)
            {
                Endianness::copyFrom (data, source);
                advance();
                source.advance();
            }
        }

        bool isFloatingPoint() const throw()                    { return (bool) SampleFormat::isFloat; }
        static int get32BitResolution() throw()                 { return (int) SampleFormat::resolution; }

        typedef Endianness EndiannessType;

    private:
        //==============================================================================
        SampleFormat data;
        InterleavingType interleaving;
    };

    //==============================================================================
    class Converter
    {
    public:
        virtual ~Converter() {}

        virtual void copySamples (void* dest, const void* source, int numSamples) const = 0;
    };

    //==============================================================================
    template <class SourceSampleType, class DestSampleType>
    class ConverterInstance  : public Converter
    {
    public:
        ConverterInstance() {}
        ~ConverterInstance() {}

        void copySamples (void* const dest, const void* const source, const int numSamples) const
        {
            SourceSampleType s (source);
            DestSampleType d (dest);
            d.copySamples (s, numSamples);
        }

    private:
        ConverterInstance (const ConverterInstance&);
        ConverterInstance& operator= (const ConverterInstance&);
    };
};
*/

//==============================================================================
/**
    A set of routines to convert buffers of 32-bit floating point data to and from
    various integer formats.

*/
class JUCE_API  AudioDataConverters
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
    AudioDataConverters (const AudioDataConverters&);
    AudioDataConverters& operator= (const AudioDataConverters&);
};


#endif   // __JUCE_AUDIODATACONVERTERS_JUCEHEADER__
