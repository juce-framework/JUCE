/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_WavAudioFormat.h"
#include "../../io/streams/juce_BufferedInputStream.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../io/files/juce_FileInputStream.h"
#include "../../io/files/juce_TemporaryFile.h"


//==============================================================================
static const char* const wavFormatName = "WAV file";
static const tchar* const wavExtensions[] =    { T(".wav"), T(".bwf"), 0 };


//==============================================================================
const tchar* const WavAudioFormat::bwavDescription      = T("bwav description");
const tchar* const WavAudioFormat::bwavOriginator       = T("bwav originator");
const tchar* const WavAudioFormat::bwavOriginatorRef    = T("bwav originator ref");
const tchar* const WavAudioFormat::bwavOriginationDate  = T("bwav origination date");
const tchar* const WavAudioFormat::bwavOriginationTime  = T("bwav origination time");
const tchar* const WavAudioFormat::bwavTimeReference    = T("bwav time reference");
const tchar* const WavAudioFormat::bwavCodingHistory    = T("bwav coding history");

const StringPairArray WavAudioFormat::createBWAVMetadata (const String& description,
                                                          const String& originator,
                                                          const String& originatorRef,
                                                          const Time& date,
                                                          const int64 timeReferenceSamples,
                                                          const String& codingHistory)
{
    StringPairArray m;

    m.set (bwavDescription, description);
    m.set (bwavOriginator, originator);
    m.set (bwavOriginatorRef, originatorRef);
    m.set (bwavOriginationDate, date.formatted (T("%Y-%m-%d")));
    m.set (bwavOriginationTime, date.formatted (T("%H:%M:%S")));
    m.set (bwavTimeReference, String (timeReferenceSamples));
    m.set (bwavCodingHistory, codingHistory);

    return m;
}


//==============================================================================
#if JUCE_MSVC
  #pragma pack (push, 1)
  #define PACKED
#elif JUCE_GCC
  #define PACKED __attribute__((packed))
#else
  #define PACKED
#endif

struct BWAVChunk
{
    uint8 description [256];
    uint8 originator [32];
    uint8 originatorRef [32];
    uint8 originationDate [10];
    uint8 originationTime [8];
    uint32 timeRefLow;
    uint32 timeRefHigh;
    uint16 version;
    uint8 umid[64];
    uint8 reserved[190];
    uint8 codingHistory[1];

    void copyTo (StringPairArray& values) const
    {
        values.set (WavAudioFormat::bwavDescription, String::fromUTF8 (description, 256));
        values.set (WavAudioFormat::bwavOriginator, String::fromUTF8 (originator, 32));
        values.set (WavAudioFormat::bwavOriginatorRef, String::fromUTF8 (originatorRef, 32));
        values.set (WavAudioFormat::bwavOriginationDate, String::fromUTF8 (originationDate, 10));
        values.set (WavAudioFormat::bwavOriginationTime, String::fromUTF8 (originationTime, 8));

        const uint32 timeLow = ByteOrder::swapIfBigEndian (timeRefLow);
        const uint32 timeHigh = ByteOrder::swapIfBigEndian (timeRefHigh);
        const int64 time = (((int64)timeHigh) << 32) + timeLow;

        values.set (WavAudioFormat::bwavTimeReference, String (time));
        values.set (WavAudioFormat::bwavCodingHistory, String::fromUTF8 (codingHistory));
    }

