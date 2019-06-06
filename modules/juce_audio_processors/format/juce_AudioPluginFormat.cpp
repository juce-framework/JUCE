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

AudioPluginFormat::AudioPluginFormat() noexcept {}
AudioPluginFormat::~AudioPluginFormat() {}

std::unique_ptr<AudioPluginInstance> AudioPluginFormat::createInstanceFromDescription (const PluginDescription& desc,
                                                                                       double initialSampleRate,
                                                                                       int initialBufferSize)
{
    String errorMessage;
    return createInstanceFromDescription (desc, initialSampleRate, initialBufferSize, errorMessage);
}

std::unique_ptr<AudioPluginInstance> AudioPluginFormat::createInstanceFromDescription (const PluginDescription& desc,
                                                                                       double initialSampleRate,
                                                                                       int initialBufferSize,
                                                                                       String& errorMessage)
{
    if (MessageManager::getInstance()->isThisTheMessageThread()
          && requiresUnblockedMessageThreadDuringCreation (desc))
    {
        errorMessage = NEEDS_TRANS ("This plug-in cannot be instantiated synchronously");
        return {};
    }

    WaitableEvent finishedSignal;
    std::unique_ptr<AudioPluginInstance> instance;

    auto callback = [&] (std::unique_ptr<AudioPluginInstance> p, const String& error)
    {
       errorMessage = error;
       instance = std::move (p);
       finishedSignal.signal();
    };

    if (! MessageManager::getInstance()->isThisTheMessageThread())
        createPluginInstanceAsync (desc, initialSampleRate, initialBufferSize, std::move (callback));
    else
        createPluginInstance (desc, initialSampleRate, initialBufferSize, std::move (callback));

    finishedSignal.wait();
    return instance;
}

void AudioPluginFormat::createPluginInstanceAsync (const PluginDescription& description,
                                                   double initialSampleRate, int initialBufferSize,
                                                   PluginCreationCallback callback)
{
    jassert (callback != nullptr);

    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        createPluginInstance (description, initialSampleRate, initialBufferSize, std::move (callback));
        return;
    }

    struct InvokeOnMessageThread  : public CallbackMessage
    {
        InvokeOnMessageThread (AudioPluginFormat& f, const PluginDescription& d,
                               double sr, int size, PluginCreationCallback call)
            : format (f), desc (d), sampleRate (sr), bufferSize (size),
              callbackToUse (std::move (call))
        {
            post();
        }

        void messageCallback() override
        {
            format.createPluginInstance (desc, sampleRate, bufferSize, std::move (callbackToUse));
        }

        AudioPluginFormat& format;
        PluginDescription desc;
        double sampleRate;
        int bufferSize;
        PluginCreationCallback callbackToUse;
    };

    new InvokeOnMessageThread (*this, description, initialSampleRate, initialBufferSize, std::move (callback));
}

} // namespace juce
