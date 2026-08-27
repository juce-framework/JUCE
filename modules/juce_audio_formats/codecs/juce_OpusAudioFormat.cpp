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

#include <juce_core/detail/juce_FunctionPointerDestructor.h>

namespace juce
{

#if JUCE_USE_OPUS

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

#if JUCE_INCLUDE_OPUS_CODE || ! defined (JUCE_INCLUDE_OPUS_CODE)
 extern "C"
 {
  #include "ogg/include/ogg/ogg.h"
  #include "opus/opusfile/include/opusfile.h"
  #include "opus/libopusenc/include/opusenc.h"
 }
#else
 extern "C"
 {
  #include <ogg/ogg.h>
  #include <opusfile.h>
  #include <opusenc.h>
 }
#endif

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
static const char* const opusFormatName = "Opus file";

static constexpr int opusQualityBitrates[] = { 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 510 };

/*  Opus uses the Vorbis channel order
    https://www.xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-810004.3.9
*/
static std::optional<Array<AudioChannelSet::ChannelType>> getOpusChannels (int numChannels)
{
    using CT = AudioChannelSet::ChannelType;

    if (numChannels == 1)  return { { CT::centre } };
    if (numChannels == 2)  return { { CT::left, CT::right } };
    if (numChannels == 3)  return { { CT::left, CT::centre, CT::right } };
    if (numChannels == 4)  return { { CT::left, CT::right, CT::leftSurround, CT::rightSurround } };
    if (numChannels == 5)  return { { CT::left, CT::centre, CT::right, CT::leftSurround, CT::rightSurround } };
    if (numChannels == 6)  return { { CT::left, CT::centre, CT::right, CT::leftSurround, CT::rightSurround, CT::LFE } };
    if (numChannels == 7)  return { { CT::left, CT::centre, CT::right, CT::leftSurround, CT::rightSurround, CT::centreSurround, CT::LFE } };
    if (numChannels == 8)  return { { CT::left, CT::centre, CT::right, CT::leftSurroundSide, CT::rightSurroundSide, CT::leftSurroundRear, CT::rightSurroundRear, CT::LFE } };

    return std::nullopt;
}

bool OpusAudioFormat::isChannelLayoutSupported (const AudioChannelSet& channelSet)
{
    const auto opusChannels = getOpusChannels (channelSet.size());
    return opusChannels.has_value() && channelSet == AudioChannelSet::channelSetWithChannels (*opusChannels);
}

class OpusChannelMapping
{
public:
    OpusChannelMapping() = default;

    explicit OpusChannelMapping (const Array<AudioChannelSet::ChannelType>& opusChannels)
        : juceChannelIndexForOpusChannelIndex ((size_t) opusChannels.size()),
          opusChannelIndexForJuceChannelIndex ((size_t) opusChannels.size())
    {
        const auto channelSet = AudioChannelSet::channelSetWithChannels (opusChannels);

        for (const auto [opusIndex, channelType] : enumerate (opusChannels, int{}))
        {
            const auto juceIndex = channelSet.getChannelIndexForType (channelType);
            juceChannelIndexForOpusChannelIndex[(size_t) opusIndex] = juceIndex;
            opusChannelIndexForJuceChannelIndex[(size_t) juceIndex] = opusIndex;
        }
    }

    int getJuceChannelIndex (int opusChannelIndex) const
    {
        return juceChannelIndexForOpusChannelIndex[(size_t) opusChannelIndex];
    }

