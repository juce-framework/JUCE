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

#include "juce_ChangeBroadcaster.h"
#include "juce_MessageManager.h"


//==============================================================================
ChangeBroadcaster::ChangeBroadcaster() throw()
{
    // are you trying to create this object before or after juce has been intialised??
    jassert (MessageManager::instance != 0);
}

ChangeBroadcaster::~ChangeBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    jassert (MessageManager::instance != 0);
}

void ChangeBroadcaster::addChangeListener (ChangeListener* const listener) throw()
{
    changeListenerList.addChangeListener (listener);
}

void ChangeBroadcaster::removeChangeListener (ChangeListener* const listener) throw()
{
    jassert (changeListenerList.isValidMessageListener());

    if (changeListenerList.isValidMessageListener())
        changeListenerList.removeChangeListener (listener);
}

void ChangeBroadcaster::removeAllChangeListeners() throw()
{
    changeListenerList.removeAllChangeListeners();
}

void ChangeBroadcaster::sendChangeMessage (void* objectThatHasChanged) throw()
{
    changeListenerList.sendChangeMessage (objectThatHasChanged);
}

void ChangeBroadcaster::sendSynchronousChangeMessage (void* objectThatHasChanged)
{
    changeListenerList.sendSynchronousChangeMessage (objectThatHasChanged);
}

void ChangeBroadcaster::dispatchPendingMessages()
{
    changeListenerList.dispatchPendingMessages();
}


END_JUCE_NAMESPACE
