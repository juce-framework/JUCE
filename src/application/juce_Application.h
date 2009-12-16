/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_APPLICATION_JUCEHEADER__
#define __JUCE_APPLICATION_JUCEHEADER__

#include "juce_ApplicationCommandTarget.h"
#include "../events/juce_ActionListener.h"
#include "../threads/juce_InterProcessLock.h"


//==============================================================================
/**
    An instance of this class is used to specify initialisation and shutdown
    code for the application.

    An application that wants to run in the JUCE framework needs to declare a
    subclass of JUCEApplication and implement its various pure virtual methods.

    It then needs to use the START_JUCE_APPLICATION macro somewhere in a cpp file
    to declare an instance of this class and generate a suitable platform-specific
    main() function.

    e.g. @code
        class MyJUCEApp  : public JUCEApplication
        {
            // NEVER put objects inside a JUCEApplication class - only use pointers to
            // objects, which you must create in the initialise() method.
            MyApplicationWindow* myMainWindow;

        public:
            MyJUCEApp()
                : myMainWindow (0)
            {
                // never create any Juce objects in the constructor - do all your initialisation
                // in the initialise() method.
            }

            ~MyJUCEApp()
            {
                // all your shutdown code must have already been done in the shutdown() method -
                // nothing should happen in this destructor.
            }

            void initialise (const String& commandLine)
            {
                myMainWindow = new MyApplicationWindow();
                myMainWindow->setBounds (100, 100, 400, 500);
                myMainWindow->setVisible (true);
            }

            void shutdown()
            {
                delete myMainWindow;
            }

            const String getApplicationName()
            {
                return T("Super JUCE-o-matic");
            }

            const String getApplicationVersion()
            {
                return T("1.0");
            }
        };

        // this creates wrapper code to actually launch the app properly.
        START_JUCE_APPLICATION (MyJUCEApp)
    @endcode

    Because this object will be created before Juce has properly initialised, you must
    NEVER add any member variable objects that will be automatically constructed. Likewise
    don't put ANY code in the constructor that could call Juce functions. Any objects that
    you want to add to the class must be pointers, which you should instantiate during the
    initialise() method, and delete in the shutdown() method.

    @see MessageManager, DeletedAtShutdown
*/
class JUCE_API  JUCEApplication  : public ApplicationCommandTarget,
                                   private ActionListener
{
protected:
    //==============================================================================
    /** Constructs a JUCE app object.

        If subclasses implement a constructor or destructor, they shouldn't call any
        JUCE code in there - put your startup/shutdown code in initialise() and
        shutdown() instead.
    */
    JUCEApplication();

public:
    /** Destructor.

        If subclasses implement a constructor or destructor, they shouldn't call any
        JUCE code in there - put your startup/shutdown code in initialise() and
        shutdown() instead.
    */
    virtual ~JUCEApplication();

    //==============================================================================
    /** Returns the global instance of the application object being run. */
    static JUCEApplication* getInstance() throw();

    //==============================================================================
    /** Called when the application starts.

        This will be called once to let the application do whatever initialisation
        it needs, create its windows, etc.

        After the method returns, the normal event-dispatch loop will be run,
        until the quit() method is called, at which point the shutdown()
        method will be called to let the application clear up anything it needs
        to delete.

        If during the initialise() method, the application decides not to start-up
        after all, it can just call the quit() method and the event loop won't be run.

        @param commandLineParameters    the line passed in does not include the
                                        name of the executable, just the parameter list.
        @see shutdown, quit
    */
    virtual void initialise (const String& commandLineParameters) = 0;

    /** Returns true if the application hasn't yet completed its initialise() method
        and entered the main event loop.

        This is handy for things like splash screens to know when the app's up-and-running
        properly.
    */
    bool isInitialising() const throw();

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

    //==============================================================================
    /** Returns the application's name.

        An application must implement this to name itself.
    */
    virtual const String getApplicationName() = 0;

    /** Returns the application's version number.

        An application can implement this to give itself a version.
        (The default implementation of this just returns an empty string).
    */
    virtual const String getApplicationVersion();

    /** Checks whether multiple instances of the app are allowed.

        If you application class returns true for this, more than one instance is
        permitted to run (except on the Mac where this isn't possible).

        If it's false, the second instance won't start, but it you will still get a
        callback to anotherInstanceStarted() to tell you about this - which
        gives you a chance to react to what the user was trying to do.
    */
    virtual bool moreThanOneInstanceAllowed();

    /** Indicates that the user has tried to start up another instance of the app.

        This will get called even if moreThanOneInstanceAllowed() is false.
    */
    virtual void anotherInstanceStarted (const String& commandLine);

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
    virtual void systemRequestedQuit();

    /** If any unhandled exceptions make it through to the message dispatch loop, this
        callback will be triggered, in case you want to log them or do some other
        type of error-handling.

        If the type of exception is derived from the std::exception class, the pointer
        passed-in will be valid. If the exception is of unknown type, this pointer
        will be null.
    */
    virtual void unhandledException (const std::exception* e,
                                     const String& sourceFilename,
                                     const int lineNumber);

    //==============================================================================
    /** Signals that the main message loop should stop and the application should terminate.

        This isn't synchronous, it just posts a quit message to the main queue, and
        when this message arrives, the message loop will stop, the shutdown() method
        will be called, and the app will exit.

        Note that this will cause an unconditional quit to happen, so if you need an
        extra level before this, e.g. to give the user the chance to save their work
        and maybe cancel the quit, you'll need to handle this in the systemRequestedQuit()
        method - see that method's help for more info.

        @see MessageManager, DeletedAtShutdown
    */
    static void quit();

    /** Sets the value that should be returned as the application's exit code when the
        app quits.

        This is the value that's returned by the main() function. Normally you'd leave this
        as 0 unless you want to indicate an error code.

        @see getApplicationReturnValue
    */
    void setApplicationReturnValue (const int newReturnValue) throw();

    /** Returns the value that has been set as the application's exit code.
        @see setApplicationReturnValue
    */
    int getApplicationReturnValue() const throw()                   { return appReturnValue; }

    /** Returns the application's command line params.
    */
    const String getCommandLineParameters() const throw()           { return commandLineParameters; }

    //==============================================================================
    // These are used by the START_JUCE_APPLICATION() macro and aren't for public use.

    /** @internal */
    static int main (String& commandLine, JUCEApplication* const newApp);
    /** @internal */
    static int main (int argc, char* argv[], JUCEApplication* const newApp);

    /** @internal */
    static void sendUnhandledException (const std::exception* const e,
                                        const char* const sourceFile,
                                        const int lineNumber);

    //==============================================================================
    /** @internal */
    ApplicationCommandTarget* getNextCommandTarget();
    /** @internal */
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
    /** @internal */
    void getAllCommands (Array <CommandID>& commands);
    /** @internal */
    bool perform (const InvocationInfo& info);
    /** @internal */
    void actionListenerCallback (const String& message);

private:
    //==============================================================================
    String commandLineParameters;
    int appReturnValue;
    bool stillInitialising;
    InterProcessLock* appLock;

    JUCEApplication (const JUCEApplication&);
    const JUCEApplication& operator= (const JUCEApplication&);

public:
    /** @internal */
    bool initialiseApp (String& commandLine);
    /** @internal */
    static int shutdownAppAndClearUp();
};


#endif   // __JUCE_APPLICATION_JUCEHEADER__
