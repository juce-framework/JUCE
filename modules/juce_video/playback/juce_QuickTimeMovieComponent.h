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

#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
#define __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__

// (NB: This stuff mustn't go inside the "#if QUICKTIME" block, or it'll break the
// amalgamated build)
#ifndef DOXYGEN
 #if JUCE_WINDOWS
  typedef ActiveXControlComponent QTCompBaseClass;
 #elif JUCE_MAC
  typedef NSViewComponent QTCompBaseClass;
 #endif
#endif

#if JUCE_QUICKTIME || DOXYGEN

//==============================================================================
/**
    A window that can play back a QuickTime movie.

*/
class JUCE_API  QuickTimeMovieComponent     : public QTCompBaseClass
{
public:
    //==============================================================================
    /** Creates a QuickTimeMovieComponent, initially blank.

        Use the loadMovie() method to load a movie once you've added the
        component to a window, (or put it on the desktop as a heavyweight window).
        Loading a movie when the component isn't visible can cause problems, as
        QuickTime needs a window handle to initialise properly.
    */
    QuickTimeMovieComponent();

    /** Destructor. */
    ~QuickTimeMovieComponent();

    /** Returns true if QT is installed and working on this machine.
    */
    static bool isQuickTimeAvailable() noexcept;

    //==============================================================================
    /** Tries to load a QuickTime movie from a file into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a movie when the
        component isn't visible can cause problems, because QuickTime needs a window
        handle to do its stuff.

        @param movieFile    the .mov file to open
        @param isControllerVisible  whether to show a controller bar at the bottom
        @returns true if the movie opens successfully
    */
    bool loadMovie (const File& movieFile,
                    bool isControllerVisible);

    /** Tries to load a QuickTime movie from a URL into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a movie when the
        component isn't visible can cause problems, because QuickTime needs a window
        handle to do its stuff.

        @param movieURL    the .mov file to open
        @param isControllerVisible  whether to show a controller bar at the bottom
        @returns true if the movie opens successfully
    */
    bool loadMovie (const URL& movieURL,
                    bool isControllerVisible);

    /** Tries to load a QuickTime movie from a stream into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a movie when the
        component isn't visible can cause problems, because QuickTime needs a window
        handle to do its stuff.

        @param movieStream    a stream containing a .mov file. The component may try
                              to read the whole stream before playing, rather than
                              streaming from it.
        @param isControllerVisible  whether to show a controller bar at the bottom
        @returns true if the movie opens successfully
    */
    bool loadMovie (InputStream* movieStream,
                    bool isControllerVisible);

    /** Closes the movie, if one is open. */
    void closeMovie();

    /** Returns the movie file that is currently open.

        If there isn't one, this returns File::nonexistent
    */
    File getCurrentMovieFile() const;

    /** Returns true if there's currently a movie open. */
    bool isMovieOpen() const;

    /** Returns the length of the movie, in seconds. */
    double getMovieDuration() const;

    /** Returns the movie's natural size, in pixels.

        You can use this to resize the component to show the movie at its preferred
        scale.

        If no movie is loaded, the size returned will be 0 x 0.
    */
    void getMovieNormalSize (int& width, int& height) const;

    /** This will position the component within a given area, keeping its aspect
        ratio correct according to the movie's normal size.

        The component will be made as large as it can go within the space, and will
        be aligned according to the justification value if this means there are gaps at
        the top or sides.
    */
    void setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                          const RectanglePlacement& placement);

    /** Starts the movie playing. */
    void play();

    /** Stops the movie playing. */
    void stop();

    /** Returns true if the movie is currently playing. */
    bool isPlaying() const;

    /** Moves the movie's position back to the start. */
    void goToStart();

    /** Sets the movie's position to a given time. */
    void setPosition (double seconds);

    /** Returns the current play position of the movie. */
    double getPosition() const;

    /** Changes the movie playback rate.

        A value of 1 is normal speed, greater values play it proportionately faster,
        smaller values play it slower.
    */
    void setSpeed (float newSpeed);

    /** Changes the movie's playback volume.

        @param newVolume    the volume in the range 0 (silent) to 1.0 (full)
    */
    void setMovieVolume (float newVolume);

    /** Returns the movie's playback volume.

        @returns the volume in the range 0 (silent) to 1.0 (full)
    */
    float getMovieVolume() const;

    /** Tells the movie whether it should loop. */
    void setLooping (bool shouldLoop);

    /** Returns true if the movie is currently looping.

        @see setLooping
    */
    bool isLooping() const;

    /** True if the native QuickTime controller bar is shown in the window.

        @see loadMovie
    */
    bool isControllerVisible() const;


    //==============================================================================
    /** @internal */
    void paint (Graphics& g);


private:
    //==============================================================================
    File movieFile;
    bool movieLoaded, controllerVisible, looping;

#if JUCE_WINDOWS
    void parentHierarchyChanged();
    void visibilityChanged();

    void createControlIfNeeded();
    bool isControlCreated() const;

    class Pimpl;
    friend class ScopedPointer <Pimpl>;
    ScopedPointer <Pimpl> pimpl;
#else
    void* movie;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuickTimeMovieComponent)
};

#endif
#endif   // __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
