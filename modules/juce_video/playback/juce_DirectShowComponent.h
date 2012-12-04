/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_DIRECTSHOWCOMPONENT_JUCEHEADER__
#define __JUCE_DIRECTSHOWCOMPONENT_JUCEHEADER__

#if JUCE_DIRECTSHOW || DOXYGEN

//==============================================================================
/**
    A window that can play back a DirectShow video.

    @note Controller is not implemented
*/
class JUCE_API  DirectShowComponent  : public Component
{
public:
    //==============================================================================
    /** DirectShow video renderer type.

        See MSDN for adivce about choosing the right renderer.
    */
    enum VideoRendererType
    {
        dshowDefault, /**< VMR7 for Windows XP, EVR for Windows Vista and later */
        dshowVMR7,    /**< Video Mixing Renderer 7 */
        dshowEVR      /**< Enhanced Video Renderer */
    };

    /** Creates a DirectShowComponent, initially blank.

        Use the loadMovie() method to load a video once you've added the
        component to a window, (or put it on the desktop as a heavyweight window).
        Loading a video when the component isn't visible can cause problems, as
        DirectShow needs a window handle to initialise properly.

        @see VideoRendererType
    */
    DirectShowComponent (VideoRendererType type = dshowDefault);

    /** Destructor. */
    ~DirectShowComponent();

    /** Returns true if DirectShow is installed and working on this machine. */
    static bool isDirectShowAvailable();

    //==============================================================================
    /** Tries to load a DirectShow video from a file or URL into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a video when the
        component isn't visible can cause problems, because DirectShow needs a window
        handle to do its stuff.

        @param fileOrURLPath    the file or URL path to open
        @returns true if the video opens successfully
    */
    bool loadMovie (const String& fileOrURLPath);

    /** Tries to load a DirectShow video from a file into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a video when the
        component isn't visible can cause problems, because DirectShow needs a window
        handle to do its stuff.

        @param videoFile    the video file to open
        @returns true if the video opens successfully
    */
    bool loadMovie (const File& videoFile);

    /** Tries to load a DirectShow video from a URL into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a video when the
        component isn't visible can cause problems, because DirectShow needs a window
        handle to do its stuff.

        @param videoURL    the video URL to open
        @returns true if the video opens successfully
    */
    bool loadMovie (const URL& videoURL);

    /** Closes the video, if one is open. */
    void closeMovie();

    /** Returns the file path or URL from which the video file was loaded.
        If there isn't one, this returns an empty string.
    */
    File getCurrentMoviePath() const;

    /** Returns true if there's currently a video open. */
    bool isMovieOpen() const;

    /** Returns the length of the video, in seconds. */
    double getMovieDuration() const;

    /** Returns the video's natural size, in pixels.

        You can use this to resize the component to show the video at its preferred
        scale.

        If no video is loaded, the size returned will be 0 x 0.
    */
    void getMovieNormalSize (int& width, int& height) const;

    /** This will position the component within a given area, keeping its aspect
        ratio correct according to the video's normal size.

        The component will be made as large as it can go within the space, and will
        be aligned according to the justification value if this means there are gaps at
        the top or sides.

        @note Not implemented
    */
    void setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                          const RectanglePlacement& placement);

    /** Starts the video playing. */
    void play();

    /** Stops the video playing. */
    void stop();

    /** Returns true if the video is currently playing. */
    bool isPlaying() const;

    /** Moves the video's position back to the start. */
    void goToStart();

    /** Sets the video's position to a given time. */
    void setPosition (double seconds);

    /** Returns the current play position of the video. */
    double getPosition() const;

    /** Changes the video playback rate.

        A value of 1 is normal speed, greater values play it proportionately faster,
        smaller values play it slower.
    */
    void setSpeed (float newSpeed);

    /** Changes the video's playback volume.

        @param newVolume    the volume in the range 0 (silent) to 1.0 (full)
    */
    void setMovieVolume (float newVolume);

    /** Returns the video's playback volume.

        @returns the volume in the range 0 (silent) to 1.0 (full)
    */
    float getMovieVolume() const;

    /** Tells the video whether it should loop. */
    void setLooping (bool shouldLoop);

    /** Returns true if the video is currently looping.

        @see setLooping
    */
    bool isLooping() const;


    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

private:
    //==============================================================================
    String videoPath;
    bool videoLoaded, looping;

    class DirectShowContext;
    friend class DirectShowContext;
    friend class ScopedPointer <DirectShowContext>;
    ScopedPointer <DirectShowContext> context;

    class DirectShowComponentWatcher;
    friend class DirectShowComponentWatcher;
    friend class ScopedPointer <DirectShowComponentWatcher>;
    ScopedPointer <DirectShowComponentWatcher> componentWatcher;

    //==============================================================================
    void updateContextPosition();
    void showContext (bool shouldBeVisible);
    void recreateNativeWindowAsync();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowComponent)
};

#endif
#endif   // __JUCE_DIRECTSHOWCOMPONENT_JUCEHEADER__
