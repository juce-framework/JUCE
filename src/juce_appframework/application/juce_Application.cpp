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

#include "../../juce_core/basics/juce_StandardHeader.h"

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4245 4514 4100)
  #include <crtdbg.h>
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_Application.h"
#include "juce_DeletedAtShutdown.h"
#include "../events/juce_MessageManager.h"
#include "../gui/graphics/contexts/juce_Graphics.h"
#include "../gui/components/windows/juce_AlertWindow.h"
#include "../gui/components/buttons/juce_TextButton.h"
#include "../gui/components/lookandfeel/juce_LookAndFeel.h"
#include "../../juce_core/basics/juce_Time.h"
#include "../../juce_core/basics/juce_Initialisation.h"
#include "../../juce_core/threads/juce_Process.h"
#include "../../juce_core/threads/juce_InterProcessLock.h"

void juce_setCurrentExecutableFileName (const String& filename) throw();
void juce_setCurrentThreadName (const String& name) throw();

static JUCEApplication* appInstance = 0;


//==============================================================================
JUCEApplication::JUCEApplication()
    : appReturnValue (0),
      stillInitialising (true)
{
}

JUCEApplication::~JUCEApplication()
{
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

void JUCEApplication::quit (const bool useMaximumForce)
{
    MessageManager::getInstance()->postQuitMessage (useMaximumForce);
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
        result.setInfo ("Quit",
                        "Quits the application",
                        "Application",
                        0);

        result.defaultKeypresses.add (KeyPress (T('q'), ModifierKeys::commandModifier, 0));
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
    jassert (appInstance == 0);
    appInstance = app;
    bool useForce = true;

    initialiseJuce_GUI();

    InterProcessLock* appLock = 0;

    if (! app->moreThanOneInstanceAllowed())
    {
        appLock = new InterProcessLock ("juceAppLock_" + app->getApplicationName());

        if (! appLock->enter(0))
        {
            MessageManager::broadcastMessage (app->getApplicationName() + "/" + commandLine);

            delete appInstance;
            appInstance = 0;
            commandLine = String::empty;

            DBG ("Another instance is running - quitting...");
            return 0;
        }
    }

    JUCE_TRY
    {
        juce_setCurrentThreadName ("Juce Message Thread");

        // let the app do its setting-up..
        app->initialise (commandLine.trim());

        commandLine = String::empty;

        // register for broadcast new app messages
        MessageManager::getInstance()->registerBroadcastListener (app);

        app->stillInitialising = false;

        // now loop until a quit message is received..
        useForce = MessageManager::getInstance()->runDispatchLoop();

        MessageManager::getInstance()->deregisterBroadcastListener (app);

        if (appLock != 0)
        {
            appLock->exit();
            delete appLock;
        }
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

    return shutdownAppAndClearUp (useForce);
}

int JUCEApplication::shutdownAppAndClearUp (const bool useMaximumForce)
{
    jassert (appInstance != 0);
    JUCEApplication* const app = appInstance;
    int returnValue = 0;

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
            delete app;
        }
        JUCE_CATCH_ALL_ASSERT

        if (useMaximumForce)
        {
            Process::terminate();
        }

        reentrancyCheck = false;
    }

    return returnValue;
}

int JUCEApplication::main (int argc, char* argv[],
                           JUCEApplication* const newApp)
{
    juce_setCurrentExecutableFileName (argv[0]);

    String cmd;
    for (int i = 1; i < argc; ++i)
        cmd << argv[i] << T(' ');

    return JUCEApplication::main (cmd, newApp);
}

void JUCEApplication::actionListenerCallback (const String& message)
{
    if (message.startsWith (getApplicationName() + "/"))
    {
        anotherInstanceStarted (message.fromFirstOccurrenceOf (T("/"), false, false));
    }
}

//==============================================================================
static bool juceInitialisedGUI = false;


void JUCE_PUBLIC_FUNCTION initialiseJuce_GUI()
{
    if (! juceInitialisedGUI)
    {
        juceInitialisedGUI = true;

        initialiseJuce_NonGUI();
        MessageManager::getInstance();
        Font::initialiseDefaultFontNames();
        LookAndFeel::setDefaultLookAndFeel (0);

#if JUCE_WIN32 && JUCE_DEBUG
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
        DeletedAtShutdown::deleteAll();

        LookAndFeel::clearDefaultLookAndFeel();
        shutdownJuce_NonGUI();

        juceInitialisedGUI = false;
    }
}

END_JUCE_NAMESPACE
