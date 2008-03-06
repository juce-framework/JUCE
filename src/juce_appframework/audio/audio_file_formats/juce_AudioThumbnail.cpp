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

#include "juce_AudioThumbnail.h"
#include "juce_AudioThumbnailCache.h"

const int timeBeforeDeletingReader = 1000;


//==============================================================================
struct AudioThumbnailDataFormat
{
    char thumbnailMagic[4];
    int samplesPerThumbSample;
    int64 totalSamples;         // source samples
    int64 numFinishedSamples;   // source samples
    int numThumbnailSamples;
    int numChannels;
    int sampleRate;
    char future[16];
    char data[1];
};

#if JUCE_BIG_ENDIAN
 static void swap (int& n)   { n = (int) swapByteOrder ((uint32) n); }
 static void swap (int64& n) { n = (int64) swapByteOrder ((uint64) n); }
#endif

static void swapEndiannessIfNeeded (AudioThumbnailDataFormat* const d)
{
    (void) d;

#if JUCE_BIG_ENDIAN
    swap (d->samplesPerThumbSample);
    swap (d->totalSamples);
    swap (d->numFinishedSamples);
    swap (d->numThumbnailSamples);
    swap (d->numChannels);
    swap (d->sampleRate);
#endif
}

//==============================================================================
AudioThumbnail::AudioThumbnail (const int orginalSamplesPerThumbnailSample_,
                                AudioFormatManager& formatManagerToUse_,
                                AudioThumbnailCache& cacheToUse)
    : cache (cacheToUse),
      formatManagerToUse (formatManagerToUse_),
      source (0),
      reader (0),
      orginalSamplesPerThumbnailSample (orginalSamplesPerThumbnailSample_)
{
    clear();
}

AudioThumbnail::~AudioThumbnail()
{
    cache.removeThumbnail (this);

    const ScopedLock sl (readerLock);
    deleteAndZero (reader);

    delete source;
}

void AudioThumbnail::setSource (InputSource* const newSource)
{
    cache.removeThumbnail (this);

    delete source;
    source = newSource;

    {
        const ScopedLock sl (readerLock);

        deleteAndZero (reader);
        reader = createReader();
    }

    clear();

    if (reader != 0)
    {
        startTimer (timeBeforeDeletingReader);

        initialiseFromAudioFile (*reader);

        cache.loadThumb (*this, newSource->hashCode());

        if (! isFullyLoaded())
            cache.addThumbnail (this);
    }
}

bool AudioThumbnail::useTimeSlice()
{
    const ScopedLock sl (readerLock);

    if (isFullyLoaded())
    {
        if (reader != 0)
            startTimer (timeBeforeDeletingReader);

        cache.removeThumbnail (this);
        return false;
    }

    if (reader == 0)
        reader = createReader();

    if (reader != 0)
    {
        readNextBlockFromAudioFile (*reader);
        startTimer (timeBeforeDeletingReader);

        sendChangeMessage (this);

        const bool justFinished = isFullyLoaded();

        if (justFinished)
            cache.storeThumb (*this, source->hashCode());

        return ! justFinished;
    }

    return false;
}

AudioFormatReader* AudioThumbnail::createReader() const
{
    if (source != 0)
    {
        InputStream* const audioFileStream = source->createInputStream();

        if (audioFileStream != 0)
            return formatManagerToUse.createReaderFor (audioFileStream);
    }

    return 0;
}

void AudioThumbnail::timerCallback()
{
    stopTimer();

    const ScopedLock sl (readerLock);
    deleteAndZero (reader);
}

void AudioThumbnail::clear()
{
    data.setSize (sizeof (AudioThumbnailDataFormat) + 3);

    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();

    d->thumbnailMagic[0] = 'j';
    d->thumbnailMagic[1] = 'a';
    d->thumbnailMagic[2] = 't';
    d->thumbnailMagic[3] = 'm';

    d->samplesPerThumbSample = orginalSamplesPerThumbnailSample;
    d->totalSamples = 0;
    d->numFinishedSamples = 0;
    d->numThumbnailSamples = 0;
    d->numChannels = 0;
    d->sampleRate = 0;

    numSamplesCached = 0;
}

void AudioThumbnail::loadFrom (InputStream& input)
{
    data.setSize (0);
    input.readIntoMemoryBlock (data);

    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();
    swapEndiannessIfNeeded (d);

    if (! (d->thumbnailMagic[0] == 'j'
             && d->thumbnailMagic[1] == 'a'
             && d->thumbnailMagic[2] == 't'
             && d->thumbnailMagic[3] == 'm'))
    {
        clear();
    }

    numSamplesCached = 0;
}

