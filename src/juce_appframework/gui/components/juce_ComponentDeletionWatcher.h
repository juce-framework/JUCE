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

#ifndef __JUCE_COMPONENTDELETIONWATCHER_JUCEHEADER__
#define __JUCE_COMPONENTDELETIONWATCHER_JUCEHEADER__

#include "juce_Component.h"


//==============================================================================
/**
    Object for monitoring a component, and later testing whether it's still valid.

    Slightly obscure, this one, but it's used internally for making sure that
    after some callbacks, a component hasn't been deleted. It's more reliable than
    just using isValidComponent(), which can provide false-positives if a new
    component is created at the same memory location as an old one.
*/
class JUCE_API  ComponentDeletionWatcher
{
public:
    //==============================================================================
    /** Creates a watcher for a given component.

        The component must be valid at the time it's passed in.
    */
    ComponentDeletionWatcher (const Component* const componentToWatch) throw();

    /** Destructor. */
    ~ComponentDeletionWatcher() throw();

    /** Returns true if the component has been deleted since the time that this
        object was created.
    */
    bool hasBeenDeleted() const throw();

    /** Returns the component that's being watched, or null if it has been deleted. */
    const Component* getComponent() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    const Component* const componentToWatch;
    const uint32 componentUID;

    ComponentDeletionWatcher (const ComponentDeletionWatcher&);
    const ComponentDeletionWatcher& operator= (const ComponentDeletionWatcher&);
};


#endif   // __JUCE_COMPONENTDELETIONWATCHER_JUCEHEADER__
