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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

//==============================================================================
/**
    Makes it easy to quickly draw scaled views of the waveform shape of an
    audio file.

    To use this class, just create an AudioThumbnail class for the file you want
    to draw, call setSource to tell it which file or resource to use, then call
    drawChannel() to draw it.

    The class will asynchronously scan the wavefile to create its scaled-down view,
    so you should make your UI repaint itself as this data comes in. To do this, the
    AudioThumbnail is a ChangeBroadcaster, and will broadcast a message when its
    listeners should repaint themselves.

    The thumbnail stores an internal low-res version of the wave data, and this can
    be loaded and saved to avoid having to scan the file again.

    @see AudioThumbnailCache, AudioThumbnailBase

    @tags{Audio}
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
    ~AudioThumbnail() override;

    //==============================================================================
    /** Clears and resets the thumbnail. */
    void clear() override;

    /** Specifies the file or stream that contains the audio file.

        For a file, just call
        @code
        setSource (new FileInputSource (file))
        @endcode

        You can pass a nullptr in here to clear the thumbnail.
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

    /** Sets an AudioBuffer as the source for the thumbnail.

        The buffer contents aren't copied and you must ensure that the lifetime of the buffer is
        valid for as long as the AudioThumbnail uses it as its source. Calling this function will
        start reading the audio in a background thread (unless the hash code can be looked-up
        successfully in the thumbnail cache).
    */
    void setSource (const AudioBuffer<float>* newSource, double sampleRate, int64 hashCode);

    /** Same as the other setSource() overload except for int data. */
    void setSource (const AudioBuffer<int>* newSource, double sampleRate, int64 hashCode);

    /** Resets the thumbnail, ready for adding data with the specified format.
        If you're going to generate a thumbnail yourself, call this before using addBlock()
        to add the data.
    */
    void reset (int numChannels, double sampleRate, int64 totalSamplesInSource = 0) override;

    /** Adds a block of level data to the thumbnail.
        Call reset() before using this, to tell the thumbnail about the data format.
    */
    void addBlock (int64 sampleNumberInSource, const AudioBuffer<float>& newData,
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

    std::unique_ptr<LevelDataSource> source;
    std::unique_ptr<CachedWindow> window;
    OwnedArray<ThumbData> channels;

    int32 samplesPerThumbSample = 0;
    int64 totalSamples { 0 };
    int64 numSamplesFinished = 0;
    int32 numChannels = 0;
    double sampleRate = 0;
    CriticalSection lock;

    void clearChannelData();
    bool setDataSource (LevelDataSource* newSource);
    void setLevels (const MinMaxValue* const* values, int thumbIndex, int numChans, int numValues);
    void createChannels (int length);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnail)
};

} // namespace juce
