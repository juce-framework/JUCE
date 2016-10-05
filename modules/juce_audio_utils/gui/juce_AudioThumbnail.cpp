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

struct AudioThumbnail::MinMaxValue
{
    MinMaxValue() noexcept
    {
        values[0] = 0;
        values[1] = 0;
    }

    inline void set (const int8 newMin, const int8 newMax) noexcept
    {
        values[0] = newMin;
        values[1] = newMax;
    }

    inline int8 getMinValue() const noexcept        { return values[0]; }
    inline int8 getMaxValue() const noexcept        { return values[1]; }

    inline void setFloat (Range<float> newRange) noexcept
    {
        values[0] = (int8) jlimit (-128, 127, roundFloatToInt (newRange.getStart() * 127.0f));
        values[1] = (int8) jlimit (-128, 127, roundFloatToInt (newRange.getEnd()   * 127.0f));

        if (values[0] == values[1])
        {
            if (values[1] == 127)
                values[0]--;
            else
                values[1]++;
        }
    }

    inline bool isNonZero() const noexcept
    {
        return values[1] > values[0];
    }

    inline int getPeak() const noexcept
    {
        return jmax (std::abs ((int) values[0]),
                     std::abs ((int) values[1]));
    }

    inline void read (InputStream& input)      { input.read (values, 2); }
    inline void write (OutputStream& output)   { output.write (values, 2); }

private:
    int8 values[2];
};

//==============================================================================
class AudioThumbnail::LevelDataSource   : public TimeSliceClient
{
public:
    LevelDataSource (AudioThumbnail& thumb, AudioFormatReader* newReader, int64 hash)
        : lengthInSamples (0), numSamplesFinished (0), sampleRate (0), numChannels (0),
          hashCode (hash), owner (thumb), reader (newReader), lastReaderUseTime (0)
    {
    }

    LevelDataSource (AudioThumbnail& thumb, InputSource* src)
        : lengthInSamples (0), numSamplesFinished (0), sampleRate (0), numChannels (0),
          hashCode (src->hashCode()), owner (thumb), source (src), lastReaderUseTime (0)
    {
    }

    ~LevelDataSource()
    {
        owner.cache.getTimeSliceThread().removeTimeSliceClient (this);
    }

    enum { timeBeforeDeletingReader = 3000 };

    void initialise (int64 samplesFinished)
    {
        const ScopedLock sl (readerLock);

        numSamplesFinished = samplesFinished;

        createReader();

        if (reader != nullptr)
        {
            lengthInSamples = reader->lengthInSamples;
            numChannels = reader->numChannels;
            sampleRate = reader->sampleRate;

            if (lengthInSamples <= 0 || isFullyLoaded())
                reader = nullptr;
            else
                owner.cache.getTimeSliceThread().addTimeSliceClient (this);
        }
    }

    void getLevels (int64 startSample, int numSamples, Array<Range<float> >& levels)
    {
        const ScopedLock sl (readerLock);

        if (reader == nullptr)
        {
            createReader();

            if (reader != nullptr)
            {
                lastReaderUseTime = Time::getMillisecondCounter();
                owner.cache.getTimeSliceThread().addTimeSliceClient (this);
            }
        }

        if (reader != nullptr)
        {
            if (levels.size() < (int) reader->numChannels)
                levels.insertMultiple (0, Range<float>(), (int) reader->numChannels - levels.size());

            reader->readMaxLevels (startSample, numSamples, levels.getRawDataPointer(), (int) reader->numChannels);

            lastReaderUseTime = Time::getMillisecondCounter();
        }
    }

    void releaseResources()
    {
        const ScopedLock sl (readerLock);
        reader = nullptr;
    }

    int useTimeSlice() override
    {
        if (isFullyLoaded())
        {
            if (reader != nullptr && source != nullptr)
            {
                if (Time::getMillisecondCounter() > lastReaderUseTime + timeBeforeDeletingReader)
                    releaseResources();
                else
                    return 200;
            }

            return -1;
        }

        bool justFinished = false;

        {
            const ScopedLock sl (readerLock);

            createReader();

            if (reader != nullptr)
            {
                if (! readNextBlock())
                    return 0;

                justFinished = true;
            }
        }

        if (justFinished)
            owner.cache.storeThumb (owner, hashCode);

        return 200;
    }

