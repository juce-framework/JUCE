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

#include "juce_AiffAudioFormat.h"
#include "../../io/streams/juce_BufferedInputStream.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../text/juce_LocalisedStrings.h"

//==============================================================================
static const char* const aiffFormatName = "AIFF file";
static const tchar* const aiffExtensions[] =    { T(".aiff"), T(".aif"), 0 };


//==============================================================================
class AiffAudioFormatReader  : public AudioFormatReader
{
public:
    int bytesPerFrame;
    int64 dataChunkStart;
    bool littleEndian;

    //==============================================================================
    AiffAudioFormatReader (InputStream* in)
        : AudioFormatReader (in, TRANS (aiffFormatName))
    {
        if (input->readInt() == chunkName ("FORM"))
        {
            const int len = input->readIntBigEndian();
            const int64 end = input->getPosition() + len;

            const int nextType = input->readInt();
            if (nextType == chunkName ("AIFF") || nextType == chunkName ("AIFC"))
            {
                bool hasGotVer = false;
                bool hasGotData = false;
                bool hasGotType = false;

                while (input->getPosition() < end)
                {
                    const int type = input->readInt();
                    const uint32 length = (uint32) input->readIntBigEndian();
                    const int64 chunkEnd = input->getPosition() + length;

                    if (type == chunkName ("FVER"))
                    {
                        hasGotVer = true;

                        const int ver = input->readIntBigEndian();
                        if (ver != 0 && ver != (int)0xa2805140)
                            break;
                    }
                    else if (type == chunkName ("COMM"))
                    {
                        hasGotType = true;

                        numChannels = (unsigned int)input->readShortBigEndian();
                        lengthInSamples = input->readIntBigEndian();
                        bitsPerSample = input->readShortBigEndian();
                        bytesPerFrame = (numChannels * bitsPerSample) >> 3;

                        unsigned char sampleRateBytes[10];
                        input->read (sampleRateBytes, 10);
                        const int byte0 = sampleRateBytes[0];

                        if ((byte0 & 0x80) != 0
                             || byte0 <= 0x3F || byte0 > 0x40
                             || (byte0 == 0x40 && sampleRateBytes[1] > 0x1C))
                            break;

                        unsigned int sampRate = ByteOrder::bigEndianInt ((char*) sampleRateBytes + 2);
                        sampRate >>= (16414 - ByteOrder::bigEndianShort ((char*) sampleRateBytes));
                        sampleRate = (int) sampRate;

                        if (length <= 18)
                        {
                            // some types don't have a chunk large enough to include a compression
                            // type, so assume it's just big-endian pcm
                            littleEndian = false;
                        }
                        else
                        {
                            const int compType = input->readInt();

                            if (compType == chunkName ("NONE") || compType == chunkName ("twos"))
                            {
                                littleEndian = false;
                            }
                            else if (compType == chunkName ("sowt"))
                            {
                                littleEndian = true;
                            }
                            else
                            {
                                sampleRate = 0;
                                break;
                            }
                        }
                    }
                    else if (type == chunkName ("SSND"))
                    {
                        hasGotData = true;

                        const int offset = input->readIntBigEndian();
                        dataChunkStart = input->getPosition() + 4 + offset;
                        lengthInSamples = (bytesPerFrame > 0) ? jmin (lengthInSamples, (int64) (length / bytesPerFrame)) : 0;
                    }
                    else if ((hasGotVer && hasGotData && hasGotType)
                              || chunkEnd < input->getPosition()
                              || input->isExhausted())
                    {
                        break;
                    }

                    input->setPosition (chunkEnd);
                }
            }
        }
    }

