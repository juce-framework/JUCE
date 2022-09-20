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
    A base class for classes which convert Universal MIDI Packets to other
    formats.

    @tags{Audio}
*/
struct U32InputHandler
{
    virtual ~U32InputHandler() noexcept = default;

    virtual void reset() = 0;
    virtual void pushMidiData (const uint32_t* begin, const uint32_t* end, double time) = 0;
};

/**
    Parses a continuous stream of U32 words and emits complete MidiMessages whenever a full
    message is received.

    @tags{Audio}
*/
struct U32ToBytestreamHandler : public U32InputHandler
{
    U32ToBytestreamHandler (MidiInput& i, MidiInputCallback& c)
        : input (i), callback (c), dispatcher (2048) {}

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

        std::unique_ptr<U32ToBytestreamHandler> operator() (MidiInput& i) const
        {
            if (callback != nullptr)
                return std::make_unique<U32ToBytestreamHandler> (i, *callback);

            jassertfalse;
            return {};
        }

    private:
        MidiInputCallback* callback = nullptr;
    };

    void reset() override { dispatcher.reset(); }

    void pushMidiData (const uint32_t* begin, const uint32_t* end, double time) override
    {
        dispatcher.dispatch (begin, end, time, [this] (const MidiMessage& m)
        {
            callback.handleIncomingMidiMessage (&input, m);
        });
    }

    MidiInput& input;
    MidiInputCallback& callback;
    ToBytestreamDispatcher dispatcher;
};

/**
    Parses a continuous stream of U32 words and emits full messages in the requested
    UMP format.

    @tags{Audio}
*/
struct U32ToUMPHandler : public U32InputHandler
{
    U32ToUMPHandler (PacketProtocol protocol, Receiver& c)
        : recipient (c), converter (protocol) {}

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

        std::unique_ptr<U32ToUMPHandler> operator() (MidiInput&) const
        {
            return std::make_unique<U32ToUMPHandler> (protocol, callback);
        }

    private:
        PacketProtocol protocol;
        Receiver& callback;
    };

    void reset() override
    {
        dispatcher.reset();
        converter.reset();
    }

    void pushMidiData (const uint32_t* begin, const uint32_t* end, double time) override
    {
        dispatcher.dispatch (begin, end, time, [this] (const View& view, double thisTime)
        {
            converter.convert (view, [&] (const View& converted)
            {
                recipient.packetReceived (converted, thisTime);
            });
        });
    }

    Receiver& recipient;
    Dispatcher dispatcher;
    GenericUMPConverter converter;
};

}
}

#endif