    int getOpusChannelIndex (int juceChannelIndex) const
    {
        return opusChannelIndexForJuceChannelIndex[(size_t) juceChannelIndex];
    }

private:
    std::vector<int> juceChannelIndexForOpusChannelIndex;
    std::vector<int> opusChannelIndexForJuceChannelIndex;
};

//==============================================================================
class OpusReader final : public AudioFormatReader
{
public:
    explicit OpusReader (InputStream* inp)  : AudioFormatReader (inp, opusFormatName)
    {
        sampleRate = 0;
        usesFloatingPointData = true;

        const OpusFileCallbacks callbacks { &opusReadCallback,
                                            &opusSeekCallback,
                                            &opusTellCallback,
                                            nullptr };

        int err = 0;
        opusFile.reset (op_open_callbacks (input, &callbacks, nullptr, 0, &err));

        if (opusFile != nullptr)
        {
            lengthInSamples = op_pcm_total (opusFile.get(), -1);
            numChannels = (unsigned int) op_channel_count (opusFile.get(), -1);
            const auto opusChannels = getOpusChannels ((int) numChannels);
            channelMapping = OpusChannelMapping { *opusChannels };
            bitsPerSample = 32;
            sampleRate = 48000.0;

            if (auto* tags = op_tags (opusFile.get(), -1))
            {
                addMetadataItem (tags, "ENCODER",     OpusAudioFormat::encoderName);
                addMetadataItem (tags, "TITLE",       OpusAudioFormat::id3title);
                addMetadataItem (tags, "ARTIST",      OpusAudioFormat::id3artist);
                addMetadataItem (tags, "ALBUM",       OpusAudioFormat::id3album);
                addMetadataItem (tags, "COMMENT",     OpusAudioFormat::id3comment);
                addMetadataItem (tags, "DATE",        OpusAudioFormat::id3date);
                addMetadataItem (tags, "GENRE",       OpusAudioFormat::id3genre);
                addMetadataItem (tags, "TRACKNUMBER", OpusAudioFormat::id3trackNumber);
            }

            reservoir.setSize ((int) numChannels, (int) jmin (lengthInSamples, (int64) 4096));
            interleaved.malloc (reservoir.getNumSamples() * (int) numChannels);
        }
    }

    AudioChannelSet getChannelLayout() override
    {
        return AudioChannelSet::channelSetWithChannels (*getOpusChannels ((int) numChannels));
    }

    void addMetadataItem (const OpusTags* tags, const char* name, const char* metadataName)
    {
        if (auto* value = opus_tags_query (tags, name, 0))
            metadataValues.set (metadataName, value);
    }

    //==============================================================================
    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        const auto getBufferedRange = [this] { return bufferedRange; };

        const auto readFromReservoir = [this, &destSamples, &numDestChannels, &startOffsetInDestBuffer, &startSampleInFile] (const Range<int64> rangeToRead)
        {
            const auto bufferIndices = rangeToRead - bufferedRange.getStart();
            const auto writePos = (int64) startOffsetInDestBuffer + (rangeToRead.getStart() - startSampleInFile);

            for (int i = jmin (numDestChannels, reservoir.getNumChannels()); --i >= 0;)
                if (destSamples[i] != nullptr)
                    memcpy (destSamples[i] + writePos,
                            reservoir.getReadPointer (i) + bufferIndices.getStart(),
                            (size_t) bufferIndices.getLength() * sizeof (float));
        };

        const auto fillReservoir = [this] (int64 requestedStart)
        {
            const auto newStart = jmax ((int64) 0, requestedStart);
            bufferedRange = Range<int64> { newStart, newStart + reservoir.getNumSamples() };

            if (bufferedRange.getStart() != op_pcm_tell (opusFile.get()))
                op_pcm_seek (opusFile.get(), bufferedRange.getStart());

            int offset = 0;
            int numToRead = (int) bufferedRange.getLength();

            while (numToRead > 0)
            {
                const auto samps = op_read_float (opusFile.get(),
                                                  interleaved,
                                                  numToRead * (int) numChannels,
                                                  nullptr);
                if (samps <= 0)
                    break;

                jassert (samps <= numToRead);

                for (int i = jmin ((int) numChannels, reservoir.getNumChannels()); --i >= 0;)
                {
                    auto* dst = reservoir.getWritePointer (i, offset);
                    const auto opusChannelIndex = channelMapping.getOpusChannelIndex (i);

                    for (int j = 0; j < samps; ++j)
                        dst[j] = interleaved[j * (int) numChannels + opusChannelIndex];
                }

                numToRead -= samps;
                offset += samps;
            }

            if (numToRead > 0)
                reservoir.clear (offset, numToRead);
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

    //==============================================================================
    static int opusReadCallback (void* datasource, unsigned char* ptr, int nbytes)
    {
        return static_cast<InputStream*> (datasource)->read (ptr, nbytes);
    }

    static int opusSeekCallback (void* datasource, opus_int64 offset, int whence)
    {
        auto* in = static_cast<InputStream*> (datasource);

        if (whence == SEEK_CUR)
            offset += in->getPosition();
        else if (whence == SEEK_END)
            offset += in->getTotalLength();

        return in->setPosition (offset) ? 0 : -1;
    }

    static opus_int64 opusTellCallback (void* datasource)
    {
        return (opus_int64) static_cast<InputStream*> (datasource)->getPosition();
    }

private:
    std::unique_ptr<OggOpusFile, FunctionPointerDestructor<op_free>> opusFile;
    OpusChannelMapping channelMapping;
    AudioBuffer<float> reservoir;
    HeapBlock<float> interleaved;
    Range<int64> bufferedRange;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpusReader)
};

//==============================================================================
class OpusWriter final : public AudioFormatWriter
{
public:
    using Encoder = std::unique_ptr<OggOpusEnc, FunctionPointerDestructor<ope_encoder_destroy>>;

