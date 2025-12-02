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

class Input::Impl final : private Consumer,
                          private DisconnectionListener
{
public:
    class Native
    {
    public:
        virtual ~Native() = default;

        /*  Returns the ID of the endpoint to which this connection is connected. */
        virtual EndpointId getEndpointId() const = 0;

        /*  The protocol to which incoming messages are converted. */
        virtual PacketProtocol getProtocol() const = 0;
    };

    EndpointId getEndpointId() const
    {
        return identifier;
    }

    PacketProtocol getProtocol() const
    {
        return protocol;
    }

    void addConsumer (Consumer& x)
    {
        consumers.add (x);
    }

    void removeConsumer (Consumer& x)
    {
        consumers.remove (x);
    }

    void addDisconnectionListener (DisconnectionListener& x)
    {
        disconnectListeners.add (&x);
    }

    void removeDisconnectionListener (DisconnectionListener& x)
    {
        disconnectListeners.remove (&x);
    }

    bool isAlive() const
    {
        return native != nullptr;
    }

    template <typename Callback>
    static Input makeInput (Callback&& callback)
    {
        auto impl = rawToUniquePtr (new Impl);
        impl->native = callback (static_cast<DisconnectionListener&> (*impl), static_cast<Consumer&> (*impl));

        if (impl->native == nullptr)
            return {};

        impl->identifier = impl->native->getEndpointId();
        impl->protocol = impl->native->getProtocol();
        return Input { std::move (impl) };
    }

private:
    Impl() = default;

    void consume (Iterator b, Iterator e, double t) override
    {
        consumers.call ([&] (auto& l) { l.consume (b, e, t); });
    }

    void disconnected() override
    {
        JUCE_ASSERT_MESSAGE_THREAD

        native = nullptr;
        disconnectListeners.call ([] (auto& x) { x.disconnected(); });
    }

    ListenerList<DisconnectionListener> disconnectListeners;
    WaitFreeListeners<Consumer> consumers;
    EndpointId identifier;
    PacketProtocol protocol;
    std::unique_ptr<Native> native;
};

Input::Input() = default;
Input::~Input() = default;
Input::Input (std::unique_ptr<Impl> x) : impl (std::move (x)) {}
Input::Input (Input&&) noexcept = default;
Input& Input::operator= (Input&&) noexcept = default;

EndpointId Input::getEndpointId() const
{
    if (impl != nullptr)
        return impl->getEndpointId();

    return {};
}

PacketProtocol Input::getProtocol() const
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        return impl->getProtocol();

    return {};
}

void Input::addConsumer (Consumer& x)
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        impl->addConsumer (x);
}

void Input::removeConsumer (Consumer& x)
{
    if (impl != nullptr)
        impl->removeConsumer (x);
}

void Input::addDisconnectionListener (DisconnectionListener& x)
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        impl->addDisconnectionListener (x);
}

void Input::removeDisconnectionListener (DisconnectionListener& x)
{
    if (impl != nullptr)
        impl->removeDisconnectionListener (x);
}

bool Input::isAlive() const
{
    if (impl != nullptr)
        return impl->isAlive();

    return false;
}

}
