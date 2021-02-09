/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_USE_WAVPACK

namespace WavPackNamespace
{
#if JUCE_INCLUDE_WAVPACK_CODE || ! defined (JUCE_INCLUDE_WAVPACK_CODE)

#if ! JUCE_MSVC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#if JUCE_MSVC
#pragma warning(push)
#pragma warning(disable : 4838)
#pragma warning(disable : 4389)
#pragma warning(disable : 4245)
#pragma warning(disable : 4189)
#endif

#include "wavpack/common_utils.c"
#include "wavpack/entropy_utils.c"
#include "wavpack/decorr_utils.c"
#include "wavpack/tags.c"
#include "wavpack/open_legacy.c"
#include "wavpack/open_utils.c"
#include "wavpack/read_words.c"
#include "wavpack/unpack_floats.c"
#include "wavpack/unpack.c"
#include "wavpack/unpack_seek.c"
#include "wavpack/unpack_utils.c"

#if ! JUCE_MSVC
#pragma GCC diagnostic pop
#endif

#if JUCE_MSVC
#pragma warning(pop)
#endif

#else
#include <wavpack/wavpack.h>
#endif
}

//==============================================================================
static const char* const wavPackFormatName = "WavPack file";

//==============================================================================
class WavPackReader : public AudioFormatReader
{
public:
    WavPackReader (InputStream* const inp)
        : AudioFormatReader (inp, wavPackFormatName),
          reservoirStart (0),
          samplesInReservoir (0),
          sampleBuffer (nullptr)
    {
        using namespace WavPackNamespace;
        sampleRate = 0;
        usesFloatingPointData = true;

        wvReader.read_bytes = &wv_read_bytes;
        wvReader.get_pos = &wv_get_pos;
        wvReader.set_pos_abs = &wv_set_pos_abs;
        wvReader.set_pos_rel = &wv_set_pos_rel;
        wvReader.get_length = &wv_get_length;
        wvReader.can_seek = &wv_can_seek;
        wvReader.push_back_byte = &wv_push_back_byte;

        wvContext = WavpackOpenFileInputEx (&wvReader, input, nullptr, wvErrorBuffer, OPEN_NORMALIZE, 0);

        if (wvContext != nullptr)
        {
            lengthInSamples = (uint32) WavpackGetNumSamples(wvContext);
            numChannels = (unsigned int) WavpackGetNumChannels(wvContext);
            bitsPerSample = WavpackGetBitsPerSample(wvContext);
            sampleRate = WavpackGetSampleRate(wvContext);

            reservoir.setSize ((int) numChannels, (int) jmin (lengthInSamples, (int64) 4096));
        }
    }

    ~WavPackReader()
    {
        using namespace WavPackNamespace;
        WavpackCloseFile (wvContext);
    }

    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        using namespace WavPackNamespace;
        while (numSamples > 0)
        {
            const int numAvailable = (int) (reservoirStart + samplesInReservoir - startSampleInFile);

            if (startSampleInFile >= reservoirStart && numAvailable > 0)
            {
                // got a few samples overlapping, so use them before seeking..
                const int numToUse = jmin (numSamples, numAvailable);

                for (int i = jmin (numDestChannels, reservoir.getNumChannels()); --i >= 0;)
                    if (destSamples[i] != nullptr)
                        memcpy (destSamples[i] + startOffsetInDestBuffer,
                                reservoir.getReadPointer (i, (int) (startSampleInFile - reservoirStart)),
                                sizeof (float) * (size_t) numToUse);

                startSampleInFile += numToUse;
                numSamples -= numToUse;
                startOffsetInDestBuffer += numToUse;

                if (numSamples == 0)
                    break;
            }

            if (startSampleInFile < reservoirStart
                || startSampleInFile + numSamples > reservoirStart + samplesInReservoir)
            {
                // buffer miss, so refill the reservoir
                reservoirStart = jmax (0, (int) startSampleInFile);
                samplesInReservoir = reservoir.getNumSamples();

                if (reservoirStart != (int) WavpackGetSampleIndex (wvContext))
                    WavpackSeekSample (wvContext, reservoirStart);

                int offset = 0;
                int numToRead = samplesInReservoir;

                while (numToRead > 0)
                {
                    // initialize buffer
                    if (sampleBuffer == nullptr)
                    {
                        sampleBufferSize = numToRead * numChannels;
                        sampleBuffer = new int32_t[numToRead * numChannels];
                    }

                    // reallocate if buffer size is too small
                    if (sampleBufferSize < numToRead * numChannels)
                    {
                        sampleBufferSize = numToRead * numChannels;
                        delete []sampleBuffer;
                        sampleBuffer = new int32_t[sampleBufferSize];
                    }

                    const long samps = WavpackUnpackSamples (wvContext, sampleBuffer, numToRead);

                    if (samps <= 0)
                        break;

                    jassert (samps <= numToRead);

                    auto p1 = reservoir.getWritePointer (0, offset);
                    auto p2 = reservoir.getWritePointer (1, offset);

                    float *fp1 = p1;
                    float *fp2 = p2;
                    int32_t *in = sampleBuffer;

                    float maxF = 1.0f;
                    if (bitsPerSample == 16)
                        maxF = 32767.0f;
                    else if (bitsPerSample == 24)
                        maxF = 8388607.0f;
                    else if (bitsPerSample == 32)
                        maxF = 32768.0f * 65536.0f;

                    if (WavpackGetMode(wvContext) & MODE_FLOAT)
                        maxF = 1.0f;

                    for (int i = 0; i < samps; ++i)
                    {
                        *fp1 = (float)*in / maxF;
                        in++;
                        fp1++;

                        *fp2 = (float)*in / maxF;
                        in++;
                        fp2++;
                    }

                    numToRead -= samps;
                    offset += samps;
                }

                if (numToRead > 0)
                    reservoir.clear (offset, numToRead);
            }
        }