    /** Creates an encoder writing to the stream, or nullptr if libopusenc rejects
        the options. The stream must outlive the encoder.
    */
    static Encoder createEncoder (OutputStream& out, const AudioFormatWriterOptions& options)
    {
        const std::unique_ptr<OggOpusComments, FunctionPointerDestructor<ope_comments_destroy>> comments (ope_comments_create());
        const auto metadata = options.getMetadataValues();

        const std::pair<const char*, const char*> tags[] = { { OpusAudioFormat::encoderName,    "ENCODER" },
                                                             { OpusAudioFormat::id3title,       "TITLE" },
                                                             { OpusAudioFormat::id3artist,      "ARTIST" },
                                                             { OpusAudioFormat::id3album,       "ALBUM" },
                                                             { OpusAudioFormat::id3comment,     "COMMENT" },
                                                             { OpusAudioFormat::id3date,        "DATE" },
                                                             { OpusAudioFormat::id3genre,       "GENRE" },
                                                             { OpusAudioFormat::id3trackNumber, "TRACKNUMBER" } };

        for (const auto& [key, tag] : tags)
            if (const auto it = metadata.find (key); it != metadata.end() && it->second.isNotEmpty())
                ope_comments_add (comments.get(), tag, it->second.toRawUTF8());

        const OpusEncCallbacks callbacks { &writeCallback, &closeCallback };
        const auto numChans = options.getNumChannels();
        int err = 0;

        Encoder encoder (ope_encoder_create_callbacks (&callbacks,
                                                       &out,
                                                       comments.get(),
                                                       (opus_int32) options.getSampleRate(),
                                                       numChans,
                                                       numChans > 2 ? 1 : 0,
                                                       &err));

        if (encoder != nullptr)
            ope_encoder_ctl (encoder.get(), OPUS_SET_BITRATE (getBitrateForQualityIndex (options.getQualityOptionIndex())));

        return encoder;
    }

    OpusWriter (OutputStream* out, Encoder encoderToUse, const AudioFormatWriterOptions& options)
        : AudioFormatWriter (out,
                             opusFormatName,
                             options.getSampleRate(),
                             AudioChannelSet::channelSetWithChannels (*getOpusChannels (options.getNumChannels())),
                             (unsigned int) options.getBitsPerSample()),
          channelMapping (*getOpusChannels (options.getNumChannels())),
          encoder (std::move (encoderToUse))
    {
    }

    ~OpusWriter() override
    {
        ope_encoder_drain (encoder.get());
        output->flush();
    }

