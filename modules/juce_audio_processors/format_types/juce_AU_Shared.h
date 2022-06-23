/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifndef DOXYGEN

// This macro can be set if you need to override this internal name for some reason..
#ifndef JUCE_STATE_DICTIONARY_KEY
 #define JUCE_STATE_DICTIONARY_KEY   "jucePluginState"
#endif

namespace juce
{

struct AudioUnitHelpers
{
    class ChannelRemapper
    {
    public:
        void alloc (AudioProcessor& processor)
        {
            const int numInputBuses  = AudioUnitHelpers::getBusCount (processor, true);
            const int numOutputBuses = AudioUnitHelpers::getBusCount (processor, false);

            initializeChannelMapArray (processor, true, numInputBuses);
            initializeChannelMapArray (processor, false, numOutputBuses);

            for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
                fillLayoutChannelMaps (processor, true, busIdx);

            for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                fillLayoutChannelMaps (processor, false, busIdx);
        }

        void release()
        {
            inputLayoutMap = outputLayoutMap = nullptr;
            inputLayoutMapPtrStorage.free();
            outputLayoutMapPtrStorage.free();
            inputLayoutMapStorage.free();
            outputLayoutMapStorage.free();
        }

        inline const int* get (bool input, int bus) const noexcept { return (input ? inputLayoutMap : outputLayoutMap)[bus]; }

    private:
        //==============================================================================
        HeapBlock<int*> inputLayoutMapPtrStorage, outputLayoutMapPtrStorage;
        HeapBlock<int>  inputLayoutMapStorage, outputLayoutMapStorage;
        int** inputLayoutMap  = nullptr;
        int** outputLayoutMap = nullptr;

        //==============================================================================
        void initializeChannelMapArray (AudioProcessor& processor, bool isInput, const int numBuses)
        {
            HeapBlock<int*>& layoutMapPtrStorage = isInput ? inputLayoutMapPtrStorage : outputLayoutMapPtrStorage;
            HeapBlock<int>& layoutMapStorage = isInput ? inputLayoutMapStorage : outputLayoutMapStorage;
            int**& layoutMap = isInput ? inputLayoutMap : outputLayoutMap;

            const int totalInChannels  = processor.getTotalNumInputChannels();
            const int totalOutChannels = processor.getTotalNumOutputChannels();

            layoutMapPtrStorage.calloc (static_cast<size_t> (numBuses));
            layoutMapStorage.calloc (static_cast<size_t> (isInput ? totalInChannels : totalOutChannels));

            layoutMap = layoutMapPtrStorage.get();

            int ch = 0;
            for (int busIdx = 0; busIdx < numBuses; ++busIdx)
            {
                layoutMap[busIdx] = layoutMapStorage.get() + ch;
                ch += processor.getChannelCountOfBus (isInput, busIdx);
            }
        }

        void fillLayoutChannelMaps (AudioProcessor& processor, bool isInput, int busNr)
        {
            int* layoutMap = (isInput ? inputLayoutMap : outputLayoutMap)[busNr];
            auto channelFormat = processor.getChannelLayoutOfBus (isInput, busNr);
            AudioChannelLayout coreAudioLayout;

            zerostruct (coreAudioLayout);
            coreAudioLayout.mChannelLayoutTag = CoreAudioLayouts::toCoreAudio (channelFormat);

            const int numChannels = channelFormat.size();
            auto coreAudioChannels = CoreAudioLayouts::getCoreAudioLayoutChannels (coreAudioLayout);

            for (int i = 0; i < numChannels; ++i)
                layoutMap[i] = coreAudioChannels.indexOf (channelFormat.getTypeOfChannel (i));
        }
    };

