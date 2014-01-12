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

ApplicationCommandManager::ApplicationCommandManager()
    : firstTarget (nullptr)
{
    keyMappings = new KeyPressMappingSet (*this);
    Desktop::getInstance().addFocusChangeListener (this);
}

ApplicationCommandManager::~ApplicationCommandManager()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    keyMappings = nullptr;
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

    if (getCommandForID (newCommand.commandID) == 0)
    {
        ApplicationCommandInfo* const newInfo = new ApplicationCommandInfo (newCommand);
        newInfo->flags &= ~ApplicationCommandInfo::isTicked;
        commands.add (newInfo);

        keyMappings->resetToDefaultMapping (newCommand.commandID);

        triggerAsyncUpdate();
    }
    else
    {
        // trying to re-register the same command ID with different parameters?
        jassert (newCommand.shortName == getCommandForID (newCommand.commandID)->shortName
                  && (newCommand.description == getCommandForID (newCommand.commandID)->description || newCommand.description.isEmpty())
                  && newCommand.categoryName == getCommandForID (newCommand.commandID)->categoryName
                  && newCommand.defaultKeypresses == getCommandForID (newCommand.commandID)->defaultKeypresses
                  && (newCommand.flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor))
                       == (getCommandForID (newCommand.commandID)->flags & (ApplicationCommandInfo::wantsKeyUpDownCallbacks | ApplicationCommandInfo::hiddenFromKeyEditor | ApplicationCommandInfo::readOnlyInKeyEditor)));
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
const ApplicationCommandInfo* ApplicationCommandManager::getCommandForID (const CommandID commandID) const noexcept
{
    for (int i = commands.size(); --i >= 0;)
        if (commands.getUnchecked(i)->commandID == commandID)
            return commands.getUnchecked(i);

    return nullptr;
}

String ApplicationCommandManager::getNameOfCommand (const CommandID commandID) const noexcept
{
    if (const ApplicationCommandInfo* const ci = getCommandForID (commandID))
        return ci->shortName;

    return String();
}

String ApplicationCommandManager::getDescriptionOfCommand (const CommandID commandID) const noexcept
{
    if (const ApplicationCommandInfo* const ci = getCommandForID (commandID))
        return ci->description.isNotEmpty() ? ci->description
                                            : ci->shortName;

    return String();
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
bool ApplicationCommandManager::invokeDirectly (const CommandID commandID, const bool asynchronously)
{
    ApplicationCommandTarget::InvocationInfo info (commandID);
    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::direct;

    return invoke (info, asynchronously);
}

bool ApplicationCommandManager::invoke (const ApplicationCommandTarget::InvocationInfo& inf, const bool asynchronously)
{
    // This call isn't thread-safe for use from a non-UI thread without locking the message
    // manager first..
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    bool ok = false;
    ApplicationCommandInfo commandInfo (0);

    if (ApplicationCommandTarget* const target = getTargetForCommand (inf.commandID, commandInfo))
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
ApplicationCommandTarget* ApplicationCommandManager::getFirstCommandTarget (const CommandID)
{
    return firstTarget != nullptr ? firstTarget
                                  : findDefaultComponentTarget();
}

void ApplicationCommandManager::setFirstCommandTarget (ApplicationCommandTarget* const newTarget) noexcept
{
    firstTarget = newTarget;
}

ApplicationCommandTarget* ApplicationCommandManager::getTargetForCommand (const CommandID commandID,
                                                                          ApplicationCommandInfo& upToDateInfo)
{
    ApplicationCommandTarget* target = getFirstCommandTarget (commandID);

    if (target == nullptr)
        target = JUCEApplication::getInstance();

    if (target != nullptr)
        target = target->getTargetForCommand (commandID);

    if (target != nullptr)
        target->getCommandInfo (commandID, upToDateInfo);

    return target;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::findTargetForComponent (Component* c)
{
    ApplicationCommandTarget* target = dynamic_cast <ApplicationCommandTarget*> (c);

    if (target == nullptr && c != nullptr)
        target = c->findParentComponentOfClass<ApplicationCommandTarget>();

    return target;
}

ApplicationCommandTarget* ApplicationCommandManager::findDefaultComponentTarget()
{
    Component* c = Component::getCurrentlyFocusedComponent();

    if (c == nullptr)
    {
        if (TopLevelWindow* const activeWindow = TopLevelWindow::getActiveTopLevelWindow())
        {
            c = activeWindow->getPeer()->getLastFocusedSubcomponent();

            if (c == nullptr)
                c = activeWindow;
        }
    }

    if (c == nullptr && Process::isForegroundProcess())
    {
        Desktop& desktop = Desktop::getInstance();

        // getting a bit desperate now: try all desktop comps..
        for (int i = desktop.getNumComponents(); --i >= 0;)
            if (ComponentPeer* const peer = desktop.getComponent(i)->getPeer())
                if (ApplicationCommandTarget* const target = findTargetForComponent (peer->getLastFocusedSubcomponent()))
                    return target;
    }

    if (c != nullptr)
    {
        // if we're focused on a ResizableWindow, chances are that it's the content
        // component that really should get the event. And if not, the event will
        // still be passed up to the top level window anyway, so let's send it to the
        // content comp.
        if (ResizableWindow* const resizableWindow = dynamic_cast <ResizableWindow*> (c))
            if (Component* const content = resizableWindow->getContentComponent())
                c = content;

        if (ApplicationCommandTarget* const target = findTargetForComponent (c))
            return target;
    }

    return JUCEApplication::getInstance();
}

//==============================================================================
void ApplicationCommandManager::addListener (ApplicationCommandManagerListener* const listener)
{
    listeners.add (listener);
}

void ApplicationCommandManager::removeListener (ApplicationCommandManagerListener* const listener)
{
    listeners.remove (listener);
}

void ApplicationCommandManager::sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo& info)
{
    listeners.call (&ApplicationCommandManagerListener::applicationCommandInvoked, info);
}

void ApplicationCommandManager::handleAsyncUpdate()
{
    listeners.call (&ApplicationCommandManagerListener::applicationCommandListChanged);
}

void ApplicationCommandManager::globalFocusChanged (Component*)
{
    commandStatusChanged();
}
