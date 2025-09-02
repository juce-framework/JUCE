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

/** @cond */
namespace juce::universal_midi_packets
{
    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to MIDI 1.0 messages in UMP format.

        @tags{Audio}
    */
    struct ToUMP1Converter
    {
        template <typename Fn>
        void convert (const BytestreamMidiView& m, Fn&& fn)
        {
            Conversion::toMidi1 (m, std::forward<Fn> (fn));
        }

        template <typename Fn>
        void convert (const View& v, Fn&& fn)
        {
            Conversion::midi2ToMidi1DefaultTranslation (v, std::forward<Fn> (fn));
        }
    };

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to MIDI 2.0 messages in UMP format.

        @tags{Audio}
    */
    struct ToUMP2Converter
    {
        template <typename Fn>
        void convert (const BytestreamMidiView& m, Fn&& fn)
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

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to UMP format.

        The packet protocol can be selected using the constructor parameter.

        @tags{Audio}
    */
    class GenericUMPConverter
    {
        template <typename This, typename... Args>
        static void visit (This& t, Args&&... args)
        {
            if (t.mode == PacketProtocol::MIDI_1_0)
                convertImpl (std::get<0> (t.converters), std::forward<Args> (args)...);
            else
                convertImpl (std::get<1> (t.converters), std::forward<Args> (args)...);
        }

    public:
        explicit GenericUMPConverter (PacketProtocol m)
            : mode (m) {}

        void reset()
        {
            std::get<1> (converters).reset();
        }

        template <typename Converter, typename Fn>
        static void convertImpl (Converter& converter, const BytestreamMidiView& m, Fn&& fn)
        {
            converter.convert (m, std::forward<Fn> (fn));
        }

        template <typename Converter, typename Fn>
        static void convertImpl (Converter& converter, const View& m, Fn&& fn)
        {
            converter.convert (m, std::forward<Fn> (fn));
        }

        template <typename Converter, typename Fn>
        static void convertImpl (Converter& converter, Iterator b, Iterator e, Fn&& fn)
        {
            std::for_each (b, e, [&] (const auto& v)
            {
                convertImpl (converter, v, fn);
            });
        }

        template <typename Fn>
        void convert (const BytestreamMidiView& m, Fn&& fn)
        {
            visit (*this, m, std::forward<Fn> (fn));
        }

        template <typename Fn>
        void convert (const View& v, Fn&& fn)
        {
            visit (*this, v, std::forward<Fn> (fn));
        }

        template <typename Fn>
        void convert (Iterator begin, Iterator end, Fn&& fn)
        {
            visit (*this, begin, end, std::forward<Fn> (fn));
        }

        PacketProtocol getProtocol() const noexcept { return mode; }

    private:
        std::tuple<ToUMP1Converter, ToUMP2Converter> converters;
        const PacketProtocol mode{};
    };

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to bytestream format.

        @tags{Audio}
    */
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
} // namespace juce::universal_midi_packets
/** @endcond */
