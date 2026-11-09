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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#if JUCE_USE_FLAC

#define FLAC__NO_DLL 1

#if ! defined (JUCE_INCLUDE_FLAC_CODE) || JUCE_INCLUDE_FLAC_CODE
 extern "C"
 {
     #include "flac/all.h"
 }
#else
 extern "C"
 {
     #include JUCE_FLAC_INCLUDE_PATH
 }
#endif

//==============================================================================
static const char* const flacFormatName = "FLAC file";

template <typename Item>
auto emptyRange (Item item) { return Range<Item>::emptyRange (item); }

//==============================================================================
class FlacReader final : public AudioFormatReader
{
public:
    FlacReader (InputStream* in)  : AudioFormatReader (in, flacFormatName)
    {
        lengthInSamples = 0;
        decoder = FLAC__stream_decoder_new();

        ok = FLAC__stream_decoder_init_stream (decoder,
                                               readCallback_, seekCallback_, tellCallback_, lengthCallback_,
                                               eofCallback_, writeCallback_, metadataCallback_, errorCallback_,
                                               this) == FLAC__STREAM_DECODER_INIT_STATUS_OK;

        if (ok)
        {
            FLAC__stream_decoder_process_until_end_of_metadata (decoder);

            if (lengthInSamples == 0 && sampleRate > 0)
            {
                // the length hasn't been stored in the metadata, so we'll need to
                // work it out the length the hard way, by scanning the whole file
                scanningForLength = true;
                FLAC__stream_decoder_process_until_end_of_stream (decoder);
                scanningForLength = false;
                auto tempLength = lengthInSamples;

                FLAC__stream_decoder_reset (decoder);
                FLAC__stream_decoder_process_until_end_of_metadata (decoder);
                lengthInSamples = tempLength;
            }
        }
    }

    ~FlacReader() override
    {
        FLAC__stream_decoder_delete (decoder);
    }

    void useMetadata (const FLAC__StreamMetadata_StreamInfo& info)
    {
        sampleRate = info.sample_rate;
        bitsPerSample = info.bits_per_sample;
        lengthInSamples = (unsigned int) info.total_samples;
        numChannels = info.channels;

        reservoir.setSize ((int) numChannels, 2 * (int) info.max_blocksize, false, false, true);
    }

    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        if (! ok)
            return false;

        const auto getBufferedRange = [this] { return bufferedRange; };

        const auto readFromReservoir = [this, &destSamples, &numDestChannels, &startOffsetInDestBuffer, &startSampleInFile] (const Range<int64> rangeToRead)
        {
            const auto bufferIndices = rangeToRead - bufferedRange.getStart();
            const auto writePos = (int64) startOffsetInDestBuffer + (rangeToRead.getStart() - startSampleInFile);

            for (int i = jmin (numDestChannels, reservoir.getNumChannels()); --i >= 0;)
            {
                if (destSamples[i] != nullptr)
                {
                    memcpy (destSamples[i] + writePos,
                            reservoir.getReadPointer (i) + bufferIndices.getStart(),
                            (size_t) bufferIndices.getLength() * sizeof (int));
                }
            }
        };

        const auto fillReservoir = [this] (const int64 requestedStart)
        {
            if (requestedStart >= lengthInSamples)
            {
                bufferedRange = emptyRange (requestedStart);
                return;
            }

            if (requestedStart < bufferedRange.getStart()
                || jmax (bufferedRange.getEnd(), bufferedRange.getStart() + (int64) 511) < requestedStart)
            {
                // had some problems with flac crashing if the read pos is aligned more
                // accurately than this. Probably fixed in newer versions of the library, though.
                bufferedRange = emptyRange (requestedStart & ~511);
                FLAC__stream_decoder_seek_absolute (decoder, (FLAC__uint64) bufferedRange.getStart());
                return;
            }

            bufferedRange = emptyRange (bufferedRange.getEnd());
            FLAC__stream_decoder_process_single (decoder);
        };

        const auto remainingSamples = Reservoir::doBufferedRead (Range<int64> { startSampleInFile, startSampleInFile + numSamples },
                                                                 getBufferedRange,
                                                                 readFromReservoir,
                                                                 fillReservoir);

