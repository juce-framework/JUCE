/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_WavAudioFormat.h"
#include "../../../juce_core/io/streams/juce_BufferedInputStream.h"
#include "../../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
#define formatName                          TRANS("WAV file")
static const tchar* const extensions[] =    { T(".wav"), T(".bwf"), 0 };


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
#elif defined (JUCE_GCC)
  #define PACKED __attribute__((packed))
#else
  #define PACKED
#endif

struct BWAVChunk
{
    char description [256];
    char originator [32];
    char originatorRef [32];
    char originationDate [10];
    char originationTime [8];
    uint32 timeRefLow;
    uint32 timeRefHigh;
    uint16 version;
    uint8 umid[64];
    uint8 reserved[190];
    char codingHistory[1];

    void copyTo (StringPairArray& values) const
    {
        values.set (WavAudioFormat::bwavDescription, String (description, 256));
        values.set (WavAudioFormat::bwavOriginator, String (originator, 32));
        values.set (WavAudioFormat::bwavOriginatorRef, String (originatorRef, 32));
        values.set (WavAudioFormat::bwavOriginationDate, String (originationDate, 10));
        values.set (WavAudioFormat::bwavOriginationTime, String (originationTime, 8));

        const uint32 timeLow = swapIfBigEndian (timeRefLow);
        const uint32 timeHigh = swapIfBigEndian (timeRefHigh);
        const int64 time = (((int64)timeHigh) << 32) + timeLow;

        values.set (WavAudioFormat::bwavTimeReference, String (time));
        values.set (WavAudioFormat::bwavCodingHistory, String (codingHistory));
    }

