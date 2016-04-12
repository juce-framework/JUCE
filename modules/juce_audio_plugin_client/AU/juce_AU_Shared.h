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

// This macro can be set if you need to override this internal name for some reason..
#ifndef JUCE_STATE_DICTIONARY_KEY
 #define JUCE_STATE_DICTIONARY_KEY   "jucePluginState"
#endif

struct AudioUnitHelpers
{
    // maps a channel index into an AU format to an index of a juce format
    struct AUChannelStreamOrder
    {
        AudioChannelLayoutTag auLayoutTag;
        AudioChannelLabel speakerOrder[8];
    };

    static AUChannelStreamOrder auChannelStreamOrder[];

    static AudioChannelSet::ChannelType CoreAudioChannelLabelToJuceType (AudioChannelLabel label) noexcept
    {
        if (label >= kAudioChannelLabel_Discrete_0 && label <= kAudioChannelLabel_Discrete_65535)
        {
            const unsigned int discreteChannelNum = label - kAudioChannelLabel_Discrete_0;
            return static_cast<AudioChannelSet::ChannelType> (AudioChannelSet::discreteChannel0 + discreteChannelNum);
        }

        switch (label)
        {
            case kAudioChannelLabel_Center:
            case kAudioChannelLabel_Mono:                   return AudioChannelSet::centre;
            case kAudioChannelLabel_Left:
            case kAudioChannelLabel_HeadphonesLeft:         return AudioChannelSet::left;
            case kAudioChannelLabel_Right:
            case kAudioChannelLabel_HeadphonesRight:        return AudioChannelSet::right;
            case kAudioChannelLabel_LFEScreen:              return AudioChannelSet::subbass;
            case kAudioChannelLabel_LeftSurround:           return AudioChannelSet::leftSurround;
            case kAudioChannelLabel_RightSurround:          return AudioChannelSet::rightSurround;
            case kAudioChannelLabel_LeftCenter:             return AudioChannelSet::leftCentre;
            case kAudioChannelLabel_RightCenter:            return AudioChannelSet::rightCentre;
            case kAudioChannelLabel_CenterSurround:         return AudioChannelSet::surround;
            case kAudioChannelLabel_LeftSurroundDirect:     return AudioChannelSet::leftSurroundDirect;
            case kAudioChannelLabel_RightSurroundDirect:    return AudioChannelSet::rightSurroundDirect;
            case kAudioChannelLabel_TopCenterSurround:      return AudioChannelSet::topMiddle;
            case kAudioChannelLabel_VerticalHeightLeft:     return AudioChannelSet::topFrontLeft;
            case kAudioChannelLabel_VerticalHeightRight:    return AudioChannelSet::topFrontRight;
            case kAudioChannelLabel_VerticalHeightCenter:   return AudioChannelSet::topFrontCentre;
            case kAudioChannelLabel_TopBackLeft:            return AudioChannelSet::topRearLeft;
            case kAudioChannelLabel_RearSurroundLeft:       return AudioChannelSet::leftRearSurround;
            case kAudioChannelLabel_TopBackRight:           return AudioChannelSet::topRearRight;
            case kAudioChannelLabel_RearSurroundRight:      return AudioChannelSet::rightRearSurround;
            case kAudioChannelLabel_TopBackCenter:          return AudioChannelSet::topRearCentre;
            case kAudioChannelLabel_LFE2:                   return AudioChannelSet::subbass2;
            case kAudioChannelLabel_LeftWide:               return AudioChannelSet::wideLeft;
            case kAudioChannelLabel_RightWide:              return AudioChannelSet::wideRight;
            case kAudioChannelLabel_Ambisonic_W:            return AudioChannelSet::ambisonicW;
            case kAudioChannelLabel_Ambisonic_X:            return AudioChannelSet::ambisonicX;
            case kAudioChannelLabel_Ambisonic_Y:            return AudioChannelSet::ambisonicY;
            case kAudioChannelLabel_Ambisonic_Z:            return AudioChannelSet::ambisonicZ;
            default:                                        return AudioChannelSet::unknown;
        }
    }

