/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

void MessageManager::doPlatformSpecificInitialisation() {}
void MessageManager::doPlatformSpecificShutdown() {}

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool)
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

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, deliverMessage, void, (JNIEnv* env, jobject, jlong value))
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
