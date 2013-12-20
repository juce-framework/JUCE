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

struct UndoManager::ActionSet
{
    ActionSet (const String& transactionName)
        : name (transactionName),
          time (Time::getCurrentTime())
    {}

    OwnedArray <UndoableAction> actions;
    String name;
    Time time;

    bool perform() const
    {
        for (int i = 0; i < actions.size(); ++i)
            if (! actions.getUnchecked(i)->perform())
                return false;

        return true;
    }

    bool undo() const
    {
        for (int i = actions.size(); --i >= 0;)
            if (! actions.getUnchecked(i)->undo())
                return false;

        return true;
    }

    int getTotalSize() const
    {
        int total = 0;

        for (int i = actions.size(); --i >= 0;)
            total += actions.getUnchecked(i)->getSizeInUnits();

        return total;
    }
};

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
}

//==============================================================================
void UndoManager::clearUndoHistory()
{
    transactions.clear();
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
bool UndoManager::perform (UndoableAction* const newAction, const String& actionName)
{
    if (newAction != nullptr)
    {
        ScopedPointer<UndoableAction> action (newAction);

        if (reentrancyCheck)
        {
            jassertfalse;  // don't call perform() recursively from the UndoableAction::perform()
                           // or undo() methods, or else these actions will be discarded!
            return false;
        }

        if (actionName.isNotEmpty())
            currentTransactionName = actionName;

        if (action->perform())
        {
            ActionSet* actionSet = getCurrentSet();

            if (actionSet != nullptr && ! newTransaction)
            {
                if (UndoableAction* const lastAction = actionSet->actions.getLast())
                {
                    if (UndoableAction* const coalescedAction = lastAction->createCoalescedAction (action))
                    {
                        action = coalescedAction;
                        totalUnitsStored -= lastAction->getSizeInUnits();
                        actionSet->actions.removeLast();
                    }
                }
            }
            else
            {
                actionSet = new ActionSet (currentTransactionName);
                transactions.insert (nextIndex, actionSet);
                ++nextIndex;
            }

            totalUnitsStored += action->getSizeInUnits();
            actionSet->actions.add (action.release());
            newTransaction = false;

            clearFutureTransactions();
            sendChangeMessage();
            return true;
        }
    }

    return false;
}

void UndoManager::clearFutureTransactions()
{
    while (nextIndex < transactions.size())
    {
        totalUnitsStored -= transactions.getLast()->getTotalSize();
        transactions.removeLast();
    }

    while (nextIndex > 0
            && totalUnitsStored > maxNumUnitsToKeep
            && transactions.size() > minimumTransactionsToKeep)
    {
        totalUnitsStored -= transactions.getFirst()->getTotalSize();
        transactions.remove (0);
        --nextIndex;

        // if this fails, then some actions may not be returning
        // consistent results from their getSizeInUnits() method
        jassert (totalUnitsStored >= 0);
    }
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
UndoManager::ActionSet* UndoManager::getCurrentSet() const noexcept     { return transactions [nextIndex - 1]; }
UndoManager::ActionSet* UndoManager::getNextSet() const noexcept        { return transactions [nextIndex]; }

bool UndoManager::canUndo() const   { return getCurrentSet() != nullptr; }
bool UndoManager::canRedo() const   { return getNextSet()    != nullptr; }

bool UndoManager::undo()
{
    if (const ActionSet* const s = getCurrentSet())
    {
        const ScopedValueSetter<bool> setter (reentrancyCheck, true);

        if (s->undo())
            --nextIndex;
        else
            clearUndoHistory();

        beginNewTransaction();
        sendChangeMessage();
        return true;
    }

    return false;
}

bool UndoManager::redo()
{
    if (const ActionSet* const s = getNextSet())
    {
        const ScopedValueSetter<bool> setter (reentrancyCheck, true);

        if (s->perform())
            ++nextIndex;
        else
            clearUndoHistory();

        beginNewTransaction();
        sendChangeMessage();
        return true;
    }

    return false;
}

String UndoManager::getUndoDescription() const
{
    if (const ActionSet* const s = getCurrentSet())
        return s->name;

    return String();
}

String UndoManager::getRedoDescription() const
{
    if (const ActionSet* const s = getNextSet())
        return s->name;

    return String();
}

Time UndoManager::getTimeOfUndoTransaction() const
{
    if (const ActionSet* const s = getCurrentSet())
        return s->time;

    return Time();
}

Time UndoManager::getTimeOfRedoTransaction() const
{
    if (const ActionSet* const s = getNextSet())
        return s->time;

    return Time::getCurrentTime();
}

bool UndoManager::undoCurrentTransactionOnly()
{
    return newTransaction ? false : undo();
}

void UndoManager::getActionsInCurrentTransaction (Array <const UndoableAction*>& actionsFound) const
{
    if (! newTransaction)
        if (const ActionSet* const s = getCurrentSet())
            for (int i = 0; i < s->actions.size(); ++i)
                actionsFound.add (s->actions.getUnchecked(i));
}

int UndoManager::getNumActionsInCurrentTransaction() const
{
    if (! newTransaction)
        if (const ActionSet* const s = getCurrentSet())
            return s->actions.size();

    return 0;
}
