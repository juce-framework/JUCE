/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_AUDIOTHUMBNAILBASE_H_INCLUDED
#define JUCE_AUDIOTHUMBNAILBASE_H_INCLUDED

class AudioThumbnailCache;

//==============================================================================
/**
    Provides a base for classes that can store and draw scaled views of an
    audio waveform.

    Typically, you'll want to use the derived class AudioThumbnail, which provides
    a concrete implementation.

    @see AudioThumbnail, AudioThumbnailCache
*/
class JUCE_API  AudioThumbnailBase    : public ChangeBroadcaster,
                                        public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver
{
public:
    //==============================================================================
    AudioThumbnailBase() {}
    virtual ~AudioThumbnailBase() {}

    //==============================================================================
    /** Clears and resets the thumbnail. */
    virtual void clear() = 0;

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
    virtual bool setSource (InputSource* newSource) = 0;

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
    virtual void setReader (AudioFormatReader* newReader, int64 hashCode) = 0;

    //==============================================================================
    /** Reloads the low res thumbnail data from an input stream.

        This is not an audio file stream! It takes a stream of thumbnail data that would
        previously have been created by the saveTo() method.
        @see saveTo
    */
    virtual bool loadFrom (InputStream& input) = 0;

    /** Saves the low res thumbnail data to an output stream.

        The data that is written can later be reloaded using loadFrom().
        @see loadFrom
    */
    virtual void saveTo (OutputStream& output) const = 0;

    //==============================================================================
    /** Returns the number of channels in the file. */
    virtual int getNumChannels() const noexcept = 0;

    /** Returns the length of the audio file, in seconds. */
    virtual double getTotalLength() const noexcept = 0;

    /** Draws the waveform for a channel.

        The waveform will be drawn within  the specified rectangle, where startTime
        and endTime specify the times within the audio file that should be positioned
        at the left and right edges of the rectangle.

        The waveform will be scaled vertically so that a full-volume sample will fill
        the rectangle vertically, but you can also specify an extra vertical scale factor
        with the verticalZoomFactor parameter.
    */
    virtual void drawChannel (Graphics& g,
                              const Rectangle<int>& area,
                              double startTimeSeconds,
                              double endTimeSeconds,
                              int channelNum,
                              float verticalZoomFactor) = 0;

    /** Draws the waveforms for all channels in the thumbnail.

        This will call drawChannel() to render each of the thumbnail's channels, stacked
        above each other within the specified area.

        @see drawChannel
    */
    virtual void drawChannels (Graphics& g,
                               const Rectangle<int>& area,
                               double startTimeSeconds,
                               double endTimeSeconds,
                               float verticalZoomFactor) = 0;

    /** Returns true if the low res preview is fully generated. */
    virtual bool isFullyLoaded() const noexcept = 0;

    /** Returns the number of samples that have been set in the thumbnail. */
    virtual int64 getNumSamplesFinished() const noexcept = 0;

    /** Returns the highest level in the thumbnail.
        Note that because the thumb only stores low-resolution data, this isn't
        an accurate representation of the highest value, it's only a rough approximation.
    */
    virtual float getApproximatePeak() const = 0;

    /** Reads the approximate min and max levels from a section of the thumbnail.
        The lowest and highest samples are returned in minValue and maxValue, but obviously
        because the thumb only stores low-resolution data, these numbers will only be a rough
        approximation of the true values.
    */
    virtual void getApproximateMinMax (double startTime, double endTime, int channelIndex,
                                       float& minValue, float& maxValue) const noexcept = 0;

    /** Returns the hash code that was set by setSource() or setReader(). */
    virtual int64 getHashCode() const = 0;
};

#endif   // JUCE_AUDIOTHUMBNAILBASE_H_INCLUDED
