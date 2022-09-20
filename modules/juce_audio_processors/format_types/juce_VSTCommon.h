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

namespace juce
{

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
            auto n = static_cast<int> (sizeof (channels) / sizeof (ChannelType));

            for (int i = 0; i < n; ++i)
            {
                if (channels[i] == unknown)  return (i == chans.size());
                if (i == chans.size())       return (channels[i] == unknown);

                if (channels[i] != chans.getUnchecked (i))
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

        if (channels == AudioChannelSet::disabled())
            return Vst2::kSpeakerArrEmpty;

        auto chans = channels.getChannelTypes();

        for (auto* m = getMappings(); m->vst2 != Vst2::kSpeakerArrEmpty; ++m)
            if (m->matches (chans))
                return m->vst2;

        return Vst2::kSpeakerArrUserDefined;
    }

    static void channelSetToVstArrangement (const AudioChannelSet& channels, Vst2::VstSpeakerArrangement& result)
    {
        result.type = channelSetToVstArrangementType (channels);
        result.numChannels = channels.size();

        for (int i = 0; i < result.numChannels; ++i)
        {
            auto& speaker = result.speakers[i];

            zeromem (&speaker, sizeof (Vst2::VstSpeakerProperties));
            speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
        }
    }

    /** Class to hold a speaker configuration */
    class VstSpeakerConfigurationHolder
    {
    public:
        VstSpeakerConfigurationHolder()
        {
            clear();
        }

        VstSpeakerConfigurationHolder (const Vst2::VstSpeakerArrangement& vstConfig)
        {
            operator= (vstConfig);
        }

        VstSpeakerConfigurationHolder (const VstSpeakerConfigurationHolder& other)
        {
            operator= (other.get());
        }

        VstSpeakerConfigurationHolder (VstSpeakerConfigurationHolder&& other)
            : storage (std::move (other.storage))
        {
            other.clear();
        }

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
            auto arrangementSize = (size_t) (jmax (8, numChannels) - 8) * sizeof (Vst2::VstSpeakerProperties)
                                    + sizeof (Vst2::VstSpeakerArrangement);

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
            { Vst2::kSpeakerArr81Cine,         { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
            { Vst2::kSpeakerArr81Music,        { left, right, centre, LFE, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
            { Vst2::kSpeakerArr102,            { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, LFE2, unknown } },
            { Vst2::kSpeakerArrEmpty,          { unknown } }
        };

        return mappings;
    }

    static int32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
    {
        static const std::map<AudioChannelSet::ChannelType, int32> speakerTypeMap =
        {
            { AudioChannelSet::left,              Vst2::kSpeakerL },
            { AudioChannelSet::right,             Vst2::kSpeakerR },
            { AudioChannelSet::centre,            Vst2::kSpeakerC },
            { AudioChannelSet::LFE,               Vst2::kSpeakerLfe },
            { AudioChannelSet::leftSurround,      Vst2::kSpeakerLs },
            { AudioChannelSet::rightSurround,     Vst2::kSpeakerRs },
            { AudioChannelSet::leftCentre,        Vst2::kSpeakerLc },
            { AudioChannelSet::rightCentre,       Vst2::kSpeakerRc },
            { AudioChannelSet::surround,          Vst2::kSpeakerS },
            { AudioChannelSet::leftSurroundRear,  Vst2::kSpeakerSl },
            { AudioChannelSet::rightSurroundRear, Vst2::kSpeakerSr },
            { AudioChannelSet::topMiddle,         Vst2::kSpeakerTm },
            { AudioChannelSet::topFrontLeft,      Vst2::kSpeakerTfl },
            { AudioChannelSet::topFrontCentre,    Vst2::kSpeakerTfc },
            { AudioChannelSet::topFrontRight,     Vst2::kSpeakerTfr },
            { AudioChannelSet::topRearLeft,       Vst2::kSpeakerTrl },
            { AudioChannelSet::topRearCentre,     Vst2::kSpeakerTrc },
            { AudioChannelSet::topRearRight,      Vst2::kSpeakerTrr },
            { AudioChannelSet::LFE2,              Vst2::kSpeakerLfe2 }
        };

        if (speakerTypeMap.find (type) == speakerTypeMap.end())
            return 0;

        return speakerTypeMap.at (type);
    }
};

} // namespace juce