    static AudioChannelLabel JuceChannelTypeToCoreAudioLabel (const AudioChannelSet::ChannelType& label) noexcept
    {
        if (label >= AudioChannelSet::discreteChannel0)
        {
            const unsigned int discreteChannelNum = label - AudioChannelSet::discreteChannel0;;
            return static_cast<AudioChannelLabel> (kAudioChannelLabel_Discrete_0 + discreteChannelNum);
        }

        switch (label)
        {
            case AudioChannelSet::centre:              return kAudioChannelLabel_Center;
            case AudioChannelSet::left:                return kAudioChannelLabel_Left;
            case AudioChannelSet::right:               return kAudioChannelLabel_Right;
            case AudioChannelSet::subbass:             return kAudioChannelLabel_LFEScreen;
            case AudioChannelSet::leftRearSurround:    return kAudioChannelLabel_RearSurroundLeft;
            case AudioChannelSet::rightRearSurround:   return kAudioChannelLabel_RearSurroundRight;
            case AudioChannelSet::leftCentre:          return kAudioChannelLabel_LeftCenter;
            case AudioChannelSet::rightCentre:         return kAudioChannelLabel_RightCenter;
            case AudioChannelSet::surround:            return kAudioChannelLabel_CenterSurround;
            case AudioChannelSet::leftSurround:        return kAudioChannelLabel_LeftSurround;
            case AudioChannelSet::rightSurround:       return kAudioChannelLabel_RightSurround;
            case AudioChannelSet::topMiddle:           return kAudioChannelLabel_TopCenterSurround;
            case AudioChannelSet::topFrontLeft:        return kAudioChannelLabel_VerticalHeightLeft;
            case AudioChannelSet::topFrontRight:       return kAudioChannelLabel_VerticalHeightRight;
            case AudioChannelSet::topFrontCentre:      return kAudioChannelLabel_VerticalHeightCenter;
            case AudioChannelSet::topRearLeft:         return kAudioChannelLabel_TopBackLeft;
            case AudioChannelSet::topRearRight:        return kAudioChannelLabel_TopBackRight;
            case AudioChannelSet::topRearCentre:       return kAudioChannelLabel_TopBackCenter;
            case AudioChannelSet::subbass2:            return kAudioChannelLabel_LFE2;
            case AudioChannelSet::wideLeft:            return kAudioChannelLabel_LeftWide;
            case AudioChannelSet::wideRight:           return kAudioChannelLabel_RightWide;
            case AudioChannelSet::ambisonicW:          return kAudioChannelLabel_Ambisonic_W;
            case AudioChannelSet::ambisonicX:          return kAudioChannelLabel_Ambisonic_X;
            case AudioChannelSet::ambisonicY:          return kAudioChannelLabel_Ambisonic_Y;
            case AudioChannelSet::ambisonicZ:          return kAudioChannelLabel_Ambisonic_Z;
            case AudioChannelSet::leftSurroundDirect:  return kAudioChannelLabel_LeftSurroundDirect;
            case AudioChannelSet::rightSurroundDirect: return kAudioChannelLabel_RightSurroundDirect;
            case AudioChannelSet::unknown:             return kAudioChannelLabel_Unknown;
            case AudioChannelSet::discreteChannel0:    return kAudioChannelLabel_Discrete_0;
        }

        return kAudioChannelLabel_Unknown;
    }

    static AudioChannelSet CoreAudioChannelBitmapToJuceType (UInt32 bitmap) noexcept
    {
        AudioChannelSet set;

        if ((bitmap & kAudioChannelBit_Left)                 != 0) set.addChannel (AudioChannelSet::left);
        if ((bitmap & kAudioChannelBit_Right)                != 0) set.addChannel (AudioChannelSet::right);
        if ((bitmap & kAudioChannelBit_Center)               != 0) set.addChannel (AudioChannelSet::centre);
        if ((bitmap & kAudioChannelBit_LFEScreen)            != 0) set.addChannel (AudioChannelSet::subbass);
        if ((bitmap & kAudioChannelBit_LeftSurroundDirect)   != 0) set.addChannel (AudioChannelSet::leftSurroundDirect);
        if ((bitmap & kAudioChannelBit_RightSurroundDirect)  != 0) set.addChannel (AudioChannelSet::rightSurroundDirect);
        if ((bitmap & kAudioChannelBit_LeftCenter)           != 0) set.addChannel (AudioChannelSet::leftCentre);
        if ((bitmap & kAudioChannelBit_RightCenter)          != 0) set.addChannel (AudioChannelSet::rightCentre);
        if ((bitmap & kAudioChannelBit_CenterSurround)       != 0) set.addChannel (AudioChannelSet::surround);
        if ((bitmap & kAudioChannelBit_LeftSurround)         != 0) set.addChannel (AudioChannelSet::leftSurround);
        if ((bitmap & kAudioChannelBit_RightSurround)        != 0) set.addChannel (AudioChannelSet::rightSurround);
        if ((bitmap & kAudioChannelBit_TopCenterSurround)    != 0) set.addChannel (AudioChannelSet::topMiddle);
        if ((bitmap & kAudioChannelBit_VerticalHeightLeft)   != 0) set.addChannel (AudioChannelSet::topFrontLeft);
        if ((bitmap & kAudioChannelBit_VerticalHeightCenter) != 0) set.addChannel (AudioChannelSet::topFrontCentre);
        if ((bitmap & kAudioChannelBit_VerticalHeightRight)  != 0) set.addChannel (AudioChannelSet::topFrontRight);
        if ((bitmap & kAudioChannelBit_TopBackLeft)          != 0) set.addChannel (AudioChannelSet::topRearLeft);
        if ((bitmap & kAudioChannelBit_TopBackCenter)        != 0) set.addChannel (AudioChannelSet::topRearCentre);
        if ((bitmap & kAudioChannelBit_TopBackRight)         != 0) set.addChannel (AudioChannelSet::topRearRight);

        return set;
    }

