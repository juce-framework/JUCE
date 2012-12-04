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

#ifndef __JUCE_APPLICATION_JUCEHEADER__
#define __JUCE_APPLICATION_JUCEHEADER__


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
        public:
            MyJUCEApp()
            {
            }

            ~MyJUCEApp()
            {
            }

            void initialise (const String& commandLine)
            {
                myMainWindow = new MyApplicationWindow();
                myMainWindow->setBounds (100, 100, 400, 500);
                myMainWindow->setVisible (true);
            }

            void shutdown()
            {
                myMainWindow = 0;
            }

            const String getApplicationName()
            {
                return "Super JUCE-o-matic";
            }

            const String getApplicationVersion()
            {
                return "1.0";
            }

        private:
            ScopedPointer <MyApplicationWindow> myMainWindow;
        };

        // this creates wrapper code to actually launch the app properly.
        START_JUCE_APPLICATION (MyJUCEApp)
    @endcode

    @see MessageManager
*/
class JUCE_API  JUCEApplication  : public JUCEApplicationBase,
                                   public ApplicationCommandTarget
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
    static JUCEApplication* getInstance() noexcept          { return dynamic_cast <JUCEApplication*> (JUCEApplicationBase::getInstance()); }

    //==============================================================================
    /** Returns true if the application hasn't yet completed its initialise() method
        and entered the main event loop.

        This is handy for things like splash screens to know when the app's up-and-running
        properly.
    */
    bool isInitialising() const noexcept                    { return stillInitialising; }

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
        permitted to run (except on OSX where the OS automatically stops you launching
        a second instance of an app without explicitly starting it from the command-line).

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

    /** This method is called when the application is being put into background mode
        by the operating system.
    */
    virtual void suspended();

    /** This method is called when the application is being woken from background mode
        by the operating system.
    */
    virtual void resumed();

    /** If any unhandled exceptions make it through to the message dispatch loop, this
        callback will be triggered, in case you want to log them or do some other
        type of error-handling.

        If the type of exception is derived from the std::exception class, the pointer
        passed-in will be valid. If the exception is of unknown type, this pointer
        will be null.
    */
    virtual void unhandledException (const std::exception* e,
                                     const String& sourceFilename,
                                     int lineNumber);

    //==============================================================================
    /** Signals that the main message loop should stop and the application should terminate.

        This isn't synchronous, it just posts a quit message to the main queue, and
        when this message arrives, the message loop will stop, the shutdown() method
        will be called, and the app will exit.

        Note that this will cause an unconditional quit to happen, so if you need an
        extra level before this, e.g. to give the user the chance to save their work
        and maybe cancel the quit, you'll need to handle this in the systemRequestedQuit()
        method - see that method's help for more info.

        @see MessageManager
    */
    static void quit();

    /** Sets the value that should be returned as the application's exit code when the
        app quits.

        This is the value that's returned by the main() function. Normally you'd leave this
        as 0 unless you want to indicate an error code.

        @see getApplicationReturnValue
    */
    void setApplicationReturnValue (int newReturnValue) noexcept;

    /** Returns the value that has been set as the application's exit code.
        @see setApplicationReturnValue
    */
    int getApplicationReturnValue() const noexcept                  { return appReturnValue; }

    /** Returns the application's command line parameters as a set of strings.
        @see getCommandLineParameters
    */
    static StringArray JUCE_CALLTYPE getCommandLineParameterArray();

    /** Returns the application's command line parameters as a single string.
        @see getCommandLineParameterArray
    */
    static String JUCE_CALLTYPE getCommandLineParameters();

    /** Returns true if this executable is running as an app (as opposed to being a plugin
        or other kind of shared library. */
    static inline bool isStandaloneApp() noexcept                   { return createInstance != nullptr; }

    //==============================================================================
    /** @internal */
    ApplicationCommandTarget* getNextCommandTarget();
    /** @internal */
    void getCommandInfo (CommandID, ApplicationCommandInfo&);
    /** @internal */
    void getAllCommands (Array <CommandID>&);
    /** @internal */
    bool perform (const InvocationInfo&);

    //==============================================================================
   #ifndef DOXYGEN
    // The following methods are internal calls - not for public use.
    static int main();
    static int main (int argc, const char* argv[]);
    static void sendUnhandledException (const std::exception*, const char* sourceFile, int lineNumber);
    bool initialiseApp();
    int shutdownApp();

protected:
    bool sendCommandLineToPreexistingInstance();
   #endif

private:
    //==============================================================================
    struct MultipleInstanceHandler;
    friend struct MultipleInstanceHandler;
    friend class ScopedPointer<MultipleInstanceHandler>;
    ScopedPointer<MultipleInstanceHandler> multipleInstanceHandler;

    int appReturnValue;
    bool stillInitialising;

    JUCE_DECLARE_NON_COPYABLE (JUCEApplication)
};


#endif   // __JUCE_APPLICATION_JUCEHEADER__
