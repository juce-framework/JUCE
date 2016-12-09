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

#ifndef JUCE_VSTCOMMON_H_INCLUDED
#define JUCE_VSTCOMMON_H_INCLUDED

//==============================================================================
struct SpeakerMappings  : private AudioChannelSet // (inheritance only to give easier access to items in the namespace)
{
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
        if      (arr == vstSpeakerConfigTypeEmpty)          return AudioChannelSet::disabled();
        else if (arr == vstSpeakerConfigTypeMono)           return AudioChannelSet::mono();
        else if (arr == vstSpeakerConfigTypeLR)             return AudioChannelSet::stereo();
        else if (arr == vstSpeakerConfigTypeLRC)            return AudioChannelSet::createLCR();
        else if (arr == vstSpeakerConfigTypeLRS)            return AudioChannelSet::createLRS();
        else if (arr == vstSpeakerConfigTypeLRCS)           return AudioChannelSet::createLCRS();
        else if (arr == vstSpeakerConfigTypeLRCLsRs)        return AudioChannelSet::create5point0();
        else if (arr == vstSpeakerConfigTypeLRCLfeLsRs)     return AudioChannelSet::create5point1();
        else if (arr == vstSpeakerConfigTypeLRCLsRsCs)      return AudioChannelSet::create6point0();
        else if (arr == vstSpeakerConfigTypeLRCLfeLsRsCs)   return AudioChannelSet::create6point1();
        else if (arr == vstSpeakerConfigTypeLRLsRsSlSr)     return AudioChannelSet::create6point0Music();
        else if (arr == vstSpeakerConfigTypeLRLfeLsRsSlSr)  return AudioChannelSet::create6point1Music();
        else if (arr == vstSpeakerConfigTypeLRCLsRsSlSr)    return AudioChannelSet::create7point0();
        else if (arr == vstSpeakerConfigTypeLRCLsRsLcRc)    return AudioChannelSet::create7point0SDDS();
        else if (arr == vstSpeakerConfigTypeLRCLfeLsRsSlSr) return AudioChannelSet::create7point1();
        else if (arr == vstSpeakerConfigTypeLRCLfeLsRsLcRc) return AudioChannelSet::create7point1SDDS();
        else if (arr == vstSpeakerConfigTypeLRLsRs)         return AudioChannelSet::quadraphonic();

