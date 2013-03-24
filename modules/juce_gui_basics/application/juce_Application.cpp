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

#if JUCE_MAC
 extern void juce_initialiseMacMainMenu();
#endif

#if ! (JUCE_IOS || JUCE_ANDROID)
 #define JUCE_HANDLE_MULTIPLE_INSTANCES 1
#endif

//==============================================================================
#if JUCE_HANDLE_MULTIPLE_INSTANCES
struct JUCEApplication::MultipleInstanceHandler  : public ActionListener
{
public:
    MultipleInstanceHandler (const String& appName)
        : appLock ("juceAppLock_" + appName)
    {
    }

    bool sendCommandLineToPreexistingInstance()
    {
        if (appLock.enter (0))
            return false;

        JUCEApplication* const app = JUCEApplication::getInstance();
        jassert (app != nullptr);

        MessageManager::broadcastMessage (app->getApplicationName()
                                            + "/" + app->getCommandLineParameters());
        return true;
    }

    void actionListenerCallback (const String& message)
    {
        if (JUCEApplication* const app = JUCEApplication::getInstance())
        {
            const String appName (app->getApplicationName());

            if (message.startsWith (appName + "/"))
                app->anotherInstanceStarted (message.substring (appName.length() + 1));
        }
    }

private:
    InterProcessLock appLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultipleInstanceHandler)
};
#else
struct JUCEApplication::MultipleInstanceHandler {};
#endif

//==============================================================================
JUCEApplication::JUCEApplication()
    : appReturnValue (0),
      stillInitialising (true)
{
}

JUCEApplication::~JUCEApplication()
{
}

//==============================================================================
bool JUCEApplication::moreThanOneInstanceAllowed()  { return true; }
void JUCEApplication::anotherInstanceStarted (const String&) {}

void JUCEApplication::suspended() {}
void JUCEApplication::resumed() {}

void JUCEApplication::systemRequestedQuit()
{
    quit();
}

void JUCEApplication::quit()
{
    MessageManager::getInstance()->stopDispatchLoop();
}

void JUCEApplication::setApplicationReturnValue (const int newReturnValue) noexcept
{
    appReturnValue = newReturnValue;
}

//==============================================================================
void JUCEApplication::unhandledException (const std::exception*, const String&, int)
{
    jassertfalse;
}

void JUCEApplication::sendUnhandledException (const std::exception* const e,
                                              const char* const sourceFile,
                                              const int lineNumber)
{
    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->unhandledException (e, sourceFile, lineNumber);
}

//==============================================================================
ApplicationCommandTarget* JUCEApplication::getNextCommandTarget()
{
    return nullptr;
}

void JUCEApplication::getAllCommands (Array <CommandID>& commands)
{
    commands.add (StandardApplicationCommandIDs::quit);
}

void JUCEApplication::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    if (commandID == StandardApplicationCommandIDs::quit)
    {
        result.setInfo (TRANS("Quit"),
                        TRANS("Quits the application"),
                        "Application",
                        0);

        result.defaultKeypresses.add (KeyPress ('q', ModifierKeys::commandModifier, 0));
    }
}

bool JUCEApplication::perform (const InvocationInfo& info)
{
    if (info.commandID == StandardApplicationCommandIDs::quit)
    {
        systemRequestedQuit();
        return true;
    }

    return false;
}

#if JUCE_HANDLE_MULTIPLE_INSTANCES
bool JUCEApplication::sendCommandLineToPreexistingInstance()
{
    jassert (multipleInstanceHandler == nullptr); // this must only be called once!

    multipleInstanceHandler = new MultipleInstanceHandler (getApplicationName());
    return multipleInstanceHandler->sendCommandLineToPreexistingInstance();
}
#endif

//==============================================================================
bool JUCEApplication::initialiseApp()
{
   #if JUCE_HANDLE_MULTIPLE_INSTANCES
    if ((! moreThanOneInstanceAllowed()) && sendCommandLineToPreexistingInstance())
    {
        DBG ("Another instance is running - quitting...");
        return false;
    }
   #endif

    // let the app do its setting-up..
    initialise (getCommandLineParameters());

    stillInitialising = false;

    if (! MessageManager::getInstance()->hasStopMessageBeenSent())
    {
       #if JUCE_MAC
        juce_initialiseMacMainMenu(); // (needs to get the app's name)
       #endif

       #if JUCE_HANDLE_MULTIPLE_INSTANCES
        if (multipleInstanceHandler != nullptr)
            MessageManager::getInstance()->registerBroadcastListener (multipleInstanceHandler);
       #endif
    }

    return true;
}

int JUCEApplication::shutdownApp()
{
    jassert (JUCEApplicationBase::getInstance() == this);

   #if JUCE_HANDLE_MULTIPLE_INSTANCES
    if (multipleInstanceHandler != nullptr)
        MessageManager::getInstance()->deregisterBroadcastListener (multipleInstanceHandler);
   #endif

    JUCE_TRY
    {
        // give the app a chance to clean up..
        shutdown();
    }
    JUCE_CATCH_EXCEPTION

    multipleInstanceHandler = nullptr;
    return getApplicationReturnValue();
}

//==============================================================================
#if JUCE_ANDROID

StringArray JUCEApplication::getCommandLineParameterArray() { return StringArray(); }
String JUCEApplication::getCommandLineParameters()          { return String::empty; }

#else

int JUCEApplication::main()
{
    ScopedJuceInitialiser_GUI libraryInitialiser;
    jassert (createInstance != nullptr);

    const ScopedPointer<JUCEApplication> app (dynamic_cast <JUCEApplication*> (createInstance()));
    jassert (app != nullptr);

    if (! app->initialiseApp())
        return 0;

    JUCE_TRY
    {
        // loop until a quit message is received..
        MessageManager::getInstance()->runDispatchLoop();
    }
    JUCE_CATCH_EXCEPTION

    return app->shutdownApp();
}

#if ! JUCE_WINDOWS
#if JUCE_IOS
 extern int juce_iOSMain (int argc, const char* argv[]);
#endif

#if JUCE_MAC
 extern void initialiseNSApplication();
#endif

extern const char** juce_argv;  // declared in juce_core
extern int juce_argc;

StringArray JUCEApplication::getCommandLineParameterArray()
{
    return StringArray (juce_argv + 1, juce_argc - 1);
}

String JUCEApplication::getCommandLineParameters()
{
    String argString;

    for (int i = 1; i < juce_argc; ++i)
    {
        String arg (juce_argv[i]);

        if (arg.containsChar (' ') && ! arg.isQuotedString())
            arg = arg.quoted ('"');

        argString << arg << ' ';
    }

    return argString.trim();
}

int JUCEApplication::main (int argc, const char* argv[])
{
    JUCE_AUTORELEASEPOOL
    {
        juce_argc = argc;
        juce_argv = argv;

       #if JUCE_MAC
        initialiseNSApplication();
       #endif

       #if JUCE_IOS
        return juce_iOSMain (argc, argv);
       #else
        return JUCEApplication::main();
       #endif
    }
}
#endif

#endif
