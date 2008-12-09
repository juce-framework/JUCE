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

#ifndef __JUCE_MESSAGEMANAGER_JUCEHEADER__
#define __JUCE_MESSAGEMANAGER_JUCEHEADER__

#include "../application/juce_DeletedAtShutdown.h"
#include "../../juce_core/containers/juce_SortedSet.h"
#include "../../juce_core/containers/juce_VoidArray.h"
#include "../../juce_core/threads/juce_Thread.h"
#include "juce_ActionListenerList.h"
class Component;
class MessageManagerLock;


//==============================================================================
/** See MessageManager::callFunctionOnMessageThread() for use of this function type
*/
typedef void* (MessageCallbackFunction) (void* userData);


//==============================================================================
/** Delivers Message objects to MessageListeners, and handles the event-dispatch loop.

    @see Message, MessageListener, MessageManagerLock, JUCEApplication
*/
class JUCE_API  MessageManager  : private DeletedAtShutdown
{
public:
    //==============================================================================
    /** Returns the global instance of the MessageManager. */
    static MessageManager* getInstance() throw();

    //==============================================================================
    void runDispatchLoop();
    void stopDispatchLoop();
    bool hasStopMessageBeenSent() const throw()         { return quitMessagePosted; }

    /** Synchronously dispatches messages until a given time has elapsed.

        Returns false if a quit message has been posted, otherwise returns true.
    */
    bool runDispatchLoopUntil (int millisecondsToRunFor);

    //==============================================================================
    /*int runModalLoop (Component* componentToBeModal);
    bool makeComponentModal (Component* componentToBeModal);
    bool isComponentModal (const Component* component) const;
    Component* getCurrentModalComponent() const;
    void exitModalLoop (int returnValue);*/

    //==============================================================================
    /** Calls a function using the message-thread.

        This can be used by any thread to cause this function to be called-back
        by the message thread. If it's the message-thread that's calling this method,
        then the function will just be called; if another thread is calling, a message
        will be posted to the queue, and this method will block until that message
        is delivered, the function is called, and the result is returned.

        Be careful not to cause any deadlocks with this! It's easy to do - e.g. if the caller
        thread has a critical section locked, which an unrelated message callback then tries to lock
        before the message thread gets round to processing this callback.

        @param callback     the function to call - its signature must be @code
                            void* myCallbackFunction (void*) @endcode
        @param userData     a user-defined pointer that will be passed to the function that gets called
        @returns            the value that the callback function returns.
        @see MessageManagerLock
    */
    void* callFunctionOnMessageThread (MessageCallbackFunction* callback,
                                       void* userData);

    /** Returns true if the caller-thread is the message thread. */
    bool isThisTheMessageThread() const throw();

    /** Called to tell the manager which thread is the one that's running the dispatch loop.

        (Best to ignore this method unless you really know what you're doing..)
        @see getCurrentMessageThread
    */
    void setCurrentMessageThread (const Thread::ThreadID threadId) throw();

    /** Returns the ID of the current message thread, as set by setCurrentMessageThread().

        (Best to ignore this method unless you really know what you're doing..)
        @see setCurrentMessageThread
    */
    Thread::ThreadID getCurrentMessageThread() const throw()             { return messageThreadId; }

    /** Returns true if the caller thread has currenltly got the message manager locked.

        see the MessageManagerLock class for more info about this.

        This will be true if the caller is the message thread, because that automatically
        gains a lock while a message is being dispatched.
    */
    bool currentThreadHasLockedMessageManager() const throw();

    //==============================================================================
    /** Sends a message to all other JUCE applications that are running.

        @param messageText      the string that will be passed to the actionListenerCallback()
                                method of the broadcast listeners in the other app.
        @see registerBroadcastListener, ActionListener
    */
    static void broadcastMessage (const String& messageText) throw();

    /** Registers a listener to get told about broadcast messages.

        The actionListenerCallback() callback's string parameter
        is the message passed into broadcastMessage().

        @see broadcastMessage
    */
    void registerBroadcastListener (ActionListener* listener) throw();

