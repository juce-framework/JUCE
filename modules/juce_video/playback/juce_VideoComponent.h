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

#ifndef JUCE_VIDEOCOMPONENT_H_INCLUDED
#define JUCE_VIDEOCOMPONENT_H_INCLUDED

namespace juce
{

//==============================================================================
/**
    A component that can play a movie.

    Use the load() method to open a video once you've added this component to
    a parent (or put it on the desktop).

    @tags{Video}
*/
class JUCE_API  VideoComponent  : public Component,
                                  private Timer
{
public:
    //==============================================================================
    /** Creates an empty VideoComponent.

        Use the load() method to open a video once you've added this component to
        a parent (or put it on the desktop).
    */
    VideoComponent();

    /** Destructor. */
    ~VideoComponent();

    //==============================================================================
    /** Tries to load a video from a local file.
        @returns an error if the file failed to be loaded correctly
    */
    Result load (const File& file);

    /** Tries to load a video from a URL.
        @returns an error if the file failed to be loaded correctly
    */
    Result load (const URL& url);

    /** Closes the video and resets the component. */
    void closeVideo();

    /** Returns true if a video is currently open. */
    bool isVideoOpen() const;

    /** Returns the last file that was loaded.
        If nothing is open, or if it was a URL rather than a file, this will return File().
    */
    File getCurrentVideoFile() const;

    /** Returns the last URL that was loaded.
        If nothing is open, or if it was a file rather than a URL, this will return URL().
    */
    URL getCurrentVideoURL() const;

    //==============================================================================
    /** Returns the length of the video, in seconds. */
    double getVideoDuration() const;

    /** Returns the video's natural size, in pixels.
        If no video is loaded, an empty rectangle will be returned.
    */
    Rectangle<int> getVideoNativeSize() const;

    /** Starts the video playing. */
    void play();

    /** Stops the video playing. */
    void stop();

    /** Returns true if the video is currently playing. */
    bool isPlaying() const;

    /** Sets the video's position to a given time. */
    void setPlayPosition (double newPositionSeconds);

    /** Returns the current play position of the video. */
    double getPlayPosition() const;

    /** Changes the video playback rate.
        A value of 1.0 is normal speed, greater values will play faster, smaller
        values play more slowly.
    */
    void setPlaySpeed (double newSpeed);

    /** Changes the video's playback volume.
        @param newVolume    the volume in the range 0 (silent) to 1.0 (full)
    */
    void setAudioVolume (float newVolume);

    /** Returns the video's playback volume.
        @returns the volume in the range 0 (silent) to 1.0 (full)
    */
    float getAudioVolume() const;

private:
    //==============================================================================
    struct Pimpl;
    friend struct Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    std::unique_ptr<Pimpl> pimpl;

    void resized() override;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoComponent)
};


#endif

} // namespace juce
