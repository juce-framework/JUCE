/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

class PropertyExchangeCache
{
public:
    explicit PropertyExchangeCache (std::function<void()> term)
        : onTerminate (std::move (term)) {}

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

        onTerminate = nullptr;
        const auto encodingString = headerJson.getProperty ("mutualEncoding", "ASCII").toString();

        if (chunk.thisChunkNum != chunk.totalNumChunks)
            return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::partial };

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

        onTerminate = nullptr;
        return std::optional<OwningResult> { std::in_place, PropertyExchangeResult::Error::notify };
    }

    void terminate()
    {
        if (auto t = std::exchange (onTerminate, nullptr))
            t();
    }

private:
    std::vector<char> headerStorage;
    std::vector<std::byte> bodyStorage;
    std::function<void()> onTerminate;
    uint16_t lastChunk = 0;
};

//==============================================================================
class PropertyExchangeCacheArray
{
public:
    PropertyExchangeCacheArray() = default;

    ErasedScopeGuard primeCacheForRequestId (std::byte id,
                                             std::function<void (const PropertyExchangeResult&)> onDone,
                                             std::function<void()> onTerminate)
    {
        auto& entry = caches[(uint8_t) id];
        entry = std::make_shared<Transaction> (std::move (onDone), std::move (onTerminate));
        auto weak = std::weak_ptr<Transaction> (entry);

        return ErasedScopeGuard { [&entry, weak]
        {
            // If this fails, then the transaction finished before the ErasedScopeGuard was destroyed.
            if (auto locked = weak.lock())
            {
                entry->cache.terminate();
                entry = nullptr;
            }
        } };
    }

    void addChunk (std::byte b, const Message::DynamicSizePropertyExchange& chunk)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.addChunk (chunk); });
    }

    void notify (std::byte b, Span<const std::byte> header)
    {
        updateCache (b, [&] (PropertyExchangeCache& c) { return c.notify (header); });
    }

    bool hasTransaction (std::byte id) const
    {
        return caches[(uint8_t) id] != nullptr;
    }

    uint8_t countOngoingTransactions() const
    {
        return (uint8_t) std::count_if (caches.begin(), caches.end(), [] (auto& c) { return c != nullptr; });
    }

    /** MSB of result is set on failure. */
    std::byte findUnusedId (uint8_t maxSimultaneousTransactions) const
    {
        if (countOngoingTransactions() >= maxSimultaneousTransactions)
            return std::byte { 0xff };

        return (std::byte) std::distance (caches.begin(), std::find (caches.begin(), caches.end(), nullptr));
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
        Transaction (std::function<void (const PropertyExchangeResult&)> onSuccess,
                     std::function<void()> onTerminate)
            : cache (std::move (onTerminate)), onFinish (std::move (onSuccess)) {}

        PropertyExchangeCache cache;
        std::function<void (const PropertyExchangeResult&)> onFinish;
    };

    template <typename WithCache>
    void updateCache (std::byte b, WithCache&& withCache)
    {
        if (auto& entry = caches[(uint8_t) b])
        {
            if (const auto result = withCache (entry->cache))
            {
                const auto tmp = std::move (entry->onFinish);
                entry = nullptr;
                NullCheckedInvocation::invoke (tmp, result->result);
            }
        }
    }

    std::array<std::shared_ptr<Transaction>, numCaches> caches;
};

//==============================================================================
class InitiatorPropertyExchangeCache::Impl
{
public:
    TokenAndId primeCache (uint8_t maxSimultaneousRequests,
                           std::function<void (const PropertyExchangeResult&)> onDone,
                           std::function<void (std::byte)> onTerminate)
    {
        const auto id = array.findUnusedId (maxSimultaneousRequests);

        if ((id & std::byte { 0x80 }) != std::byte{})
        {
            NullCheckedInvocation::invoke (onDone, PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });
            return {};
        }

        auto token = array.primeCacheForRequestId (id,
                                                   std::move (onDone),
                                                   [id, term = std::move (onTerminate)] { NullCheckedInvocation::invoke (term, id); });
        return { std::move (token), id };
    }

    void addChunk (std::byte b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    void notify (std::byte b, Span<const std::byte> header) { array.notify (b, header); }
    int countOngoingTransactions() const { return array.countOngoingTransactions(); }
    bool isAwaitingResponse() const { return countOngoingTransactions() != 0; }

private:
    PropertyExchangeCacheArray array;
};

//==============================================================================
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache() : pimpl (std::make_unique<Impl>()) {}
InitiatorPropertyExchangeCache::InitiatorPropertyExchangeCache (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache& InitiatorPropertyExchangeCache::operator= (InitiatorPropertyExchangeCache&&) noexcept = default;
InitiatorPropertyExchangeCache::~InitiatorPropertyExchangeCache() = default;

InitiatorPropertyExchangeCache::TokenAndId InitiatorPropertyExchangeCache::primeCache (uint8_t maxSimultaneousTransactions,
                                                                                       std::function<void (const PropertyExchangeResult&)> onDone,
                                                                                       std::function<void (std::byte)> onTerminate)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone), std::move (onTerminate));
}

void InitiatorPropertyExchangeCache::addChunk (std::byte b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
void InitiatorPropertyExchangeCache::notify (std::byte b, Span<const std::byte> header) { pimpl->notify (b, header); }
int InitiatorPropertyExchangeCache::countOngoingTransactions() const { return pimpl->countOngoingTransactions(); }
bool InitiatorPropertyExchangeCache::isAwaitingResponse() const { return pimpl->isAwaitingResponse(); }

//==============================================================================
class ResponderPropertyExchangeCache::Impl
{
public:
    void primeCache (uint8_t maxSimultaneousTransactions,
                     std::function<void (const PropertyExchangeResult&)> onDone,
                     std::byte id)
    {
        if (array.hasTransaction (id))
            return;

        if (array.countOngoingTransactions() >= maxSimultaneousTransactions)
            NullCheckedInvocation::invoke (onDone, PropertyExchangeResult { PropertyExchangeResult::Error::tooManyTransactions });
        else
            array.primeCacheForRequestId (id, std::move (onDone), nullptr).release();
    }

    void addChunk (std::byte b, const Message::DynamicSizePropertyExchange& chunk) { array.addChunk (b, chunk); }
    void notify (std::byte b, Span<const std::byte> header) { array.notify (b, header); }
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
                                                 std::byte id)
{
    return pimpl->primeCache (maxSimultaneousTransactions, std::move (onDone), id);
}

void ResponderPropertyExchangeCache::addChunk (std::byte b, const Message::DynamicSizePropertyExchange& chunk) { pimpl->addChunk (b, chunk); }
void ResponderPropertyExchangeCache::notify (std::byte b, Span<const std::byte> header) { pimpl->notify (b, header); }
int ResponderPropertyExchangeCache::countOngoingTransactions() const { return pimpl->countOngoingTransactions(); }

} // namespace juce::midi_ci
