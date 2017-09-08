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

namespace juce
{

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor,           "<init>",           "()V") \
  METHOD (post,                  "post",             "(Ljava/lang/Runnable;)Z") \

DECLARE_JNI_CLASS (JNIHandler, "android/os/Handler");
#undef JNI_CLASS_MEMBERS


//==============================================================================
namespace Android
{
    class Runnable : public juce::AndroidInterfaceImplementer
    {
    public:
        virtual void run() = 0;

    private:
        jobject invoke (jobject proxy, jobject method, jobjectArray args) override
        {
            auto* env = getEnv();
            auto methodName = juce::juceString ((jstring) env->CallObjectMethod (method, Method.getName));

            if (methodName == "run")
            {
                run();
                return nullptr;
            }

            // invoke base class
            return AndroidInterfaceImplementer::invoke (proxy, method, args);
        }
    };

    struct Handler
    {
        juce_DeclareSingleton (Handler, false)

        Handler() : nativeHandler (getEnv()->NewObject (JNIHandler, JNIHandler.constructor)) {}

        bool post (Runnable* runnable)
        {
            return (getEnv()->CallBooleanMethod (nativeHandler.get(), JNIHandler.post,
                                                 CreateJavaInterface (runnable, "java/lang/Runnable").get()) != 0);
        }

        GlobalRef nativeHandler;
    };

    juce_ImplementSingleton (Handler);
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation() { Android::Handler::getInstance(); }
void MessageManager::doPlatformSpecificShutdown()       {}

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool)
{
    Logger::outputDebugString ("*** Modal loops are not possible in Android!! Exiting...");
    exit (1);

    return true;
}

//==============================================================================
struct AndroidMessageCallback : public Android::Runnable
{
    AndroidMessageCallback (const MessageManager::MessageBase::Ptr& messageToDeliver)
        : message (messageToDeliver)
    {}

    AndroidMessageCallback (MessageManager::MessageBase::Ptr && messageToDeliver)
        : message (static_cast<MessageManager::MessageBase::Ptr&&> (messageToDeliver))
    {}

    void run() override
    {
        JUCE_TRY
        {
            message->messageCallback();

            // delete the message already here as Java will only run the
            // destructor of this runnable the next time the garbage
            // collector kicks in.
            message = nullptr;
        }
        JUCE_CATCH_EXCEPTION
    }

    MessageManager::MessageBase::Ptr message;
};

bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    return Android::Handler::getInstance()->post (new AndroidMessageCallback (message));
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

} // namespace juce
