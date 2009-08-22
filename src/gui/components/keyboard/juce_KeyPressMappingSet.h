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

#ifndef __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
#define __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__

#include "juce_KeyListener.h"
#include "../../../application/juce_ApplicationCommandManager.h"
#include "../../../events/juce_ChangeBroadcaster.h"
#include "../../../text/juce_StringArray.h"
#include "../../../text/juce_XmlElement.h"
#include "../../../containers/juce_OwnedArray.h"


//==============================================================================
/**
    Manages and edits a list of keypresses, which it uses to invoke the appropriate
    command in a ApplicationCommandManager.

    Normally, you won't actually create a KeyPressMappingSet directly, because
    each ApplicationCommandManager contains its own KeyPressMappingSet, so typically
    you'd create yourself an ApplicationCommandManager, and call its
    ApplicationCommandManager::getKeyMappings() method to get a pointer to its
    KeyPressMappingSet.

    For one of these to actually use keypresses, you'll need to add it as a KeyListener
    to the top-level component for which you want to handle keystrokes. So for example:

    @code
    class MyMainWindow  : public Component
    {
        ApplicationCommandManager* myCommandManager;

    public:
        MyMainWindow()
        {
            myCommandManager = new ApplicationCommandManager();

            // first, make sure the command manager has registered all the commands that its
            // targets can perform..
            myCommandManager->registerAllCommandsForTarget (myCommandTarget1);
            myCommandManager->registerAllCommandsForTarget (myCommandTarget2);

            // this will use the command manager to initialise the KeyPressMappingSet with
            // the default keypresses that were specified when the targets added their commands
            // to the manager.
            myCommandManager->getKeyMappings()->resetToDefaultMappings();

            // having set up the default key-mappings, you might now want to load the last set
            // of mappings that the user configured.
            myCommandManager->getKeyMappings()->restoreFromXml (lastSavedKeyMappingsXML);

            // Now tell our top-level window to send any keypresses that arrive to the
            // KeyPressMappingSet, which will use them to invoke the appropriate commands.
            addKeyListener (myCommandManager->getKeyMappings());
        }

        ...
    }
    @endcode

    KeyPressMappingSet derives from ChangeBroadcaster so that interested parties can
    register to be told when a command or mapping is added, removed, etc.

    There's also a UI component called KeyMappingEditorComponent that can be used
    to easily edit the key mappings.

    @see Component::addKeyListener(), KeyMappingEditorComponent, ApplicationCommandManager
*/
class JUCE_API  KeyPressMappingSet  : public KeyListener,
                                      public ChangeBroadcaster,
                                      public FocusChangeListener
{
public:
    //==============================================================================
    /** Creates a KeyPressMappingSet for a given command manager.

        Normally, you won't actually create a KeyPressMappingSet directly, because
        each ApplicationCommandManager contains its own KeyPressMappingSet, so the
        best thing to do is to create your ApplicationCommandManager, and use the
        ApplicationCommandManager::getKeyMappings() method to access its mappings.

        When a suitable keypress happens, the manager's invoke() method will be
        used to invoke the appropriate command.

        @see ApplicationCommandManager
    */
    KeyPressMappingSet (ApplicationCommandManager* const commandManager) throw();

    /** Creates an copy of a KeyPressMappingSet. */
    KeyPressMappingSet (const KeyPressMappingSet& other) throw();

    /** Destructor. */
    ~KeyPressMappingSet();

    //==============================================================================
    ApplicationCommandManager* getCommandManager() const throw()        { return commandManager; }

    //==============================================================================
    /** Returns a list of keypresses that are assigned to a particular command.

        @param commandID        the command's ID
    */
    const Array <KeyPress> getKeyPressesAssignedToCommand (const CommandID commandID) const throw();

    /** Assigns a keypress to a command.

        If the keypress is already assigned to a different command, it will first be
        removed from that command, to avoid it triggering multiple functions.

        @param commandID    the ID of the command that you want to add a keypress to. If
                            this is 0, the keypress will be removed from anything that it
                            was previously assigned to, but not re-assigned
        @param newKeyPress  the new key-press
        @param insertIndex  if this is less than zero, the key will be appended to the
                            end of the list of keypresses; otherwise the new keypress will
                            be inserted into the existing list at this index
    */
    void addKeyPress (const CommandID commandID,
                      const KeyPress& newKeyPress,
                      int insertIndex = -1) throw();

    /** Reset all mappings to the defaults, as dictated by the ApplicationCommandManager.

        @see resetToDefaultMapping
    */
    void resetToDefaultMappings() throw();

    /** Resets all key-mappings to the defaults for a particular command.

        @see resetToDefaultMappings
    */
    void resetToDefaultMapping (const CommandID commandID) throw();

    /** Removes all keypresses that are assigned to any commands. */
    void clearAllKeyPresses() throw();

    /** Removes all keypresses that are assigned to a particular command. */
    void clearAllKeyPresses (const CommandID commandID) throw();

    /** Removes one of the keypresses that are assigned to a command.

        See the getKeyPressesAssignedToCommand() for the list of keypresses to
        which the keyPressIndex refers.
    */
    void removeKeyPress (const CommandID commandID,
                         const int keyPressIndex) throw();

    /** Removes a keypress from any command that it may be assigned to.
    */
    void removeKeyPress (const KeyPress& keypress) throw();

    /** Returns true if the given command is linked to this key. */
    bool containsMapping (const CommandID commandID,
                          const KeyPress& keyPress) const throw();

    //==============================================================================
    /** Looks for a command that corresponds to a keypress.

        @returns the UID of the command or 0 if none was found
    */
    CommandID findCommandForKeyPress (const KeyPress& keyPress) const throw();

    //==============================================================================
    /** Tries to recreate the mappings from a previously stored state.

        The XML passed in must have been created by the createXml() method.

        If the stored state makes any reference to commands that aren't
        currently available, these will be ignored.

        If the set of mappings being loaded was a set of differences (using createXml (true)),
        then this will call resetToDefaultMappings() and then merge the saved mappings
        on top. If the saved set was created with createXml (false), then this method
        will first clear all existing mappings and load the saved ones as a complete set.

        @returns true if it manages to load the XML correctly
        @see createXml
    */
    bool restoreFromXml (const XmlElement& xmlVersion);

    /** Creates an XML representation of the current mappings.

        This will produce a lump of XML that can be later reloaded using
        restoreFromXml() to recreate the current mapping state.

        The object that is returned must be deleted by the caller.

        @param saveDifferencesFromDefaultSet    if this is false, then all keypresses
                            will be saved into the XML. If it's true, then the XML will
                            only store the differences between the current mappings and
                            the default mappings you'd get from calling resetToDefaultMappings().
                            The advantage of saving a set of differences from the default is that
                            if you change the default mappings (in a new version of your app, for
                            example), then these will be merged into a user's saved preferences.

        @see restoreFromXml
    */
    XmlElement* createXml (const bool saveDifferencesFromDefaultSet) const;

    //==============================================================================
    /** @internal */
    bool keyPressed (const KeyPress& key, Component* originatingComponent);
    /** @internal */
    bool keyStateChanged (const bool isKeyDown, Component* originatingComponent);
    /** @internal */
    void globalFocusChanged (Component* focusedComponent);


    //==============================================================================
    juce_UseDebuggingNewOperator


private:
    //==============================================================================
    ApplicationCommandManager* commandManager;

    struct CommandMapping
    {
        CommandID commandID;
        Array <KeyPress> keypresses;
        bool wantsKeyUpDownCallbacks;
    };

    OwnedArray <CommandMapping> mappings;

    struct KeyPressTime
    {
        KeyPress key;
        uint32 timeWhenPressed;
    };

    OwnedArray <KeyPressTime> keysDown;

    void handleMessage (const Message& message);

    void invokeCommand (const CommandID commandID,
                        const KeyPress& keyPress,
                        const bool isKeyDown,
                        const int millisecsSinceKeyPressed,
                        Component* const originatingComponent) const;

    const KeyPressMappingSet& operator= (const KeyPressMappingSet&);
};


#endif   // __JUCE_KEYPRESSMAPPINGSET_JUCEHEADER__
