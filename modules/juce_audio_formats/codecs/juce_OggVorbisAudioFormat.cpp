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

#if JUCE_USE_OGGVORBIS

#if JUCE_MAC && ! defined (__MACOSX__)
 #define __MACOSX__ 1
#endif

#if JUCE_INCLUDE_OGGVORBIS_CODE || ! defined (JUCE_INCLUDE_OGGVORBIS_CODE)
 extern "C"
 {
  #include "ogg/include/ogg/ogg.h"
 }
#else
 extern "C"
 {
  #include <ogg/ogg.h>
 }
#endif

#endif

namespace juce
{

#if JUCE_USE_OGGVORBIS

namespace OggVorbisNamespace
{
#if JUCE_INCLUDE_OGGVORBIS_CODE || ! defined (JUCE_INCLUDE_OGGVORBIS_CODE)
 JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4267 4127 4244 4100 4701 4702 4013 4133 4206 4305 4189 4706 4995 4365 4456 4457 4459 6297 6011 6001 6308 6255 6386 6385 6246 6387 6263 6262 28182)

 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                      "-Wconversion",
                                      "-Wdeprecated-register",
                                      "-Wfloat-conversion",
                                      "-Wfloat-equal",
                                      "-Wmaybe-uninitialized",
                                      "-Wmisleading-indentation",
                                      "-Wmissing-prototypes",
                                      "-Wredundant-decls",
                                      "-Wshadow",
                                      "-Wsign-conversion",
                                      "-Wswitch-default",
                                      "-Wswitch-enum",
                                      "-Wzero-as-null-pointer-constant")

 JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

 JUCE_BEGIN_NO_SANITIZE ("undefined")

 #include "vorbis/include/vorbis/vorbisenc.h"
 #include "vorbis/include/vorbis/codec.h"
 #include "vorbis/include/vorbis/vorbisfile.h"

 #include "vorbis/lib/analysis.c"
 #include "vorbis/lib/bitrate.c"
 #include "vorbis/lib/block.c"
 #include "vorbis/lib/codebook.c"
 #include "vorbis/lib/envelope.c"
 #include "vorbis/lib/floor0.c"
 #include "vorbis/lib/floor1.c"
 #include "vorbis/lib/info.c"
 #include "vorbis/lib/lpc.c"
 #include "vorbis/lib/lsp.c"
 #include "vorbis/lib/mapping0.c"
 #include "vorbis/lib/mdct.c"
 #include "vorbis/lib/psy.c"
 #include "vorbis/lib/registry.c"
 #include "vorbis/lib/res0.c"
 #include "vorbis/lib/sharedbook.c"
 #include "vorbis/lib/smallft.c"
 #include "vorbis/lib/synthesis.c"
 #include "vorbis/lib/vorbisenc.c"
 #include "vorbis/lib/vorbisfile.c"
 #include "vorbis/lib/window.c"

 JUCE_END_NO_SANITIZE
 JUCE_END_IGNORE_DEPRECATION_WARNINGS
 JUCE_END_IGNORE_WARNINGS_MSVC
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#else
 #include <vorbis/vorbisenc.h>
 #include <vorbis/codec.h>
 #include <vorbis/vorbisfile.h>
#endif
}

#undef max
#undef min

//==============================================================================
static const char* const oggFormatName = "Ogg-Vorbis file";

const char* const OggVorbisAudioFormat::encoderName = "encoder";
const char* const OggVorbisAudioFormat::id3title = "id3title";
const char* const OggVorbisAudioFormat::id3artist = "id3artist";
const char* const OggVorbisAudioFormat::id3album = "id3album";
const char* const OggVorbisAudioFormat::id3comment = "id3comment";
const char* const OggVorbisAudioFormat::id3date = "id3date";
const char* const OggVorbisAudioFormat::id3genre = "id3genre";
const char* const OggVorbisAudioFormat::id3trackNumber = "id3trackNumber";


