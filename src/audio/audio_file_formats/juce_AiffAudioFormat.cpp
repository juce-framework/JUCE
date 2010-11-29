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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AiffAudioFormat.h"
#include "../../io/streams/juce_BufferedInputStream.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../text/juce_LocalisedStrings.h"


//==============================================================================
static const char* const aiffFormatName = "AIFF file";
static const char* const aiffExtensions[] = { ".aiff", ".aif", 0 };


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
                        if (ver != 0 && ver != (int) 0xa2805140)
                            break;
                    }
                    else if (type == chunkName ("COMM"))
                    {
                        hasGotType = true;

                        numChannels = (unsigned int) input->readShortBigEndian();
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

                        unsigned int sampRate = ByteOrder::bigEndianInt (sampleRateBytes + 2);
                        sampRate >>= (16414 - ByteOrder::bigEndianShort (sampleRateBytes));
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

        while (numSamples > 0)
        {
            const int tempBufSize = 480 * 3 * 4; // (keep this a multiple of 3)
            char tempBuffer [tempBufSize];

            const int numThisTime = jmin (tempBufSize / bytesPerFrame, numSamples);
            const int bytesRead = input->read (tempBuffer, numThisTime * bytesPerFrame);

            if (bytesRead < numThisTime * bytesPerFrame)
            {
                jassert (bytesRead >= 0);
                zeromem (tempBuffer + bytesRead, numThisTime * bytesPerFrame - bytesRead);
            }

            jassert (! usesFloatingPointData); // (would need to add support for this if it's possible)

            if (littleEndian)
            {
                switch (bitsPerSample)
                {
                    case 8:     ReadHelper<AudioData::Int32, AudioData::Int8,  AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 16:    ReadHelper<AudioData::Int32, AudioData::Int16, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 24:    ReadHelper<AudioData::Int32, AudioData::Int24, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 32:    ReadHelper<AudioData::Int32, AudioData::Int32, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    default:    jassertfalse; break;
                }
            }
            else
            {
                switch (bitsPerSample)
                {
                    case 8:     ReadHelper<AudioData::Int32, AudioData::Int8,  AudioData::BigEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 16:    ReadHelper<AudioData::Int32, AudioData::Int16, AudioData::BigEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 24:    ReadHelper<AudioData::Int32, AudioData::Int24, AudioData::BigEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    case 32:    ReadHelper<AudioData::Int32, AudioData::Int32, AudioData::BigEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, tempBuffer, numChannels, numThisTime); break;
                    default:    jassertfalse; break;
                }
            }

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
        }

        return true;
    }

private:
    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AiffAudioFormatReader);
};

//==============================================================================
class AiffAudioFormatWriter  : public AudioFormatWriter
{
public:
    //==============================================================================
    AiffAudioFormatWriter (OutputStream* out, double sampleRate_, unsigned int numChans, int bits)
        : AudioFormatWriter (out, TRANS (aiffFormatName), sampleRate_, numChans, bits),
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
        jassert (data != 0 && *data != 0); // the input must contain at least one channel!

        if (writeFailed)
            return false;

        const int bytes = numChannels * numSamples * bitsPerSample / 8;
        tempBlock.ensureSize (bytes, false);

        switch (bitsPerSample)
        {
            case 8:     WriteHelper<AudioData::Int8,  AudioData::Int32, AudioData::BigEndian>::write (tempBlock.getData(), numChannels, data, numSamples); break;
            case 16:    WriteHelper<AudioData::Int16, AudioData::Int32, AudioData::BigEndian>::write (tempBlock.getData(), numChannels, data, numSamples); break;
            case 24:    WriteHelper<AudioData::Int24, AudioData::Int32, AudioData::BigEndian>::write (tempBlock.getData(), numChannels, data, numSamples); break;
            case 32:    WriteHelper<AudioData::Int32, AudioData::Int32, AudioData::BigEndian>::write (tempBlock.getData(), numChannels, data, numSamples); break;
            default:    jassertfalse; break;
        }

        if (bytesWritten + bytes >= (uint32) 0xfff00000
             || ! output->write (tempBlock.getData(), bytes))
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

private:
    MemoryBlock tempBlock;
    uint32 lengthInSamples, bytesWritten;
    int64 headerPosition;
    bool writeFailed;

    static inline int chunkName (const char* const name)   { return (int) ByteOrder::littleEndianInt (name); }

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
                jassertfalse;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AiffAudioFormatWriter);
};

//==============================================================================
AiffAudioFormat::AiffAudioFormat()
    : AudioFormat (TRANS (aiffFormatName), StringArray (aiffExtensions))
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

bool AiffAudioFormat::canDoStereo() { return true; }
bool AiffAudioFormat::canDoMono()   { return true; }

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

AudioFormatReader* AiffAudioFormat::createReaderFor (InputStream* sourceStream, const bool deleteStreamIfOpeningFails)
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
                                                     unsigned int numberOfChannels,
                                                     int bitsPerSample,
                                                     const StringPairArray& /*metadataValues*/,
                                                     int /*qualityOptionIndex*/)
{
    if (getPossibleBitDepths().contains (bitsPerSample))
        return new AiffAudioFormatWriter (out, sampleRate, numberOfChannels, bitsPerSample);

    return 0;
}

END_JUCE_NAMESPACE
