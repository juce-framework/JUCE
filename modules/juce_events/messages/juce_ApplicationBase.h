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

#ifndef __JUCE_APPLICATIONBASE_JUCEHEADER__
#define __JUCE_APPLICATIONBASE_JUCEHEADER__


//==============================================================================
/**
    Abstract base class for application classes.

    This class shouldn't be used directly - you'll normally use JUCEApplication as
    the base for your app, and that inherits from this, adding some more functionality
    to it.

    @see JUCEApplication
*/
class JUCE_API  JUCEApplicationBase
{
protected:
    //==============================================================================
    JUCEApplicationBase();

public:
    /** Destructor. */
    virtual ~JUCEApplicationBase();

    //==============================================================================
    /** Returns the global instance of the application object that's running. */
    static JUCEApplicationBase* getInstance() noexcept          { return appInstance; }

    //==============================================================================
    /** Returns the application's name.
        An application must implement this to name itself.
    */
    virtual const String getApplicationName() = 0;

    /** Returns the application's version number.
    */
    virtual const String getApplicationVersion() = 0;

    /** Checks whether multiple instances of the app are allowed.

        If you application class returns true for this, more than one instance is
        permitted to run (except on the Mac where this isn't possible).

        If it's false, the second instance won't start, but it you will still get a
        callback to anotherInstanceStarted() to tell you about this - which
        gives you a chance to react to what the user was trying to do.
    */
    virtual bool moreThanOneInstanceAllowed() = 0;

    /** Called when the application starts.

        This will be called once to let the application do whatever initialisation
        it needs, create its windows, etc.

        After the method returns, the normal event-dispatch loop will be run,
        until the quit() method is called, at which point the shutdown()
        method will be called to let the application clear up anything it needs
        to delete.

        If during the initialise() method, the application decides not to start-up
        after all, it can just call the quit() method and the event loop won't be run.

        @param commandLineParameters    the line passed in does not include the name of
                                        the executable, just the parameter list. To get the
                                        parameters as an array, you can call
                                        JUCEApplication::getCommandLineParameters()
        @see shutdown, quit
    */
    virtual void initialise (const String& commandLineParameters) = 0;

    /* Called to allow the application to clear up before exiting.

       After JUCEApplication::quit() has been called, the event-dispatch loop will
       terminate, and this method will get called to allow the app to sort itself
       out.

       Be careful that nothing happens in this method that might rely on messages
       being sent, or any kind of window activity, because the message loop is no
       longer running at this point.

        @see DeletedAtShutdown
    */
    virtual void shutdown() = 0;

    /** Indicates that the user has tried to start up another instance of the app.

        This will get called even if moreThanOneInstanceAllowed() is false.
    */
    virtual void anotherInstanceStarted (const String& commandLine) = 0;

    /** Called when the operating system is trying to close the application.

        The default implementation of this method is to call quit(), but it may
        be overloaded to ignore the request or do some other special behaviour
        instead. For example, you might want to offer the user the chance to save
        their changes before quitting, and give them the chance to cancel.

        If you want to send a quit signal to your app, this is the correct method
        to call, because it means that requests that come from the system get handled
        in the same way as those from your own application code. So e.g. you'd
        call this method from a "quit" item on a menu bar.
    */
    virtual void systemRequestedQuit() = 0;

    /** This method is called when the application is being put into background mode
        by the operating system.
    */
    virtual void suspended() = 0;

    /** This method is called when the application is being woken from background mode
        by the operating system.
    */
    virtual void resumed() = 0;

    /** If any unhandled exceptions make it through to the message dispatch loop, this
        callback will be triggered, in case you want to log them or do some other
        type of error-handling.

        If the type of exception is derived from the std::exception class, the pointer
        passed-in will be valid. If the exception is of unknown type, this pointer
        will be null.
    */
    virtual void unhandledException (const std::exception*,
                                     const String& sourceFilename,
                                     int lineNumber) = 0;

    //==============================================================================
    /** Returns true if this executable is running as an app (as opposed to being a plugin
        or other kind of shared library. */
    static inline bool isStandaloneApp() noexcept                   { return createInstance != nullptr; }

    //==============================================================================
   #ifndef DOXYGEN
    static void appWillTerminateByForce();
    typedef JUCEApplicationBase* (*CreateInstanceFunction)();
    static CreateInstanceFunction createInstance;

protected:
    virtual int shutdownApp() = 0;
   #endif

private:
    //==============================================================================
    static JUCEApplicationBase* appInstance;

    JUCE_DECLARE_NON_COPYABLE (JUCEApplicationBase)
};


#endif   // __JUCE_APPLICATIONBASE_JUCEHEADER__