void AudioThumbnail::saveTo (OutputStream& output) const
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();
    swapEndiannessIfNeeded (d);
    output.write (data.getData(), data.getSize());
    swapEndiannessIfNeeded (d);
}

bool AudioThumbnail::initialiseFromAudioFile (AudioFormatReader& reader)
{
    AudioThumbnailDataFormat* d = (AudioThumbnailDataFormat*) data.getData();

    d->totalSamples = reader.lengthInSamples;
    d->numChannels = jmin (2, reader.numChannels);
    d->numFinishedSamples = 0;
    d->sampleRate = roundDoubleToInt (reader.sampleRate);
    d->numThumbnailSamples = (int) (d->totalSamples / d->samplesPerThumbSample) + 1;

    data.setSize (sizeof (AudioThumbnailDataFormat) + 3 + d->numThumbnailSamples * d->numChannels * 2);

    d = (AudioThumbnailDataFormat*) data.getData();
    zeromem (&(d->data[0]), d->numThumbnailSamples * d->numChannels * 2);

    return d->totalSamples > 0;
}

bool AudioThumbnail::readNextBlockFromAudioFile (AudioFormatReader& reader)
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();

    if (d->numFinishedSamples < d->totalSamples)
    {
        const int numToDo = (int) jmin ((int64) 65536, d->totalSamples - d->numFinishedSamples);

        generateSection (reader,
                         d->numFinishedSamples,
                         numToDo);

        d->numFinishedSamples += numToDo;
    }

    numSamplesCached = 0; // zap the cache
    return (d->numFinishedSamples < d->totalSamples);
}

int AudioThumbnail::getNumChannels() const throw()
{
    const AudioThumbnailDataFormat* const d = (const AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    return d->numChannels;
}

double AudioThumbnail::getTotalLength() const throw()
{
    const AudioThumbnailDataFormat* const d = (const AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    if (d->sampleRate > 0)
        return d->totalSamples / (double)d->sampleRate;
    else
        return 0.0;
}

void AudioThumbnail::generateSection (AudioFormatReader& reader,
                                      int64 startSample,
                                      int numSamples)
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    int firstDataPos = (int) (startSample / d->samplesPerThumbSample);
    int lastDataPos = (int) ((startSample + numSamples) / d->samplesPerThumbSample);

    char* l = getChannelData (0);
    char* r = getChannelData (1);

    for (int i = firstDataPos; i < lastDataPos; ++i)
    {
        const int sourceStart = i * d->samplesPerThumbSample;
        const int sourceEnd = sourceStart + d->samplesPerThumbSample;

        float lowestLeft, highestLeft, lowestRight, highestRight;

        reader.readMaxLevels (sourceStart,
                              sourceEnd - sourceStart,
                              lowestLeft,
                              highestLeft,
                              lowestRight,
                              highestRight);

        int n = i * 2;

        if (r != 0)
        {
            l [n]   = (char) jlimit (-128.0f, 127.0f, lowestLeft * 127.0f);
            r [n++] = (char) jlimit (-128.0f, 127.0f, lowestRight * 127.0f);
            l [n]   = (char) jlimit (-128.0f, 127.0f, highestLeft * 127.0f);
            r [n++] = (char) jlimit (-128.0f, 127.0f, highestRight * 127.0f);
        }
        else
        {
            l [n++] = (char) jlimit (-128.0f, 127.0f, lowestLeft * 127.0f);
            l [n++] = (char) jlimit (-128.0f, 127.0f, highestLeft * 127.0f);
        }
    }
}

char* AudioThumbnail::getChannelData (int channel) const
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    if (channel >= 0 && channel < d->numChannels)
        return d->data + (channel * 2 * d->numThumbnailSamples);

    return 0;
}