    static MemoryBlock createFrom (const StringPairArray& values)
    {
        const size_t sizeNeeded = sizeof (BWAVChunk) + values [WavAudioFormat::bwavCodingHistory].copyToUTF8 (0) - 1;
        MemoryBlock data ((sizeNeeded + 3) & ~3);
        data.fillWith (0);

        BWAVChunk* b = (BWAVChunk*) data.getData();

        // Allow these calls to overwrite an extra byte at the end, which is fine as long
        // as they get called in the right order..
        values [WavAudioFormat::bwavDescription].copyToUTF8 (b->description, 257);
        values [WavAudioFormat::bwavOriginator].copyToUTF8 (b->originator, 33);
        values [WavAudioFormat::bwavOriginatorRef].copyToUTF8 (b->originatorRef, 33);
        values [WavAudioFormat::bwavOriginationDate].copyToUTF8 (b->originationDate, 11);
        values [WavAudioFormat::bwavOriginationTime].copyToUTF8 (b->originationTime, 9);

        const int64 time = values [WavAudioFormat::bwavTimeReference].getLargeIntValue();
        b->timeRefLow = ByteOrder::swapIfBigEndian ((uint32) (time & 0xffffffff));
        b->timeRefHigh = ByteOrder::swapIfBigEndian ((uint32) (time >> 32));

        values [WavAudioFormat::bwavCodingHistory].copyToUTF8 (b->codingHistory);

        if (b->description[0] != 0
            || b->originator[0] != 0
            || b->originationDate[0] != 0
            || b->originationTime[0] != 0
            || b->codingHistory[0] != 0
            || time != 0)
        {
            return data;
        }

        return MemoryBlock();
    }

} PACKED;


//==============================================================================
struct SMPLChunk
{
    struct SampleLoop
    {
        uint32 identifier;
        uint32 type;
        uint32 start;
        uint32 end;
        uint32 fraction;
        uint32 playCount;
    } PACKED;

    uint32 manufacturer;
    uint32 product;
    uint32 samplePeriod;
    uint32 midiUnityNote;
    uint32 midiPitchFraction;
    uint32 smpteFormat;
    uint32 smpteOffset;
    uint32 numSampleLoops;
    uint32 samplerData;
    SampleLoop loops[1];

    void copyTo (StringPairArray& values, const int totalSize) const
    {
        values.set (T("Manufacturer"),      String (ByteOrder::swapIfBigEndian (manufacturer)));
        values.set (T("Product"),           String (ByteOrder::swapIfBigEndian (product)));
        values.set (T("SamplePeriod"),      String (ByteOrder::swapIfBigEndian (samplePeriod)));
        values.set (T("MidiUnityNote"),     String (ByteOrder::swapIfBigEndian (midiUnityNote)));
        values.set (T("MidiPitchFraction"), String (ByteOrder::swapIfBigEndian (midiPitchFraction)));
        values.set (T("SmpteFormat"),       String (ByteOrder::swapIfBigEndian (smpteFormat)));
        values.set (T("SmpteOffset"),       String (ByteOrder::swapIfBigEndian (smpteOffset)));
        values.set (T("NumSampleLoops"),    String (ByteOrder::swapIfBigEndian (numSampleLoops)));
        values.set (T("SamplerData"),       String (ByteOrder::swapIfBigEndian (samplerData)));

        for (uint32 i = 0; i < numSampleLoops; ++i)
        {
            if ((uint8*) (loops + (i + 1)) > ((uint8*) this) + totalSize)
                break;

            values.set (String::formatted (T("Loop%dIdentifier"), i),   String (ByteOrder::swapIfBigEndian (loops[i].identifier)));
            values.set (String::formatted (T("Loop%dType"), i),         String (ByteOrder::swapIfBigEndian (loops[i].type)));
            values.set (String::formatted (T("Loop%dStart"), i),        String (ByteOrder::swapIfBigEndian (loops[i].start)));
            values.set (String::formatted (T("Loop%dEnd"), i),          String (ByteOrder::swapIfBigEndian (loops[i].end)));
            values.set (String::formatted (T("Loop%dFraction"), i),     String (ByteOrder::swapIfBigEndian (loops[i].fraction)));
            values.set (String::formatted (T("Loop%dPlayCount"), i),    String (ByteOrder::swapIfBigEndian (loops[i].playCount)));
        }
    }
} PACKED;

#if JUCE_MSVC
  #pragma pack (pop)
#endif

#undef PACKED