    static AudioChannelSet CoreAudioChannelLayoutToJuceType (const AudioChannelLayout& layout) noexcept
    {
        const AudioChannelLayoutTag tag = layout.mChannelLayoutTag;

        if (tag == kAudioChannelLayoutTag_UseChannelBitmap)         return CoreAudioChannelBitmapToJuceType (layout.mChannelBitmap);
        if (tag == kAudioChannelLayoutTag_UseChannelDescriptions)
        {
            if (layout.mNumberChannelDescriptions <= 8)
            {
                // first try to convert the layout via the auChannelStreamOrder array
                int layoutIndex;
                for (layoutIndex = 0; auChannelStreamOrder[layoutIndex].auLayoutTag != 0; ++layoutIndex)
                {
                    const AUChannelStreamOrder& streamOrder = auChannelStreamOrder[layoutIndex];

                    int numChannels;
                    for (numChannels = 0; numChannels < 8 && streamOrder.speakerOrder[numChannels] != 0;)
                        ++numChannels;

                    if (numChannels != (int) layout.mNumberChannelDescriptions)
                        continue;

                    int ch;
                    for (ch = 0; ch < numChannels; ++ch)
                        if (streamOrder.speakerOrder[ch] != layout.mChannelDescriptions[ch].mChannelLabel)
                            break;

                    // match!
                    if (ch == numChannels)
                        break;
                }

                if (auChannelStreamOrder[layoutIndex].auLayoutTag != 0)
                    return CALayoutTagToChannelSet (auChannelStreamOrder[layoutIndex].auLayoutTag);
            }
            AudioChannelSet set;
            for (unsigned int i = 0; i < layout.mNumberChannelDescriptions; ++i)
                set.addChannel (CoreAudioChannelLabelToJuceType (layout.mChannelDescriptions[i].mChannelLabel));

            return set;
        }

        return CALayoutTagToChannelSet (tag);
    }

    static AudioChannelSet CALayoutTagToChannelSet (AudioChannelLayoutTag tag) noexcept
    {
        switch (tag)
        {
            case kAudioChannelLayoutTag_Unknown:                return AudioChannelSet::disabled();
            case kAudioChannelLayoutTag_Mono:                   return AudioChannelSet::mono();
            case kAudioChannelLayoutTag_Stereo:
            case kAudioChannelLayoutTag_StereoHeadphones:
            case kAudioChannelLayoutTag_Binaural:               return AudioChannelSet::stereo();
            case kAudioChannelLayoutTag_Quadraphonic:           return AudioChannelSet::quadraphonic();
            case kAudioChannelLayoutTag_Pentagonal:             return AudioChannelSet::pentagonal();
            case kAudioChannelLayoutTag_Hexagonal:              return AudioChannelSet::hexagonal();
            case kAudioChannelLayoutTag_Octagonal:              return AudioChannelSet::octagonal();
            case kAudioChannelLayoutTag_Ambisonic_B_Format:     return AudioChannelSet::ambisonic();
            case kAudioChannelLayoutTag_AudioUnit_6_0:          return AudioChannelSet::create6point0();
            case kAudioChannelLayoutTag_DTS_6_0_A:              return AudioChannelSet::create6point0Music();
            case kAudioChannelLayoutTag_MPEG_6_1_A:             return AudioChannelSet::create6point1();
            case kAudioChannelLayoutTag_MPEG_5_0_B:             return AudioChannelSet::create5point0();
            case kAudioChannelLayoutTag_MPEG_5_1_A:             return AudioChannelSet::create5point1();
            case kAudioChannelLayoutTag_DTS_7_1:
            case kAudioChannelLayoutTag_MPEG_7_1_C:             return AudioChannelSet::create7point1();
            case kAudioChannelLayoutTag_AudioUnit_7_0:          return AudioChannelSet::create7point0();
            case kAudioChannelLayoutTag_AudioUnit_7_0_Front:    return AudioChannelSet::createFront7point0();
            case kAudioChannelLayoutTag_AudioUnit_7_1_Front:    return AudioChannelSet::createFront7point1();
            case kAudioChannelLayoutTag_MPEG_3_0_A:
            case kAudioChannelLayoutTag_MPEG_3_0_B:             return AudioChannelSet::createLCR();
            case kAudioChannelLayoutTag_MPEG_4_0_A:
            case kAudioChannelLayoutTag_MPEG_4_0_B:             return AudioChannelSet::createLCRS();
            case kAudioChannelLayoutTag_ITU_2_1:                return AudioChannelSet::createLRS();
            case kAudioChannelLayoutTag_EAC3_7_1_C:             return AudioChannelSet::create7point1AC3();
        }

        if (int numChannels = static_cast<int> (tag) & 0xffff)
            return AudioChannelSet::discreteChannels (numChannels);

        // Bitmap and channel description array layout tags are currently unsupported :-(
        jassertfalse;
        return AudioChannelSet();
    }

