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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_KeyPressMappingSet.h"
#include "../../../../juce_core/basics/juce_Time.h"
#include "../../../../juce_core/misc/juce_PlatformUtilities.h"


//==============================================================================
KeyPressMappingSet::KeyPressMappingSet (ApplicationCommandManager* const commandManager_) throw()
    : commandManager (commandManager_)
{
    // A manager is needed to get the descriptions of commands, and will be called when
    // a command is invoked. So you can't leave this null..
    jassert (commandManager_ != 0);

    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::KeyPressMappingSet (const KeyPressMappingSet& other) throw()
    : commandManager (other.commandManager)
{
    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::~KeyPressMappingSet()
{
    Desktop::getInstance().removeFocusChangeListener (this);
}

//==============================================================================
const Array <KeyPress> KeyPressMappingSet::getKeyPressesAssignedToCommand (const CommandID commandID) const throw()
{
    for (int i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked(i)->commandID == commandID)
            return mappings.getUnchecked (i)->keypresses;

    return Array <KeyPress> ();
}

void KeyPressMappingSet::addKeyPress (const CommandID commandID,
                                      const KeyPress& newKeyPress,
                                      int insertIndex) throw()
{
    if (findCommandForKeyPress (newKeyPress) != commandID)
    {
        removeKeyPress (newKeyPress);

        if (newKeyPress.isValid())
        {
            for (int i = mappings.size(); --i >= 0;)
            {
                if (mappings.getUnchecked(i)->commandID == commandID)
                {
                    mappings.getUnchecked(i)->keypresses.insert (insertIndex, newKeyPress);

                    sendChangeMessage (this);
                    return;
                }
            }

            const ApplicationCommandInfo* const ci = commandManager->getCommandForID (commandID);

            if (ci != 0)
            {
                CommandMapping* const cm = new CommandMapping();
                cm->commandID = commandID;
                cm->keypresses.add (newKeyPress);
                cm->wantsKeyUpDownCallbacks = (ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) != 0;

                mappings.add (cm);
                sendChangeMessage (this);
            }
        }
    }
}

void KeyPressMappingSet::resetToDefaultMappings() throw()
{
    mappings.clear();

    for (int i = 0; i < commandManager->getNumCommands(); ++i)
    {
        const ApplicationCommandInfo* const ci = commandManager->getCommandForIndex (i);

        for (int j = 0; j < ci->defaultKeypresses.size(); ++j)
        {
            addKeyPress (ci->commandID,
                         ci->defaultKeypresses.getReference (j));
        }
    }

    sendChangeMessage (this);
}

void KeyPressMappingSet::resetToDefaultMapping (const CommandID commandID) throw()
{
    clearAllKeyPresses (commandID);

    const ApplicationCommandInfo* const ci = commandManager->getCommandForID (commandID);

    for (int j = 0; j < ci->defaultKeypresses.size(); ++j)
    {
        addKeyPress (ci->commandID,
                     ci->defaultKeypresses.getReference (j));
    }
}

void KeyPressMappingSet::clearAllKeyPresses() throw()
{
    if (mappings.size() > 0)
    {
        sendChangeMessage (this);
        mappings.clear();
    }
}

void KeyPressMappingSet::clearAllKeyPresses (const CommandID commandID) throw()
{
    for (int i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked(i)->commandID == commandID)
        {
            mappings.remove (i);
            sendChangeMessage (this);
        }
    }
}

void KeyPressMappingSet::removeKeyPress (const KeyPress& keypress) throw()
{
    if (keypress.isValid())
    {
        for (int i = mappings.size(); --i >= 0;)
        {
            CommandMapping* const cm = mappings.getUnchecked(i);

            for (int j = cm->keypresses.size(); --j >= 0;)
            {
                if (keypress == cm->keypresses [j])
                {
                    cm->keypresses.remove (j);
                    sendChangeMessage (this);
                }
            }
        }
    }
}

void KeyPressMappingSet::removeKeyPress (const CommandID commandID,
                                         const int keyPressIndex) throw()
{
    for (int i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked(i)->commandID == commandID)
        {
            mappings.getUnchecked(i)->keypresses.remove (keyPressIndex);
            sendChangeMessage (this);
            break;
        }
    }
}

//==============================================================================
CommandID KeyPressMappingSet::findCommandForKeyPress (const KeyPress& keyPress) const throw()
{
    for (int i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked(i)->keypresses.contains (keyPress))
            return mappings.getUnchecked(i)->commandID;

    return 0;
}

bool KeyPressMappingSet::containsMapping (const CommandID commandID,
                                          const KeyPress& keyPress) const throw()
{
    for (int i = mappings.size(); --i >= 0;)
        if (mappings.getUnchecked(i)->commandID == commandID)
            return mappings.getUnchecked(i)->keypresses.contains (keyPress);

    return false;
}

void KeyPressMappingSet::invokeCommand (const CommandID commandID,
                                        const KeyPress& key,
                                        const bool isKeyDown,
                                        const int millisecsSinceKeyPressed,
                                        Component* const originatingComponent) const
{
    ApplicationCommandTarget::InvocationInfo info (commandID);

    info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromKeyPress;
    info.isKeyDown = isKeyDown;
    info.keyPress = key;
    info.millisecsSinceKeyPressed = millisecsSinceKeyPressed;
    info.originatingComponent = originatingComponent;

    commandManager->invoke (info, false);
}

