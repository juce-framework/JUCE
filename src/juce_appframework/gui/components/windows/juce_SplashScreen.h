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

#ifndef __JUCE_SPLASHSCREEN_JUCEHEADER__
#define __JUCE_SPLASHSCREEN_JUCEHEADER__

#include "../juce_Component.h"
#include "../../../events/juce_Timer.h"
#include "../../../application/juce_DeletedAtShutdown.h"


//==============================================================================
/** A component for showing a splash screen while your app starts up.

    This will automatically position itself, and delete itself when the app has
    finished initialising (it uses the JUCEApplication::isInitialising() to detect
    this).

    To use it, just create one of these in your JUCEApplication::initialise() method,
    call its show() method and let the object delete itself later.

    E.g. @code

    void MyApp::initialise (const String& commandLine)
    {
        SplashScreen* splash = new SplashScreen();

        splash->show (T("welcome to my app"),
                      ImageCache::getFromFile (File ("/foobar/splash.jpg")),
                      4000, false);

        .. no need to delete the splash screen - it'll do that itself.
    }

    @endcode
*/
class JUCE_API  SplashScreen  : public Component,
                                public Timer,
                                private DeletedAtShutdown
{
public:
    //==============================================================================
    /** Creates a SplashScreen object.

        After creating one of these (or your subclass of it), call one of the show()
        methods to display it.
    */
    SplashScreen();

    /** Destructor. */
    ~SplashScreen();

    //==============================================================================
    /** Creates a SplashScreen object that will display an image.

        As soon as this is called, the SplashScreen will be displayed in the centre of the
        screen. This method will also dispatch any pending messages to make sure that when
        it returns, the splash screen has been completely drawn, and your initialisation
        code can carry on.

        @param title            the name to give the component
        @param backgroundImage  an image to draw on the component. The component's size
                                will be set to the size of this image, and if the image is
                                semi-transparent, the component will be made semi-transparent
                                too. This image will be deleted (or released from the ImageCache
                                if that's how it was created) by the splash screen object when
                                it is itself deleted.
        @param minimumTimeToDisplayFor    how long (in milliseconds) the splash screen
                                should stay visible for. If the initialisation takes longer than
                                this time, the splash screen will wait for it to finish before
                                disappearing, but if initialisation is very quick, this lets
                                you make sure that people get a good look at your splash.
        @param useDropShadow    if true, the window will have a drop shadow
    */
    void show (const String& title,
               Image* const backgroundImage,
               const int minimumTimeToDisplayFor,
               const bool useDropShadow);

    /** Creates a SplashScreen object with a specified size.

        For a custom splash screen, you can use this method to display it at a certain size
        and then override the paint() method yourself to do whatever's necessary.

        As soon as this is called, the SplashScreen will be displayed in the centre of the
        screen. This method will also dispatch any pending messages to make sure that when
        it returns, the splash screen has been completely drawn, and your initialisation
        code can carry on.

        @param title            the name to give the component
        @param width            the width to use
        @param height           the height to use
        @param minimumTimeToDisplayFor    how long (in milliseconds) the splash screen
                                should stay visible for. If the initialisation takes longer than
                                this time, the splash screen will wait for it to finish before
                                disappearing, but if initialisation is very quick, this lets
                                you make sure that people get a good look at your splash.
        @param useDropShadow    if true, the window will have a drop shadow
    */
    void show (const String& title,
               const int width,
               const int height,
               const int minimumTimeToDisplayFor,
               const bool useDropShadow);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void timerCallback();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Image* backgroundImage;
    Time earliestTimeToDelete;
    bool isImageInCache;

    SplashScreen (const SplashScreen&);
    const SplashScreen& operator= (const SplashScreen&);
};


#endif   // __JUCE_SPLASHSCREEN_JUCEHEADER__