//==============================================================================
class WavAudioFormatReader  : public AudioFormatReader
{
    int bytesPerFrame;
    int64 dataChunkStart, dataLength;

    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }

    WavAudioFormatReader (const WavAudioFormatReader&);
    const WavAudioFormatReader& operator= (const WavAudioFormatReader&);

public:
    int64 bwavChunkStart, bwavSize;

    //==============================================================================
    WavAudioFormatReader (InputStream* const in)
        : AudioFormatReader (in, TRANS (wavFormatName)),
          dataLength (0),
          bwavChunkStart (0),
          bwavSize (0)
    {
        if (input->readInt() == chunkName ("RIFF"))
        {
            const uint32 len = (uint32) input->readInt();
            const int64 end = input->getPosition() + len;
            bool hasGotType = false;
            bool hasGotData = false;

            if (input->readInt() == chunkName ("WAVE"))
            {
                while (input->getPosition() < end
                        && ! input->isExhausted())
                {
                    const int chunkType = input->readInt();
                    uint32 length = (uint32) input->readInt();
                    const int64 chunkEnd = input->getPosition() + length + (length & 1);

                    if (chunkType == chunkName ("fmt "))
                    {
                        // read the format chunk
                        const short format = input->readShort();
                        const short numChans = input->readShort();
                        sampleRate = input->readInt();
                        const int bytesPerSec = input->readInt();

                        numChannels = numChans;
                        bytesPerFrame = bytesPerSec / (int)sampleRate;
                        bitsPerSample = 8 * bytesPerFrame / numChans;

                        if (format == 3)
                            usesFloatingPointData = true;
                        else if (format != 1)
                            bytesPerFrame = 0;

                        hasGotType = true;
                    }
                    else if (chunkType == chunkName ("data"))
                    {
                        // get the data chunk's position
                        dataLength = length;
                        dataChunkStart = input->getPosition();
                        lengthInSamples = (bytesPerFrame > 0) ? (dataLength / bytesPerFrame) : 0;

                        hasGotData = true;
                    }
                    else if (chunkType == chunkName ("bext"))
                    {
                        bwavChunkStart = input->getPosition();
                        bwavSize = length;

                        // Broadcast-wav extension chunk..
                        HeapBlock <BWAVChunk> bwav;
                        bwav.calloc (jmax ((size_t) length + 1, sizeof (BWAVChunk)), 1);
                        input->read (bwav, length);
                        bwav->copyTo (metadataValues);
                    }
                    else if (chunkType == chunkName ("smpl"))
                    {
                        HeapBlock <SMPLChunk> smpl;
                        smpl.calloc (jmax ((size_t) length + 1, sizeof (SMPLChunk)), 1);
                        input->read (smpl, length);
                        smpl->copyTo (metadataValues, length);
                    }
                    else if (chunkEnd <= input->getPosition())
                    {
                        break;
                    }

                    input->setPosition (chunkEnd);
                }
            }
        }
    }

    ~WavAudioFormatReader()
    {
    }

    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        const int64 samplesAvailable = lengthInSamples - startSampleInFile;

        if (samplesAvailable < numSamples)
        {
            for (int i = numDestChannels; --i >= 0;)
                if (destSamples[i] != 0)
                    zeromem (destSamples[i] + startOffsetInDestBuffer, sizeof (int) * numSamples);

            numSamples = (int) samplesAvailable;
        }

        if (numSamples <= 0)
            return true;

        input->setPosition (dataChunkStart + startSampleInFile * bytesPerFrame);

        const int tempBufSize = 480 * 3 * 4; // (keep this a multiple of 3)
        char tempBuffer [tempBufSize];

        while (numSamples > 0)
        {
            int* left = destSamples[0];
            if (left != 0)
                left += startOffsetInDestBuffer;

            int* right = numDestChannels > 1 ? destSamples[1] : 0;
            if (right != 0)
                right += startOffsetInDestBuffer;

            const int numThisTime = jmin (tempBufSize / bytesPerFrame, numSamples);
            const int bytesRead = input->read (tempBuffer, numThisTime * bytesPerFrame);

            if (bytesRead < numThisTime * bytesPerFrame)
                zeromem (tempBuffer + bytesRead, numThisTime * bytesPerFrame - bytesRead);

            if (bitsPerSample == 16)
            {
                const short* src = (const short*) tempBuffer;

                if (numChannels > 1)
                {
                    if (left == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            ++src;
                            *right++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                        }
                    }
                    else if (right == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                            ++src;
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                            *right++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                        }
                    }
                }
                else
                {
                    for (int i = numThisTime; --i >= 0;)
                    {
                        *left++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                    }
                }
            }
            else if (bitsPerSample == 24)
            {
                const char* src = (const char*) tempBuffer;

                if (numChannels > 1)
                {
                    if (left == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            src += 3;
                            *right++ = ByteOrder::littleEndian24Bit (src) << 8;
                            src += 3;
                        }
                    }
                    else if (right == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ByteOrder::littleEndian24Bit (src) << 8;
                            src += 6;
                        }
                    }
                    else
                    {
                        for (int i = 0; i < numThisTime; ++i)
                        {
                            *left++ = ByteOrder::littleEndian24Bit (src) << 8;
                            src += 3;
                            *right++ = ByteOrder::littleEndian24Bit (src) << 8;
                            src += 3;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < numThisTime; ++i)
                    {
                        *left++ = ByteOrder::littleEndian24Bit (src) << 8;
                        src += 3;
                    }
                }
            }
            else if (bitsPerSample == 32)
            {
                const unsigned int* src = (const unsigned int*) tempBuffer;
                unsigned int* l = (unsigned int*) left;
                unsigned int* r = (unsigned int*) right;

                if (numChannels > 1)
                {
                    if (l == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            ++src;
                            *r++ = ByteOrder::swapIfBigEndian (*src++);
                        }
                    }
                    else if (r == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *l++ = ByteOrder::swapIfBigEndian (*src++);
                            ++src;
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *l++ = ByteOrder::swapIfBigEndian (*src++);
                            *r++ = ByteOrder::swapIfBigEndian (*src++);
                        }
                    }
                }
                else
                {
                    for (int i = numThisTime; --i >= 0;)
                    {
                        *l++ = ByteOrder::swapIfBigEndian (*src++);
                    }
                }

                left = (int*)l;
                right = (int*)r;
            }
            else if (bitsPerSample == 8)
            {
                const unsigned char* src = (const unsigned char*) tempBuffer;

                if (numChannels > 1)
                {
                    if (left == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            ++src;
                            *right++ = ((int) *src++ - 128) << 24;
                        }
                    }
                    else if (right == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ((int) *src++ - 128) << 24;
                            ++src;
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ((int) *src++ - 128) << 24;
                            *right++ = ((int) *src++ - 128) << 24;
                        }
                    }
                }
                else
                {
                    for (int i = numThisTime; --i >= 0;)
                    {
                        *left++ = ((int)*src++ - 128) << 24;
                    }
                }
            }

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
        }

        if (numSamples > 0)
        {
            for (int i = numDestChannels; --i >= 0;)
                if (destSamples[i] != 0)
                    zeromem (destSamples[i] + startOffsetInDestBuffer,
                             sizeof (int) * numSamples);
        }

        return true;
    }

    juce_UseDebuggingNewOperator
};