        if (numSamples > 0)
        {
            for (int i = numDestChannels; --i >= 0;)
                if (destSamples[i] != nullptr)
                    zeromem (destSamples[i] + startOffsetInDestBuffer, sizeof (int) * (size_t) numSamples);
        }

        return true;
    }

    //==============================================================================
    static int32_t wv_read_bytes (void *id, void *data, int32_t bcount)
    {
        return (int32_t) (static_cast<InputStream*> (id)->read (data, (int) bcount));
    }

    static uint32_t wv_get_pos (void *id)
    {
        InputStream* const in = static_cast<InputStream*> (id);
        return in->getPosition ();
    }

    static int wv_set_pos_abs (void *id, uint32_t pos)
    {
        InputStream* const in = static_cast<InputStream*> (id);
        in->setPosition (pos);
        return 0;
    }

    static int wv_push_back_byte (void *id, int c)
    {
        InputStream* const in = static_cast<InputStream*> (id);

        if (0 == in->setPosition (in->getPosition() - 1))
        {
            return EOF;
        }

        return c;
    }

    static int wv_set_pos_rel (void *id, int32_t delta, int mode)
    {
        InputStream* const in = static_cast<InputStream*> (id);

        if (mode == SEEK_CUR)
            delta += in->getPosition();
        else if (mode == SEEK_END)
            delta += in->getTotalLength();

        in->setPosition (delta);
        return 0;
    }

    static uint32_t wv_get_length (void *id)
    {
        return static_cast<InputStream*> (id)->getTotalLength ();
    }

    static int wv_can_seek (void *id)
    {
        return 1;
    }

private:
    WavPackNamespace::WavpackStreamReader wvReader;
    WavPackNamespace::WavpackContext* wvContext;
    char wvErrorBuffer[80];
    AudioSampleBuffer reservoir;
    int reservoirStart, samplesInReservoir;
    int32_t *sampleBuffer;
    size_t sampleBufferSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavPackReader)
};

//==============================================================================
WavPackAudioFormat::WavPackAudioFormat()  : AudioFormat (wavPackFormatName, ".wv")
{
}

WavPackAudioFormat::~WavPackAudioFormat()
{
}

Array<int> WavPackAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 8000, 11025, 12000, 16000, 22050, 32000,
                          44100, 48000, 88200, 96000, 176400, 192000 };

    return Array<int> (rates, numElementsInArray (rates));
}

Array<int> WavPackAudioFormat::getPossibleBitDepths()
{
    const int depths[] = { 16, 24, 32 };

    return Array<int> (depths, numElementsInArray (depths));
}

bool WavPackAudioFormat::canDoStereo()    { return true; }
bool WavPackAudioFormat::canDoMono()      { return true; }
bool WavPackAudioFormat::isCompressed()   { return true; }

AudioFormatReader* WavPackAudioFormat::createReaderFor (InputStream* in, const bool deleteStreamIfOpeningFails)
{
    ScopedPointer<WavPackReader> r (new WavPackReader (in));

    if (r->sampleRate > 0)
    {
        return r.release();
    }

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* WavPackAudioFormat::createWriterFor (OutputStream* out,
                                                        double sampleRate,
                                                        unsigned int numChannels,
                                                        int bitsPerSample,
                                                        const StringPairArray& metadataValues,
                                                        int qualityOptionIndex)
{
    jassertfalse; // not yet implemented!
    return nullptr;
}

StringArray WavPackAudioFormat::getQualityOptions()
{
    static const char* options[] = { "fast", "high", "very high", 0 };
    return StringArray (options);
}

#endif
