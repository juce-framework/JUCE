/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../juce_core/basics/juce_StandardHeader.h"

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