    bool isFullyLoaded() const noexcept
    {
        return numSamplesFinished >= lengthInSamples;
    }

    inline int sampleToThumbSample (const int64 originalSample) const noexcept
    {
        return (int) (originalSample / owner.samplesPerThumbSample);
    }

    int64 lengthInSamples, numSamplesFinished;
    double sampleRate;
    unsigned int numChannels;
    int64 hashCode;

private:
    AudioThumbnail& owner;
    ScopedPointer<InputSource> source;
    ScopedPointer<AudioFormatReader> reader;
    CriticalSection readerLock;
    uint32 lastReaderUseTime;

    void createReader()
    {
        if (reader == nullptr && source != nullptr)
            if (InputStream* audioFileStream = source->createInputStream())
                reader = owner.formatManagerToUse.createReaderFor (audioFileStream);
    }

    bool readNextBlock()
    {
        jassert (reader != nullptr);

        if (! isFullyLoaded())
        {
            const int numToDo = (int) jmin (256 * (int64) owner.samplesPerThumbSample, lengthInSamples - numSamplesFinished);

            if (numToDo > 0)
            {
                int64 startSample = numSamplesFinished;

                const int firstThumbIndex = sampleToThumbSample (startSample);
                const int lastThumbIndex  = sampleToThumbSample (startSample + numToDo);
                const int numThumbSamps = lastThumbIndex - firstThumbIndex;

                HeapBlock<MinMaxValue> levelData ((size_t) numThumbSamps * numChannels);
                HeapBlock<MinMaxValue*> levels (numChannels);

                for (int i = 0; i < (int) numChannels; ++i)
                    levels[i] = levelData + i * numThumbSamps;

                HeapBlock<Range<float> > levelsRead (numChannels);

                for (int i = 0; i < numThumbSamps; ++i)
                {
                    reader->readMaxLevels ((firstThumbIndex + i) * owner.samplesPerThumbSample,
                                           owner.samplesPerThumbSample, levelsRead, (int) numChannels);

                    for (int j = 0; j < (int) numChannels; ++j)
                        levels[j][i].setFloat (levelsRead[j]);
                }

                {
                    const ScopedUnlock su (readerLock);
                    owner.setLevels (levels, firstThumbIndex, (int) numChannels, numThumbSamps);
                }

                numSamplesFinished += numToDo;
                lastReaderUseTime = Time::getMillisecondCounter();
            }
        }

        return isFullyLoaded();
    }
};

//==============================================================================
class AudioThumbnail::ThumbData
{
public:
    ThumbData (const int numThumbSamples)
        : peakLevel (-1)
    {
        ensureSize (numThumbSamples);
    }

    inline MinMaxValue* getData (const int thumbSampleIndex) noexcept
    {
        jassert (thumbSampleIndex < data.size());
        return data.getRawDataPointer() + thumbSampleIndex;
    }

    int getSize() const noexcept
    {
        return data.size();
    }

    void getMinMax (int startSample, int endSample, MinMaxValue& result) const noexcept
    {
        if (startSample >= 0)
        {
            endSample = jmin (endSample, data.size() - 1);

            int8 mx = -128;
            int8 mn = 127;

            while (startSample <= endSample)
            {
                const MinMaxValue& v = data.getReference (startSample);

                if (v.getMinValue() < mn)  mn = v.getMinValue();
                if (v.getMaxValue() > mx)  mx = v.getMaxValue();

                ++startSample;
            }

            if (mn <= mx)
            {
                result.set (mn, mx);
                return;
            }
        }

        result.set (1, 0);
    }

    void write (const MinMaxValue* const values, const int startIndex, const int numValues)
    {
        resetPeak();

        if (startIndex + numValues > data.size())
            ensureSize (startIndex + numValues);

        MinMaxValue* const dest = getData (startIndex);

        for (int i = 0; i < numValues; ++i)
            dest[i] = values[i];
    }

