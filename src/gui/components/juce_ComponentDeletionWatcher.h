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
