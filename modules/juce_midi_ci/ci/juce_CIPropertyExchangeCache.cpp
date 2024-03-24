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

namespace juce::midi_ci
{

class PropertyExchangeCache
{
public:
    PropertyExchangeCache() = default;

    struct OwningResult
    {
        explicit OwningResult (PropertyExchangeResult::Error e)
            : result (e) {}

        OwningResult (var header, std::vector<std::byte> body)
            : backingStorage (std::move (body)),
              result (header, backingStorage) {}

        OwningResult (OwningResult&&) noexcept = default;
        OwningResult& operator= (OwningResult&&) noexcept = default;

        JUCE_DECLARE_NON_COPYABLE (OwningResult)

        std::vector<std::byte> backingStorage;
        PropertyExchangeResult result;
    };

    std::optional<OwningResult> addChunk (Message::DynamicSizePropertyExchange chunk)
    {
        jassert (chunk.thisChunkNum == lastChunk + 1 || chunk.thisChunkNum == 0);
        lastChunk = chunk.thisChunkNum;
        headerStorage.reserve (headerStorage.size() + chunk.header.size());
        std::transform (chunk.header.begin(),
                        chunk.header.end(),
                        std::back_inserter (headerStorage),
                        [] (std::byte b) { return char (b); });
        bodyStorage.insert (bodyStorage.end(), chunk.data.begin(), chunk.data.end());

        if (chunk.thisChunkNum != 0 && chunk.thisChunkNum != chunk.totalNumChunks)
            return {};

        const auto headerJson = JSON::parse (String (headerStorage.data(), headerStorage.size()));

        terminate();
        const auto encodingString = headerJson.getProperty ("mutualEncoding", "ASCII").toString();

        if (chunk.thisChunkNum != chunk.totalNumChunks)
            return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::partial };

        const int status = headerJson.getProperty ("status", 200);

        if (status == 343)
            return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::tooManyTransactions };

        return std::optional<OwningResult> { std::in_place,
                                             headerJson,
                                             Encodings::decode (bodyStorage, EncodingUtils::toEncoding (encodingString.toRawUTF8()).value_or (Encoding::ascii)) };
    }

    std::optional<OwningResult> notify (Span<const std::byte> header)
    {
        const auto headerJson = JSON::parse (String (reinterpret_cast<const char*> (header.data()), header.size()));

        if (! headerJson.isObject())
            return {};

        const auto status = headerJson.getProperty ("status", {});

        if (! status.isInt() || (int) status == 100)
            return {};

        terminate();
        return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::notify };
    }

    bool terminate()
    {
        return std::exchange (ongoing, false);
    }

private:
    std::vector<char> headerStorage;
    std::vector<std::byte> bodyStorage;
    uint16_t lastChunk = 0;
    bool ongoing = true;
};

//==============================================================================
class PropertyExchangeCacheArray
{
public:
    PropertyExchangeCacheArray() = default;

    Token64 primeCacheForRequestId (uint8_t id, std::function<void (const PropertyExchangeResult&)> onDone)
    {
        jassert (id < caches.size());

        ++lastKey;

        auto& entry = caches[id];

        if (entry.has_value())
        {
            // Trying to start a new message with the same id as another in-progress message
            jassertfalse;
            ids.erase (entry->key);
        }

        const auto& item = entry.emplace (id, std::move (onDone), Token64 { lastKey });
        ids.emplace (item.key, id);
        return item.key;
    }

    bool terminate (Token64 key)
    {
        const auto iter = ids.find (key);

        // If the key isn't found, then the transaction must have completed already
        if (iter == ids.end())
            return false;

        // We're about to terminate this transaction, so we don't need to retain this record
        auto index = iter->second;
        ids.erase (iter);

        auto& entry = caches[index];

        // If the entry is null, something's gone wrong. The ids map should only contain elements for
        // non-null cache entries.
        if (! entry.has_value())
        {
            jassertfalse;
            return false;
        }

        const auto result = entry->cache.terminate();
        entry.reset();
        return result;
    }

    void addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.addChunk (chunk); });
    }

    void notify (RequestID b, Span<const std::byte> header)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.notify (header); });
    }

    std::optional<Token64> getKeyForId (RequestID id) const
    {
        if (auto& c = caches[id.asInt()])
            return c->key;

        return {};
    }

    bool hasTransaction (RequestID id) const
    {
        return getKeyForId (id).has_value();
    }

    std::optional<RequestID> getIdForKey (Token64 key) const
    {
        const auto iter = ids.find (key);
        return iter != ids.end() ? RequestID::create (iter->second) : std::nullopt;
    }

    auto countOngoingTransactions() const
    {
        jassert (ids.size() == (size_t) std::count_if (caches.begin(), caches.end(), [] (auto& c) { return c.has_value(); }));

        return (int) ids.size();
    }

    auto getOngoingTransactions() const
    {
        jassert (ids.size() == (size_t) std::count_if (caches.begin(), caches.end(), [] (auto& c) { return c.has_value(); }));

        std::vector<Token64> result (ids.size());
        std::transform (ids.begin(), ids.end(), result.begin(), [] (const auto& p) { return Token64 { p.first }; });
        return result;
    }

    std::optional<RequestID> findUnusedId (uint8_t maxSimultaneousTransactions) const
    {
        if (countOngoingTransactions() >= maxSimultaneousTransactions)
            return {};

        return RequestID::create ((uint8_t) std::distance (caches.begin(), std::find (caches.begin(), caches.end(), std::nullopt)));
    }

    // Instances must stay at the same location to ensure that references captured in the
    // ErasedScopeGuard returned from primeCacheForRequestId do not dangle.
    JUCE_DECLARE_NON_COPYABLE (PropertyExchangeCacheArray)
    JUCE_DECLARE_NON_MOVEABLE (PropertyExchangeCacheArray)