    void resetPeak() noexcept
    {
        peakLevel = -1;
    }

    int getPeak() noexcept
    {
        if (peakLevel < 0)
        {
            for (int i = 0; i < data.size(); ++i)
            {
                const int peak = data[i].getPeak();
                if (peak > peakLevel)
                    peakLevel = peak;
            }
        }

        return peakLevel;
    }

private:
    Array<MinMaxValue> data;
    int peakLevel;

    void ensureSize (const int thumbSamples)
    {
        const int extraNeeded = thumbSamples - data.size();
        if (extraNeeded > 0)
            data.insertMultiple (-1, MinMaxValue(), extraNeeded);
    }
};

//==============================================================================
class AudioThumbnail::CachedWindow
{
public:
    CachedWindow()
        : cachedStart (0), cachedTimePerPixel (0),
          numChannelsCached (0), numSamplesCached (0),
          cacheNeedsRefilling (true)
    {
    }

    void invalidate()
    {
        cacheNeedsRefilling = true;
    }

    void drawChannel (Graphics& g, const Rectangle<int>& area,
                      const double startTime, const double endTime,
                      const int channelNum, const float verticalZoomFactor,
                      const double rate, const int numChans, const int sampsPerThumbSample,
                      LevelDataSource* levelData, const OwnedArray<ThumbData>& chans)
    {
        if (refillCache (area.getWidth(), startTime, endTime, rate,
                         numChans, sampsPerThumbSample, levelData, chans)
             && isPositiveAndBelow (channelNum, numChannelsCached))
        {
            const Rectangle<int> clip (g.getClipBounds().getIntersection (area.withWidth (jmin (numSamplesCached, area.getWidth()))));

            if (! clip.isEmpty())
            {
                const float topY = (float) area.getY();
                const float bottomY = (float) area.getBottom();
                const float midY = (topY + bottomY) * 0.5f;
                const float vscale = verticalZoomFactor * (bottomY - topY) / 256.0f;

                const MinMaxValue* cacheData = getData (channelNum, clip.getX() - area.getX());

                RectangleList<float> waveform;
                waveform.ensureStorageAllocated (clip.getWidth());

                float x = (float) clip.getX();

                for (int w = clip.getWidth(); --w >= 0;)
                {
                    if (cacheData->isNonZero())
                    {
                        const float top    = jmax (midY - cacheData->getMaxValue() * vscale - 0.3f, topY);
                        const float bottom = jmin (midY - cacheData->getMinValue() * vscale + 0.3f, bottomY);

                        waveform.addWithoutMerging (Rectangle<float> (x, top, 1.0f, bottom - top));
                    }

                    x += 1.0f;
                    ++cacheData;
                }

                g.fillRectList (waveform);
            }
        }
    }

private:
    Array<MinMaxValue> data;
    double cachedStart, cachedTimePerPixel;
    int numChannelsCached, numSamplesCached;
    bool cacheNeedsRefilling;