        if (! remainingSamples.isEmpty())
            for (int i = numDestChannels; --i >= 0;)
                if (destSamples[i] != nullptr)
                    zeromem (destSamples[i] + startOffsetInDestBuffer + (remainingSamples.getStart() - startSampleInFile),
                             (size_t) remainingSamples.getLength() * sizeof (int));

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
                reservoir.setSize ((int) numChannels, numSamples, false, false, true);

            auto bitsToShift = 32 - bitsPerSample;

            for (int i = 0; i < (int) numChannels; ++i)
            {
                auto* src = buffer[i];
                int n = i;

                while (src == nullptr && n > 0)
                    src = buffer [--n];

                if (src != nullptr)
                {
                    auto* dest = reinterpret_cast<int*> (reservoir.getWritePointer (i));

                    for (int j = 0; j < numSamples; ++j)
                        dest[j] = src[j] << bitsToShift;
                }
            }

            bufferedRange.setLength (numSamples);
        }
    }

    //==============================================================================
    static FLAC__StreamDecoderReadStatus readCallback_ (const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t* bytes, void* client_data)
    {
        *bytes = (size_t) static_cast<const FlacReader*> (client_data)->input->read (buffer, (int) *bytes);
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    static FLAC__StreamDecoderSeekStatus seekCallback_ (const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void* client_data)
    {
        static_cast<const FlacReader*> (client_data)->input->setPosition ((int) absolute_byte_offset);
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }

    static FLAC__StreamDecoderTellStatus tellCallback_ (const FLAC__StreamDecoder*, FLAC__uint64* absolute_byte_offset, void* client_data)
    {
        *absolute_byte_offset = (uint64) static_cast<const FlacReader*> (client_data)->input->getPosition();
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    static FLAC__StreamDecoderLengthStatus lengthCallback_ (const FLAC__StreamDecoder*, FLAC__uint64* stream_length, void* client_data)
    {
        *stream_length = (uint64) static_cast<const FlacReader*> (client_data)->input->getTotalLength();
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }

    static FLAC__bool eofCallback_ (const FLAC__StreamDecoder*, void* client_data)
    {
        return static_cast<const FlacReader*> (client_data)->input->isExhausted();
    }

    static FLAC__StreamDecoderWriteStatus writeCallback_ (const FLAC__StreamDecoder*,
                                                          const FLAC__Frame* frame,
                                                          const FLAC__int32* const buffer[],
                                                          void* client_data)
    {
        static_cast<FlacReader*> (client_data)->useSamples (buffer, (int) frame->header.blocksize);
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    static void metadataCallback_ (const FLAC__StreamDecoder*,
                                   const FLAC__StreamMetadata* metadata,
                                   void* client_data)
    {
        static_cast<FlacReader*> (client_data)->useMetadata (metadata->data.stream_info);
    }

    static void errorCallback_ (const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*)
    {
    }

private:
    FLAC__StreamDecoder* decoder;
    AudioBuffer<float> reservoir;
    Range<int64> bufferedRange;
    bool ok = false, scanningForLength = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlacReader)
};


//==============================================================================
class FlacWriter final : public AudioFormatWriter
{
public:
    FlacWriter (OutputStream* out, double rate, uint32 numChans, uint32 bits, int qualityOptionIndex)
        : AudioFormatWriter (out, flacFormatName, rate, numChans, bits),
          streamStartPos (output != nullptr ? jmax (output->getPosition(), 0ll) : 0ll)
    {
        encoder = FLAC__stream_encoder_new();

        if (qualityOptionIndex > 0)
            FLAC__stream_encoder_set_compression_level (encoder, (uint32) jmin (8, qualityOptionIndex));

        FLAC__stream_encoder_set_do_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_loose_mid_side_stereo (encoder, numChannels == 2);
        FLAC__stream_encoder_set_channels (encoder, numChannels);
        FLAC__stream_encoder_set_bits_per_sample (encoder, jmin ((unsigned int) 24, bitsPerSample));
        FLAC__stream_encoder_set_sample_rate (encoder, (unsigned int) sampleRate);
        FLAC__stream_encoder_set_blocksize (encoder, 0);
        FLAC__stream_encoder_set_do_escape_coding (encoder, true);

        ok = FLAC__stream_encoder_init_stream (encoder,
                                               encodeWriteCallback, encodeSeekCallback,
                                               encodeTellCallback, encodeMetadataCallback,
                                               this) == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
    }

    ~FlacWriter() override
    {
        if (ok)
        {
            FLAC__stream_encoder_finish (encoder);
            output->flush();
        }
        else
        {
            output = nullptr; // to stop the base class deleting this, as it needs to be returned
                              // to the caller of createWriter()
        }

        FLAC__stream_encoder_delete (encoder);
    }

    //==============================================================================
    bool write (const int** samplesToWrite, int numSamples) override
    {
        if (! ok)
            return false;

        HeapBlock<int*> channels;
        HeapBlock<int> temp;
        auto bitsToShift = 32 - (int) bitsPerSample;

        if (bitsToShift > 0)
        {
            temp.malloc (numChannels * (size_t) numSamples);
            channels.calloc (numChannels + 1);

            for (unsigned int i = 0; i < numChannels; ++i)
            {
                if (samplesToWrite[i] == nullptr)
                    break;

                auto* destData = temp.get() + i * (size_t) numSamples;
                channels[i] = destData;

                for (int j = 0; j < numSamples; ++j)
                    destData[j] = (samplesToWrite[i][j] >> bitsToShift);
            }

            samplesToWrite = const_cast<const int**> (channels.get());
        }

        return FLAC__stream_encoder_process (encoder, (const FLAC__int32**) samplesToWrite, (unsigned) numSamples) != 0;
    }

    bool writeData (const void* const data, const int size) const
    {
        return output->write (data, (size_t) size);
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
        auto& info = metadata->data.stream_info;

        unsigned char buffer[FLAC__STREAM_METADATA_STREAMINFO_LENGTH];
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

        [[maybe_unused]] const bool seekOk = output->setPosition (streamStartPos + 4);

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
        return static_cast<FlacWriter*> (client_data)->writeData (buffer, (int) bytes)
                ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK
                : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }

    static FLAC__StreamEncoderSeekStatus encodeSeekCallback (const FLAC__StreamEncoder*, FLAC__uint64, void*)
    {
        return FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED;
    }

    static FLAC__StreamEncoderTellStatus encodeTellCallback (const FLAC__StreamEncoder*, FLAC__uint64* absolute_byte_offset, void* client_data)
    {
        if (client_data == nullptr)
            return FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED;

        *absolute_byte_offset = (FLAC__uint64) static_cast<FlacWriter*> (client_data)->output->getPosition();
        return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
    }

    static void encodeMetadataCallback (const FLAC__StreamEncoder*, const FLAC__StreamMetadata* metadata, void* client_data)
    {
        static_cast<FlacWriter*> (client_data)->writeMetaData (metadata);
    }

    bool ok = false;

private:
    FLAC__StreamEncoder* encoder;
    int64 streamStartPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlacWriter)
};