    static AudioChannelLayoutTag ChannelSetToCALayoutTag (const AudioChannelSet& set) noexcept
    {
        if (set == AudioChannelSet::mono())                  return kAudioChannelLayoutTag_Mono;
        if (set == AudioChannelSet::stereo())                return kAudioChannelLayoutTag_Stereo;
        if (set == AudioChannelSet::createLCR())             return kAudioChannelLayoutTag_MPEG_3_0_A;
        if (set == AudioChannelSet::createLRS())             return kAudioChannelLayoutTag_ITU_2_1;
        if (set == AudioChannelSet::createLCRS())            return kAudioChannelLayoutTag_MPEG_4_0_A;
        if (set == AudioChannelSet::quadraphonic())          return kAudioChannelLayoutTag_Quadraphonic;
        if (set == AudioChannelSet::pentagonal())            return kAudioChannelLayoutTag_Pentagonal;
        if (set == AudioChannelSet::hexagonal())             return kAudioChannelLayoutTag_Hexagonal;
        if (set == AudioChannelSet::octagonal())             return kAudioChannelLayoutTag_Octagonal;
        if (set == AudioChannelSet::ambisonic())             return kAudioChannelLayoutTag_Ambisonic_B_Format;
        if (set == AudioChannelSet::create5point0())         return kAudioChannelLayoutTag_MPEG_5_0_B;
        if (set == AudioChannelSet::create5point1())         return kAudioChannelLayoutTag_MPEG_5_1_A;
        if (set == AudioChannelSet::create6point0())         return kAudioChannelLayoutTag_AudioUnit_6_0;
        if (set == AudioChannelSet::create6point0Music())    return kAudioChannelLayoutTag_DTS_6_0_A;
        if (set == AudioChannelSet::create6point1())         return kAudioChannelLayoutTag_MPEG_6_1_A;
        if (set == AudioChannelSet::create7point0())         return kAudioChannelLayoutTag_AudioUnit_7_0;
        if (set == AudioChannelSet::create7point1())         return kAudioChannelLayoutTag_MPEG_7_1_C;
        if (set == AudioChannelSet::createFront7point0())    return kAudioChannelLayoutTag_AudioUnit_7_0_Front;
        if (set == AudioChannelSet::createFront7point1())    return kAudioChannelLayoutTag_AudioUnit_7_1_Front;
        if (set == AudioChannelSet::create7point1AC3())      return kAudioChannelLayoutTag_EAC3_7_1_C;
        if (set == AudioChannelSet::disabled())              return kAudioChannelLayoutTag_Unknown;

        return static_cast<AudioChannelLayoutTag> ((int) kAudioChannelLayoutTag_DiscreteInOrder | set.size());
    }

    static int auChannelIndexToJuce (int auIndex, const AudioChannelSet& channelSet)
    {
        if (auIndex >= 8) return auIndex;

        AudioChannelLayoutTag currentLayout = ChannelSetToCALayoutTag (channelSet);

        int layoutIndex;
        for (layoutIndex = 0; auChannelStreamOrder[layoutIndex].auLayoutTag != currentLayout; ++layoutIndex)
            if (auChannelStreamOrder[layoutIndex].auLayoutTag == 0) return auIndex;

        AudioChannelSet::ChannelType channelType
        = CoreAudioChannelLabelToJuceType (auChannelStreamOrder[layoutIndex].speakerOrder[auIndex]);

        // We need to map surround channels to rear surround channels for petagonal and hexagonal
        if (channelSet == AudioChannelSet::pentagonal() || channelSet == AudioChannelSet::hexagonal())
        {
            switch (channelType)
            {
                case AudioChannelSet::leftSurround:
                    channelType = AudioChannelSet::leftRearSurround;
                    break;
                case AudioChannelSet::rightSurround:
                    channelType = AudioChannelSet::rightRearSurround;
                    break;
                default:
                    break;
            }
        }

        const int juceIndex = channelSet.getChannelTypes().indexOf (channelType);

        jassert (juceIndex >= 0);
        return juceIndex >= 0 ? juceIndex : auIndex;
    }

