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

#include "juce_LV2Common.h"

namespace juce
{

class LV2PluginFormatTests : public UnitTest
{
public:
    LV2PluginFormatTests()
        : UnitTest ("LV2 Hosting", UnitTestCategories::audioProcessors)
    {
    }

    void runTest() override
    {
        using namespace lv2_shared;

        beginTest ("ChannelMapping for well-ordered stereo buses does no mapping");
        {
            const AudioProcessor::BusesLayout host { { AudioChannelSet::stereo() }, { AudioChannelSet::stereo() } };
            const ParsedBuses client { { ParsedGroup { "a", { SinglePortInfo { 0, AudioChannelSet::left,  false },
                                                              SinglePortInfo { 1, AudioChannelSet::right, false } } } },
                                       { ParsedGroup { "b", { SinglePortInfo { 2, AudioChannelSet::left,  false },
                                                              SinglePortInfo { 3, AudioChannelSet::right, false } } } } };
            const PortToAudioBufferMap map { host, client };

            expect (map.getChannelForPort (0) == 0);
            expect (map.getChannelForPort (1) == 1);

            expect (map.getChannelForPort (2) == 0);
            expect (map.getChannelForPort (3) == 1);

            expect (map.getChannelForPort (4) == -1);
        }

        beginTest ("ChannelMapping for layout with backwards ports is converted to JUCE order");
        {
            const AudioProcessor::BusesLayout host { { AudioChannelSet::stereo() }, { AudioChannelSet::stereo() } };
            const ParsedBuses client { { ParsedGroup { "a", { SinglePortInfo { 0, AudioChannelSet::right, false },
                                                              SinglePortInfo { 1, AudioChannelSet::left,  false } } } },
                                       { ParsedGroup { "b", { SinglePortInfo { 2, AudioChannelSet::right, false },
                                                              SinglePortInfo { 3, AudioChannelSet::left,  false } } } } };
            const PortToAudioBufferMap map { host, client };

            expect (map.getChannelForPort (0) == 1);
            expect (map.getChannelForPort (1) == 0);

            expect (map.getChannelForPort (2) == 1);
            expect (map.getChannelForPort (3) == 0);

            expect (map.getChannelForPort (4) == -1);
        }

        beginTest ("ChannelMapping for layout with multiple buses works as expected");
        {
            const AudioProcessor::BusesLayout host { { AudioChannelSet::create5point1(), AudioChannelSet::mono() },
                                                     { AudioChannelSet::mono(), AudioChannelSet::createLCRS(), AudioChannelSet::stereo() } };
            const ParsedBuses client { { ParsedGroup { "a", { SinglePortInfo {  0, AudioChannelSet::right,         false },
                                                              SinglePortInfo {  1, AudioChannelSet::left,          false },
                                                              SinglePortInfo {  2, AudioChannelSet::LFE,           false },
                                                              SinglePortInfo {  3, AudioChannelSet::centre,        false },
                                                              SinglePortInfo {  4, AudioChannelSet::rightSurround, false },
                                                              SinglePortInfo {  5, AudioChannelSet::leftSurround,  false } } },
                                         ParsedGroup { "b", { SinglePortInfo {  6, AudioChannelSet::centre,        false } } } },
                                       { ParsedGroup { "c", { SinglePortInfo {  7, AudioChannelSet::centre,        false } } },
                                         ParsedGroup { "d", { SinglePortInfo {  8, AudioChannelSet::surround,      false },
                                                              SinglePortInfo {  9, AudioChannelSet::centre,        false },
                                                              SinglePortInfo { 10, AudioChannelSet::right,         false },
                                                              SinglePortInfo { 11, AudioChannelSet::left,          false } } },
                                         ParsedGroup { "e", { SinglePortInfo { 12, AudioChannelSet::left,          false },
                                                              SinglePortInfo { 13, AudioChannelSet::right,         false } } } } };
            const PortToAudioBufferMap map { host, client };

            expect (map.getChannelForPort ( 0) ==  1);
            expect (map.getChannelForPort ( 1) ==  0);
            expect (map.getChannelForPort ( 2) ==  3);
            expect (map.getChannelForPort ( 3) ==  2);
            expect (map.getChannelForPort ( 4) ==  5);
            expect (map.getChannelForPort ( 5) ==  4);
            expect (map.getChannelForPort ( 6) ==  6);

            expect (map.getChannelForPort ( 7) ==  0);
            expect (map.getChannelForPort ( 8) ==  4);
            expect (map.getChannelForPort ( 9) ==  3);
            expect (map.getChannelForPort (10) ==  2);
            expect (map.getChannelForPort (11) ==  1);
            expect (map.getChannelForPort (12) ==  5);
            expect (map.getChannelForPort (13) ==  6);

            expect (map.getChannelForPort (14) == -1);
        }

        beginTest ("Optional client buses may correspond to a disabled host bus");
        {
            const ParsedBuses client { { ParsedGroup { "a", { SinglePortInfo {  0, AudioChannelSet::right,         true },
                                                              SinglePortInfo {  1, AudioChannelSet::left,          true },
                                                              SinglePortInfo {  2, AudioChannelSet::LFE,           true },
                                                              SinglePortInfo {  3, AudioChannelSet::centre,        true },
                                                              SinglePortInfo {  4, AudioChannelSet::rightSurround, true },
                                                              SinglePortInfo {  5, AudioChannelSet::leftSurround,  true } } },
                                         ParsedGroup { "b", { SinglePortInfo {  6, AudioChannelSet::centre,        true } } } },
                                       { ParsedGroup { "c", { SinglePortInfo {  7, AudioChannelSet::centre,        true } } },
                                         ParsedGroup { "d", { SinglePortInfo {  8, AudioChannelSet::surround,      true },
                                                              SinglePortInfo {  9, AudioChannelSet::centre,        true },
                                                              SinglePortInfo { 10, AudioChannelSet::right,         true },
                                                              SinglePortInfo { 11, AudioChannelSet::left,          true } } },
                                         ParsedGroup { "e", { SinglePortInfo { 12, AudioChannelSet::left,          true },
                                                              SinglePortInfo { 13, AudioChannelSet::right,         true } } } } };

            const PortToAudioBufferMap mapA { AudioProcessor::BusesLayout { { AudioChannelSet::disabled(), AudioChannelSet::mono() },
                                                                            { AudioChannelSet::mono(), AudioChannelSet::disabled(), AudioChannelSet::stereo() } },
                                              client };

            expect (mapA.getChannelForPort ( 0) == -1);
            expect (mapA.getChannelForPort ( 1) == -1);
            expect (mapA.getChannelForPort ( 2) == -1);
            expect (mapA.getChannelForPort ( 3) == -1);
            expect (mapA.getChannelForPort ( 4) == -1);
            expect (mapA.getChannelForPort ( 5) == -1);
            expect (mapA.getChannelForPort ( 6) ==  0);

            expect (mapA.getChannelForPort ( 7) ==  0);
            expect (mapA.getChannelForPort ( 8) == -1);
            expect (mapA.getChannelForPort ( 9) == -1);
            expect (mapA.getChannelForPort (10) == -1);
            expect (mapA.getChannelForPort (11) == -1);
            expect (mapA.getChannelForPort (12) ==  1);
            expect (mapA.getChannelForPort (13) ==  2);

            expect (mapA.getChannelForPort (14) == -1);

            const PortToAudioBufferMap mapB { AudioProcessor::BusesLayout { { AudioChannelSet::create5point1(), AudioChannelSet::disabled() },
                                                                            { AudioChannelSet::disabled(), AudioChannelSet::disabled(), AudioChannelSet::stereo() } },
                                              client };

            expect (mapB.getChannelForPort ( 0) ==  1);
            expect (mapB.getChannelForPort ( 1) ==  0);
            expect (mapB.getChannelForPort ( 2) ==  3);
            expect (mapB.getChannelForPort ( 3) ==  2);
            expect (mapB.getChannelForPort ( 4) ==  5);
            expect (mapB.getChannelForPort ( 5) ==  4);
            expect (mapB.getChannelForPort ( 6) == -1);

            expect (mapB.getChannelForPort ( 7) == -1);
            expect (mapB.getChannelForPort ( 8) == -1);
            expect (mapB.getChannelForPort ( 9) == -1);
            expect (mapB.getChannelForPort (10) == -1);
            expect (mapB.getChannelForPort (11) == -1);
            expect (mapB.getChannelForPort (12) ==  0);
            expect (mapB.getChannelForPort (13) ==  1);

            expect (mapB.getChannelForPort (14) == -1);
        }

        beginTest ("A plugin with only grouped ports will have the same number of buses as groups");
        {
            const auto parsed = findStableBusOrder ("foo",
                                                    { { "sidechain", { SinglePortInfo { 0, AudioChannelSet::left,   false },
                                                                       SinglePortInfo { 1, AudioChannelSet::right,  false } } },
                                                      { "foo",       { SinglePortInfo { 2, AudioChannelSet::centre, false } } } },
                                                    {});
            expect (parsed.size() == 2);

            expect (parsed[0].uid == "foo");       // The main bus should always be first
            expect (parsed[0].info.size() == 1);

            expect (parsed[1].uid == "sidechain");
            expect (parsed[1].info.size() == 2);
        }

        beginTest ("A plugin with grouped and ungrouped ports will add a bus for each ungrouped port");
        {
            const auto parsed = findStableBusOrder ("foo",
                                                    { { "sidechain", { SinglePortInfo { 0, AudioChannelSet::left,   false },
                                                                       SinglePortInfo { 1, AudioChannelSet::right,  false } } },
                                                      { "foo",       { SinglePortInfo { 2, AudioChannelSet::centre, false } } } },
                                                    { SinglePortInfo { 3, AudioChannelSet::leftSurround,  false },
                                                      SinglePortInfo { 4, AudioChannelSet::centre,        true },
                                                      SinglePortInfo { 5, AudioChannelSet::rightSurround, false } });
            expect (parsed.size() == 5);

            expect (parsed[0].uid == "foo");       // The main bus should always be first
            expect (parsed[0].info.size() == 1);

            expect (parsed[1].uid == "sidechain");
            expect (parsed[1].info.size() == 2);

            expect (parsed[2].uid == "");
            expect (parsed[2].info.size() == 1);

            expect (parsed[3].uid == "");
            expect (parsed[3].info.size() == 1);

            expect (parsed[4].uid == "");
            expect (parsed[4].info.size() == 1);
        }

        beginTest ("A plugin with only ungrouped, required ports will have a single bus");
        {
            const auto parsed = findStableBusOrder ("foo",
                                                    {},
                                                    { SinglePortInfo { 0, AudioChannelSet::leftSurround,  false },
                                                      SinglePortInfo { 1, AudioChannelSet::rightSurround, false },
                                                      SinglePortInfo { 2, AudioChannelSet::left,          false },
                                                      SinglePortInfo { 3, AudioChannelSet::right,         false } });

            expect (parsed == std::vector<ParsedGroup> { { "", { SinglePortInfo { 0, AudioChannelSet::leftSurround,  false },
                                                                 SinglePortInfo { 1, AudioChannelSet::rightSurround, false },
                                                                 SinglePortInfo { 2, AudioChannelSet::left,          false },
                                                                 SinglePortInfo { 3, AudioChannelSet::right,         false } } } });
        }

        beginTest ("A plugin with only ungrouped, optional ports will have a bus per port");
        {
            const auto parsed = findStableBusOrder ("foo",
                                                    {},
                                                    { SinglePortInfo { 0, AudioChannelSet::leftSurround,  true },
                                                      SinglePortInfo { 1, AudioChannelSet::rightSurround, true },
                                                      SinglePortInfo { 2, AudioChannelSet::left,          true },
                                                      SinglePortInfo { 3, AudioChannelSet::right,         true } });

            expect (parsed == std::vector<ParsedGroup> { { "", { SinglePortInfo { 0, AudioChannelSet::leftSurround,  true } } },
                                                         { "", { SinglePortInfo { 1, AudioChannelSet::rightSurround, true } } },
                                                         { "", { SinglePortInfo { 2, AudioChannelSet::left,          true } } },
                                                         { "", { SinglePortInfo { 3, AudioChannelSet::right,         true } } } });
        }

        beginTest ("A plugin with a mix of required and optional ports will have the required ports grouped together on a single bus");
        {
            const auto parsed = findStableBusOrder ("foo",
                                                    {},
                                                    { SinglePortInfo { 0, AudioChannelSet::leftSurround,  true },
                                                      SinglePortInfo { 1, AudioChannelSet::rightSurround, false },
                                                      SinglePortInfo { 2, AudioChannelSet::left,          true },
                                                      SinglePortInfo { 3, AudioChannelSet::right,         false } });

            expect (parsed == std::vector<ParsedGroup> { { "", { SinglePortInfo { 1, AudioChannelSet::rightSurround, false },
                                                                 SinglePortInfo { 3, AudioChannelSet::right,         false } } },
                                                         { "", { SinglePortInfo { 0, AudioChannelSet::leftSurround,  true } } },
                                                         { "", { SinglePortInfo { 2, AudioChannelSet::left,          true } } } });
        }
    }
};

static LV2PluginFormatTests lv2PluginFormatTests;

} // namespace juce
