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
    Parses a raw stream of uint32_t, and calls a user-provided callback every time
    a full Universal MIDI Packet is encountered.

    @tags{Audio}
*/
class Dispatcher
{
public:
    /** Clears the dispatcher. */
    void reset() { currentPacketLen = 0; }

    /** Calls `callback` with a View of each packet encountered in the range delimited
        by `begin` and `end`.

        If the range ends part-way through a packet, the next call to `dispatch` will
        continue from that point in the packet (unless `reset` is called first).
    */
    template <typename PacketCallbackFunction>
    void dispatch (Span<const uint32_t> words,
                   double timeStamp,
                   PacketCallbackFunction&& callback)
    {
        for (const auto word : words)
        {
            nextPacket[currentPacketLen++] = word;

            if (currentPacketLen == Utils::getNumWordsForMessageType (nextPacket.front()))
            {
                callback (View (nextPacket.data()), timeStamp);
                currentPacketLen = 0;
            }
        }
    }

private:
    std::array<uint32_t, 4> nextPacket;
    size_t currentPacketLen = 0;
};

//==============================================================================
/**
    Parses a stream of bytes representing a sequence of bytestream-encoded MIDI 1.0 messages,
    converting the messages to UMP format and passing the packets to a user-provided callback
    as they become ready.

    @tags{Audio}
*/
class BytestreamToUMPDispatcher
{
public:
    /** Initialises the dispatcher.

        Channel messages will be converted to the requested protocol format `pp`.
        `storageSize` bytes will be allocated to store incomplete messages.
    */
    BytestreamToUMPDispatcher (uint8_t targetGroup, PacketProtocol pp, int storageSize)
        : concatenator (storageSize),
          converter (pp),
          group (targetGroup)
    {}

    void reset()
    {
        concatenator.reset();
        converter.reset();
    }

    /** Calls `callback` with a View of each converted packet as it becomes ready.

        @param bytes        range of bytes representing bytestream-encoded MIDI messages.
        @param timestamp    a timestamp to apply to the created packets.
        @param callback     a callback which will be passed a View pointing to each new packet as it becomes ready.
    */
    template <typename PacketCallbackFunction>
    void dispatch (Span<const std::byte> bytes,
                   double timestamp,
                   PacketCallbackFunction&& callback)
    {
        using CallbackPtr = decltype (std::addressof (callback));

        struct Callback
        {
            Callback (BytestreamToUMPDispatcher& d, CallbackPtr c)
                : dispatch (d), callbackPtr (c) {}

            void handleIncomingMidiMessage (void*, const MidiMessage& msg) const
            {
                Conversion::toMidi1 ({ dispatch.group, msg.asSpan() }, [&] (const View& view)
                {
                    dispatch.converter.convert (view, [&] (const View& v)
                    {
                        (*callbackPtr) (v, msg.getTimeStamp());
                    });
                });
            }

            void handlePartialSysexMessage (void*, const uint8*, int, double) const {}

            BytestreamToUMPDispatcher& dispatch;
            CallbackPtr callbackPtr = nullptr;
        };

        Callback inputCallback { *this, &callback };
        concatenator.pushMidiData (bytes, timestamp, (void*) nullptr, inputCallback);
    }

    PacketProtocol getProtocol() const
    {
        return converter.getProtocol();
    }

private:
    MidiDataConcatenator concatenator;
    GenericUMPConverter converter;
    uint8_t group{};
};

//==============================================================================
/**
    Parses a stream of 32-bit words representing a sequence of UMP-encoded MIDI messages,
    converting the messages to MIDI 1.0 bytestream format and passing them to a user-provided
    callback as they become ready.

    @tags{Audio}
*/
class ToBytestreamDispatcher
{
public:
    /** Initialises the dispatcher.

        `storageSize` bytes will be allocated to store incomplete messages.
    */
    explicit ToBytestreamDispatcher (int storageSize)
        : converter (storageSize) {}

    /** Clears the dispatcher. */
    void reset()
    {
        dispatcher.reset();
        converter.reset();
    }

    /** Calls `callback` with converted bytestream-formatted MidiMessage whenever
        a new message becomes available.

        @param begin        the first word in a stream of words representing UMP-encoded MIDI packets.
        @param end          one-past the last word in a stream of words representing UMP-encoded MIDI packets.
        @param timestamp    a timestamp to apply to converted messages.
        @param callback     a callback which will be passed a MidiMessage each time a new message becomes ready.
    */
    template <typename BytestreamMessageCallback>
    void dispatch (Span<const uint32_t> words,
                   double timestamp,
                   BytestreamMessageCallback&& callback)
    {
        dispatcher.dispatch (words, timestamp, [&] (const View& view, double time)
        {
            converter.convert (view, time, callback);
        });
    }

private:
    Dispatcher dispatcher;
    ToBytestreamConverter converter;
};

} // namespace juce::universal_midi_packets
/** @endcond */
