/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace AudioPluginFormatHelpers
{
    struct CallbackInvoker
    {
        struct InvokeOnMessageThread : public CallbackMessage
        {
            InvokeOnMessageThread (AudioPluginInstance* inInstance, const String& inError,
                                   AudioPluginFormat::InstantiationCompletionCallback* inCompletion,
                                   CallbackInvoker* invoker)
                : instance (inInstance), error (inError), compCallback (inCompletion), owner (invoker)
            {}

            void messageCallback() override     { compCallback->completionCallback (instance, error); }

            //==============================================================================
            AudioPluginInstance* instance;
            String error;
            ScopedPointer<AudioPluginFormat::InstantiationCompletionCallback> compCallback;
            ScopedPointer<CallbackInvoker> owner;
        };

        //==============================================================================
        CallbackInvoker (AudioPluginFormat::InstantiationCompletionCallback* cc)  : completion (cc)
        {}

        void completionCallback (AudioPluginInstance* instance, const String& error)
        {
            (new InvokeOnMessageThread (instance, error, completion, this))->post();
        }

        static void staticCompletionCallback (void* userData, AudioPluginInstance* instance, const String& error)
        {
            reinterpret_cast<CallbackInvoker*> (userData)->completionCallback (instance, error);
        }

        //==============================================================================
        AudioPluginFormat::InstantiationCompletionCallback* completion;
    };
}

AudioPluginFormat::AudioPluginFormat() noexcept {}
AudioPluginFormat::~AudioPluginFormat() {}

AudioPluginInstance* AudioPluginFormat::createInstanceFromDescription (const PluginDescription& desc,
                                                                       double initialSampleRate,
                                                                       int initialBufferSize)
{
    String errorMessage;
    return createInstanceFromDescription (desc, initialSampleRate, initialBufferSize, errorMessage);
}

//==============================================================================
struct EventSignaler : public AudioPluginFormat::InstantiationCompletionCallback
{
    EventSignaler (WaitableEvent& inEvent, AudioPluginInstance*& inInstance, String& inErrorMessage)
        : event (inEvent), outInstance (inInstance), outErrorMessage (inErrorMessage)
    {}

    void completionCallback (AudioPluginInstance* newInstance, const String& result) override
    {
        outInstance = newInstance;
        outErrorMessage = result;
        event.signal();
    }

    static void staticCompletionCallback (void* userData, AudioPluginInstance* pluginInstance, const String& error)
    {
        reinterpret_cast<EventSignaler*> (userData)->completionCallback (pluginInstance, error);
    }

    WaitableEvent& event;
    AudioPluginInstance*& outInstance;
    String& outErrorMessage;

    JUCE_DECLARE_NON_COPYABLE (EventSignaler)
};

AudioPluginInstance* AudioPluginFormat::createInstanceFromDescription (const PluginDescription& desc,
                                                                       double initialSampleRate,
                                                                       int initialBufferSize,
                                                                       String& errorMessage)
{
    if (MessageManager::getInstance()->isThisTheMessageThread()
          && requiresUnblockedMessageThreadDuringCreation(desc))
    {
        errorMessage = NEEDS_TRANS ("This plug-in cannot be instantiated synchronously");
        return nullptr;
    }

    WaitableEvent waitForCreation;
    AudioPluginInstance* instance = nullptr;

    ScopedPointer<EventSignaler> eventSignaler (new EventSignaler (waitForCreation, instance, errorMessage));

    if (! MessageManager::getInstance()->isThisTheMessageThread())
        createPluginInstanceAsync (desc, initialSampleRate, initialBufferSize, eventSignaler.release());
    else
        createPluginInstance (desc, initialSampleRate, initialBufferSize,
                              eventSignaler.get(), EventSignaler::staticCompletionCallback);


    waitForCreation.wait();

    return instance;
}

void AudioPluginFormat::createPluginInstanceAsync (const PluginDescription& description,
                                                   double initialSampleRate,
                                                   int initialBufferSize,
                                                   AudioPluginFormat::InstantiationCompletionCallback* callback)
{
    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        createPluginInstanceOnMessageThread (description, initialSampleRate, initialBufferSize, callback);
        return;
    }

    //==============================================================================
    struct InvokeOnMessageThread  : public CallbackMessage
    {
        InvokeOnMessageThread (AudioPluginFormat* myself,
                               const PluginDescription& descriptionParam,
                               double initialSampleRateParam,
                               int initialBufferSizeParam,
                               AudioPluginFormat::InstantiationCompletionCallback* callbackParam)
            : owner (myself), descr (descriptionParam), sampleRate (initialSampleRateParam),
              bufferSize (initialBufferSizeParam), call (callbackParam)
        {}

        void messageCallback() override
        {
            owner->createPluginInstanceOnMessageThread (descr, sampleRate, bufferSize, call);
        }

        AudioPluginFormat* owner;
        PluginDescription descr;
        double sampleRate;
        int bufferSize;
        AudioPluginFormat::InstantiationCompletionCallback* call;
    };

    (new InvokeOnMessageThread (this, description, initialSampleRate, initialBufferSize, callback))->post();
}

void AudioPluginFormat::createPluginInstanceAsync (const PluginDescription& description,
                                                   double initialSampleRate,
                                                   int initialBufferSize,
                                                   std::function<void (AudioPluginInstance*, const String&)> f)
{
    struct CallbackInvoker  : public AudioPluginFormat::InstantiationCompletionCallback
    {
        CallbackInvoker (std::function<void (AudioPluginInstance*, const String&)> inCompletion)
            : completion (inCompletion)
        {}

        void completionCallback (AudioPluginInstance* instance, const String& error) override
        {
            completion (instance, error);
        }

        std::function<void (AudioPluginInstance*, const String&)> completion;
    };

    createPluginInstanceAsync (description, initialSampleRate, initialBufferSize, new CallbackInvoker (f));
}

void AudioPluginFormat::createPluginInstanceOnMessageThread (const PluginDescription& description,
                                                             double initialSampleRate,
                                                             int initialBufferSize,
                                                             AudioPluginFormat::InstantiationCompletionCallback* callback)
{
    jassert (callback != nullptr);
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    //==============================================================================


    //==============================================================================
    AudioPluginFormatHelpers::CallbackInvoker* completion = new AudioPluginFormatHelpers::CallbackInvoker (callback);

    createPluginInstance (description, initialSampleRate, initialBufferSize, completion,
                          AudioPluginFormatHelpers::CallbackInvoker::staticCompletionCallback);
}

} // namespace juce
