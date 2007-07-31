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

#if JUCE_USE_OGGVORBIS

#include "../../../juce_core/basics/juce_StandardHeader.h"

#if JUCE_MAC
  #define __MACOSX__ 1
#endif

#include "oggvorbis/vorbisenc.h"
#include "oggvorbis/codec.h"
#include "oggvorbis/vorbisfile.h"

BEGIN_JUCE_NAMESPACE

#include "juce_OggVorbisAudioFormat.h"
#include "../../application/juce_Application.h"
#include "../../../juce_core/basics/juce_Random.h"
#include "../../../juce_core/io/files/juce_FileInputStream.h"
#include "../../../juce_core/text/juce_LocalisedStrings.h"

//==============================================================================
#define formatName                          TRANS("Ogg-Vorbis file")
static const tchar* const extensions[] =    { T(".ogg"), 0 };


//==============================================================================
class OggReader : public AudioFormatReader
{
    OggVorbis_File ovFile;
    ov_callbacks callbacks;
    AudioSampleBuffer reservoir;
    int reservoirStart, samplesInReservoir;

public:
    //==============================================================================
    OggReader (InputStream* const inp)
        : AudioFormatReader (inp, formatName),
          reservoir (2, 2048),
          reservoirStart (0),
          samplesInReservoir (0)
    {
        sampleRate = 0;
        usesFloatingPointData = true;

        callbacks.read_func = &oggReadCallback;
        callbacks.seek_func = &oggSeekCallback;
        callbacks.close_func = &oggCloseCallback;
        callbacks.tell_func = &oggTellCallback;

        const int err = ov_open_callbacks (input, &ovFile, 0, 0, callbacks);

        if (err == 0)
        {
            vorbis_info* info = ov_info (&ovFile, -1);
            lengthInSamples = (uint32) ov_pcm_total (&ovFile, -1);
            numChannels = info->channels;
            bitsPerSample = 16;
            sampleRate = info->rate;

            reservoir.setSize (numChannels,
                               (int) jmin (lengthInSamples, (int64) reservoir.getNumSamples()));
        }
    }

    ~OggReader()
    {
        ov_clear (&ovFile);
    }

    //==============================================================================
    bool read (int** destSamples,
               int64 startSampleInFile,
               int numSamples)
    {
        if (startSampleInFile < reservoirStart
            || startSampleInFile + numSamples > reservoirStart + samplesInReservoir)
        {
            // buffer miss, so refill the reservoir
            int bitStream = 0;

            reservoirStart = (int) jmax ((int64) 0, startSampleInFile - 32);
            samplesInReservoir = jmax (numSamples + 32, reservoir.getNumSamples());
            reservoir.setSize (numChannels, samplesInReservoir, false, false, true);

            if (reservoirStart != (int) ov_pcm_tell (&ovFile))
                ov_pcm_seek (&ovFile, reservoirStart);

            int offset = 0;
            int numToRead = samplesInReservoir;

            while (numToRead > 0)
            {
                float** dataIn = 0;

                const int samps = ov_read_float (&ovFile, &dataIn, numToRead, &bitStream);
                if (samps == 0)
                    break;

                jassert (samps <= numToRead);

                for (int i = jmin (numChannels, reservoir.getNumChannels()); --i >= 0;)
                {
                    memcpy (reservoir.getSampleData (i, offset),
                            dataIn[i],
                            sizeof (float) * samps);
                }

                numToRead -= samps;
                offset += samps;
            }

            if (numToRead > 0)
                reservoir.clear (offset, numToRead);
        }

        if (numSamples > 0)
        {
            for (unsigned int i = 0; i < numChannels; ++i)
            {
                if (destSamples[i] == 0)
                    break;

                memcpy (destSamples[i],
                        reservoir.getSampleData (jmin (i, reservoir.getNumChannels()),
                                                 (int) (startSampleInFile - reservoirStart)),
                        sizeof (float) * numSamples);
            }
        }

        return true;
    }

    //==============================================================================
    static size_t oggReadCallback (void* ptr, size_t size, size_t nmemb, void* datasource)
    {
        return (size_t) (((InputStream*) datasource)->read (ptr, (int) (size * nmemb)) / size);
    }

    static int oggSeekCallback (void* datasource, ogg_int64_t offset, int whence)
    {
        InputStream* const in = (InputStream*) datasource;

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
        return (long) ((InputStream*) datasource)->getPosition();
    }

    juce_UseDebuggingNewOperator
};

//==============================================================================
class OggWriter  : public AudioFormatWriter
{
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;

public:
    bool ok;