    static int juceChannelIndexToAu (int juceIndex, const AudioChannelSet& channelSet)
    {
        AudioChannelLayoutTag currentLayout = ChannelSetToCALayoutTag (channelSet);

        int layoutIndex;
        for (layoutIndex = 0; auChannelStreamOrder[layoutIndex].auLayoutTag != currentLayout; ++layoutIndex)
        {
            if (auChannelStreamOrder[layoutIndex].auLayoutTag == 0)
            {
                jassertfalse;
                return juceIndex;
            }
        }

        const AUChannelStreamOrder& channelOrder = auChannelStreamOrder[layoutIndex];
        AudioChannelSet::ChannelType channelType = channelSet.getTypeOfChannel (juceIndex);

        // We need to map rear surround channels to surround channels for petagonal and hexagonal
        if (channelSet == AudioChannelSet::pentagonal() || channelSet == AudioChannelSet::hexagonal())
        {
            switch (channelType)
            {
                case AudioChannelSet::leftRearSurround:
                    channelType = AudioChannelSet::leftSurround;
                    break;
                case AudioChannelSet::rightRearSurround:
                    channelType = AudioChannelSet::rightSurround;
                    break;
                default:
                    break;
            }
        }

        for (int i = 0; i < 8 && channelOrder.speakerOrder[i] != 0; ++i)
            if (CoreAudioChannelLabelToJuceType (channelOrder.speakerOrder[i]) == channelType)
                return i;

        jassertfalse;
        return juceIndex;
    }

    class ChannelRemapper
    {
    public:
        ChannelRemapper (PluginBusUtilities& bUtils) : busUtils (bUtils), inputLayoutMap (nullptr), outputLayoutMap (nullptr) {}
        ~ChannelRemapper() {}

        void alloc()
        {
            const int numInputBuses  = busUtils.getBusCount (true);
            const int numOutputBuses = busUtils.getBusCount (false);

            initializeChannelMapArray (true, numInputBuses);
            initializeChannelMapArray (false, numOutputBuses);

            for (int busIdx = 0; busIdx < numInputBuses; ++busIdx)
                fillLayoutChannelMaps (true, busIdx);

            for (int busIdx = 0; busIdx < numOutputBuses; ++busIdx)
                fillLayoutChannelMaps (false, busIdx);
        }

        void release()
        {
            inputLayoutMap = outputLayoutMap = nullptr;
            inputLayoutMapPtrStorage.free();
            outputLayoutMapPtrStorage.free();
            inputLayoutMapStorage.free();
            outputLayoutMapStorage.free();
        }

        inline const int* get (bool input, int bus) const noexcept { return (input ? inputLayoutMap : outputLayoutMap) [bus]; }

    private:
        //==============================================================================
        PluginBusUtilities& busUtils;
        HeapBlock<int*> inputLayoutMapPtrStorage, outputLayoutMapPtrStorage;
        HeapBlock<int>  inputLayoutMapStorage, outputLayoutMapStorage;
        int** inputLayoutMap;
        int** outputLayoutMap;

        //==============================================================================
        void initializeChannelMapArray (bool isInput, const int numBuses)
        {
            HeapBlock<int*>& layoutMapPtrStorage = isInput ? inputLayoutMapPtrStorage : outputLayoutMapPtrStorage;
            HeapBlock<int>& layoutMapStorage = isInput ? inputLayoutMapStorage : outputLayoutMapStorage;
            int**& layoutMap = isInput ? inputLayoutMap : outputLayoutMap;

            const int totalInChannels  = busUtils.findTotalNumChannels (true);
            const int totalOutChannels = busUtils.findTotalNumChannels (false);

            layoutMapPtrStorage.calloc (static_cast<size_t> (numBuses));
            layoutMapStorage.calloc (static_cast<size_t> (isInput ? totalInChannels : totalOutChannels));

            layoutMap  = layoutMapPtrStorage. getData();

            int ch = 0;
            for (int busIdx = 0; busIdx < numBuses; ++busIdx)
            {
                layoutMap[busIdx] = layoutMapStorage.getData() + ch;
                ch += busUtils.getNumChannels (isInput, busIdx);
            }
        }

        void fillLayoutChannelMaps (bool isInput, int busNr)
        {
            int* layoutMap = (isInput ? inputLayoutMap : outputLayoutMap)[busNr];
            const AudioChannelSet& channelFormat = busUtils.getChannelSet (isInput, busNr);
            const int numChannels = channelFormat.size();

            for (int i = 0; i < numChannels; ++i)
                layoutMap[i] = AudioUnitHelpers::juceChannelIndexToAu (i, channelFormat);
        }
    };

