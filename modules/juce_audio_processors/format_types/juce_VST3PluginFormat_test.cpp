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

#include "juce_VST3Headers.h"
#include "juce_VST3Common.h"

namespace juce
{

class VST3PluginFormatTests : public UnitTest
{
public:
    VST3PluginFormatTests()
        : UnitTest ("VST3 Hosting", UnitTestCategories::audioProcessors)
    {
    }

    void runTest() override
    {
        beginTest ("ChannelMapping for a stereo bus performs no remapping");
        {
            ChannelMapping map (AudioChannelSet::stereo());
            expect (map.size() == 2);

            expect (map.getJuceChannelForVst3Channel (0) == 0); // L -> left
            expect (map.getJuceChannelForVst3Channel (1) == 1); // R -> right
        }

        beginTest ("ChannelMapping for a 9.1.6 bus remaps the channels appropriately");
        {
            ChannelMapping map (AudioChannelSet::create9point1point6());
            expect (map.size() == 16);

            // VST3 order is:
            //      L
            //      R
            //      C
            //      Lfe
            //      Ls
            //      Rs
            //      Lc
            //      Rc
            //      Sl
            //      Sr
            //      Tfl
            //      Tfr
            //      Trl
            //      Trr
            //      Tsl
            //      Tsr
            // JUCE order is:
            //      Left
            //      Right
            //      Centre
            //      LFE
            //      Left Surround Side
            //      Right Surround Side
            //      Top Front Left
            //      Top Front Right
            //      Top Rear Left
            //      Top Rear Right
            //      Left Surround Rear
            //      Right Surround Rear
            //      Wide Left
            //      Wide Right
            //      Top Side Left
            //      Top Side Right

            expect (map.getJuceChannelForVst3Channel (0)  == 12); // L   -> wideLeft
            expect (map.getJuceChannelForVst3Channel (1)  == 13); // R   -> wideRight
            expect (map.getJuceChannelForVst3Channel (2)  == 2);  // C   -> centre
            expect (map.getJuceChannelForVst3Channel (3)  == 3);  // Lfe -> LFE
            expect (map.getJuceChannelForVst3Channel (4)  == 10); // Ls  -> leftSurroundRear
            expect (map.getJuceChannelForVst3Channel (5)  == 11); // Rs  -> rightSurroundRear
            expect (map.getJuceChannelForVst3Channel (6)  == 0);  // Lc  -> left
            expect (map.getJuceChannelForVst3Channel (7)  == 1);  // Rc  -> right
            expect (map.getJuceChannelForVst3Channel (8)  == 4);  // Sl  -> leftSurroundSide
            expect (map.getJuceChannelForVst3Channel (9)  == 5);  // Sl  -> leftSurroundSide
            expect (map.getJuceChannelForVst3Channel (10) == 6);  // Tfl -> topFrontLeft
            expect (map.getJuceChannelForVst3Channel (11) == 7);  // Tfr -> topFrontRight
            expect (map.getJuceChannelForVst3Channel (12) == 8);  // Trl -> topRearLeft
            expect (map.getJuceChannelForVst3Channel (13) == 9);  // Trr -> topRearRight
            expect (map.getJuceChannelForVst3Channel (14) == 14); // Tsl -> topSideLeft
            expect (map.getJuceChannelForVst3Channel (15) == 15); // Tsr -> topSideRight
        }

        const auto blockSize = 128;

        beginTest ("If the host provides more buses than the plugin knows about, the remapped buffer is silent and uses only internal channels");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (2, blockSize * 2);

            const std::vector<DynamicChannelMapping> emptyBuses;
            const std::vector<DynamicChannelMapping> stereoBus { DynamicChannelMapping { AudioChannelSet::stereo() } };

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 2).withBus (testBuffers, 1);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 2).withBus (testBuffers, 1);
            auto data = makeProcessData (blockSize, ins, outs);

