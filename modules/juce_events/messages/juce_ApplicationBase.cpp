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

JUCEApplicationBase::CreateInstanceFunction JUCEApplicationBase::createInstance = 0;
JUCEApplicationBase* JUCEApplicationBase::appInstance = nullptr;

JUCEApplicationBase::JUCEApplicationBase()
    : appReturnValue (0),
      stillInitialising (true)
{
    jassert (isStandaloneApp() && appInstance == nullptr);
    appInstance = this;
}

JUCEApplicationBase::~JUCEApplicationBase()
{
    jassert (appInstance == this);
    appInstance = nullptr;
}

void JUCEApplicationBase::setApplicationReturnValue (const int newReturnValue) noexcept
{
    appReturnValue = newReturnValue;
}

// This is called on the Mac and iOS where the OS doesn't allow the stack to unwind on shutdown..
void JUCEApplicationBase::appWillTerminateByForce()
{
    JUCE_AUTORELEASEPOOL
    {
        {
            const ScopedPointer<JUCEApplicationBase> app (appInstance);

            if (app != nullptr)
                app->shutdownApp();
        }

        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }
}

void JUCEApplicationBase::quit()
{
    MessageManager::getInstance()->stopDispatchLoop();
}

void JUCEApplicationBase::sendUnhandledException (const std::exception* const e,
                                                  const char* const sourceFile,
                                                  const int lineNumber)
{
    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->unhandledException (e, sourceFile, lineNumber);
}

//==============================================================================
#if ! (JUCE_IOS || JUCE_ANDROID)
 #define JUCE_HANDLE_MULTIPLE_INSTANCES 1
#endif

#if JUCE_HANDLE_MULTIPLE_INSTANCES
struct JUCEApplicationBase::MultipleInstanceHandler  : public ActionListener
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

        JUCEApplicationBase* const app = JUCEApplicationBase::getInstance();
        jassert (app != nullptr);

        MessageManager::broadcastMessage (app->getApplicationName()
                                            + "/" + app->getCommandLineParameters());
        return true;
    }

    void actionListenerCallback (const String& message) override
    {
        if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
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

bool JUCEApplicationBase::sendCommandLineToPreexistingInstance()
{
    jassert (multipleInstanceHandler == nullptr); // this must only be called once!

    multipleInstanceHandler = new MultipleInstanceHandler (getApplicationName());
    return multipleInstanceHandler->sendCommandLineToPreexistingInstance();
}

#else
struct JUCEApplicationBase::MultipleInstanceHandler {};
#endif

//==============================================================================
#if JUCE_ANDROID

StringArray JUCEApplicationBase::getCommandLineParameterArray() { return StringArray(); }
String JUCEApplicationBase::getCommandLineParameters()          { return String::empty; }

#else

#if JUCE_WINDOWS

String JUCE_CALLTYPE JUCEApplicationBase::getCommandLineParameters()
{
    return CharacterFunctions::findEndOfToken (CharPointer_UTF16 (GetCommandLineW()),
                                               CharPointer_UTF16 (L" "),
                                               CharPointer_UTF16 (L"\"")).findEndOfWhitespace();
}

StringArray JUCE_CALLTYPE JUCEApplicationBase::getCommandLineParameterArray()
{
    StringArray s;

    int argc = 0;
    if (LPWSTR* const argv = CommandLineToArgvW (GetCommandLineW(), &argc))
    {
        s = StringArray (argv + 1, argc - 1);
        LocalFree (argv);
    }

    return s;
}

#else

#if JUCE_IOS
 extern int juce_iOSMain (int argc, const char* argv[]);
#endif

#if JUCE_MAC
 extern void initialiseNSApplication();
#endif

extern const char* const* juce_argv;  // declared in juce_core
extern int juce_argc;

String JUCEApplicationBase::getCommandLineParameters()
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

StringArray JUCEApplicationBase::getCommandLineParameterArray()
{
    return StringArray (juce_argv + 1, juce_argc - 1);
}

int JUCEApplicationBase::main (int argc, const char* argv[])
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
        return JUCEApplicationBase::main();
       #endif
    }
}

#endif

//==============================================================================
int JUCEApplicationBase::main()
{
    ScopedJuceInitialiser_GUI libraryInitialiser;
    jassert (createInstance != nullptr);

    const ScopedPointer<JUCEApplicationBase> app (createInstance());
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

#endif

//==============================================================================
bool JUCEApplicationBase::initialiseApp()
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

    if (MessageManager::getInstance()->hasStopMessageBeenSent())
        return false;

   #if JUCE_HANDLE_MULTIPLE_INSTANCES
    if (multipleInstanceHandler != nullptr)
        MessageManager::getInstance()->registerBroadcastListener (multipleInstanceHandler);
   #endif

    return true;
}

int JUCEApplicationBase::shutdownApp()
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
