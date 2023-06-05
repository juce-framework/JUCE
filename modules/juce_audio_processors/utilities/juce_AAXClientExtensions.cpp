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

int32 AAXClientExtensions::getPluginIDForMainBusConfig (const AudioChannelSet& mainInputLayout,
                                                        const AudioChannelSet& mainOutputLayout,
                                                        bool idForAudioSuite) const
{
    int uniqueFormatId = 0;

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir == 0);
        auto& set = (isInput ? mainInputLayout : mainOutputLayout);
        int aaxFormatIndex = 0;

        const AudioChannelSet sets[]
        {
            AudioChannelSet::disabled(),
            AudioChannelSet::mono(),
            AudioChannelSet::stereo(),
            AudioChannelSet::createLCR(),
            AudioChannelSet::createLCRS(),
            AudioChannelSet::quadraphonic(),
            AudioChannelSet::create5point0(),
            AudioChannelSet::create5point1(),
            AudioChannelSet::create6point0(),
            AudioChannelSet::create6point1(),
            AudioChannelSet::create7point0(),
            AudioChannelSet::create7point1(),
            AudioChannelSet::create7point0SDDS(),
            AudioChannelSet::create7point1SDDS(),
            AudioChannelSet::create7point0point2(),
            AudioChannelSet::create7point1point2(),
            AudioChannelSet::ambisonic (1),
            AudioChannelSet::ambisonic (2),
            AudioChannelSet::ambisonic (3),
            AudioChannelSet::create5point0point2(),
            AudioChannelSet::create5point1point2(),
            AudioChannelSet::create5point0point4(),
            AudioChannelSet::create5point1point4(),
            AudioChannelSet::create7point0point4(),
            AudioChannelSet::create7point1point4(),
            AudioChannelSet::create7point0point6(),
            AudioChannelSet::create7point1point6(),
            AudioChannelSet::create9point0point4(),
            AudioChannelSet::create9point1point4(),
            AudioChannelSet::create9point0point6(),
            AudioChannelSet::create9point1point6(),
            AudioChannelSet::ambisonic (4),
            AudioChannelSet::ambisonic (5),
            AudioChannelSet::ambisonic (6),
            AudioChannelSet::ambisonic (7)
        };

        const auto index = (int) std::distance (std::begin (sets), std::find (std::begin (sets), std::end (sets), set));

        if (index != numElementsInArray (sets))
            aaxFormatIndex = index;
        else
            jassertfalse;

        uniqueFormatId = (uniqueFormatId << 8) | aaxFormatIndex;
    }

    return (idForAudioSuite ? 0x6a796161 /* 'jyaa' */ : 0x6a636161 /* 'jcaa' */) + uniqueFormatId;
}

String AAXClientExtensions::getPageFileName() const
{
   #ifdef JucePlugin_AAXPageTableFile
    #warning "JucePlugin_AAXPageTableFile is deprecated, instead implement AAXClientExtensions::getPageFileName()"
    return JucePlugin_AAXPageTableFile;
   #else
    return {};
   #endif
}

} // namespace juce
