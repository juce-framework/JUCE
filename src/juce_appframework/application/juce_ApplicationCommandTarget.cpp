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

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationCommandTarget.h"
#include "juce_Application.h"


//==============================================================================
ApplicationCommandTarget::ApplicationCommandTarget()
    : messageInvoker (0)
{
}

ApplicationCommandTarget::~ApplicationCommandTarget()
{
    deleteAndZero (messageInvoker);
}

//==============================================================================
bool ApplicationCommandTarget::tryToInvoke (const InvocationInfo& info, const bool async)
{
    if (isCommandActive (info.commandID))
    {
        if (async)
        {
            if (messageInvoker == 0)
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

    if (c != 0)
        // (unable to use the syntax findParentComponentOfClass <ApplicationCommandTarget> () because of a VC6 compiler bug)
        return c->findParentComponentOfClass ((ApplicationCommandTarget*) 0);

    return 0;
}

ApplicationCommandTarget* ApplicationCommandTarget::getTargetForCommand (const CommandID commandID)
{
    ApplicationCommandTarget* target = this;
    int depth = 0;

    while (target != 0)
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

    if (target == 0)
    {
        target = JUCEApplication::getInstance();

        if (target != 0)
        {
            Array <CommandID> commandIDs;
            target->getAllCommands (commandIDs);

            if (commandIDs.contains (commandID))
                return target;
        }
    }

    return 0;
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

    while (target != 0)
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

    if (target == 0)
    {
        target = JUCEApplication::getInstance();

        if (target != 0)
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
ApplicationCommandTarget::InvocationInfo::InvocationInfo (const CommandID commandID_) throw()
    : commandID (commandID_),
      commandFlags (0),
      invocationMethod (direct),
      originatingComponent (0),
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
    InvocationInfo* const info = (InvocationInfo*) message.pointerParameter;
    owner->tryToInvoke (*info, false);
    delete info;
}


END_JUCE_NAMESPACE