    //==============================================================================
    OggWriter (OutputStream* const out,
               const double sampleRate,
               const int numChannels,
               const int bitsPerSample,
               const int qualityIndex)
        : AudioFormatWriter (out, formatName,
                             sampleRate,
                             numChannels,
                             bitsPerSample)
    {
        ok = false;

        vorbis_info_init (&vi);

        if (vorbis_encode_init_vbr (&vi,
                                    numChannels,
                                    (int) sampleRate,
                                    jlimit (0.0f, 1.0f, qualityIndex * 0.5f)) == 0)
        {
            vorbis_comment_init (&vc);

            if (JUCEApplication::getInstance() != 0)
                vorbis_comment_add_tag (&vc, "ENCODER",
                                        (char*) (const char*) JUCEApplication::getInstance()->getApplicationName());

            vorbis_analysis_init (&vd, &vi);
            vorbis_block_init (&vd, &vb);

            ogg_stream_init (&os, Random::getSystemRandom().nextInt());

            ogg_packet header;
            ogg_packet header_comm;
            ogg_packet header_code;

            vorbis_analysis_headerout (&vd, &vc, &header, &header_comm, &header_code);

            ogg_stream_packetin (&os, &header);
            ogg_stream_packetin (&os, &header_comm);
            ogg_stream_packetin (&os, &header_code);

            for (;;)
            {
                if (ogg_stream_flush (&os, &og) == 0)
                    break;

                output->write (og.header, og.header_len);
                output->write (og.body, og.body_len);
            }

            ok = true;
        }
    }

    ~OggWriter()
    {
        if (ok)
        {
            ogg_stream_clear (&os);
            vorbis_block_clear (&vb);
            vorbis_dsp_clear (&vd);
            vorbis_comment_clear (&vc);

            vorbis_info_clear (&vi);
            output->flush();
        }
        else
        {
            vorbis_info_clear (&vi);
            output = 0; // to stop the base class deleting this, as it needs to be returned
                        // to the caller of createWriter()
        }
    }

    //==============================================================================
    bool write (const int** samplesToWrite, int numSamples)
    {
        if (! ok)
            return false;

        if (numSamples > 0)
        {
            const double gain = 1.0 / 0x80000000u;
            float** const vorbisBuffer = vorbis_analysis_buffer (&vd, numSamples);

            for (int i = numChannels; --i >= 0;)
            {
                float* const dst = vorbisBuffer[i];
                const int* const src = samplesToWrite [i];

                if (src != 0 && dst != 0)
                {
                    for (int j = 0; j < numSamples; ++j)
                        dst[j] = (float) (src[j] * gain);
                }
            }
        }

        vorbis_analysis_wrote (&vd, numSamples);

        while (vorbis_analysis_blockout (&vd, &vb) == 1)
        {
            vorbis_analysis (&vb, 0);
            vorbis_bitrate_addblock (&vb);

            while (vorbis_bitrate_flushpacket (&vd, &op))
            {
                ogg_stream_packetin (&os, &op);

                for (;;)
                {
                    if (ogg_stream_pageout (&os, &og) == 0)
                        break;

                    output->write (og.header, og.header_len);
                    output->write (og.body, og.body_len);

                    if (ogg_page_eos (&og))
                        break;
                }
            }
        }

        return true;
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
OggVorbisAudioFormat::OggVorbisAudioFormat()
    : AudioFormat (formatName, (const tchar**) extensions)
{
}

OggVorbisAudioFormat::~OggVorbisAudioFormat()
{
}

const Array <int> OggVorbisAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 22050, 32000, 44100, 48000, 0 };
    return Array <int> (rates);
}

const Array <int> OggVorbisAudioFormat::getPossibleBitDepths()
{
    Array <int> depths;
    depths.add (32);
    return depths;
}

bool OggVorbisAudioFormat::canDoStereo()
{
    return true;
}

bool OggVorbisAudioFormat::canDoMono()
{
    return true;
}

AudioFormatReader* OggVorbisAudioFormat::createReaderFor (InputStream* in,
                                                          const bool deleteStreamIfOpeningFails)
{
    OggReader* r = new OggReader (in);

    if (r->sampleRate == 0)
    {
        if (! deleteStreamIfOpeningFails)
            r->input = 0;

        deleteAndZero (r);
    }

    return r;
}

AudioFormatWriter* OggVorbisAudioFormat::createWriterFor (OutputStream* out,
                                                          double sampleRate,
                                                          unsigned int numChannels,
                                                          int bitsPerSample,
                                                          const StringPairArray& /*metadataValues*/,
                                                          int qualityOptionIndex)
{
    OggWriter* w = new OggWriter (out,
                                  sampleRate,
                                  numChannels,
                                  bitsPerSample,
                                  qualityOptionIndex);

    if (! w->ok)
        deleteAndZero (w);

    return w;
}

bool OggVorbisAudioFormat::isCompressed()
{
    return true;
}

const StringArray OggVorbisAudioFormat::getQualityOptions()
{
    StringArray s;
    s.add ("Low Quality");
    s.add ("Medium Quality");
    s.add ("High Quality");
    return s;
}

int OggVorbisAudioFormat::estimateOggFileQuality (const File& source)
{
    FileInputStream* const in = source.createInputStream();

    if (in != 0)
    {
        AudioFormatReader* const r = createReaderFor (in, true);

        if (r != 0)
        {
            const int64 numSamps = r->lengthInSamples;
            delete r;

            const int64 fileNumSamps = source.getSize() / 4;
            const double ratio = numSamps / (double) fileNumSamps;

            if (ratio > 12.0)
                return 0;
            else if (ratio > 6.0)
                return 1;
            else
                return 2;
        }
    }

    return 1;
}

END_JUCE_NAMESPACE

#endif
