/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

KeyPressMappingSet::KeyPressMappingSet (ApplicationCommandManager& cm)
    : commandManager (cm)
{
    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::KeyPressMappingSet (const KeyPressMappingSet& other)
    : KeyListener(), ChangeBroadcaster(), FocusChangeListener(), commandManager (other.commandManager)
{
    Desktop::getInstance().addFocusChangeListener (this);
}

KeyPressMappingSet::~KeyPressMappingSet()
{
    Desktop::getInstance().removeFocusChangeListener (this);
}

//==============================================================================
Array<KeyPress> KeyPressMappingSet::getKeyPressesAssignedToCommand (const CommandID commandID) const
{
    for (int i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked(i)->commandID == commandID)
            return mappings.getUnchecked (i)->keypresses;

    return {};
}

void KeyPressMappingSet::addKeyPress (const CommandID commandID, const KeyPress& newKeyPress, int insertIndex)
{
    // If you specify an upper-case letter but no shift key, how is the user supposed to press it!?
    // Stick to lower-case letters when defining a keypress, to avoid ambiguity.
    jassert (! (CharacterFunctions::isUpperCase (newKeyPress.getTextCharacter())
                 && ! newKeyPress.getModifiers().isShiftDown()));

    if (findCommandForKeyPress (newKeyPress) != commandID)
    {
        if (newKeyPress.isValid())
        {
            for (int i = mappings.size(); --i >= 0;)
            {
                if (mappings.getUnchecked(i)->commandID == commandID)
                {
                    mappings.getUnchecked(i)->keypresses.insert (insertIndex, newKeyPress);

                    sendChangeMessage();
                    return;
                }
            }

            if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (commandID))
            {
                CommandMapping* const cm = new CommandMapping();
                cm->commandID = commandID;
                cm->keypresses.add (newKeyPress);
                cm->wantsKeyUpDownCallbacks = (ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) != 0;

                mappings.add (cm);
                sendChangeMessage();
            }
            else
            {
                // If you hit this, you're trying to attach a keypress to a command ID that
                // doesn't exist, so the key is not being attached.
                jassertfalse;
            }
        }
    }
}

static void addKeyPresses (KeyPressMappingSet& set, const ApplicationCommandInfo* const ci)
{
    for (int j = 0; j < ci->defaultKeypresses.size(); ++j)
        set.addKeyPress (ci->commandID, ci->defaultKeypresses.getReference (j));
}

void KeyPressMappingSet::resetToDefaultMappings()
{
    mappings.clear();

    for (int i = 0; i < commandManager.getNumCommands(); ++i)
        addKeyPresses (*this, commandManager.getCommandForIndex (i));

    sendChangeMessage();
}

void KeyPressMappingSet::resetToDefaultMapping (const CommandID commandID)
{
    clearAllKeyPresses (commandID);

    if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (commandID))
        addKeyPresses (*this, ci);
}

void KeyPressMappingSet::clearAllKeyPresses()
{
    if (mappings.size() > 0)
    {
        sendChangeMessage();
        mappings.clear();
    }
}

void KeyPressMappingSet::clearAllKeyPresses (const CommandID commandID)
{
    for (int i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked(i)->commandID == commandID)
        {
            mappings.remove (i);
            sendChangeMessage();
        }
    }
}

void KeyPressMappingSet::removeKeyPress (const KeyPress& keypress)
{
    if (keypress.isValid())
    {
        for (int i = mappings.size(); --i >= 0;)
        {
            CommandMapping& cm = *mappings.getUnchecked(i);

            for (int j = cm.keypresses.size(); --j >= 0;)
            {
                if (keypress == cm.keypresses [j])
                {
                    cm.keypresses.remove (j);
                    sendChangeMessage();
                }
            }
        }
    }
}

void KeyPressMappingSet::removeKeyPress (const CommandID commandID, const int keyPressIndex)
{
    for (int i = mappings.size(); --i >= 0;)
    {
        if (mappings.getUnchecked(i)->commandID == commandID)
        {
            mappings.getUnchecked(i)->keypresses.remove (keyPressIndex);
            sendChangeMessage();
            break;
        }
    }
}

//==============================================================================
CommandID KeyPressMappingSet::findCommandForKeyPress (const KeyPress& keyPress) const noexcept
{
    for (int i = 0; i < mappings.size(); ++i)
        if (mappings.getUnchecked(i)->keypresses.contains (keyPress))
            return mappings.getUnchecked(i)->commandID;

    return 0;
}

bool KeyPressMappingSet::containsMapping (const CommandID commandID, const KeyPress& keyPress) const noexcept
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

    commandManager.invoke (info, false);
}

