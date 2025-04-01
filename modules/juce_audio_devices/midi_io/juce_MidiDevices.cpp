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

namespace juce
{

MidiDeviceListConnection MidiDeviceListConnection::make (std::function<void()> callback)
{
    auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();

    const auto key = broadcaster.add (std::move (callback));

    MidiDeviceListConnection result;
    result.token = ErasedScopeGuard { [&broadcaster, key] { broadcaster.remove (key); } };
    return result;
}

//==============================================================================
static std::shared_ptr<ump::Session> getLegacySession()
{
    static std::weak_ptr<ump::Session> weak;

    if (auto strong = weak.lock())
        return strong;

    if (auto session = ump::Endpoints::getInstance()->makeSession (ump::Endpoints::Impl::getGlobalMidiClientName()))
    {
        auto strong = std::make_shared<ump::Session> (std::move (session));
        weak = strong;
        return strong;
    }

    return nullptr;
}

class MidiInput::Impl : private ump::Consumer
{
public:
    void start()
    {
        const SpinLock::ScopedLockType lock { spinLock };
        active = true;
    }

    void stop()
    {
        const SpinLock::ScopedLockType lock { spinLock };
        active = false;
    }

    MidiDeviceInfo getDeviceInfo() const noexcept
    {
        return customName.has_value() ? storedInfo.withName (*customName) : storedInfo;
    }

    void setName (String x)
    {
        customName = std::move (x);
    }

    void addCallback (MidiInputCallback& cb)
    {
        callbacks.add (cb);
    }

    void removeCallback (MidiInputCallback& cb)
    {
        callbacks.remove (cb);
    }

    /*  session may be null, in which case it's up to the caller to ensure that the session lives
        long enough for the connection to be useful.
    */
    static std::unique_ptr<MidiInput> make (std::shared_ptr<ump::Session> session,
                                            ump::Input connection,
                                            uint8_t group,
                                            const MidiDeviceInfo& info,
                                            MidiInputCallback* cb,
                                            ump::LegacyVirtualInput virtualEndpoint)
    {
        auto result = rawToUniquePtr (new MidiInput);
        result->pimpl = rawToUniquePtr (new Impl (session,
                                                  std::move (connection),
                                                  group,
                                                  result.get(),
                                                  info,
                                                  std::move (virtualEndpoint)));

        if (cb != nullptr)
            result->addCallback (*cb);

        return result;
    }

    uint8_t getGroup() const
    {
        return group;
    }

    ump::EndpointId getEndpointId() const
    {
        return connection.getEndpointId();
    }

    ~Impl() override
    {
        connection.removeConsumer (*this);
    }

private:
    Impl (std::shared_ptr<ump::Session> s,
          ump::Input x,
          uint8_t g,
          MidiInput* o,
          MidiDeviceInfo i,
          ump::LegacyVirtualInput v)
        : session (s),
          virtualEndpoint (std::move (v)),
          connection (std::move (x)),
          storedInfo (i),
          group (g),
          owner (o)
    {
        connection.addConsumer (*this);
    }

    void consume (ump::Iterator b, ump::Iterator e, double time) override
    {
        const SpinLock::ScopedTryLockType lock { spinLock };

        if (! lock.isLocked() || ! active)
            return;

        for (const auto& view : makeRange (b, e))
        {
            if (ump::Utils::getGroup (view[0]) != group)
                continue;

            converter.convert (view, time, [this] (ump::BytesOnGroup v, double t)
            {
                const MidiMessage msg { v.bytes.data(), (int) v.bytes.size(), t };

                callbacks.call ([&] (MidiInputCallback& l)
                {
                    l.handleIncomingMidiMessage (owner, msg);
                });
            });
        }
    }

    std::shared_ptr<ump::Session> session;
    ump::LegacyVirtualInput virtualEndpoint;
    std::optional<String> customName;
    ump::Input connection;
    MidiDeviceInfo storedInfo;
    ump::ToBytestreamConverter converter { 4096 };
    WaitFreeListeners<MidiInputCallback> callbacks;
    uint8_t group{};
    MidiInput* owner = nullptr;
    SpinLock spinLock;
    bool active = false;
};

MidiInput::MidiInput() = default;
MidiInput::~MidiInput() = default;

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    Array<MidiDeviceInfo> result;
    MidiDeviceListConnectionBroadcaster::get().getAllMidiDeviceInfo (ump::IOKind::src, result);
    return result;
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const String& deviceIdentifier, MidiInputCallback* callback)
{
    const auto address = MidiDeviceListConnectionBroadcaster::get().getEndpointGroupForId (ump::IOKind::src, deviceIdentifier);

    if (! address.has_value())
        return {};

    const auto info = MidiDeviceListConnectionBroadcaster::get().getInfoForId (ump::IOKind::src, deviceIdentifier);

    if (! info.has_value())
        return {};

    auto session = getLegacySession();

    if (session == nullptr)
        return {};

    auto connection = session->connectInput (address->endpointId, ump::PacketProtocol::MIDI_1_0);

    if (! connection.isAlive())
        return {};

    return Impl::make (session, std::move (connection), address->group, *info, callback, {});
}

