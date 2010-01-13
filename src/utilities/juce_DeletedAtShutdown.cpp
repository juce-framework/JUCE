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

#include "juce_DeletedAtShutdown.h"
#include "../containers/juce_VoidArray.h"
#include "../threads/juce_ScopedLock.h"
#include "../application/juce_Application.h"


//==============================================================================
static VoidArray objectsToDelete;
static CriticalSection lock;

//==============================================================================
DeletedAtShutdown::DeletedAtShutdown()
{
    const ScopedLock sl (lock);
    objectsToDelete.add (this);
}

DeletedAtShutdown::~DeletedAtShutdown()
{
    const ScopedLock sl (lock);
    objectsToDelete.removeValue (this);
}

void DeletedAtShutdown::deleteAll()
{
    // make a local copy of the array, so it can't get into a loop if something
    // creates another DeletedAtShutdown object during its destructor.
    lock.enter();
    const VoidArray localCopy (objectsToDelete);
    lock.exit();

    for (int i = localCopy.size(); --i >= 0;)
    {
        JUCE_TRY
        {
            DeletedAtShutdown* deletee = (DeletedAtShutdown*) localCopy.getUnchecked(i);

            // double-check that it's not already been deleted during another object's destructor.
            {
                const ScopedLock sl (lock);
                if (! objectsToDelete.contains (deletee))
                    deletee = 0;
            }

            delete deletee;
        }
        JUCE_CATCH_EXCEPTION
    }

    // if no objects got re-created during shutdown, this should have been emptied by their
    // destructors
    jassert (objectsToDelete.size() == 0);

    objectsToDelete.clear(); // just to make sure the array doesn't have any memory still allocated
}

END_JUCE_NAMESPACE