    //==============================================================================
    class CoreAudioBufferList
    {
    public:
        void prepare (const AudioProcessor::BusesLayout& layout, int maxFrames)
        {
            const auto getChannelOffsets = [] (const auto& range)
            {
                std::vector<int> result { 0 };

                for (const auto& bus : range)
                    result.push_back (result.back() + bus.size());

                return result;
            };

            inputBusOffsets  = getChannelOffsets (layout.inputBuses);
            outputBusOffsets = getChannelOffsets (layout.outputBuses);

            const auto numChannels = jmax (inputBusOffsets.back(), outputBusOffsets.back());
            scratch.setSize (numChannels, maxFrames);
            channels.resize (static_cast<size_t> (numChannels), nullptr);

            reset();
        }

        void release()
        {
            scratch.setSize (0, 0);
            channels = {};
            inputBusOffsets = outputBusOffsets = std::vector<int>();
        }

        void reset()
        {
            std::fill (channels.begin(), channels.end(), nullptr);
        }

        float* setBuffer (const int idx, float* ptr = nullptr) noexcept
        {
            jassert (idx < scratch.getNumChannels());
            return channels[(size_t) idx] = uniqueBuffer (idx, ptr);
        }

        AudioBuffer<float>& getBuffer (UInt32 frames) noexcept
        {
            jassert (std::none_of (channels.begin(), channels.end(), [] (auto* x) { return x == nullptr; }));

            const auto channelPtr = channels.empty() ? scratch.getArrayOfWritePointers() : channels.data();
            mutableBuffer.setDataToReferTo (channelPtr, (int) channels.size(), static_cast<int> (frames));

            return mutableBuffer;
        }

        void set (int bus, AudioBufferList& bufferList, const int* channelMap) noexcept
        {
            if (bufferList.mNumberBuffers <= 0 || ! isPositiveAndBelow (bus, inputBusOffsets.size() - 1))
                return;

            const auto n = (UInt32) (bufferList.mBuffers[0].mDataByteSize / (bufferList.mBuffers[0].mNumberChannels * sizeof (float)));
            const auto isInterleaved = isAudioBufferInterleaved (bufferList);
            const auto numChannels = (int) (isInterleaved ? bufferList.mBuffers[0].mNumberChannels
                                                          : bufferList.mNumberBuffers);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = channels[(size_t) (inputBusOffsets[(size_t) bus] + ch)];

                const auto mappedChannel = channelMap[ch];

                if (isInterleaved || static_cast<float*> (bufferList.mBuffers[mappedChannel].mData) != data)
                    copyAudioBuffer (bufferList, mappedChannel, n, data);
            }
        }

        void get (int bus, AudioBufferList& buffer, const int* channelMap) noexcept
        {
            if (buffer.mNumberBuffers <= 0 || ! isPositiveAndBelow (bus, outputBusOffsets.size() - 1))
                return;

            const auto n = (UInt32) (buffer.mBuffers[0].mDataByteSize / (buffer.mBuffers[0].mNumberChannels * sizeof (float)));
            const auto isInterleaved = isAudioBufferInterleaved (buffer);
            const auto numChannels = (int) (isInterleaved ? buffer.mBuffers[0].mNumberChannels
                                                          : buffer.mNumberBuffers);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = channels[(size_t) (outputBusOffsets[(size_t) bus] + ch)];

                const auto mappedChannel = channelMap[ch];

                if (data == buffer.mBuffers[mappedChannel].mData && ! isInterleaved)
                    continue; // no copying necessary

                if (buffer.mBuffers[mappedChannel].mData == nullptr && ! isInterleaved)
                    buffer.mBuffers[mappedChannel].mData = data;
                else
                    copyAudioBuffer (data, mappedChannel, n, buffer);
            }
        }

        void clearInputBus (int index, int bufferLength)
        {
            if (isPositiveAndBelow (index, inputBusOffsets.size() - 1))
                clearChannels ({ inputBusOffsets[(size_t) index], inputBusOffsets[(size_t) (index + 1)] }, bufferLength);
        }

        void clearUnusedChannels (int bufferLength)
        {
            jassert (! inputBusOffsets .empty());
            jassert (! outputBusOffsets.empty());

            clearChannels ({ inputBusOffsets.back(), outputBusOffsets.back() }, bufferLength);
        }

