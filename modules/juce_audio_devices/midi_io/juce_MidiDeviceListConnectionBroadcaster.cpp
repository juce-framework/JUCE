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

/*  Identifies a single group of an endpoint. */
struct EndpointGroup
{
    ump::EndpointId endpointId;
    uint8_t group; // 0 to 15 inclusive

    bool operator== (const EndpointGroup& other) const
    {
        const auto tie = [] (auto& x) { return std::tuple (x.endpointId, x.group); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const EndpointGroup& other) const { return ! operator== (other); }
};

class MidiDeviceListConnectionBroadcaster : private AsyncUpdater,
                                            private ump::EndpointsListener
{
    struct GroupId
    {
        String identifier;
        ump::IOKind kind;

        auto tie() const { return std::tuple (identifier, kind); }
        bool operator== (const GroupId& other) const { return tie() == other.tie(); }
        bool operator!= (const GroupId& other) const { return tie() != other.tie(); }
        bool operator<  (const GroupId& other) const { return tie() <  other.tie(); }
    };

    class Endpoints
    {
        auto tie() const { return std::tie (idBuffer, map, inputs, outputs); }

    public:
        void refresh()
        {
            updateIdBuffer();
            map = computeMap();
            inputs = computeInfo (ump::IOKind::src);
            outputs = computeInfo (ump::IOKind::dst);
        }

        std::optional<EndpointGroup> getEndpointGroupForId (ump::IOKind direction, const String& id) const
        {
            JUCE_ASSERT_MESSAGE_THREAD

            const auto iter = map.find ({ id, direction });

            if (iter == map.end())
                return {};

            return iter->second;
        }

        void getAllMidiDeviceInfo (ump::IOKind direction, Array<MidiDeviceInfo>& buffer) const
        {
            JUCE_ASSERT_MESSAGE_THREAD
            buffer.addArray (direction == ump::IOKind::src ? inputs : outputs);
        }

        bool operator== (const Endpoints& other) const
        {
            return tie() == other.tie();
        }

        bool operator!= (const Endpoints& other) const
        {
            return ! operator== (other);
        }

    private:
        void updateIdBuffer()
        {
            idBuffer.clear();

            auto* ep = ump::Endpoints::getInstance();

            if (ep == nullptr)
                return;

            ep->getEndpoints (idBuffer);
        }

        std::map<GroupId, EndpointGroup> computeMap() const
        {
            auto* ep = ump::Endpoints::getInstance();

            if (ep == nullptr)
                return {};

            std::map<GroupId, EndpointGroup> result;

            for (const auto& id : idBuffer)
            {
                const auto info = ep->getStaticDeviceInfo (id);

                if (! info.has_value())
                    continue;

                for (const auto kind : ump::ioKinds)
                {
                    const auto groups = info->getLegacyIdentifiers (kind);

                    for (const auto [index, groupId] : enumerate (groups, uint8_t{}))
                    {
                        if (groupId.isNotEmpty())
                            result.emplace (GroupId { groupId, kind }, EndpointGroup { id, index });
                    }
                }
            }

            return result;
        }

        Array<MidiDeviceInfo> computeInfo (ump::IOKind dir) const
        {
            auto* ep = ump::Endpoints::getInstance();

            if (ep == nullptr)
                return {};

            Array<MidiDeviceInfo> result;

            for (const auto& id : idBuffer)
            {
                const auto endpoint = ep->getEndpoint (id);

                if (! endpoint.has_value())
                    continue;

                const auto info = ep->getStaticDeviceInfo (id);

                if (! info.has_value())
                    continue;

                const auto groups = info->getLegacyIdentifiers (dir);

                for (const auto [groupIndex, groupId] : enumerate (groups, uint8_t{}))
                {
                    if (groupId.isEmpty())
                        continue;

                    const auto blockName = findNameForGroup (groupIndex, dir, endpoint->getBlocks());

                    if (! blockName.has_value())
                        continue;

                    const auto separator = endpoint->getName().isEmpty() || blockName->isEmpty() ? "" : " ";
                    const auto name = endpoint->getName() + separator + *blockName;

                    result.add (MidiDeviceInfo { name, groupId });
                }
            }

            return result;
        }

        std::vector<ump::EndpointId> idBuffer = std::invoke ([]() -> std::vector<ump::EndpointId>
        {
            auto* ep = ump::Endpoints::getInstance();

            if (ep == nullptr)
                return {};

            std::vector<ump::EndpointId> result;
            ep->getEndpoints (result);
            return result;
        });

        std::map<GroupId, EndpointGroup> map = computeMap();
        Array<MidiDeviceInfo> inputs = computeInfo (ump::IOKind::src), outputs = computeInfo (ump::IOKind::dst);
    };

public:
    ~MidiDeviceListConnectionBroadcaster() override
    {
        if (auto* instance = ump::Endpoints::getInstanceWithoutCreating())
            instance->removeListener (*this);

        cancelPendingUpdate();
    }

    MidiDeviceListConnection::Key add (std::function<void()> callback)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        return callbacks.emplace (key++, std::move (callback)).first->first;
    }

