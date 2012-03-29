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
bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    message->incReferenceCount();
    android.activity.callVoidMethod (JuceAppActivity.postMessage, (jlong) (pointer_sized_uint) message);
    return true;
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, deliverMessage, void, (jobject activity, jlong value))
{
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

        void messageCallback()
        {
            android.activity.callVoidMethod (JuceAppActivity.finish);
        }
    };

    (new QuitCallback())->post();
    quitMessagePosted = true;
}
