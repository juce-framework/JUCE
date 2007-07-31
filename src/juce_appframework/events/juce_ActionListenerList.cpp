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

#include "juce_ActionListenerList.h"
#include "../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
// special message of our own with a string in it
class ActionMessage  : public Message
{
public:
    const String message;

    ActionMessage (const String& messageText,
                   void* const listener_) throw()
        : message (messageText)
    {
        pointerParameter = listener_;
    }

    ~ActionMessage() throw()
    {
    }


private:
    ActionMessage (const ActionMessage&);
    const ActionMessage& operator= (const ActionMessage&);
};

//==============================================================================
ActionListenerList::ActionListenerList() throw()
{
}

ActionListenerList::~ActionListenerList() throw()
{
}

void ActionListenerList::addActionListener (ActionListener* const listener) throw()
{
    const ScopedLock sl (actionListenerLock_);

    jassert (listener != 0);
    jassert (! actionListeners_.contains (listener)); // trying to add a listener to the list twice!

    if (listener != 0)
        actionListeners_.add (listener);
}

void ActionListenerList::removeActionListener  (ActionListener* const listener) throw()
{
    const ScopedLock sl (actionListenerLock_);

    jassert (actionListeners_.contains (listener)); // trying to remove a listener that isn't on the list!

    actionListeners_.removeValue (listener);
}

void ActionListenerList::removeAllActionListeners() throw()
{
    const ScopedLock sl (actionListenerLock_);
    actionListeners_.clear();
}

void ActionListenerList::sendActionMessage (const String& message) const
{
    const ScopedLock sl (actionListenerLock_);

    for (int i = actionListeners_.size(); --i >= 0;)
    {
        postMessage (new ActionMessage (message,
                                        (ActionListener*) actionListeners_.getUnchecked(i)));
    }
}

void ActionListenerList::handleMessage (const Message& message)
{
    const ActionMessage& am = (const ActionMessage&) message;

    if (actionListeners_.contains (am.pointerParameter))
        ((ActionListener*) am.pointerParameter)->actionListenerCallback (am.message);
}

END_JUCE_NAMESPACE
