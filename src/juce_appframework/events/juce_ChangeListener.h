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

#ifndef __JUCE_CHANGELISTENER_JUCEHEADER__
#define __JUCE_CHANGELISTENER_JUCEHEADER__


//==============================================================================
/**
    Receives callbacks about changes to some kind of object.

    Many objects use a ChangeListenerList to keep a set of listeners which they
    will inform when something changes. A subclass of ChangeListener
    is used to receive these callbacks.

    Note that the major difference between an ActionListener and a ChangeListener
    is that for a ChangeListener, multiple changes will be coalesced into fewer
    callbacks, but ActionListeners perform one callback for every event posted.

    @see ChangeListenerList, ChangeBroadcaster, ActionListener
*/
class JUCE_API  ChangeListener
{
public:
    /** Destructor. */
    virtual ~ChangeListener()  {}

    /** Overridden by your subclass to receive the callback.

        @param objectThatHasChanged the value that was passed to the
                                    ChangeListenerList::sendChangeMessage() method
    */
    virtual void changeListenerCallback (void* objectThatHasChanged) = 0;
};


#endif   // __JUCE_CHANGELISTENER_JUCEHEADER__
