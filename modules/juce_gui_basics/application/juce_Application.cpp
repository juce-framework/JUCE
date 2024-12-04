/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
