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

#ifndef DOXYGEN

namespace juce::universal_midi_packets
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
        dispatcher.dispatch (begin, end, time, [this] (const BytestreamMidiView& m)
        {
            callback.handleIncomingMidiMessage (&input, m.getMessage());
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

} // namespace juce::universal_midi_packets


#endif
