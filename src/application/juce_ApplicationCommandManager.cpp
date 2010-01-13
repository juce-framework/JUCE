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

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationCommandManager.h"
#include "juce_Application.h"
#include "../gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../gui/components/windows/juce_ResizableWindow.h"
#include "../gui/components/juce_Desktop.h"
#include "../events/juce_MessageManager.h"
#include "../threads/juce_Process.h"


//==============================================================================
ApplicationCommandManager::ApplicationCommandManager()
    : firstTarget (0)
{
    keyMappings = new KeyPressMappingSet (this);

    Desktop::getInstance().addFocusChangeListener (this);
}

ApplicationCommandManager::~ApplicationCommandManager()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    keyMappings = 0;
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
        // trying to re-register the same command with different parameters?
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
    if (target != 0)
    {
        Array <CommandID> commandIDs;
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

            const Array <KeyPress> keys (keyMappings->getKeyPressesAssignedToCommand (commandID));

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
const ApplicationCommandInfo* ApplicationCommandManager::getCommandForID (const CommandID commandID) const throw()
{
    for (int i = commands.size(); --i >= 0;)
        if (commands.getUnchecked(i)->commandID == commandID)
            return commands.getUnchecked(i);

    return 0;
}

const String ApplicationCommandManager::getNameOfCommand (const CommandID commandID) const throw()
{
    const ApplicationCommandInfo* const ci = getCommandForID (commandID);

    return (ci != 0) ? ci->shortName : String::empty;
}

const String ApplicationCommandManager::getDescriptionOfCommand (const CommandID commandID) const throw()
{
    const ApplicationCommandInfo* const ci = getCommandForID (commandID);

    return (ci != 0) ? (ci->description.isNotEmpty() ? ci->description : ci->shortName)
                     : String::empty;
}

const StringArray ApplicationCommandManager::getCommandCategories() const throw()
{
    StringArray s;

    for (int i = 0; i < commands.size(); ++i)
        s.addIfNotAlreadyThere (commands.getUnchecked(i)->categoryName, false);

    return s;
}

const Array <CommandID> ApplicationCommandManager::getCommandsInCategory (const String& categoryName) const throw()
{
    Array <CommandID> results;

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

bool ApplicationCommandManager::invoke (const ApplicationCommandTarget::InvocationInfo& info_, const bool asynchronously)
{
    // This call isn't thread-safe for use from a non-UI thread without locking the message
    // manager first..
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    ApplicationCommandTarget* const target = getFirstCommandTarget (info_.commandID);

    if (target == 0)
        return false;

    ApplicationCommandInfo commandInfo (0);
    target->getCommandInfo (info_.commandID, commandInfo);

    ApplicationCommandTarget::InvocationInfo info (info_);
    info.commandFlags = commandInfo.flags;

    sendListenerInvokeCallback (info);

    const bool ok = target->invoke (info, asynchronously);

    commandStatusChanged();

    return ok;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::getFirstCommandTarget (const CommandID)
{
    return firstTarget != 0 ? firstTarget
                            : findDefaultComponentTarget();
}

void ApplicationCommandManager::setFirstCommandTarget (ApplicationCommandTarget* const newTarget) throw()
{
    firstTarget = newTarget;
}

ApplicationCommandTarget* ApplicationCommandManager::getTargetForCommand (const CommandID commandID,
                                                                          ApplicationCommandInfo& upToDateInfo)
{
    ApplicationCommandTarget* target = getFirstCommandTarget (commandID);

    if (target == 0)
        target = JUCEApplication::getInstance();

    if (target != 0)
        target = target->getTargetForCommand (commandID);

    if (target != 0)
        target->getCommandInfo (commandID, upToDateInfo);

    return target;
}

//==============================================================================
ApplicationCommandTarget* ApplicationCommandManager::findTargetForComponent (Component* c)
{
    ApplicationCommandTarget* target = dynamic_cast <ApplicationCommandTarget*> (c);

    if (target == 0 && c != 0)
        // (unable to use the syntax findParentComponentOfClass <ApplicationCommandTarget> () because of a VC6 compiler bug)
        target = c->findParentComponentOfClass ((ApplicationCommandTarget*) 0);

    return target;
}

ApplicationCommandTarget* ApplicationCommandManager::findDefaultComponentTarget()
{
    Component* c = Component::getCurrentlyFocusedComponent();

    if (c == 0)
    {
        TopLevelWindow* const activeWindow = TopLevelWindow::getActiveTopLevelWindow();

        if (activeWindow != 0)
        {
            c = activeWindow->getPeer()->getLastFocusedSubcomponent();

            if (c == 0)
                c = activeWindow;
        }
    }

    if (c == 0 && Process::isForegroundProcess())
    {
        // getting a bit desperate now - try all desktop comps..
        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            ApplicationCommandTarget* const target
                = findTargetForComponent (Desktop::getInstance().getComponent (i)
                                              ->getPeer()->getLastFocusedSubcomponent());

            if (target != 0)
                return target;
        }
    }

    if (c != 0)
    {
        ResizableWindow* const resizableWindow = dynamic_cast <ResizableWindow*> (c);

        // if we're focused on a ResizableWindow, chances are that it's the content
        // component that really should get the event. And if not, the event will
        // still be passed up to the top level window anyway, so let's send it to the
        // content comp.
        if (resizableWindow != 0 && resizableWindow->getContentComponent() != 0)
            c = resizableWindow->getContentComponent();

        ApplicationCommandTarget* const target = findTargetForComponent (c);

        if (target != 0)
            return target;
    }

    return JUCEApplication::getInstance();
}

//==============================================================================
void ApplicationCommandManager::addListener (ApplicationCommandManagerListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void ApplicationCommandManager::removeListener (ApplicationCommandManagerListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void ApplicationCommandManager::sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo& info) const
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((ApplicationCommandManagerListener*) listeners.getUnchecked (i))->applicationCommandInvoked (info);
        i = jmin (i, listeners.size());
    }
}

void ApplicationCommandManager::handleAsyncUpdate()
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((ApplicationCommandManagerListener*) listeners.getUnchecked (i))->applicationCommandListChanged();
        i = jmin (i, listeners.size());
    }
}

void ApplicationCommandManager::globalFocusChanged (Component*)
{
    commandStatusChanged();
}


END_JUCE_NAMESPACE