//==============================================================================
FlacAudioFormat::FlacAudioFormat()  : AudioFormat (flacFormatName, ".flac") {}
FlacAudioFormat::~FlacAudioFormat() {}

Array<int> FlacAudioFormat::getPossibleSampleRates()
{
    return { 8000, 11025, 12000, 16000, 22050, 32000, 44100, 48000,
             88200, 96000, 176400, 192000, 352800, 384000 };
}

Array<int> FlacAudioFormat::getPossibleBitDepths()
{
    return { 16, 24 };
}

bool FlacAudioFormat::canDoStereo()     { return true; }
bool FlacAudioFormat::canDoMono()       { return true; }
bool FlacAudioFormat::isCompressed()    { return true; }

AudioFormatReader* FlacAudioFormat::createReaderFor (InputStream* in, const bool deleteStreamIfOpeningFails)
{
    std::unique_ptr<FlacReader> r (new FlacReader (in));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

std::unique_ptr<AudioFormatWriter> FlacAudioFormat::createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                                     const AudioFormatWriterOptions& options)
{
    if (streamToWriteTo == nullptr || ! getPossibleBitDepths().contains (options.getBitsPerSample()))
        return nullptr;

    auto writer = std::make_unique<FlacWriter> (std::exchange (streamToWriteTo, {}).release(),
                                                options.getSampleRate(),
                                                (uint32) options.getNumChannels(),
                                                (uint32) options.getBitsPerSample(),
                                                options.getQualityOptionIndex());

    if (! writer->ok)
        return nullptr;

    return writer;
}

StringArray FlacAudioFormat::getQualityOptions()
{
    return { "0 (Fastest)", "1", "2", "3", "4", "5 (Default)","6", "7", "8 (Highest quality)" };
}

#endif

} // namespace juce