    bool refillCache (const int numSamples, double startTime, const double endTime,
                      const double rate, const int numChans, const int sampsPerThumbSample,
                      LevelDataSource* levelData, const OwnedArray<ThumbData>& chans)
    {
        const double timePerPixel = (endTime - startTime) / numSamples;

        if (numSamples <= 0 || timePerPixel <= 0.0 || rate <= 0)
        {
            invalidate();
            return false;
        }

        if (numSamples == numSamplesCached
             && numChannelsCached == numChans
             && startTime == cachedStart
             && timePerPixel == cachedTimePerPixel
             && ! cacheNeedsRefilling)
        {
            return ! cacheNeedsRefilling;
        }

        numSamplesCached = numSamples;
        numChannelsCached = numChans;
        cachedStart = startTime;
        cachedTimePerPixel = timePerPixel;
        cacheNeedsRefilling = false;

        ensureSize (numSamples);

        if (timePerPixel * rate <= sampsPerThumbSample && levelData != nullptr)
        {
            int sample = roundToInt (startTime * rate);
            Array<Range<float> > levels;

            int i;
            for (i = 0; i < numSamples; ++i)
            {
                const int nextSample = roundToInt ((startTime + timePerPixel) * rate);

                if (sample >= 0)
                {
                    if (sample >= levelData->lengthInSamples)
                    {
                        for (int chan = 0; chan < numChannelsCached; ++chan)
                            *getData (chan, i) = MinMaxValue();
                    }
                    else
                    {
                        levelData->getLevels (sample, jmax (1, nextSample - sample), levels);

                        const int totalChans = jmin (levels.size(), numChannelsCached);

                        for (int chan = 0; chan < totalChans; ++chan)
                            getData (chan, i)->setFloat (levels.getReference (chan));
                    }
                }

                startTime += timePerPixel;
                sample = nextSample;
            }

            numSamplesCached = i;
        }
        else
        {
            jassert (chans.size() == numChannelsCached);

            for (int channelNum = 0; channelNum < numChannelsCached; ++channelNum)
            {
                ThumbData* channelData = chans.getUnchecked (channelNum);
                MinMaxValue* cacheData = getData (channelNum, 0);

                const double timeToThumbSampleFactor = rate / (double) sampsPerThumbSample;

                startTime = cachedStart;
                int sample = roundToInt (startTime * timeToThumbSampleFactor);

                for (int i = numSamples; --i >= 0;)
                {
                    const int nextSample = roundToInt ((startTime + timePerPixel) * timeToThumbSampleFactor);

                    channelData->getMinMax (sample, nextSample, *cacheData);

                    ++cacheData;
                    startTime += timePerPixel;
                    sample = nextSample;
                }
            }
        }

        return true;
    }

    MinMaxValue* getData (const int channelNum, const int cacheIndex) noexcept
    {
        jassert (isPositiveAndBelow (channelNum, numChannelsCached) && isPositiveAndBelow (cacheIndex, data.size()));

        return data.getRawDataPointer() + channelNum * numSamplesCached
                                        + cacheIndex;
    }

    void ensureSize (const int numSamples)
    {
        const int itemsRequired = numSamples * numChannelsCached;

        if (data.size() < itemsRequired)
            data.insertMultiple (-1, MinMaxValue(), itemsRequired - data.size());
    }
};

//==============================================================================
AudioThumbnail::AudioThumbnail (const int originalSamplesPerThumbnailSample,
                                AudioFormatManager& formatManager,
                                AudioThumbnailCache& cacheToUse)
    : formatManagerToUse (formatManager),
      cache (cacheToUse),
      window (new CachedWindow()),
      samplesPerThumbSample (originalSamplesPerThumbnailSample),
      totalSamples (0),
      numSamplesFinished (0),
      numChannels (0),
      sampleRate (0)
{
}

AudioThumbnail::~AudioThumbnail()
{
    clear();
}

void AudioThumbnail::clear()
{
    source = nullptr;
    const ScopedLock sl (lock);
    clearChannelData();
}

void AudioThumbnail::clearChannelData()
{
    window->invalidate();
    channels.clear();
    totalSamples = numSamplesFinished = 0;
    numChannels = 0;
    sampleRate = 0;

    sendChangeMessage();
}

void AudioThumbnail::reset (int newNumChannels, double newSampleRate, int64 totalSamplesInSource)
{
    clear();

    const ScopedLock sl (lock);
    numChannels = newNumChannels;
    sampleRate = newSampleRate;
    totalSamples = totalSamplesInSource;

    createChannels (1 + (int) (totalSamplesInSource / samplesPerThumbSample));
}

void AudioThumbnail::createChannels (const int length)
{
    while (channels.size() < numChannels)
        channels.add (new ThumbData (length));
}