            for (const auto& config : { Config { stereoBus, stereoBus }, Config { emptyBuses, stereoBus }, Config { stereoBus, emptyBuses } })
            {
                testBuffers.init();

                {
                    const ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                    auto& remapped = scopedBuffer.buffer;

                    expect (remapped.getNumChannels() == config.getNumChannels());
                    expect (remapped.getNumSamples() == blockSize);

                    for (auto i = 0; i < remapped.getNumChannels(); ++i)
                        expect (allMatch (remapped, i, 0.0f));
                }

                expect (! testBuffers.isClear (0));
                expect (! testBuffers.isClear (1));
                expect (! testBuffers.isClear (2));
                expect (testBuffers.isClear (3));
                expect (testBuffers.isClear (4));
                expect (testBuffers.isClear (5));
            }
        }

        beginTest ("If the host provides fewer buses than the plugin knows about, the remapped buffer is silent and uses only internal channels");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (3, blockSize * 2);

            const std::vector<DynamicChannelMapping> noBus;
            const std::vector<DynamicChannelMapping> oneBus { DynamicChannelMapping { AudioChannelSet::mono() } };
            const std::vector<DynamicChannelMapping> twoBuses { DynamicChannelMapping { AudioChannelSet::mono() },
                                                                DynamicChannelMapping { AudioChannelSet::stereo() } };

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 1);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 1);
            auto data = makeProcessData (blockSize, ins, outs);

            for (const auto& config : { Config { noBus, twoBuses },
                                        Config { twoBuses, noBus },
                                        Config { oneBus, twoBuses },
                                        Config { twoBuses, oneBus },
                                        Config { twoBuses, twoBuses } })
            {
                testBuffers.init();

                {
                    const ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                    auto& remapped = scopedBuffer.buffer;

                    expect (remapped.getNumChannels() == config.getNumChannels());
                    expect (remapped.getNumSamples() == blockSize);

                    // The remapped buffer will only be cleared if the host's input layout does not
                    // match the client's input layout.
                    if (config.ins.size() != 1)
                        for (auto i = 0; i < remapped.getNumChannels(); ++i)
                            expect (allMatch (remapped, i, 0.0f));
                }

                expect (! testBuffers.isClear (0));
                expect (testBuffers.isClear (1));
            }
        }

        beginTest ("If the host channel count on any bus is incorrect, the remapped buffer is silent and uses only internal channels");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (3, blockSize * 2);

            const std::vector<DynamicChannelMapping> monoBus { DynamicChannelMapping { AudioChannelSet::mono() } };
            const std::vector<DynamicChannelMapping> stereoBus { DynamicChannelMapping { AudioChannelSet::stereo() } };

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 1);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 2);
            auto data = makeProcessData (blockSize, ins, outs);

            for (const auto& config : { Config { stereoBus, monoBus },
                                        Config { stereoBus, stereoBus },
                                        Config { monoBus, monoBus } })
            {
                testBuffers.init();

                {
                    const ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                    auto& remapped = scopedBuffer.buffer;

                    expect (remapped.getNumChannels() == config.getNumChannels());
                    expect (remapped.getNumSamples() == blockSize);

                    // The remapped buffer will only be cleared if the host's input layout does not
                    // match the client's input layout.
                    if (config.ins.front().size() != 1)
                        for (auto i = 0; i < remapped.getNumChannels(); ++i)
                            expect (allMatch (remapped, i, 0.0f));
                }

                expect (! testBuffers.isClear (0));
                expect (testBuffers.isClear (1));
                expect (testBuffers.isClear (2));
            }
        }

        beginTest ("A layout with more output channels than input channels leaves unused inputs untouched");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (20, blockSize * 2);

            const Config config { { DynamicChannelMapping { AudioChannelSet::mono() },
                                    DynamicChannelMapping { AudioChannelSet::create5point1() } },
                                  { DynamicChannelMapping { AudioChannelSet::stereo() },
                                    DynamicChannelMapping { AudioChannelSet::create7point1() } } };

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 1).withBus (testBuffers, 6);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 2).withBus (testBuffers, 8);

            auto data = makeProcessData (blockSize, ins, outs);

            testBuffers.init();

            {
                ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                auto& remapped = scopedBuffer.buffer;

                expect (remapped.getNumChannels() == 10);

                // Data from the input channels is copied to the correct channels of the remapped buffer
                expect (allMatch (remapped, 0, 1.0f));
                expect (allMatch (remapped, 1, 2.0f));
                expect (allMatch (remapped, 2, 3.0f));
                expect (allMatch (remapped, 3, 4.0f));
                expect (allMatch (remapped, 4, 5.0f));
                expect (allMatch (remapped, 5, 6.0f));
                expect (allMatch (remapped, 6, 7.0f));
                // The remaining channels are output-only, so they may contain any data

                // Write some data to the buffer in JUCE layout
                for (auto i = 0; i < remapped.getNumChannels(); ++i)
                {
                    auto* ptr = remapped.getWritePointer (i);
                    std::fill (ptr, ptr + remapped.getNumSamples(), (float) i);
                }
            }

            // Channels are copied back to the correct output buffer
            expect (channelStartsWithValue (data.outputs[0], 0, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 1, 1.0f));

            expect (channelStartsWithValue (data.outputs[1], 0, 2.0f));
            expect (channelStartsWithValue (data.outputs[1], 1, 3.0f));
            expect (channelStartsWithValue (data.outputs[1], 2, 4.0f));
            expect (channelStartsWithValue (data.outputs[1], 3, 5.0f));
            expect (channelStartsWithValue (data.outputs[1], 4, 8.0f));  // JUCE surround side -> VST3 surround side
            expect (channelStartsWithValue (data.outputs[1], 5, 9.0f));
            expect (channelStartsWithValue (data.outputs[1], 6, 6.0f));  // JUCE surround rear -> VST3 surround rear
            expect (channelStartsWithValue (data.outputs[1], 7, 7.0f));
        }

        beginTest ("A layout with more input channels than output channels doesn't attempt to output any input channels");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (15, blockSize * 2);

            const Config config { { DynamicChannelMapping { AudioChannelSet::create7point1point6() },
                                    DynamicChannelMapping { AudioChannelSet::mono() } },
                                  { DynamicChannelMapping { AudioChannelSet::createLCRS() },
                                    DynamicChannelMapping { AudioChannelSet::stereo() } } };

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 14).withBus (testBuffers, 1);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 4) .withBus (testBuffers, 2);

            auto data = makeProcessData (blockSize, ins, outs);

            testBuffers.init();

            {
                ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                auto& remapped = scopedBuffer.buffer;

                expect (remapped.getNumChannels() == 15);

                // Data from the input channels is copied to the correct channels of the remapped buffer
                expect (allMatch (remapped, 0,   1.0f));
                expect (allMatch (remapped, 1,   2.0f));
                expect (allMatch (remapped, 2,   3.0f));
                expect (allMatch (remapped, 3,   4.0f));
                expect (allMatch (remapped, 4,   7.0f));
                expect (allMatch (remapped, 5,   8.0f));
                expect (allMatch (remapped, 6,   9.0f));
                expect (allMatch (remapped, 7,  10.0f));
                expect (allMatch (remapped, 8,  11.0f));
                expect (allMatch (remapped, 9,  12.0f));
                expect (allMatch (remapped, 10,  5.0f));
                expect (allMatch (remapped, 11,  6.0f));
                expect (allMatch (remapped, 12, 13.0f));
                expect (allMatch (remapped, 13, 14.0f));
                expect (allMatch (remapped, 14, 15.0f));

                // Write some data to the buffer in JUCE layout
                for (auto i = 0; i < remapped.getNumChannels(); ++i)
                {
                    auto* ptr = remapped.getWritePointer (i);
                    std::fill (ptr, ptr + remapped.getNumSamples(), (float) i);
                }
            }

            // Channels are copied back to the correct output buffer
            expect (channelStartsWithValue (data.outputs[0], 0, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 1, 1.0f));
            expect (channelStartsWithValue (data.outputs[0], 2, 2.0f));
            expect (channelStartsWithValue (data.outputs[0], 3, 3.0f));

            expect (channelStartsWithValue (data.outputs[1], 0, 4.0f));
            expect (channelStartsWithValue (data.outputs[1], 1, 5.0f));
        }

        beginTest ("Inactive buses are ignored");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (18, blockSize * 2);

            Config config { { DynamicChannelMapping { AudioChannelSet::create7point1point6() },
                              DynamicChannelMapping { AudioChannelSet::mono(), false },
                              DynamicChannelMapping { AudioChannelSet::quadraphonic() },
                              DynamicChannelMapping { AudioChannelSet::mono(), false } },
                            { DynamicChannelMapping { AudioChannelSet::create5point0(), false },
                              DynamicChannelMapping { AudioChannelSet::createLCRS() },
                              DynamicChannelMapping { AudioChannelSet::stereo() } } };

            config.ins[1].setHostActive (false);
            config.ins[3].setHostActive (false);

            TestBuffers testBuffers { blockSize };

            // The host doesn't need to provide trailing buses that are inactive, as long as the
            // client knows those buses are inactive.
            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 14).withBus (testBuffers, 1).withBus (testBuffers, 4);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 5) .withBus (testBuffers, 4).withBus (testBuffers, 2);

            auto data = makeProcessData (blockSize, ins, outs);

            testBuffers.init();

            {
                ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                auto& remapped = scopedBuffer.buffer;

                expect (remapped.getNumChannels() == 18);

                // Data from the input channels is copied to the correct channels of the remapped buffer
                expect (allMatch (remapped, 0,   1.0f));
                expect (allMatch (remapped, 1,   2.0f));
                expect (allMatch (remapped, 2,   3.0f));
                expect (allMatch (remapped, 3,   4.0f));
                expect (allMatch (remapped, 4,   7.0f));
                expect (allMatch (remapped, 5,   8.0f));
                expect (allMatch (remapped, 6,   9.0f));
                expect (allMatch (remapped, 7,  10.0f));
                expect (allMatch (remapped, 8,  11.0f));
                expect (allMatch (remapped, 9,  12.0f));
                expect (allMatch (remapped, 10,  5.0f));
                expect (allMatch (remapped, 11,  6.0f));
                expect (allMatch (remapped, 12, 13.0f));
                expect (allMatch (remapped, 13, 14.0f));

                expect (allMatch (remapped, 14, 16.0f));
                expect (allMatch (remapped, 15, 17.0f));
                expect (allMatch (remapped, 16, 18.0f));
                expect (allMatch (remapped, 17, 19.0f));

                // Write some data to the buffer in JUCE layout
                for (auto i = 0; i < remapped.getNumChannels(); ++i)
                {
                    auto* ptr = remapped.getWritePointer (i);
                    std::fill (ptr, ptr + remapped.getNumSamples(), (float) i);
                }
            }

            // All channels on the first output bus should be cleared, because the plugin
            // thinks that this bus is inactive.
            expect (channelStartsWithValue (data.outputs[0], 0, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 1, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 2, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 3, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 4, 0.0f));

            // Remaining channels should be copied back as normal
            expect (channelStartsWithValue (data.outputs[1], 0, 0.0f));
            expect (channelStartsWithValue (data.outputs[1], 1, 1.0f));
            expect (channelStartsWithValue (data.outputs[1], 2, 2.0f));
            expect (channelStartsWithValue (data.outputs[1], 3, 3.0f));

            expect (channelStartsWithValue (data.outputs[2], 0, 4.0f));
            expect (channelStartsWithValue (data.outputs[2], 1, 5.0f));
        }

        beginTest ("Null pointers are allowed on inactive buses provided to clients");
        {
            ClientBufferMapperData<float> remapper;
            remapper.prepare (8, blockSize * 2);

            const std::vector<ChannelMapping> emptyBuses;
            const std::vector<ChannelMapping> stereoBus { ChannelMapping { AudioChannelSet::stereo() } };

            Config config { { DynamicChannelMapping { AudioChannelSet::stereo() },
                              DynamicChannelMapping { AudioChannelSet::quadraphonic(), false },
                              DynamicChannelMapping { AudioChannelSet::stereo() } },
                            { DynamicChannelMapping { AudioChannelSet::quadraphonic() },
                              DynamicChannelMapping { AudioChannelSet::stereo(), false },
                              DynamicChannelMapping { AudioChannelSet::quadraphonic() } } };

            config.ins[1].setHostActive (false);
            config.outs[1].setHostActive (false);

            TestBuffers testBuffers { blockSize };

            auto ins  = MultiBusBuffers{}.withBus (testBuffers, 2).withBus (testBuffers, 4).withBus (testBuffers, 2);
            auto outs = MultiBusBuffers{}.withBus (testBuffers, 4).withBus (testBuffers, 2).withBus (testBuffers, 4);

            auto data = makeProcessData (blockSize, ins, outs);

            for (auto i = 0; i < 4; ++i)
                data.inputs [1].channelBuffers32[i] = nullptr;

            for (auto i = 0; i < 2; ++i)
                data.outputs[1].channelBuffers32[i] = nullptr;

            testBuffers.init();

            {
                ClientRemappedBuffer<float> scopedBuffer { remapper, &config.ins, &config.outs, data };
                auto& remapped = scopedBuffer.buffer;

                expect (remapped.getNumChannels() == 8);

                expect (allMatch (remapped, 0,   1.0f));
                expect (allMatch (remapped, 1,   2.0f));
                // skip 4 inactive channels
                expect (allMatch (remapped, 2,   7.0f));
                expect (allMatch (remapped, 3,   8.0f));

                // Write some data to the buffer in JUCE layout
                for (auto i = 0; i < remapped.getNumChannels(); ++i)
                {
                    auto* ptr = remapped.getWritePointer (i);
                    std::fill (ptr, ptr + remapped.getNumSamples(), (float) i);
                }
            }

            expect (channelStartsWithValue (data.outputs[0], 0, 0.0f));
            expect (channelStartsWithValue (data.outputs[0], 1, 1.0f));
            expect (channelStartsWithValue (data.outputs[0], 2, 2.0f));
            expect (channelStartsWithValue (data.outputs[0], 3, 3.0f));

            expect (channelStartsWithValue (data.outputs[2], 0, 4.0f));
            expect (channelStartsWithValue (data.outputs[2], 1, 5.0f));
            expect (channelStartsWithValue (data.outputs[2], 2, 6.0f));
            expect (channelStartsWithValue (data.outputs[2], 3, 7.0f));
        }

        beginTest ("HostBufferMapper reorders channels correctly");
        {
            HostBufferMapper mapper;

            {
                mapper.prepare ({ ChannelMapping { AudioChannelSet::stereo() },
                                  ChannelMapping { AudioChannelSet::create7point1point2() },
                                  ChannelMapping { AudioChannelSet::create9point1point6(), false },
                                  ChannelMapping { AudioChannelSet::createLCRS() } });
                AudioBuffer<float> hostBuffer (16, blockSize);
                const auto* clientBuffers = mapper.getVst3LayoutForJuceBuffer (hostBuffer);

                expect (clientBuffers[0].numChannels == 2);
                expect (clientBuffers[1].numChannels == 10);
                // Even though it's disabled, this bus should still have the correct channel count
                expect (clientBuffers[2].numChannels == 16);
                expect (clientBuffers[3].numChannels == 4);

                expect (clientBuffers[0].channelBuffers32[0]  == hostBuffer.getReadPointer (0));
                expect (clientBuffers[0].channelBuffers32[1]  == hostBuffer.getReadPointer (1));

                expect (clientBuffers[1].channelBuffers32[0]  == hostBuffer.getReadPointer (2));
                expect (clientBuffers[1].channelBuffers32[1]  == hostBuffer.getReadPointer (3));
                expect (clientBuffers[1].channelBuffers32[2]  == hostBuffer.getReadPointer (4));
                expect (clientBuffers[1].channelBuffers32[3]  == hostBuffer.getReadPointer (5));
                expect (clientBuffers[1].channelBuffers32[4]  == hostBuffer.getReadPointer (8));
                expect (clientBuffers[1].channelBuffers32[5]  == hostBuffer.getReadPointer (9));
                expect (clientBuffers[1].channelBuffers32[6]  == hostBuffer.getReadPointer (6));
                expect (clientBuffers[1].channelBuffers32[7]  == hostBuffer.getReadPointer (7));
                expect (clientBuffers[1].channelBuffers32[8]  == hostBuffer.getReadPointer (10));
                expect (clientBuffers[1].channelBuffers32[9]  == hostBuffer.getReadPointer (11));

                for (auto i = 0; i < clientBuffers[2].numChannels; ++i)
                    expect (clientBuffers[2].channelBuffers32[i] == nullptr);

                expect (clientBuffers[3].channelBuffers32[0]  == hostBuffer.getReadPointer (12));
                expect (clientBuffers[3].channelBuffers32[1]  == hostBuffer.getReadPointer (13));
                expect (clientBuffers[3].channelBuffers32[2]  == hostBuffer.getReadPointer (14));
                expect (clientBuffers[3].channelBuffers32[3]  == hostBuffer.getReadPointer (15));
            }

            {
                mapper.prepare ({ ChannelMapping { AudioChannelSet::mono() },
                                  ChannelMapping { AudioChannelSet::mono(), false },
                                  ChannelMapping { AudioChannelSet::mono() },
                                  ChannelMapping { AudioChannelSet::mono(), false } });
                AudioBuffer<double> hostBuffer (2, blockSize);
                const auto* clientBuffers = mapper.getVst3LayoutForJuceBuffer (hostBuffer);

                expect (clientBuffers[0].numChannels == 1);
                expect (clientBuffers[1].numChannels == 1);
                expect (clientBuffers[2].numChannels == 1);
                expect (clientBuffers[3].numChannels == 1);

                expect (clientBuffers[0].channelBuffers64[0] == hostBuffer.getReadPointer (0));
                expect (clientBuffers[1].channelBuffers64[0] == nullptr);
                expect (clientBuffers[2].channelBuffers64[0] == hostBuffer.getReadPointer (1));
                expect (clientBuffers[3].channelBuffers64[0] == nullptr);
            }
        }

        beginTest ("Speaker layout conversions");
        {
            using namespace Steinberg::Vst::SpeakerArr;

            for (const auto& [channelSet, arr] : { std::tuple (AudioChannelSet::ambisonic (1), kAmbi1stOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (2), kAmbi2cdOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (3), kAmbi3rdOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (4), kAmbi4thOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (5), kAmbi5thOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (6), kAmbi6thOrderACN),
                                                   std::tuple (AudioChannelSet::ambisonic (7), kAmbi7thOrderACN), })
            {
                expect (getVst3SpeakerArrangement (channelSet) == arr);
                expect (channelSet == getChannelSetForSpeakerArrangement (arr));
            }
        }
    }