//==============================================================================
class WavAudioFormatWriter  : public AudioFormatWriter
{
    MemoryBlock tempBlock, bwavChunk;
    uint32 lengthInSamples, bytesWritten;
    int64 headerPosition;
    bool writeFailed;

    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }

    WavAudioFormatWriter (const WavAudioFormatWriter&);
    const WavAudioFormatWriter& operator= (const WavAudioFormatWriter&);

    void writeHeader()
    {
        const bool seekedOk = output->setPosition (headerPosition);
        (void) seekedOk;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassert (seekedOk);

        const int bytesPerFrame = numChannels * bitsPerSample / 8;
        output->writeInt (chunkName ("RIFF"));
        output->writeInt ((int) (lengthInSamples * bytesPerFrame
                                   + ((bwavChunk.getSize() > 0) ? (44 + bwavChunk.getSize()) : 36)));

        output->writeInt (chunkName ("WAVE"));
        output->writeInt (chunkName ("fmt "));
        output->writeInt (16);
        output->writeShort ((bitsPerSample < 32) ? (short) 1 /*WAVE_FORMAT_PCM*/
                                                 : (short) 3 /*WAVE_FORMAT_IEEE_FLOAT*/);
        output->writeShort ((short) numChannels);
        output->writeInt ((int) sampleRate);
        output->writeInt (bytesPerFrame * (int) sampleRate);
        output->writeShort ((short) bytesPerFrame);
        output->writeShort ((short) bitsPerSample);

        if (bwavChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("bext"));
            output->writeInt ((int) bwavChunk.getSize());
            output->write (bwavChunk.getData(), (int) bwavChunk.getSize());
        }

        output->writeInt (chunkName ("data"));
        output->writeInt (lengthInSamples * bytesPerFrame);

        usesFloatingPointData = (bitsPerSample == 32);
    }