//==============================================================================
bool AudioThumbnail::loadFrom (InputStream& rawInput)
{
    BufferedInputStream input (rawInput, 4096);

    if (input.readByte() != 'j' || input.readByte() != 'a' || input.readByte() != 't' || input.readByte() != 'm')
        return false;

    const ScopedLock sl (lock);
    clearChannelData();

    samplesPerThumbSample = input.readInt();
    totalSamples = input.readInt64();             // Total number of source samples.
    numSamplesFinished = input.readInt64();       // Number of valid source samples that have been read into the thumbnail.
    int32 numThumbnailSamples = input.readInt();  // Number of samples in the thumbnail data.
    numChannels = input.readInt();                // Number of audio channels.
    sampleRate = input.readInt();                 // Source sample rate.
    input.skipNextBytes (16);                     // (reserved)

    createChannels (numThumbnailSamples);

    for (int i = 0; i < numThumbnailSamples; ++i)
        for (int chan = 0; chan < numChannels; ++chan)
            channels.getUnchecked(chan)->getData(i)->read (input);

    return true;
}

void AudioThumbnail::saveTo (OutputStream& output) const
{
    const ScopedLock sl (lock);

    const int numThumbnailSamples = channels.size() == 0 ? 0 : channels.getUnchecked(0)->getSize();

    output.write ("jatm", 4);
    output.writeInt (samplesPerThumbSample);
    output.writeInt64 (totalSamples);
    output.writeInt64 (numSamplesFinished);
    output.writeInt (numThumbnailSamples);
    output.writeInt (numChannels);
    output.writeInt ((int) sampleRate);
    output.writeInt64 (0);
    output.writeInt64 (0);

    for (int i = 0; i < numThumbnailSamples; ++i)
        for (int chan = 0; chan < numChannels; ++chan)
            channels.getUnchecked(chan)->getData(i)->write (output);
}

//==============================================================================
bool AudioThumbnail::setDataSource (LevelDataSource* newSource)
{
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    numSamplesFinished = 0;

    if (cache.loadThumb (*this, newSource->hashCode) && isFullyLoaded())
    {
        source = newSource; // (make sure this isn't done before loadThumb is called)

        source->lengthInSamples = totalSamples;
        source->sampleRate = sampleRate;
        source->numChannels = (unsigned int) numChannels;
        source->numSamplesFinished = numSamplesFinished;
    }
    else
    {
        source = newSource; // (make sure this isn't done before loadThumb is called)

        const ScopedLock sl (lock);
        source->initialise (numSamplesFinished);

        totalSamples = source->lengthInSamples;
        sampleRate = source->sampleRate;
        numChannels = (int32) source->numChannels;

        createChannels (1 + (int) (totalSamples / samplesPerThumbSample));
    }

    return sampleRate > 0 && totalSamples > 0;
}

bool AudioThumbnail::setSource (InputSource* const newSource)
{
    clear();

    return newSource != nullptr && setDataSource (new LevelDataSource (*this, newSource));
}

void AudioThumbnail::setReader (AudioFormatReader* newReader, int64 hash)
{
    clear();

    if (newReader != nullptr)
        setDataSource (new LevelDataSource (*this, newReader, hash));
}

int64 AudioThumbnail::getHashCode() const
{
    return source == nullptr ? 0 : source->hashCode;
}

void AudioThumbnail::addBlock (const int64 startSample, const AudioSampleBuffer& incoming,
                               int startOffsetInBuffer, int numSamples)
{
    jassert (startSample >= 0
              && startOffsetInBuffer >= 0
              && startOffsetInBuffer + numSamples <= incoming.getNumSamples());

    const int firstThumbIndex = (int) (startSample / samplesPerThumbSample);
    const int lastThumbIndex  = (int) ((startSample + numSamples + (samplesPerThumbSample - 1)) / samplesPerThumbSample);
    const int numToDo = lastThumbIndex - firstThumbIndex;

    if (numToDo > 0)
    {
        const int numChans = jmin (channels.size(), incoming.getNumChannels());

        const HeapBlock<MinMaxValue> thumbData ((size_t) (numToDo * numChans));
        const HeapBlock<MinMaxValue*> thumbChannels ((size_t) numChans);

        for (int chan = 0; chan < numChans; ++chan)
        {
            const float* const sourceData = incoming.getReadPointer (chan, startOffsetInBuffer);
            MinMaxValue* const dest = thumbData + numToDo * chan;
            thumbChannels [chan] = dest;

            for (int i = 0; i < numToDo; ++i)
            {
                const int start = i * samplesPerThumbSample;
                dest[i].setFloat (FloatVectorOperations::findMinAndMax (sourceData + start, jmin (samplesPerThumbSample, numSamples - start)));
            }
        }

        setLevels (thumbChannels, firstThumbIndex, numChans, numToDo);
    }
}

