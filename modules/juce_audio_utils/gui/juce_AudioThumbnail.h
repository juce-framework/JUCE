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

#ifndef JUCE_AUDIOTHUMBNAIL_H_INCLUDED
#define JUCE_AUDIOTHUMBNAIL_H_INCLUDED


//==============================================================================
/**
    Makes it easy to quickly draw scaled views of the waveform shape of an
    audio file.

    To use this class, just create an AudioThumbNail class for the file you want
    to draw, call setSource to tell it which file or resource to use, then call
    drawChannel() to draw it.

    The class will asynchronously scan the wavefile to create its scaled-down view,
    so you should make your UI repaint itself as this data comes in. To do this, the
    AudioThumbnail is a ChangeBroadcaster, and will broadcast a message when its
    listeners should repaint themselves.

    The thumbnail stores an internal low-res version of the wave data, and this can
    be loaded and saved to avoid having to scan the file again.

    @see AudioThumbnailCache, AudioThumbnailBase
*/
class JUCE_API  AudioThumbnail    : public AudioThumbnailBase
{
public:
    //==============================================================================
    /** Creates an audio thumbnail.

        @param sourceSamplesPerThumbnailSample  when creating a stored, low-res version
                        of the audio data, this is the scale at which it should be done. (This
                        number is the number of original samples that will be averaged for each
                        low-res sample)
        @param formatManagerToUse   the audio format manager that is used to open the file
        @param cacheToUse   an instance of an AudioThumbnailCache - this provides a background
                            thread and storage that is used to by the thumbnail, and the cache
                            object can be shared between multiple thumbnails
    */
    AudioThumbnail (int sourceSamplesPerThumbnailSample,
                    AudioFormatManager& formatManagerToUse,
                    AudioThumbnailCache& cacheToUse);

    /** Destructor. */
    ~AudioThumbnail();

    //==============================================================================
    /** Clears and resets the thumbnail. */
    void clear() override;

    /** Specifies the file or stream that contains the audio file.

        For a file, just call
        @code
        setSource (new FileInputSource (file))
        @endcode

        You can pass a zero in here to clear the thumbnail.
        The source that is passed in will be deleted by this object when it is no longer needed.
        @returns true if the source could be opened as a valid audio file, false if this failed for
        some reason.
    */
    bool setSource (InputSource* newSource) override;

    /** Gives the thumbnail an AudioFormatReader to use directly.
        This will start parsing the audio in a background thread (unless the hash code
        can be looked-up successfully in the thumbnail cache). Note that the reader
        object will be held by the thumbnail and deleted later when no longer needed.
        The thumbnail will actually keep hold of this reader until you clear the thumbnail
        or change the input source, so the file will be held open for all this time. If
        you don't want the thumbnail to keep a file handle open continuously, you
        should use the setSource() method instead, which will only open the file when
        it needs to.
    */
    void setReader (AudioFormatReader* newReader, int64 hashCode) override;

    /** Resets the thumbnail, ready for adding data with the specified format.
        If you're going to generate a thumbnail yourself, call this before using addBlock()
        to add the data.
    */
    void reset (int numChannels, double sampleRate, int64 totalSamplesInSource = 0) override;

    /** Adds a block of level data to the thumbnail.
        Call reset() before using this, to tell the thumbnail about the data format.
    */
    void addBlock (int64 sampleNumberInSource, const AudioSampleBuffer& newData,
                   int startOffsetInBuffer, int numSamples) override;

    //==============================================================================
    /** Reloads the low res thumbnail data from an input stream.

        This is not an audio file stream! It takes a stream of thumbnail data that would
        previously have been created by the saveTo() method.
        @see saveTo
    */
    bool loadFrom (InputStream& input) override;

    /** Saves the low res thumbnail data to an output stream.

        The data that is written can later be reloaded using loadFrom().
        @see loadFrom
    */
    void saveTo (OutputStream& output) const override;

    //==============================================================================
    /** Returns the number of channels in the file. */
    int getNumChannels() const noexcept override;

    /** Returns the length of the audio file, in seconds. */
    double getTotalLength() const noexcept override;

    /** Draws the waveform for a channel.

        The waveform will be drawn within  the specified rectangle, where startTime
        and endTime specify the times within the audio file that should be positioned
        at the left and right edges of the rectangle.

        The waveform will be scaled vertically so that a full-volume sample will fill
        the rectangle vertically, but you can also specify an extra vertical scale factor
        with the verticalZoomFactor parameter.
    */
    void drawChannel (Graphics& g,
                      const Rectangle<int>& area,
                      double startTimeSeconds,
                      double endTimeSeconds,
                      int channelNum,
                      float verticalZoomFactor) override;

    /** Draws the waveforms for all channels in the thumbnail.

        This will call drawChannel() to render each of the thumbnail's channels, stacked
        above each other within the specified area.

        @see drawChannel
    */
    void drawChannels (Graphics& g,
                       const Rectangle<int>& area,
                       double startTimeSeconds,
                       double endTimeSeconds,
                       float verticalZoomFactor) override;

    /** Returns true if the low res preview is fully generated. */
    bool isFullyLoaded() const noexcept override;

    /** Returns a value between 0 and 1 to indicate the progress towards loading the entire file. */
    double getProportionComplete() const noexcept;

    /** Returns the number of samples that have been set in the thumbnail. */
    int64 getNumSamplesFinished() const noexcept override;

    /** Returns the highest level in the thumbnail.
        Note that because the thumb only stores low-resolution data, this isn't
        an accurate representation of the highest value, it's only a rough approximation.
    */
    float getApproximatePeak() const override;

    /** Reads the approximate min and max levels from a section of the thumbnail.
        The lowest and highest samples are returned in minValue and maxValue, but obviously
        because the thumb only stores low-resolution data, these numbers will only be a rough
        approximation of the true values.
    */
    void getApproximateMinMax (double startTime, double endTime, int channelIndex,
                               float& minValue, float& maxValue) const noexcept override;

    /** Returns the hash code that was set by setSource() or setReader(). */
    int64 getHashCode() const override;

private:
    //==============================================================================
    AudioFormatManager& formatManagerToUse;
    AudioThumbnailCache& cache;

    class LevelDataSource;
    struct MinMaxValue;
    class ThumbData;
    class CachedWindow;

    friend class LevelDataSource;
    friend class ThumbData;
    friend class CachedWindow;
    friend struct ContainerDeletePolicy<LevelDataSource>;
    friend struct ContainerDeletePolicy<ThumbData>;
    friend struct ContainerDeletePolicy<CachedWindow>;

    ScopedPointer<LevelDataSource> source;
    ScopedPointer<CachedWindow> window;
    OwnedArray<ThumbData> channels;

    int32 samplesPerThumbSample;
    int64 totalSamples, numSamplesFinished;
    int32 numChannels;
    double sampleRate;
    CriticalSection lock;

    void clearChannelData();
    bool setDataSource (LevelDataSource* newSource);
    void setLevels (const MinMaxValue* const* values, int thumbIndex, int numChans, int numValues);
    void createChannels (int length);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnail)
};


#endif   // JUCE_AUDIOTHUMBNAIL_H_INCLUDED
