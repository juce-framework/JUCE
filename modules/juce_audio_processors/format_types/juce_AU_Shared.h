/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

/** @cond */
// This macro can be set if you need to override this internal name for some reason..
#ifndef JUCE_STATE_DICTIONARY_KEY
 #define JUCE_STATE_DICTIONARY_KEY   "jucePluginState"
#endif


#if (JUCE_IOS && JUCE_IOS_API_VERSION_CAN_BE_BUILT (15, 0)) \
   || (JUCE_MAC && JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0))
 #define JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED 1
#else
 #define JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED 0
#endif

#include <juce_audio_basics/midi/juce_MidiDataConcatenator.h>

#if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
 #include <juce_audio_basics/midi/ump/juce_UMP.h>
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

    struct Channels
    {
        SInt16 ins, outs;

        std::pair<SInt16, SInt16> makePair() const noexcept { return std::make_pair (ins, outs); }
        AUChannelInfo makeChannelInfo() const noexcept { return { ins, outs }; }

        bool operator<  (const Channels& other) const noexcept { return makePair() <  other.makePair(); }
        bool operator== (const Channels& other) const noexcept { return makePair() == other.makePair(); }

        // The 'standard' layout with the most channels defined is AudioChannelSet::create9point1point6().
        // This value should be updated if larger standard channel layouts are added in the future.
        static constexpr auto maxNumChanToCheckFor = 16;
    };

    /*  Removes non-wildcard layouts that are already included by other wildcard layouts.
    */
    static void removeNonWildcardLayouts (std::set<Channels>& layouts)
    {
        for (auto it = layouts.begin(); it != layouts.end();)
        {
            if ((it->ins != -1 && layouts.find ({ -1, it->outs }) != layouts.end())
                || (it->outs != -1 && layouts.find ({ it->ins, -1 }) != layouts.end()))
            {
                it = layouts.erase (it);
            }
            else
            {
                ++it;
            }
        }
    }

    static std::set<Channels> getAUChannelInfo (const AudioProcessor& processor)
    {
       #ifdef JucePlugin_AUMainType
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-char-constants")
        if constexpr (JucePlugin_AUMainType == kAudioUnitType_MIDIProcessor)
        {
            // A MIDI effect requires an output bus in order to determine the sample rate.
            // No audio will be written to the output bus, so it can have any number of channels.
            // No input bus is required.
            return { Channels { 0, -1 } };
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
       #endif

        const auto defaultInputs  = processor.getChannelCountOfBus (true,  0);
        const auto defaultOutputs = processor.getChannelCountOfBus (false, 0);
        const auto hasMainInputBus  = (AudioUnitHelpers::getBusCountForWrapper (processor, true)  > 0);
        const auto hasMainOutputBus = (AudioUnitHelpers::getBusCountForWrapper (processor, false) > 0);

        std::set<Channels> supportedChannels;

        // add the current configuration
        if (defaultInputs != 0 || defaultOutputs != 0)
            supportedChannels.insert ({ static_cast<SInt16> (defaultInputs),
                                        static_cast<SInt16> (defaultOutputs) });

        static const auto layoutsToTry = std::invoke ([&]
        {
            std::vector<AudioChannelSet> sets;

            for (auto i = 1; i <= Channels::maxNumChanToCheckFor; ++i)
            {
                const auto setsWithSizeI = AudioChannelSet::channelSetsWithNumberOfChannels (i);
                std::copy (setsWithSizeI.begin(), setsWithSizeI.end(), std::back_inserter (sets));
            }

            return sets;
        });

        std::vector<char> inputHasOutputRestrictions (layoutsToTry.size()), outputHasInputRestrictions (layoutsToTry.size());

        for (const auto [inputIndex, inputLayout] : enumerate (layoutsToTry))
        {
            for (const auto [outputIndex, outputLayout] : enumerate (layoutsToTry))
            {
                auto copy = processor.getBusesLayout();

                if (! copy.inputBuses.isEmpty())
                    copy.inputBuses.getReference (0) = inputLayout;

                if (! copy.outputBuses.isEmpty())
                    copy.outputBuses.getReference (0) = outputLayout;

                if (processor.checkBusesLayoutSupported (copy))
                {
                    supportedChannels.insert ({ (SInt16) inputLayout.size(), (SInt16) outputLayout.size() });
                }
                else
                {
                    inputHasOutputRestrictions[(size_t) inputIndex]  = true;
                    outputHasInputRestrictions[(size_t) outputIndex] = true;
                }
            }
        }

        static constexpr auto identity = [] (auto x) { return x; };
        const auto noRestrictions = std::none_of (inputHasOutputRestrictions.begin(), inputHasOutputRestrictions.end(), identity)
                                 && std::none_of (outputHasInputRestrictions.begin(), outputHasInputRestrictions.end(), identity);

        if (noRestrictions)
        {
            if (hasMainInputBus)
            {
                if (hasMainOutputBus)
                    return { Channels { -1, -2 } };

                return { Channels { -1, 0 } };
            }

            return { Channels { 0, -1 } };
        }

        const auto allMatchedLayoutsExclusivelySupported = std::invoke ([&]
        {
            for (SInt16 i = 1; i <= Channels::maxNumChanToCheckFor; ++i)
                if (supportedChannels.find ({ i, i }) == supportedChannels.end())
                    return false;

            return std::all_of (supportedChannels.begin(), supportedChannels.end(), [] (auto x) { return x.ins == x.outs; });
        });

        if (allMatchedLayoutsExclusivelySupported)
            return { Channels { -1, -1 } };

        std::set<Channels> filteredChannels;

        for (auto& c : supportedChannels)
        {
            const auto findDistance = [&] (auto channelCount)
            {
                return std::distance (layoutsToTry.begin(),
                                      std::lower_bound (layoutsToTry.begin(),
                                                        layoutsToTry.end(),
                                                        channelCount,
                                                        [] (auto a, auto b) { return a.size() < b; }));
            };

            const auto findChannelCount = [&] (auto& restrictions,
                                               auto thisChannelCount,
                                               auto otherChannelCount,
                                               auto hasMainBus)
            {
                const auto lower = findDistance (otherChannelCount);
                const auto upper = findDistance (otherChannelCount + 1);

                const auto wildcard = std::all_of (restrictions.begin() + lower,
                                                   restrictions.begin() + upper,
                                                   identity)
                                    ? thisChannelCount
                                    : (SInt16) -1;
                return hasMainBus ? wildcard : (SInt16) 0;
            };

            const auto ins  = findChannelCount (outputHasInputRestrictions, c.ins, c.outs, hasMainInputBus);
            const auto outs = findChannelCount (inputHasOutputRestrictions, c.outs, c.ins, hasMainOutputBus);

            const Channels layout { ins, outs };
            filteredChannels.insert (layout == Channels { -1, -1 } ? c : layout);
        }

        removeNonWildcardLayouts (filteredChannels);

        return filteredChannels;
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
       #ifdef JucePlugin_AUMainType
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfour-char-constants")
        constexpr auto pluginIsMidiEffect = JucePlugin_AUMainType == kAudioUnitType_MIDIProcessor;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
       #else
        constexpr auto pluginIsMidiEffect = false;
       #endif

        const auto numRequiredBuses = (isInput || ! pluginIsMidiEffect) ? 0 : 1;

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

   #if JUCE_APPLE_MIDI_EVENT_LIST_SUPPORTED
    class ScopedMIDIEventListBlock
    {
    public:
        ScopedMIDIEventListBlock() = default;

        ScopedMIDIEventListBlock (ScopedMIDIEventListBlock&& other) noexcept
            : midiEventListBlock (std::exchange (other.midiEventListBlock, nil)) {}

        ScopedMIDIEventListBlock& operator= (ScopedMIDIEventListBlock&& other) noexcept
        {
            ScopedMIDIEventListBlock { std::move (other) }.swap (*this);
            return *this;
        }

        ~ScopedMIDIEventListBlock()
        {
            if (midiEventListBlock != nil)
                [midiEventListBlock release];
        }

        static ScopedMIDIEventListBlock copy (AUMIDIEventListBlock b)
        {
            return ScopedMIDIEventListBlock { b };
        }

        explicit operator bool() const { return midiEventListBlock != nil; }

        void operator() (AUEventSampleTime eventSampleTime, uint8_t cable, const struct MIDIEventList * eventList) const
        {
            jassert (midiEventListBlock != nil);
            midiEventListBlock (eventSampleTime, cable, eventList);
        }

    private:
        void swap (ScopedMIDIEventListBlock& other) noexcept
        {
            std::swap (other.midiEventListBlock, midiEventListBlock);
        }

        explicit ScopedMIDIEventListBlock (AUMIDIEventListBlock b) : midiEventListBlock ([b copy]) {}

        AUMIDIEventListBlock midiEventListBlock = nil;
    };

    class EventListOutput
    {
    public:
        API_AVAILABLE (macos (12.0), ios (15.0))
        void setBlock (ScopedMIDIEventListBlock x)
        {
            block = std::move (x);
        }

        API_AVAILABLE (macos (12.0), ios (15.0))
        void setBlock (AUMIDIEventListBlock x)
        {
            setBlock (ScopedMIDIEventListBlock::copy (x));
        }

        bool trySend (const MidiBuffer& buffer, int64_t baseTimeStamp)
        {
            if (! block)
                return false;

            struct MIDIEventList stackList = {};
            MIDIEventPacket* end = nullptr;

            const auto init = [&]
            {
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
                end = MIDIEventListInit (&stackList, kMIDIProtocol_1_0);
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            };

            const auto send = [&]
            {
                block (baseTimeStamp, 0, &stackList);
            };

            const auto add = [&] (const ump::View& view, int timeStamp)
            {
                static_assert (sizeof (uint32_t) == sizeof (UInt32)
                               && alignof (uint32_t) == alignof (UInt32),
                               "If this fails, the cast below will be broken too!");
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")
                using List = struct MIDIEventList;
                end = MIDIEventListAdd (&stackList,
                                        sizeof (List::packet),
                                        end,
                                        (MIDITimeStamp) timeStamp,
                                        view.size(),
                                        reinterpret_cast<const UInt32*> (view.data()));
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            };

            init();

            for (const auto metadata : buffer)
            {
                toUmp1Converter.convert (ump::BytestreamMidiView (metadata), [&] (const ump::View& view)
                {
                    add (view, metadata.samplePosition);

                    if (end != nullptr)
                        return;

                    send();
                    init();
                    add (view, metadata.samplePosition);
                });
            }

            send();

            return true;
        }

    private:
        ScopedMIDIEventListBlock block;
        ump::ToUMP1Converter toUmp1Converter;
    };
   #endif
};

} // namespace juce
/** @endcond */