    private:
        void clearChannels (Range<int> range, int bufferLength)
        {
            jassert (bufferLength <= scratch.getNumSamples());

            if (range.getEnd() <= (int) channels.size())
            {
                std::for_each (channels.begin() + range.getStart(),
                               channels.begin() + range.getEnd(),
                               [bufferLength] (float* ptr) { zeromem (ptr, sizeof (float) * (size_t) bufferLength); });
            }
        }

        float* uniqueBuffer (int idx, float* buffer) noexcept
        {
            if (buffer == nullptr)
                return scratch.getWritePointer (idx);

            for (int ch = 0; ch < idx; ++ch)
                if (buffer == channels[(size_t) ch])
                    return scratch.getWritePointer (idx);

            return buffer;
        }

        //==============================================================================
        AudioBuffer<float> scratch, mutableBuffer;
        std::vector<float*> channels;
        std::vector<int> inputBusOffsets, outputBusOffsets;
    };

    static bool isAudioBufferInterleaved (const AudioBufferList& audioBuffer) noexcept
    {
        return (audioBuffer.mNumberBuffers == 1 && audioBuffer.mBuffers[0].mNumberChannels > 1);
    }

    static void clearAudioBuffer (const AudioBufferList& audioBuffer) noexcept
    {
        for (unsigned int ch = 0; ch < audioBuffer.mNumberBuffers; ++ch)
            zeromem (audioBuffer.mBuffers[ch].mData, audioBuffer.mBuffers[ch].mDataByteSize);
    }

    static void copyAudioBuffer (const AudioBufferList& audioBuffer, const int channel, const UInt32 size, float* dst) noexcept
    {
        if (! isAudioBufferInterleaved (audioBuffer))
        {
            jassert (channel < static_cast<int> (audioBuffer.mNumberBuffers));
            jassert (audioBuffer.mBuffers[channel].mDataByteSize == (size * sizeof (float)));

            memcpy (dst, audioBuffer.mBuffers[channel].mData, size * sizeof (float));
        }
        else
        {
            const int numChannels = static_cast<int> (audioBuffer.mBuffers[0].mNumberChannels);
            const UInt32 n = static_cast<UInt32> (numChannels) * size;
            const float* src = static_cast<const float*> (audioBuffer.mBuffers[0].mData);

            jassert (channel < numChannels);
            jassert (audioBuffer.mBuffers[0].mDataByteSize == (n * sizeof (float)));

            for (const float* inData = src; inData < (src + n); inData += numChannels)
                *dst++ = inData[channel];
        }
    }

    static void copyAudioBuffer (const float *src, const int channel, const UInt32 size, AudioBufferList& audioBuffer) noexcept
    {
        if (! isAudioBufferInterleaved (audioBuffer))
        {
            jassert (channel < static_cast<int> (audioBuffer.mNumberBuffers));
            jassert (audioBuffer.mBuffers[channel].mDataByteSize == (size * sizeof (float)));

            memcpy (audioBuffer.mBuffers[channel].mData, src, size * sizeof (float));
        }
        else
        {
            const int numChannels = static_cast<int> (audioBuffer.mBuffers[0].mNumberChannels);
            const UInt32 n = static_cast<UInt32> (numChannels) * size;
            float* dst = static_cast<float*> (audioBuffer.mBuffers[0].mData);

            jassert (channel < numChannels);
            jassert (audioBuffer.mBuffers[0].mDataByteSize == (n * sizeof (float)));

            for (float* outData = dst; outData < (dst + n); outData += numChannels)
                outData[channel] = *src++;
        }
    }

    template <size_t numLayouts>
    static bool isLayoutSupported (const AudioProcessor& processor,
                                   bool isInput, int busIdx,
                                   int numChannels,
                                   const short (&channelLayoutList)[numLayouts][2],
                                   bool hasLayoutMap = true)
    {
        if (const AudioProcessor::Bus* bus = processor.getBus (isInput, busIdx))
        {
            if (! bus->isNumberOfChannelsSupported (numChannels))
                return false;

            if (! hasLayoutMap)
                return true;

            const int numConfigs = sizeof (channelLayoutList) / sizeof (short[2]);

            for (int i = 0; i < numConfigs; ++i)
            {
                if (channelLayoutList[i][isInput ? 0 : 1] == numChannels)
                    return true;
            }
        }

        return false;
    }

    static Array<AUChannelInfo> getAUChannelInfo (const AudioProcessor& processor)
    {
        Array<AUChannelInfo> channelInfo;

        auto hasMainInputBus  = (AudioUnitHelpers::getBusCountForWrapper (processor, true)  > 0);
        auto hasMainOutputBus = (AudioUnitHelpers::getBusCountForWrapper (processor, false) > 0);

        if ((! hasMainInputBus) && (! hasMainOutputBus))
        {
            // midi effect plug-in: no audio
            AUChannelInfo info;
            info.inChannels = 0;
            info.outChannels = 0;

            return { &info, 1 };
        }

        auto layout = processor.getBusesLayout();
        auto maxNumChanToCheckFor = 9;

        auto defaultInputs  = processor.getChannelCountOfBus (true,  0);
        auto defaultOutputs = processor.getChannelCountOfBus (false, 0);

        struct Channels
        {
            SInt16 ins, outs;

            std::pair<SInt16, SInt16> makePair() const noexcept { return std::make_pair (ins, outs); }

            bool operator<  (const Channels& other) const noexcept { return makePair() <  other.makePair(); }
            bool operator== (const Channels& other) const noexcept { return makePair() == other.makePair(); }
        };

        SortedSet<Channels> supportedChannels;

        // add the current configuration
        if (defaultInputs != 0 || defaultOutputs != 0)
            supportedChannels.add ({ static_cast<SInt16> (defaultInputs),
                                     static_cast<SInt16> (defaultOutputs) });

        for (auto inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
        {
            auto inLayout = layout;

            if (auto* inBus = processor.getBus (true, 0))
                if (! isNumberOfChannelsSupported (inBus, inChanNum, inLayout))
                    continue;

            for (auto outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
            {
                auto outLayout = inLayout;

                if (auto* outBus = processor.getBus (false, 0))
                    if (! isNumberOfChannelsSupported (outBus, outChanNum, outLayout))
                        continue;

                supportedChannels.add ({ static_cast<SInt16> (hasMainInputBus  ? outLayout.getMainInputChannels()  : 0),
                                         static_cast<SInt16> (hasMainOutputBus ? outLayout.getMainOutputChannels() : 0) });
            }
        }

        auto hasInOutMismatch = false;

        for (const auto& supported : supportedChannels)
        {
            if (supported.ins != supported.outs)
            {
                hasInOutMismatch = true;
                break;
            }
        }

        auto hasUnsupportedInput = ! hasMainInputBus, hasUnsupportedOutput = ! hasMainOutputBus;

        for (auto inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
        {
            Channels channelConfiguration { static_cast<SInt16> (inChanNum),
                                            static_cast<SInt16> (hasInOutMismatch ? defaultOutputs : inChanNum) };

            if (! supportedChannels.contains (channelConfiguration))
            {
                hasUnsupportedInput = true;
                break;
            }
        }

        for (auto outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
        {
            Channels channelConfiguration { static_cast<SInt16> (hasInOutMismatch ? defaultInputs : outChanNum),
                                            static_cast<SInt16> (outChanNum) };

            if (! supportedChannels.contains (channelConfiguration))
            {
                hasUnsupportedOutput = true;
                break;
            }
        }

        for (const auto& supported : supportedChannels)
        {
            AUChannelInfo info;

            // see here: https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/TheAudioUnit.html
            info.inChannels  = static_cast<SInt16> (hasMainInputBus  ? (hasUnsupportedInput  ? supported.ins  : (hasInOutMismatch && (! hasUnsupportedOutput) ? -2 : -1)) : 0);
            info.outChannels = static_cast<SInt16> (hasMainOutputBus ? (hasUnsupportedOutput ? supported.outs : (hasInOutMismatch && (! hasUnsupportedInput)  ? -2 : -1)) : 0);

            if (info.inChannels == -2 && info.outChannels == -2)
                info.inChannels = -1;

            int j;
            for (j = 0; j < channelInfo.size(); ++j)
                if (info.inChannels == channelInfo.getReference (j).inChannels
                      && info.outChannels == channelInfo.getReference (j).outChannels)
                    break;

            if (j >= channelInfo.size())
                channelInfo.add (info);
        }

        return channelInfo;
    }

    static bool isNumberOfChannelsSupported (const AudioProcessor::Bus* b, int numChannels, AudioProcessor::BusesLayout& inOutCurrentLayout)
    {
        auto potentialSets = AudioChannelSet::channelSetsWithNumberOfChannels (static_cast<int> (numChannels));

        for (auto set : potentialSets)
        {
            auto copy = inOutCurrentLayout;

            if (b->isLayoutSupported (set, &copy))
            {
                inOutCurrentLayout = copy;
                return true;
            }
        }

        return false;
    }

    //==============================================================================
    static int getBusCount (const AudioProcessor& juceFilter, bool isInput)
    {
        int busCount = juceFilter.getBusCount (isInput);

       #ifdef JucePlugin_PreferredChannelConfigurations
        short configs[][2] = {JucePlugin_PreferredChannelConfigurations};
        const int numConfigs = sizeof (configs) / sizeof (short[2]);

        bool hasOnlyZeroChannels = true;

        for (int i = 0; i < numConfigs && hasOnlyZeroChannels == true; ++i)
            if (configs[i][isInput ? 0 : 1] != 0)
                hasOnlyZeroChannels = false;

        busCount = jmin (busCount, hasOnlyZeroChannels ? 0 : 1);
       #endif

        return busCount;
    }

    static int getBusCountForWrapper (const AudioProcessor& juceFilter, bool isInput)
    {
       #if JucePlugin_IsMidiEffect
        const auto numRequiredBuses = isInput ? 0 : 1;
       #else
        const auto numRequiredBuses = 0;
       #endif

        return jmax (numRequiredBuses, getBusCount (juceFilter, isInput));
    }

    static bool setBusesLayout (AudioProcessor* juceFilter, const AudioProcessor::BusesLayout& requestedLayouts)
    {
       #ifdef JucePlugin_PreferredChannelConfigurations
        AudioProcessor::BusesLayout copy (requestedLayouts);

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);

            const int actualBuses = juceFilter->getBusCount (isInput);
            const int auNumBuses  = getBusCount (*juceFilter, isInput);
            Array<AudioChannelSet>& buses = (isInput ? copy.inputBuses : copy.outputBuses);

            for (int i = auNumBuses; i < actualBuses; ++i)
                buses.add (AudioChannelSet::disabled());
        }

        return juceFilter->setBusesLayout (copy);
       #else
        return juceFilter->setBusesLayout (requestedLayouts);
       #endif
    }

    static AudioProcessor::BusesLayout getBusesLayout (const AudioProcessor* juceFilter)
    {
       #ifdef JucePlugin_PreferredChannelConfigurations
        AudioProcessor::BusesLayout layout = juceFilter->getBusesLayout();

        for (int dir = 0; dir < 2; ++dir)
        {
            const bool isInput = (dir == 0);

            const int actualBuses = juceFilter->getBusCount (isInput);
            const int auNumBuses  = getBusCount (*juceFilter, isInput);
            auto& buses = (isInput ? layout.inputBuses : layout.outputBuses);

            for (int i = auNumBuses; i < actualBuses; ++i)
                buses.removeLast();
        }

        return layout;
       #else
        return juceFilter->getBusesLayout();
       #endif
    }
};

} // namespace juce

#endif