    void getAllMidiDeviceInfo (ump::IOKind direction, Array<MidiDeviceInfo>& buffer)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        if (! endpoints.has_value())
            endpoints = Endpoints{};

        endpoints->getAllMidiDeviceInfo (direction, buffer);
    }

    std::optional<MidiDeviceInfo> getInfoForId (ump::IOKind direction, const String& id)
    {
        if (const auto eg = getEndpointGroupForId (direction, id))
            return findInfoForDeviceIdentifier ({ id, direction }, *eg);

        return {};
    }

    std::optional<EndpointGroup> getEndpointGroupForId (ump::IOKind direction, const String& id)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        if (! endpoints.has_value())
            endpoints = Endpoints{};

        return endpoints->getEndpointGroupForId (direction, id);
    }

    void remove (const MidiDeviceListConnection::Key k)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        callbacks.erase (k);
    }

    static auto& get()
    {
        static MidiDeviceListConnectionBroadcaster result;
        return result;
    }

private:
    MidiDeviceListConnectionBroadcaster()
    {
        ump::Endpoints::getInstance()->addListener (*this);
    }

    static std::optional<String> findNameForGroup (uint8_t group, ump::IOKind kind, Span<const ump::Block> blocks)
    {
        const auto matchingGroup = std::find_if (blocks.begin(), blocks.end(), [&] (const ump::Block& b)
        {
            return directionsMatch (b, kind) && group == b.getFirstGroup() && b.getNumGroups() == 1;
        });

        if (matchingGroup != blocks.end())
            return matchingGroup->getName();

        return {};
    }

    void endpointsChanged() override
    {
        auto* mm = MessageManager::getInstanceWithoutCreating();

        if (mm == nullptr)
            return;

        if (! mm->isThisTheMessageThread())
        {
            triggerAsyncUpdate();
            return;
        }

        cancelPendingUpdate();

        if (auto prev = std::exchange (endpoints, Endpoints{}); prev != endpoints)
            for (auto it = callbacks.begin(); it != callbacks.end();)
                NullCheckedInvocation::invoke ((it++)->second);
    }

    void handleAsyncUpdate() override
    {
        endpointsChanged();
    }

    static bool directionsMatch (ump::BlockDirection d, ump::IOKind k)
    {
        switch (d)
        {
            case ump::BlockDirection::unknown:       return true;
            case ump::BlockDirection::bidirectional: return true;
            case ump::BlockDirection::sender:        return k == ump::IOKind::src;
            case ump::BlockDirection::receiver:      return k == ump::IOKind::dst;
        }

        jassertfalse;
        return false;
    }

    static bool directionsMatch (const ump::Block& b, ump::IOKind k)
    {
        return directionsMatch (b.getDirection(), k);
    }

    static std::optional<MidiDeviceInfo> findInfoForDeviceIdentifier (const GroupId& key, const EndpointGroup& value)
    {
        auto* endpoints = ump::Endpoints::getInstance();

        if (endpoints == nullptr)
            return {};

        const auto endpoint = endpoints->getEndpoint (value.endpointId);

        if (! endpoint.has_value())
            return {};

        const auto blockName = findNameForGroup (value.group, key.kind, endpoint->getBlocks());

        if (! blockName.has_value())
            return {};

        const auto separator = endpoint->getName().isEmpty() || blockName->isEmpty() ? "" : " : ";
        const auto name = endpoint->getName() + separator + *blockName;

        return MidiDeviceInfo { name, key.identifier };
    }

    std::optional<Endpoints> endpoints;
    std::map<MidiDeviceListConnection::Key, std::function<void()>> callbacks;
    MidiDeviceListConnection::Key key = 0;
};

} // namespace juce