    //==============================================================================
    class CoreAudioBufferList
    {
    public:
        CoreAudioBufferList() { reset(); }

        //==============================================================================
        void prepare (int inChannels, int outChannels, int maxFrames)
        {
            const int numChannels = jmax (inChannels, outChannels);

            scratch.setSize (numChannels, maxFrames);
            channels.calloc (static_cast<size_t> (numChannels));

            reset();
        }

        void release()
        {
            scratch.setSize (0, 0);
            channels.free();
        }

        void reset() noexcept
        {
            pushIdx = 0;
            popIdx = 0;
            zeromem (channels.getData(), sizeof(float*) * static_cast<size_t> (scratch.getNumChannels()));
        }

        //==============================================================================
        float* setBuffer (const int idx, float* ptr = nullptr) noexcept
        {
            jassert (idx < scratch.getNumChannels());
            return (channels [idx] = uniqueBuffer (idx, ptr));
        }

        //==============================================================================
        float* push() noexcept
        {
            jassert (pushIdx < scratch.getNumChannels());
            return channels [pushIdx++];
        }

        void push (AudioBufferList& bufferList, const int* channelMap) noexcept
        {
            jassert (pushIdx < scratch.getNumChannels());

            if (bufferList.mNumberBuffers > 0)
            {
                const UInt32 n = bufferList.mBuffers [0].mDataByteSize /
                (bufferList.mBuffers [0].mNumberChannels * sizeof (float));
                const bool isInterleaved = isAudioBufferInterleaved (bufferList);
                const int numChannels = static_cast<int> (isInterleaved ? bufferList.mBuffers [0].mNumberChannels
                                                          : bufferList.mNumberBuffers);

                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float* data = push();

                    int mappedChannel = channelMap [ch];
                    if (isInterleaved || static_cast<float*> (bufferList.mBuffers [mappedChannel].mData) != data)
                        copyAudioBuffer (bufferList, mappedChannel, n, data);
                }
            }
        }

        //==============================================================================
        float* pop() noexcept
        {
            jassert (popIdx < scratch.getNumChannels());
            return channels[popIdx++];
        }

        void pop (AudioBufferList& buffer, const int* channelMap) noexcept
        {
            if (buffer.mNumberBuffers > 0)
            {
                const UInt32 n = buffer.mBuffers [0].mDataByteSize / (buffer.mBuffers [0].mNumberChannels * sizeof (float));
                const bool isInterleaved = isAudioBufferInterleaved (buffer);
                const int numChannels = static_cast<int> (isInterleaved ? buffer.mBuffers [0].mNumberChannels : buffer.mNumberBuffers);

                for (int ch = 0; ch < numChannels; ++ch)
                {
                    int mappedChannel = channelMap [ch];
                    float* nextBuffer = pop();

                    if (nextBuffer == buffer.mBuffers [mappedChannel].mData && ! isInterleaved)
                        continue; // no copying necessary

                    if (buffer.mBuffers [mappedChannel].mData == nullptr && ! isInterleaved)
                        buffer.mBuffers [mappedChannel].mData = nextBuffer;
                    else
                        copyAudioBuffer (nextBuffer, mappedChannel, n, buffer);
                }
            }
        }

        //==============================================================================
        AudioSampleBuffer& getBuffer (UInt32 frames) noexcept
        {
            jassert (pushIdx == scratch.getNumChannels());

          #if JUCE_DEBUG
            for (int i = 0; i < pushIdx; ++i)
                jassert (channels [i] != nullptr);
          #endif

            mutableBuffer.setDataToReferTo (channels, pushIdx, static_cast<int> (frames));
            return mutableBuffer;
        }

    private:
        float* uniqueBuffer (int idx, float* buffer) noexcept
        {
            if (buffer == nullptr)
                return scratch.getWritePointer (idx);

            for (int ch = 0; ch < idx; ++ch)
                if (buffer == channels[ch])
                    return scratch.getWritePointer (idx);

            return buffer;
        }

        //==============================================================================
        AudioSampleBuffer scratch;
        AudioSampleBuffer mutableBuffer;

        HeapBlock<float*> channels;
        int pushIdx, popIdx;
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

