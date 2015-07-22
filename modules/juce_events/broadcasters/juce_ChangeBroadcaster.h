/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
    ChangeBroadcasterCallback callback;
    ListenerList <ChangeListener> changeListeners;

    void callListeners();

    JUCE_DECLARE_NON_COPYABLE (ChangeBroadcaster)
};


#endif   // JUCE_CHANGEBROADCASTER_H_INCLUDED
