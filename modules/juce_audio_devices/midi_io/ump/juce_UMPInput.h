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

namespace juce::universal_midi_packets
{

//==============================================================================
/**
    An interface class for entities that consume Universal MIDI Packets from some producer.

    @tags{Audio}
*/
struct Consumer
{
    Consumer() = default;

    Consumer (const Consumer&) = default;
    Consumer (Consumer&&) noexcept = default;

    Consumer& operator= (const Consumer&) = default;
    Consumer& operator= (Consumer&&) noexcept = default;

    virtual ~Consumer() noexcept = default;

    /** This will be called each time a new packet is ready for processing. */
    virtual void consume (Iterator b, Iterator e, double time) = 0;
};

//==============================================================================
/**
    An input (from the JUCE project's perspective) that receives messages sent by an endpoint.

    An Input is conceptually similar to a unique_ptr, in that it's a nullable move-only type.
    You can check the null state of an instance by calling isAlive().
    isAlive() will return true for an Input that's currently connected, or false otherwise.
    In particular, isAlive() will return false for a default-constructed Input.
    If isAlive() returns false, you should avoid calling other member functions:
    although this won't result in undefined behaviour, none of the functions will produce useful
    results in this state.

    In the case that the device connected to the Input becomes unavailable (e.g. it is unplugged or
    the bluetooth connection is dropped), the Input will null itself, and calls to isAlive() will
    return false. You can register a callback to handle this event by calling
    addDisconnectionListener().

    A particular pitfall to watch out for is calling addConsumer(), removeConsumer(),
    addDisconnectionListener(), and removeDisconnectionListener() on a default-constructed Input
    or other Input for which isAlive() returns false. This will have no effect. Instead, if you want
    to attach listeners to an Input, you should use Session::connectInput() to create an Input,
    and ensure that isAlive() returns true on that Input before attaching listeners.

    @tags{Audio}
*/
class Input
{
public:
    /** Creates a disconnected input.
        A default-constructed input will never receive any messages.
    */
    Input();
    ~Input();

    Input (Input&&) noexcept;
    Input& operator= (Input&&) noexcept;

    Input (const Input&) = delete;
    Input& operator= (const Input&) = delete;

    /** Returns this connection's endpoint. */
    EndpointId getEndpointId() const;

    /** Returns the protocol that was requested when creating this connection. */
    PacketProtocol getProtocol() const;

    /** Attaches a receiver that will receive MIDI messages from this input.

        Incoming messages will be converted to the protocol that was requested when
        opening the Input.

        If isAlive() returns false at the point where this function is called, this function
        will have no effect. This can commonly happen when attempting to add listeners to a
        default-constructed Input, or if the input device got disconnected.

        It is an error to add or remove a consumer from within the consumer callback.
        This will cause deadlocks, so be careful!
    */
    void addConsumer (Consumer& r);

    /** Detaches the receiver so that it will no longer receive MIDI messages from this
        Input.

        It is an error to add or remove a consumer from within the consumer callback.
        This will cause deadlocks, so be careful!
    */
    void removeConsumer (Consumer& r);

    /** Attaches a listener that will be notified when this endpoint is disconnected.

        Calling this function on an instance for which isAlive() returns false has no effect.
    */
    void addDisconnectionListener (DisconnectionListener& r);

    /** Removes a disconnection listener. */
    void removeDisconnectionListener (DisconnectionListener& r);

    /** True if this connection is currently active.

        This function returns false for a default-constructed instance.
    */
    bool isAlive() const;

    explicit operator bool() const { return isAlive(); }

    /** @internal */
    class Impl;

private:
    explicit Input (std::unique_ptr<Impl>);

    std::unique_ptr<Impl> impl;
};

}
