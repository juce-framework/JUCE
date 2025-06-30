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

/**
    An output (from the JUCE project's perspective) that sends messages to an endpoint.

    An Output is conceptually similar to a unique_ptr, in that it's a nullable move-only type.
    You can check the null state of an instance by calling isAlive().
    isAlive() will return true for an Output that's currently connected, or false otherwise.
    In particular, isAlive() will return false for a default-constructed Output.
    If isAlive() returns false, you should avoid calling other member functions:
    although this won't result in undefined behaviour, none of the functions will produce useful
    results in this state.

    In the case that the device connected to the Output becomes unavailable (e.g. it is unplugged or
    the bluetooth connection is dropped), the Output will null itself, and calls to isAlive() will
    return false. You can register a callback to handle this event by calling
    addDisconnectionListener().

    A particular pitfall to watch out for is calling addDisconnectionListener() and
    removeDisconnectionListener() on a default-constructed Output or other Output for which
    isAlive() returns false. This will have no effect. Instead, if you want
    to attach listeners to an Output, you should use Session::connectOutput() to create an Output,
    and ensure that isAlive() returns true on that Output before attaching listeners.

    @tags{Audio}
*/
class Output
{
public:
    /** Creates a disconnected output.
        Sending messages to a default-constructed output won't do anything.
    */
    Output();
    ~Output();

    Output (Output&&) noexcept;
    Output& operator= (Output&&) noexcept;

    Output (const Output&) = delete;
    Output& operator= (const Output&) = delete;

    /** Returns this connection's endpoint. */
    EndpointId getEndpointId() const;

    /** Sends a range of messages to this endpoint.
        If isAlive() returns false at the point where this function is called, then this
        function has no effect.

        Returns true on success, false on failure.

        You may send messages using any protocol, and they will be converted automatically
        to the protocol expected by the receiver.
    */
    bool send (Iterator beginIterator, Iterator endIterator);

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
    explicit Output (std::unique_ptr<Impl>);

    std::unique_ptr<Impl> impl;
};

}
