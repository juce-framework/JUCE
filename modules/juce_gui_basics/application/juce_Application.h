/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An instance of this class is used to specify initialisation and shutdown
    code for the application.

    Any application that wants to run an event loop must declare a subclass of
    JUCEApplicationBase or JUCEApplication, and implement its various pure virtual
    methods.

    It then needs to use the START_JUCE_APPLICATION macro somewhere in a CPP file
    to declare an instance of this class and generate suitable platform-specific
    boilerplate code to launch the app.

    Note that this class is derived from JUCEApplicationBase, which contains most
    of the useful methods and functionality. This derived class is here simply as
    a convenient way to also inherit from an ApplicationCommandTarget, and to implement
    default versions of some of the pure virtual base class methods. But you can derive
    your app object directly from JUCEApplicationBase if you want to, and by doing so
    can avoid having a dependency on the juce_gui_basics module.

    e.g. @code
        class MyJUCEApp  : public JUCEApplication
        {
        public:
            MyJUCEApp()  {}
            ~MyJUCEApp() {}

            void initialise (const String& commandLine) override
            {
                myMainWindow = new MyApplicationWindow();
                myMainWindow->setBounds (100, 100, 400, 500);
                myMainWindow->setVisible (true);
            }

            void shutdown() override
            {
                myMainWindow = nullptr;
            }

            const String getApplicationName() override
            {
                return "Super JUCE-o-matic";
            }

            const String getApplicationVersion() override
            {
                return "1.0";
            }

        private:
            ScopedPointer<MyApplicationWindow> myMainWindow;
        };

        // this generates boilerplate code to launch our app class:
        START_JUCE_APPLICATION (MyJUCEApp)
    @endcode

    @see JUCEApplicationBase, START_JUCE_APPLICATION
*/
class JUCE_API  JUCEApplication  : public JUCEApplicationBase,
                                   public ApplicationCommandTarget
{
public:
    //==============================================================================
    /** Constructs a JUCE app object.

        If subclasses implement a constructor or destructor, they shouldn't call any
        JUCE code in there - put your startup/shutdown code in initialise() and
        shutdown() instead.
    */
    JUCEApplication();

    /** Destructor.

        If subclasses implement a constructor or destructor, they shouldn't call any
        JUCE code in there - put your startup/shutdown code in initialise() and
        shutdown() instead.
    */
    ~JUCEApplication();

    //==============================================================================
    /** Returns the global instance of the application object being run. */
    static JUCEApplication* JUCE_CALLTYPE getInstance() noexcept;

    //==============================================================================
   #if DOXYGEN
    /** Returns the application's name. */
    virtual const String getApplicationName() = 0;

    /** Returns the application's version number. */
    virtual const String getApplicationVersion() = 0;
   #endif

    /** Checks whether multiple instances of the app are allowed.

        If you application class returns true for this, more than one instance is
        permitted to run (except on OSX where the OS automatically stops you launching
        a second instance of an app without explicitly starting it from the command-line).

        If it's false, the second instance won't start, but it you will still get a
        callback to anotherInstanceStarted() to tell you about this - which
        gives you a chance to react to what the user was trying to do.
    */
    bool moreThanOneInstanceAllowed() override;

    /** Indicates that the user has tried to start up another instance of the app.
        This will get called even if moreThanOneInstanceAllowed() is false.
    */
    void anotherInstanceStarted (const String& commandLine) override;

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
    void systemRequestedQuit() override;

    /** This method is called when the application is being put into background mode
        by the operating system.
    */
    void suspended() override;

    /** This method is called when the application is being woken from background mode
        by the operating system.
    */
    void resumed() override;

    /** If any unhandled exceptions make it through to the message dispatch loop, this
        callback will be triggered, in case you want to log them or do some other
        type of error-handling.

        If the type of exception is derived from the std::exception class, the pointer
        passed-in will be valid. If the exception is of unknown type, this pointer
        will be null.
    */
    void unhandledException (const std::exception* e,
                             const String& sourceFilename,
                             int lineNumber) override;

    //==============================================================================
    /** @internal */
    ApplicationCommandTarget* getNextCommandTarget() override;
    /** @internal */
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    /** @internal */
    void getAllCommands (Array<CommandID>&) override;
    /** @internal */
    bool perform (const InvocationInfo&) override;

private:
    bool initialiseApp() override;

    JUCE_DECLARE_NON_COPYABLE (JUCEApplication)
};

} // namespace juce
