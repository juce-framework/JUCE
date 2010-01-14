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

#include "juce_AudioThumbnail.h"
#include "juce_AudioThumbnailCache.h"

const int timeBeforeDeletingReader = 2000;


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
 static void swap (int& n)   { n = (int) ByteOrder::swap ((uint32) n); }
 static void swap (int64& n) { n = (int64) ByteOrder::swap ((uint64) n); }
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
    : formatManagerToUse (formatManagerToUse_),
      cache (cacheToUse),
      orginalSamplesPerThumbnailSample (orginalSamplesPerThumbnailSample_)
{
    clear();
}

AudioThumbnail::~AudioThumbnail()
{
    cache.removeThumbnail (this);

    const ScopedLock sl (readerLock);
    reader = 0;
}

void AudioThumbnail::setSource (InputSource* const newSource)
{
    cache.removeThumbnail (this);
    timerCallback(); // stops the timer and deletes the reader

    source = newSource;
    clear();

    if (newSource != 0
          && ! (cache.loadThumb (*this, newSource->hashCode())
                 && isFullyLoaded()))
    {
        {
            const ScopedLock sl (readerLock);
            reader = createReader();
        }

        if (reader != 0)
        {
            initialiseFromAudioFile (*reader);
            cache.addThumbnail (this);
        }
    }

    sendChangeMessage (this);
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
        stopTimer();

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
    reader = 0;
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
    cacheNeedsRefilling = true;
}

void AudioThumbnail::loadFrom (InputStream& input)
{
    const ScopedLock sl (readerLock);

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
    cacheNeedsRefilling = true;
}

void AudioThumbnail::saveTo (OutputStream& output) const
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();
    swapEndiannessIfNeeded (d);
    output.write (data.getData(), (int) data.getSize());
    swapEndiannessIfNeeded (d);
}

bool AudioThumbnail::initialiseFromAudioFile (AudioFormatReader& fileReader)
{
    AudioThumbnailDataFormat* d = (AudioThumbnailDataFormat*) data.getData();

    d->totalSamples = fileReader.lengthInSamples;
    d->numChannels = jmin ((uint32) 2, fileReader.numChannels);
    d->numFinishedSamples = 0;
    d->sampleRate = roundToInt (fileReader.sampleRate);
    d->numThumbnailSamples = (int) (d->totalSamples / d->samplesPerThumbSample) + 1;

    data.setSize (sizeof (AudioThumbnailDataFormat) + 3 + d->numThumbnailSamples * d->numChannels * 2);

    d = (AudioThumbnailDataFormat*) data.getData();
    zeromem (&(d->data[0]), d->numThumbnailSamples * d->numChannels * 2);

    return d->totalSamples > 0;
}

bool AudioThumbnail::readNextBlockFromAudioFile (AudioFormatReader& fileReader)
{
    AudioThumbnailDataFormat* const d = (AudioThumbnailDataFormat*) data.getData();

    if (d->numFinishedSamples < d->totalSamples)
    {
        const int numToDo = (int) jmin ((int64) 65536, d->totalSamples - d->numFinishedSamples);

        generateSection (fileReader,
                         d->numFinishedSamples,
                         numToDo);

        d->numFinishedSamples += numToDo;
    }

    cacheNeedsRefilling = true;
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

void AudioThumbnail::generateSection (AudioFormatReader& fileReader,
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

        fileReader.readMaxLevels (sourceStart,
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
        cacheNeedsRefilling = true;
        return;
    }

    if (numSamples == numSamplesCached
        && numChannelsCached == d->numChannels
        && startTime == cachedStart
        && timePerPixel == cachedTimePerPixel
        && ! cacheNeedsRefilling)
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

    cacheNeedsRefilling = false;

    if (needExtraDetail && reader == 0)
        reader = createReader();

    if (reader != 0 && timePerPixel * d->sampleRate <= d->samplesPerThumbSample)
    {
        startTimer (timeBeforeDeletingReader);

        char* cacheData = (char*) cachedLevels.getData();
        int sample = roundToInt (startTime * d->sampleRate);

        for (int i = numSamples; --i >= 0;)
        {
            const int nextSample = roundToInt ((startTime + timePerPixel) * d->sampleRate);

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
        for (int channelNum = 0; channelNum < numChannelsCached; ++channelNum)
        {
            char* const channelData = getChannelData (channelNum);
            char* cacheData = ((char*) cachedLevels.getData()) + channelNum * 2;

            const double timeToThumbSampleFactor = d->sampleRate / (double) d->samplesPerThumbSample;

            startTime = cachedStart;
            int sample = roundToInt (startTime * timeToThumbSampleFactor);
            const int numFinished = (int) (d->numFinishedSamples / d->samplesPerThumbSample);

            for (int i = numSamples; --i >= 0;)
            {
                const int nextSample = roundToInt ((startTime + timePerPixel) * timeToThumbSampleFactor);

                if (sample >= 0 && channelData != 0)
                {
                    char mx = -128;
                    char mn = 127;

                    while (sample <= nextSample)
                    {
                        if (sample >= numFinished)
                            break;

                        const int n = sample << 1;
                        const char sampMin = channelData [n];
                        const char sampMax = channelData [n + 1];

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
        const int skipLeft = jlimit (0, w, clip.getX() - x);
        w -= skipLeft;
        x += skipLeft;

        const char* cacheData = ((const char*) cachedLevels.getData())
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
