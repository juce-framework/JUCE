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

static SpinLock deletedAtShutdownLock;

DeletedAtShutdown::DeletedAtShutdown()
{
    const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
    getObjects().add (this);
}

DeletedAtShutdown::~DeletedAtShutdown()
{
    const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
    getObjects().removeFirstMatchingValue (this);
}

void DeletedAtShutdown::deleteAll()
{
    // make a local copy of the array, so it can't get into a loop if something
    // creates another DeletedAtShutdown object during its destructor.
    Array <DeletedAtShutdown*> localCopy;

    {
        const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
        localCopy = getObjects();
    }

    for (int i = localCopy.size(); --i >= 0;)
    {
        JUCE_TRY
        {
            DeletedAtShutdown* deletee = localCopy.getUnchecked(i);

            // double-check that it's not already been deleted during another object's destructor.
            {
                const SpinLock::ScopedLockType sl (deletedAtShutdownLock);
                if (! getObjects().contains (deletee))
                    deletee = nullptr;
            }

            delete deletee;
        }
        JUCE_CATCH_EXCEPTION
    }

    // if no objects got re-created during shutdown, this should have been emptied by their
    // destructors
    jassert (getObjects().size() == 0);

    getObjects().clear(); // just to make sure the array doesn't have any memory still allocated
}

Array <DeletedAtShutdown*>& DeletedAtShutdown::getObjects()
{
    static Array <DeletedAtShutdown*> objects;
    return objects;
}
