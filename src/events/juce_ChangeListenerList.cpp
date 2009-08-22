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

#include "juce_ChangeListenerList.h"


//==============================================================================
ChangeListenerList::ChangeListenerList() throw()
   : lastChangedObject (0),
     messagePending (false)
{
}

ChangeListenerList::~ChangeListenerList() throw()
{
}

void ChangeListenerList::addChangeListener (ChangeListener* const listener) throw()
{
    const ScopedLock sl (lock);

    jassert (listener != 0);

    if (listener != 0)
        listeners.add (listener);
}

void ChangeListenerList::removeChangeListener (ChangeListener* const listener) throw()
{
    const ScopedLock sl (lock);
    listeners.removeValue (listener);
}

void ChangeListenerList::removeAllChangeListeners() throw()
{
    const ScopedLock sl (lock);
    listeners.clear();
}

void ChangeListenerList::sendChangeMessage (void* const objectThatHasChanged) throw()
{
    const ScopedLock sl (lock);

    if ((! messagePending) && (listeners.size() > 0))
    {
        lastChangedObject = objectThatHasChanged;
        postMessage (new Message (0, 0, 0, objectThatHasChanged));
        messagePending = true;
    }
}

void ChangeListenerList::handleMessage (const Message& message)
{
    sendSynchronousChangeMessage (message.pointerParameter);
}

void ChangeListenerList::sendSynchronousChangeMessage (void* const objectThatHasChanged)
{
    const ScopedLock sl (lock);
    messagePending = false;

    for (int i = listeners.size(); --i >= 0;)
    {
        ChangeListener* const l = (ChangeListener*) listeners.getUnchecked (i);

        {
            const ScopedUnlock tempUnlocker (lock);
            l->changeListenerCallback (objectThatHasChanged);
        }

        i = jmin (i, listeners.size());
    }
}

void ChangeListenerList::dispatchPendingMessages()
{
    if (messagePending)
        sendSynchronousChangeMessage (lastChangedObject);
}

END_JUCE_NAMESPACE
