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

#ifndef __JUCE_ASYNCUPDATER_JUCEHEADER__
#define __JUCE_ASYNCUPDATER_JUCEHEADER__

#include "juce_MessageListener.h"


//==============================================================================
/**
    Has a callback method that is triggered asynchronously.

    This object allows an asynchronous callback function to be triggered, for
    tasks such as coalescing multiple updates into a single callback later on.

    Basically, one or more calls to the triggerAsyncUpdate() will result in the
    message thread calling handleAsyncUpdate() as soon as it can.
*/
class JUCE_API  AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an AsyncUpdater object. */
    AsyncUpdater() throw();

    /** Destructor.

        If there are any pending callbacks when the object is deleted, these are lost.
    */
    virtual ~AsyncUpdater();

    //==============================================================================
    /** Causes the callback to be triggered at a later time.

        This method returns immediately, having made sure that a callback
        to the handleAsyncUpdate() method will occur as soon as possible.

        If an update callback is already pending but hasn't happened yet, calls
        to this method will be ignored.

        It's thread-safe to call this method from any number of threads without
        needing to worry about locking.
    */
    void triggerAsyncUpdate() throw();

    /** This will stop any pending updates from happening.

        If called after triggerAsyncUpdate() and before the handleAsyncUpdate()
        callback happens, this will cancel the handleAsyncUpdate() callback.
    */
    void cancelPendingUpdate() throw();

    /** If an update has been triggered and is pending, this will invoke it
        synchronously.

        Use this as a kind of "flush" operation - if an update is pending, the
        handleAsyncUpdate() method will be called immediately; if no update is
        pending, then nothing will be done.
    */
    void handleUpdateNowIfNeeded();

    //==============================================================================
    /** Called back to do whatever your class needs to do.

        This method is called by the message thread at the next convenient time
        after the triggerAsyncUpdate() method has been called.
    */
    virtual void handleAsyncUpdate() = 0;


private:
    //==============================================================================
    class AsyncUpdaterInternal  : public MessageListener
    {
    public:
        AsyncUpdaterInternal() throw() {}
        ~AsyncUpdaterInternal() {}

        void handleMessage (const Message&);

        AsyncUpdater* owner;

    private:
        AsyncUpdaterInternal (const AsyncUpdaterInternal&);
        const AsyncUpdaterInternal& operator= (const AsyncUpdaterInternal&);
    };

    AsyncUpdaterInternal internalAsyncHandler;
    bool asyncMessagePending;
};


#endif   // __JUCE_ASYNCUPDATER_JUCEHEADER__
