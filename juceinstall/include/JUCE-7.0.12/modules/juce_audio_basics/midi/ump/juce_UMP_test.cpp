/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::universal_midi_packets
{

constexpr uint8_t  operator""_u8  (unsigned long long int i) { return static_cast<uint8_t>  (i); }
constexpr uint16_t operator""_u16 (unsigned long long int i) { return static_cast<uint16_t> (i); }
constexpr uint32_t operator""_u32 (unsigned long long int i) { return static_cast<uint32_t> (i); }
constexpr uint64_t operator""_u64 (unsigned long long int i) { return static_cast<uint64_t> (i); }

class UniversalMidiPacketTests final : public UnitTest
{
public:
    UniversalMidiPacketTests()
        : UnitTest ("Universal MIDI Packet", UnitTestCategories::midi)
    {
    }

    void runTest() override
    {
        auto random = getRandom();

        beginTest ("Short bytestream midi messages can be round-tripped through the UMP converter");
        {
            Midi1ToBytestreamTranslator translator (0);

            forEachNonSysExTestMessage (random, [&] (const MidiMessage& m)
            {
                const auto packets = toMidi1 (m);
                expect (packets.size() == 1);

                // Make sure that the message type is correct
                const auto msgType = Utils::getMessageType (packets.data()[0]);
                expect (msgType == ((m.getRawData()[0] >> 0x4) == 0xf ? 0x1 : 0x2));

                translator.dispatch (View {packets.data() },
                                     0,
                                     [&] (const BytestreamMidiView& roundTripped)
                                     {
                                         expect (equal (m, roundTripped.getMessage()));
                                     });
            });
        }

        beginTest ("Bytestream SysEx converts to universal packets");
        {
            {
                // Zero length message
                const auto packets = toMidi1 (createRandomSysEx (random, 0));
                expect (packets.size() == 2);

                expect (packets.data()[0] == 0x30000000);
                expect (packets.data()[1] == 0x00000000);
            }

            {
                const auto message = createRandomSysEx (random, 1);
                const auto packets = toMidi1 (message);
                expect (packets.size() == 2);

                const auto* sysEx = message.getSysExData();
                expect (packets.data()[0] == Utils::bytesToWord (std::byte { 0x30 },
                                                                 std::byte { 0x01 },
                                                                 std::byte { sysEx[0] },
                                                                 std::byte { 0 }));
                expect (packets.data()[1] == 0x00000000);
            }

            {
                const auto message = createRandomSysEx (random, 6);
                const auto packets = toMidi1 (message);
                expect (packets.size() == 2);

                const auto* sysEx = message.getSysExData();
                expect (packets.data()[0] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x06 },     std::byte { sysEx[0] }, std::byte { sysEx[1] }));
                expect (packets.data()[1] == Utils::bytesToWord (std::byte { sysEx[2] }, std::byte { sysEx[3] }, std::byte { sysEx[4] }, std::byte { sysEx[5] }));
            }

            {
                const auto message = createRandomSysEx (random, 12);
                const auto packets = toMidi1 (message);
                expect (packets.size() == 4);

                const auto* sysEx = message.getSysExData();
                expect (packets.data()[0] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x16 },     std::byte { sysEx[0] },  std::byte { sysEx[1] }));
                expect (packets.data()[1] == Utils::bytesToWord (std::byte { sysEx[2] }, std::byte { sysEx[3] }, std::byte { sysEx[4] },  std::byte { sysEx[5] }));
                expect (packets.data()[2] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x36 },     std::byte { sysEx[6] },  std::byte { sysEx[7] }));
                expect (packets.data()[3] == Utils::bytesToWord (std::byte { sysEx[8] }, std::byte { sysEx[9] }, std::byte { sysEx[10] }, std::byte { sysEx[11] }));
            }

            {
                const auto message = createRandomSysEx (random, 13);
                const auto packets = toMidi1 (message);
                expect (packets.size() == 6);

                const auto* sysEx = message.getSysExData();
                expect (packets.data()[0] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x16 },     std::byte { sysEx[0] },  std::byte { sysEx[1] }));
                expect (packets.data()[1] == Utils::bytesToWord (std::byte { sysEx[2] }, std::byte { sysEx[3] }, std::byte { sysEx[4] },  std::byte { sysEx[5] }));
                expect (packets.data()[2] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x26 },     std::byte { sysEx[6] },  std::byte { sysEx[7] }));
                expect (packets.data()[3] == Utils::bytesToWord (std::byte { sysEx[8] }, std::byte { sysEx[9] }, std::byte { sysEx[10] }, std::byte { sysEx[11] }));
                expect (packets.data()[4] == Utils::bytesToWord (std::byte { 0x30 },     std::byte { 0x31 },     std::byte { sysEx[12] }, std::byte { 0 }));
                expect (packets.data()[5] == 0x00000000);
            }
        }

        ToBytestreamDispatcher converter (0);
        Packets packets;

        const auto checkRoundTrip = [&] (const MidiBuffer& expected)
        {
            for (const auto meta : expected)
                Conversion::toMidi1 (ump::BytestreamMidiView (meta), [&] (const auto p) { packets.add (p); });

            MidiBuffer output;
            converter.dispatch (packets.data(),
                                packets.data() + packets.size(),
                                0,
                                [&] (const BytestreamMidiView& roundTripped)
                                {
                                    output.addEvent (roundTripped.getMessage(), int (roundTripped.timestamp));
                                });
            packets.clear();

            expect (equal (expected, output));
        };

        beginTest ("Long SysEx bytestream midi messages can be round-tripped through the UMP converter");
        {
            for (auto length : { 0, 1, 2, 3, 4, 5, 6, 7, 13, 20, 100, 1000 })
            {
                MidiBuffer expected;
                expected.addEvent (createRandomSysEx (random, size_t (length)), 0);
                checkRoundTrip (expected);
            }
        }

        beginTest ("UMP SysEx7 messages interspersed with utility messages convert to bytestream");
        {
            const auto sysEx = createRandomSysEx (random, 100);
            const auto originalPackets = toMidi1 (sysEx);

            Packets modifiedPackets;

            const auto addRandomUtilityUMP = [&]
            {
                const auto newPacket = createRandomUtilityUMP (random);
                modifiedPackets.add (View (newPacket.data()));
            };

            for (const auto& packet : originalPackets)
            {
                addRandomUtilityUMP();
                modifiedPackets.add (packet);
                addRandomUtilityUMP();
            }

            MidiBuffer output;
            converter.dispatch (modifiedPackets.data(),
                                modifiedPackets.data() + modifiedPackets.size(),
                                0,
                                [&] (const BytestreamMidiView& roundTripped)
                                {
                                    output.addEvent (roundTripped.getMessage(), int (roundTripped.timestamp));
                                });

            // All Utility messages should have been ignored
            expect (output.getNumEvents() == 1);

            for (const auto meta : output)
                expect (equal (meta.getMessage(), sysEx));
        }

        beginTest ("UMP SysEx7 messages interspersed with System Realtime messages convert to bytestream");
        {
            const auto sysEx = createRandomSysEx (random, 200);
            const auto originalPackets = toMidi1 (sysEx);

            Packets modifiedPackets;
            MidiBuffer realtimeMessages;

            const auto addRandomRealtimeUMP = [&]
            {
                const auto newPacket = createRandomRealtimeUMP (random);
                modifiedPackets.add (View (newPacket.data()));
                realtimeMessages.addEvent (Midi1ToBytestreamTranslator::fromUmp (newPacket), 0);
            };

            for (const auto& packet : originalPackets)
            {
                addRandomRealtimeUMP();
                modifiedPackets.add (packet);
                addRandomRealtimeUMP();
            }

            MidiBuffer output;
            converter.dispatch (modifiedPackets.data(),
                                modifiedPackets.data() + modifiedPackets.size(),
                                0,
                                [&] (const BytestreamMidiView& roundTripped)
                                {
                                    output.addEvent (roundTripped.getMessage(), int (roundTripped.timestamp));
                                });

            const auto numOutputs = output.getNumEvents();
            const auto numInputs = realtimeMessages.getNumEvents();
            expect (numOutputs == numInputs + 1);

            if (numOutputs == numInputs + 1)
            {
                const auto isMetadataEquivalent = [] (const MidiMessageMetadata& a,
                                                      const MidiMessageMetadata& b)
                {
                    return equal (a.getMessage(), b.getMessage());
                };

                auto it = output.begin();

                for (const auto meta : realtimeMessages)
                {
                    if (! isMetadataEquivalent (*it, meta))
                    {
                        expect (equal ((*it).getMessage(), sysEx));
                        ++it;
                    }

                    expect (isMetadataEquivalent (*it, meta));
                    ++it;
                }
            }
        }

        beginTest ("UMP SysEx7 messages interspersed with System Realtime and Utility messages convert to bytestream");
        {
            const auto sysEx = createRandomSysEx (random, 300);
            const auto originalPackets = toMidi1 (sysEx);

            Packets modifiedPackets;
            MidiBuffer realtimeMessages;

            const auto addRandomRealtimeUMP = [&]
            {
                const auto newPacket = createRandomRealtimeUMP (random);
                modifiedPackets.add (View (newPacket.data()));
                realtimeMessages.addEvent (Midi1ToBytestreamTranslator::fromUmp (newPacket), 0);
            };

            const auto addRandomUtilityUMP = [&]
            {
                const auto newPacket = createRandomUtilityUMP (random);
                modifiedPackets.add (View (newPacket.data()));
            };

            for (const auto& packet : originalPackets)
            {
                addRandomRealtimeUMP();
                addRandomUtilityUMP();
                modifiedPackets.add (packet);
                addRandomRealtimeUMP();
                addRandomUtilityUMP();
            }

            MidiBuffer output;
            converter.dispatch (modifiedPackets.data(),
                                modifiedPackets.data() + modifiedPackets.size(),
                                0,
                                [&] (const BytestreamMidiView& roundTripped)
                                {
                                    output.addEvent (roundTripped.getMessage(), int (roundTripped.timestamp));
                                });

            const auto numOutputs = output.getNumEvents();
            const auto numInputs = realtimeMessages.getNumEvents();
            expect (numOutputs == numInputs + 1);

            if (numOutputs == numInputs + 1)
            {
                const auto isMetadataEquivalent = [] (const MidiMessageMetadata& a, const MidiMessageMetadata& b)
                {
                    return equal (a.getMessage(), b.getMessage());
                };

                auto it = output.begin();

                for (const auto meta : realtimeMessages)
                {
                    if (! isMetadataEquivalent (*it, meta))
                    {
                        expect (equal ((*it).getMessage(), sysEx));
                        ++it;
                    }

                    expect (isMetadataEquivalent (*it, meta));
                    ++it;
                }
            }
        }

        beginTest ("SysEx messages are terminated by non-Utility, non-Realtime messages");
        {
            const auto noteOn = [&]
            {
                MidiBuffer b;
                b.addEvent (MidiMessage::noteOn (1, uint8_t (64), uint8_t (64)), 0);
                return b;
            }();

            const auto noteOnPackets = [&]
            {
                Packets p;

                for (const auto meta : noteOn)
                    Conversion::toMidi1 (ump::BytestreamMidiView (meta), [&] (const auto packet) { p.add (packet); });

                return p;
            }();

            const auto sysEx = createRandomSysEx (random, 300);

            const auto originalPackets = toMidi1 (sysEx);

            const auto modifiedPackets = [&]
            {
                Packets p;

                const auto insertionPoint = std::next (originalPackets.begin(), 10);
                std::for_each (originalPackets.begin(),
                               insertionPoint,
                               [&] (const View& view) { p.add (view); });

                for (const auto& view : noteOnPackets)
                    p.add (view);

                std::for_each (insertionPoint,
                               originalPackets.end(),
                               [&] (const View& view) { p.add (view); });

                return p;
            }();

            // modifiedPackets now contains some SysEx packets interrupted by a MIDI 1 noteOn

            MidiBuffer output;

            const auto pushToOutput = [&] (const Packets& p)
            {
                converter.dispatch (p.data(),
                                    p.data() + p.size(),
                                    0,
                                    [&] (const BytestreamMidiView& roundTripped)
                                    {
                                        output.addEvent (roundTripped.getMessage(), int (roundTripped.timestamp));
                                    });
            };

            pushToOutput (modifiedPackets);

            // Interrupted sysEx shouldn't be present
            expect (equal (output, noteOn));

            const auto newSysEx = createRandomSysEx (random, 300);
            const auto newSysExPackets = toMidi1 (newSysEx);

            // If we push another midi event without interrupting it,
            // it should get through without being modified,
            // and it shouldn't be affected by the previous (interrupted) sysex.

            output.clear();
            pushToOutput (newSysExPackets);

            expect (output.getNumEvents() == 1);

            for (const auto meta : output)
                expect (equal (meta.getMessage(), newSysEx));
        }

        beginTest ("Widening conversions work");
        {
            // This is similar to the 'slow' example code from the MIDI 2.0 spec
            const auto baselineScale = [] (uint32_t srcVal, uint32_t srcBits, uint32_t dstBits)
            {
                const auto scaleBits = (uint32_t) (dstBits - srcBits);

                auto bitShiftedValue = (uint32_t) (srcVal << scaleBits);

                const auto srcCenter = (uint32_t) (1 << (srcBits - 1));

                if (srcVal <= srcCenter)
                    return bitShiftedValue;

                const auto repeatBits = (uint32_t) (srcBits - 1);
                const auto repeatMask = (uint32_t) ((1 << repeatBits) - 1);

                auto repeatValue = (uint32_t) (srcVal & repeatMask);

                if (scaleBits > repeatBits)
                    repeatValue <<= scaleBits - repeatBits;
                else
                    repeatValue >>= repeatBits - scaleBits;

                while (repeatValue != 0)
                {
                    bitShiftedValue |= repeatValue;
                    repeatValue >>= repeatBits;
                }

                return bitShiftedValue;
            };

            const auto baselineScale7To8 = [&] (uint8_t in)
            {
                return baselineScale (in, 7, 8);
            };

            const auto baselineScale7To16 = [&] (uint8_t in)
            {
                return baselineScale (in, 7, 16);
            };

            const auto baselineScale14To16 = [&] (uint16_t in)
            {
                return baselineScale (in, 14, 16);
            };

            const auto baselineScale7To32 = [&] (uint8_t in)
            {
                return baselineScale (in, 7, 32);
            };

            const auto baselineScale14To32 = [&] (uint16_t in)
            {
                return baselineScale (in, 14, 32);
            };

            for (auto i = 0; i != 100; ++i)
            {
                const auto rand = (uint8_t) random.nextInt (0x80);
                expectEquals ((int64_t) Conversion::scaleTo8 (rand),
                              (int64_t) baselineScale7To8 (rand));
            }

            expectEquals ((int64_t) Conversion::scaleTo16 ((uint8_t) 0x00), (int64_t) 0x0000);
            expectEquals ((int64_t) Conversion::scaleTo16 ((uint8_t) 0x0a), (int64_t) 0x1400);
            expectEquals ((int64_t) Conversion::scaleTo16 ((uint8_t) 0x40), (int64_t) 0x8000);
            expectEquals ((int64_t) Conversion::scaleTo16 ((uint8_t) 0x57), (int64_t) 0xaeba);
            expectEquals ((int64_t) Conversion::scaleTo16 ((uint8_t) 0x7f), (int64_t) 0xffff);

            for (auto i = 0; i != 100; ++i)
            {
                const auto rand = (uint8_t) random.nextInt (0x80);
                expectEquals ((int64_t) Conversion::scaleTo16 (rand),
                              (int64_t) baselineScale7To16 (rand));
            }

            for (auto i = 0; i != 100; ++i)
            {
                const auto rand = (uint16_t) random.nextInt (0x4000);
                expectEquals ((int64_t) Conversion::scaleTo16 (rand),
                              (int64_t) baselineScale14To16 (rand));
            }

            for (auto i = 0; i != 100; ++i)
            {
                const auto rand = (uint8_t) random.nextInt (0x80);
                expectEquals ((int64_t) Conversion::scaleTo32 (rand),
                              (int64_t) baselineScale7To32 (rand));
            }

            expectEquals ((int64_t) Conversion::scaleTo32 ((uint16_t) 0x0000), (int64_t) 0x00000000);
            expectEquals ((int64_t) Conversion::scaleTo32 ((uint16_t) 0x2000), (int64_t) 0x80000000);
            expectEquals ((int64_t) Conversion::scaleTo32 ((uint16_t) 0x3fff), (int64_t) 0xffffffff);

            for (auto i = 0; i != 100; ++i)
            {
                const auto rand = (uint16_t) random.nextInt (0x4000);
                expectEquals ((int64_t) Conversion::scaleTo32 (rand),
                              (int64_t) baselineScale14To32 (rand));
            }
        }

        beginTest ("Round-trip widening/narrowing conversions work");
        {
            for (auto i = 0; i != 100; ++i)
            {
                {
                    const auto rand = (uint8_t) random.nextInt (0x80);
                    expectEquals (Conversion::scaleTo7 (Conversion::scaleTo8 (rand)), rand);
                }

                {
                    const auto rand = (uint8_t) random.nextInt (0x80);
                    expectEquals (Conversion::scaleTo7 (Conversion::scaleTo16 (rand)), rand);
                }

                {
                    const auto rand = (uint8_t) random.nextInt (0x80);
                    expectEquals (Conversion::scaleTo7 (Conversion::scaleTo32 (rand)), rand);
                }

                {
                    const auto rand = (uint16_t) random.nextInt (0x4000);
                    expectEquals ((uint64_t) Conversion::scaleTo14 (Conversion::scaleTo16 (rand)), (uint64_t) rand);
                }

                {
                    const auto rand = (uint16_t) random.nextInt (0x4000);
                    expectEquals ((uint64_t) Conversion::scaleTo14 (Conversion::scaleTo32 (rand)), (uint64_t) rand);
                }
            }
        }

        beginTest ("MIDI 2 -> 1 note on conversions");
        {
            {
                Packets midi2;
                midi2.add (PacketX2 { 0x41946410, 0x12345678 });

                Packets midi1;
                midi1.add (PacketX1 { 0x21946409 });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }

            {
                // If the velocity is close to 0, the output velocity should still be 1
                Packets midi2;
                midi2.add (PacketX2 { 0x4295327f, 0x00345678 });

                Packets midi1;
                midi1.add (PacketX1 { 0x22953201 });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }
        }

        beginTest ("MIDI 2 -> 1 note off conversion");
        {
            Packets midi2;
            midi2.add (PacketX2 { 0x448b0520, 0xfedcba98 });

            Packets midi1;
            midi1.add (PacketX1 { 0x248b057f });

            checkMidi2ToMidi1Conversion (midi2, midi1);
        }

        beginTest ("MIDI 2 -> 1 poly pressure conversion");
        {
            Packets midi2;
            midi2.add (PacketX2 { 0x49af0520, 0x80dcba98 });

            Packets midi1;
            midi1.add (PacketX1 { 0x29af0540 });

            checkMidi2ToMidi1Conversion (midi2, midi1);
        }

        beginTest ("MIDI 2 -> 1 control change conversion");
        {
            Packets midi2;
            midi2.add (PacketX2 { 0x49b00520, 0x80dcba98 });

            Packets midi1;
            midi1.add (PacketX1 { 0x29b00540 });

            checkMidi2ToMidi1Conversion (midi2, midi1);
        }

        beginTest ("MIDI 2 -> 1 channel pressure conversion");
        {
            Packets midi2;
            midi2.add (PacketX2 { 0x40d20520, 0x80dcba98 });

            Packets midi1;
            midi1.add (PacketX1 { 0x20d24000 });

            checkMidi2ToMidi1Conversion (midi2, midi1);
        }

        beginTest ("MIDI 2 -> 1 nrpn rpn conversion");
        {
            {
                Packets midi2;
                midi2.add (PacketX2 { 0x44240123, 0x456789ab });

                Packets midi1;
                midi1.add (PacketX1 { 0x24b46501 });
                midi1.add (PacketX1 { 0x24b46423 });
                midi1.add (PacketX1 { 0x24b40622 });
                midi1.add (PacketX1 { 0x24b42659 });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }

            {
                Packets midi2;
                midi2.add (PacketX2 { 0x48347f7f, 0xffffffff });

                Packets midi1;
                midi1.add (PacketX1 { 0x28b4637f });
                midi1.add (PacketX1 { 0x28b4627f });
                midi1.add (PacketX1 { 0x28b4067f });
                midi1.add (PacketX1 { 0x28b4267f });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }
        }

        beginTest ("MIDI 2 -> 1 program change and bank select conversion");
        {
            {
                // If the bank valid bit is 0, just emit a program change
                Packets midi2;
                midi2.add (PacketX2 { 0x4cc10000, 0x70004020 });

                Packets midi1;
                midi1.add (PacketX1 { 0x2cc17000 });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }

            {
                // If the bank valid bit is 1, emit bank select control changes and a program change
                Packets midi2;
                midi2.add (PacketX2 { 0x4bc20001, 0x70004020 });

                Packets midi1;
                midi1.add (PacketX1 { 0x2bb20040 });
                midi1.add (PacketX1 { 0x2bb22020 });
                midi1.add (PacketX1 { 0x2bc27000 });

                checkMidi2ToMidi1Conversion (midi2, midi1);
            }
        }

        beginTest ("MIDI 2 -> 1 pitch bend conversion");
        {
            Packets midi2;
            midi2.add (PacketX2 { 0x4eee0000, 0x12340000 });

            Packets midi1;
            midi1.add (PacketX1 { 0x2eee0d09 });

            checkMidi2ToMidi1Conversion (midi2, midi1);
        }

        beginTest ("MIDI 2 -> 1 messages which don't convert");
        {
            const std::byte opcodes[] { std::byte { 0x0 },
                                        std::byte { 0x1 },
                                        std::byte { 0x4 },
                                        std::byte { 0x5 },
                                        std::byte { 0x6 },
                                        std::byte { 0xf } };

            for (const auto opcode : opcodes)
            {
                Packets midi2;
                midi2.add (PacketX2 { Utils::bytesToWord (std::byte { 0x40 }, std::byte { opcode << 0x4 }, std::byte { 0 }, std::byte { 0 }), 0x0 });
                checkMidi2ToMidi1Conversion (midi2, {});
            }
        }

        beginTest ("MIDI 2 -> 1 messages which are passed through");
        {
            const uint8_t typecodesX1[] { 0x0, 0x1, 0x2 };

            for (const auto typecode : typecodesX1)
            {
                Packets p;
                p.add (PacketX1 { (uint32_t) ((int64_t) typecode << 0x1c | (random.nextInt64() & 0xffffff)) });

                checkMidi2ToMidi1Conversion (p, p);
            }

            {
                Packets p;
                p.add (PacketX2 { (uint32_t) (0x3 << 0x1c | (random.nextInt64() & 0xffffff)),
                                  (uint32_t) (random.nextInt64() & 0xffffffff) });

                checkMidi2ToMidi1Conversion (p, p);
            }

            {
                Packets p;
                p.add (PacketX4 { (uint32_t) (0x5 << 0x1c | (random.nextInt64() & 0xffffff)),
                                  (uint32_t) (random.nextInt64() & 0xffffffff),
                                  (uint32_t) (random.nextInt64() & 0xffffffff),
                                  (uint32_t) (random.nextInt64() & 0xffffffff) });

                checkMidi2ToMidi1Conversion (p, p);
            }
        }

        beginTest ("MIDI 2 -> 1 control changes which should be ignored");
        {
            const uint8_t CCs[] { 6, 38, 98, 99, 100, 101, 0, 32 };

            for (const auto cc : CCs)
            {
                Packets midi2;
                midi2.add (PacketX2 { (uint32_t) (0x40b00000 | (cc << 0x8)), 0x00000000 });

                checkMidi2ToMidi1Conversion (midi2, {});
            }
        }

        beginTest ("MIDI 1 -> 2 note on conversions");
        {
            {
                Packets midi1;
                midi1.add (PacketX1 { 0x20904040 });

                Packets midi2;
                midi2.add (PacketX2 { 0x40904000, static_cast<uint32_t> (Conversion::scaleTo16 (0x40_u8)) << 0x10 });

                checkMidi1ToMidi2Conversion (midi1, midi2);
            }

            // If velocity is 0, convert to a note-off
            {
                Packets midi1;
                midi1.add (PacketX1 { 0x23935100 });

                Packets midi2;
                midi2.add (PacketX2 { 0x43835100, 0x0 });

                checkMidi1ToMidi2Conversion (midi1, midi2);
            }
        }

        beginTest ("MIDI 1 -> 2 note off conversions");
        {
            Packets midi1;
            midi1.add (PacketX1 { 0x21831020 });

            Packets midi2;
            midi2.add (PacketX2 { 0x41831000, static_cast<uint32_t> (Conversion::scaleTo16 (0x20_u8)) << 0x10 });

            checkMidi1ToMidi2Conversion (midi1, midi2);
        }

        beginTest ("MIDI 1 -> 2 poly pressure conversions");
        {
            Packets midi1;
            midi1.add (PacketX1 { 0x20af7330 });

            Packets midi2;
            midi2.add (PacketX2 { 0x40af7300, Conversion::scaleTo32 (0x30_u8) });

            checkMidi1ToMidi2Conversion (midi1, midi2);
        }

        beginTest ("individual MIDI 1 -> 2 control changes which should be ignored");
        {
            const uint8_t CCs[] { 6, 38, 98, 99, 100, 101, 0, 32 };

            for (const auto cc : CCs)
            {
                Packets midi1;
                midi1.add (PacketX1 { Utils::bytesToWord (std::byte { 0x20 }, std::byte { 0xb0 }, std::byte { cc }, std::byte { 0x00 }) });

                checkMidi1ToMidi2Conversion (midi1, {});
            }
        }

        beginTest ("MIDI 1 -> 2 control change conversions");
        {
            // normal control change
            {
                Packets midi1;
                midi1.add (PacketX1 { 0x29b1017f });

                Packets midi2;
                midi2.add (PacketX2 { 0x49b10100, Conversion::scaleTo32 (0x7f_u8) });

                checkMidi1ToMidi2Conversion (midi1, midi2);
            }

            // nrpn
            {
                Packets midi1;
                midi1.add (PacketX1 { 0x20b06301 });
                midi1.add (PacketX1 { 0x20b06223 });
                midi1.add (PacketX1 { 0x20b00645 });
                midi1.add (PacketX1 { 0x20b02667 });

                Packets midi2;
                midi2.add (PacketX2 { 0x40300123, Conversion::scaleTo32 (static_cast<uint16_t> ((0x45 << 7) | 0x67)) });

                checkMidi1ToMidi2Conversion (midi1, midi2);
            }

            // rpn
            {
                Packets midi1;
                midi1.add (PacketX1 { 0x20b06543 });
                midi1.add (PacketX1 { 0x20b06421 });
                midi1.add (PacketX1 { 0x20b00601 });
                midi1.add (PacketX1 { 0x20b02623 });

                Packets midi2;
                midi2.add (PacketX2 { 0x40204321, Conversion::scaleTo32 (static_cast<uint16_t> ((0x01 << 7) | 0x23)) });

                checkMidi1ToMidi2Conversion (midi1, midi2);
            }
        }

        beginTest ("MIDI 1 -> MIDI 2 program change and bank select");
        {
            Packets midi1;
            // program change with bank
            midi1.add (PacketX1 { 0x2bb20030 });
            midi1.add (PacketX1 { 0x2bb22010 });
            midi1.add (PacketX1 { 0x2bc24000 });
            // program change without bank (different group and channel)
            midi1.add (PacketX1 { 0x20c01000 });

            Packets midi2;
            midi2.add (PacketX2 { 0x4bc20001, 0x40003010 });
            midi2.add (PacketX2 { 0x40c00000, 0x10000000 });

            checkMidi1ToMidi2Conversion (midi1, midi2);
        }

        beginTest ("MIDI 1 -> MIDI 2 channel pressure conversions");
        {
            Packets midi1;
            midi1.add (PacketX1 { 0x20df3000 });

            Packets midi2;
            midi2.add (PacketX2 { 0x40df0000, Conversion::scaleTo32 (0x30_u8) });

            checkMidi1ToMidi2Conversion (midi1, midi2);
        }

        beginTest ("MIDI 1 -> MIDI 2 pitch bend conversions");
        {
            Packets midi1;
            midi1.add (PacketX1 { 0x20e74567 });

            Packets midi2;
            midi2.add (PacketX2 { 0x40e70000, Conversion::scaleTo32 (static_cast<uint16_t> ((0x67 << 7) | 0x45)) });

            checkMidi1ToMidi2Conversion (midi1, midi2);
        }
    }

