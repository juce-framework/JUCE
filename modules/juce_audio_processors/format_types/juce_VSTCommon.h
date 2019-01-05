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

//==============================================================================
#if JUCE_BIG_ENDIAN
 #define JUCE_MULTICHAR_CONSTANT(a, b, c, d) (a | (((uint32) b) << 8) | (((uint32) c) << 16) | (((uint32) d) << 24))
#else
 #define JUCE_MULTICHAR_CONSTANT(a, b, c, d) (d | (((uint32) c) << 8) | (((uint32) b) << 16) | (((uint32) a) << 24))
#endif

//==============================================================================
/** Structure for VST speaker mappings

    @tags{Audio}
*/
struct SpeakerMappings  : private AudioChannelSet // (inheritance only to give easier access to items in the namespace)
{
    /** Structure describing a mapping */
    struct Mapping
    {
        int32 vst2;
        ChannelType channels[13];

        bool matches (const Array<ChannelType>& chans) const noexcept
        {
            const int n = sizeof (channels) / sizeof (ChannelType);

            for (int i = 0; i < n; ++i)
            {
                if (channels[i] == unknown)  return (i == chans.size());
                if (i == chans.size())       return (channels[i] == unknown);

                if (channels[i] != chans.getUnchecked(i))
                    return false;
            }

            return true;
        }
    };

    static AudioChannelSet vstArrangementTypeToChannelSet (int32 arr, int fallbackNumChannels)
    {
        if      (arr == Vst2::kSpeakerArrEmpty)        return AudioChannelSet::disabled();
        else if (arr == Vst2::kSpeakerArrMono)         return AudioChannelSet::mono();
        else if (arr == Vst2::kSpeakerArrStereo)       return AudioChannelSet::stereo();
        else if (arr == Vst2::kSpeakerArr30Cine)       return AudioChannelSet::createLCR();
        else if (arr == Vst2::kSpeakerArr30Music)      return AudioChannelSet::createLRS();
        else if (arr == Vst2::kSpeakerArr40Cine)       return AudioChannelSet::createLCRS();
        else if (arr == Vst2::kSpeakerArr50)           return AudioChannelSet::create5point0();
        else if (arr == Vst2::kSpeakerArr51)           return AudioChannelSet::create5point1();
        else if (arr == Vst2::kSpeakerArr60Cine)       return AudioChannelSet::create6point0();
        else if (arr == Vst2::kSpeakerArr61Cine)       return AudioChannelSet::create6point1();
        else if (arr == Vst2::kSpeakerArr60Music)      return AudioChannelSet::create6point0Music();
        else if (arr == Vst2::kSpeakerArr61Music)      return AudioChannelSet::create6point1Music();
        else if (arr == Vst2::kSpeakerArr70Music)      return AudioChannelSet::create7point0();
        else if (arr == Vst2::kSpeakerArr70Cine)       return AudioChannelSet::create7point0SDDS();
        else if (arr == Vst2::kSpeakerArr71Music)      return AudioChannelSet::create7point1();
        else if (arr == Vst2::kSpeakerArr71Cine)       return AudioChannelSet::create7point1SDDS();
        else if (arr == Vst2::kSpeakerArr40Music)      return AudioChannelSet::quadraphonic();

        for (const Mapping* m = getMappings(); m->vst2 != Vst2::kSpeakerArrEmpty; ++m)
        {
            if (m->vst2 == arr)
            {
                AudioChannelSet s;

                for (int i = 0; m->channels[i] != 0; ++i)
                    s.addChannel (m->channels[i]);

                return s;
            }
        }

        return AudioChannelSet::discreteChannels (fallbackNumChannels);
    }

    static AudioChannelSet vstArrangementTypeToChannelSet (const Vst2::VstSpeakerArrangement& arr)
    {
        return vstArrangementTypeToChannelSet (arr.type, arr.numChannels);
    }

    static int32 channelSetToVstArrangementType (AudioChannelSet channels)
    {
        if      (channels == AudioChannelSet::disabled())           return Vst2::kSpeakerArrEmpty;
        else if (channels == AudioChannelSet::mono())               return Vst2::kSpeakerArrMono;
        else if (channels == AudioChannelSet::stereo())             return Vst2::kSpeakerArrStereo;
        else if (channels == AudioChannelSet::createLCR())          return Vst2::kSpeakerArr30Cine;
        else if (channels == AudioChannelSet::createLRS())          return Vst2::kSpeakerArr30Music;
        else if (channels == AudioChannelSet::createLCRS())         return Vst2::kSpeakerArr40Cine;
        else if (channels == AudioChannelSet::create5point0())      return Vst2::kSpeakerArr50;
        else if (channels == AudioChannelSet::create5point1())      return Vst2::kSpeakerArr51;
        else if (channels == AudioChannelSet::create6point0())      return Vst2::kSpeakerArr60Cine;
        else if (channels == AudioChannelSet::create6point1())      return Vst2::kSpeakerArr61Cine;
        else if (channels == AudioChannelSet::create6point0Music()) return Vst2::kSpeakerArr60Music;
        else if (channels == AudioChannelSet::create6point1Music()) return Vst2::kSpeakerArr61Music;
        else if (channels == AudioChannelSet::create7point0())      return Vst2::kSpeakerArr70Music;
        else if (channels == AudioChannelSet::create7point0SDDS())  return Vst2::kSpeakerArr70Cine;
        else if (channels == AudioChannelSet::create7point1())      return Vst2::kSpeakerArr71Music;
        else if (channels == AudioChannelSet::create7point1SDDS())  return Vst2::kSpeakerArr71Cine;
        else if (channels == AudioChannelSet::quadraphonic())       return Vst2::kSpeakerArr40Music;

        Array<AudioChannelSet::ChannelType> chans (channels.getChannelTypes());

        if (channels == AudioChannelSet::disabled())
            return Vst2::kSpeakerArrEmpty;

        for (const Mapping* m = getMappings(); m->vst2 != Vst2::kSpeakerArrEmpty; ++m)
            if (m->matches (chans))
                return m->vst2;

        return Vst2::kSpeakerArrUserDefined;
    }