//==============================================================================
bool KeyPressMappingSet::restoreFromXml (const XmlElement& xmlVersion)
{
    if (xmlVersion.hasTagName (T("KEYMAPPINGS")))
    {
        if (xmlVersion.getBoolAttribute (T("basedOnDefaults"), true))
        {
            // if the XML was created as a set of differences from the default mappings,
            // (i.e. by calling createXml (true)), then we need to first restore the defaults.
            resetToDefaultMappings();
        }
        else
        {
            // if the XML was created calling createXml (false), then we need to clear all
            // the keys and treat the xml as describing the entire set of mappings.
            clearAllKeyPresses();
        }

        forEachXmlChildElement (xmlVersion, map)
        {
            const CommandID commandId = map->getStringAttribute (T("commandId")).getHexValue32();

            if (commandId != 0)
            {
                const KeyPress key (KeyPress::createFromDescription (map->getStringAttribute (T("key"))));

                if (map->hasTagName (T("MAPPING")))
                {
                    addKeyPress (commandId, key);
                }
                else if (map->hasTagName (T("UNMAPPING")))
                {
                    if (containsMapping (commandId, key))
                        removeKeyPress (key);
                }
            }
        }

        return true;
    }

    return false;
}

XmlElement* KeyPressMappingSet::createXml (const bool saveDifferencesFromDefaultSet) const
{
    KeyPressMappingSet* defaultSet = 0;

    if (saveDifferencesFromDefaultSet)
    {
        defaultSet = new KeyPressMappingSet (commandManager);
        defaultSet->resetToDefaultMappings();
    }

    XmlElement* const doc = new XmlElement (T("KEYMAPPINGS"));

    doc->setAttribute (T("basedOnDefaults"), saveDifferencesFromDefaultSet);

    int i;
    for (i = 0; i < mappings.size(); ++i)
    {
        const CommandMapping* const cm = mappings.getUnchecked(i);

        for (int j = 0; j < cm->keypresses.size(); ++j)
        {
            if (defaultSet == 0
                 || ! defaultSet->containsMapping (cm->commandID, cm->keypresses.getReference (j)))
            {
                XmlElement* const map = new XmlElement (T("MAPPING"));

                map->setAttribute (T("commandId"), String::toHexString ((int) cm->commandID));
                map->setAttribute (T("description"), commandManager->getDescriptionOfCommand (cm->commandID));
                map->setAttribute (T("key"), cm->keypresses.getReference (j).getTextDescription());

                doc->addChildElement (map);
            }
        }
    }

    if (defaultSet != 0)
    {
        for (i = 0; i < defaultSet->mappings.size(); ++i)
        {
            const CommandMapping* const cm = defaultSet->mappings.getUnchecked(i);

            for (int j = 0; j < cm->keypresses.size(); ++j)
            {
                if (! containsMapping (cm->commandID, cm->keypresses.getReference (j)))
                {
                    XmlElement* const map = new XmlElement (T("UNMAPPING"));

                    map->setAttribute (T("commandId"), String::toHexString ((int) cm->commandID));
                    map->setAttribute (T("description"), commandManager->getDescriptionOfCommand (cm->commandID));
                    map->setAttribute (T("key"), cm->keypresses.getReference (j).getTextDescription());

                    doc->addChildElement (map);
                }
            }
        }

        delete defaultSet;
    }

    return doc;
}

//==============================================================================
bool KeyPressMappingSet::keyPressed (const KeyPress& key,
                                     Component* originatingComponent)
{
    bool used = false;

    const CommandID commandID = findCommandForKeyPress (key);

    const ApplicationCommandInfo* const ci = commandManager->getCommandForID (commandID);

    if (ci != 0
         && (ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) == 0)
    {
        ApplicationCommandInfo info (0);

        if (commandManager->getTargetForCommand (commandID, info) != 0
             && (info.flags & ApplicationCommandInfo::isDisabled) == 0)
        {
            invokeCommand (commandID, key, true, 0, originatingComponent);
            used = true;
        }
        else
        {
            PlatformUtilities::beep();
        }
    }

    return used;
}

bool KeyPressMappingSet::keyStateChanged (Component* originatingComponent)
{
    bool used = false;
    const uint32 now = Time::getMillisecondCounter();

    for (int i = mappings.size(); --i >= 0;)
    {
        CommandMapping* const cm =  mappings.getUnchecked(i);

        if (cm->wantsKeyUpDownCallbacks)
        {
            for (int j = cm->keypresses.size(); --j >= 0;)
            {
                const KeyPress key (cm->keypresses.getReference (j));
                const bool isDown = key.isCurrentlyDown();

                int keyPressEntryIndex = 0;
                bool wasDown = false;

                for (int k = keysDown.size(); --k >= 0;)
                {
                    if (key == keysDown.getUnchecked(k)->key)
                    {
                        keyPressEntryIndex = k;
                        wasDown = true;
                        break;
                    }
                }

                if (isDown != wasDown)
                {
                    int millisecs = 0;

                    if (isDown)
                    {
                        KeyPressTime* const k = new KeyPressTime();
                        k->key = key;
                        k->timeWhenPressed = now;

                        keysDown.add (k);
                    }
                    else
                    {
                        const uint32 pressTime = keysDown.getUnchecked (keyPressEntryIndex)->timeWhenPressed;

                        if (now > pressTime)
                            millisecs = now - pressTime;

                        keysDown.remove (keyPressEntryIndex);
                    }

                    invokeCommand (cm->commandID, key, isDown, millisecs, originatingComponent);
                    used = true;
                }
            }
        }
    }

    return used;
}

void KeyPressMappingSet::globalFocusChanged (Component* focusedComponent)
{
    if (focusedComponent != 0)
        focusedComponent->keyStateChanged();
}


END_JUCE_NAMESPACE
