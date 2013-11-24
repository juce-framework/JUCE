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

#ifndef JUCE_APPLICATIONCOMMANDINFO_H_INCLUDED
#define JUCE_APPLICATIONCOMMANDINFO_H_INCLUDED


//==============================================================================
/**
    Holds information describing an application command.

    This object is used to pass information about a particular command, such as its
    name, description and other usage flags.

    When an ApplicationCommandTarget is asked to provide information about the commands
    it can perform, this is the structure gets filled-in to describe each one.

    @see ApplicationCommandTarget, ApplicationCommandTarget::getCommandInfo(),
         ApplicationCommandManager
*/
struct JUCE_API  ApplicationCommandInfo
{
    //==============================================================================
    explicit ApplicationCommandInfo (CommandID commandID) noexcept;

    //==============================================================================
    /** Sets a number of the structures values at once.

        The meanings of each of the parameters is described below, in the appropriate
        member variable's description.
    */
    void setInfo (const String& shortName,
                  const String& description,
                  const String& categoryName,
                  int flags) noexcept;

    /** An easy way to set or remove the isDisabled bit in the structure's flags field.

        If isActive is true, the flags member has the isDisabled bit cleared; if isActive
        is false, the bit is set.
    */
    void setActive (bool isActive) noexcept;

    /** An easy way to set or remove the isTicked bit in the structure's flags field.
    */
    void setTicked (bool isTicked) noexcept;

    /** Handy method for adding a keypress to the defaultKeypresses array.

        This is just so you can write things like:
        @code
        myinfo.addDefaultKeypress ('s', ModifierKeys::commandModifier);
        @endcode
        instead of
        @code
        myinfo.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier));
        @endcode
    */
    void addDefaultKeypress (int keyCode, ModifierKeys modifiers) noexcept;

    //==============================================================================
    /** The command's unique ID number.
    */
    CommandID commandID;

    /** A short name to describe the command.

        This should be suitable for use in menus, on buttons that trigger the command, etc.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    String shortName;

    /** A longer description of the command.

        This should be suitable for use in contexts such as a KeyMappingEditorComponent or
        pop-up tooltip describing what the command does.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    String description;

    /** A named category that the command fits into.

        You can give your commands any category you like, and these will be displayed in
        contexts such as the KeyMappingEditorComponent, where the category is used to group
        commands together.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    String categoryName;

    /** A list of zero or more keypresses that should be used as the default keys for
        this command.

        Methods such as KeyPressMappingSet::resetToDefaultMappings() will use the keypresses in
        this list to initialise the default set of key-to-command mappings.

        @see addDefaultKeypress
    */
    Array<KeyPress> defaultKeypresses;

    //==============================================================================
    /** Flags describing the ways in which this command should be used.

        A bitwise-OR of these values is stored in the ApplicationCommandInfo::flags
        variable.
    */
    enum CommandFlags
    {
        /** Indicates that the command can't currently be performed.

            The ApplicationCommandTarget::getCommandInfo() method must set this flag if it's
            not currently permissable to perform the command. If the flag is set, then
            components that trigger the command, e.g. PopupMenu, may choose to grey-out the
            command or show themselves as not being enabled.

            @see ApplicationCommandInfo::setActive
        */
        isDisabled                  = 1 << 0,

        /** Indicates that the command should have a tick next to it on a menu.

            If your command is shown on a menu and this is set, it'll show a tick next to
            it. Other components such as buttons may also use this flag to indicate that it
            is a value that can be toggled, and is currently in the 'on' state.

            @see ApplicationCommandInfo::setTicked
        */
        isTicked                    = 1 << 1,

        /** If this flag is present, then when a KeyPressMappingSet invokes the command,
            it will call the command twice, once on key-down and again on key-up.

            @see ApplicationCommandTarget::InvocationInfo
        */
        wantsKeyUpDownCallbacks     = 1 << 2,

        /** If this flag is present, then a KeyMappingEditorComponent will not display the
            command in its list.
        */
        hiddenFromKeyEditor         = 1 << 3,

        /** If this flag is present, then a KeyMappingEditorComponent will display the
            command in its list, but won't allow the assigned keypress to be changed.
        */
        readOnlyInKeyEditor         = 1 << 4,

        /** If this flag is present and the command is invoked from a keypress, then any
            buttons or menus that are also connected to the command will not flash to
            indicate that they've been triggered.
        */
        dontTriggerVisualFeedback   = 1 << 5
    };

    /** A bitwise-OR of the values specified in the CommandFlags enum.

        You can use the setInfo() method to quickly set this and some of the command's
        other properties.
    */
    int flags;
};


#endif   // JUCE_APPLICATIONCOMMANDINFO_H_INCLUDED