        for (const Mapping* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
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

    static AudioChannelSet vstArrangementTypeToChannelSet (const VstSpeakerConfiguration& arr)
    {
        return vstArrangementTypeToChannelSet (arr.type, arr.numberOfChannels);
    }

    static int32 channelSetToVstArrangementType (AudioChannelSet channels)
    {
        if      (channels == AudioChannelSet::disabled())           return vstSpeakerConfigTypeEmpty;
        else if (channels == AudioChannelSet::mono())               return vstSpeakerConfigTypeMono;
        else if (channels == AudioChannelSet::stereo())             return vstSpeakerConfigTypeLR;
        else if (channels == AudioChannelSet::createLCR())          return vstSpeakerConfigTypeLRC;
        else if (channels == AudioChannelSet::createLRS())          return vstSpeakerConfigTypeLRS;
        else if (channels == AudioChannelSet::createLCRS())         return vstSpeakerConfigTypeLRCS;
        else if (channels == AudioChannelSet::create5point0())      return vstSpeakerConfigTypeLRCLsRs;
        else if (channels == AudioChannelSet::create5point1())      return vstSpeakerConfigTypeLRCLfeLsRs;
        else if (channels == AudioChannelSet::create6point0())      return vstSpeakerConfigTypeLRCLsRsCs;
        else if (channels == AudioChannelSet::create6point1())      return vstSpeakerConfigTypeLRCLfeLsRsCs;
        else if (channels == AudioChannelSet::create6point0Music()) return vstSpeakerConfigTypeLRLsRsSlSr;
        else if (channels == AudioChannelSet::create6point1Music()) return vstSpeakerConfigTypeLRLfeLsRsSlSr;
        else if (channels == AudioChannelSet::create7point0())      return vstSpeakerConfigTypeLRCLsRsSlSr;
        else if (channels == AudioChannelSet::create7point0SDDS())  return vstSpeakerConfigTypeLRCLsRsLcRc;
        else if (channels == AudioChannelSet::create7point1())      return vstSpeakerConfigTypeLRCLfeLsRsSlSr;
        else if (channels == AudioChannelSet::create7point1SDDS())  return vstSpeakerConfigTypeLRCLfeLsRsLcRc;
        else if (channels == AudioChannelSet::quadraphonic())       return vstSpeakerConfigTypeLRLsRs;

        Array<AudioChannelSet::ChannelType> chans (channels.getChannelTypes());

        if (channels == AudioChannelSet::disabled())
            return vstSpeakerConfigTypeEmpty;

        for (const Mapping* m = getMappings(); m->vst2 != vstSpeakerConfigTypeEmpty; ++m)
            if (m->matches (chans))
                return m->vst2;

        return vstSpeakerConfigTypeUser;
    }

    static void channelSetToVstArrangement (const AudioChannelSet& channels, VstSpeakerConfiguration& result)
    {
        result.type = channelSetToVstArrangementType (channels);
        result.numberOfChannels = channels.size();

        for (int i = 0; i < result.numberOfChannels; ++i)
        {
            VstIndividualSpeakerInfo& speaker = result.speakers[i];

            zeromem (&speaker, sizeof (VstIndividualSpeakerInfo));
            speaker.type = getSpeakerType (channels.getTypeOfChannel (i));
        }
    }

    static const Mapping* getMappings() noexcept
    {
        static const Mapping mappings[] =
        {
            { vstSpeakerConfigTypeMono,           { centre, unknown } },
            { vstSpeakerConfigTypeLR,             { left, right, unknown } },
            { vstSpeakerConfigTypeLsRs,           { leftSurround, rightSurround, unknown } },
            { vstSpeakerConfigTypeLcRc,           { leftCentre, rightCentre, unknown } },
            { vstSpeakerConfigTypeSlSr,           { leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeCLfe,           { centre, LFE, unknown } },
            { vstSpeakerConfigTypeLRC,            { left, right, centre, unknown } },
            { vstSpeakerConfigTypeLRS,            { left, right, surround, unknown } },
            { vstSpeakerConfigTypeLRCLfe,         { left, right, centre, LFE, unknown } },
            { vstSpeakerConfigTypeLRLfeS,         { left, right, LFE, surround, unknown } },
            { vstSpeakerConfigTypeLRCS,           { left, right, centre, surround, unknown } },
            { vstSpeakerConfigTypeLRLsRs,         { left, right, leftSurround, rightSurround, unknown } },
            { vstSpeakerConfigTypeLRCLfeS,        { left, right, centre, LFE, surround, unknown } },
            { vstSpeakerConfigTypeLRLfeLsRs,      { left, right, LFE, leftSurround, rightSurround, unknown } },
            { vstSpeakerConfigTypeLRCLsRs,        { left, right, centre, leftSurround, rightSurround, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRs,     { left, right, centre, LFE, leftSurround, rightSurround, unknown } },
            { vstSpeakerConfigTypeLRCLsRsCs,      { left, right, centre, leftSurround, rightSurround, surround, unknown } },
            { vstSpeakerConfigTypeLRLsRsSlSr,     { left, right, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsCs,   { left, right, centre, LFE, leftSurround, rightSurround, surround, unknown } },
            { vstSpeakerConfigTypeLRLfeLsRsSlSr,  { left, right, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLsRsLcRc,    { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
            { vstSpeakerConfigTypeLRCLsRsSlSr,    { left, right, centre, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsLcRc, { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsSlSr, { left, right, centre, LFE, leftSurround, rightSurround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLsRsLcRcCs,  { left, right, centre, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
            { vstSpeakerConfigTypeLRCLsRsCsSlSr,  { left, right, centre, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsLcRcCs,   { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontRight, surround, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsCsSlSr,   { left, right, centre, LFE, leftSurround, rightSurround, surround, leftSurroundRear, rightSurroundRear, unknown } },
            { vstSpeakerConfigTypeLRCLfeLsRsTflTfcTfrTrlTrrLfe2,            { left, right, centre, LFE, leftSurround, rightSurround, topFrontLeft, topFrontCentre, topFrontRight, topRearLeft, topRearRight, LFE2, unknown } },
            { vstSpeakerConfigTypeEmpty,          { unknown } }
        };

        return mappings;
    }

    static inline int32 getSpeakerType (AudioChannelSet::ChannelType type) noexcept
    {
        switch (type)
        {
            case AudioChannelSet::left:              return vstIndividualSpeakerTypeLeft;
            case AudioChannelSet::right:             return vstIndividualSpeakerTypeRight;
            case AudioChannelSet::centre:            return vstIndividualSpeakerTypeCentre;
            case AudioChannelSet::LFE:           return vstIndividualSpeakerTypeLFE;
            case AudioChannelSet::leftSurround:      return vstIndividualSpeakerTypeLeftSurround;
            case AudioChannelSet::rightSurround:     return vstIndividualSpeakerTypeRightSurround;
            case AudioChannelSet::leftCentre:        return vstIndividualSpeakerTypeLeftCentre;
            case AudioChannelSet::rightCentre:       return vstIndividualSpeakerTypeRightCentre;
            case AudioChannelSet::surround:          return vstIndividualSpeakerTypeSurround;
            case AudioChannelSet::leftSurroundRear:  return vstIndividualSpeakerTypeLeftRearSurround;
            case AudioChannelSet::rightSurroundRear: return vstIndividualSpeakerTypeRightRearSurround;
            case AudioChannelSet::topMiddle:         return vstIndividualSpeakerTypeTopMiddle;
            case AudioChannelSet::topFrontLeft:      return vstIndividualSpeakerTypeTopFrontLeft;
            case AudioChannelSet::topFrontCentre:    return vstIndividualSpeakerTypeTopFrontCentre;
            case AudioChannelSet::topFrontRight:     return vstIndividualSpeakerTypeTopFrontRight;
            case AudioChannelSet::topRearLeft:       return vstIndividualSpeakerTypeTopRearLeft;
            case AudioChannelSet::topRearCentre:     return vstIndividualSpeakerTypeTopRearCentre;
            case AudioChannelSet::topRearRight:      return vstIndividualSpeakerTypeTopRearRight;
            case AudioChannelSet::LFE2:          return vstIndividualSpeakerTypeLFE2;
            default: break;
        }

        return 0;
    }

    static inline AudioChannelSet::ChannelType getChannelType (int32 type) noexcept
    {
        switch (type)
        {
            case vstIndividualSpeakerTypeLeft:                 return AudioChannelSet::left;
            case vstIndividualSpeakerTypeRight:                return AudioChannelSet::right;
            case vstIndividualSpeakerTypeCentre:               return AudioChannelSet::centre;
            case vstIndividualSpeakerTypeLFE:              return AudioChannelSet::LFE;
            case vstIndividualSpeakerTypeLeftSurround:         return AudioChannelSet::leftSurround;
            case vstIndividualSpeakerTypeRightSurround:        return AudioChannelSet::rightSurround;
            case vstIndividualSpeakerTypeLeftCentre:           return AudioChannelSet::leftCentre;
            case vstIndividualSpeakerTypeRightCentre:          return AudioChannelSet::rightCentre;
            case vstIndividualSpeakerTypeSurround:             return AudioChannelSet::surround;
            case vstIndividualSpeakerTypeLeftRearSurround:     return AudioChannelSet::leftSurroundRear;
            case vstIndividualSpeakerTypeRightRearSurround:    return AudioChannelSet::rightSurroundRear;
            case vstIndividualSpeakerTypeTopMiddle:            return AudioChannelSet::topMiddle;
            case vstIndividualSpeakerTypeTopFrontLeft:         return AudioChannelSet::topFrontLeft;
            case vstIndividualSpeakerTypeTopFrontCentre:       return AudioChannelSet::topFrontCentre;
            case vstIndividualSpeakerTypeTopFrontRight:        return AudioChannelSet::topFrontRight;
            case vstIndividualSpeakerTypeTopRearLeft:          return AudioChannelSet::topRearLeft;
            case vstIndividualSpeakerTypeTopRearCentre:        return AudioChannelSet::topRearCentre;
            case vstIndividualSpeakerTypeTopRearRight:         return AudioChannelSet::topRearRight;
            case vstIndividualSpeakerTypeLFE2:             return AudioChannelSet::LFE2;
            default: break;
        }

        return AudioChannelSet::unknown;
    }
};

#endif   // JUCE_VSTCOMMON_H_INCLUDED
