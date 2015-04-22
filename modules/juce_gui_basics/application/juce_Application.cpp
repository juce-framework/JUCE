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

JUCEApplication::JUCEApplication() {}
JUCEApplication::~JUCEApplication() {}

//==============================================================================
JUCEApplication* JUCE_CALLTYPE JUCEApplication::getInstance() noexcept
{
    return dynamic_cast <JUCEApplication*> (JUCEApplicationBase::getInstance());
}

bool JUCEApplication::moreThanOneInstanceAllowed()  { return true; }
void JUCEApplication::anotherInstanceStarted (const String&) {}

void JUCEApplication::suspended() {}
void JUCEApplication::resumed() {}

void JUCEApplication::systemRequestedQuit()         { quit(); }

void JUCEApplication::unhandledException (const std::exception*, const String&, int)
{
    jassertfalse;
}

//==============================================================================
ApplicationCommandTarget* JUCEApplication::getNextCommandTarget()
{
    return nullptr;
}

void JUCEApplication::getAllCommands (Array<CommandID>& commands)
{
    commands.add (StandardApplicationCommandIDs::quit);
}

void JUCEApplication::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    if (commandID == StandardApplicationCommandIDs::quit)
    {
        result.setInfo (TRANS("Quit"),
                        TRANS("Quits the application"),
                        "Application", 0);

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
#if JUCE_MAC
 extern void juce_initialiseMacMainMenu();
#endif

bool JUCEApplication::initialiseApp()
{
    if (JUCEApplicationBase::initialiseApp())
    {
       #if JUCE_MAC
        juce_initialiseMacMainMenu(); // (needs to get the app's name)
       #endif

        return true;
    }

    return false;
}