static inline bool isValidMidi1VirtualEndpoint (const std::optional<ump::Endpoint>& ep,
                                                ump::BlockDirection dir)
{
    if (! ep.has_value())
        return false;

    if (! ep->hasMidi1Support())
        return false;

    if (ep->getProtocol() != ump::PacketProtocol::MIDI_1_0)
        return false;

    if (! ep->hasStaticBlocks())
        return false;

    auto blocks = ep->getBlocks();
    const auto iter = std::find_if (blocks.begin(), blocks.end(), [&] (const ump::Block& b)
    {
        return b.getDirection() == dir
               && b.getNumGroups() == 1
               && b.isEnabled()
               && b.getMIDI1ProxyKind() != ump::BlockMIDI1ProxyKind::inapplicable;
    });

    if (iter == blocks.end())
        return false;

    return true;
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const String& name, MidiInputCallback* callback)
{
    auto session = getLegacySession();

    if (! session)
        return {};

    auto port = session->createLegacyVirtualInput (name);

    if (! port)
        return {};

    jassert (isValidMidi1VirtualEndpoint (ump::Endpoints::getInstance()->getEndpoint (port.getId()),
                                          ump::BlockDirection::receiver));

    auto connection = session->connectInput (port.getId(), ump::PacketProtocol::MIDI_1_0);

    if (! connection)
        return {};

    return Impl::make (session, std::move (connection), 0, { name, {} }, callback, std::move (port));
}

void MidiInput::start()
{
    pimpl->start();
}

void MidiInput::stop()
{
    pimpl->stop();
}

MidiDeviceInfo MidiInput::getDeviceInfo() const noexcept
{
    return pimpl->getDeviceInfo();
}

void MidiInput::setName (const String& newName) noexcept
{
    pimpl->setName (newName);
}

uint8_t MidiInput::getGroup() const
{
    return pimpl->getGroup();
}

ump::EndpointId MidiInput::getEndpointId() const
{
    return pimpl->getEndpointId();
}

void MidiInput::addCallback (MidiInputCallback& callback)
{
    pimpl->addCallback (callback);
}

void MidiInput::removeCallback (MidiInputCallback& callback)
{
    pimpl->removeCallback (callback);
}

//==============================================================================
MidiOutput::MidiOutput (std::shared_ptr<ump::Session> s,
                        ump::Output x,
                        uint8_t g,
                        const MidiDeviceInfo& i,
                        ump::LegacyVirtualOutput v)
    : session (s),
      virtualEndpoint (std::move (v)),
      connection (std::move (x)),
      storedInfo (i),
      group (g)
{
}

Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()
{
    Array<MidiDeviceInfo> result;
    MidiDeviceListConnectionBroadcaster::get().getAllMidiDeviceInfo (ump::IOKind::dst, result);
    return result;
}

std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String& deviceIdentifier)
{
    const auto address = MidiDeviceListConnectionBroadcaster::get().getEndpointGroupForId (ump::IOKind::dst, deviceIdentifier);

    if (! address.has_value())
        return {};

    const auto info = MidiDeviceListConnectionBroadcaster::get().getInfoForId (ump::IOKind::dst, deviceIdentifier);

    if (! info.has_value())
        return {};

    auto session = getLegacySession();

    if (session == nullptr)
        return {};

    auto connection = session->connectOutput (address->endpointId);

    if (! connection.isAlive())
        return {};

    return rawToUniquePtr (new MidiOutput (session, std::move (connection), address->group, *info, {}));
}

std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const String& name)
{
    auto session = getLegacySession();

    if (! session)
        return {};

    auto port = session->createLegacyVirtualOutput (name);

    if (! port)
        return {};

    jassert (isValidMidi1VirtualEndpoint (ump::Endpoints::getInstance()->getEndpoint (port.getId()),
                                          ump::BlockDirection::sender));

    auto connection = session->connectOutput (port.getId());

    if (! connection)
        return {};

    return rawToUniquePtr (new MidiOutput (session, std::move (connection), 0, { name, {} }, std::move (port)));
}

MidiDeviceInfo MidiOutput::getDeviceInfo() const noexcept
{
    return customName.has_value() ? storedInfo.withName (*customName) : storedInfo;
}

} // namespace juce