//==============================================================================
class OggReader final : public AudioFormatReader
{
public:
    OggReader (InputStream* inp)  : AudioFormatReader (inp, oggFormatName)
    {
        sampleRate = 0;
        usesFloatingPointData = true;

        callbacks.read_func  = &oggReadCallback;
        callbacks.seek_func  = &oggSeekCallback;
        callbacks.close_func = &oggCloseCallback;
        callbacks.tell_func  = &oggTellCallback;

        auto err = ov_open_callbacks (input, &ovFile, nullptr, 0, callbacks);

        if (err == 0)
        {
            auto* info = ov_info (&ovFile, -1);

            auto* comment = ov_comment (&ovFile, -1);
            addMetadataItem (comment, "ENCODER",     OggVorbisAudioFormat::encoderName);
            addMetadataItem (comment, "TITLE",       OggVorbisAudioFormat::id3title);
            addMetadataItem (comment, "ARTIST",      OggVorbisAudioFormat::id3artist);
            addMetadataItem (comment, "ALBUM",       OggVorbisAudioFormat::id3album);
            addMetadataItem (comment, "COMMENT",     OggVorbisAudioFormat::id3comment);
            addMetadataItem (comment, "DATE",        OggVorbisAudioFormat::id3date);
            addMetadataItem (comment, "GENRE",       OggVorbisAudioFormat::id3genre);
            addMetadataItem (comment, "TRACKNUMBER", OggVorbisAudioFormat::id3trackNumber);

            lengthInSamples = (uint32) ov_pcm_total (&ovFile, -1);
            numChannels = (unsigned int) info->channels;
            bitsPerSample = 16;
            sampleRate = (double) info->rate;

            reservoir.setSize ((int) numChannels, (int) jmin (lengthInSamples, (int64) 4096));
        }
    }

    ~OggReader() override
    {
        ov_clear (&ovFile);
    }

    void addMetadataItem (OggVorbisNamespace::vorbis_comment* comment, const char* name, const char* metadataName)
    {
        if (auto* value = vorbis_comment_query (comment, name, 0))
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

            if (bufferedRange.getStart() != ov_pcm_tell (&ovFile))
                ov_pcm_seek (&ovFile, bufferedRange.getStart());

            int bitStream = 0;
            int offset = 0;
            int numToRead = (int) bufferedRange.getLength();

            while (numToRead > 0)
            {
                float** dataIn = nullptr;
                auto samps = static_cast<int> (ov_read_float (&ovFile, &dataIn, numToRead, &bitStream));

                if (samps <= 0)
                    break;

                jassert (samps <= numToRead);

                for (int i = jmin ((int) numChannels, reservoir.getNumChannels()); --i >= 0;)
                    memcpy (reservoir.getWritePointer (i, offset), dataIn[i], (size_t) samps * sizeof (float));

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
    static size_t oggReadCallback (void* ptr, size_t size, size_t nmemb, void* datasource)
    {
        return (size_t) (static_cast<InputStream*> (datasource)->read (ptr, (int) (size * nmemb))) / size;
    }

    static int oggSeekCallback (void* datasource, ogg_int64_t offset, int whence)
    {
        auto* in = static_cast<InputStream*> (datasource);

        if (whence == SEEK_CUR)
            offset += in->getPosition();
        else if (whence == SEEK_END)
            offset += in->getTotalLength();

        in->setPosition (offset);
        return 0;
    }

    static int oggCloseCallback (void*)
    {
        return 0;
    }

    static long oggTellCallback (void* datasource)
    {
        return (long) static_cast<InputStream*> (datasource)->getPosition();
    }

private:
    OggVorbisNamespace::OggVorbis_File ovFile;
    OggVorbisNamespace::ov_callbacks callbacks;
    AudioBuffer<float> reservoir;
    Range<int64> bufferedRange;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggReader)
};

//==============================================================================
class OggWriter final : public AudioFormatWriter
{
public:
    struct VorbisInfoDeleter
    {
        void operator() (OggVorbisNamespace::vorbis_info* v) const
        {
            vorbis_info_clear (v);
            delete v;
        }
    };