    ~AiffAudioFormatReader()
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
                if (littleEndian)
                {
                    const short* src = (const short*) tempBuffer;

                    if (numChannels > 1)
                    {
                        if (left == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *right++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
                                ++src;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                ++src;
                                *left++ = (int) ByteOrder::swapIfBigEndian ((unsigned short) *src++) << 16;
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
                else
                {
                    const char* src = (const char*) tempBuffer;

                    if (numChannels > 1)
                    {
                        if (left == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *right++ = ByteOrder::bigEndianShort (src) << 16;
                                src += 4;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                src += 2;
                                *left++ = ByteOrder::bigEndianShort (src) << 16;
                                src += 2;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *left++ = ByteOrder::bigEndianShort (src) << 16;
                                src += 2;
                                *right++ = ByteOrder::bigEndianShort (src) << 16;
                                src += 2;
                            }
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ByteOrder::bigEndianShort (src) << 16;
                            src += 2;
                        }
                    }
                }
            }
            else if (bitsPerSample == 24)
            {
                const char* src = (const char*)tempBuffer;

                if (littleEndian)
                {
                    if (numChannels > 1)
                    {
                        if (left == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *right++ = ByteOrder::littleEndian24Bit (src) << 8;
                                src += 6;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                src += 3;
                                *left++ = ByteOrder::littleEndian24Bit (src) << 8;
                                src += 3;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
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
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ByteOrder::littleEndian24Bit (src) << 8;
                            src += 3;
                        }
                    }
                }
                else
                {
                    if (numChannels > 1)
                    {
                        if (left == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *right++ = ByteOrder::bigEndian24Bit (src) << 8;
                                src += 6;
                            }
                        }
                        else if (right == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                src += 3;
                                *left++ = ByteOrder::bigEndian24Bit (src) << 8;
                                src += 3;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *left++ = ByteOrder::bigEndian24Bit (src) << 8;
                                src += 3;
                                *right++ = ByteOrder::bigEndian24Bit (src) << 8;
                                src += 3;
                            }
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ByteOrder::bigEndian24Bit (src) << 8;
                            src += 3;
                        }
                    }
                }
            }
            else if (bitsPerSample == 32)
            {
                const unsigned int* src = (const unsigned int*) tempBuffer;
                unsigned int* l = (unsigned int*) left;
                unsigned int* r = (unsigned int*) right;

                if (littleEndian)
                {
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
                }
                else
                {
                    if (numChannels > 1)
                    {
                        if (l == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                ++src;
                                *r++ = ByteOrder::swapIfLittleEndian (*src++);
                            }
                        }
                        else if (r == 0)
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *l++ = ByteOrder::swapIfLittleEndian (*src++);
                                ++src;
                            }
                        }
                        else
                        {
                            for (int i = numThisTime; --i >= 0;)
                            {
                                *l++ = ByteOrder::swapIfLittleEndian (*src++);
                                *r++ = ByteOrder::swapIfLittleEndian (*src++);
                            }
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *l++ = ByteOrder::swapIfLittleEndian (*src++);
                        }
                    }
                }

                left = (int*) l;
                right = (int*) r;
            }
            else if (bitsPerSample == 8)
            {
                const char* src = (const char*) tempBuffer;

                if (numChannels > 1)
                {
                    if (left == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *right++ = ((int) *src++) << 24;
                            ++src;
                        }
                    }
                    else if (right == 0)
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            ++src;
                            *left++ = ((int) *src++) << 24;
                        }
                    }
                    else
                    {
                        for (int i = numThisTime; --i >= 0;)
                        {
                            *left++ = ((int) *src++) << 24;
                            *right++ = ((int) *src++) << 24;
                        }
                    }
                }
                else
                {
                    for (int i = numThisTime; --i >= 0;)
                    {
                        *left++ = ((int) *src++) << 24;
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

private:
    AiffAudioFormatReader (const AiffAudioFormatReader&);
    const AiffAudioFormatReader& operator= (const AiffAudioFormatReader&);

    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }
};

//==============================================================================
class AiffAudioFormatWriter  : public AudioFormatWriter
{
    MemoryBlock tempBlock;
    uint32 lengthInSamples, bytesWritten;
    int64 headerPosition;
    bool writeFailed;

    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }

    AiffAudioFormatWriter (const AiffAudioFormatWriter&);
    const AiffAudioFormatWriter& operator= (const AiffAudioFormatWriter&);

    void writeHeader()
    {
        const bool couldSeekOk = output->setPosition (headerPosition);
        (void) couldSeekOk;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassert (couldSeekOk);

        const int headerLen = 54;
        int audioBytes = lengthInSamples * ((bitsPerSample * numChannels) / 8);
        audioBytes += (audioBytes & 1);

        output->writeInt (chunkName ("FORM"));
        output->writeIntBigEndian (headerLen + audioBytes - 8);
        output->writeInt (chunkName ("AIFF"));
        output->writeInt (chunkName ("COMM"));
        output->writeIntBigEndian (18);
        output->writeShortBigEndian ((short) numChannels);
        output->writeIntBigEndian (lengthInSamples);
        output->writeShortBigEndian ((short) bitsPerSample);

        uint8 sampleRateBytes[10];
        zeromem (sampleRateBytes, 10);

        if (sampleRate <= 1)
        {
            sampleRateBytes[0] = 0x3f;
            sampleRateBytes[1] = 0xff;
            sampleRateBytes[2] = 0x80;
        }
        else
        {
            int mask = 0x40000000;
            sampleRateBytes[0] = 0x40;

            if (sampleRate >= mask)
            {
                jassertfalse
                sampleRateBytes[1] = 0x1d;
            }
            else
            {
                int n = (int) sampleRate;

                int i;
                for (i = 0; i <= 32 ; ++i)
                {
                    if ((n & mask) != 0)
                        break;

                    mask >>= 1;
                }

                n = n << (i + 1);

                sampleRateBytes[1] = (uint8) (29 - i);
                sampleRateBytes[2] = (uint8) ((n >> 24) & 0xff);
                sampleRateBytes[3] = (uint8) ((n >> 16) & 0xff);
                sampleRateBytes[4] = (uint8) ((n >>  8) & 0xff);
                sampleRateBytes[5] = (uint8) (n & 0xff);
            }
        }

        output->write (sampleRateBytes, 10);

        output->writeInt (chunkName ("SSND"));
        output->writeIntBigEndian (audioBytes + 8);
        output->writeInt (0);
        output->writeInt (0);

        jassert (output->getPosition() == headerLen);
    }

