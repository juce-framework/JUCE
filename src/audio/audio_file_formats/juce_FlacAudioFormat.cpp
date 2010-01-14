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

#include "../../core/juce_TargetPlatform.h"
#include "../../../juce_Config.h"

#if JUCE_USE_FLAC

#if JUCE_WINDOWS
 #include <windows.h>
#endif

#include "../../core/juce_StandardHeader.h"

#ifdef _MSC_VER
  #pragma warning (disable : 4505)
  #pragma warning (push)
#endif

namespace FlacNamespace
{
#if JUCE_INCLUDE_FLAC_CODE
 #define FLAC__NO_DLL 1

 #if ! defined (SIZE_MAX)
  #define SIZE_MAX 0xffffffff
 #endif

 #define __STDC_LIMIT_MACROS 1
 #include "flac/all.h"
 #include "flac/libFLAC/bitmath.c"
 #include "flac/libFLAC/bitreader.c"
 #include "flac/libFLAC/bitwriter.c"
 #include "flac/libFLAC/cpu.c"
 #include "flac/libFLAC/crc.c"
 #include "flac/libFLAC/fixed.c"
 #include "flac/libFLAC/float.c"
 #include "flac/libFLAC/format.c"
 #include "flac/libFLAC/lpc_flac.c"
 #include "flac/libFLAC/md5.c"
 #include "flac/libFLAC/memory.c"
 #include "flac/libFLAC/stream_decoder.c"
 #include "flac/libFLAC/stream_encoder.c"
 #include "flac/libFLAC/stream_encoder_framing.c"
 #include "flac/libFLAC/window_flac.c"
#else
 #include <FLAC/all.h>
#endif
}

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_FlacAudioFormat.h"
#include "../../text/juce_LocalisedStrings.h"

using namespace FlacNamespace;

//==============================================================================
static const char* const flacFormatName = "FLAC file";
static const tchar* const flacExtensions[] =    { T(".flac"), 0 };


//==============================================================================
class FlacReader  : public AudioFormatReader
{
    FLAC__StreamDecoder* decoder;
    AudioSampleBuffer reservoir;
    int reservoirStart, samplesInReservoir;
    bool ok, scanningForLength;

public:
    //==============================================================================
    FlacReader (InputStream* const in)
        : AudioFormatReader (in, TRANS (flacFormatName)),
          reservoir (2, 0),
          reservoirStart (0),
          samplesInReservoir (0),
          scanningForLength (false)
    {
        using namespace FlacNamespace;
        lengthInSamples = 0;

        decoder = FLAC__stream_decoder_new();

        ok = FLAC__stream_decoder_init_stream (decoder,
                                               readCallback_, seekCallback_, tellCallback_, lengthCallback_,
                                               eofCallback_, writeCallback_, metadataCallback_, errorCallback_,
                                               (void*) this) == FLAC__STREAM_DECODER_INIT_STATUS_OK;

        if (ok)
        {
            FLAC__stream_decoder_process_until_end_of_metadata (decoder);

            if (lengthInSamples == 0 && sampleRate > 0)
            {
                // the length hasn't been stored in the metadata, so we'll need to
                // work it out the length the hard way, by scanning the whole file..
                scanningForLength = true;
                FLAC__stream_decoder_process_until_end_of_stream (decoder);
                scanningForLength = false;
                const int64 tempLength = lengthInSamples;

                FLAC__stream_decoder_reset (decoder);
                FLAC__stream_decoder_process_until_end_of_metadata (decoder);
                lengthInSamples = tempLength;
            }
        }
    }

    ~FlacReader()
    {
        FLAC__stream_decoder_delete (decoder);
    }

    void useMetadata (const FLAC__StreamMetadata_StreamInfo& info)
    {
        sampleRate = info.sample_rate;
        bitsPerSample = info.bits_per_sample;
        lengthInSamples = (unsigned int) info.total_samples;
        numChannels = info.channels;

        reservoir.setSize (numChannels, 2 * info.max_blocksize, false, false, true);
    }

