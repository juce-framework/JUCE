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

#ifndef JUCE_CHANGELISTENER_H_INCLUDED
#define JUCE_CHANGELISTENER_H_INCLUDED

class ChangeBroadcaster;

//==============================================================================
/**
    Receives change event callbacks that are sent out by a ChangeBroadcaster.

    A ChangeBroadcaster keeps a set of listeners to which it broadcasts a message when
    the ChangeBroadcaster::sendChangeMessage() method is called. A subclass of
    ChangeListener is used to receive these callbacks.

    Note that the major difference between an ActionListener and a ChangeListener
    is that for a ChangeListener, multiple changes will be coalesced into fewer
    callbacks, but ActionListeners perform one callback for every event posted.

    @see ChangeBroadcaster, ActionListener
*/
class JUCE_API  ChangeListener
{
public:
    /** Destructor. */
    virtual ~ChangeListener()  {}

    /** Your subclass should implement this method to receive the callback.
        @param source the ChangeBroadcaster that triggered the callback.
    */
    virtual void changeListenerCallback (ChangeBroadcaster* source) = 0;


    //==============================================================================
   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This method's signature has changed to take a ChangeBroadcaster parameter - please update your code!
    private: virtual int changeListenerCallback (void*) { return 0; }
   #endif
};


#endif   // JUCE_CHANGELISTENER_H_INCLUDED