    //==============================================================================
    bool write (const int** samplesToWrite, int numSamples) override
    {
        if (numSamples <= 0)
            return true;

        interleaved.ensureSize ((size_t) numSamples * numChannels * sizeof (float));
        auto* dst = static_cast<float*> (interleaved.getData());

        // samplesToWrite is a null terminated array and may contain fewer channels than numChannels.
        // Since the Opus channel ordering is different from JUCE's, this means we may have to use
        // a scratch buffer to bridge over this gap.
        const auto numAvailableJuceChannels = std::invoke ([&]
        {
            for (int i = 0; i < (int) numChannels; ++i)
            {
                if (*(samplesToWrite + i) == nullptr)
                    return i;
            }

            return (int) numChannels;
        });

        const auto numAvailableOpusChannels = std::invoke ([&]
        {
            int maxOpusChannelIndex = -1;

            for (int juceChannel = 0; juceChannel < numAvailableJuceChannels; ++juceChannel)
                maxOpusChannelIndex = std::max (maxOpusChannelIndex, channelMapping.getOpusChannelIndex (juceChannel));

            return maxOpusChannelIndex + 1;
        });

        const auto useScratchBuffer = numAvailableOpusChannels > numAvailableJuceChannels;

        if (useScratchBuffer && (int) scratchBuffer.size() < numSamples)
            scratchBuffer.resize ((size_t) numSamples);

        std::vector<const int*> channelsInOpusOrder ((size_t) (numAvailableOpusChannels + 1));

        for (int opusChannel = 0; opusChannel < numAvailableOpusChannels; ++opusChannel)
        {
            const auto juceChannel = channelMapping.getJuceChannelIndex (opusChannel);
            channelsInOpusOrder[(size_t) opusChannel] = juceChannel < numAvailableJuceChannels ? *(samplesToWrite + juceChannel)
                                                                                               : scratchBuffer.data();
        }

        WriteHelper<AudioData::Float32, AudioData::Int32, AudioData::NativeEndian>::write (dst,
                                                                                           (int) numChannels,
                                                                                           channelsInOpusOrder.data(),
                                                                                           numSamples);

        return ope_encoder_write_float (encoder.get(), dst, numSamples) == OPE_OK;
    }

private:
    static int getBitrateForQualityIndex (int index)
    {
        if (index <= 0)
            return OPUS_AUTO;

        return 1000 * opusQualityBitrates[jlimit (0, numElementsInArray (opusQualityBitrates) - 1, index - 1)];
    }

    static int writeCallback (void* userData, const unsigned char* ptr, opus_int32 len)
    {
        return static_cast<OutputStream*> (userData)->write (ptr, (size_t) len) ? 0 : 1;
    }

    static int closeCallback (void*)
    {
        return 0;
    }

    OpusChannelMapping channelMapping;
    std::vector<int> scratchBuffer;
    Encoder encoder;
    MemoryBlock interleaved;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpusWriter)
};

//==============================================================================
OpusAudioFormat::OpusAudioFormat()  : AudioFormat (opusFormatName, ".opus")
{
}

Array<int> OpusAudioFormat::getPossibleSampleRates()
{
    return { 8000, 11025, 12000, 16000, 22050, 24000, 32000,
             44100, 48000, 88200, 96000, 176400, 192000 };
}

Array<int> OpusAudioFormat::getPossibleBitDepths()
{
    return { 32 };
}

bool OpusAudioFormat::canDoStereo()    { return true; }
bool OpusAudioFormat::canDoMono()      { return true; }
bool OpusAudioFormat::isCompressed()   { return true; }

AudioFormatReader* OpusAudioFormat::createReaderFor (InputStream* in, bool deleteStreamIfOpeningFails)
{
    std::unique_ptr<OpusReader> r (new OpusReader (in));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

std::unique_ptr<AudioFormatWriter> OpusAudioFormat::createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                                     const AudioFormatWriterOptions& options)
{
    if (streamToWriteTo == nullptr)
        return nullptr;

    if (auto encoder = OpusWriter::createEncoder (*streamToWriteTo, options))
        return std::make_unique<OpusWriter> (std::exchange (streamToWriteTo, {}).release(), std::move (encoder), options);

    return nullptr;
}

StringArray OpusAudioFormat::getQualityOptions()
{
    StringArray options { "Auto" };

    for (const auto rate : opusQualityBitrates)
        options.add (String (rate) + " kbps");

    return options;
}

//==============================================================================
#if JUCE_UNIT_TESTS

struct OpusAudioFormatTests final : public UnitTest
{
    OpusAudioFormatTests()  : UnitTest ("Opus audio format tests", UnitTestCategories::audio) {}