    using VorbisInfo = std::unique_ptr<OggVorbisNamespace::vorbis_info, VorbisInfoDeleter>;

    /** Creates the encoder settings for the options, or nullptr if libvorbis
        rejects them.
    */
    static VorbisInfo createVorbisInfo (const AudioFormatWriterOptions& options)
    {
        VorbisInfo vi (new OggVorbisNamespace::vorbis_info);
        vorbis_info_init (vi.get());

        if (vorbis_encode_init_vbr (vi.get(),
                                    options.getNumChannels(),
                                    (long) options.getSampleRate(),
                                    jlimit (0.0f, 1.0f, (float) options.getQualityOptionIndex() * 0.1f)) != 0)
            return nullptr;

        return vi;
    }

    OggWriter (OutputStream* out, VorbisInfo infoToUse, const AudioFormatWriterOptions& options)
        : AudioFormatWriter (out,
                             oggFormatName,
                             options.getSampleRate(),
                             (unsigned int) options.getNumChannels(),
                             (unsigned int) options.getBitsPerSample()),
          vi (std::move (infoToUse))
    {
        vorbis_comment_init (&vc);

        const auto metadata = options.getMetadataValues();

        const std::pair<const char*, const char*> tags[] = { { OggVorbisAudioFormat::encoderName,    "ENCODER" },
                                                             { OggVorbisAudioFormat::id3title,       "TITLE" },
                                                             { OggVorbisAudioFormat::id3artist,      "ARTIST" },
                                                             { OggVorbisAudioFormat::id3album,       "ALBUM" },
                                                             { OggVorbisAudioFormat::id3comment,     "COMMENT" },
                                                             { OggVorbisAudioFormat::id3date,        "DATE" },
                                                             { OggVorbisAudioFormat::id3genre,       "GENRE" },
                                                             { OggVorbisAudioFormat::id3trackNumber, "TRACKNUMBER" } };

        for (const auto& [key, tag] : tags)
            if (const auto it = metadata.find (key); it != metadata.end() && it->second.isNotEmpty())
                vorbis_comment_add_tag (&vc, tag, it->second.toRawUTF8());

        vorbis_analysis_init (&vd, vi.get());
        vorbis_block_init (&vd, &vb);

        ogg_stream_init (&os, Random::getSystemRandom().nextInt());

        ogg_packet header, header_comm, header_code;
        vorbis_analysis_headerout (&vd, &vc, &header, &header_comm, &header_code);

        ogg_stream_packetin (&os, &header);
        ogg_stream_packetin (&os, &header_comm);
        ogg_stream_packetin (&os, &header_code);

        for (;;)
        {
            if (ogg_stream_flush (&os, &og) == 0)
                break;

            output->write (og.header, (size_t) og.header_len);
            output->write (og.body,   (size_t) og.body_len);
        }
    }

    ~OggWriter() override
    {
        // write a zero-length packet to show ogg that we're finished
        writeSamples (0);

        ogg_stream_clear (&os);
        vorbis_block_clear (&vb);
        vorbis_dsp_clear (&vd);
        vorbis_comment_clear (&vc);
        output->flush();
    }

    //==============================================================================
    bool write (const int** samplesToWrite, int numSamples) override
    {
        if (numSamples > 0)
        {
            const double gain = 1.0 / 0x80000000u;
            float** const vorbisBuffer = vorbis_analysis_buffer (&vd, numSamples);

            for (int i = (int) numChannels; --i >= 0;)
            {
                if (auto* dst = vorbisBuffer[i])
                {
                    if (const int* src = samplesToWrite [i])
                    {
                        for (int j = 0; j < numSamples; ++j)
                            dst[j] = (float) (src[j] * gain);
                    }
                }
            }
        }

        writeSamples (numSamples);
        return true;
    }