    static MemoryBlock createFrom (const StringPairArray& values)
    {
        const int sizeNeeded = sizeof (BWAVChunk) + values [WavAudioFormat::bwavCodingHistory].length();
        MemoryBlock data ((sizeNeeded + 3) & ~3);
        data.fillWith (0);

        BWAVChunk* b = (BWAVChunk*) data.getData();

        // although copyToBuffer may overrun by one byte, that's ok as long as these
        // operations get done in the right order
        values [WavAudioFormat::bwavDescription].copyToBuffer (b->description, 256);
        values [WavAudioFormat::bwavOriginator].copyToBuffer (b->originator, 32);
        values [WavAudioFormat::bwavOriginatorRef].copyToBuffer (b->originatorRef, 32);
        values [WavAudioFormat::bwavOriginationDate].copyToBuffer (b->originationDate, 10);
        values [WavAudioFormat::bwavOriginationTime].copyToBuffer (b->originationTime, 8);

        const int64 time = values [WavAudioFormat::bwavTimeReference].getLargeIntValue();
        b->timeRefLow = swapIfBigEndian ((uint32) (time & 0xffffffff));
        b->timeRefHigh = swapIfBigEndian ((uint32) (time >> 32));

        values [WavAudioFormat::bwavCodingHistory].copyToBuffer (b->codingHistory, 256 * 1024);

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

#if JUCE_MSVC
  #pragma pack (pop)
#endif

#undef PACKED

#define chunkName(a) ((int) littleEndianInt(a))

//==============================================================================
class WavAudioFormatReader  : public AudioFormatReader
{
    int bytesPerFrame;
    int64 dataChunkStart, dataLength;

    WavAudioFormatReader (const WavAudioFormatReader&);
    const WavAudioFormatReader& operator= (const WavAudioFormatReader&);

public:
    //==============================================================================
    WavAudioFormatReader (InputStream* const in)
        : AudioFormatReader (in, formatName),
          dataLength (0)
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
                        // Broadcast-wav extension chunk..
                        BWAVChunk* const bwav = (BWAVChunk*) juce_calloc (jmax (length + 1, (int) sizeof (BWAVChunk)));

                        if (bwav != 0)
                        {
                            input->read (bwav, length);
                            bwav->copyTo (metadataValues);
                            juce_free (bwav);
                        }
                    }
                    else if ((hasGotType && hasGotData) || chunkEnd <= input->getPosition())
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
    bool read (int** destSamples,
               int64 startSampleInFile,
               int numSamples)
    {
        int64 start = startSampleInFile;
        int startOffsetInDestBuffer = 0;

        if (startSampleInFile < 0)
        {
            const int silence = (int) jmin (-startSampleInFile, (int64) numSamples);

            int** destChan = destSamples;

            for (int i = 2; --i >= 0;)
            {
                if (*destChan != 0)
                {
                    zeromem (*destChan, sizeof (int) * silence);
                    ++destChan;
                }
            }

            startOffsetInDestBuffer += silence;
            numSamples -= silence;
            start = 0;
        }

        const int numToDo = (int) jlimit ((int64) 0, (int64) numSamples, lengthInSamples - start);

        if (numToDo > 0)
        {
            input->setPosition (dataChunkStart + start * bytesPerFrame);

            int num = numToDo;
            int* left = destSamples[0];
            if (left != 0)
                 left += startOffsetInDestBuffer;

            int* right = destSamples[1];
            if (right != 0)
                right += startOffsetInDestBuffer;

            // (keep this a multiple of 3)
            const int tempBufSize = 1440 * 4;
            char tempBuffer [tempBufSize];

            while (num > 0)
            {
                const int numThisTime = jmin (tempBufSize / bytesPerFrame, num);
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
                                *right++ = (int) swapIfBigEndian ((unsigned short) *src++) << 16;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *left++ = (int) swapIfBigEndian ((unsigned short) *src++) << 16;
                                ++src;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *left++ = (int) swapIfBigEndian ((unsigned short) *src++) << 16;
                                *right++ = (int) swapIfBigEndian ((unsigned short) *src++) << 16;
                            }
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = (int) swapIfBigEndian ((unsigned short) *src++) << 16;
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
                                src += 6;
                                *right++ = littleEndian24Bit (src) << 8;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *left++ = littleEndian24Bit (src) << 8;
                                src += 6;
                            }
                        }
                        else
                        {
                            for (int i = 0; i < numThisTime; ++i)
                            {
                                *left++ = littleEndian24Bit (src) << 8;
                                src += 3;
                                *right++ = littleEndian24Bit (src) << 8;
                                src += 3;
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < numThisTime; ++i)
                        {
                            *left++ = littleEndian24Bit (src) << 8;
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
                                *r++ = swapIfBigEndian (*src++);
                            }
                        }
                        else if (r == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *l++ = swapIfBigEndian (*src++);
                                ++src;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *l++ = swapIfBigEndian (*src++);
                                *r++ = swapIfBigEndian (*src++);
                            }
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *l++ = swapIfBigEndian (*src++);
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

                num -= numThisTime;
            }
        }

        if (numToDo < numSamples)
        {
            int** destChan = destSamples;

            while (*destChan != 0)
            {
                zeromem ((*destChan) + (startOffsetInDestBuffer + numToDo),
                          sizeof (int) * (numSamples - numToDo));
                ++destChan;
            }
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
        output->writeInt (lengthInSamples * bytesPerFrame
                            + ((bwavChunk.getSize() > 0) ? (44 + bwavChunk.getSize()) : 36));

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
            output->writeInt (bwavChunk.getSize());
            output->write (bwavChunk.getData(), bwavChunk.getSize());
        }

        output->writeInt (chunkName ("data"));
        output->writeInt (lengthInSamples * bytesPerFrame);

        usesFloatingPointData = (bitsPerSample == 32);
    }

public:
    //==============================================================================
    WavAudioFormatWriter (OutputStream* const out,
                          const double sampleRate,
                          const unsigned int numChannels_,
                          const int bits,
                          const StringPairArray& metadataValues)
        : AudioFormatWriter (out,
                             formatName,
                             sampleRate,
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
                    *b++ = (short) swapIfBigEndian ((unsigned short) (*left++ >> 16));
                    *b++ = (short) swapIfBigEndian ((unsigned short) (*right++ >> 16));
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (short) swapIfBigEndian ((unsigned short) (*left++ >> 16));
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
                    littleEndian24BitToChars ((*left++) >> 8, b);
                    b += 3;
                    littleEndian24BitToChars ((*right++) >> 8, b);
                    b += 3;
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    littleEndian24BitToChars ((*left++) >> 8, b);
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
                    *b++ = swapIfBigEndian ((unsigned int) *left++);
                    *b++ = swapIfBigEndian ((unsigned int) *right++);
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = swapIfBigEndian ((unsigned int) *left++);
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
    : AudioFormat (formatName, (const tchar**) extensions)
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
    WavAudioFormatReader* r = new WavAudioFormatReader (sourceStream);

    if (r->sampleRate == 0)
    {
        if (! deleteStreamIfOpeningFails)
            r->input = 0;

        deleteAndZero (r);
    }

    return r;
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

END_JUCE_NAMESPACE
