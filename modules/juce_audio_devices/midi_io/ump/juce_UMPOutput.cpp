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

class Output::Impl final : private DisconnectionListener
{
public:
    class Native
    {
    public:
        virtual ~Native() = default;

        virtual EndpointId getEndpointId() const = 0;

        virtual bool send (Iterator b, Iterator e) = 0;
    };

    EndpointId getEndpointId() const
    {
        return identifier;
    }

    bool send (Iterator beginIterator, Iterator endIterator)
    {
        if (native != nullptr)
            return native->send (beginIterator, endIterator);

        return false;
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
    static Output makeOutput (Callback&& callback)
    {
        auto impl = rawToUniquePtr (new Impl);
        impl->native = callback (static_cast<DisconnectionListener&> (*impl));

        if (impl->native == nullptr)
            return {};

        impl->identifier = impl->native->getEndpointId();
        return Output { std::move (impl) };
    }

private:
    Impl() = default;

    void disconnected() override
    {
        JUCE_ASSERT_MESSAGE_THREAD

        native = nullptr;
        disconnectListeners.call ([] (auto& x) { x.disconnected(); });
    }

    ListenerList<DisconnectionListener> disconnectListeners;
    std::unique_ptr<Native> native;
    EndpointId identifier;
};

Output::Output() = default;
Output::~Output() = default;
Output::Output (std::unique_ptr<Impl> x) : impl (std::move (x)) {}
Output::Output (Output&&) noexcept = default;
Output& Output::operator= (Output&&) noexcept = default;

EndpointId Output::getEndpointId() const
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        return impl->getEndpointId();

    return {};
}

bool Output::send (Iterator beginIterator, Iterator endIterator)
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        return impl->send (beginIterator, endIterator);

    return false;
}

void Output::addDisconnectionListener (DisconnectionListener& x)
{
    // You should ensure that isAlive() returns true before calling other member functions!
    jassert (isAlive());

    if (impl != nullptr)
        impl->addDisconnectionListener (x);
}

void Output::removeDisconnectionListener (DisconnectionListener& x)
{
    if (impl != nullptr)
        impl->removeDisconnectionListener (x);
}

bool Output::isAlive() const
{
    if (impl != nullptr)
        return impl->isAlive();

    return false;
}

}
