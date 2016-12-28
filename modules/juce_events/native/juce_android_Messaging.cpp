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
bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    message->incReferenceCount();
    android.activity.callVoidMethod (JuceAppActivity.postMessage, (jlong) (pointer_sized_uint) message);
    return true;
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, deliverMessage, void, (JNIEnv* env, jobject activity, jlong value))
{
    setEnv (env);

    JUCE_TRY
    {
        MessageManager::MessageBase* const message = (MessageManager::MessageBase*) (pointer_sized_uint) value;
        message->messageCallback();
        message->decReferenceCount();
    }
    JUCE_CATCH_EXCEPTION
}

//==============================================================================
void MessageManager::broadcastMessage (const String&)
{
}

void MessageManager::runDispatchLoop()
{
}

void MessageManager::stopDispatchLoop()
{
    struct QuitCallback  : public CallbackMessage
    {
        QuitCallback() {}

        void messageCallback() override
        {
            android.activity.callVoidMethod (JuceAppActivity.finish);
        }
    };

    (new QuitCallback())->post();
    quitMessagePosted = true;
}
