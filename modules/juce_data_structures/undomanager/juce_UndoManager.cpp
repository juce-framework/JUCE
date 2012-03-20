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

UndoManager::UndoManager (const int maxNumberOfUnitsToKeep,
                          const int minimumTransactions)
   : totalUnitsStored (0),
     nextIndex (0),
     newTransaction (true),
     reentrancyCheck (false)
{
    setMaxNumberOfStoredUnits (maxNumberOfUnitsToKeep,
                               minimumTransactions);
}

UndoManager::~UndoManager()
{
    clearUndoHistory();
}

//==============================================================================
void UndoManager::clearUndoHistory()
{
    transactions.clear();
    transactionNames.clear();
    totalUnitsStored = 0;
    nextIndex = 0;
    sendChangeMessage();
}

int UndoManager::getNumberOfUnitsTakenUpByStoredCommands() const
{
    return totalUnitsStored;
}

void UndoManager::setMaxNumberOfStoredUnits (const int maxNumberOfUnitsToKeep,
                                             const int minimumTransactions)
{
    maxNumUnitsToKeep          = jmax (1, maxNumberOfUnitsToKeep);
    minimumTransactionsToKeep  = jmax (1, minimumTransactions);
}

//==============================================================================
bool UndoManager::perform (UndoableAction* const command_, const String& actionName)
{
    if (command_ != nullptr)
    {
        ScopedPointer<UndoableAction> command (command_);

        if (actionName.isNotEmpty())
            currentTransactionName = actionName;

        if (reentrancyCheck)
        {
            jassertfalse;    // don't call perform() recursively from the UndoableAction::perform() or
                             // undo() methods, or else these actions won't actually get done.

            return false;
        }
        else if (command->perform())
        {
            OwnedArray<UndoableAction>* commandSet = transactions [nextIndex - 1];

            if (commandSet != nullptr && ! newTransaction)
            {
                UndoableAction* lastAction = commandSet->getLast();

                if (lastAction != nullptr)
                {
                    UndoableAction* coalescedAction = lastAction->createCoalescedAction (command);

                    if (coalescedAction != nullptr)
                    {
                        command = coalescedAction;
                        totalUnitsStored -= lastAction->getSizeInUnits();
                        commandSet->removeLast();
                    }
                }
            }
            else
            {
                commandSet = new OwnedArray<UndoableAction>();
                transactions.insert (nextIndex, commandSet);
                transactionNames.insert (nextIndex, currentTransactionName);
                ++nextIndex;
            }

            totalUnitsStored += command->getSizeInUnits();
            commandSet->add (command.release());
            newTransaction = false;

            while (nextIndex < transactions.size())
            {
                const OwnedArray <UndoableAction>* const lastSet = transactions.getLast();

                for (int i = lastSet->size(); --i >= 0;)
                    totalUnitsStored -= lastSet->getUnchecked (i)->getSizeInUnits();

                transactions.removeLast();
                transactionNames.remove (transactionNames.size() - 1);
            }

            while (nextIndex > 0
                   && totalUnitsStored > maxNumUnitsToKeep
                   && transactions.size() > minimumTransactionsToKeep)
            {
                const OwnedArray <UndoableAction>* const firstSet = transactions.getFirst();

                for (int i = firstSet->size(); --i >= 0;)
                    totalUnitsStored -= firstSet->getUnchecked (i)->getSizeInUnits();

                jassert (totalUnitsStored >= 0); // something fishy going on if this fails!

                transactions.remove (0);
                transactionNames.remove (0);
                --nextIndex;
            }

            sendChangeMessage();

            return true;
        }
    }

    return false;
}

void UndoManager::beginNewTransaction (const String& actionName)
{
    newTransaction = true;
    currentTransactionName = actionName;
}

void UndoManager::setCurrentTransactionName (const String& newName)
{
    currentTransactionName = newName;
}

//==============================================================================
bool UndoManager::canUndo() const
{
    return nextIndex > 0;
}

bool UndoManager::canRedo() const
{
    return nextIndex < transactions.size();
}

String UndoManager::getUndoDescription() const
{
    return transactionNames [nextIndex - 1];
}

String UndoManager::getRedoDescription() const
{
    return transactionNames [nextIndex];
}

bool UndoManager::undo()
{
    const OwnedArray<UndoableAction>* const commandSet = transactions [nextIndex - 1];

    if (commandSet == nullptr)
        return false;

    bool failed = false;

    {
        const ScopedValueSetter<bool> setter (reentrancyCheck, true);

        for (int i = commandSet->size(); --i >= 0;)
        {
            if (! commandSet->getUnchecked(i)->undo())
            {
                jassertfalse;
                failed = true;
                break;
            }
        }
    }

    if (failed)
        clearUndoHistory();
    else
        --nextIndex;

    beginNewTransaction();

    sendChangeMessage();
    return true;
}

bool UndoManager::redo()
{
    const OwnedArray<UndoableAction>* const commandSet = transactions [nextIndex];

    if (commandSet == nullptr)
        return false;

    bool failed = false;

    {
        const ScopedValueSetter<bool> setter (reentrancyCheck, true);

        for (int i = 0; i < commandSet->size(); ++i)
        {
            if (! commandSet->getUnchecked(i)->perform())
            {
                jassertfalse;
                failed = true;
                break;
            }
        }
    }

    if (failed)
        clearUndoHistory();
    else
        ++nextIndex;

    beginNewTransaction();

    sendChangeMessage();
    return true;
}

bool UndoManager::undoCurrentTransactionOnly()
{
    return newTransaction ? false : undo();
}

void UndoManager::getActionsInCurrentTransaction (Array <const UndoableAction*>& actionsFound) const
{
    const OwnedArray <UndoableAction>* const commandSet = transactions [nextIndex - 1];

    if (commandSet != nullptr && ! newTransaction)
    {
        for (int i = 0; i < commandSet->size(); ++i)
            actionsFound.add (commandSet->getUnchecked(i));
    }
}

int UndoManager::getNumActionsInCurrentTransaction() const
{
    const OwnedArray <UndoableAction>* const commandSet = transactions [nextIndex - 1];

    if (commandSet != nullptr && ! newTransaction)
        return commandSet->size();

    return 0;
}
