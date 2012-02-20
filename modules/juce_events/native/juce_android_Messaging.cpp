/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

void MessageManager::doPlatformSpecificInitialisation() {}
void MessageManager::doPlatformSpecificShutdown() {}

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool returnIfNoPendingMessages)
{
    Logger::outputDebugString ("*** Modal loops are not possible in Android!! Exiting...");
    exit (1);

    return true;
}

//==============================================================================
bool MessageManager::postMessageToSystemQueue (Message* message)
{
    message->incReferenceCount();
    getEnv()->CallVoidMethod (android.activity, JuceAppActivity.postMessage, (jlong) (pointer_sized_uint) message);
    return true;
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, deliverMessage, void, (jobject activity, jlong value))
{
    Message* const message = (Message*) (pointer_sized_uint) value;
    MessageManager::getInstance()->deliverMessage (message);
    message->decReferenceCount();
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
        : result (nullptr), func (func_), parameter (parameter_)
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

void MessageManager::runDispatchLoop()
{
}

class QuitCallback  : public CallbackMessage
{
public:
    QuitCallback() {}

    void messageCallback()
    {
        android.activity.callVoidMethod (JuceAppActivity.finish);
    }
};

void MessageManager::stopDispatchLoop()
{
    (new QuitCallback())->post();
    quitMessagePosted = true;
}
