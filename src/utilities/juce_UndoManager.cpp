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


#include "juce_UndoManager.h"
#include "../application/juce_Application.h"


//==============================================================================
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
    sendChangeMessage (this);
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
bool UndoManager::perform (UndoableAction* const command, const String& actionName)
{
    if (command != 0)
    {
        if (actionName.isNotEmpty())
            currentTransactionName = actionName;

        if (reentrancyCheck)
        {
            jassertfalse    // don't call perform() recursively from the UndoableAction::perform() or
                            // undo() methods, or else these actions won't actually get done.

            return false;
        }
        else
        {
            bool success = false;

            JUCE_TRY
            {
                success = command->perform();
            }
            JUCE_CATCH_EXCEPTION

            jassert (success);
            if (success)
            {
                if (nextIndex > 0 && ! newTransaction)
                {
                    OwnedArray<UndoableAction>* commandSet = transactions [nextIndex - 1];

                    jassert (commandSet != 0);
                    if (commandSet == 0)
                        return false;

                    commandSet->add (command);
                }
                else
                {
                    OwnedArray<UndoableAction>* commandSet = new OwnedArray<UndoableAction>();
                    commandSet->add (command);
                    transactions.insert (nextIndex, commandSet);
                    transactionNames.insert (nextIndex, currentTransactionName);
                    ++nextIndex;
                }

                totalUnitsStored += command->getSizeInUnits();
                newTransaction = false;
            }

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

            sendChangeMessage (this);

            return success;
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

const String UndoManager::getUndoDescription() const
{
    return transactionNames [nextIndex - 1];
}

const String UndoManager::getRedoDescription() const
{
    return transactionNames [nextIndex];
}

bool UndoManager::undo()
{
    const OwnedArray<UndoableAction>* const commandSet = transactions [nextIndex - 1];

    if (commandSet == 0)
        return false;

    reentrancyCheck = true;
    bool failed = false;

    for (int i = commandSet->size(); --i >= 0;)
    {
        if (! commandSet->getUnchecked(i)->undo())
        {
            jassertfalse
            failed = true;
            break;
        }
    }

    reentrancyCheck = false;

    if (failed)
    {
        clearUndoHistory();
    }
    else
    {
        --nextIndex;
    }

    beginNewTransaction();

    sendChangeMessage (this);
    return true;
}

bool UndoManager::redo()
{
    const OwnedArray<UndoableAction>* const commandSet = transactions [nextIndex];

    if (commandSet == 0)
        return false;

    reentrancyCheck = true;
    bool failed = false;

    for (int i = 0; i < commandSet->size(); ++i)
    {
        if (! commandSet->getUnchecked(i)->perform())
        {
            jassertfalse
            failed = true;
            break;
        }
    }

    reentrancyCheck = false;

    if (failed)
    {
        clearUndoHistory();
    }
    else
    {
        ++nextIndex;
    }

    beginNewTransaction();

    sendChangeMessage (this);
    return true;
}

bool UndoManager::undoCurrentTransactionOnly()
{
    return newTransaction ? false
                          : undo();
}

void UndoManager::getActionsInCurrentTransaction (Array <const UndoableAction*>& actionsFound) const
{
    const OwnedArray <UndoableAction>* const commandSet = transactions [nextIndex - 1];

    if (commandSet != 0 && ! newTransaction)
    {
        for (int i = 0; i < commandSet->size(); ++i)
            actionsFound.add (commandSet->getUnchecked(i));
    }
}


END_JUCE_NAMESPACE