public:
    //==============================================================================
    AiffAudioFormatWriter (OutputStream* out,
                           const double sampleRate_,
                           const unsigned int chans,
                           const int bits)
        : AudioFormatWriter (out,
                             TRANS (aiffFormatName),
                             sampleRate_,
                             chans,
                             bits),
          lengthInSamples (0),
          bytesWritten (0),
          writeFailed (false)
    {
        headerPosition = out->getPosition();
        writeHeader();
    }

    ~AiffAudioFormatWriter()
    {
        if ((bytesWritten & 1) != 0)
            output->writeByte (0);

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
                    *b++ = (short) ByteOrder::swapIfLittleEndian ((uint16) (*left++ >> 16));
                    *b++ = (short) ByteOrder::swapIfLittleEndian ((uint16) (*right++ >> 16));
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (short) ByteOrder::swapIfLittleEndian ((uint16) (*left++ >> 16));
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
                    ByteOrder::bigEndian24BitToChars (*left++ >> 8, b);
                    b += 3;
                    ByteOrder::bigEndian24BitToChars (*right++ >> 8, b);
                    b += 3;
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    ByteOrder::bigEndian24BitToChars (*left++ >> 8, b);
                    b += 3;
                }
            }
        }
        else if (bitsPerSample == 32)
        {
            uint32* b = (uint32*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = ByteOrder::swapIfLittleEndian ((uint32) *left++);
                    *b++ = ByteOrder::swapIfLittleEndian ((uint32) *right++);
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = ByteOrder::swapIfLittleEndian ((uint32) *left++);
                }
            }
        }
        else if (bitsPerSample == 8)
        {
            char* b = (char*) buffer;

            if (numChannels > 1)
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (char) (*left++ >> 24);
                    *b++ = (char) (*right++ >> 24);
                }
            }
            else
            {
                for (int i = numSamples; --i >= 0;)
                {
                    *b++ = (char) (*left++ >> 24);
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
AiffAudioFormat::AiffAudioFormat()
    : AudioFormat (TRANS (aiffFormatName), (const tchar**) aiffExtensions)
{
}

AiffAudioFormat::~AiffAudioFormat()
{
}

const Array <int> AiffAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 0 };
    return Array <int> (rates);
}

const Array <int> AiffAudioFormat::getPossibleBitDepths()
{
    const int depths[] = { 8, 16, 24, 0 };
    return Array <int> (depths);
}

bool AiffAudioFormat::canDoStereo()
{
    return true;
}

bool AiffAudioFormat::canDoMono()
{
    return true;
}

#if JUCE_MAC
bool AiffAudioFormat::canHandleFile (const File& f)
{
    if (AudioFormat::canHandleFile (f))
        return true;

    const OSType type = PlatformUtilities::getTypeOfFile (f.getFullPathName());
    return type == 'AIFF' || type == 'AIFC'
        || type == 'aiff' || type == 'aifc';
}
#endif

AudioFormatReader* AiffAudioFormat::createReaderFor (InputStream* sourceStream,
                                                     const bool deleteStreamIfOpeningFails)
{
    ScopedPointer <AiffAudioFormatReader> w (new AiffAudioFormatReader (sourceStream));

    if (w->sampleRate != 0)
        return w.release();

    if (! deleteStreamIfOpeningFails)
        w->input = 0;

    return 0;
}

AudioFormatWriter* AiffAudioFormat::createWriterFor (OutputStream* out,
                                                     double sampleRate,
                                                     unsigned int chans,
                                                     int bitsPerSample,
                                                     const StringPairArray& /*metadataValues*/,
                                                     int /*qualityOptionIndex*/)
{
    if (getPossibleBitDepths().contains (bitsPerSample))
    {
        return new AiffAudioFormatWriter (out,
                                          sampleRate,
                                          chans,
                                          bitsPerSample);
    }

    return 0;
}

END_JUCE_NAMESPACE