    void writeSamples (int numSamples)
    {
        vorbis_analysis_wrote (&vd, numSamples);

        while (vorbis_analysis_blockout (&vd, &vb) == 1)
        {
            vorbis_analysis (&vb, nullptr);
            vorbis_bitrate_addblock (&vb);

            while (vorbis_bitrate_flushpacket (&vd, &op))
            {
                ogg_stream_packetin (&os, &op);

                for (;;)
                {
                    if (ogg_stream_pageout (&os, &og) == 0)
                        break;

                    output->write (og.header, (size_t) og.header_len);
                    output->write (og.body,   (size_t) og.body_len);

                    if (ogg_page_eos (&og))
                        break;
                }
            }
        }
    }

private:
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    VorbisInfo vi;
    OggVorbisNamespace::vorbis_comment vc;
    OggVorbisNamespace::vorbis_dsp_state vd;
    OggVorbisNamespace::vorbis_block vb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OggWriter)
};


//==============================================================================
OggVorbisAudioFormat::OggVorbisAudioFormat()  : AudioFormat (oggFormatName, ".ogg")
{
}

OggVorbisAudioFormat::~OggVorbisAudioFormat()
{
}

Array<int> OggVorbisAudioFormat::getPossibleSampleRates()
{
    return { 8000, 11025, 12000, 16000, 22050, 32000,
             44100, 48000, 88200, 96000, 176400, 192000 };
}

Array<int> OggVorbisAudioFormat::getPossibleBitDepths()
{
    return { 32 };
}

bool OggVorbisAudioFormat::canDoStereo()    { return true; }
bool OggVorbisAudioFormat::canDoMono()      { return true; }
bool OggVorbisAudioFormat::isCompressed()   { return true; }

AudioFormatReader* OggVorbisAudioFormat::createReaderFor (InputStream* in, bool deleteStreamIfOpeningFails)
{
    std::unique_ptr<OggReader> r (new OggReader (in));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

std::unique_ptr<AudioFormatWriter> OggVorbisAudioFormat::createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                                          const AudioFormatWriterOptions& options)
{
    if (streamToWriteTo == nullptr)
        return nullptr;

    if (auto info = OggWriter::createVorbisInfo (options))
        return std::make_unique<OggWriter> (std::exchange (streamToWriteTo, {}).release(), std::move (info), options);

    return nullptr;
}

StringArray OggVorbisAudioFormat::getQualityOptions()
{
    return { "64 kbps", "80 kbps", "96 kbps", "112 kbps", "128 kbps", "160 kbps",
             "192 kbps", "224 kbps", "256 kbps", "320 kbps", "500 kbps" };
}

int OggVorbisAudioFormat::estimateOggFileQuality (const File& source)
{
    if (auto in = source.createInputStream())
    {
        if (auto r = std::unique_ptr<AudioFormatReader> (createReaderFor (in.release(), true)))
        {
            auto lengthSecs = (double) r->lengthInSamples / r->sampleRate;
            auto approxBitsPerSecond = (int) ((double) source.getSize() * 8 / lengthSecs);

            auto qualities = getQualityOptions();
            int bestIndex = 0;
            int bestDiff = 10000;

            for (int i = qualities.size(); --i >= 0;)
            {
                auto diff = std::abs (qualities[i].getIntValue() - approxBitsPerSecond);

                if (diff < bestDiff)
                {
                    bestDiff = diff;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }
    }

    return 0;
}

//==============================================================================
#if JUCE_UNIT_TESTS

struct OggVorbisAudioFormatTests final : public UnitTest
{
    OggVorbisAudioFormatTests()  : UnitTest ("Ogg Vorbis audio format tests", UnitTestCategories::audio) {}

    void runTest() override
    {
        OggVorbisAudioFormat format;

        beginTest ("Rejected writer options leave the stream with the caller");
        {
            MemoryBlock block;
            std::unique_ptr<OutputStream> out = std::make_unique<MemoryOutputStream> (block, false);

            auto writer = format.createWriterFor (out, AudioFormatWriterOptions{}.withSampleRate (44100)
                                                                                 .withNumChannels (0)
                                                                                 .withBitsPerSample (32));
            expect (writer == nullptr);
            expect (out != nullptr);
        }
    }
};

static const OggVorbisAudioFormatTests oggVorbisAudioFormatTests;

#endif

#endif

} // namespace juce
