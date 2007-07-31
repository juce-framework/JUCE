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

#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
#define __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__

#include "../../../../juce_core/io/files/juce_File.h"

// this is used to disable QuickTime, and is defined in juce_Config.h
#if JUCE_QUICKTIME || DOXYGEN

#if JUCE_WIN32
  #include "juce_ActiveXControlComponent.h"
  typedef ActiveXControlComponent QTWinBaseClass;
#else
  #include "../juce_Component.h"
  #include "../../../events/juce_Timer.h"
  typedef Component QTWinBaseClass;
#endif

//==============================================================================
/**
    A window that can play back a QuickTime movie.

*/
class JUCE_API  QuickTimeMovieComponent     : public QTWinBaseClass
                                              #if JUCE_MAC
                                               , private Timer
                                              #endif
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

    //==============================================================================
    /** Tries to load a QuickTime movie into the player.

        It's best to call this function once you've added the component to a window,
        (or put it on the desktop as a heavyweight window). Loading a movie when the
        component isn't visible can cause problems, because QuickTime needs a window
        handle to do its stuff.

        @param movieFile    the .mov file to open
        @param isControllerVisible  whether to show a controller bar at the bottom
        @returns true if the movie opens successfully
    */
    bool loadMovie (const File& movieFile,
                    const bool isControllerVisible);

    /** Closes the movie, if one is open. */
    void closeMovie();

    /** Returns the movie file that is currently open.

        If there isn't one, this returns File::nonexistent
    */
    const File getCurrentMovieFile() const;

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
    void setBoundsWithCorrectAspectRatio (const Rectangle& spaceToFitWithin,
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
    void setPosition (const double seconds);

    /** Returns the current play position of the movie. */
    double getPosition() const;

    /** Changes the movie's playback volume.

        @param newVolume    the volume in the range 0 (silent) to 1.0 (full)
    */
    void setMovieVolume (const float newVolume);

    /** Returns the movie's playback volume.

        @returns the volume in the range 0 (silent) to 1.0 (full)
    */
    float getMovieVolume() const;

    /** Tells the movie whether it should loop. */
    void setLooping (const bool shouldLoop);

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
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    void visibilityChanged();
#if JUCE_MAC
    /** @internal */
    void handleMCEvent (void*);
    /** @internal */
    void assignMovieToWindow();
    /** @internal */
    void timerCallback();
    /** @internal */
    void moved();
    /** @internal */
    void resized();
#endif

    juce_UseDebuggingNewOperator

private:
    File movieFile;
    bool movieLoaded, controllerVisible;

    void* internal;

#if JUCE_MAC
    void* associatedWindow;
    Rectangle lastPositionApplied;
    bool controllerAssignedToWindow, reentrant, looping;
    void checkWindowAssociation();
#endif

    void createControlIfNeeded();
    bool isControlCreated() const;

    QuickTimeMovieComponent (const QuickTimeMovieComponent&);
    const QuickTimeMovieComponent& operator= (const QuickTimeMovieComponent&);
};

#endif
#endif   // __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