private:
    //==============================================================================
    struct Config
    {
        Config (std::vector<DynamicChannelMapping> i, std::vector<DynamicChannelMapping> o)
            : ins (std::move (i)), outs (std::move (o))
        {
            for (auto container : { &ins, &outs })
                for (auto& x : *container)
                    x.setHostActive (true);
        }

        std::vector<DynamicChannelMapping> ins, outs;

        int getNumChannels() const { return countUsedClientChannels (ins, outs); }
    };

    struct TestBuffers
    {
        explicit TestBuffers (int samples) : numSamples (samples) {}

        void init()
        {
            auto index = 1;

            for (auto& channel : buffers)
                std::fill (channel.begin(), channel.end(), (float) index++);
        }

        bool allMatch (int channel, float value) const
        {
            const auto& buf = buffers[(size_t) channel];
            return std::all_of (buf.begin(), buf.end(), [&] (auto x) { return exactlyEqual (x, value); });
        }

        bool isClear (int channel) const
        {
            return allMatch (channel, 0.0f);
        }

        float* addChannel()
        {
            buffers.emplace_back (numSamples);
            return buffers.back().data();
        }

              float* get (int channel)       { return buffers[(size_t) channel].data(); }
        const float* get (int channel) const { return buffers[(size_t) channel].data(); }

        std::vector<std::vector<float>> buffers;
        int numSamples = 0;
    };

    static bool channelStartsWithValue (Steinberg::Vst::AudioBusBuffers& bus, size_t index, float value)
    {
        return exactlyEqual (bus.channelBuffers32[index][0], value);
    }

    static bool allMatch (const AudioBuffer<float>& buf, int index, float value)
    {
        const auto* ptr = buf.getReadPointer (index);
        return std::all_of (ptr, ptr + buf.getNumSamples(), [&] (auto x) { return exactlyEqual (x, value); });
    }

    struct MultiBusBuffers
    {
        std::vector<Steinberg::Vst::AudioBusBuffers> buffers;
        std::vector<std::vector<float*>> pointerStorage;

        MultiBusBuffers withBus (TestBuffers& storage, int numChannels) &&
        {
            MultiBusBuffers result { std::move (buffers), std::move (pointerStorage) };

            std::vector<float*> pointers;

            for (auto i = 0; i < numChannels; ++i)
                pointers.push_back (storage.addChannel());

            Steinberg::Vst::AudioBusBuffers buffer;
            buffer.numChannels = (Steinberg::int32) pointers.size();
            buffer.channelBuffers32 = pointers.data();

            result.buffers.push_back (buffer);
            result.pointerStorage.push_back (std::move (pointers));

            return result;
        }
    };

    static Steinberg::Vst::ProcessData makeProcessData (int blockSize, MultiBusBuffers& ins, MultiBusBuffers& outs)
    {
        Steinberg::Vst::ProcessData result;
        result.numSamples = blockSize;
        result.inputs = ins.buffers.data();
        result.numInputs = (Steinberg::int32) ins.buffers.size();
        result.outputs = outs.buffers.data();
        result.numOutputs = (Steinberg::int32) outs.buffers.size();
        return result;
    }
};

static VST3PluginFormatTests vst3PluginFormatTests;

} // namespace juce