    static Array<AUChannelInfo> getAUChannelInfo (PluginBusUtilities& busUtils)
    {
        Array<AUChannelInfo> channelInfo;

        AudioProcessor* juceFilter = &busUtils.processor;
        const AudioProcessor::AudioBusArrangement& arr = juceFilter->busArrangement;
        PluginBusUtilities::ScopedBusRestorer restorer (busUtils);

        const bool hasMainInputBus  = (busUtils.getNumEnabledBuses (true)  > 0);
        const bool hasMainOutputBus = (busUtils.getNumEnabledBuses (false) > 0);

        if ((! hasMainInputBus)  && (! hasMainOutputBus))
        {
            // midi effect plug-in: no audio
            AUChannelInfo info;
            info.inChannels = 0;
            info.outChannels = 0;

            channelInfo.add (info);
            return channelInfo;
        }
        else
        {
            const uint32_t maxNumChanToCheckFor = 9;

            uint32_t defaultInputs  = static_cast<uint32_t> (busUtils.getNumChannels (true,  0));
            uint32_t defaultOutputs = static_cast<uint32_t> (busUtils.getNumChannels (false, 0));

            uint32_t lastInputs  = defaultInputs;
            uint32_t lastOutputs = defaultOutputs;

            SortedSet<uint32_t> supportedChannels;

            // add the current configuration
            if (lastInputs != 0 || lastOutputs != 0)
                supportedChannels.add ((lastInputs << 16) | lastOutputs);

            for (uint32_t inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
            {
                const AudioChannelSet dfltInLayout = busUtils.getDefaultLayoutForChannelNumAndBus(true, 0, static_cast<int> (inChanNum));

                if (inChanNum != 0 && dfltInLayout.isDisabled())
                    continue;

                for (uint32_t outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
                {
                    const AudioChannelSet dfltOutLayout = busUtils.getDefaultLayoutForChannelNumAndBus(false, 0, static_cast<int> (outChanNum));
                    if (outChanNum != 0 && dfltOutLayout.isDisabled())
                        continue;

                    // get the number of channels again. This is only needed for some processors that change their configuration
                    // even when they indicate that setPreferredBusArrangement failed.
                    lastInputs  = hasMainInputBus  ? static_cast<uint32_t> (arr.inputBuses. getReference (0). channels.size()) : 0;
                    lastOutputs = hasMainOutputBus ? static_cast<uint32_t> (arr.outputBuses.getReference (0). channels.size()) : 0;

                    uint32_t channelConfiguration = (inChanNum << 16) | outChanNum;

                    // did we already try this configuration?
                    if (supportedChannels.contains (channelConfiguration)) continue;

                    if (lastInputs != inChanNum && (! dfltInLayout.isDisabled()))
                    {
                        if (! juceFilter->setPreferredBusArrangement (true, 0, dfltInLayout)) continue;

                        lastInputs = inChanNum;
                        lastOutputs = hasMainOutputBus ? static_cast<uint32_t> (arr.outputBuses.getReference (0). channels.size()) : 0;

                        supportedChannels.add ((lastInputs << 16) | lastOutputs);
                    }

                    if (lastOutputs != outChanNum && (! dfltOutLayout.isDisabled()))
                    {
                        if (! juceFilter->setPreferredBusArrangement (false, 0, dfltOutLayout)) continue;

                        lastInputs = hasMainInputBus ? static_cast<uint32_t> (arr.inputBuses.getReference (0).channels.size()) : 0;
                        lastOutputs = outChanNum;

                        supportedChannels.add ((lastInputs << 16) | lastOutputs);
                    }
                }
            }

            bool hasInOutMismatch = false;
            for (int i = 0; i < supportedChannels.size(); ++i)
            {
                const uint32_t numInputs  = (supportedChannels[i] >> 16) & 0xffff;
                const uint32_t numOutputs = (supportedChannels[i] >> 0)  & 0xffff;

                if (numInputs != numOutputs)
                {
                    hasInOutMismatch = true;
                    break;
                }
            }

            bool hasUnsupportedInput = ! hasMainOutputBus, hasUnsupportedOutput = ! hasMainInputBus;
            for (uint32_t inChanNum = hasMainInputBus ? 1 : 0; inChanNum <= (hasMainInputBus ? maxNumChanToCheckFor : 0); ++inChanNum)
            {
                uint32_t channelConfiguration = (inChanNum << 16) | (hasInOutMismatch ? defaultOutputs : inChanNum);
                if (! supportedChannels.contains (channelConfiguration))
                {
                    hasUnsupportedInput = true;
                    break;
                }
            }

            for (uint32_t outChanNum = hasMainOutputBus ? 1 : 0; outChanNum <= (hasMainOutputBus ? maxNumChanToCheckFor : 0); ++outChanNum)
            {
                uint32_t channelConfiguration = ((hasInOutMismatch ? defaultInputs : outChanNum) << 16) | outChanNum;
                if (! supportedChannels.contains (channelConfiguration))
                {
                    hasUnsupportedOutput = true;
                    break;
                }
            }

            for (int i = 0; i < supportedChannels.size(); ++i)
            {
                const int numInputs  = (supportedChannels[i] >> 16) & 0xffff;
                const int numOutputs = (supportedChannels[i] >> 0)  & 0xffff;

                AUChannelInfo info;

                // see here: https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/TheAudioUnit.html
                info.inChannels  = static_cast<SInt16> (hasMainInputBus  ? (hasUnsupportedInput  ? numInputs :  (hasInOutMismatch && (! hasUnsupportedOutput) ? -2 : -1)) : 0);
                info.outChannels = static_cast<SInt16> (hasMainOutputBus ? (hasUnsupportedOutput ? numOutputs : (hasInOutMismatch && (! hasUnsupportedInput)  ? -2 : -1)) : 0);

                if (info.inChannels == -2 && info.outChannels == -2)
                    info.inChannels = -1;

                int j;
                for (j = 0; j < channelInfo.size(); ++j)
                    if (channelInfo[j].inChannels == info.inChannels && channelInfo[j].outChannels == info.outChannels)
                        break;

                if (j >= channelInfo.size())
                    channelInfo.add (info);
            }
        }

        return channelInfo;
    }
};

AudioUnitHelpers::AUChannelStreamOrder AudioUnitHelpers::auChannelStreamOrder[] =
{
    {kAudioChannelLayoutTag_Mono,               {kAudioChannelLabel_Center, 0, 0, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_Stereo,             {kAudioChannelLabel_Left, kAudioChannelLabel_Right, 0, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_StereoHeadphones,   {kAudioChannelLabel_HeadphonesLeft, kAudioChannelLabel_HeadphonesRight, 0, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_Binaural,           {kAudioChannelLabel_Left, kAudioChannelLabel_Right, 0, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_Quadraphonic,       {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_Pentagonal,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, 0, 0, 0}},
    {kAudioChannelLayoutTag_Hexagonal,          {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, kAudioChannelLabel_CenterSurround, 0, 0}},
    {kAudioChannelLayoutTag_Octagonal,          {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, kAudioChannelLabel_CenterSurround, kAudioChannelLabel_LeftWide, kAudioChannelLabel_RightWide}},
    {kAudioChannelLayoutTag_Ambisonic_B_Format, {kAudioChannelLabel_Ambisonic_W, kAudioChannelLabel_Ambisonic_X, kAudioChannelLabel_Ambisonic_Y, kAudioChannelLabel_Ambisonic_Z, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_5_0_B,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, 0, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_5_1_A,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, kAudioChannelLabel_LFEScreen, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, 0, 0}},
    {kAudioChannelLayoutTag_AudioUnit_6_0,      {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, kAudioChannelLabel_CenterSurround, 0, 0}},
    {kAudioChannelLayoutTag_DTS_6_0_A,          {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_RearSurroundLeft, kAudioChannelLabel_RearSurroundRight, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_6_1_A,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, kAudioChannelLabel_LFEScreen, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_CenterSurround, 0}},
    {kAudioChannelLayoutTag_AudioUnit_7_0,      {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, kAudioChannelLabel_RearSurroundLeft, kAudioChannelLabel_RearSurroundRight, 0}},
    {kAudioChannelLayoutTag_MPEG_7_1_C,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, kAudioChannelLabel_LFEScreen, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_RearSurroundLeft, kAudioChannelLabel_RearSurroundRight}},
    {kAudioChannelLayoutTag_AudioUnit_7_0_Front,{kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_Center, kAudioChannelLabel_LeftCenter, kAudioChannelLabel_RightCenter, 0}},
    {kAudioChannelLayoutTag_AudioUnit_7_1_Front,{kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, kAudioChannelLabel_LFEScreen, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_LeftCenter, kAudioChannelLabel_RightCenter}},
    {kAudioChannelLayoutTag_MPEG_3_0_A,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_3_0_B,         {kAudioChannelLabel_Center, kAudioChannelLabel_Left, kAudioChannelLabel_Right, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_4_0_A,         {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_Center, kAudioChannelLabel_CenterSurround, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_MPEG_4_0_B,         {kAudioChannelLabel_Center, kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_CenterSurround, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_ITU_2_1,            {kAudioChannelLabel_Left, kAudioChannelLabel_Right, kAudioChannelLabel_CenterSurround, 0, 0, 0, 0, 0}},
    {kAudioChannelLayoutTag_EAC3_7_1_C,         {kAudioChannelLabel_Left, kAudioChannelLabel_Center, kAudioChannelLabel_Right, kAudioChannelLabel_LeftSurround, kAudioChannelLabel_RightSurround, kAudioChannelLabel_LFEScreen, kAudioChannelLabel_LeftSurroundDirect, kAudioChannelLabel_RightSurroundDirect}},
    {0,                                         {0,0,0,0,0,0,0,0}}
};