    /** Deregisters a broadcast listener. */
    void deregisterBroadcastListener (ActionListener* listener) throw();

    //==============================================================================
    /** @internal */
    void deliverMessage (void*);
    /** @internal */
    void deliverBroadcastMessage (const String&);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    MessageManager() throw();
    ~MessageManager() throw();

    friend class MessageListener;
    friend class ChangeBroadcaster;
    friend class ActionBroadcaster;
    static MessageManager* instance;

    SortedSet<const MessageListener*> messageListeners;
    ActionListenerList* broadcastListeners;

    friend class JUCEApplication;
    bool quitMessagePosted, quitMessageReceived;
    Thread::ThreadID messageThreadId;

    VoidArray modalComponents;
    static void* exitModalLoopCallback (void*);

    void postMessageToQueue (Message* const message);

    static void doPlatformSpecificInitialisation();
    static void doPlatformSpecificShutdown();

    friend class MessageManagerLock;
    CriticalSection messageDispatchLock;
    Thread::ThreadID currentLockingThreadId;

    MessageManager (const MessageManager&);
    const MessageManager& operator= (const MessageManager&);
};


//==============================================================================
/** Used to make sure that the calling thread has exclusive access to the message loop.

    Because it's not thread-safe to call any of the Component or other UI classes
    from threads other than the message thread, one of these objects can be used to
    lock the message loop and allow this to be done. The message thread will be
    suspended for the lifetime of the MessageManagerLock object, so create one on
    the stack like this: @code
    void MyThread::run()
    {
        someData = 1234;

        const MessageManagerLock mmLock;
        // the event loop will now be locked so it's safe to make a few calls..

        myComponent->setBounds (newBounds);
        myComponent->repaint();

        // ..the event loop will now be unlocked as the MessageManagerLock goes out of scope
    }
    @endcode

    Obviously be careful not to create one of these and leave it lying around, or
    your app will grind to a halt!

    Another caveat is that using this in conjunction with other CriticalSections
    can create lots of interesting ways of producing a deadlock! In particular, if
    your message thread calls stopThread() for a thread that uses these locks,
    you'll get an (occasional) deadlock..

    @see MessageManager, MessageManager::currentThreadHasLockedMessageManager
*/
class JUCE_API MessageManagerLock
{
public:
    //==============================================================================
    /** Tries to acquire a lock on the message manager.

        If this constructor
        When this constructor returns, the message manager will have finished processing the
        last message and will not send another message until this MessageManagerLock is
        deleted.

        If the current thread already has the lock, nothing will be done, so it's perfectly
        safe to create these locks recursively.
    */
    MessageManagerLock() throw();

    /** Releases the current thread's lock on the message manager.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    ~MessageManagerLock() throw();

    //==============================================================================
    /** Tries to acquire a lock on the message manager.

        This does the same thing as the normal constructor, but while it's waiting to get
        the lock, it checks the specified thread to see if it has been given the
        Thread::signalThreadShouldExit() signal. If this happens, then it will return
        without gaining the lock.

        To find out whether the lock was successful, call lockWasGained(). If this is
        false, your thread is being told to die, so you'd better get out of there.

        If the current thread already has the lock, nothing will be done, so it's perfectly
        safe to create these locks recursively.

        E.g.
        @code
        void run()
        {
            ...

            while (! threadShouldExit())
            {
                MessageManagerLock mml (Thread::getCurrentThread());

                if (! mml.lockWasGained)
                    return; // another thread is trying to kill us!

                ..do some locked stuff here..
            }

            ..and now the MM is now unlocked..
        }
        @endcode

    */
    MessageManagerLock (Thread* const threadToCheckForExitSignal) throw();


    /** Returns true if the lock was successfully acquired.

        (See the constructor that takes a Thread for more info).
    */
    bool lockWasGained() const throw()                      { return locked; }


private:
    Thread::ThreadID lastLockingThreadId;
    bool locked;
};



#endif   // __JUCE_MESSAGEMANAGER_JUCEHEADER__
