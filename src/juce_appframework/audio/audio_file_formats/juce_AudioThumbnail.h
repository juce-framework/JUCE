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

#ifndef __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
#define __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__

#include "../../../juce_core/threads/juce_TimeSliceThread.h"
#include "../../../juce_core/io/streams/juce_InputSource.h"
#include "../../../juce_core/io/juce_OutputStream.h"
#include "../../events/juce_ChangeBroadcaster.h"
#include "../../events/juce_Timer.h"
#include "../../gui/graphics/contexts/juce_Graphics.h"
#include "juce_AudioFormatReader.h"
#include "juce_AudioFormatManager.h"

class AudioThumbnailCache;


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

    @see AudioThumbnailCache
*/
class JUCE_API  AudioThumbnail    : public ChangeBroadcaster,
                                    public TimeSliceClient,
                                    private Timer
{
public:
    //==============================================================================
    /** Creates an audio thumbnail.

        @param sourceSamplesPerThumbnailSample  when creating a stored, low-res version
                        of the audio data, this is the scale at which it should be done
        @param formatManagerToUse   the audio format manager that is used to open the file
        @param cacheToUse   an instance of an AudioThumbnailCache - this provides a background
                            thread and storage that is used to by the thumbnail, and the cache
                            object can be shared between multiple thumbnails
    */
    AudioThumbnail (const int sourceSamplesPerThumbnailSample,
                    AudioFormatManager& formatManagerToUse,
                    AudioThumbnailCache& cacheToUse);

    /** Destructor. */
    ~AudioThumbnail();

    //==============================================================================
    /** Specifies the file or stream that contains the audio file.

        For a file, just call
        @code
        setSource (new FileInputSource (file))
        @endcode

        You can pass a zero in here to clear the thumbnail.

        The source that is passed in will be deleted by this object when it is no
        longer needed
    */
    void setSource (InputSource* const newSource);

    /** Reloads the low res thumbnail data from an input stream.

        The thumb will automatically attempt to reload itself from its
        AudioThumbnailCache.
    */
    void loadFrom (InputStream& input);

    /** Saves the low res thumbnail data to an output stream.

        The thumb will automatically attempt to save itself to its
        AudioThumbnailCache after it finishes scanning the wave file.
    */
    void saveTo (OutputStream& output) const;

    //==============================================================================
    /** Returns the number of channels in the file.
    */
    int getNumChannels() const throw();

    /** Returns the length of the audio file.
    */
    double getTotalLength() const throw();

    /** Renders the waveform shape for a channel.

        The waveform will be drawn within  the specified rectangle, where startTime
        and endTime specify the times within the audio file that should be positioned
        at the left and right edges of the rectangle.

        The waveform will be scaled vertically so that a full-volume sample will fill
        the rectangle vertically, but you can also specify an extra vertical scale factor
        with the verticalZoomFactor parameter.
    */
    void drawChannel (Graphics& g,
                      int x, int y, int w, int h,
                      double startTime,
                      double endTime,
                      int channelNum,
                      const float verticalZoomFactor);

    /** Returns true if the low res preview is fully generated.
    */
    bool isFullyLoaded() const throw();

    //==============================================================================
    /** @internal */
    bool useTimeSlice();
    /** @internal */
    void timerCallback();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioFormatManager& formatManagerToUse;
    AudioThumbnailCache& cache;
    InputSource* source;

    CriticalSection readerLock;
    AudioFormatReader* reader;

    MemoryBlock data, cachedLevels;
    int orginalSamplesPerThumbnailSample;

    int numChannelsCached, numSamplesCached;
    double cachedStart, cachedTimePerPixel;
    bool cacheNeedsRefilling;

    void clear();

    AudioFormatReader* createReader() const;

    void generateSection (AudioFormatReader& reader,
                          int64 startSample,
                          int numSamples);

    char* getChannelData (int channel) const;

    void refillCache (const int numSamples,
                      double startTime,
                      const double timePerPixel);

    friend class AudioThumbnailCache;

    // true if it needs more callbacks from the readNextBlockFromAudioFile() method
    bool initialiseFromAudioFile (AudioFormatReader& reader);

    // returns true if more needs to be read
    bool readNextBlockFromAudioFile (AudioFormatReader& reader);
};


#endif   // __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
