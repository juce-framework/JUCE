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

class Endpoints::Impl : private EndpointsListener
{
public:
    class Native
    {
    public:
        virtual ~Native() = default;
        virtual Backend getBackend() const = 0;
        virtual bool isVirtualMidiBytestreamServiceActive() const = 0;
        virtual bool isVirtualMidiUmpServiceActive() const = 0;
        virtual void setVirtualMidiBytestreamServiceActive (bool) {}
        virtual void setVirtualMidiUmpServiceActive (bool) {}
        virtual void getEndpoints (std::vector<EndpointId>&) const = 0;
        virtual std::optional<Endpoint> getEndpoint (const EndpointId&) const = 0;
        virtual std::optional<StaticDeviceInfo> getStaticDeviceInfo (const EndpointId&) const = 0;
        virtual std::unique_ptr<Session::Impl::Native> makeSession (const String&) = 0;

        static std::unique_ptr<Native> make (EndpointsListener&);
    };

    Backend getBackend() const
    {
        return native->getBackend();
    }

    bool isVirtualMidiBytestreamServiceActive() const
    {
        return native->isVirtualMidiBytestreamServiceActive();
    }

    bool isVirtualMidiUmpServiceActive() const
    {
        return native->isVirtualMidiUmpServiceActive();
    }

    void setVirtualMidiBytestreamServiceActive (bool x)
    {
        native->setVirtualMidiBytestreamServiceActive (x);
    }

    void setVirtualMidiUmpServiceActive (bool x)
    {
        native->setVirtualMidiUmpServiceActive (x);
    }

    void getEndpoints (std::vector<EndpointId>& x) const
    {
        native->getEndpoints (x);
    }

    std::optional<Endpoint> getEndpoint (const EndpointId& x) const
    {
        return native->getEndpoint (x);
    }

    std::optional<StaticDeviceInfo> getStaticDeviceInfo (const EndpointId& x) const
    {
        return native->getStaticDeviceInfo (x);
    }

    std::unique_ptr<Session::Impl::Native> makeSession (const String& x)
    {
        return native->makeSession (x);
    }

    void addListener (EndpointsListener& x)
    {
        listeners.add (&x);
    }

    void removeListener (EndpointsListener& x)
    {
        listeners.remove (&x);
    }

    static std::unique_ptr<Impl> make()
    {
        auto result = rawToUniquePtr (new Impl);
        result->native = Native::make (*result);

        if (result->native == nullptr)
            return {};

        return result;
    }

    static String getGlobalMidiClientName()
    {
        if (auto* app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();

        return "JUCE";
    }

private:
    void endpointsChanged() override
    {
        listeners.call ([] (auto& l) { l.endpointsChanged(); });
    }

    void virtualMidiServiceActiveChanged() override
    {
        listeners.call ([] (auto& l) { l.virtualMidiServiceActiveChanged(); });
    }

    Impl() = default;

    ListenerList<EndpointsListener> listeners;
    std::unique_ptr<Native> native;
};

void Endpoints::getEndpoints (std::vector<EndpointId>& x) const
{
    x.clear();

    if (impl != nullptr)
        impl->getEndpoints (x);
}

std::vector<EndpointId> Endpoints::getEndpoints() const
{
    std::vector<EndpointId> result;
    getEndpoints (result);
    return result;
}

std::optional<Endpoint> Endpoints::getEndpoint (const EndpointId& x) const
{
    if (impl != nullptr)
        return impl->getEndpoint (x);

    return {};
}

std::optional<StaticDeviceInfo> Endpoints::getStaticDeviceInfo (const EndpointId& x) const
{
    if (impl != nullptr)
        return impl->getStaticDeviceInfo (x);

    return {};
}

void Endpoints::addListener (EndpointsListener& x)
{
    if (impl != nullptr)
        impl->addListener (x);
}

void Endpoints::removeListener (EndpointsListener& x)
{
    if (impl != nullptr)
        impl->addListener (x);
}

Session Endpoints::makeSession (const String& x) const
{
    return Session::Impl::makeSession (impl != nullptr ? impl->makeSession (x) : nullptr);
}

std::optional<Backend> Endpoints::getBackend() const
{
    if (impl != nullptr)
        return impl->getBackend();

    return {};
}

bool Endpoints::isVirtualMidiBytestreamServiceActive() const
{
    if (impl != nullptr)
        return impl->isVirtualMidiBytestreamServiceActive();

    return false;
}

bool Endpoints::isVirtualMidiUmpServiceActive() const
{
    if (impl != nullptr)
        return impl->isVirtualMidiUmpServiceActive();

    return false;
}

void Endpoints::setVirtualMidiBytestreamServiceActive (bool x)
{
    if (impl != nullptr)
        impl->setVirtualMidiBytestreamServiceActive (x);
}

void Endpoints::setVirtualMidiUmpServiceActive (bool x)
{
    if (impl != nullptr)
        impl->setVirtualMidiUmpServiceActive (x);
}

Endpoints::Endpoints()
    : impl (Impl::make())
{
}

Endpoints::~Endpoints()
{
    clearSingletonInstance();
}

}
