/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCEApplication::JUCEApplication() {}
JUCEApplication::~JUCEApplication() {}

//==============================================================================
JUCEApplication* JUCE_CALLTYPE JUCEApplication::getInstance() noexcept
{
    return dynamic_cast<JUCEApplication*> (JUCEApplicationBase::getInstance());
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
        result.setInfo (TRANS ("Quit"),
                        TRANS ("Quits the application"),
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
bool JUCEApplication::initialiseApp()
{
    if (JUCEApplicationBase::initialiseApp())
    {
       #if JUCE_MAC
        initialiseMacMainMenu(); // (needs to get the app's name)
       #endif

        return true;
    }

    return false;
}

} // namespace juce
