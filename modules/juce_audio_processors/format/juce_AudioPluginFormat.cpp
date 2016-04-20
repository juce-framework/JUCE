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
                              eventSignaler, EventSignaler::staticCompletionCallback);


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

#if JUCE_COMPILER_SUPPORTS_LAMBDAS
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
#endif

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
