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

#ifndef JUCE_SPLASHSCREEN_H_INCLUDED
#define JUCE_SPLASHSCREEN_H_INCLUDED


//==============================================================================
/** A component for showing a splash screen while your app starts up.

    This will automatically position itself, and can be told to delete itself after
    being on-screen for a minimum length of time.

    To use it, just create one of these in your JUCEApplicationBase::initialise() method,
    and when your initialisation tasks have finished running, call its deleteAfterDelay()
    method to make it automatically get rid of itself.

    Note that although you could call deleteAfterDelay() as soon as you create the
    SplashScreen object, if you've got a long initialisation procedure, you probably
    don't want the splash to time-out and disappear before the initialisation has
    finished, which is why it makes sense to not call this method until the end of
    your init tasks.

    E.g. @code

    void MyApp::initialise (const String& commandLine)
    {
        splash = new SplashScreen ("Welcome to my app!",
                                   ImageFileFormat::loadFrom (File ("/foobar/splash.jpg")),
                                   true);

        // now kick off your initialisation work on some kind of thread or task, and
        launchBackgroundInitialisationThread();
    }

    void MyApp::myInitialisationWorkFinished()
    {
        // ..assuming this is some kind of callback method that is triggered when
        // your background initialisation threads have finished, and it's time to open
        // your main window, etc..

        splash->deleteAfterDelay (RelativeTime::seconds (4), false);

        ...etc...
    }

    @endcode
*/
class JUCE_API  SplashScreen  : public Component,
                                private Timer,
                                private DeletedAtShutdown
{
public:
    //==============================================================================
    /** Creates a SplashScreen object.

        When called, the constructor will position the SplashScreen in the centre of the
        display, and after the time specified, it will automatically delete itself.

        Bear in mind that if you call this during your JUCEApplicationBase::initialise()
        method and then block the message thread by performing some kind of task, then
        obviously neither your splash screen nor any other GUI will appear until you
        allow the message thread to resume and do its work. So if you have time-consuming
        tasks to do during startup, use a background thread for them.

        After creating one of these (or your subclass of it), you should do your app's
        initialisation work, and then call the deleteAfterDelay() method to tell this object
        to delete itself after the user has had chance to get a good look at it.

        If you're writing a custom splash screen class, there's another protected constructor
        that your subclass can call, which doesn't take an image.

        @param title            the name to give the component
        @param backgroundImage  an image to draw on the component. The component's size
                                will be set to the size of this image, and if the image is
                                semi-transparent, the component will be made non-opaque
        @param useDropShadow    if true, the window will have a drop shadow

    */
    SplashScreen (const String& title,
                  const Image& backgroundImage,
                  bool useDropShadow);

    /** Destructor. */
    ~SplashScreen();

    /** Tells the component to auto-delete itself after a timeout period, or when the
        mouse is clicked.

        You should call this after finishing your app's initialisation work.

        Note that although you could call deleteAfterDelay() as soon as you create the
        SplashScreen object, if you've got a long initialisation procedure, you probably
        don't want the splash to time-out and disappear before your initialisation has
        finished, which is why it makes sense to not call this method and start the
        self-delete timer until you're ready.

        It's safe to call this method from a non-GUI thread as long as there's no danger that
        the object may be being deleted at the same time.

        @param minimumTotalTimeToDisplayFor    how long the splash screen should stay visible for.
                                Note that this time is measured from the construction-time of this
                                object, not from the time that the deleteAfterDelay() method is
                                called, so if you call this method after a long initialisation
                                period, it may be deleted without any further delay.
        @param removeOnMouseClick   if true, the window will be deleted as soon as the user clicks
                                the mouse (anywhere)
    */
    void deleteAfterDelay (RelativeTime minimumTotalTimeToDisplayFor,
                           bool removeOnMouseClick);

protected:
    //==============================================================================
    /** This constructor is for use by custom sub-classes that don't want to provide an image. */
    SplashScreen (const String& title, int width, int height, bool useDropShadow);

    /** @internal */
    void paint (Graphics&) override;

private:
    //==============================================================================
    Image backgroundImage;
    Time creationTime;
    RelativeTime minimumVisibleTime;
    int clickCountToDelete;

    void timerCallback() override;
    void makeVisible (int w, int h, bool shadow);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplashScreen)
};


#endif   // JUCE_SPLASHSCREEN_H_INCLUDED
