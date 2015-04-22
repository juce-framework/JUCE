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
