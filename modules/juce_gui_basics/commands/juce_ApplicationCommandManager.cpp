/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ApplicationCommandManager::ApplicationCommandManager()
{
    keyMappings.reset (new KeyPressMappingSet (*this));
    Desktop::getInstance().addFocusChangeListener (this);
}

ApplicationCommandManager::~ApplicationCommandManager()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    keyMappings.reset();
}

//==============================================================================
void ApplicationCommandManager::clearCommands()
{
    commands.clear();
    keyMappings->clearAllKeyPresses();
    triggerAsyncUpdate();
}

void ApplicationCommandManager::registerCommand (const ApplicationCommandInfo& newCommand)
{
    // zero isn't a valid command ID!
    jassert (newCommand.commandID != 0);

    // the name isn't optional!
    jassert (newCommand.shortName.isNotEmpty());

    if (auto* command = getMutableCommandForID (newCommand.commandID))
    {
        // Trying to re-register the same command ID with different parameters can often indicate a typo.
        // This assertion is here because I've found it useful catching some mistakes, but it may also cause
        // false alarms if you're deliberately updating some flags for a command.
        jassert (newCommand.shortName == getCommandForID (newCommand.commandID)->shortName
                  && newCommand.categoryName == getCommandForID (newCommand.commandID)->categoryName
                  && newCommand.defaultKeypresses == getCommandForID (newCommand.commandID)->defaultKeypresses
                  && (newCommand.flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor))
                       == (getCommandForID (newCommand.commandID)->flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor)));

        *command = newCommand;
    }
    else
    {
        auto* newInfo = new ApplicationCommandInfo (newCommand);
        newInfo->flags &= ~ApplicationCommandInfo::isTicked;
        commands.add (newInfo);

        keyMappings->resetToDefaultMapping (newCommand.commandID);

        triggerAsyncUpdate();
    }
}

void ApplicationCommandManager::registerAllCommandsForTarget (ApplicationCommandTarget* target)
{
    if (target != nullptr)
    {
        Array<CommandID> commandIDs;
        target->getAllCommands (commandIDs);

        for (int i = 0; i < commandIDs.size(); ++i)
        {
            ApplicationCommandInfo info (commandIDs.getUnchecked(i));
            target->getCommandInfo (info.commandID, info);

            registerCommand (info);
        }
    }
}

void ApplicationCommandManager::removeCommand (const CommandID commandID)
{
    for (int i = commands.size(); --i >= 0;)
    {
        if (commands.getUnchecked (i)->commandID == commandID)
        {
            commands.remove (i);
            triggerAsyncUpdate();

            const Array<KeyPress> keys (keyMappings->getKeyPressesAssignedToCommand (commandID));

            for (int j = keys.size(); --j >= 0;)
                keyMappings->removeKeyPress (keys.getReference (j));
        }
    }
}

void ApplicationCommandManager::commandStatusChanged()
{
    triggerAsyncUpdate();
}

//==============================================================================
ApplicationCommandInfo* ApplicationCommandManager::getMutableCommandForID (CommandID commandID) const noexcept
{
    for (int i = commands.size(); --i >= 0;)
        if (commands.getUnchecked(i)->commandID == commandID)
            return commands.getUnchecked(i);

    return nullptr;
}

const ApplicationCommandInfo* ApplicationCommandManager::getCommandForID (CommandID commandID) const noexcept
{
    return getMutableCommandForID (commandID);
}

String ApplicationCommandManager::getNameOfCommand (CommandID commandID) const noexcept
{
    if (auto* ci = getCommandForID (commandID))
        return ci->shortName;

    return {};
}

String ApplicationCommandManager::getDescriptionOfCommand (CommandID commandID) const noexcept
{
    if (auto* ci = getCommandForID (commandID))
        return ci->description.isNotEmpty() ? ci->description
                                            : ci->shortName;

    return {};
}

StringArray ApplicationCommandManager::getCommandCategories() const
{
    StringArray s;

    for (int i = 0; i < commands.size(); ++i)
        s.addIfNotAlreadyThere (commands.getUnchecked(i)->categoryName, false);

    return s;
}

