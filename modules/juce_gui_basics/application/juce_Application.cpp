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

//==============================================================================
class AppBroadcastCallback  : public ActionListener
{
public:
    AppBroadcastCallback()    { MessageManager::getInstance()->registerBroadcastListener (this); }
    ~AppBroadcastCallback()   { MessageManager::getInstance()->deregisterBroadcastListener (this); }

    void actionListenerCallback (const String& message)
    {
        JUCEApplication* const app = JUCEApplication::getInstance();

        if (app != 0 && message.startsWith (app->getApplicationName() + "/"))
            app->anotherInstanceStarted (message.substring (app->getApplicationName().length() + 1));
    }
};

//==============================================================================
JUCEApplication::JUCEApplication()
    : appReturnValue (0),
      stillInitialising (true)
{
}

JUCEApplication::~JUCEApplication()
{
    if (appLock != nullptr)
    {
        appLock->exit();
        appLock = nullptr;
    }
}

//==============================================================================
bool JUCEApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void JUCEApplication::anotherInstanceStarted (const String&)
{
}

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
void JUCEApplication::unhandledException (const std::exception*,
                                          const String&,
                                          const int)
{
    jassertfalse;
}

void JUCEApplication::sendUnhandledException (const std::exception* const e,
                                              const char* const sourceFile,
                                              const int lineNumber)
{
    if (JUCEApplicationBase::getInstance() != nullptr)
        JUCEApplicationBase::getInstance()->unhandledException (e, sourceFile, lineNumber);
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

//==============================================================================
bool JUCEApplication::initialiseApp (const String& commandLine)
{
    commandLineParameters = commandLine.trim();

   #if ! (JUCE_IOS || JUCE_ANDROID)
    jassert (appLock == nullptr); // initialiseApp must only be called once!

    if (! moreThanOneInstanceAllowed())
    {
        appLock = new InterProcessLock ("juceAppLock_" + getApplicationName());

        if (! appLock->enter(0))
        {
            appLock = nullptr;
            MessageManager::broadcastMessage (getApplicationName() + "/" + commandLineParameters);

            DBG ("Another instance is running - quitting...");
            return false;
        }
    }
   #endif

    // let the app do its setting-up..
    initialise (commandLineParameters);

   #if JUCE_MAC
    juce_initialiseMacMainMenu(); // needs to be called after the app object has created, to get its name
   #endif

   #if ! (JUCE_IOS || JUCE_ANDROID)
    broadcastCallback = new AppBroadcastCallback();
   #endif

    stillInitialising = false;
    return true;
}

int JUCEApplication::shutdownApp()
{
    jassert (JUCEApplicationBase::getInstance() == this);

    broadcastCallback = nullptr;

    JUCE_TRY
    {
        // give the app a chance to clean up..
        shutdown();
    }
    JUCE_CATCH_EXCEPTION

    return getApplicationReturnValue();
}

//==============================================================================
#if ! JUCE_ANDROID
int JUCEApplication::main (const String& commandLine)
{
    ScopedJuceInitialiser_GUI libraryInitialiser;
    jassert (createInstance != nullptr);
    int returnCode = 0;

    {
        const ScopedPointer<JUCEApplication> app (dynamic_cast <JUCEApplication*> (createInstance()));

        jassert (app != nullptr);

        if (! app->initialiseApp (commandLine))
            return 0;

        JUCE_TRY
        {
            // loop until a quit message is received..
            MessageManager::getInstance()->runDispatchLoop();
        }
        JUCE_CATCH_EXCEPTION

        returnCode = app->shutdownApp();
    }

    return returnCode;
}

#if JUCE_IOS
 extern int juce_iOSMain (int argc, const char* argv[]);
#endif

#if ! JUCE_WINDOWS
 extern const char* juce_Argv0;
#endif

#if JUCE_MAC
 extern void initialiseNSApplication();
#endif

int JUCEApplication::main (int argc, const char* argv[])
{
    JUCE_AUTORELEASEPOOL

   #if JUCE_MAC
    initialiseNSApplication();
   #endif

   #if ! JUCE_WINDOWS
    jassert (createInstance != nullptr);
    juce_Argv0 = argv[0];
   #endif

   #if JUCE_IOS
    return juce_iOSMain (argc, argv);
   #else
    String cmd;
    for (int i = 1; i < argc; ++i)
    {
        String arg (argv[i]);
        if (arg.containsChar (' ') && ! arg.isQuotedString())
            arg = arg.quoted ('"');

        cmd << arg << ' ';
    }

    return JUCEApplication::main (cmd);
   #endif
}
#endif