private:
    static constexpr auto numCaches = 128;

    class Transaction
    {
    public:
        Transaction (uint8_t i, std::function<void (const PropertyExchangeResult&)> onSuccess, Token64 k)
            : onFinish (std::move (onSuccess)), key (k), id (i) {}

        PropertyExchangeCache cache;
        std::function<void (const PropertyExchangeResult&)> onFinish;
        Token64 key{};
        uint8_t id = 0;
    };

    template <typename WithCache>
    void updateCache (RequestID b, WithCache&& withCache)
    {
        if (auto& entry = caches[b.asInt()])
        {
            if (const auto result = withCache (entry->cache))
            {
                const auto tmp = std::move (*entry);
                ids.erase (tmp.key);
                entry.reset();
                NullCheckedInvocation::invoke (tmp.onFinish, result->result);
            }
        }
    }

    std::array<std::optional<Transaction>, numCaches> caches;
    std::map<Token64, uint8_t> ids;
    uint64_t lastKey = 0;
};

//==============================================================================
class InitiatorPropertyExchangeCache::Impl
{
public:
    std::optional<Token64> primeCache (uint8_t maxSimultaneousRequests,
                                       std::function<void (const PropertyExchangeResult&)> onDone)
    {
        const auto id = array.findUnusedId (maxSimultaneousRequests);

        return id.has_value() ? std::optional<Token64> (array.primeCacheForRequestId (id->asInt(), std::move (onDone)))
                              : std::nullopt;
    }

    bool terminate (Token64 token)
    {
        return array.terminate (token);
    }

    std::optional<Token64> getTokenForRequestId (RequestID id) const
    {
        return array.getKeyForId (id);
    }

    std::optional<RequestID> getRequestIdForToken (Token64 token) const
    {
        return array.getIdForKey (token);
    }

    void addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    void notify (RequestID b, Span<const std::byte> header) { array.notify (b, header); }
    auto getOngoingTransactions() const { return array.getOngoingTransactions(); }

private:
    PropertyExchangeCacheArray array;
};

//==============================================================================
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache() : pimpl (std::make_unique<Impl>()) {}
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache& InitiatorPropertyExchangeCache::operator= (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache::~InitiatorPropertyExchangeCache() = default;

std::optional<Token64> InitiatorPropertyExchangeCache::primeCache (uint8_t maxSimultaneousTransactions,
                                                                   std::function<void (const PropertyExchangeResult&)> onDone)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone));
}

bool InitiatorPropertyExchangeCache::terminate (Token64 token) { return pimpl->terminate (token); }
std::optional<Token64> InitiatorPropertyExchangeCache::getTokenForRequestId (RequestID id) const { return pimpl->getTokenForRequestId (id); }
std::optional<RequestID> InitiatorPropertyExchangeCache::getRequestIdForToken (Token64 token) const { return pimpl->getRequestIdForToken (token); }
void InitiatorPropertyExchangeCache::addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
void InitiatorPropertyExchangeCache::notify (RequestID b, Span<const std::byte> header) { pimpl->notify (b, header); }
std::vector<Token64> InitiatorPropertyExchangeCache::getOngoingTransactions() const { return pimpl->getOngoingTransactions(); }

//==============================================================================
class ResponderPropertyExchangeCache::Impl
{
public:
    void primeCache (uint8_t maxSimultaneousTransactions,
                     std::function<void (const PropertyExchangeResult&)> onDone,
                     RequestID id)
    {
        if (array.hasTransaction (id))
            return;

        if (array.countOngoingTransactions() >= maxSimultaneousTransactions)
            NullCheckedInvocation::invoke (onDone, PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });
        else
            array.primeCacheForRequestId (id.asInt(), std::move (onDone));
    }

    void addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    void notify (RequestID b, Span<const std::byte> header) { array.notify (b, header); }
    int countOngoingTransactions() const { return array.countOngoingTransactions(); }

private:
    PropertyExchangeCacheArray array;
};

//==============================================================================
ResponderPropertyExchangeCache::ResponderPropertyExchangeCache() : pimpl (std::make_unique<Impl>()) {}
ResponderPropertyExchangeCache::ResponderPropertyExchangeCache (ResponderPropertyExchangeCache&&) noexcept = default;
ResponderPropertyExchangeCache& ResponderPropertyExchangeCache::operator= (ResponderPropertyExchangeCache&&) noexcept = default;
ResponderPropertyExchangeCache::~ResponderPropertyExchangeCache() = default;

void ResponderPropertyExchangeCache::primeCache (uint8_t maxSimultaneousTransactions,
                                                 std::function<void (const PropertyExchangeResult&)> onDone,
                                                 RequestID id)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone), id);
}

void ResponderPropertyExchangeCache::addChunk (RequestID b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
void ResponderPropertyExchangeCache::notify (RequestID b, Span<const std::byte> header) { pimpl->notify (b, header); }
int ResponderPropertyExchangeCache::countOngoingTransactions() const { return pimpl->countOngoingTransactions(); }

} // namespace juce::midi_ci