public:
    //==============================================================================
    WavAudioFormatWriter (OutputStream* const out,
                          const double sampleRate_,
                          const unsigned int numChannels_,
                          const int bits,
                          const StringPairArray& metadataValues)
        : AudioFormatWriter (out,
                             TRANS (wavFormatName),
                             sampleRate_,
                             numChannels_,
                             bits),
          lengthInSamples (0),
          bytesWritten (0),
          writeFailed (false)
    {
        if (metadataValues.size() > 0)
            bwavChunk = BWAVChunk::createFrom (metadataValues);

        headerPosition = out->getPosition();
        writeHeader();
    }

    ~WavAudioFormatWriter()
    {
        writeHeader();
    }

    //==============================================================================
    bool write (const int** data, int numSamples)
    {
        if (writeFailed)
            return false;

        const int bytes = numChannels * numSamples * bitsPerSample / 8;
        tempBlock.ensureSize (bytes, false);
        char* buffer = (char*) tempBlock.getData();

        const int* left = data[0];
        const int* right = data[1];
        if (right == 0)
            right = left;

        if (bitsPerSample == 16)
        {
            short* b = (short*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (short) ByteOrder::swapIfBigEndian ((unsigned short) (*left++ >> 16));
                    *b++ = (short) ByteOrder::swapIfBigEndian ((unsigned short) (*right++ >> 16));
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (short) ByteOrder::swapIfBigEndian ((unsigned short) (*left++ >> 16));
                }
            }
        }
        else if (bitsPerSample == 24)
        {
            char* b = (char*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    ByteOrder::littleEndian24BitToChars ((*left++) >> 8, b);
                    b += 3;
                    ByteOrder::littleEndian24BitToChars ((*right++) >> 8, b);
                    b += 3;
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    ByteOrder::littleEndian24BitToChars ((*left++) >> 8, b);
                    b += 3;
                }
            }
        }
        else if (bitsPerSample == 32)
        {
            unsigned int* b = (unsigned int*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = ByteOrder::swapIfBigEndian ((unsigned int) *left++);
                    *b++ = ByteOrder::swapIfBigEndian ((unsigned int) *right++);
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = ByteOrder::swapIfBigEndian ((unsigned int) *left++);
                }
            }
        }
        else if (bitsPerSample == 8)
        {
            unsigned char* b = (unsigned char*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (unsigned char) (128 + (*left++ >> 24));
                    *b++ = (unsigned char) (128 + (*right++ >> 24));
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (unsigned char) (128 + (*left++ >> 24));
                }
            }
        }

        if (bytesWritten + bytes >= (uint32) 0xfff00000
             || ! output->write (buffer, bytes))
        {
            // failed to write to disk, so let's try writing the header.
            // If it's just run out of disk space, then if it does manage
            // to write the header, we'll still have a useable file..
            writeHeader();
            writeFailed = true;
            return false;
        }
        else
        {
            bytesWritten += bytes;
            lengthInSamples += numSamples;

            return true;
        }
    }

    juce_UseDebuggingNewOperator
};

