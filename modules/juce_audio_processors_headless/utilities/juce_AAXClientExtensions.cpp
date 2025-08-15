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

namespace juce
{

class AAXPluginId
{
public:
    static std::optional<AAXPluginId> create (std::array<char, 4> letters)
    {
        std::array<size_t, 4> indices{};

        for (size_t i = 0; i < letters.size(); ++i)
        {
            if (const auto index = findIndexOfChar (letters[i]))
                indices[i] = *index;
            else
                return std::nullopt;
        }

        return AAXPluginId { std::move (indices) };
    }

    std::optional<AAXPluginId> withIncrementedLetter (size_t index, size_t increment) const
    {
        if (indices.size() <= index)
            return std::nullopt;

        auto copy = *this;
        copy.indices[index] += increment;

        if ((size_t) std::size (validChars) <= copy.indices[index])
            return std::nullopt;

        return copy;
    }

    int32 getPluginId() const
    {
        int32 result = 0;

        for (size_t i = 0; i < indices.size(); ++i)
            result |= static_cast<int32> (validChars[indices[i]]) << (8 * (indices.size() - 1 - i));

        return result;
    }

    static std::optional<size_t> findIndexOfChar (char c)
    {
        const auto ptr = std::find (std::begin (validChars), std::end (validChars), c);
        return ptr != std::end (validChars) ? std::make_optional (std::distance (std::begin (validChars), ptr))
                                            : std::nullopt;
    }

private:
    static inline constexpr char validChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    explicit AAXPluginId (std::array<size_t, 4> indicesIn)
        : indices (std::move (indicesIn))
    {}

    std::array<size_t, 4> indices;
};

static const AudioChannelSet channelSets[] { AudioChannelSet::disabled(),
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
                                             AudioChannelSet::ambisonic (7) };

int32 AAXClientExtensions::getPluginIDForMainBusConfig (const AudioChannelSet& mainInputLayout,
                                                        const AudioChannelSet& mainOutputLayout,
                                                        bool idForAudioSuite) const
{
    auto pluginId = [&]
    {
        auto opt = idForAudioSuite ? AAXPluginId::create ({ 'j', 'y', 'a', 'a' })
                                   : AAXPluginId::create ({ 'j', 'c', 'a', 'a' });
        jassert (opt.has_value());
        return *opt;
    }();

    for (const auto& [channelSet, indexToModify] : { std::tuple (&mainInputLayout, (size_t) 2),
                                                     std::tuple (&mainOutputLayout, (size_t) 3) })
    {
        const auto increment = (size_t) std::distance (std::begin (channelSets),
                                                       std::find (std::begin (channelSets),
                                                                  std::end (channelSets),
                                                                  *channelSet));

        if (auto modifiedPluginIdOpt = pluginId.withIncrementedLetter (indexToModify, increment);
            increment < (size_t) std::size (channelSets) && modifiedPluginIdOpt.has_value())
        {
            pluginId = *modifiedPluginIdOpt;
        }
        else
        {
            jassertfalse;
        }
    }

    return pluginId.getPluginId();
}

String AAXClientExtensions::getPageFileName() const
{
   #ifdef JucePlugin_AAXPageTableFile
    JUCE_COMPILER_WARNING ("JucePlugin_AAXPageTableFile is deprecated, instead implement AAXClientExtensions::getPageFileName()")
    return JucePlugin_AAXPageTableFile;
   #else
    return {};
   #endif
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

static std::array<char, 4> toCharArray (int32 identifier)
{
    std::array<char, 4> result;

    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char> ((identifier >> (i * 8)) & 0xff);

    return result;
}

static bool isValidAAXPluginId (int32 pluginId)
{
    const auto chars = toCharArray (pluginId);

    return std::all_of (std::begin (chars),
                        std::end (chars),
                        [] (const auto& c)
                        {
                            return AAXPluginId::findIndexOfChar (c).has_value();
                        });
}

static int32 getPluginIDForMainBusConfigJuce705 (const AudioChannelSet& mainInputLayout,
                                                 const AudioChannelSet& mainOutputLayout,
                                                 bool idForAudioSuite)
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

        if (index != (int) std::size (sets))
            aaxFormatIndex = index;
        else
            jassertfalse;

        uniqueFormatId = (uniqueFormatId << 8) | aaxFormatIndex;
    }

    return (idForAudioSuite ? 0x6a796161 /* 'jyaa' */ : 0x6a636161 /* 'jcaa' */) + uniqueFormatId;
}

class AAXClientExtensionsTests final : public UnitTest
{
public:
    AAXClientExtensionsTests()
        : UnitTest ("AAXClientExtensionsTests", UnitTestCategories::audioProcessors)
    {}

    void runTest() override
    {
        AAXClientExtensions extensions;

        beginTest ("Previously valid PluginIds should be unchanged");
        {
            for (const auto& input : channelSets)
                for (const auto& output : channelSets)
                    for (const auto idForAudioSuite : { false, true })
                        if (const auto oldId = getPluginIDForMainBusConfigJuce705 (input, output, idForAudioSuite); isValidAAXPluginId (oldId))
                            expect (extensions.getPluginIDForMainBusConfig (input, output, idForAudioSuite) == oldId);
        }

        beginTest ("Valid, unique PluginIds should be generated for all configurations");
        {
            std::set<int32> pluginIds;

            for (const auto& input : channelSets)
                for (const auto& output : channelSets)
                    for (const auto idForAudioSuite : { false, true })
                        pluginIds.insert (extensions.getPluginIDForMainBusConfig (input, output, idForAudioSuite));

            for (auto identifier : pluginIds)
                expect (isValidAAXPluginId (identifier));

            expect (pluginIds.size() == square (std::size (channelSets)) * 2);
        }
    }
};

static AAXClientExtensionsTests aaxClientExtensionsTests;

#endif

} // namespace juce
