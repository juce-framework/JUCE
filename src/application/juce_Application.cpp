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

#include "../core/juce_StandardHeader.h"

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4245 4514 4100)
  #include <crtdbg.h>
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_Application.h"
#include "../utilities/juce_DeletedAtShutdown.h"
#include "../events/juce_MessageManager.h"
#include "../gui/graphics/contexts/juce_Graphics.h"
#include "../gui/components/windows/juce_AlertWindow.h"
#include "../gui/components/buttons/juce_TextButton.h"
#include "../gui/components/lookandfeel/juce_LookAndFeel.h"
#include "../core/juce_Time.h"
#include "../core/juce_Initialisation.h"
#include "../threads/juce_Process.h"
#include "../core/juce_PlatformUtilities.h"
#include "../text/juce_LocalisedStrings.h"

void juce_setCurrentThreadName (const String& name);

static JUCEApplication* appInstance = 0;


//==============================================================================
JUCEApplication::JUCEApplication()
    : appReturnValue (0),
      stillInitialising (true)
{
}

JUCEApplication::~JUCEApplication()
{
    if (appLock != 0)
    {
        appLock->exit();
        appLock = 0;
    }
}

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

//==============================================================================
void JUCEApplication::unhandledException (const std::exception*,
                                          const String&,
                                          const int)
{
    jassertfalse
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
int JUCEApplication::main (String& commandLine, JUCEApplication* const app)
{
    if (! app->initialiseApp (commandLine))
        return 0;

    // now loop until a quit message is received..
    JUCE_TRY
    {
        MessageManager::getInstance()->runDispatchLoop();
    }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
    catch (const std::exception& e)
    {
        app->unhandledException (&e, __FILE__, __LINE__);
    }
    catch (...)
    {
        app->unhandledException (0, __FILE__, __LINE__);
    }
#endif

    return shutdownAppAndClearUp();
}

bool JUCEApplication::initialiseApp (String& commandLine)
{
    jassert (appInstance == 0);
    appInstance = this;

    commandLineParameters = commandLine.trim();
    commandLine = String::empty;

    initialiseJuce_GUI();

#if ! JUCE_IPHONE
    jassert (appLock == 0); // initialiseApp must only be called once!

    if (! moreThanOneInstanceAllowed())
    {
        appLock = new InterProcessLock ("juceAppLock_" + getApplicationName());

        if (! appLock->enter(0))
        {
            MessageManager::broadcastMessage (getApplicationName() + "/" + commandLineParameters);

            delete appInstance;
            appInstance = 0;

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

int JUCEApplication::shutdownAppAndClearUp()
{
    jassert (appInstance != 0);
    ScopedPointer<JUCEApplication> app (appInstance);
    int returnValue = 0;

    MessageManager::getInstance()->deregisterBroadcastListener (static_cast <JUCEApplication*> (app));

    static bool reentrancyCheck = false;

    if (! reentrancyCheck)
    {
        reentrancyCheck = true;

        JUCE_TRY
        {
            // give the app a chance to clean up..
            app->shutdown();
        }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
        catch (const std::exception& e)
        {
            app->unhandledException (&e, __FILE__, __LINE__);
        }
        catch (...)
        {
            app->unhandledException (0, __FILE__, __LINE__);
        }
#endif

        JUCE_TRY
        {
            shutdownJuce_GUI();

            returnValue = app->getApplicationReturnValue();

            appInstance = 0;
            app = 0;
        }
        JUCE_CATCH_ALL_ASSERT

        reentrancyCheck = false;
    }

    return returnValue;
}

#if JUCE_IPHONE
 extern int juce_IPhoneMain (int argc, const char* argv[], JUCEApplication* app);
#endif

#if ! JUCE_WINDOWS
extern const char* juce_Argv0;
#endif

int JUCEApplication::main (int argc, const char* argv[], JUCEApplication* const newApp)
{
#if ! JUCE_WINDOWS
    juce_Argv0 = argv[0];
#endif

#if JUCE_IPHONE
    const ScopedAutoReleasePool pool;
    return juce_IPhoneMain (argc, argv, newApp);
#else

  #if JUCE_MAC
    const ScopedAutoReleasePool pool;
  #endif

    String cmd;
    for (int i = 1; i < argc; ++i)
        cmd << argv[i] << ' ';

    return JUCEApplication::main (cmd, newApp);
#endif
}

void JUCEApplication::actionListenerCallback (const String& message)
{
    if (message.startsWith (getApplicationName() + "/"))
        anotherInstanceStarted (message.substring (getApplicationName().length() + 1));
}

//==============================================================================
static bool juceInitialisedGUI = false;


void JUCE_PUBLIC_FUNCTION initialiseJuce_GUI()
{
    if (! juceInitialisedGUI)
    {
#if JUCE_MAC || JUCE_IPHONE
        const ScopedAutoReleasePool pool;
#endif
        juceInitialisedGUI = true;

        initialiseJuce_NonGUI();
        MessageManager::getInstance();
        LookAndFeel::setDefaultLookAndFeel (0);
        juce_setCurrentThreadName ("Juce Message Thread");

#if JUCE_WINDOWS && JUCE_DEBUG
        // This section is just for catching people who mess up their project settings and
        // turn RTTI off..
        try
        {
            TextButton tb (String::empty);
            Component* c = &tb;

            // Got an exception here? Then TURN ON RTTI in your compiler settings!!
            c = dynamic_cast <Button*> (c);
        }
        catch (...)
        {
            // Ended up here? If so, TURN ON RTTI in your compiler settings!! And if you
            // got as far as this catch statement, then why haven't you got exception catching
            // turned on in the debugger???
            jassertfalse
        }
#endif
    }
}

void JUCE_PUBLIC_FUNCTION shutdownJuce_GUI()
{
    if (juceInitialisedGUI)
    {
#if JUCE_MAC
        const ScopedAutoReleasePool pool;
#endif
        {
            DeletedAtShutdown::deleteAll();

            LookAndFeel::clearDefaultLookAndFeel();
        }

        delete MessageManager::getInstance();

        shutdownJuce_NonGUI();

        juceInitialisedGUI = false;
    }
}

END_JUCE_NAMESPACE
