/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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

namespace juce
{
namespace universal_midi_packets
{
    struct ToUMP1Converter
    {
        template <typename Fn>
        void convert (const MidiMessage& m, Fn&& fn)
        {
            Conversion::toMidi1 (m, std::forward<Fn> (fn));
        }

        template <typename Fn>
        void convert (const View& v, Fn&& fn)
        {
            Conversion::midi2ToMidi1DefaultTranslation (v, std::forward<Fn> (fn));
        }
    };

    struct ToUMP2Converter
    {
        template <typename Fn>
        void convert (const MidiMessage& m, Fn&& fn)
        {
            Conversion::toMidi1 (m, [&] (const View& v)
            {
                translator.dispatch (v, fn);
            });
        }

        template <typename Fn>
        void convert (const View& v, Fn&& fn)
        {
            translator.dispatch (v, std::forward<Fn> (fn));
        }

        void reset()
        {
            translator.reset();
        }

        Midi1ToMidi2DefaultTranslator translator;
    };

    class GenericUMPConverter
    {
    public:
        explicit GenericUMPConverter (PacketProtocol m)
            : mode (m) {}

        void reset()
        {
            std::get<1> (converters).reset();
        }

        template <typename Fn>
        void convert (const MidiMessage& m, Fn&& fn)
        {
            switch (mode)
            {
                case PacketProtocol::MIDI_1_0: return std::get<0> (converters).convert (m, std::forward<Fn> (fn));
                case PacketProtocol::MIDI_2_0: return std::get<1> (converters).convert (m, std::forward<Fn> (fn));
            }
        }

        template <typename Fn>
        void convert (const View& v, Fn&& fn)
        {
            switch (mode)
            {
                case PacketProtocol::MIDI_1_0: return std::get<0> (converters).convert (v, std::forward<Fn> (fn));
                case PacketProtocol::MIDI_2_0: return std::get<1> (converters).convert (v, std::forward<Fn> (fn));
            }
        }

        template <typename Fn>
        void convert (Iterator begin, Iterator end, Fn&& fn)
        {
            std::for_each (begin, end, [&] (const View& v)
            {
                convert (v, fn);
            });
        }

        PacketProtocol getProtocol() const noexcept { return mode; }

    private:
        std::tuple<ToUMP1Converter, ToUMP2Converter> converters;
        const PacketProtocol mode{};
    };

    struct ToBytestreamConverter
    {
        explicit ToBytestreamConverter (int storageSize)
            : translator (storageSize) {}

        template <typename Fn>
        void convert (const MidiMessage& m, Fn&& fn)
        {
            fn (m);
        }

        template <typename Fn>
        void convert (const View& v, double time, Fn&& fn)
        {
            Conversion::midi2ToMidi1DefaultTranslation (v, [&] (const View& midi1)
            {
                translator.dispatch (midi1, time, fn);
            });
        }

        void reset() { translator.reset(); }

        Midi1ToBytestreamTranslator translator;
    };
}
}
