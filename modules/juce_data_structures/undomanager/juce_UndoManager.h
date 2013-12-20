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

#ifndef JUCE_UNDOMANAGER_H_INCLUDED
#define JUCE_UNDOMANAGER_H_INCLUDED


//==============================================================================
/**
    Manages a list of undo/redo commands.

    An UndoManager object keeps a list of past actions and can use these actions
    to move backwards and forwards through an undo history.

    To use it, create subclasses of UndoableAction which perform all the
    actions you need, then when you need to actually perform an action, create one
    and pass it to the UndoManager's perform() method.

    The manager also uses the concept of 'transactions' to group the actions
    together - all actions performed between calls to beginNewTransaction() are
    grouped together and are all undone/redone as a group.

    The UndoManager is a ChangeBroadcaster, so listeners can register to be told
    when actions are performed or undone.

    @see UndoableAction
*/
class JUCE_API  UndoManager  : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an UndoManager.

        @param maxNumberOfUnitsToKeep       each UndoableAction object returns a value
                                            to indicate how much storage it takes up
                                            (UndoableAction::getSizeInUnits()), so this
                                            lets you specify the maximum total number of
                                            units that the undomanager is allowed to
                                            keep in memory before letting the older actions
                                            drop off the end of the list.
        @param minimumTransactionsToKeep    this specifies the minimum number of transactions
                                            that will be kept, even if this involves exceeding
                                            the amount of space specified in maxNumberOfUnitsToKeep
    */
    UndoManager (int maxNumberOfUnitsToKeep = 30000,
                 int minimumTransactionsToKeep = 30);

    /** Destructor. */
    ~UndoManager();

    //==============================================================================
    /** Deletes all stored actions in the list. */
    void clearUndoHistory();

    /** Returns the current amount of space to use for storing UndoableAction objects.
        @see setMaxNumberOfStoredUnits
    */
    int getNumberOfUnitsTakenUpByStoredCommands() const;

    /** Sets the amount of space that can be used for storing UndoableAction objects.

        @param maxNumberOfUnitsToKeep       each UndoableAction object returns a value
                                            to indicate how much storage it takes up
                                            (UndoableAction::getSizeInUnits()), so this
                                            lets you specify the maximum total number of
                                            units that the undomanager is allowed to
                                            keep in memory before letting the older actions
                                            drop off the end of the list.
        @param minimumTransactionsToKeep    this specifies the minimum number of transactions
                                            that will be kept, even if this involves exceeding
                                            the amount of space specified in maxNumberOfUnitsToKeep
        @see getNumberOfUnitsTakenUpByStoredCommands
    */
    void setMaxNumberOfStoredUnits (int maxNumberOfUnitsToKeep,
                                    int minimumTransactionsToKeep);

    //==============================================================================
    /** Performs an action and adds it to the undo history list.

        @param action   the action to perform - this will be deleted by the UndoManager
                        when no longer needed
        @param actionName   if this string is non-empty, the current transaction will be
                            given this name; if it's empty, the current transaction name will
                            be left unchanged. See setCurrentTransactionName()
        @returns true if the command succeeds - see UndoableAction::perform
        @see beginNewTransaction
    */
    bool perform (UndoableAction* action,
                  const String& actionName = String());

    /** Starts a new group of actions that together will be treated as a single transaction.

        All actions that are passed to the perform() method between calls to this
        method are grouped together and undone/redone together by a single call to
        undo() or redo().

        @param actionName   a description of the transaction that is about to be
                            performed
    */
    void beginNewTransaction (const String& actionName = String());

    /** Changes the name stored for the current transaction.

        Each transaction is given a name when the beginNewTransaction() method is
        called, but this can be used to change that name without starting a new
        transaction.
    */
    void setCurrentTransactionName (const String& newName);

    //==============================================================================
    /** Returns true if there's at least one action in the list to undo.
        @see getUndoDescription, undo, canRedo
    */
    bool canUndo() const;

    /** Returns the description of the transaction that would be next to get undone.

        The description returned is the one that was passed into beginNewTransaction
        before the set of actions was performed.

        @see undo
    */
    String getUndoDescription() const;

    /** Tries to roll-back the last transaction.
        @returns    true if the transaction can be undone, and false if it fails, or
                    if there aren't any transactions to undo
    */
    bool undo();

    /** Tries to roll-back any actions that were added to the current transaction.

        This will perform an undo() only if there are some actions in the undo list
        that were added after the last call to beginNewTransaction().

        This is useful because it lets you call beginNewTransaction(), then
        perform an operation which may or may not actually perform some actions, and
        then call this method to get rid of any actions that might have been done
        without it rolling back the previous transaction if nothing was actually
        done.

        @returns true if any actions were undone.
    */
    bool undoCurrentTransactionOnly();

    /** Returns a list of the UndoableAction objects that have been performed during the
        transaction that is currently open.

        Effectively, this is the list of actions that would be undone if undoCurrentTransactionOnly()
        were to be called now.

        The first item in the list is the earliest action performed.
    */
    void getActionsInCurrentTransaction (Array <const UndoableAction*>& actionsFound) const;

    /** Returns the number of UndoableAction objects that have been performed during the
        transaction that is currently open.
        @see getActionsInCurrentTransaction
    */
    int getNumActionsInCurrentTransaction() const;

    /** Returns the time to which the state would be restored if undo() was to be called.
        If an undo isn't currently possible, it'll return Time().
    */
    Time getTimeOfUndoTransaction() const;

    /** Returns the time to which the state would be restored if redo() was to be called.
        If a redo isn't currently possible, it'll return Time::getCurrentTime().
    */
    Time getTimeOfRedoTransaction() const;

    //==============================================================================
    /** Returns true if there's at least one action in the list to redo.
        @see getRedoDescription, redo, canUndo
    */
    bool canRedo() const;

    /** Returns the description of the transaction that would be next to get redone.
        The description returned is the one that was passed into beginNewTransaction
        before the set of actions was performed.

        @see redo
    */
    String getRedoDescription() const;

    /** Tries to redo the last transaction that was undone.
        @returns   true if the transaction can be redone, and false if it fails, or
                   if there aren't any transactions to redo
    */
    bool redo();


private:
    //==============================================================================
    struct ActionSet;
    friend struct ContainerDeletePolicy<ActionSet>;
    OwnedArray<ActionSet> transactions;
    String currentTransactionName;
    int totalUnitsStored, maxNumUnitsToKeep, minimumTransactionsToKeep, nextIndex;
    bool newTransaction, reentrancyCheck;
    ActionSet* getCurrentSet() const noexcept;
    ActionSet* getNextSet() const noexcept;
    void clearFutureTransactions();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UndoManager)
};


#endif   // JUCE_UNDOMANAGER_H_INCLUDED
