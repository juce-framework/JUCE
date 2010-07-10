/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Application.h"
#include "../utilities/juce_DeletedAtShutdown.h"
#include "../events/juce_MessageManager.h"
#include "../core/juce_Time.h"
#include "../core/juce_Initialisation.h"
#include "../threads/juce_Process.h"
#include "../core/juce_PlatformUtilities.h"
#include "../text/juce_LocalisedStrings.h"


//==============================================================================
JUCEApplication::JUCEApplication()
    : appReturnValue (0),
      stillInitialising (true)
{
    jassert (appInstance == 0);
    appInstance = this;
}

JUCEApplication::~JUCEApplication()
{
    if (appLock != 0)
    {
        appLock->exit();
        appLock = 0;
    }

    jassert (appInstance == this);
    appInstance = 0;
}

JUCEApplication* JUCEApplication::appInstance = 0;

JUCEApplication* JUCEApplication::getInstance() throw()
{
    return appInstance;
}

bool JUCEApplication::isInitialising() const throw()
{
    return stillInitialising;
}

//==============================================================================
const String JUCEApplication::getApplicationVersion()
{
    return String::empty;
}

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

void JUCEApplication::setApplicationReturnValue (const int newReturnValue) throw()
{
    appReturnValue = newReturnValue;
}

void JUCEApplication::actionListenerCallback (const String& message)
{
    if (message.startsWith (getApplicationName() + "/"))
        anotherInstanceStarted (message.substring (getApplicationName().length() + 1));
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
    if (appInstance != 0)
        appInstance->unhandledException (e, sourceFile, lineNumber);
}

//==============================================================================
ApplicationCommandTarget* JUCEApplication::getNextCommandTarget()
{
    return 0;
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

#if ! JUCE_IOS
    jassert (appLock == 0); // initialiseApp must only be called once!

    if (! moreThanOneInstanceAllowed())
    {
        appLock = new InterProcessLock ("juceAppLock_" + getApplicationName());

        if (! appLock->enter(0))
        {
            appLock = 0;
            MessageManager::broadcastMessage (getApplicationName() + "/" + commandLineParameters);

            DBG ("Another instance is running - quitting...");
            return false;
        }
    }
#endif

    // let the app do its setting-up..
    initialise (commandLineParameters);

    // register for broadcast new app messages
    MessageManager::getInstance()->registerBroadcastListener (this);

    stillInitialising = false;
    return true;
}

int JUCEApplication::shutdownApp()
{
    jassert (appInstance == this);

    MessageManager::getInstance()->deregisterBroadcastListener (this);

    JUCE_TRY
    {
        // give the app a chance to clean up..
        shutdown();
    }
    JUCE_CATCH_EXCEPTION

    return getApplicationReturnValue();
}

//==============================================================================
int JUCEApplication::main (const String& commandLine, JUCEApplication* const app)
{
    const ScopedPointer<JUCEApplication> appDeleter (app);

    if (! app->initialiseApp (commandLine))
        return 0;

    JUCE_TRY
    {
        // loop until a quit message is received..
        MessageManager::getInstance()->runDispatchLoop();
    }
    JUCE_CATCH_EXCEPTION

    return app->shutdownApp();
}

#if JUCE_IOS
 extern int juce_iOSMain (int argc, const char* argv[]);
#endif

#if ! JUCE_WINDOWS
 extern const char* juce_Argv0;
#endif

int JUCEApplication::main (int argc, const char* argv[], JUCEApplication* const newApp)
{
    JUCE_AUTORELEASEPOOL

  #if ! JUCE_WINDOWS
    juce_Argv0 = argv[0];
  #endif

  #if JUCE_IOS
    const ScopedPointer<JUCEApplication> appDeleter (newApp);
    return juce_iOSMain (argc, argv);
  #else
    String cmd;
    for (int i = 1; i < argc; ++i)
        cmd << argv[i] << ' ';

    return JUCEApplication::main (cmd, newApp);
  #endif
}

END_JUCE_NAMESPACE
