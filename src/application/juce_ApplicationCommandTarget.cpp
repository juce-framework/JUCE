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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationCommandTarget.h"
#include "juce_Application.h"


//==============================================================================
ApplicationCommandTarget::ApplicationCommandTarget()
{
}

ApplicationCommandTarget::~ApplicationCommandTarget()
{
    messageInvoker = nullptr;
}

//==============================================================================
bool ApplicationCommandTarget::tryToInvoke (const InvocationInfo& info, const bool async)
{
    if (isCommandActive (info.commandID))
    {
        if (async)
        {
            if (messageInvoker == nullptr)
                messageInvoker = new CommandTargetMessageInvoker (this);

            messageInvoker->postMessage (new Message (0, 0, 0, new ApplicationCommandTarget::InvocationInfo (info)));
            return true;
        }
        else
        {
            const bool success = perform (info);

            jassert (success);  // hmm - your target should have been able to perform this command. If it can't
                                // do it at the moment for some reason, it should clear the 'isActive' flag when it
                                // returns the command's info.
            return success;
        }
    }

    return false;
}

ApplicationCommandTarget* ApplicationCommandTarget::findFirstTargetParentComponent()
{
    Component* c = dynamic_cast <Component*> (this);

    if (c != nullptr)
        // (unable to use the syntax findParentComponentOfClass <ApplicationCommandTarget> () because of a VC6 compiler bug)
        return c->findParentComponentOfClass ((ApplicationCommandTarget*) nullptr);

    return nullptr;
}

ApplicationCommandTarget* ApplicationCommandTarget::getTargetForCommand (const CommandID commandID)
{
    ApplicationCommandTarget* target = this;
    int depth = 0;

    while (target != nullptr)
    {
        Array <CommandID> commandIDs;
        target->getAllCommands (commandIDs);

        if (commandIDs.contains (commandID))
            return target;

        target = target->getNextCommandTarget();

        ++depth;
        jassert (depth < 100); // could be a recursive command chain??
        jassert (target != this); // definitely a recursive command chain!

        if (depth > 100 || target == this)
            break;
    }

    if (target == nullptr)
    {
        target = JUCEApplication::getInstance();

        if (target != nullptr)
        {
            Array <CommandID> commandIDs;
            target->getAllCommands (commandIDs);

            if (commandIDs.contains (commandID))
                return target;
        }
    }

    return nullptr;
}

bool ApplicationCommandTarget::isCommandActive (const CommandID commandID)
{
    ApplicationCommandInfo info (commandID);
    info.flags = ApplicationCommandInfo::isDisabled;

    getCommandInfo (commandID, info);

    return (info.flags & ApplicationCommandInfo::isDisabled) == 0;
}

//==============================================================================
bool ApplicationCommandTarget::invoke (const InvocationInfo& info, const bool async)
{
    ApplicationCommandTarget* target = this;
    int depth = 0;

    while (target != nullptr)
    {
        if (target->tryToInvoke (info, async))
            return true;

        target = target->getNextCommandTarget();

        ++depth;
        jassert (depth < 100); // could be a recursive command chain??
        jassert (target != this); // definitely a recursive command chain!

        if (depth > 100 || target == this)
            break;
    }

    if (target == nullptr)
    {
        target = JUCEApplication::getInstance();

        if (target != nullptr)
            return target->tryToInvoke (info, async);
    }

    return false;
}

bool ApplicationCommandTarget::invokeDirectly (const CommandID commandID, const bool asynchronously)
{
    ApplicationCommandTarget::InvocationInfo info (commandID);
    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::direct;

    return invoke (info, asynchronously);
}

//==============================================================================
ApplicationCommandTarget::InvocationInfo::InvocationInfo (const CommandID commandID_)
    : commandID (commandID_),
      commandFlags (0),
      invocationMethod (direct),
      originatingComponent (nullptr),
      isKeyDown (false),
      millisecsSinceKeyPressed (0)
{
}

//==============================================================================
ApplicationCommandTarget::CommandTargetMessageInvoker::CommandTargetMessageInvoker (ApplicationCommandTarget* const owner_)
    : owner (owner_)
{
}

ApplicationCommandTarget::CommandTargetMessageInvoker::~CommandTargetMessageInvoker()
{
}

void ApplicationCommandTarget::CommandTargetMessageInvoker::handleMessage (const Message& message)
{
    const ScopedPointer <InvocationInfo> info (static_cast <InvocationInfo*> (message.pointerParameter));
    owner->tryToInvoke (*info, false);
}


END_JUCE_NAMESPACE
