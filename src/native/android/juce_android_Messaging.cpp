/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{

}

void MessageManager::doPlatformSpecificShutdown()
{

}

//==============================================================================
bool juce_dispatchNextMessageOnSystemQueue (const bool returnIfNoPendingMessages)
{
    // TODO

    /*
        The idea here is that this will check the system message queue, pull off a
        message if there is one, deliver it, and return true if a message was delivered.
        If the queue's empty, return false.

        If the message is one of our special ones (i.e. a Message object being delivered,
        this must call MessageManager::getInstance()->deliverMessage() to deliver it


    */

    return true;
}

//==============================================================================
bool juce_postMessageToSystemQueue (Message* message)
{
    getEnv()->CallVoidMethod (android.activity, android.postMessage, (jlong) (pointer_sized_uint) message);
    return true;
}

JUCE_JNI_CALLBACK (JuceAppActivity, deliverMessage, void, (jobject activity, jlong value))
{
    Message* m = (Message*) (pointer_sized_uint) value;
    MessageManager::getInstance()->deliverMessage ((Message*) (pointer_sized_uint) value);
}

//==============================================================================
class AsyncFunctionCaller   : public AsyncUpdater
{
public:
    static void* call (MessageCallbackFunction* func_, void* parameter_)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
            return func_ (parameter_);

        AsyncFunctionCaller caller (func_, parameter_);
        caller.triggerAsyncUpdate();
        caller.finished.wait();
        return caller.result;
    }

    void handleAsyncUpdate()
    {
        result = (*func) (parameter);
        finished.signal();
    }

private:
    WaitableEvent finished;
    MessageCallbackFunction* func;
    void* parameter;
    void* volatile result;

    AsyncFunctionCaller (MessageCallbackFunction* func_, void* parameter_)
        : result (0), func (func_), parameter (parameter_)
    {}

    JUCE_DECLARE_NON_COPYABLE (AsyncFunctionCaller);
};

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* func, void* parameter)
{
    return AsyncFunctionCaller::call (func, parameter);
}

//==============================================================================
void MessageManager::broadcastMessage (const String&)
{
}

#endif