//==============================================================================
bool KeyPressMappingSet::restoreFromXml (const XmlElement& xmlVersion)
{
    if (xmlVersion.hasTagName ("KEYMAPPINGS"))
    {
        if (xmlVersion.getBoolAttribute ("basedOnDefaults", true))
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
            const CommandID commandId = map->getStringAttribute ("commandId").getHexValue32();

            if (commandId != 0)
            {
                auto key = KeyPress::createFromDescription (map->getStringAttribute ("key"));

                if (map->hasTagName ("MAPPING"))
                {
                    addKeyPress (commandId, key);
                }
                else if (map->hasTagName ("UNMAPPING"))
                {
                    for (auto& m : mappings)
                        if (m->commandID == commandId)
                            m->keypresses.removeAllInstancesOf (key);
                }
            }
        }

        return true;
    }

    return false;
}

std::unique_ptr<XmlElement> KeyPressMappingSet::createXml (const bool saveDifferencesFromDefaultSet) const
{
    std::unique_ptr<KeyPressMappingSet> defaultSet;

    if (saveDifferencesFromDefaultSet)
    {
        defaultSet = std::make_unique<KeyPressMappingSet> (commandManager);
        defaultSet->resetToDefaultMappings();
    }

    auto doc = std::make_unique<XmlElement> ("KEYMAPPINGS");

    doc->setAttribute ("basedOnDefaults", saveDifferencesFromDefaultSet);

    for (int i = 0; i < mappings.size(); ++i)
    {
        auto& cm = *mappings.getUnchecked(i);

        for (int j = 0; j < cm.keypresses.size(); ++j)
        {
            if (defaultSet == nullptr
                 || ! defaultSet->containsMapping (cm.commandID, cm.keypresses.getReference (j)))
            {
                auto map = doc->createNewChildElement ("MAPPING");

                map->setAttribute ("commandId", String::toHexString ((int) cm.commandID));
                map->setAttribute ("description", commandManager.getDescriptionOfCommand (cm.commandID));
                map->setAttribute ("key", cm.keypresses.getReference (j).getTextDescription());
            }
        }
    }

    if (defaultSet != nullptr)
    {
        for (int i = 0; i < defaultSet->mappings.size(); ++i)
        {
            auto& cm = *defaultSet->mappings.getUnchecked(i);

            for (int j = 0; j < cm.keypresses.size(); ++j)
            {
                if (! containsMapping (cm.commandID, cm.keypresses.getReference (j)))
                {
                    auto map = doc->createNewChildElement ("UNMAPPING");

                    map->setAttribute ("commandId", String::toHexString ((int) cm.commandID));
                    map->setAttribute ("description", commandManager.getDescriptionOfCommand (cm.commandID));
                    map->setAttribute ("key", cm.keypresses.getReference (j).getTextDescription());
                }
            }
        }
    }

    return doc;
}

//==============================================================================
bool KeyPressMappingSet::keyPressed (const KeyPress& key, Component* const originatingComponent)
{
    bool commandWasDisabled = false;

    for (int i = 0; i < mappings.size(); ++i)
    {
        CommandMapping& cm = *mappings.getUnchecked(i);

        if (cm.keypresses.contains (key))
        {
            if (const ApplicationCommandInfo* const ci = commandManager.getCommandForID (cm.commandID))
            {
                if ((ci->flags & ApplicationCommandInfo::wantsKeyUpDownCallbacks) == 0)
                {
                    ApplicationCommandInfo info (0);

                    if (commandManager.getTargetForCommand (cm.commandID, info) != nullptr)
                    {
                        if ((info.flags & ApplicationCommandInfo::isDisabled) == 0)
                        {
                            invokeCommand (cm.commandID, key, true, 0, originatingComponent);
                            return true;
                        }

                        commandWasDisabled = true;
                    }
                }
            }
        }
    }

    if (originatingComponent != nullptr && commandWasDisabled)
        originatingComponent->getLookAndFeel().playAlertSound();

    return false;
}

bool KeyPressMappingSet::keyStateChanged (const bool /*isKeyDown*/, Component* originatingComponent)
{
    bool used = false;
    const uint32 now = Time::getMillisecondCounter();

    for (int i = mappings.size(); --i >= 0;)
    {
        CommandMapping& cm = *mappings.getUnchecked(i);

        if (cm.wantsKeyUpDownCallbacks)
        {
            for (int j = cm.keypresses.size(); --j >= 0;)
            {
                const KeyPress key (cm.keypresses.getReference (j));
                const bool isDown = key.isCurrentlyDown();

                int keyPressEntryIndex = 0;
                bool wasDown = false;

                for (int k = keysDown.size(); --k >= 0;)
                {
                    if (key == keysDown.getUnchecked(k)->key)
                    {
                        keyPressEntryIndex = k;
                        wasDown = true;
                        used = true;
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
                            millisecs = (int) (now - pressTime);

                        keysDown.remove (keyPressEntryIndex);
                    }

                    invokeCommand (cm.commandID, key, isDown, millisecs, originatingComponent);
                    used = true;
                }
            }
        }
    }

    return used;
}

void KeyPressMappingSet::globalFocusChanged (Component* focusedComponent)
{
    if (focusedComponent != nullptr)
        focusedComponent->keyStateChanged (false);
}

} // namespace juce