bool AudioThumbnail::isFullyLoaded() const throw()
{
    const AudioThumbnailDataFormat* const d = (const AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    return d->numFinishedSamples >= d->totalSamples;
}

void AudioThumbnail::refillCache (const int numSamples,
                                  double startTime,
                                  const double timePerPixel)
{
    const AudioThumbnailDataFormat* const d = (const AudioThumbnailDataFormat*) data.getData();
    jassert (d != 0);

    if (numSamples <= 0
         || timePerPixel <= 0.0
         || d->sampleRate <= 0)
    {
        numSamplesCached = 0;
        return;
    }

    if (numSamples == numSamplesCached
        && numChannelsCached == d->numChannels
        && startTime == cachedStart
        && timePerPixel == cachedTimePerPixel)
    {
        return;
    }

    numSamplesCached = numSamples;
    numChannelsCached = d->numChannels;
    cachedStart = startTime;
    cachedTimePerPixel = timePerPixel;

    cachedLevels.ensureSize (2 * numChannelsCached * numSamples);

    const bool needExtraDetail = (timePerPixel * d->sampleRate <= d->samplesPerThumbSample);

    const ScopedLock sl (readerLock);

    if (needExtraDetail && reader == 0)
        reader = createReader();

    if (reader != 0 && timePerPixel * d->sampleRate <= d->samplesPerThumbSample)
    {
        startTimer (timeBeforeDeletingReader);

        char* cacheData = (char*) cachedLevels;
        int sample = roundDoubleToInt (startTime * d->sampleRate);

        for (int i = numSamples; --i >= 0;)
        {
            const int nextSample = roundDoubleToInt ((startTime + timePerPixel) * d->sampleRate);

            if (sample >= 0)
            {
                if (sample >= reader->lengthInSamples)
                    break;

                float lmin, lmax, rmin, rmax;

                reader->readMaxLevels (sample,
                                       jmax (1, nextSample - sample),
                                       lmin, lmax, rmin, rmax);

                cacheData[0] = (char) jlimit (-128, 127, roundFloatToInt (lmin * 127.0f));
                cacheData[1] = (char) jlimit (-128, 127, roundFloatToInt (lmax * 127.0f));

                if (numChannelsCached > 1)
                {
                    cacheData[2] = (char) jlimit (-128, 127, roundFloatToInt (rmin * 127.0f));
                    cacheData[3] = (char) jlimit (-128, 127, roundFloatToInt (rmax * 127.0f));
                }

                cacheData += 2 * numChannelsCached;
            }

            startTime += timePerPixel;
            sample = nextSample;
        }
    }
    else
    {
        for (int channelNum = 0; channelNum < 2; ++channelNum)
        {
            char* const data = getChannelData (channelNum);
            char* cacheData = ((char*) cachedLevels) + channelNum * 2;

            const double timeToThumbSampleFactor = d->sampleRate / (double) d->samplesPerThumbSample;

            startTime = cachedStart;
            int sample = roundDoubleToInt (startTime * timeToThumbSampleFactor);
            const int numFinished = (int) (d->numFinishedSamples / d->samplesPerThumbSample);

            for (int i = numSamples; --i >= 0;)
            {
                const int nextSample = roundDoubleToInt ((startTime + timePerPixel) * timeToThumbSampleFactor);

                if (sample >= 0 && data != 0)
                {
                    char mx = -128;
                    char mn = 127;

                    while (sample <= nextSample)
                    {
                        if (sample >= numFinished)
                            break;

                        const int n = sample << 1;
                        const char sampMin = data [n];
                        const char sampMax = data [n + 1];

                        if (sampMin < mn)
                            mn = sampMin;

                        if (sampMax > mx)
                            mx = sampMax;

                        ++sample;
                    }

                    if (mn <= mx)
                    {
                        cacheData[0] = mn;
                        cacheData[1] = mx;
                    }
                    else
                    {
                        cacheData[0] = 1;
                        cacheData[1] = 0;
                    }
                }
                else
                {
                    cacheData[0] = 1;
                    cacheData[1] = 0;
                }

                cacheData += numChannelsCached * 2;
                startTime += timePerPixel;
                sample = nextSample;
            }
        }
    }
}

void AudioThumbnail::drawChannel (Graphics& g,
                                  int x, int y, int w, int h,
                                  double startTime,
                                  double endTime,
                                  int channelNum,
                                  const float verticalZoomFactor)
{
    refillCache (w, startTime, (endTime - startTime) / w);

    if (numSamplesCached >= w
        && channelNum >= 0
        && channelNum < numChannelsCached)
    {
        const float topY = (float) y;
        const float bottomY = topY + h;
        const float midY = topY + h * 0.5f;
        const float vscale = verticalZoomFactor * h / 256.0f;

        const Rectangle clip (g.getClipBounds());
        const int skipLeft = clip.getX() - x;
        w -= skipLeft;
        x += skipLeft;

        const char* cacheData = ((const char*) cachedLevels)
                                   + (channelNum << 1)
                                   + skipLeft * (numChannelsCached << 1);

        while (--w >= 0)
        {
            const char mn = cacheData[0];
            const char mx = cacheData[1];
            cacheData += numChannelsCached << 1;

            if (mn <= mx) // if the wrong way round, signifies that the sample's not yet known
                g.drawLine ((float) x, jmax (midY - mx * vscale - 0.3f, topY),
                            (float) x, jmin (midY - mn * vscale + 0.3f, bottomY));

            ++x;

            if (x >= clip.getRight())
                break;
        }
    }
}


END_JUCE_NAMESPACE