    /** Class to hold a speaker configuration */
    class VstSpeakerConfigurationHolder
    {
    public:
        VstSpeakerConfigurationHolder()                                            { clear(); }
        VstSpeakerConfigurationHolder (const Vst2::VstSpeakerArrangement& vstConfig)   { operator= (vstConfig); }
        VstSpeakerConfigurationHolder (const VstSpeakerConfigurationHolder& other) { operator= (other.get()); }
        VstSpeakerConfigurationHolder (VstSpeakerConfigurationHolder&& other)
            : storage (std::move (other.storage)) { other.clear(); }

        VstSpeakerConfigurationHolder (const AudioChannelSet& channels)
        {
            auto numberOfChannels = channels.size();
            Vst2::VstSpeakerArrangement& dst = *allocate (numberOfChannels);

            dst.type = channelSetToVstArrangementType (channels);
            dst.numChannels = numberOfChannels;

            for (int i = 0; i < dst.numChannels; ++i)
            {
                Vst2::VstSpeakerProperties& speaker = dst.speakers[i];

                zeromem (&speaker, sizeof (Vst2::VstSpeakerProperties));
                speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
            }
        }

        VstSpeakerConfigurationHolder& operator= (const VstSpeakerConfigurationHolder& vstConfig) { return operator=(vstConfig.get()); }
        VstSpeakerConfigurationHolder& operator= (const Vst2::VstSpeakerArrangement& vstConfig)
        {
            Vst2::VstSpeakerArrangement& dst = *allocate (vstConfig.numChannels);

            dst.type             = vstConfig.type;
            dst.numChannels      = vstConfig.numChannels;

            for (int i = 0; i < dst.numChannels; ++i)
                dst.speakers[i] = vstConfig.speakers[i];

            return *this;
        }

        VstSpeakerConfigurationHolder& operator= (VstSpeakerConfigurationHolder && vstConfig)
        {
            storage = std::move (vstConfig.storage);
            vstConfig.clear();

            return *this;
        }

        const Vst2::VstSpeakerArrangement& get() const { return *storage.get(); }

    private:
        JUCE_LEAK_DETECTOR (VstSpeakerConfigurationHolder)

        HeapBlock<Vst2::VstSpeakerArrangement> storage;

        Vst2::VstSpeakerArrangement* allocate (int numChannels)
        {
            auto arrangementSize = sizeof (Vst2::VstSpeakerArrangement)
                                     + sizeof (Vst2::VstSpeakerProperties) * static_cast<size_t> (jmax (8, numChannels) - 8);

            storage.malloc (1, arrangementSize);
            return storage.get();
        }

        void clear()
        {
            Vst2::VstSpeakerArrangement& dst = *allocate (0);

            dst.type = Vst2::kSpeakerArrEmpty;
            dst.numChannels = 0;
        }
    };