    void runTest() override
    {
        OpusAudioFormat format;

        beginTest ("Stereo round trip");
        const auto stereo = roundTrip (format, 2, 48000);

        beginTest ("Mono round trip");
        roundTrip (format, 1, 48000);

        for (const auto numChannels : { 3, 4, 5, 6, 8 })
        {
            beginTest (String (numChannels) + " channel round trip");
            roundTrip (format, numChannels, 48000);
        }

        beginTest ("7 channel streams use the 6.1 layout");
        {
            const auto block = roundTrip (format, 7, 48000);
            std::unique_ptr<AudioFormatReader> reader (format.createReaderFor (new MemoryInputStream (block, false), true));
            expect (reader != nullptr && reader->getChannelLayout() == AudioChannelSet::create6point1());
            expect (format.isChannelLayoutSupported (AudioChannelSet::create6point1()));
            expect (! format.isChannelLayoutSupported (AudioChannelSet::create7point0()));
        }

        beginTest ("Missing source channels are written as silence");
        roundTrip (format, 6, 48000, 2);

        beginTest ("44.1 kHz input is resampled to 48 kHz");
        roundTrip (format, 2, 44100);

        beginTest ("Seeking is sample-accurate");
        {
            std::unique_ptr<AudioFormatReader> reader (format.createReaderFor (new MemoryInputStream (stereo, false), true));
            expect (reader != nullptr);

            if (reader == nullptr)
                return;

            const auto numSamples = (int) reader->lengthInSamples;
            AudioBuffer<float> decoded { 2, numSamples };
            expect (reader->read (&decoded, 0, numSamples, 0, true, true));

            // An unaligned range behind the current read position, so that reading it
            // forces a seek. opusfile pre-rolls the decoder after seeking, so the
            // start of the range converges rather than matching exactly; by the end
            // of the range the output must line up with the linear decode.
            constexpr int seekStart = 48000 + 123;
            constexpr int seekLength = 9600;

            AudioBuffer<float> seekBuffer { 2, seekLength };
            expect (reader->read (&seekBuffer, 0, seekLength, seekStart, true, true));

            auto maxTailDifference = 0.0f;

            for (int ch = 0; ch < 2; ++ch)
            {
                auto* a = seekBuffer.getReadPointer (ch);
                auto* b = decoded.getReadPointer (ch, seekStart);

                for (int i = seekLength - 480; i < seekLength; ++i)
                    maxTailDifference = jmax (maxTailDifference, std::abs (a[i] - b[i]));
            }

            expect (maxTailDifference < 0.001f);
        }

        beginTest ("Rejected writer options leave the stream with the caller");
        {
            MemoryBlock block;
            std::unique_ptr<OutputStream> out = std::make_unique<MemoryOutputStream> (block, false);

            auto writer = format.createWriterFor (out, AudioFormatWriterOptions{}.withSampleRate (48000)
                                                                                 .withNumChannels (0)
                                                                                 .withBitsPerSample (32));
            expect (writer == nullptr);
            expect (out != nullptr);
        }

        beginTest ("Garbage input is rejected");
        {
            MemoryBlock garbage { 4096, true };
            auto random = getRandom();

            for (size_t i = 0; i < garbage.getSize(); ++i)
                garbage[i] = (char) random.nextInt (256);

            expect (format.createReaderFor (new MemoryInputStream (garbage, false), true) == nullptr);
        }

        beginTest ("AudioFormatManager recognises .opus");
        {
            AudioFormatManager manager;
            manager.registerBasicFormats();
            expect (manager.findFormatForFileExtension ("opus") != nullptr);

            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::make_unique<MemoryInputStream> (stereo, false)) };
            expect (reader != nullptr);
        }
    }

    /* Encodes two seconds of sine waves and checks each decoded channel against a
       48 kHz rendering of the same signal. When numSourceChannels is smaller than
       numChannels, only that many channels are handed to the writer and the rest
       are expected to decode as silence. Returns the encoded stream.
    */
    MemoryBlock roundTrip (OpusAudioFormat& format, int numChannels, int rate, int numSourceChannels = -1)
    {
        if (numSourceChannels < 0)
            numSourceChannels = numChannels;

        constexpr int seconds = 2;
        constexpr int decodedRate = 48000;
        auto source = makeSines (numChannels, seconds * rate, rate, numSourceChannels);
        const AudioBuffer<float> toWrite { source.getArrayOfWritePointers(), numSourceChannels, source.getNumSamples() };

        MemoryBlock block;

        {
            std::unique_ptr<OutputStream> out = std::make_unique<MemoryOutputStream> (block, false);

            auto writer = format.createWriterFor (out,
                                                  AudioFormatWriterOptions{}.withSampleRate (rate)
                                                                            .withNumChannels (numChannels)
                                                                            .withBitsPerSample (32)
                                                                            .withQualityOptionIndex (5) // 128 kbps
                                                                            .withMetadataValues ({ { OpusAudioFormat::id3title, "sines" },
                                                                                                   { OpusAudioFormat::id3artist, "JUCE" } }));
            expect (writer != nullptr);
            expect (out == nullptr);

            if (writer == nullptr)
                return {};

            expect (writer->writeFromAudioSampleBuffer (toWrite, 0, toWrite.getNumSamples()));
        }

        std::unique_ptr<AudioFormatReader> reader (format.createReaderFor (new MemoryInputStream (block, false), true));
        expect (reader != nullptr);

        if (reader == nullptr)
            return {};

        expectEquals ((int) reader->numChannels, numChannels);
        expectEquals (reader->sampleRate, (double) decodedRate);

        // When the input sample rate is not 48 kHz, the decoded length can be off by one due to resampling.
        expect (std::abs (reader->lengthInSamples - (int64) (seconds * decodedRate)) <= 1);

        expect (reader->usesFloatingPointData);
        expectEquals (reader->metadataValues[OpusAudioFormat::id3title], String ("sines"));
        expectEquals (reader->metadataValues[OpusAudioFormat::id3artist], String ("JUCE"));

        const auto numSamples = (int) reader->lengthInSamples;
        AudioBuffer<float> decoded { numChannels, numSamples };
        expect (reader->read (&decoded, 0, numSamples, 0, true, true));

        // Skip the codec's convergence at the start and the resampler's tail at the end.
        const auto reference = makeSines (numChannels, numSamples, decodedRate, numSourceChannels);
        expectChannelsMatch (reference, decoded, decodedRate / 4, numSamples - decodedRate / 4);

        return block;
    }

    /* Every channel gets a different frequency so that swapped channels are detected.
       The codec only keeps the lowest CELT band of an LFE channel, so LFE channels get a
       tone well below 200 Hz. Channels from numActiveChannels onwards are left silent.
    */
    static AudioBuffer<float> makeSines (int numChannels, int numSamples, double rate, int numActiveChannels)
    {
        const auto layout = AudioChannelSet::channelSetWithChannels (*getOpusChannels (numChannels));
        AudioBuffer<float> buffer { numChannels, numSamples };
        buffer.clear();

        for (int ch = 0; ch < numActiveChannels; ++ch)
        {
            auto* dst = buffer.getWritePointer (ch);
            const auto freq = layout.getTypeOfChannel (ch) == AudioChannelSet::LFE ? 80.0 : 440.0 * (ch + 1);

            for (int i = 0; i < numSamples; ++i)
                dst[i] = (float) (0.5 * std::sin (MathConstants<double>::twoPi * freq * i / rate));
        }

        return buffer;
    }

    /* A swapped or dropped channel scores around 0 dB or worse. The LFE stream is
       allocated only a few kbps by the encoder, so it decodes with a lower SNR
       than the other channels.
    */
    void expectChannelsMatch (const AudioBuffer<float>& reference, const AudioBuffer<float>& actual, int start, int end)
    {
        const auto layout = AudioChannelSet::channelSetWithChannels (*getOpusChannels (reference.getNumChannels()));

        for (int ch = 0; ch < reference.getNumChannels(); ++ch)
        {
            const auto minSnr = layout.getTypeOfChannel (ch) == AudioChannelSet::LFE ? 10.0f : 20.0f;
            auto* r = reference.getReadPointer (ch);
            auto* a = actual.getReadPointer (ch);
            float signal = 0.0f, noise = 0.0f;

            for (int i = start; i < end; ++i)
            {
                signal += square (r[i]);
                noise += square (a[i] - r[i]);
            }

            const auto channel = "channel " + String (ch);

            if (signal > 0.0f)
            {
                const auto snr = 10.0f * std::log10 (signal / jmax (noise, 1.0e-13f));
                expect (snr > minSnr, channel + " SNR " + String (snr, 1) + " dB");
            }
            else
                expect (std::sqrt (noise / (float) (end - start)) < 1.0e-3f, channel + " should be silent");
        }
    }
};

static const OpusAudioFormatTests opusAudioFormatTests;

#endif

#endif

} // namespace juce
