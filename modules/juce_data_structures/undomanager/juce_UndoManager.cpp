/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct UndoManager::ActionSet
{
    ActionSet (const String& transactionName)  : name (transactionName)
    {}

    bool perform() const
    {
        for (auto* a : actions)
            if (! a->perform())
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

        for (auto* a : actions)
            total += a->getSizeInUnits();

        return total;
    }

    OwnedArray<UndoableAction> actions;
    String name;
    Time time { Time::getCurrentTime() };
};

//==============================================================================
UndoManager::UndoManager (int maxNumberOfUnitsToKeep, int minimumTransactions)
{
    setMaxNumberOfStoredUnits (maxNumberOfUnitsToKeep, minimumTransactions);
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

void UndoManager::setMaxNumberOfStoredUnits (int maxUnits, int minTransactions)
{
    maxNumUnitsToKeep          = jmax (1, maxUnits);
    minimumTransactionsToKeep  = jmax (1, minTransactions);
}

//==============================================================================
bool UndoManager::perform (UndoableAction* newAction, const String& actionName)
{
    if (perform (newAction))
    {
        if (actionName.isNotEmpty())
            setCurrentTransactionName (actionName);

        return true;
    }

    return false;
}

bool UndoManager::perform (UndoableAction* newAction)
{
    if (newAction != nullptr)
    {
        std::unique_ptr<UndoableAction> action (newAction);

        if (isPerformingUndoRedo())
        {
            jassertfalse;  // Don't call perform() recursively from the UndoableAction::perform()
                           // or undo() methods, or else these actions will be discarded!
            return false;
        }

        if (action->perform())
        {
            auto* actionSet = getCurrentSet();

            if (actionSet != nullptr && ! newTransaction)
            {
                if (auto* lastAction = actionSet->actions.getLast())
                {
                    if (auto coalescedAction = lastAction->createCoalescedAction (action.get()))
                    {
                        action.reset (coalescedAction);
                        totalUnitsStored -= lastAction->getSizeInUnits();
                        actionSet->actions.removeLast();
                    }
                }
            }
            else
            {
                actionSet = new ActionSet (newTransactionName);
                transactions.insert (nextIndex, actionSet);
                ++nextIndex;
            }

            totalUnitsStored += action->getSizeInUnits();
            actionSet->actions.add (action.release());
            newTransaction = false;

            moveFutureTransactionsToStash();
            dropOldTransactionsIfTooLarge();
            sendChangeMessage();
            return true;
        }
    }

    return false;
}

void UndoManager::moveFutureTransactionsToStash()
{
    if (nextIndex < transactions.size())
    {
        stashedFutureTransactions.clear();

        while (nextIndex < transactions.size())
        {
            auto* removed = transactions.removeAndReturn (nextIndex);
            stashedFutureTransactions.add (removed);
            totalUnitsStored -= removed->getTotalSize();
        }
    }
}

void UndoManager::restoreStashedFutureTransactions()
{
    while (nextIndex < transactions.size())
    {
        totalUnitsStored -= transactions.getUnchecked (nextIndex)->getTotalSize();
        transactions.remove (nextIndex);
    }

    for (auto* stashed : stashedFutureTransactions)
    {
        transactions.add (stashed);
        totalUnitsStored += stashed->getTotalSize();
    }

    stashedFutureTransactions.clearQuick (false);
}

void UndoManager::dropOldTransactionsIfTooLarge()
{
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

void UndoManager::beginNewTransaction()
{
    beginNewTransaction ({});
}

void UndoManager::beginNewTransaction (const String& actionName)
{
    newTransaction = true;
    newTransactionName = actionName;
}

void UndoManager::setCurrentTransactionName (const String& newName)
{
    if (newTransaction)
        newTransactionName = newName;
    else if (auto* action = getCurrentSet())
        action->name = newName;
}

String UndoManager::getCurrentTransactionName() const
{
    if (auto* action = getCurrentSet())
        return action->name;

    return newTransactionName;
}

//==============================================================================
UndoManager::ActionSet* UndoManager::getCurrentSet() const     { return transactions[nextIndex - 1]; }
UndoManager::ActionSet* UndoManager::getNextSet() const        { return transactions[nextIndex]; }

bool UndoManager::isPerformingUndoRedo() const  { return isInsideUndoRedoCall; }

bool UndoManager::canUndo() const      { return getCurrentSet() != nullptr; }
bool UndoManager::canRedo() const      { return getNextSet()    != nullptr; }

bool UndoManager::undo()
{
    if (auto* s = getCurrentSet())
    {
        const ScopedValueSetter<bool> setter (isInsideUndoRedoCall, true);

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
    if (auto* s = getNextSet())
    {
        const ScopedValueSetter<bool> setter (isInsideUndoRedoCall, true);

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
    if (auto* s = getCurrentSet())
        return s->name;

    return {};
}

String UndoManager::getRedoDescription() const
{
    if (auto* s = getNextSet())
        return s->name;

    return {};
}

StringArray UndoManager::getUndoDescriptions() const
{
    StringArray descriptions;

    for (int i = nextIndex;;)
    {
        if (auto* t = transactions[--i])
            descriptions.add (t->name);
        else
            return descriptions;
    }
}

StringArray UndoManager::getRedoDescriptions() const
{
    StringArray descriptions;

    for (int i = nextIndex;;)
    {
        if (auto* t = transactions[i++])
            descriptions.add (t->name);
        else
            return descriptions;
    }
}

Time UndoManager::getTimeOfUndoTransaction() const
{
    if (auto* s = getCurrentSet())
        return s->time;

    return {};
}

Time UndoManager::getTimeOfRedoTransaction() const
{
    if (auto* s = getNextSet())
        return s->time;

    return Time::getCurrentTime();
}

bool UndoManager::undoCurrentTransactionOnly()
{
    if ((! newTransaction) && undo())
    {
        restoreStashedFutureTransactions();
        return true;
    }

    return false;
}

void UndoManager::getActionsInCurrentTransaction (Array<const UndoableAction*>& actionsFound) const
{
    if (! newTransaction)
        if (auto* s = getCurrentSet())
            for (auto* a : s->actions)
                actionsFound.add (a);
}

int UndoManager::getNumActionsInCurrentTransaction() const
{
    if (! newTransaction)
        if (auto* s = getCurrentSet())
            return s->actions.size();

    return 0;
}

} // namespace juce
