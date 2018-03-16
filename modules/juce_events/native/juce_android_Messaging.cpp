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
    class Runnable  : public juce::AndroidInterfaceImplementer
    {
    public:
        virtual void run() = 0;

    private:
        jobject invoke (jobject proxy, jobject method, jobjectArray args) override
        {
            auto* env = getEnv();
            auto methodName = juce::juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

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
        Handler() : nativeHandler (getEnv()->NewObject (JNIHandler, JNIHandler.constructor)) {}
        ~Handler() { clearSingletonInstance(); }

        JUCE_DECLARE_SINGLETON (Handler, false)

        bool post (jobject runnable)
        {
            return (getEnv()->CallBooleanMethod (nativeHandler.get(), JNIHandler.post, runnable) != 0);
        }

        GlobalRef nativeHandler;
    };

    JUCE_IMPLEMENT_SINGLETON (Handler)
}

//==============================================================================
struct AndroidMessageQueue     : private Android::Runnable
{
    JUCE_DECLARE_SINGLETON_SINGLETHREADED (AndroidMessageQueue, true)

    AndroidMessageQueue()
        : self (CreateJavaInterface (this, "java/lang/Runnable").get())
    {
    }

    ~AndroidMessageQueue()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
        clearSingletonInstance();
    }

    bool post (MessageManager::MessageBase::Ptr&& message)
    {
        queue.add (static_cast<MessageManager::MessageBase::Ptr&& > (message));

        // this will call us on the message thread
        return handler.post (self.get());
    }

private:

    void run() override
    {
        while (true)
        {
            MessageManager::MessageBase::Ptr message (queue.removeAndReturn (0));

            if (message == nullptr)
                break;

            message->messageCallback();
        }
    }

    // the this pointer to this class in Java land
    GlobalRef self;

    ReferenceCountedArray<MessageManager::MessageBase, CriticalSection> queue;
    Android::Handler handler;
};

JUCE_IMPLEMENT_SINGLETON (AndroidMessageQueue)

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation() { AndroidMessageQueue::getInstance(); }
void MessageManager::doPlatformSpecificShutdown()       { AndroidMessageQueue::deleteInstance(); }

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool)
{
    Logger::outputDebugString ("*** Modal loops are not possible in Android!! Exiting...");
    exit (1);

    return true;
}

bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    return AndroidMessageQueue::getInstance()->post (message);
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
            auto* env = getEnv();

            jmethodID quitMethod = env->GetMethodID (JuceAppActivity, "finishAndRemoveTask", "()V");

            if (quitMethod != 0)
            {
                env->CallVoidMethod (android.activity, quitMethod);
                return;
            }

            quitMethod = env->GetMethodID (JuceAppActivity, "finish", "()V");
            jassert (quitMethod != 0);
            env->CallVoidMethod (android.activity, quitMethod);
        }
    };

    (new QuitCallback())->post();
    quitMessagePosted = true;
}

} // namespace juce