    // returns the number of samples read
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        using namespace FlacNamespace;

        if (! ok)
            return false;

        while (numSamples > 0)
        {
            if (startSampleInFile >= reservoirStart
                 && startSampleInFile < reservoirStart + samplesInReservoir)
            {
                const int num = (int) jmin ((int64) numSamples,
                                            reservoirStart + samplesInReservoir - startSampleInFile);

                jassert (num > 0);

                for (int i = jmin (numDestChannels, reservoir.getNumChannels()); --i >= 0;)
                    if (destSamples[i] != 0)
                        memcpy (destSamples[i] + startOffsetInDestBuffer,
                                reservoir.getSampleData (i, (int) (startSampleInFile - reservoirStart)),
                                sizeof (int) * num);

                startOffsetInDestBuffer += num;
                startSampleInFile += num;
                numSamples -= num;
            }
            else
            {
                if (startSampleInFile >= (int) lengthInSamples)
                {
                    samplesInReservoir = 0;
                }
                else if (startSampleInFile < reservoirStart
                          || startSampleInFile > reservoirStart + jmax (samplesInReservoir, 511))
                {
                    // had some problems with flac crashing if the read pos is aligned more
                    // accurately than this. Probably fixed in newer versions of the library, though.
                    reservoirStart = (int) (startSampleInFile & ~511);
                    samplesInReservoir = 0;
                    FLAC__stream_decoder_seek_absolute (decoder, (FLAC__uint64) reservoirStart);
                }
                else
                {
                    reservoirStart += samplesInReservoir;
                    samplesInReservoir = 0;
                    FLAC__stream_decoder_process_single (decoder);
                }

                if (samplesInReservoir == 0)
                    break;
            }
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

    void useSamples (const FLAC__int32* const buffer[], int numSamples)
    {
        if (scanningForLength)
        {
            lengthInSamples += numSamples;
        }
        else
        {
            if (numSamples > reservoir.getNumSamples())
                reservoir.setSize (numChannels, numSamples, false, false, true);

            const int bitsToShift = 32 - bitsPerSample;

            for (int i = 0; i < (int) numChannels; ++i)
            {
                const FLAC__int32* src = buffer[i];

                int n = i;
                while (src == 0 && n > 0)
                    src = buffer [--n];

                if (src != 0)
                {
                    int* dest = (int*) reservoir.getSampleData(i);

                    for (int j = 0; j < numSamples; ++j)
                        dest[j] = src[j] << bitsToShift;
                }
            }

            samplesInReservoir = numSamples;
        }
    }

    //==============================================================================
    static FLAC__StreamDecoderReadStatus readCallback_ (const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t* bytes, void* client_data)
    {
        *bytes = (unsigned int) ((const FlacReader*) client_data)->input->read (buffer, (int) *bytes);
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    static FLAC__StreamDecoderSeekStatus seekCallback_ (const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void* client_data)
    {
        ((const FlacReader*) client_data)->input->setPosition ((int) absolute_byte_offset);
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }

    static FLAC__StreamDecoderTellStatus tellCallback_ (const FLAC__StreamDecoder*, FLAC__uint64* absolute_byte_offset, void* client_data)
    {
        *absolute_byte_offset = ((const FlacReader*) client_data)->input->getPosition();
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    static FLAC__StreamDecoderLengthStatus lengthCallback_ (const FLAC__StreamDecoder*, FLAC__uint64* stream_length, void* client_data)
    {
        *stream_length = ((const FlacReader*) client_data)->input->getTotalLength();
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }

    static FLAC__bool eofCallback_ (const FLAC__StreamDecoder*, void* client_data)
    {
        return ((const FlacReader*) client_data)->input->isExhausted();
    }

    static FLAC__StreamDecoderWriteStatus writeCallback_ (const FLAC__StreamDecoder*,
                                                          const FLAC__Frame* frame,
                                                          const FLAC__int32* const buffer[],
                                                          void* client_data)
    {
        ((FlacReader*) client_data)->useSamples (buffer, frame->header.blocksize);
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    static void metadataCallback_ (const FLAC__StreamDecoder*,
                                   const FLAC__StreamMetadata* metadata,
                                   void* client_data)
    {
        ((FlacReader*) client_data)->useMetadata (metadata->data.stream_info);
    }

    static void errorCallback_ (const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*)
    {
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
class FlacWriter  : public AudioFormatWriter
{
    FLAC__StreamEncoder* encoder;
    MemoryBlock temp;

public:
    bool ok;

    //==============================================================================
    FlacWriter (OutputStream* const out,
                const double sampleRate_,
                const int numChannels_,
                const int bitsPerSample_)
        : AudioFormatWriter (out, TRANS (flacFormatName),
                             sampleRate_,
                             numChannels_,
                             bitsPerSample_)
    {
        using namespace FlacNamespace;
        encoder = FLAC__stream_encoder_new();

        FLAC__stream_encoder_set_do_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_loose_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_channels (encoder, numChannels);
        FLAC__stream_encoder_set_bits_per_sample (encoder, jmin ((unsigned int) 24, bitsPerSample));
        FLAC__stream_encoder_set_sample_rate (encoder, (unsigned int) sampleRate);
        FLAC__stream_encoder_set_blocksize (encoder, 2048);
        FLAC__stream_encoder_set_do_escape_coding (encoder, true);

        ok = FLAC__stream_encoder_init_stream (encoder,
                                               encodeWriteCallback, encodeSeekCallback,
                                               encodeTellCallback, encodeMetadataCallback,
                                               (void*) this) == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
    }

    ~FlacWriter()
    {
        if (ok)
        {
            FLAC__stream_encoder_finish (encoder);
            output->flush();
        }
        else
        {
            output = 0; // to stop the base class deleting this, as it needs to be returned
                        // to the caller of createWriter()
        }

        FLAC__stream_encoder_delete (encoder);
    }

    //==============================================================================
    bool write (const int** samplesToWrite, int numSamples)
    {
        if (! ok)
            return false;

        int* buf[3];
        const int bitsToShift = 32 - bitsPerSample;

        if (bitsToShift > 0)
        {
            const int numChannelsToWrite = (samplesToWrite[1] == 0) ? 1 : 2;
            temp.setSize (sizeof (int) * numSamples * numChannelsToWrite);

            buf[0] = (int*) temp.getData();
            buf[1] = buf[0] + numSamples;
            buf[2] = 0;

            for (int i = numChannelsToWrite; --i >= 0;)
            {
                if (samplesToWrite[i] != 0)
                {
                    for (int j = 0; j < numSamples; ++j)
                        buf [i][j] = (samplesToWrite [i][j] >> bitsToShift);
                }
            }

            samplesToWrite = (const int**) buf;
        }

        return FLAC__stream_encoder_process (encoder,
                                             (const FLAC__int32**) samplesToWrite,
                                             numSamples) != 0;
    }

    bool writeData (const void* const data, const int size) const
    {
        return output->write (data, size);
    }

    static void packUint32 (FLAC__uint32 val, FLAC__byte* b, const int bytes)
    {
        b += bytes;

        for (int i = 0; i < bytes; ++i)
        {
            *(--b) = (FLAC__byte) (val & 0xff);
            val >>= 8;
        }
    }

    void writeMetaData (const FLAC__StreamMetadata* metadata)
    {
        using namespace FlacNamespace;
        const FLAC__StreamMetadata_StreamInfo& info = metadata->data.stream_info;

        unsigned char buffer [FLAC__STREAM_METADATA_STREAMINFO_LENGTH];
        const unsigned int channelsMinus1 = info.channels - 1;
        const unsigned int bitsMinus1 = info.bits_per_sample - 1;

        packUint32 (info.min_blocksize, buffer, 2);
        packUint32 (info.max_blocksize, buffer + 2, 2);
        packUint32 (info.min_framesize, buffer + 4, 3);
        packUint32 (info.max_framesize, buffer + 7, 3);
        buffer[10] = (uint8) ((info.sample_rate >> 12) & 0xff);
        buffer[11] = (uint8) ((info.sample_rate >> 4) & 0xff);
        buffer[12] = (uint8) (((info.sample_rate & 0x0f) << 4) | (channelsMinus1 << 1) | (bitsMinus1 >> 4));
        buffer[13] = (FLAC__byte) (((bitsMinus1 & 0x0f) << 4) | (unsigned int) ((info.total_samples >> 32) & 0x0f));
        packUint32 ((FLAC__uint32) info.total_samples, buffer + 14, 4);
        memcpy (buffer + 18, info.md5sum, 16);

        const bool seekOk = output->setPosition (4);
        (void) seekOk;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassert (seekOk);

        output->writeIntBigEndian (FLAC__STREAM_METADATA_STREAMINFO_LENGTH);
        output->write (buffer, FLAC__STREAM_METADATA_STREAMINFO_LENGTH);
    }

    //==============================================================================
    static FLAC__StreamEncoderWriteStatus encodeWriteCallback (const FLAC__StreamEncoder*,
                                                               const FLAC__byte buffer[],
                                                               size_t bytes,
                                                               unsigned int /*samples*/,
                                                               unsigned int /*current_frame*/,
                                                               void* client_data)
    {
        using namespace FlacNamespace;
        return ((FlacWriter*) client_data)->writeData (buffer, (int) bytes)
                ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK
                : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }

    static FLAC__StreamEncoderSeekStatus encodeSeekCallback (const FLAC__StreamEncoder*, FLAC__uint64, void*)
    {
        return FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED;
    }

    static FLAC__StreamEncoderTellStatus encodeTellCallback (const FLAC__StreamEncoder*, FLAC__uint64* absolute_byte_offset, void* client_data)
    {
        if (client_data == 0)
            return FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED;

        *absolute_byte_offset = (FLAC__uint64) ((FlacWriter*) client_data)->output->getPosition();
        return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
    }

    static void encodeMetadataCallback (const FLAC__StreamEncoder*,
                                        const FLAC__StreamMetadata* metadata,
                                        void* client_data)
    {
        ((FlacWriter*) client_data)->writeMetaData (metadata);
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
FlacAudioFormat::FlacAudioFormat()
    : AudioFormat (TRANS (flacFormatName), (const tchar**) flacExtensions)
{
}

FlacAudioFormat::~FlacAudioFormat()
{
}

const Array <int> FlacAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 22050, 32000, 44100, 48000, 88200, 96000, 0 };
    return Array <int> (rates);
}

const Array <int> FlacAudioFormat::getPossibleBitDepths()
{
    const int depths[] = { 16, 24, 0 };
    return Array <int> (depths);
}

bool FlacAudioFormat::canDoStereo()
{
    return true;
}

bool FlacAudioFormat::canDoMono()
{
    return true;
}

bool FlacAudioFormat::isCompressed()
{
    return true;
}

AudioFormatReader* FlacAudioFormat::createReaderFor (InputStream* in,
                                                     const bool deleteStreamIfOpeningFails)
{
    ScopedPointer <FlacReader> r (new FlacReader (in));

    if (r->sampleRate != 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = 0;

    return 0;
}

AudioFormatWriter* FlacAudioFormat::createWriterFor (OutputStream* out,
                                                     double sampleRate,
                                                     unsigned int numberOfChannels,
                                                     int bitsPerSample,
                                                     const StringPairArray& /*metadataValues*/,
                                                     int /*qualityOptionIndex*/)
{
    if (getPossibleBitDepths().contains (bitsPerSample))
    {
        ScopedPointer <FlacWriter> w (new FlacWriter (out,
                                                      sampleRate,
                                                      numberOfChannels,
                                                      bitsPerSample));

        if (w->ok)
            return w.release();
    }

    return 0;
}

END_JUCE_NAMESPACE

#endif
