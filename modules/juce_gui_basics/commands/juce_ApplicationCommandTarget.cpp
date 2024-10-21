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

class ApplicationCommandTarget::CommandMessage final : public MessageManager::MessageBase
{
public:
    CommandMessage (ApplicationCommandTarget* const target, const InvocationInfo& inf)
        : owner (target), info (inf)
    {
    }

    void messageCallback() override
    {
        if (ApplicationCommandTarget* const target = owner)
            target->tryToInvoke (info, false);
    }

private:
    WeakReference<ApplicationCommandTarget> owner;
    const InvocationInfo info;

    JUCE_DECLARE_NON_COPYABLE (CommandMessage)
};

//==============================================================================
ApplicationCommandTarget::ApplicationCommandTarget() {}
ApplicationCommandTarget::~ApplicationCommandTarget() {}

//==============================================================================
bool ApplicationCommandTarget::tryToInvoke (const InvocationInfo& info, const bool async)
{
    if (isCommandActive (info.commandID))
    {
        if (async)
        {
            (new CommandMessage (this, info))->post();
            return true;
        }

        if (perform (info))
            return true;

        // Hmm.. your target claimed that it could perform this command, but failed to do so.
        // If it can't do it at the moment for some reason, it should clear the 'isActive' flag
        // when it returns the command's info.
        jassertfalse;
    }

    return false;
}

ApplicationCommandTarget* ApplicationCommandTarget::findFirstTargetParentComponent()
{
    if (Component* const c = dynamic_cast<Component*> (this))
        return c->findParentComponentOfClass<ApplicationCommandTarget>();

    return nullptr;
}

ApplicationCommandTarget* ApplicationCommandTarget::getTargetForCommand (const CommandID commandID)
{
    ApplicationCommandTarget* target = this;
    int depth = 0;

    while (target != nullptr)
    {
        Array<CommandID> commandIDs;
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
            Array<CommandID> commandIDs;
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
ApplicationCommandTarget::InvocationInfo::InvocationInfo (const CommandID command)
    : commandID (command),
      commandFlags (0),
      invocationMethod (direct),
      originatingComponent (nullptr),
      isKeyDown (false),
      millisecsSinceKeyPressed (0)
{
}

} // namespace juce