    static const Mapping* getMappings() noexcept
    {
        static const Mapping mappings[] =
        {
            { Vst2::kSpeakerArrMono,           { centre, unknown } },
            { Vst2::kSpeakerArrStereo,         { left, right, unknown } },
            { Vst2::kSpeakerArrStereoSurround, { leftSurround, rightSurround, unknown } },
            { Vst2::kSpeakerArrStereoCenter,   { leftCentre, rightCentre, unknown } },
            { Vst2::kSpeakerArrStereoSide,     { leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArrStereoCLfe,     { centre, LFE, unknown } },
            { Vst2::kSpeakerArr30Cine,         { left, right, centre, unknown } },
            { Vst2::kSpeakerArr30Music,        { left, right, surround, unknown } },
            { Vst2::kSpeakerArr31Cine,         { left, right, centre, LFE, unknown } },
            { Vst2::kSpeakerArr31Music,        { left, right, LFE, surround, unknown } },
            { Vst2::kSpeakerArr40Cine,         { left, right, centre, surround, unknown } },
            { Vst2::kSpeakerArr40Music,        { left, right, leftSurround, rightSurround, unknown } },
            { Vst2::kSpeakerArr41Cine,         { left, right, centre, LFE, surround, unknown } },
            { Vst2::kSpeakerArr41Music,        { left, right, LFE, leftSurround, rightSurround, unknown } },
            { Vst2::kSpeakerArr50,             { left, right, centre, leftSurround, rightSurround, unknown } },
            { Vst2::kSpeakerArr51,             { left, right, centre, LFE, leftSurround, rightSurround, unknown } },
            { Vst2::kSpeakerArr60Cine,         { left, right, centre, leftSurround, rightSurround, surround, unknown } },
            { Vst2::kSpeakerArr60Music,        { left, right, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr61Cine,         { left, right, centre, LFE, leftSurround, rightSurround, surround, unknown } },
            { Vst2::kSpeakerArr61Music,        { left, right, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr70Cine,         { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
            { Vst2::kSpeakerArr70Music,        { left, right, centre, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr71Cine,         { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
            { Vst2::kSpeakerArr71Music,        { left, right, centre, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr80Cine,         { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
            { Vst2::kSpeakerArr80Music,        { left, right, centre, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr81Cine,  { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
            { Vst2::kSpeakerArr81Music, { left, right, centre, LFE, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr102,     { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, LFE2, unknown } },
            { Vst2::kSpeakerArrEmpty,          { unknown } }
        };

        return mappings;
    }

    static inline int32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
    {
        switch (type)
        {
            case AudioChannelSet::left:              return Vst2::kSpeakerL;
            case AudioChannelSet::right:             return Vst2::kSpeakerR;
            case AudioChannelSet::centre:            return Vst2::kSpeakerC;
            case AudioChannelSet::LFE:               return Vst2::kSpeakerLfe;
            case AudioChannelSet::leftSurround:      return Vst2::kSpeakerLs;
            case AudioChannelSet::rightSurround:     return Vst2::kSpeakerRs;
            case AudioChannelSet::leftCentre:        return Vst2::kSpeakerLc;
            case AudioChannelSet::rightCentre:       return Vst2::kSpeakerRc;
            case AudioChannelSet::surround:          return Vst2::kSpeakerS;
            case AudioChannelSet::leftSurroundRear:  return Vst2::kSpeakerSl;
            case AudioChannelSet::rightSurroundRear: return Vst2::kSpeakerSr;
            case AudioChannelSet::topMiddle:         return Vst2::kSpeakerTm;
            case AudioChannelSet::topFrontLeft:      return Vst2::kSpeakerTfl;
            case AudioChannelSet::topFrontCentre:    return Vst2::kSpeakerTfc;
            case AudioChannelSet::topFrontRight:     return Vst2::kSpeakerTfr;
            case AudioChannelSet::topRearLeft:       return Vst2::kSpeakerTrl;
            case AudioChannelSet::topRearCentre:     return Vst2::kSpeakerTrc;
            case AudioChannelSet::topRearRight:      return Vst2::kSpeakerTrr;
            case AudioChannelSet::LFE2:              return Vst2::kSpeakerLfe2;
            default: break;
        }

        return 0;
    }

    static inline AudioChannelSet::ChannelType getChannelType (int32 type) noexcept
    {
        switch (type)
        {
            case Vst2::kSpeakerL:      return AudioChannelSet::left;
            case Vst2::kSpeakerR:      return AudioChannelSet::right;
            case Vst2::kSpeakerC:      return AudioChannelSet::centre;
            case Vst2::kSpeakerLfe:    return AudioChannelSet::LFE;
            case Vst2::kSpeakerLs:     return AudioChannelSet::leftSurround;
            case Vst2::kSpeakerRs:     return AudioChannelSet::rightSurround;
            case Vst2::kSpeakerLc:     return AudioChannelSet::leftCentre;
            case Vst2::kSpeakerRc:     return AudioChannelSet::rightCentre;
            case Vst2::kSpeakerS:      return AudioChannelSet::surround;
            case Vst2::kSpeakerSl:     return AudioChannelSet::leftSurroundRear;
            case Vst2::kSpeakerSr:     return AudioChannelSet::rightSurroundRear;
            case Vst2::kSpeakerTm:     return AudioChannelSet::topMiddle;
            case Vst2::kSpeakerTfl:    return AudioChannelSet::topFrontLeft;
            case Vst2::kSpeakerTfc:    return AudioChannelSet::topFrontCentre;
            case Vst2::kSpeakerTfr:    return AudioChannelSet::topFrontRight;
            case Vst2::kSpeakerTrl:    return AudioChannelSet::topRearLeft;
            case Vst2::kSpeakerTrc:    return AudioChannelSet::topRearCentre;
            case Vst2::kSpeakerTrr:    return AudioChannelSet::topRearRight;
            case Vst2::kSpeakerLfe2:   return AudioChannelSet::LFE2;
            default: break;
        }

        return AudioChannelSet::unknown;
    }
};

} // namespace juce
