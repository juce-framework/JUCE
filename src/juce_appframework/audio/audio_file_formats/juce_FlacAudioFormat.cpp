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

#include "../../../../juce_Config.h"

#if JUCE_USE_FLAC

#define FLAC__NO_DLL 1

#include "../../../juce_core/basics/juce_StandardHeader.h"
#include "flac/all.h"

BEGIN_JUCE_NAMESPACE

#include "juce_FlacAudioFormat.h"
#include "../../../juce_core/text/juce_LocalisedStrings.h"

//==============================================================================
#define formatName                          TRANS("FLAC file")
static const tchar* const extensions[] =    { T(".flac"), 0 };


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
        : AudioFormatReader (in, formatName),
          reservoir (2, 0),
          reservoirStart (0),
          samplesInReservoir (0),
          scanningForLength (false)
    {
        lengthInSamples = 0;

        decoder = FLAC__stream_decoder_new();

        ok = FLAC__stream_decoder_init_stream (decoder,
                                               readCallback_, seekCallback_, tellCallback_, lengthCallback_,
                                               eofCallback_, writeCallback_, metadataCallback_, errorCallback_,
                                               (void*) this) == FLAC__STREAM_DECODER_INIT_STATUS_OK;

        if (ok)
        {
            FLAC__stream_decoder_process_until_end_of_metadata (decoder);

            if (lengthInSamples == 0)
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
    bool read (int** destSamples,
               int64 startSampleInFile,
               int numSamples)
    {
        if (! ok)
            return false;

        int offset = 0;

        if (startSampleInFile < 0)
        {
            const int num = (int) jmin ((int64) numSamples, -startSampleInFile);

            int n = 0;
            while (destSamples[n] != 0)
            {
                zeromem (destSamples[n], sizeof (int) * num);
                ++n;
            }

            offset += num;
            startSampleInFile += num;
            numSamples -= num;
        }

        while (numSamples > 0)
        {
            if (startSampleInFile >= reservoirStart
                 && startSampleInFile < reservoirStart + samplesInReservoir)
            {
                const int num = (int) jmin ((int64) numSamples,
                                            reservoirStart + samplesInReservoir - startSampleInFile);

                jassert (num > 0);

                int n = 0;
                while (destSamples[n] != 0)
                {
                    memcpy (destSamples[n] + offset,
                            reservoir.getSampleData (n, (int) (startSampleInFile - reservoirStart)),
                            sizeof (int) * num);
                    ++n;
                }

                offset += num;
                startSampleInFile += num;
                numSamples -= num;
            }
            else
            {
                if (startSampleInFile < reservoirStart
                     || startSampleInFile > reservoirStart + jmax (samplesInReservoir, 511))
                {
                    if (startSampleInFile >= (int) lengthInSamples)
                    {
                        samplesInReservoir = 0;
                        break;
                    }

                    // had some problems with flac crashing if the read pos is aligned more
                    // accurately than this. Probably fixed in newer versions of the library, though.
                    reservoirStart = (int) (startSampleInFile & ~511);
                    FLAC__stream_decoder_seek_absolute (decoder, (FLAC__uint64) reservoirStart);
                }
                else
                {
                    reservoirStart += samplesInReservoir;
                }

                samplesInReservoir = 0;

                FLAC__stream_decoder_process_single (decoder);

                if (samplesInReservoir == 0)
                    break;
            }
        }

        if (numSamples > 0)
        {
            int n = 0;
            while (destSamples[n] != 0)
            {
                zeromem (destSamples[n] + offset, sizeof (int) * numSamples);
                ++n;
            }
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
                const double sampleRate,
                const int numChannels,
                const int bitsPerSample_)
        : AudioFormatWriter (out, formatName,
                             sampleRate,
                             numChannels,
                             bitsPerSample_)
    {
        encoder = FLAC__stream_encoder_new();

        FLAC__stream_encoder_set_do_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_loose_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_channels (encoder, numChannels);
        FLAC__stream_encoder_set_bits_per_sample (encoder, jmin (24, bitsPerSample));
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
            const int numChannels = (samplesToWrite[1] == 0) ? 1 : 2;
            temp.setSize (sizeof (int) * numSamples * numChannels);

            buf[0] = (int*) temp.getData();
            buf[1] = buf[0] + numSamples;
            buf[2] = 0;

            for (int i = numChannels; --i >= 0;)
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

        const bool ok = output->setPosition (4);
        (void) ok;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassert (ok);

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
        return ((FlacWriter*) client_data)->writeData (buffer, (int) bytes)
                ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK
                : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }

    static FLAC__StreamEncoderSeekStatus encodeSeekCallback (const FLAC__StreamEncoder*, FLAC__uint64, void*)
    {
        return FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED;
    }

    static FLAC__StreamEncoderTellStatus encodeTellCallback (const FLAC__StreamEncoder*, FLAC__uint64*, void*)
    {
        return FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED;
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
    : AudioFormat (formatName, (const tchar**) extensions)
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
    FlacReader* r = new FlacReader (in);

    if (r->sampleRate == 0)
    {
        if (! deleteStreamIfOpeningFails)
            r->input = 0;

        deleteAndZero (r);
    }

    return r;
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
        FlacWriter* w = new FlacWriter (out,
                                        sampleRate,
                                        numberOfChannels,
                                        bitsPerSample);

        if (! w->ok)
            deleteAndZero (w);

        return w;
    }

    return 0;
}

END_JUCE_NAMESPACE

#endif
