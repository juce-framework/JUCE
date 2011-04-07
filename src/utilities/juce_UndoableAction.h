/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_UNDOABLEACTION_JUCEHEADER__
#define __JUCE_UNDOABLEACTION_JUCEHEADER__


//==============================================================================
/**
    Used by the UndoManager class to store an action which can be done
    and undone.

    @see UndoManager
*/
class JUCE_API  UndoableAction
{
protected:
    /** Creates an action. */
    UndoableAction() noexcept   {}

public:
    /** Destructor. */
    virtual ~UndoableAction()   {}

    //==============================================================================
    /** Overridden by a subclass to perform the action.

        This method is called by the UndoManager, and shouldn't be used directly by
        applications.

        Be careful not to make any calls in a perform() method that could call
        recursively back into the UndoManager::perform() method

        @returns    true if the action could be performed.
        @see UndoManager::perform
    */
    virtual bool perform() = 0;

    /** Overridden by a subclass to undo the action.

        This method is called by the UndoManager, and shouldn't be used directly by
        applications.

        Be careful not to make any calls in an undo() method that could call
        recursively back into the UndoManager::perform() method

        @returns    true if the action could be undone without any errors.
        @see UndoManager::perform
    */
    virtual bool undo() = 0;

    //==============================================================================
    /** Returns a value to indicate how much memory this object takes up.

        Because the UndoManager keeps a list of UndoableActions, this is used
        to work out how much space each one will take up, so that the UndoManager
        can work out how many to keep.

        The default value returned here is 10 - units are arbitrary and
        don't have to be accurate.

        @see UndoManager::getNumberOfUnitsTakenUpByStoredCommands,
             UndoManager::setMaxNumberOfStoredUnits
    */
    virtual int getSizeInUnits()    { return 10; }

    /** Allows multiple actions to be coalesced into a single action object, to reduce storage space.

        If possible, this method should create and return a single action that does the same job as
        this one followed by the supplied action.

        If it's not possible to merge the two actions, the method should return zero.
    */
    virtual UndoableAction* createCoalescedAction (UndoableAction* nextAction)  { (void) nextAction; return nullptr; }
};


#endif   // __JUCE_UNDOABLEACTION_JUCEHEADER__
