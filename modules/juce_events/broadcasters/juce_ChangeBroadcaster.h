/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_CHANGEBROADCASTER_H_INCLUDED
#define JUCE_CHANGEBROADCASTER_H_INCLUDED


//==============================================================================
/**
    Holds a list of ChangeListeners, and sends messages to them when instructed.

    @see ChangeListener
*/
class JUCE_API  ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an ChangeBroadcaster. */
    ChangeBroadcaster() noexcept;

    /** Destructor. */
    virtual ~ChangeBroadcaster();

    //==============================================================================
    /** Registers a listener to receive change callbacks from this broadcaster.
        Trying to add a listener that's already on the list will have no effect.
    */
    void addChangeListener (ChangeListener* listener);

    /** Unregisters a listener from the list.
        If the listener isn't on the list, this won't have any effect.
    */
    void removeChangeListener (ChangeListener* listener);

    /** Removes all listeners from the list. */
    void removeAllChangeListeners();

    //==============================================================================
    /** Causes an asynchronous change message to be sent to all the registered listeners.

        The message will be delivered asynchronously by the main message thread, so this
        method will return immediately. To call the listeners synchronously use
        sendSynchronousChangeMessage().
    */
    void sendChangeMessage();

    /** Sends a synchronous change message to all the registered listeners.

        This will immediately call all the listeners that are registered. For thread-safety
        reasons, you must only call this method on the main message thread.

        @see dispatchPendingMessages
    */
    void sendSynchronousChangeMessage();

    /** If a change message has been sent but not yet dispatched, this will call
        sendSynchronousChangeMessage() to make the callback immediately.

        For thread-safety reasons, you must only call this method on the main message thread.
    */
    void dispatchPendingMessages();

private:
    //==============================================================================
    class ChangeBroadcasterCallback  : public AsyncUpdater
    {
    public:
        ChangeBroadcasterCallback();
        void handleAsyncUpdate() override;

        ChangeBroadcaster* owner;
    };

    friend class ChangeBroadcasterCallback;
    ChangeBroadcasterCallback broadcastCallback;
    ListenerList <ChangeListener> changeListeners;

    void callListeners();

    JUCE_DECLARE_NON_COPYABLE (ChangeBroadcaster)
};


#endif   // JUCE_CHANGEBROADCASTER_H_INCLUDED