private:
    static Packets toMidi1 (const MidiMessage& msg)
    {
        Packets packets;
        Conversion::toMidi1 (ump::BytestreamMidiView (&msg), [&] (const auto p) { packets.add (p); });
        return packets;
    }

    static Packets convertMidi2ToMidi1 (const Packets& midi2)
    {
        Packets r;

        for (const auto& packet : midi2)
            Conversion::midi2ToMidi1DefaultTranslation (packet, [&r] (const View& v) { r.add (v); });

        return r;
    }

    static Packets convertMidi1ToMidi2 (const Packets& midi1)
    {
        Packets r;
        Midi1ToMidi2DefaultTranslator translator;

        for (const auto& packet : midi1)
            translator.dispatch (packet, [&r] (const View& v) { r.add (v); });

        return r;
    }

    void checkBytestreamConversion (const Packets& actual, const Packets& expected)
    {
        expectEquals ((int) actual.size(), (int) expected.size());

        if (actual.size() != expected.size())
            return;

        auto actualPtr = actual.data();

        std::for_each (expected.data(),
                       expected.data() + expected.size(),
                       [&] (const uint32_t word) { expectEquals ((uint64_t) *actualPtr++, (uint64_t) word); });
    }

    void checkMidi2ToMidi1Conversion (const Packets& midi2, const Packets& expected)
    {
        checkBytestreamConversion (convertMidi2ToMidi1 (midi2), expected);
    }

    void checkMidi1ToMidi2Conversion (const Packets& midi1, const Packets& expected)
    {
        checkBytestreamConversion (convertMidi1ToMidi2 (midi1), expected);
    }

    MidiMessage createRandomSysEx (Random& random, size_t sysExBytes)
    {
        std::vector<uint8_t> data;
        data.reserve (sysExBytes);

        for (size_t i = 0; i != sysExBytes; ++i)
            data.push_back (uint8_t (random.nextInt (0x80)));

        return MidiMessage::createSysExMessage (data.data(), int (data.size()));
    }

    PacketX1 createRandomUtilityUMP (Random& random)
    {
        const auto status = random.nextInt (3);

        return PacketX1 { Utils::bytesToWord (std::byte { 0 },
                                              std::byte (status << 0x4),
                                              std::byte (status == 0 ? 0 : random.nextInt (0x100)),
                                              std::byte (status == 0 ? 0 : random.nextInt (0x100))) };
    }

    PacketX1 createRandomRealtimeUMP (Random& random)
    {
        const auto status = [&]
        {
            switch (random.nextInt (6))
            {
                case 0: return std::byte { 0xf8 };
                case 1: return std::byte { 0xfa };
                case 2: return std::byte { 0xfb };
                case 3: return std::byte { 0xfc };
                case 4: return std::byte { 0xfe };
                case 5: return std::byte { 0xff };
            }

            jassertfalse;
            return std::byte { 0x00 };
        }();

        return PacketX1 { Utils::bytesToWord (std::byte { 0x10 }, status, std::byte { 0x00 }, std::byte { 0x00 }) };
    }

    template <typename Fn>
    void forEachNonSysExTestMessage (Random& random, Fn&& fn)
    {
        for (uint16_t counter = 0x80; counter != 0x100; ++counter)
        {
            const auto firstByte = (uint8_t) counter;

            if (firstByte == 0xf0 || firstByte == 0xf7)
                continue; // sysEx is tested separately

            const auto length = MidiMessage::getMessageLengthFromFirstByte (firstByte);
            const auto getDataByte = [&] { return uint8_t (random.nextInt (256) & 0x7f); };

            const auto message = [&]
            {
                switch (length)
                {
                    case 1: return MidiMessage (firstByte);
                    case 2: return MidiMessage (firstByte, getDataByte());
                    case 3: return MidiMessage (firstByte, getDataByte(), getDataByte());
                }

                return MidiMessage();
            }();

            fn (message);
        }
    }

    static bool equal (const MidiMessage& a, const MidiMessage& b) noexcept
    {
        return a.getRawDataSize() == b.getRawDataSize()
               && std::equal (a.getRawData(), a.getRawData() + a.getRawDataSize(), b.getRawData());
    }

    static bool equal (const MidiBuffer& a, const MidiBuffer& b) noexcept
    {
        return a.data == b.data;
    }
};

static UniversalMidiPacketTests universalMidiPacketTests;

} // namespace juce::universal_midi_packets
