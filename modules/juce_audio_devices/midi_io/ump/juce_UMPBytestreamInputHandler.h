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
    A base class for classes which convert bytestream midi to other formats.

    @tags{Audio}
*/
struct BytestreamInputHandler
{
    virtual ~BytestreamInputHandler() noexcept = default;

    virtual void reset() = 0;
    virtual void pushMidiData (const void* data, int bytes, double time) = 0;
};

/**
    Parses a continuous bytestream and emits complete MidiMessages whenever a full
    message is received.

    @tags{Audio}
*/
struct BytestreamToBytestreamHandler : public BytestreamInputHandler
{
    BytestreamToBytestreamHandler (MidiInput& i, MidiInputCallback& c)
        : input (i), callback (c), concatenator (2048) {}

    /**
        Provides an `operator()` which can create an input handler for a given
        MidiInput.

        All handler classes should have a similar Factory to facilitate
        creation of handlers in generic contexts.
    */
    class Factory
    {
    public:
        explicit Factory (MidiInputCallback* c)
            : callback (c) {}

        std::unique_ptr<BytestreamToBytestreamHandler> operator() (MidiInput& i) const
        {
            if (callback != nullptr)
                return std::make_unique<BytestreamToBytestreamHandler> (i, *callback);

            jassertfalse;
            return {};
        }

    private:
        MidiInputCallback* callback = nullptr;
    };

    void reset() override { concatenator.reset(); }

    void pushMidiData (const void* data, int bytes, double time) override
    {
        concatenator.pushMidiData (data, bytes, time, &input, callback);
    }

    MidiInput& input;
    MidiInputCallback& callback;
    MidiDataConcatenator concatenator;
};

/**
    Parses a continuous MIDI 1.0 bytestream, and emits full messages in the requested
    UMP format.

    @tags{Audio}
*/
struct BytestreamToUMPHandler : public BytestreamInputHandler
{
    BytestreamToUMPHandler (PacketProtocol protocol, Receiver& c)
        : recipient (c), dispatcher (protocol, 2048) {}

    /**
        Provides an `operator()` which can create an input handler for a given
        MidiInput.

        All handler classes should have a similar Factory to facilitate
        creation of handlers in generic contexts.
    */
    class Factory
    {
    public:
        Factory (PacketProtocol p, Receiver& c)
            : protocol (p), callback (c) {}

        std::unique_ptr<BytestreamToUMPHandler> operator() (MidiInput&) const
        {
            return std::make_unique<BytestreamToUMPHandler> (protocol, callback);
        }

    private:
        PacketProtocol protocol;
        Receiver& callback;
    };

    void reset() override { dispatcher.reset(); }

    void pushMidiData (const void* data, int bytes, double time) override
    {
        const auto* ptr = static_cast<const uint8_t*> (data);
        dispatcher.dispatch (ptr, ptr + bytes, time, [&] (const View& v)
        {
            recipient.packetReceived (v, time);
        });
    }

    Receiver& recipient;
    BytestreamToUMPDispatcher dispatcher;
};

}
}

#endif