//==============================================================================
WavAudioFormat::WavAudioFormat()
    : AudioFormat (TRANS (wavFormatName), (const tchar**) wavExtensions)
{
}

WavAudioFormat::~WavAudioFormat()
{
}

const Array <int> WavAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 0 };
    return Array <int> (rates);
}

const Array <int> WavAudioFormat::getPossibleBitDepths()
{
    const int depths[] = { 8, 16, 24, 32, 0 };
    return Array <int> (depths);
}

bool WavAudioFormat::canDoStereo()
{
    return true;
}

bool WavAudioFormat::canDoMono()
{
    return true;
}

AudioFormatReader* WavAudioFormat::createReaderFor (InputStream* sourceStream,
                                                    const bool deleteStreamIfOpeningFails)
{
    ScopedPointer <WavAudioFormatReader> r (new WavAudioFormatReader (sourceStream));

    if (r->sampleRate != 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = 0;

    return 0;
}

AudioFormatWriter* WavAudioFormat::createWriterFor (OutputStream* out,
                                                    double sampleRate,
                                                    unsigned int numChannels,
                                                    int bitsPerSample,
                                                    const StringPairArray& metadataValues,
                                                    int /*qualityOptionIndex*/)
{
    if (getPossibleBitDepths().contains (bitsPerSample))
    {
        return new WavAudioFormatWriter (out,
                                         sampleRate,
                                         numChannels,
                                         bitsPerSample,
                                         metadataValues);
    }

    return 0;
}

static bool juce_slowCopyOfWavFileWithNewMetadata (const File& file, const StringPairArray& metadata)
{
    TemporaryFile tempFile (file);

    WavAudioFormat wav;
    ScopedPointer <AudioFormatReader> reader (wav.createReaderFor (file.createInputStream(), true));

    if (reader != 0)
    {
        ScopedPointer <OutputStream> outStream (tempFile.getFile().createOutputStream());

        if (outStream != 0)
        {
            ScopedPointer <AudioFormatWriter> writer (wav.createWriterFor (outStream, reader->sampleRate,
                                                                           reader->numChannels, reader->bitsPerSample,
                                                                           metadata, 0));

            if (writer != 0)
            {
                outStream.release();

                bool ok = writer->writeFromAudioReader (*reader, 0, -1);
                writer = 0;

                return ok && tempFile.overwriteTargetFileWithTemporary();
            }
        }
    }

    return false;
}

bool WavAudioFormat::replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata)
{
    ScopedPointer <WavAudioFormatReader> reader ((WavAudioFormatReader*) createReaderFor (wavFile.createInputStream(), true));

    if (reader != 0)
    {
        const int64 bwavPos = reader->bwavChunkStart;
        const int64 bwavSize = reader->bwavSize;
        reader = 0;

        if (bwavSize > 0)
        {
            MemoryBlock chunk = BWAVChunk::createFrom (newMetadata);

            if (chunk.getSize() <= (size_t) bwavSize)
            {
                // the new one will fit in the space available, so write it directly..
                const int64 oldSize = wavFile.getSize();

                {
                    ScopedPointer <FileOutputStream> out (wavFile.createOutputStream());
                    out->setPosition (bwavPos);
                    out->write (chunk.getData(), (int) chunk.getSize());
                    out->setPosition (oldSize);
                }

                jassert (wavFile.getSize() == oldSize);

                return true;
            }
        }
    }

    return juce_slowCopyOfWavFileWithNewMetadata (wavFile, newMetadata);
}


END_JUCE_NAMESPACE
