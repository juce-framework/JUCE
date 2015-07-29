/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_APPLICATIONCOMMANDID_H_INCLUDED
#define JUCE_APPLICATIONCOMMANDID_H_INCLUDED


//==============================================================================
/** A type used to hold the unique ID for an application command.

    This is a numeric type, so it can be stored as an integer.

    @see ApplicationCommandInfo, ApplicationCommandManager,
         ApplicationCommandTarget, KeyPressMappingSet
*/
typedef int CommandID;


//==============================================================================
/** A set of general-purpose application command IDs.

    Because these commands are likely to be used in most apps, they're defined
    here to help different apps to use the same numeric values for them.

    Of course you don't have to use these, but some of them are used internally by
    Juce - e.g. the quit ID is recognised as a command by the JUCEApplication class.

    @see ApplicationCommandInfo, ApplicationCommandManager,
         ApplicationCommandTarget, KeyPressMappingSet
*/
namespace StandardApplicationCommandIDs
{
    enum
    {
        /** This command ID should be used to send a "Quit the App" command.

            This command is recognised by the JUCEApplication class, so if it is invoked
            and no other ApplicationCommandTarget handles the event first, the JUCEApplication
            object will catch it and call JUCEApplicationBase::systemRequestedQuit().
        */
        quit           = 0x1001,

        /** The command ID that should be used to send a "Delete" command. */
        del            = 0x1002,

        /** The command ID that should be used to send a "Cut" command. */
        cut            = 0x1003,

        /** The command ID that should be used to send a "Copy to clipboard" command. */
        copy           = 0x1004,

        /** The command ID that should be used to send a "Paste from clipboard" command. */
        paste          = 0x1005,

        /** The command ID that should be used to send a "Select all" command. */
        selectAll      = 0x1006,

        /** The command ID that should be used to send a "Deselect all" command. */
        deselectAll    = 0x1007,

        /** The command ID that should be used to send a "undo" command. */
        undo           = 0x1008,

        /** The command ID that should be used to send a "redo" command. */
        redo           = 0x1009
    };
}


#endif   // JUCE_APPLICATIONCOMMANDID_H_INCLUDED