Array<CommandID> ApplicationCommandManager::getCommandsInCategory (const String& categoryName) const
{
    Array<CommandID> results;

    for (int i = 0; i < commands.size(); ++i)
        if (commands.getUnchecked(i)->categoryName == categoryName)
            results.add (commands.getUnchecked(i)->commandID);

    return results;
}

//==============================================================================
bool ApplicationCommandManager::invokeDirectly (CommandID commandID, bool asynchronously)
{
    ApplicationCommandTarget::InvocationInfo info (commandID);
    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::direct;

    return invoke (info, asynchronously);
}

bool ApplicationCommandManager::invoke (const ApplicationCommandTarget::InvocationInfo& inf, bool asynchronously)
{
    // This call isn't thread-safe for use from a non-UI thread without locking the message
    // manager first..
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    bool ok = false;
    ApplicationCommandInfo commandInfo (0);

    if (auto* target = getTargetForCommand (inf.commandID, commandInfo))
    {
        ApplicationCommandTarget::InvocationInfo info (inf);
        info.commandFlags = commandInfo.flags;

        sendListenerInvokeCallback (info);
        ok = target->invoke (info, asynchronously);
        commandStatusChanged();
    }

    return ok;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::getFirstCommandTarget (CommandID)
{
    return firstTarget != nullptr ? firstTarget
                                  : findDefaultComponentTarget();
}

void ApplicationCommandManager::setFirstCommandTarget (ApplicationCommandTarget* newTarget) noexcept
{
    firstTarget = newTarget;
}

ApplicationCommandTarget* ApplicationCommandManager::getTargetForCommand (CommandID commandID,
                                                                          ApplicationCommandInfo& upToDateInfo)
{
    auto* target = getFirstCommandTarget (commandID);

    if (target == nullptr)
        target = JUCEApplication::getInstance();

    if (target != nullptr)
        target = target->getTargetForCommand (commandID);

    if (target != nullptr)
    {
        upToDateInfo.commandID = commandID;
        target->getCommandInfo (commandID, upToDateInfo);
    }

    return target;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::findTargetForComponent (Component* c)
{
    auto* target = dynamic_cast<ApplicationCommandTarget*> (c);

    if (target == nullptr && c != nullptr)
        target = c->findParentComponentOfClass<ApplicationCommandTarget>();

    return target;
}

ApplicationCommandTarget* ApplicationCommandManager::findDefaultComponentTarget()
{
    auto* c = Component::getCurrentlyFocusedComponent();

    if (c == nullptr)
    {
        if (auto* activeWindow = TopLevelWindow::getActiveTopLevelWindow())
        {
            c = activeWindow->getPeer()->getLastFocusedSubcomponent();

            if (c == nullptr)
                c = activeWindow;
        }
    }

    if (c == nullptr && Process::isForegroundProcess())
    {
        auto& desktop = Desktop::getInstance();

        // getting a bit desperate now: try all desktop comps..
        for (int i = desktop.getNumComponents(); --i >= 0;)
            if (auto* peer = desktop.getComponent(i)->getPeer())
                if (auto* target = findTargetForComponent (peer->getLastFocusedSubcomponent()))
                    return target;
    }

    if (c != nullptr)
    {
        // if we're focused on a ResizableWindow, chances are that it's the content
        // component that really should get the event. And if not, the event will
        // still be passed up to the top level window anyway, so let's send it to the
        // content comp.
        if (auto* resizableWindow = dynamic_cast<ResizableWindow*> (c))
            if (auto* content = resizableWindow->getContentComponent())
                c = content;

        if (auto* target = findTargetForComponent (c))
            return target;
    }

    return JUCEApplication::getInstance();
}

//==============================================================================
void ApplicationCommandManager::addListener (ApplicationCommandManagerListener* listener)
{
    listeners.add (listener);
}

void ApplicationCommandManager::removeListener (ApplicationCommandManagerListener* listener)
{
    listeners.remove (listener);
}

void ApplicationCommandManager::sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo& info)
{
    listeners.call ([&] (ApplicationCommandManagerListener& l) { l.applicationCommandInvoked (info); });
}

void ApplicationCommandManager::handleAsyncUpdate()
{
    listeners.call ([] (ApplicationCommandManagerListener& l) { l.applicationCommandListChanged(); });
}

void ApplicationCommandManager::globalFocusChanged (Component*)
{
    commandStatusChanged();
}

} // namespace juce
