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

#ifndef DOXYGEN

namespace juce
{
namespace universal_midi_packets
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
    void dispatch (const uint32_t* begin,
                   const uint32_t* end,
                   double timeStamp,
                   PacketCallbackFunction&& callback)
    {
        std::for_each (begin, end, [&] (uint32_t word)
        {
            nextPacket[currentPacketLen++] = word;

            if (currentPacketLen == Utils::getNumWordsForMessageType (nextPacket.front()))
            {
                callback (View (nextPacket.data()), timeStamp);
                currentPacketLen = 0;
            }
        });
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
    explicit BytestreamToUMPDispatcher (PacketProtocol pp, int storageSize)
        : concatenator (storageSize),
          converter (pp)
    {}

    void reset()
    {
        concatenator.reset();
        converter.reset();
    }

    /** Calls `callback` with a View of each converted packet as it becomes ready.

        @param begin        the first byte in a range of bytes representing bytestream-encoded MIDI messages.
        @param end          one-past the last byte in a range of bytes representing bytestream-encoded MIDI messages.
        @param timestamp    a timestamp to apply to the created packets.
        @param callback     a callback which will be passed a View pointing to each new packet as it becomes ready.
    */
    template <typename PacketCallbackFunction>
    void dispatch (const uint8_t* begin,
                   const uint8_t* end,
                   double timestamp,
                   PacketCallbackFunction&& callback)
    {
        using CallbackPtr = decltype (std::addressof (callback));

       #if JUCE_MINGW
        #define JUCE_MINGW_HIDDEN_VISIBILITY __attribute__ ((visibility ("hidden")))
       #else
        #define JUCE_MINGW_HIDDEN_VISIBILITY
       #endif

        struct JUCE_MINGW_HIDDEN_VISIBILITY Callback
        {
            Callback (BytestreamToUMPDispatcher& d, CallbackPtr c)
                : dispatch (d), callbackPtr (c) {}

            void handleIncomingMidiMessage (void*, const MidiMessage& msg) const
            {
                Conversion::toMidi1 (BytestreamMidiView (&msg), [&] (const View& view)
                {
                    dispatch.converter.convert (view, *callbackPtr);
                });
            }

            void handlePartialSysexMessage (void*, const uint8_t*, int, double) const {}

            BytestreamToUMPDispatcher& dispatch;
            CallbackPtr callbackPtr = nullptr;
        };

       #undef JUCE_MINGW_HIDDEN_VISIBILITY

        Callback inputCallback { *this, &callback };
        concatenator.pushMidiData (begin, int (end - begin), timestamp, (void*) nullptr, inputCallback);
    }

private:
    MidiDataConcatenator concatenator;
    GenericUMPConverter converter;
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
    void dispatch (const uint32_t* begin,
                   const uint32_t* end,
                   double timestamp,
                   BytestreamMessageCallback&& callback)
    {
        dispatcher.dispatch (begin, end, timestamp, [&] (const View& view, double time)
        {
            converter.convert (view, time, callback);
        });
    }

private:
    Dispatcher dispatcher;
    ToBytestreamConverter converter;
};

}
}

#endif