void AudioThumbnail::setLevels (const MinMaxValue* const* values, int thumbIndex, int numChans, int numValues)
{
    const ScopedLock sl (lock);

    for (int i = jmin (numChans, channels.size()); --i >= 0;)
        channels.getUnchecked(i)->write (values[i], thumbIndex, numValues);

    const int64 start = thumbIndex * (int64) samplesPerThumbSample;
    const int64 end = (thumbIndex + numValues) * (int64) samplesPerThumbSample;

    if (numSamplesFinished >= start && end > numSamplesFinished)
        numSamplesFinished = end;

    totalSamples = jmax (numSamplesFinished, totalSamples);
    window->invalidate();
    sendChangeMessage();
}

//==============================================================================
int AudioThumbnail::getNumChannels() const noexcept
{
    return numChannels;
}

double AudioThumbnail::getTotalLength() const noexcept
{
    return sampleRate > 0 ? (totalSamples / sampleRate) : 0;
}

bool AudioThumbnail::isFullyLoaded() const noexcept
{
    return numSamplesFinished >= totalSamples - samplesPerThumbSample;
}

double AudioThumbnail::getProportionComplete() const noexcept
{
    return jlimit (0.0, 1.0, numSamplesFinished / (double) jmax ((int64) 1, totalSamples));
}

int64 AudioThumbnail::getNumSamplesFinished() const noexcept
{
    return numSamplesFinished;
}

float AudioThumbnail::getApproximatePeak() const
{
    const ScopedLock sl (lock);
    int peak = 0;

    for (int i = channels.size(); --i >= 0;)
        peak = jmax (peak, channels.getUnchecked(i)->getPeak());

    return jlimit (0, 127, peak) / 127.0f;
}

void AudioThumbnail::getApproximateMinMax (const double startTime, const double endTime, const int channelIndex,
                                           float& minValue, float& maxValue) const noexcept
{
    const ScopedLock sl (lock);
    MinMaxValue result;
    const ThumbData* const data = channels [channelIndex];

    if (data != nullptr && sampleRate > 0)
    {
        const int firstThumbIndex = (int) ((startTime * sampleRate) / samplesPerThumbSample);
        const int lastThumbIndex  = (int) (((endTime * sampleRate) + samplesPerThumbSample - 1) / samplesPerThumbSample);

        data->getMinMax (jmax (0, firstThumbIndex), lastThumbIndex, result);
    }

    minValue = result.getMinValue() / 128.0f;
    maxValue = result.getMaxValue() / 128.0f;
}

void AudioThumbnail::drawChannel (Graphics& g, const Rectangle<int>& area, double startTime,
                                  double endTime, int channelNum, float verticalZoomFactor)
{
    const ScopedLock sl (lock);

    window->drawChannel (g, area, startTime, endTime, channelNum, verticalZoomFactor,
                         sampleRate, numChannels, samplesPerThumbSample, source, channels);
}

void AudioThumbnail::drawChannels (Graphics& g, const Rectangle<int>& area, double startTimeSeconds,
                                   double endTimeSeconds, float verticalZoomFactor)
{
    for (int i = 0; i < numChannels; ++i)
    {
        const int y1 = roundToInt ((i * area.getHeight()) / numChannels);
        const int y2 = roundToInt (((i + 1) * area.getHeight()) / numChannels);

        drawChannel (g, Rectangle<int> (area.getX(), area.getY() + y1, area.getWidth(), y2 - y1),
                     startTimeSeconds, endTimeSeconds, i, verticalZoomFactor);
    }
}
