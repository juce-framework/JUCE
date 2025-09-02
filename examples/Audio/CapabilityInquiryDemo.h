/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             CapabilityInquiryDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Performs MIDI Capability Inquiry transactions

 dependencies:     juce_audio_basics, juce_audio_devices, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_midi_ci
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CapabilityInquiryDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

/** This allows listening for changes to some data. Unlike ValueTree, it remembers the types
    of all the data that's stored, which makes it a bit nicer to use. Also unlike ValueTree,
    every mutation necessitates a deep copy of the model, so this isn't necessarily suitable
    for large models that change frequently.

    Use operator-> or operator* to find the current state (without copying!).

    Use operator= to assign new state.
    After assigning new state, the old and new states will be compared, and
    observers will be notified if the new state is different to the old state.

    Use operator[] to retrieve nested states.
    This is useful when a component only depends on a small part of a larger
    model. Assigning a new value to the nested state will also cause observers
    of the parent state to be notified. Similarly, assigning a new state to
    the parent state will cause observers of the nested state to be notified,
    if the nested state actually changes in value.
*/
template <typename Value>
class State
{
public:
    State() : State (Value{}) {}

    State (Value v, std::function<Value (const Value&, Value)> n)
        : impl (std::make_unique<Root> (std::move (v), std::move (n))) {}

    explicit State (Value v)
        : State (std::move (v), [] (const Value&, Value newer) { return std::move (newer); }) {}

    template <typename Parent>
    State (State<Parent> p, Value Parent::* member)
        : impl (std::make_unique<Member<Parent>> (std::move (p), member)) {}

    template <typename Parent, typename Ind>
    State (State<Parent> p, Ind index)
        : impl (std::make_unique<Index<Parent, Ind>> (std::move (p), std::move (index))) {}

    State& operator= (const Value& v)
    {
        impl->set (v);
        return *this;
    }

    State& operator= (Value&& v)
    {
        impl->set (std::move (v));
        return *this;
    }

    const Value& operator* () const { return  impl->get(); }
    const Value* operator->() const { return &impl->get(); }

    ErasedScopeGuard observe (std::function<void (const Value&)> onChange)
    {
        // The idea is that observers want to know whenever the state changes, in order to repaint.
        // They'll also want to repaint upon first observing, so that painting is up-to-date.
        onChange (impl->get());

        // In the case that observe is called on a temporary, caching impl means that the
        // ErasedScopeGuard can still detach safely
        return impl->observe ([cached = *this, fn = std::move (onChange)] (const auto& x)
        {
            fn (x);
        });
    }

    template <typename T>
    auto operator[] (T member) const
        -> State<std::decay_t<decltype (std::declval<Value>().*std::declval<T>())>>
    {
        return { *this, std::move (member) };
    }

    template <typename T>
    auto operator[] (T index) const
        -> State<std::decay_t<decltype (std::declval<Value>()[std::declval<T>()])>>
    {
        return { *this, std::move (index) };
    }

private:
    class Provider : public std::enable_shared_from_this<Provider>
    {
    public:
        virtual ~Provider() = default;
        virtual void set (const Value&) = 0;
        virtual void set (Value&&) = 0;
        virtual const Value& get() const = 0;

        ErasedScopeGuard observe (std::function<void (const Value&)> fn)
        {
            return ErasedScopeGuard { [t = this->shared_from_this(), it = list.insert (list.end(), std::move (fn))]
                                      {
                                          t->list.erase (it);
                                      } };
        }

        void notify (const Value& v) const
        {
            for (auto& callback : list)
                callback (v);
        }

    private:
        using List = std::list<std::function<void (const Value&)>>;
        List list;
    };

    class Root final : public Provider
    {
    public:
        explicit Root (Value v, std::function<Value (const Value&, Value&&)> n)
            : value (std::move (v)), normalise (std::move (n)) {}

        void set (const Value& m) override { setImpl (m); }
        void set (Value&& m)      override { setImpl (std::move (m)); }
        const Value& get()  const override { return value; }

    private:
        template <typename T>
        void setImpl (T&& t)
        {
            // If this fails, you're updating the model from inside an observer callback.
            // That's not very unidirectional-data-flow of you!
            jassert (! reentrant);
            const ScopedValueSetter scope { reentrant, true };

            auto normalised = normalise (value, std::forward<T> (t));

            if (normalised != value)
                this->notify (std::exchange (value, std::move (normalised)));
        }

        Value value;
        std::function<Value (const Value&, Value)> normalise;
        bool reentrant = false;
    };

    template <typename Parent>
    class Member final : public Provider
    {
    public:
        Member (State<Parent> p, Value Parent::* m) : parent (std::move (p)), member (m) {}

        void set (const Value& m) override { setImpl (m); }
        void set (Value&& m)      override { setImpl (std::move (m)); }
        const Value& get()  const override { return (*parent).*member; }

    private:
        template <typename T>
        void setImpl (T&& t)
        {
            auto updated = *parent;
            updated.*member = std::forward<T> (t);
            parent = std::move (updated);
        }

        State<Parent> parent;
        Value Parent::*member;
        ErasedScopeGuard listener = parent.observe ([this] (const auto& old)
        {
            if (old.*member != get())
                this->notify (old.*member);
        });
    };

    template <typename Parent, typename Ind>
    class Index final : public Provider
    {
    public:
        Index (State<Parent> p, Ind i) : parent (std::move (p)), index (i) {}

        void set (const Value& m) override { setImpl (m); }
        void set (Value&& m)      override { setImpl (std::move (m)); }
        const Value& get()  const override { return (*parent)[index]; }

    private:
        template <typename T>
        void setImpl (T&& t)
        {
            auto updated = *parent;
            updated[index] = std::forward<T> (t);
            parent = std::move (updated);
        }

        State<Parent> parent;
        Ind index;
        ErasedScopeGuard listener = parent.observe ([this] (const auto& old)
        {
            if (isPositiveAndBelow (index, std::size (*parent))
                && isPositiveAndBelow (index, std::size (old))
                && old[index] != get())
            {
                this->notify (old[index]);
            }
        });
    };

    std::shared_ptr<Provider> impl;
};

/**
    Data types used to represent the state of the application.
    These should all be types with value semantics, so there should not be pointers or references
    between different parts of the model.
*/
struct Model
{
    Model() = delete;

    template <typename Range, typename Fn>
    static Array<var> toVarArray (Range&& range, Fn&& fn)
    {
        Array<var> result;
        result.resize ((int) std::size (range));
        std::transform (std::begin (range),
                        std::end (range),
                        result.begin(),
                        std::forward<Fn> (fn));
        return result;
    }

    template <typename Fn, typename T>
    static auto fromVarArray (var in, Fn&& fn, std::vector<T> fallback) -> std::vector<T>
    {
        auto* list = in.getArray();

        if (list == nullptr)
            return fallback;

        std::vector<T> result;
        for (auto& t : *list)
            result.push_back (fn (t));
        return result;
    }

    #define JUCE_TUPLE_RELATIONAL_OP(classname, op)                                         \
        bool operator op (const classname& other) const                                     \
        {                                                                                   \
            return tie() op other.tie();                                                    \
        }

    #define JUCE_TUPLE_RELATIONAL_OPS(classname) \
        JUCE_TUPLE_RELATIONAL_OP(classname, ==)  \
        JUCE_TUPLE_RELATIONAL_OP(classname, !=)

    template <typename T>
    struct ListWithSelection
    {
        std::vector<T> items;
        int selection = -1;

        static constexpr auto marshallingVersion = std::nullopt;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("items", t.items),
                            named ("selection", t.selection));
        }

        auto tie() const { return std::tie (items, selection); }
        JUCE_TUPLE_RELATIONAL_OPS (ListWithSelection)

    private:
        template <typename This, typename Index>
        static auto* get (This& t, Index index)
        {
            if (isPositiveAndBelow (index, t.items.size()))
                return &t.items[(size_t) index];

            return static_cast<decltype (t.items.data())> (nullptr);
        }

        template <typename This>
        static auto* getSelected (This& t)
        {
            return get (t, t.selection);
        }

    public:
        auto* getSelected()       { return getSelected (*this); }
        auto* getSelected() const { return getSelected (*this); }

        auto* get (int index)       { return get (*this, index); }
        auto* get (int index) const { return get (*this, index); }
    };

    enum class ProfileMode
    {
        edit,
        use,
    };

    struct Profiles
    {
        ListWithSelection<ci::Profile> profiles;
        std::map<ci::ProfileAtAddress, ci::SupportedAndActive> channels;
        std::optional<ci::ChannelAddress> selectedChannel;
        ProfileMode profileMode{};

        std::optional<ci::ProfileAtAddress> getSelectedProfileAtAddress() const
        {
            if (selectedChannel.has_value())
                if (auto* item = profiles.getSelected())
                    return ci::ProfileAtAddress { *item, *selectedChannel };

            return {};
        }

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("profiles", t.profiles),
                            named ("channels", t.channels),
                            named ("selectedChannel", t.selectedChannel),
                            named ("profileMode", t.profileMode));
        }

        auto tie() const { return std::tie (profiles, channels, selectedChannel, profileMode); }
        JUCE_TUPLE_RELATIONAL_OPS (Profiles)
    };

    enum class DataViewMode
    {
        ascii,
        hex,
    };

    #define JUCE_CAN_SET X(none) X(full) X(partial)

    enum class CanSet
    {
        #define X(name) name,
        JUCE_CAN_SET
        #undef X
    };

    struct CanSetUtils
    {
        CanSetUtils() = delete;

        static std::optional<CanSet> fromString (const char* str)
        {
            #define X(name) if (StringRef (str) == StringRef (#name)) return CanSet::name;
            JUCE_CAN_SET
            #undef X

            return std::nullopt;
        }

        static const char* toString (CanSet c)
        {
            switch (c)
            {
                #define X(name) case CanSet::name: return #name;
                JUCE_CAN_SET
                #undef X
            }

            return nullptr;
        }
    };

    #undef JUCE_CAN_SET

    struct PropertyValue
    {
        std::vector<std::byte> bytes; ///< decoded bytes
        String mediaType = "application/json";

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("bytes", t.bytes),
                            named ("mediaType", t.mediaType));
        }

        auto tie() const { return std::tie (bytes, mediaType); }
        JUCE_TUPLE_RELATIONAL_OPS (PropertyValue)
    };

    struct ReadableDeviceInfo
    {
        String manufacturer, family, model, version;
    };

    struct Property
    {
        String name;
        var schema;
        std::set<ci::Encoding> encodings { ci::Encoding::ascii };
        std::vector<String> mediaTypes { "application/json" };
        var columns;
        CanSet canSet = CanSet::none;
        bool canGet = true, canSubscribe = false, requireResId = false, canPaginate = false;
        PropertyValue value;

        [[nodiscard]] std::optional<ci::Encoding> getBestCommonEncoding (
            ci::Encoding firstChoice = ci::Encoding::ascii) const
        {
            if (encodings.count (firstChoice) != 0)
                return firstChoice;

            if (! encodings.empty())
                return *encodings.begin();

            return {};
        }

        std::optional<ReadableDeviceInfo> getReadableDeviceInfo() const
        {
            if (name != "DeviceInfo")
                return {};

            const auto parsed = ci::Encodings::jsonFrom7BitText (value.bytes);
            auto* object = parsed.getDynamicObject();

            if (object == nullptr)
                return {};

            if (! object->hasProperty ("manufacturer")
                || ! object->hasProperty ("family")
                || ! object->hasProperty ("model")
                || ! object->hasProperty ("version"))
            {
                return {};
            }

            return ReadableDeviceInfo { object->getProperty ("manufacturer"),
                                        object->getProperty ("family"),
                                        object->getProperty ("model"),
                                        object->getProperty ("version") };
        }

        static Property fromResourceListEntry (var entry)
        {
            Model::Property p;
            p.name           = entry.getProperty ("resource", "");
            p.canGet         = entry.getProperty ("canGet", p.canGet);
            const auto set = entry.getProperty ("canSet", {}).toString();
            p.canSet         = Model::CanSetUtils::fromString (set.toRawUTF8()).value_or (p.canSet);
            p.canSubscribe   = entry.getProperty ("canSubscribe", p.canSubscribe);
            p.requireResId   = entry.getProperty ("requireResId", p.requireResId);
            p.mediaTypes     = fromVarArray (entry.getProperty ("mediaTypes", {}),
                                             [] (String str) { return str; },
                                             p.mediaTypes);
            p.schema         = entry.getProperty ("schema", p.schema);
            p.canPaginate    = entry.getProperty ("canPaginate", p.canPaginate);
            p.columns        = entry.getProperty ("columns", p.columns);

            const auto stringToEncoding = [] (String str)
            {
                return ci::EncodingUtils::toEncoding (str.toRawUTF8());
            };
            const auto parsedEncodings = fromVarArray (entry.getProperty ("encodings", {}),
                                                       stringToEncoding,
                                                       std::vector<std::optional<ci::Encoding>>{});
            std::set<ci::Encoding> encodingsSet;
            for (const auto& parsed : parsedEncodings)
                if (parsed.has_value())
                    encodingsSet.insert (*parsed);

            p.encodings      = encodingsSet.empty() ? p.encodings : encodingsSet;
            return p;
        }

        var getResourceListEntry() const
        {
            const Model::Property defaultInfo;
            auto obj = std::make_unique<DynamicObject>();

            obj->setProperty ("resource", name);

            if (canGet != defaultInfo.canGet)
                obj->setProperty ("canGet", canGet);

            if (canSet != defaultInfo.canSet)
                obj->setProperty ("canSet", Model::CanSetUtils::toString (canSet));

            if (canSubscribe != defaultInfo.canSubscribe)
                obj->setProperty ("canSubscribe", canSubscribe);

            if (requireResId != defaultInfo.requireResId)
                obj->setProperty ("requireResId", requireResId);

            if (mediaTypes != defaultInfo.mediaTypes)
            {
                obj->setProperty ("mediaTypes",
                                  toVarArray (mediaTypes, [] (auto str) { return str; }));
            }

            if (encodings != defaultInfo.encodings)
            {
                obj->setProperty ("encodings",
                                  toVarArray (encodings, ci::EncodingUtils::toString));
            }

            if (schema != defaultInfo.schema)
                obj->setProperty ("schema", schema);

            if (name.endsWith ("List"))
            {
                if (canPaginate != defaultInfo.canPaginate)
                    obj->setProperty ("canPaginate", canPaginate);

                if (columns != defaultInfo.columns)
                    obj->setProperty ("columns", columns);
            }

            return obj.release();
        }

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("name", t.name),
                            named ("schema", t.schema),
                            named ("encodings", t.encodings),
                            named ("mediaTypes", t.mediaTypes),
                            named ("columns", t.columns),
                            named ("canSet", t.canSet),
                            named ("canGet", t.canGet),
                            named ("canSubscribe", t.canSubscribe),
                            named ("requireResId", t.requireResId),
                            named ("canPaginate", t.canPaginate),
                            named ("value", t.value));
        }

        auto tie() const
        {
            return std::tie (name,
                             schema,
                             encodings,
                             mediaTypes,
                             columns,
                             canSet,
                             canGet,
                             canSubscribe,
                             requireResId,
                             canPaginate,
                             value);
        }

        JUCE_TUPLE_RELATIONAL_OPS (Property)
    };

    struct Properties
    {
        ListWithSelection<Property> properties;
        DataViewMode mode{};

        std::optional<ReadableDeviceInfo> getReadableDeviceInfo() const
        {
            for (const auto& prop : properties.items)
                if (auto info = prop.getReadableDeviceInfo())
                    return info;

            return {};
        }

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("properties", t.properties),
                            named ("mode", t.mode));
        }

        auto tie() const { return std::tie (properties, mode); }
        JUCE_TUPLE_RELATIONAL_OPS (Properties)
    };

    struct IOSelection
    {
        std::optional<MidiDeviceInfo> input, output;

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("input", t.input),
                            named ("output", t.output));
        }

        auto tie() const { return std::tie (input, output); }
        JUCE_TUPLE_RELATIONAL_OPS (IOSelection)
    };

    struct DeviceInfo
    {
        ump::DeviceInfo deviceInfo;
        size_t maxSysExSize { 512 };
        uint8_t numPropertyExchangeTransactions { 127 };
        bool profilesSupported { false }, propertiesSupported { false };

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("deviceInfo", t.deviceInfo),
                            named ("maxSysExSize", t.maxSysExSize),
                            named ("numPropertyExchangeTransactions",
                                   t.numPropertyExchangeTransactions),
                            named ("profilesSupported", t.profilesSupported),
                            named ("propertiesSupported", t.propertiesSupported));
        }

        auto tie() const
        {
            return std::tie (deviceInfo,
                             maxSysExSize,
                             numPropertyExchangeTransactions,
                             profilesSupported,
                             propertiesSupported);
        }
        JUCE_TUPLE_RELATIONAL_OPS (DeviceInfo)
    };

    enum class MessageKind
    {
        incoming,
        outgoing,
    };

    struct LogView
    {
        std::optional<MessageKind> filter;
        DataViewMode mode = DataViewMode::ascii;

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("filter", t.filter),
                            named ("mode", t.mode));
        }

        auto tie() const { return std::tie (filter, mode); }
        JUCE_TUPLE_RELATIONAL_OPS (LogView)
    };

    /**
        The bits of the app state that we want to save and restore
    */
    struct Saved
    {
        DeviceInfo fundamentals;
        IOSelection ioSelection;
        Profiles profiles;
        Properties properties;
        LogView logView;

        static constexpr auto marshallingVersion = 0;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("fundamentals", t.fundamentals),
                            named ("ioSelection", t.ioSelection),
                            named ("profiles", t.profiles),
                            named ("properties", t.properties),
                            named ("logView", t.logView));
        }

        auto tie() const
        {
            return std::tie (fundamentals,
                             ioSelection,
                             profiles,
                             properties,
                             logView);
        }
        JUCE_TUPLE_RELATIONAL_OPS (Saved)
    };

    struct Device
    {
        ci::MUID muid = ci::MUID::makeUnchecked (0);
        DeviceInfo info;
        Profiles profiles;
        Properties properties;
        std::map<ci::SubscriptionKey, ci::Subscription> subscriptions;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("muid", t.muid),
                            named ("info", t.info),
                            named ("profiles", t.profiles),
                            named ("properties", t.properties),
                            named ("subscribeIdForResource", t.subscribeIdForResource));
        }

        auto tie() const
        {
            return std::tie (muid, info, profiles, properties, subscriptions);
        }
        JUCE_TUPLE_RELATIONAL_OPS (Device)
    };

    struct LogEntry
    {
        std::vector<std::byte> message;
        uint8_t group{};
        Time time;
        MessageKind kind;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("message", t.message),
                            named ("group", t.group),
                            named ("time", t.time),
                            named ("kind", t.kind));
        }

        auto tie() const { return std::tie (message, group, time, kind); }
        JUCE_TUPLE_RELATIONAL_OPS (LogEntry)
    };

    /**
        App state that needs to be displayed somehow, but which shouldn't be saved or restored.
    */
    struct Transient
    {
        // property -> device -> subId
        std::map<String, std::map<ci::MUID, std::set<String>>> subscribers;
        ListWithSelection<Device> devices;
        std::deque<LogEntry> logEntries;

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("subscribers", t.subscribers),
                            named ("devices", t.devices),
                            named ("logEntries", t.logEntries));
        }

        auto tie() const { return std::tie (subscribers, devices, logEntries); }
        JUCE_TUPLE_RELATIONAL_OPS (Transient)
    };

    struct App
    {
        Saved saved;
        Transient transient;

        /** Removes properties from Transient::subscribers that are no longer present in
            Properties::properties.
        */
        void syncSubscribers()
        {
            std::set<String> currentProperties;
            for (auto& v : saved.properties.properties.items)
                currentProperties.insert (v.name);

            std::set<String> removedProperties;
            for (const auto& oldSubscriber : transient.subscribers)
                if (currentProperties.find (oldSubscriber.first) == currentProperties.end())
                    removedProperties.insert (oldSubscriber.first);

            for (const auto& removed : removedProperties)
                transient.subscribers.erase (removed);
        }

        template <typename Archive, typename This>
        static auto serialise (Archive& archive, This& t)
        {
            return archive (named ("saved", t.saved),
                            named ("transient", t.transient));
        }

        auto tie() const { return std::tie (saved, transient); }
        JUCE_TUPLE_RELATIONAL_OPS (App)
    };

    #undef JUCE_TUPLE_RELATIONAL_OPS
    #undef JUCE_TUPLE_RELATIONAL_OP
};

template <>
struct juce::SerialisationTraits<MidiDeviceInfo>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename This>
    static auto serialise (Archive& archive, This& x)
    {
        archive (named ("name", x.name), named ("identifier", x.identifier));
    }
};

template <>
struct juce::SerialisationTraits<ci::ChannelAddress>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive>
    static auto load (Archive& archive, ci::ChannelAddress& x)
    {
        auto group = x.getGroup();
        auto channel = x.getChannel();
        archive (named ("group", group),
                 named ("channel", channel));
        x = ci::ChannelAddress{}.withGroup (group).withChannel (channel);
    }

    template <typename Archive>
    static auto save (Archive& archive, const ci::ChannelAddress& x)
    {
        archive (named ("group", x.getGroup()),
                 named ("channel", x.getChannel()));
    }
};

template <>
struct juce::SerialisationTraits<ci::ProfileAtAddress>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename This>
    static auto serialise (Archive& archive, This& x)
    {
        archive (named ("profile", x.profile), named ("address", x.address));
    }
};

template <>
struct juce::SerialisationTraits<ci::SupportedAndActive>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename This>
    static auto serialise (Archive& archive, This& x)
    {
        archive (named ("supported", x.supported), named ("active", x.active));
    }
};

class MonospaceEditor : public TextEditor
{
public:
    MonospaceEditor()
    {
        setFont (FontOptions { Font::getDefaultMonospacedFontName(), 12, 0 });
    }

    void onCommit (std::function<void()> fn)
    {
        onEscapeKey = onReturnKey = onFocusLost = std::move (fn);
    }

    void set (const String& str)
    {
        setText (str, false);
    }
};

class MonospaceLabel : public Label
{
public:
    MonospaceLabel()
    {
        setFont (FontOptions { Font::getDefaultMonospacedFontName(), 12, 0 });
        setMinimumHorizontalScale (1.0f);
        setInterceptsMouseClicks (false, false);
    }

    void onCommit (std::function<void()>) {}

    void set (const String& str)
    {
        setText (str, dontSendNotification);
    }
};

enum class Editable
{
    no,
    yes,
};

template <Editable>
class TextField;

template <>
class TextField<Editable::yes> : public MonospaceEditor { using MonospaceEditor::MonospaceEditor; };

template <>
class TextField<Editable::no> : public MonospaceLabel { using MonospaceLabel::MonospaceLabel; };

struct Utils
{
    Utils() = delete;

    static constexpr auto padding = 10;
    static constexpr auto standardControlHeight = 30;

    template <typename T, typename... Ts>
    static auto makeVector (T&& t, Ts&&... ts)
    {
        std::vector<T> result;
        result.reserve (1 + sizeof... (ts));
        result.emplace_back (std::forward<T> (t));
        (result.emplace_back (std::forward<Ts> (ts)), ...);
        return result;
    }

    static std::unique_ptr<Label> makeListRowLabel (StringRef text, bool selected)
    {
        auto label = std::make_unique<TextField<Editable::no>>();
        label->set (text);

        if (selected)
        {
            const auto fg = label->findColour (TextEditor::ColourIds::highlightedTextColourId);
            label->setColour (Label::ColourIds::textColourId, fg);

            const auto bg = label->findColour (TextEditor::ColourIds::highlightColourId);
            label->setColour (Label::ColourIds::backgroundColourId, bg);
        }

        return label;
    }

    static SparseSet<int> setWithSingleIndex (int i)
    {
        SparseSet<int> result;
        result.addRange ({ i, i + 1 });
        return result;
    }

    template <typename... Items>
    static void doTwoColumnLayout (Rectangle<int> bounds, Items&&... items)
    {
        Grid grid;
        grid.autoFlow = Grid::AutoFlow::row;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.columnGap = grid.rowGap = Grid::Px { Utils::padding };
        grid.templateColumns = { Grid::TrackInfo { Grid::Px { 400 } },
                                 Grid::TrackInfo { Grid::Fr { 1 } } };
        grid.items = { GridItem { items }... };
        grid.performLayout (bounds);
    }

    template <typename... Items>
    static void doColumnLayout (Rectangle<int> bounds, Items&&... items)
    {
        Grid grid;
        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.columnGap = grid.rowGap = Grid::Px { Utils::padding };
        grid.items = { GridItem { items }... };
        grid.performLayout (bounds);
    }

    template <size_t N>
    static void stringToByteArray (const String& str, std::array<std::byte, N>& a)
    {
        const auto asInt = str.removeCharacters (" ").getHexValue64();

        for (size_t i = 0; i < N; ++i)
            a[i] = std::byte ((asInt >> (8 * (N - 1 - i))) & 0x7f);
    }

    template <typename Callback>
    static void forAllChannelAddresses (Callback&& callback)
    {
        for (uint8_t group = 0; group < 16; ++group)
        {
            for (uint8_t channel = 0; channel < 17; ++channel)
            {
                const auto channelKind = channel < 16 ? ci::ChannelInGroup (channel)
                                                      : ci::ChannelInGroup::wholeGroup;
                callback (ci::ChannelAddress{}.withGroup (group).withChannel (channelKind));
            }
        }

        callback (ci::ChannelAddress{}.withChannel (ci::ChannelInGroup::wholeBlock));
    }

    template <typename Request>
    static std::tuple<Model::Property, String> attemptSetPartial (Model::Property prop,
                                                                  const Request& request)
    {
        if (request.header.mediaType != "application/json")
        {
            return std::tuple (std::move (prop),
                               "Partial updates are only supported when the request body is JSON");
        }

        if (prop.mediaTypes != std::vector<String> { "application/json" })
        {
            return std::tuple (std::move (prop),
                               "Partial updates are only supported when the target property is "
                               "JSON");
        }

        const auto pointers = JSON::parse (String::fromUTF8 (
            reinterpret_cast<const char*> (request.body.data()),
            (int) request.body.size()));
        const auto* obj = pointers.getDynamicObject();

        if (obj == nullptr)
        {
            return std::tuple (std::move (prop),
                               "Property data for a partial update must be a json object");
        }

        for (const auto& pair : obj->getProperties())
        {
            if (pair.value.isObject() || pair.value.isArray())
            {
                return std::tuple (std::move (prop),
                                   "Property data for a partial update must not contain nested "
                                   "arrays or objects");
            }
        }

        const auto updatedPropertyValue = [&]() -> std::optional<var>
        {
            std::optional result { JSON::parse (String::fromUTF8 (
                reinterpret_cast<const char*> (prop.value.bytes.data()),
                (int) prop.value.bytes.size())) };

            for (const auto& [key, value] : obj->getProperties())
            {
                if (! result.has_value())
                    return {};

                result = JSONUtils::setPointer (*result, key.toString(), value);
            }

            return result;
        }();

        if (! updatedPropertyValue)
        {
            return std::tuple (std::move (prop),
                               "Partial updates do not apply, perhaps the JSON pointer does not "
                               "refer to an extant node");
        }

        prop.value.bytes = ci::Encodings::jsonTo7BitText (*updatedPropertyValue);
        return std::tuple (std::move (prop), String());
    }

    template <typename... Args>
    static auto forwardFunction (std::function<void (Args...)>& fn)
    {
        return [&fn] (auto&&... args)
        {
            NullCheckedInvocation::invoke (fn, std::forward<decltype (args)> (args)...);
        };
    }
};

template <typename Kind>
class IOPickerModel : public ListBoxModel
{
public:
    explicit IOPickerModel (State<std::optional<MidiDeviceInfo>> s)
        : state (s) {}

    int getNumRows() override
    {
        return Kind::getAvailableDevices().size();
    }

    void paintListBoxItem (int, Graphics&, int, int, bool) override {}

    Component* refreshComponentForRow (int rowNumber,
                                       bool rowIsSelected,
                                       Component* existingComponentToUpdate) override
    {
        const auto toDelete = rawToUniquePtr (existingComponentToUpdate);
        const auto info = Kind::getAvailableDevices()[rowNumber];
        return Utils::makeListRowLabel (info.name, rowIsSelected).release();
    }

    void selectedRowsChanged (int newSelection) override
    {
        state = getInfoForRow (newSelection);
    }

    std::optional<MidiDeviceInfo> getInfoForRow (int row) const
    {
        return isPositiveAndBelow (row, Kind::getAvailableDevices().size())
               ? std::optional (Kind::getAvailableDevices()[row])
               : std::nullopt;
    }

private:
    State<std::optional<MidiDeviceInfo>> state;
};

template <typename Kind>
class IOPickerList : public Component
{
public:
    explicit IOPickerList (State<std::optional<MidiDeviceInfo>> s)
        : state (s),
          model (s)
    {
        addAndMakeVisible (label);
        label.setJustificationType (Justification::centred);

        addAndMakeVisible (list);
        list.setMultipleSelectionEnabled (false);
        list.setClickingTogglesRowSelection (true);
        list.updateContent();
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.items = { FlexItem { label }.withHeight (Utils::standardControlHeight),
                     FlexItem{}.withHeight (Utils::padding),
                     FlexItem { list }.withFlex (1) };
        fb.performLayout (getLocalBounds());
    }

private:
    void setSelected (std::optional<MidiDeviceInfo> selected)
    {
        list.setSelectedRows ({}, dontSendNotification);
        list.updateContent();

        for (auto i = 0; i < model.getNumRows(); ++i)
            if (model.getInfoForRow (i) == selected)
                list.setSelectedRows (Utils::setWithSingleIndex (i), dontSendNotification);
    }

    static String getLabel (std::in_place_type_t<MidiInput>)  { return "Input"; }
    static String getLabel (std::in_place_type_t<MidiOutput>) { return "Output"; }

    State<std::optional<MidiDeviceInfo>> state;

    Label label { "", getLabel (std::in_place_type<Kind>) };
    IOPickerModel<Kind> model;
    ListBox list { "Devices", &model };

    MidiDeviceListConnection connection = MidiDeviceListConnection::make ([this]
    {
        list.updateContent();
        setSelected (*state);
    });

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        setSelected (*state);
    });
};

static constexpr auto ioLabelText = R"(Pick the input and output used to talk to the Capability Inquiry (CI) responder.

In order to use this demo you'll need another program/device that understands MIDI CI.
You could run a second copy of this CapabilityInquiryDemo, or install and use one of the apps listed below.
If you want to communicate with a program that doesn't have its own virtual MIDI ports, you may need to set up virtual ports yourself, e.g. by enabling the IAC MIDI driver on macOS.)";

class IOPickerLists : public Component
{
public:
    explicit IOPickerLists (State<Model::IOSelection> selection)
        : inputs (selection[&Model::IOSelection::input]),
          outputs (selection[&Model::IOSelection::output])
    {
        addAndMakeVisible (inputs);
        addAndMakeVisible (outputs);

        addAndMakeVisible (label);
        addAndMakeVisible (toolsHeader);
        addAndMakeVisible (workbenchButton);
        addAndMakeVisible (responderButton);
        toolsHeader.setJustificationType (Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (Utils::padding);

        responderButton.setBounds (bounds.removeFromBottom (20));
        workbenchButton.setBounds (bounds.removeFromBottom (20));
        toolsHeader.setBounds (bounds.removeFromBottom (20));
        label.setBounds (bounds.removeFromBottom (200).withSizeKeepingCentre (jmin (600, bounds.getWidth()), 200));

        Utils::doColumnLayout (bounds, inputs, outputs);
    }

private:
    IOPickerList<MidiInput> inputs;
    IOPickerList<MidiOutput> outputs;
    Label label { "", ioLabelText };

    Label toolsHeader { "", "Other MIDI-CI software for testing:" };
    HyperlinkButton workbenchButton { "MIDI 2.0 Workbench", URL { "https://github.com/midi2-dev/MIDI2.0Workbench" } };
    HyperlinkButton responderButton { "Bome MIDI-CI Responder", URL { "https://www.bome.com/products/midi-ci-tools" } };
};

class SectionHeader : public Component
{
public:
    explicit SectionHeader (String text)
        : label ("", text)
    {
        addAndMakeVisible (label);
        setInterceptsMouseClicks (false, false);
        setSize (1, Utils::standardControlHeight);
    }

    void resized() override
    {
        label.setBounds (getLocalBounds());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (TextEditor::ColourIds::highlightColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 2.0f);
    }

    void set (String str)
    {
        label.setText (str, dontSendNotification);
    }

private:
    Label label;
};

class ToggleSectionHeader : public Component
{
public:
    ToggleSectionHeader (String text, State<bool> s)
        : state (s),
          button (text)
    {
        addAndMakeVisible (button);
        setSize (1, Utils::standardControlHeight);
        button.onClick = [this] { state = button.getToggleState(); };
    }

    void resized() override
    {
        button.setBounds (getLocalBounds());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (TextEditor::ColourIds::highlightColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 2.0f);
    }

private:
    State<bool> state;
    ToggleButton button;
    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        button.setToggleState (*state, dontSendNotification);
    });
};

class PropertySectionHeader : public Component
{
public:
    explicit PropertySectionHeader (State<Model::DeviceInfo> s)
        : state (s)
    {
        addAndMakeVisible (toggle);

        addAndMakeVisible (numConcurrentLabel);
        addAndMakeVisible (numConcurrent);
        numConcurrent.setJustification (Justification::centredLeft);

        setSize (1, toggle.getHeight());

        numConcurrent.onCommit ([this]
        {
            const auto clamped = (uint8_t) jlimit (1, 127, numConcurrent.getText().getIntValue());
            state[&Model::DeviceInfo::numPropertyExchangeTransactions] = clamped;
            refresh();
        });
    }

    void resized() override
    {
        toggle.setBounds (getLocalBounds());

        Utils::doTwoColumnLayout (getLocalBounds(), nullptr, numConcurrent);

        Grid grid;
        grid.autoFlow = Grid::AutoFlow::row;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.columnGap = grid.rowGap = Grid::Px { Utils::padding };
        grid.templateColumns = { Grid::TrackInfo { Grid::Fr { 1 } },
                                 Grid::TrackInfo { Grid::Fr { 1 } } };
        grid.items = { GridItem { numConcurrentLabel }, GridItem { numConcurrent } };
        grid.performLayout (numConcurrent.getBounds().reduced (Utils::padding, 4));
    }

private:
    void refresh()
    {
        numConcurrent.set (String (state->numPropertyExchangeTransactions));
    }

    State<Model::DeviceInfo> state;

    ToggleSectionHeader toggle { "Properties", state[&Model::DeviceInfo::propertiesSupported] };
    Label numConcurrentLabel { "", "Max Concurrent Streams" };
    TextField<Editable::yes> numConcurrent;

    std::vector<ErasedScopeGuard> listeners = Utils::makeVector
    (
        state[&Model::DeviceInfo::propertiesSupported].observe ([this] (auto)
        {
            const auto enabled = state->propertiesSupported;
            numConcurrentLabel.setEnabled (enabled);
            numConcurrent.setEnabled (enabled);
        }),
        state[&Model::DeviceInfo::numPropertyExchangeTransactions].observe ([this] (auto)
        {
            refresh();
        })
    );
};

class HeterogeneousListContents : public Component
{
public:
    void addItem (Component& item)
    {
        contents.emplace_back (&item);
        addAndMakeVisible (item);
    }

    void clear()
    {
        for (auto* c : contents)
            removeChildComponent (c);

        contents.clear();
    }

    void resized() override
    {
        auto y = 0;

        for (auto* item : contents)
        {
            item->setBounds ({ 0, y, getWidth(), item->getHeight() });
            y += item->getHeight();
        }
    }

    int getIdealHeight() const
    {
        return std::accumulate (contents.begin(), contents.end(), 0, [] (auto acc, auto& item)
        {
            return acc + item->getHeight();
        });
    }

private:
    std::vector<Component*> contents;
};

class HeterogeneousListView : public Component
{
public:
    HeterogeneousListView()
    {
        addAndMakeVisible (viewport);
        viewport.setViewedComponent (&contents, false);
        viewport.setScrollBarsShown (true, false);
    }

    void resized() override
    {
        viewport.setBounds (getLocalBounds());
        contents.setBounds ({ getWidth(), contents.getIdealHeight() });
    }

    void addItem (Component& item)
    {
        contents.addItem (item);
    }

    void clear()
    {
        contents.clear();
    }

private:
    HeterogeneousListContents contents;
    Viewport viewport;
};

class ChannelStateButtonGrid : public Component
{
public:
    static constexpr auto numChannelColumns = 17;
    static constexpr auto numGroups = 16;

    explicit ChannelStateButtonGrid (State<Model::Profiles> s)
        : state (s)
    {
        auto index = 0;

        for (auto& b : buttons)
        {
            const auto group = index / numChannelColumns;
            const auto channel = index % numChannelColumns;

            addAndMakeVisible (b);
            b.setButtonText ("Group " + String (group) + " Channel " + String (channel));
            b.onClick = [this, group, channel]
            {
                const auto channelKind = channel < 16 ? ci::ChannelInGroup (channel)
                                                      : ci::ChannelInGroup::wholeGroup;
                const auto address = ci::ChannelAddress{}.withGroup (group)
                                                         .withChannel (channelKind);

                auto selected = state[&Model::Profiles::selectedChannel];

                if (*selected != address)
                    selected = address;
                else
                    selected = std::nullopt;
            };

            ++index;
        }

        addAndMakeVisible (block);
        block.setButtonText ("Function Block");
        block.setDimensions ({ numChannelColumns, 1.0f });
        block.onClick = [this]
        {
            const auto address = ci::ChannelAddress{}.withChannel (ci::ChannelInGroup::wholeBlock);

            auto selected = state[&Model::Profiles::selectedChannel];

            if (*selected != address)
                selected = address;
            else
                selected = std::nullopt;
        };

        updateButtonVisibility();
    }

    void resized() override
    {
        const auto availableBounds = getLocalBounds().reduced (Utils::padding)
                                                     .withTrimmedLeft (labelWidth)
                                                     .withTrimmedTop (labelHeight);
        const auto maxButtonSize = std::min (availableBounds.getWidth() / numChannelColumns,
                                             availableBounds.getHeight() / (numGroups + 1));

        Grid grid;
        grid.autoFlow = Grid::AutoFlow::row;
        grid.rowGap = grid.columnGap = Grid::Px { 1 };
        grid.autoColumns = grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.templateColumns = []
        {
            Array<Grid::TrackInfo> array;
            for (auto i = 0; i < 17; ++i)
                array.add (Grid::TrackInfo { Grid::Fr { 1 } });
            return array;
        }();

        for (auto& b : buttons)
            grid.items.add (GridItem { b });

        grid.items.add (GridItem { block }.withArea ({}, GridItem::Span { numChannelColumns }));

        const Rectangle<int> gridBounds { maxButtonSize * numChannelColumns,
                                          maxButtonSize * (numGroups + 1) };
        const RectanglePlacement placement(RectanglePlacement::yTop | RectanglePlacement::xMid);
        grid.performLayout (placement.appliedTo (gridBounds, availableBounds));

        block.setDimensions (block.getBounds().toFloat());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (Label::ColourIds::textColourId));

        const auto groupWidth = 100;
        GlyphArrangement groupArrangement;
        groupArrangement.addJustifiedText (FontOptions{},
                                           "Group",
                                           0,
                                           0,
                                           groupWidth,
                                           Justification::horizontallyCentred);

        Path groupPath;
        groupArrangement.createPath (groupPath);

        g.fillPath (groupPath,
                    AffineTransform().rotated (-MathConstants<float>::halfPi, groupWidth / 2, 0)
                                     .translated (-groupWidth / 2, 0)
                                     .translated (Point { buttons[0].getX() - 40,
                                                          buttons[8 * 17].getY() }.toFloat()));

        for (auto i = 0; i < 16; ++i)
        {
            const auto bounds = buttons[(size_t) i * 17].getBounds();
            g.drawSingleLineText (String (i + 1),
                                  bounds.getX() - Utils::padding,
                                  bounds.getBottom(),
                                  Justification::right);
        }

        g.drawSingleLineText ("Block",
                              block.getX() - Utils::padding,
                              block.getBottom(),
                              Justification::right);

        g.drawSingleLineText ("Channel",
                              buttons[8].getBounds().getCentreX(),
                              buttons[0].getY() - 40,
                              Justification::horizontallyCentred);

        for (auto i = 0; i < 17; ++i)
        {
            const auto bounds = buttons[(size_t) i].getBounds();

            GlyphArrangement channelArrangement;
            channelArrangement.addJustifiedText (FontOptions{},
                                                 i < 16 ? String (i + 1) : "All",
                                                 0,
                                                 0,
                                                 groupWidth,
                                                 Justification::left);

            Path channelPath;
            channelArrangement.createPath (channelPath);

            const auto transform = AffineTransform()
                                       .rotated (-MathConstants<float>::halfPi * 0.8, 0, 0)
                                       .translated (Point { bounds.getRight(),
                                                            bounds.getY() - Utils::padding }
                                                        .toFloat());
            g.fillPath (channelPath, transform);
        }

        if (auto selected = state->selectedChannel)
        {
            auto buttonBounds = getButtonForAddress (*selected).getBounds();
            g.setColour (Colours::cyan);
            g.drawRect (buttonBounds, 2);
        }
    }

private:
    class Button : public ShapeButton
    {
    public:
        Button()
            : ShapeButton ("",
                           Colours::black.withAlpha (0.4f),
                           Colours::black.withAlpha (0.4f).brighter(),
                           Colours::black.withAlpha (0.4f).darker())
        {
            setDimensions ({ 1.0f, 1.0f });

            const auto onColour = findColour (TextEditor::ColourIds::highlightColourId);
            setOnColours (onColour, onColour.brighter(), onColour.darker());
            shouldUseOnColours (true);
            setClickingTogglesState (true);
        }

        void setDimensions (Rectangle<float> dimensions)
        {
            Path s;
            s.addRectangle (dimensions);
            setShape (s, false, true, false);
        }
    };

    Button& getButtonForAddress (ci::ChannelAddress address)
    {
        if (address.isBlock())
            return block;

        const auto channelIndex = address.getChannel() != ci::ChannelInGroup::wholeGroup
                                ? (size_t) address.getChannel()
                                : 16;
        const auto buttonIndex = (size_t) address.getGroup() * numChannelColumns + channelIndex;
        return buttons[buttonIndex];
    }

    void updateButtonVisibility()
    {
        if (auto* profile = state->profiles.getSelected())
        {
            Utils::forAllChannelAddresses ([&] (auto address)
            {
                auto& button = getButtonForAddress (address);

                const auto iter = state->channels.find ({ *profile, address });

                const auto visible = state->profileMode == Model::ProfileMode::edit
                                     || iter != state->channels.end();
                button.setVisible (visible);

                const auto lit = [&]
                {
                    if (iter == state->channels.end())
                        return false;

                    if (state->profileMode == Model::ProfileMode::edit)
                        return iter->second.supported != 0;

                    return iter->second.active != 0;
                }();

                button.setToggleState (lit, dontSendNotification);
            });
        }
    }

    static constexpr auto labelWidth = 50;
    static constexpr auto labelHeight = 50;
    State<Model::Profiles> state;
    std::array<Button, numGroups * numChannelColumns> buttons;
    Button block;

    ErasedScopeGuard listener = state.observe ([this] (const auto& old)
    {
        if (old.selectedChannel != state->selectedChannel)
            repaint();

        updateButtonVisibility();
    });
};

class ProfileListModel : public ListBoxModel
{
public:
    explicit ProfileListModel (State<Model::Profiles> s)
        : state (s) {}

    int getNumRows() override
    {
        return (int) state->profiles.items.size();
    }

    void paintListBoxItem (int, Graphics&, int, int, bool) override {}

    Component* refreshComponentForRow (int rowNumber,
                                       bool rowIsSelected,
                                       Component* existingComponentToUpdate) override
    {
        const auto toDelete = rawToUniquePtr (existingComponentToUpdate);

        const auto currentState = *state;

        if (auto* item = currentState.profiles.get (rowNumber))
        {
            const auto name = String::toHexString (item->data(), (int) item->size());
            return Utils::makeListRowLabel (name, rowIsSelected).release();
        }

        return nullptr;
    }

    void selectedRowsChanged (int newSelection) override
    {
        state[&Model::Profiles::profiles]
             [&Model::ListWithSelection<ci::Profile>::selection] = newSelection;
        state[&Model::Profiles::selectedChannel] = std::nullopt;
    }

private:
    State<Model::Profiles> state;
};

template <Editable editable>
class ProfileList : public Component
{
public:
    explicit ProfileList (State<Model::Profiles> s)
        : state (s)
    {
        addAndMakeVisible (list);

        if constexpr (editable == Editable::yes)
        {
            addAndMakeVisible (add);
            add.onClick = [this]
            {
                auto updated = *state;
                updated.profiles.items.emplace_back();
                updated.profiles.selection = (int) updated.profiles.items.size() - 1;
                updated.selectedChannel = std::nullopt;
                state = std::move (updated);
            };

            addAndMakeVisible (remove);
            remove.onClick = [this]
            {
                auto updated = *state;

                if (auto* item = updated.profiles.getSelected())
                {
                    const auto toErase = *item;

                    Utils::forAllChannelAddresses ([&] (auto address)
                    {
                        updated.channels.erase (ci::ProfileAtAddress { toErase, address });
                    });

                    const auto erase = updated.profiles.items.begin() + updated.profiles.selection;
                    updated.profiles.items.erase (erase);
                    updated.profiles.selection = -1;

                    state = std::move (updated);
                }
            };
        }
    }

    void resized() override
    {
        if constexpr (editable == Editable::yes)
        {
            FlexBox fb;
            fb.flexDirection = FlexBox::Direction::column;
            fb.items = { FlexItem { list }.withFlex (1),
                         FlexItem{}.withHeight (Utils::padding),
                         FlexItem{}.withHeight (Utils::standardControlHeight) };
            fb.performLayout (getLocalBounds());
            Utils::doColumnLayout (fb.items.getLast().currentBounds.getSmallestIntegerContainer(),
                                   add,
                                   remove);
        }
        else
        {
            list.setBounds (getLocalBounds());
        }
    }

private:
    State<Model::Profiles> state;
    ProfileListModel model { state };
    ListBox list { "Profiles", &model };
    TextButton add { "+" }, remove { "-" };

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        list.setSelectedRows ({}, dontSendNotification);
        list.updateContent();

        const auto& profs = state->profiles;
        auto* item = profs.getSelected();

        remove.setEnabled (item != nullptr);

        if (item != nullptr)
        {
            list.setSelectedRows (Utils::setWithSingleIndex (profs.selection),
                                  dontSendNotification);
        }
    });
};

class ProfileNamePane : public Component
{
public:
    explicit ProfileNamePane (State<Model::Profiles> s)
        : state (s)
    {
        addAndMakeVisible (label);
        addAndMakeVisible (field);
        field.onCommit ([this]
        {
            auto updated = *state;

            if (auto* item = updated.profiles.getSelected())
            {
                Utils::stringToByteArray (field.getText(), *item);
                state = std::move (updated);
            }

            refresh();
        });
    }

    void resized() override
    {
        Utils::doColumnLayout (getLocalBounds(), label, field);
    }

private:
    void refresh()
    {
        if (auto* item = state->profiles.getSelected())
            field.set (String::toHexString (item->data(), (int) item->size()));
    }

    State<Model::Profiles> state;
    Label label { "", "Profile Identifier" };
    TextField<Editable::yes> field;

    ErasedScopeGuard listener = state.observe ([this] (auto) { refresh(); });
};

class NumChannelsPane : public Component
{
public:
    explicit NumChannelsPane (State<Model::Profiles> s)
        : state (s)
    {
        addAndMakeVisible (label);
        addAndMakeVisible (field);
        field.onCommit ([this]
        {
            NullCheckedInvocation::invoke (onChannelsRequested,
                                           (uint16_t) field.getText().getIntValue());
            refresh();
        });
    }

    void resized() override
    {
        Utils::doColumnLayout (getLocalBounds(), label, field);
    }

    std::function<void (uint16_t)> onChannelsRequested;

private:
    void refresh()
    {
        const auto attributes = [&]() -> ci::SupportedAndActive
        {
            const auto selected = state->getSelectedProfileAtAddress();

            if (! selected.has_value())
                return {};

            const auto iter = state->channels.find (*selected);

            if (iter == state->channels.end())
                return {};

            return iter->second;
        }();

        const auto numToShow = state->profileMode == Model::ProfileMode::edit
                             ? attributes.supported
                             : attributes.active;
        field.set (String (numToShow));
    }

    State<Model::Profiles> state;
    Label label;
    TextField<Editable::yes> field;

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        const auto text = state->profileMode == Model::ProfileMode::edit
                        ? "Num Supported Channels"
                        : "Num Active Channels";
        label.setText (text, dontSendNotification);
        refresh();
    });
};

template <Editable editable>
class ProfileDetailsPane : public Component
{
public:
    explicit ProfileDetailsPane (State<Model::Profiles> s)
        : state (s)
    {
        if constexpr (editable == Editable::yes)
        {
            addAndMakeVisible (name);
            addAndMakeVisible (edit);
            edit.setClickingTogglesState (false);
            edit.onClick = [this]
            {
                state[&Model::Profiles::profileMode] = Model::ProfileMode::edit;
            };

            addAndMakeVisible (use);
            use.setClickingTogglesState (false);
            use.onClick = [this]
            {
                state[&Model::Profiles::profileMode] = Model::ProfileMode::use;
            };

            channels.onChannelsRequested = [this] (auto numChannels)
            {
                auto updated = *state;
                const auto selected = updated.getSelectedProfileAtAddress();

                if (! selected.has_value())
                    return;

                auto& value = updated.channels[*selected];

                if (updated.profileMode == Model::ProfileMode::edit)
                    value.supported = jmax ((uint16_t) 0, numChannels);
                else
                    value.active = jlimit ((uint16_t) 0, value.supported, numChannels);

                state = std::move (updated);
            };

            invert.onClick = [this]
            {
                if (const auto selected = state->getSelectedProfileAtAddress())
                {
                    auto updated = *state;
                    auto& value = updated.channels[*selected];

                    auto& toInvert = updated.profileMode == Model::ProfileMode::edit
                                     ? value.supported
                                     : value.active;

                    toInvert = toInvert == 0 ? 1 : 0;
                    state = std::move (updated);
                }
            };
        }
        else
        {
            channels.onChannelsRequested = Utils::forwardFunction (onChannelsRequested);

            invert.onClick = [this]
            {
                if (const auto selected = state->getSelectedProfileAtAddress())
                {
                    const auto iter = state->channels.find (*selected);

                    if (iter == state->channels.end())
                        return;

                    auto& toInvert = state->profileMode == Model::ProfileMode::edit
                                     ? iter->second.supported
                                     : iter->second.active;

                    NullCheckedInvocation::invoke (onChannelsRequested,
                                                   (uint16) (toInvert == 0 ? 1 : 0));
                }
            };
        }

        addAndMakeVisible (grid);
        addAndMakeVisible (channels);
        addAndMakeVisible (invert);
    }

    void resized() override
    {
        Grid g;
        g.autoFlow = Grid::AutoFlow::row;
        g.autoColumns = g.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        g.columnGap = g.rowGap = Grid::Px { Utils::padding };
        g.templateColumns = { Grid::TrackInfo { Grid::Fr { 1 } },
                              Grid::TrackInfo { Grid::Fr { 1 } } };

        if constexpr (editable == Editable::yes)
        {
            g.templateRows = { Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                               Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                               Grid::TrackInfo { Grid::Fr { 1 } },
                               Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                               Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } } };
            g.items = { GridItem { name }.withArea ({}, GridItem::Span { 2 }),
                        GridItem { edit }, GridItem { use },
                        GridItem { grid }.withArea ({}, GridItem::Span { 2 }),
                        GridItem { invert }.withArea ({}, GridItem::Span { 2 }),
                        GridItem { channels }.withArea ({}, GridItem::Span { 2 }) };
        }
        else
        {
            g.templateRows = { Grid::TrackInfo { Grid::Fr { 1 } },
                               Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                               Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } } };
            g.items = { GridItem { grid }.withArea ({}, GridItem::Span { 2 }),
                        GridItem { invert }.withArea ({}, GridItem::Span { 2 }),
                        GridItem { channels }.withArea ({}, GridItem::Span { 2 }) };
        }

        g.performLayout (getLocalBounds());
    }

    std::function<void (uint16_t)> onChannelsRequested;

private:
    State<Model::Profiles> state;
    ProfileNamePane name { state };
    ToggleButton edit { "Show Supported Channels" }, use { "Show Active Channels" };
    TextButton invert { "Toggle Member Channels" };
    ChannelStateButtonGrid grid { state };
    NumChannelsPane channels { state };

    std::vector<ErasedScopeGuard> listeners = Utils::makeVector
    (
        state[&Model::Profiles::profileMode].observe ([this] (auto)
        {
            edit.setToggleState (state->profileMode == Model::ProfileMode::edit,
                                 dontSendNotification);
            use.setToggleState (state->profileMode == Model::ProfileMode::use,
                                dontSendNotification);
        }),
        state.observe ([this] (auto)
        {
            invert.setVisible (state->getSelectedProfileAtAddress().has_value());
            channels.setVisible (state->getSelectedProfileAtAddress().has_value());
        })
    );
};

template <Editable editable>
class ProfileConfigPanel : public Component
{
public:
    explicit ProfileConfigPanel (State<Model::Profiles> s)
        : state (s)
    {
        setSize (1, 500);

        addAndMakeVisible (list);
    }

    void resized() override
    {
        auto* d = details.has_value() ? &*details : nullptr;
        Utils::doTwoColumnLayout (getLocalBounds().reduced (Utils::padding), list, d);
    }

    std::function<void (uint16_t)> onChannelsRequested;

private:
    State<Model::Profiles> state;
    ProfileList<editable> list { state };
    std::optional<ProfileDetailsPane<editable>> details;

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        if (state->profiles.getSelected() != nullptr)
        {
            if (! details.has_value())
            {
                addAndMakeVisible (details.emplace (state));
                details->onChannelsRequested = Utils::forwardFunction (onChannelsRequested);
                resized();
            }
        }
        else
        {
            details.reset();
        }
    });
};

template <Editable editable>
class DiscoveryInfoPanel : public Component
{
public:
    DiscoveryInfoPanel (State<ci::MUID> m, State<Model::DeviceInfo> s)
        : muidState (m), state (s)
    {
        const auto setStateCallback = [this] { setStateFromUI(); };
        [&] (auto&&... item)
        {
            (addAndMakeVisible (item), ...);
            ((item.onCommit (setStateCallback)), ...);
        } (manufacturer, family, modelNumber, revision, maxSysExSize);

        [&] (auto&&... item)
        {
            (addAndMakeVisible (item), ...);
        } (muid,
           muidLabel,
           manufacturerLabel,
           familyLabel,
           modelNumberLabel,
           revisionLabel,
           maxSysExSizeLabel);

        setSize (1, 6 * Utils::standardControlHeight + 7 * Utils::padding);
    }

    void resized() override
    {
        Utils::doTwoColumnLayout (getLocalBounds().reduced (Utils::padding),
                                  muidLabel, muid,
                                  maxSysExSizeLabel, maxSysExSize,
                                  manufacturerLabel, manufacturer,
                                  familyLabel, family,
                                  modelNumberLabel, modelNumber,
                                  revisionLabel, revision);
    }

private:
    void setStateFromUI()
    {
        auto updated = *state;

        Utils::stringToByteArray (manufacturer.getText(), updated.deviceInfo.manufacturer);
        Utils::stringToByteArray (family      .getText(), updated.deviceInfo.family);
        Utils::stringToByteArray (modelNumber .getText(), updated.deviceInfo.modelNumber);
        Utils::stringToByteArray (revision    .getText(), updated.deviceInfo.revision);
        updated.maxSysExSize = (size_t) maxSysExSize.getText().getIntValue();

        state = std::move (updated);
        refresh();
    }

    void refresh()
    {
        maxSysExSize.set (String (state->maxSysExSize));

        auto& info = state->deviceInfo;
        manufacturer.set (byteArrayToString (info.manufacturer));
        family      .set (byteArrayToString (info.family));
        modelNumber .set (byteArrayToString (info.modelNumber));
        revision    .set (byteArrayToString (info.revision));
    }

    static String byteArrayToString (Span<const std::byte> bytes)
    {
        return String::toHexString (bytes.data(), (int) bytes.size());
    }

    State<ci::MUID> muidState;
    State<Model::DeviceInfo> state;

    Label muidLabel             { "", "MUID (hex)" },
          maxSysExSizeLabel     { "", "Maximum SysEx size (decimal)" },
          manufacturerLabel     { "", "Manufacturer (hex, 3 bytes)" },
          familyLabel           { "", "Family (hex, 2 bytes)" },
          modelNumberLabel      { "", "Model (hex, 2 bytes)" },
          revisionLabel         { "", "Revision (hex, 4 bytes)" };

    TextField<Editable::no> muid;
    TextField<editable> maxSysExSize, manufacturer, family, modelNumber, revision;

    std::vector<ErasedScopeGuard> listeners = Utils::makeVector
    (
        muidState.observe ([this] (auto)
        {
            muid.set (String::toHexString (muidState->get()));
        }),
        state.observe ([this] (auto) { refresh(); })
    );
};

class PropertyListModel : public ListBoxModel
{
public:
    explicit PropertyListModel (State<Model::Properties> s)
        : state (s) {}

    int getNumRows() override
    {
        return (int) state->properties.items.size();
    }

    void paintListBoxItem (int, Graphics&, int, int, bool) override {}

    Component* refreshComponentForRow (int rowNumber,
                                       bool rowIsSelected,
                                       Component* existingComponentToUpdate) override
    {
        const auto toDelete = rawToUniquePtr (existingComponentToUpdate);

        const auto currentState = *state;

        if (auto* item = currentState.properties.get (rowNumber))
        {
            const auto name = item->name;
            return Utils::makeListRowLabel (name, rowIsSelected).release();
        }

        return nullptr;
    }

    void selectedRowsChanged (int newSelection) override
    {
        state[&Model::Properties::properties]
             [&Model::ListWithSelection<Model::Property>::selection] = newSelection;
    }

private:
    State<Model::Properties> state;
};

template <Editable editable>
class PropertyList : public Component
{
public:
    explicit PropertyList (State<Model::Properties> s)
        : state (s)
    {
        addAndMakeVisible (list);

        if constexpr (editable == Editable::yes)
        {
            addAndMakeVisible (add);
            add.onClick = [this]
            {
                auto updated = *state;
                updated.properties.items.emplace_back();
                updated.properties.selection = (int) updated.properties.items.size() - 1;
                state = std::move (updated);
            };

            addAndMakeVisible (remove);
            remove.onClick = [this]
            {
                auto updated = *state;
                auto& props = updated.properties;

                if (0 <= props.selection)
                {
                    props.items.erase (props.items.begin() + props.selection);
                    props.selection = -1;

                    state = std::move (updated);
                }
            };
        }
    }

    void resized() override
    {
        if constexpr (editable == Editable::yes)
        {
            FlexBox fb;
            fb.flexDirection = FlexBox::Direction::column;
            fb.items = { FlexItem { list }.withFlex (1),
                         FlexItem{}.withHeight (Utils::padding),
                         FlexItem{}.withHeight (Utils::standardControlHeight) };
            fb.performLayout (getLocalBounds());

            Utils::doColumnLayout (fb.items.getLast().currentBounds.getSmallestIntegerContainer(),
                                   add,
                                   remove);
        }
        else
        {
            list.setBounds (getLocalBounds());
        }
    }

private:
    State<Model::Properties> state;
    PropertyListModel model { state };
    ListBox list { "Profiles", &model };
    TextButton add { "+" }, remove { "-" };

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        list.setSelectedRows ({}, dontSendNotification);
        list.updateContent();

        const auto& props = state->properties;
        auto* item = props.getSelected();

        remove.setEnabled (item != nullptr);

        if (item != nullptr)
        {
            list.setSelectedRows (Utils::setWithSingleIndex (props.selection),
                                  dontSendNotification);
        }
    });
};

class PropertySubscribersModel : public ListBoxModel
{
public:
    explicit PropertySubscribersModel (State<Model::App> s) : state (s) {}

    int getNumRows() override { return (int) entries.size(); }

    void paintListBoxItem (int, Graphics&, int, int, bool) override {}

    Component* refreshComponentForRow (int rowNumber,
                                       bool,
                                       Component* existingComponentToUpdate) override
    {
        const auto toDelete = rawToUniquePtr (existingComponentToUpdate);

        if (! isPositiveAndBelow (rowNumber, entries.size()))
            return nullptr;

        const auto info = entries[(size_t) rowNumber];
        return Utils::makeListRowLabel (info, false).release();
    }

    void refresh()
    {
        entries = [&]() -> std::vector<String>
        {
            const auto& props = state->saved.properties;

            if (auto* item = props.properties.getSelected())
            {
                const auto selected = item->name;
                const auto& subs = state->transient.subscribers;
                const auto iter = subs.find (selected);

                if (iter == subs.end())
                    return {};

                std::vector<String> result;

                for (const auto& [device, subscriptions] : iter->second)
                    for (const auto& s : subscriptions)
                        result.push_back (String::toHexString (device.get()) + " (" + s + ")");

                return result;
            }

            return {};
        }();
    }

private:
    State<Model::App> state;
    std::vector<String> entries;
};

template <Editable editable>
class PropertySubscribersPanel : public Component
{
public:
    explicit PropertySubscribersPanel (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (list);
        list.setMultipleSelectionEnabled (false);
        list.setClickingTogglesRowSelection (false);
    }

    void resized() override
    {
        list.setBounds (getLocalBounds().reduced (Utils::padding));
    }

private:
    State<Model::App> state;
    PropertySubscribersModel model { state };
    ListBox list { "Subscribers", &model };

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        model.refresh();
        list.updateContent();
    });
};

template <Editable editable>
class PropertyValuePanel : public Component
{
    enum class Kind { full, partial };

    auto getFileChooserCallback (Kind kind)
    {
        return [this, kind] (const auto&)
        {
            if (propertyFileChooser.getResults().isEmpty())
                return;

            auto updated = *state;

            if (auto* item = updated.properties.getSelected())
            {
                MemoryBlock block;
                propertyFileChooser.getResult().loadFileAsData (block);
                const auto* data = static_cast<const std::byte*> (block.getData());

                if constexpr (editable == Editable::yes)
                {
                    std::vector<std::byte> asByteVec (data, data + block.getSize());

                    if (kind == Kind::full)
                    {
                        item->value.bytes = std::move (asByteVec);
                    }
                    else
                    {
                        struct Header
                        {
                            String mediaType;
                            ci::Encoding mutualEncoding{};
                        };

                        struct Request
                        {
                            Header header;
                            std::vector<std::byte> body;
                        };

                        Request request { { "application/json", ci::Encoding::ascii },
                                          std::move (asByteVec) };

                        auto [newItem, error] = Utils::attemptSetPartial (std::move (*item),
                                                                          std::move (request));
                        *item = std::move (newItem);
                        jassert (error.isEmpty()); // Inspect error to find out what went wrong
                    }

                    state = std::move (updated);
                }
                else
                {
                    NullCheckedInvocation::invoke (kind == Kind::full ? onSetFullRequested
                                                                      : onSetPartialRequested,
                                                   Span (data, block.getSize()));
                }
            }
        };
    }

public:
    explicit PropertyValuePanel (State<Model::Properties> s)
        : PropertyValuePanel (s, {}) {}

    PropertyValuePanel (State<Model::Properties> s, State<std::map<ci::SubscriptionKey, ci::Subscription>> subState)
        : state (s), subscriptions (subState)
    {
        addAndMakeVisible (value);
        value.setReadOnly (true);
        value.setMultiLine (true);

        addAndMakeVisible (formatLabel);
        addAndMakeVisible (format);

        format.onCommit ([this]
        {
            auto adjusted = *state;

            if (auto* item = adjusted.properties.getSelected())
            {
                item->value.mediaType = format.getText();
                state = adjusted;
            }

            refresh();
        });

        addAndMakeVisible (viewAsHex);
        viewAsHex.onClick = [this]
        {
            state[&Model::Properties::mode] = Model::DataViewMode::hex;
        };

        addAndMakeVisible (viewAsAscii);
        viewAsAscii.onClick = [this]
        {
            state[&Model::Properties::mode] = Model::DataViewMode::ascii;
        };

        addAndMakeVisible (setFull);
        setFull.onClick = [this]
        {
            propertyFileChooser.launchAsync (FileBrowserComponent::canSelectFiles
                                             | FileBrowserComponent::openMode,
                                             getFileChooserCallback (Kind::full));
        };

        addAndMakeVisible (setPartial);
        setPartial.onClick = [this]
        {
            propertyFileChooser.launchAsync (FileBrowserComponent::canSelectFiles
                                             | FileBrowserComponent::openMode,
                                             getFileChooserCallback (Kind::partial));
        };

        if constexpr (editable == Editable::no)
        {
            addAndMakeVisible (get);
            get.onClick = Utils::forwardFunction (onGetRequested);

            addAndMakeVisible (subscribe);
            subscribe.onClick = Utils::forwardFunction (onSubscribeRequested);
        }
    }

    void resized() override
    {
        Grid grid;
        grid.autoFlow = Grid::AutoFlow::row;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo { Grid::Fr { 1 } };
        grid.columnGap = grid.rowGap = Grid::Px { Utils::padding };
        grid.templateColumns = { Grid::TrackInfo { Grid::Fr { 1 } },
                                 Grid::TrackInfo { Grid::Fr { 1 } } };
        if constexpr (editable == Editable::yes)
        {
            grid.templateRows = { Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Fr { 1 } } };
            grid.items = { GridItem { setFull }, GridItem { setPartial },
                           GridItem { formatLabel }, GridItem { format },
                           GridItem { viewAsHex },   GridItem { viewAsAscii },
                           GridItem { value }.withArea ({}, GridItem::Span { 2 }) };
        }
        else
        {
            grid.templateRows = { Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Px { Utils::standardControlHeight } },
                                  Grid::TrackInfo { Grid::Fr { 1 } } };
            grid.items = { GridItem { setFull }, GridItem { setPartial },
                           GridItem { get }, GridItem { subscribe },
                           GridItem { formatLabel }, GridItem { format },
                           GridItem { viewAsHex },   GridItem { viewAsAscii },
                           GridItem { value }.withArea ({}, GridItem::Span { 2 }) };
        }
        grid.performLayout (getLocalBounds().reduced (Utils::padding));
    }

    std::function<void()> onGetRequested, onSubscribeRequested;
    std::function<void (Span<const std::byte>)> onSetFullRequested, onSetPartialRequested;

private:
    void refresh()
    {
        const auto& currentState = *state;

        if (auto* item = currentState.properties.getSelected())
            format.set (item->value.mediaType);

        const auto mode = state->mode;
        viewAsHex.setToggleState (mode == Model::DataViewMode::hex, dontSendNotification);
        viewAsAscii.setToggleState (mode == Model::DataViewMode::ascii, dontSendNotification);
    }

    void updateSubButtonText()
    {
        const auto& sub = *subscriptions;

        if (const auto* selectedProp = state->properties.getSelected())
        {
            const auto text = std::any_of (sub.begin(), sub.end(), [&] (const auto& p) { return p.second.resource == selectedProp->name; }) ? "Unsubscribe" : "Subscribe";
            subscribe.setButtonText (text);
        }
    }

    State<Model::Properties> state;
    State<std::map<ci::SubscriptionKey, ci::Subscription>> subscriptions;

    MonospaceEditor value;
    TextField<editable> format;
    Label formatLabel { "", "Media Type" };
    ToggleButton viewAsHex { "Hex" };
    ToggleButton viewAsAscii { "ASCII" };
    TextButton setFull { "Set Full..." },
               setPartial { "Set Partial..." },
               get { "Get" },
               subscribe { "Subscribe" };

    FileChooser propertyFileChooser { "Property Data", {}, "*", true, false, this };

    std::vector<ErasedScopeGuard> listeners = Utils::makeVector
    (
        state.observe ([this] (const auto& old)
        {
            updateSubButtonText();
            refresh();

            const auto& currentState = *state;

            if (auto* item = currentState.properties.getSelected())
            {
                const auto mode = currentState.mode;

                if (mode == old.mode
                    && old.properties.getSelected() != nullptr
                    && *old.properties.getSelected() == *item)
                {
                    return;
                }

                constexpr auto isEditable = editable == Editable::yes;
                const auto canSetFull = item->canSet != Model::CanSet::none
                                        || isEditable;
                setFull.setEnabled (canSetFull);
                setPartial.setEnabled (item->canSet == Model::CanSet::partial);
                get.setEnabled (item->canGet);

                auto* oldSelection = old.properties.getSelected();
                const auto needsValueUpdate = old.mode != mode
                                              || oldSelection == nullptr
                                              || oldSelection->value.bytes != item->value.bytes;

                if (! needsValueUpdate)
                    return;

                value.set ("");

                switch (mode)
                {
                    case Model::DataViewMode::hex:
                        value.setColour (TextEditor::ColourIds::textColourId,
                                         findColour (TextEditor::ColourIds::textColourId));
                        value.setText (String::toHexString (item->value.bytes.data(),
                                                            (int) item->value.bytes.size()),
                                       false);
                        break;

                    case Model::DataViewMode::ascii:
                        String toShow;

                        for (auto& b : item->value.bytes)
                        {
                            const char ascii[] { (char) b, 0 };
                            toShow << (b < std::byte { 0x80 } ? ascii : "\xef\xbf\xbd");
                        }

                        value.set (toShow);
                        break;
                }
            }
        }),
        subscriptions.observe ([this] (const auto&)
        {
            updateSubButtonText();
        })
    );
};

template <Editable editable>
class PropertyInfoPanel : public Component
{
public:
    explicit PropertyInfoPanel (State<Model::Properties> s)
        : state (s)
    {
        constexpr auto isEditable = editable == Editable::yes;

        if constexpr (isEditable)
        {

            addAndMakeVisible (canSet);
            canSet.addItemList ({ "None", "Full", "Partial" }, 1);
            canSet.onChange = [this] { updateStateFromUI(); };
        }
        else
        {
            addAndMakeVisible (canSetField);
        }

        const auto updateStateCallback = [this] { updateStateFromUI(); };

        [&] (auto&&... args)
        {
            (addAndMakeVisible (args), ...);
            (args.setClickingTogglesState (isEditable), ...);
            ((args.onClick = updateStateCallback), ...);
        } (canGet,
           canSubscribe,
           canPaginate,
           requireResId,
           canAsciiEncode,
           canMcoded7Encode,
           canZipMcoded7Encode);

        [&] (auto&&... args)
        {
            (addAndMakeVisible (args), ...);
        } (nameLabel, schemaLabel, mediaTypesLabel, columnsLabel, canSetLabel, listLabel);

        [&] (auto&&... args)
        {
            (addAndMakeVisible (args), ...);
            (args.setReadOnly (! isEditable), ...);
            (args.setMultiLine (true), ...);
            ((args.onReturnKey = args.onEscapeKey
                               = args.onFocusLost
                               = updateStateCallback), ...);
        } (schema, mediaTypes, columns);

        addAndMakeVisible (name);
        name.onCommit ([this]
        {
            updateStateFromUI();
            refresh();
        });

        updateUIFromState();
    }

    void resized() override
    {
        Grid grid;
        grid.autoFlow = Grid::AutoFlow::row;
        grid.autoColumns = Grid::TrackInfo { Grid::Fr { 1 } };
        const Grid::TrackInfo tallRow { Grid::Px { 100 } },
                              shortRow {  Grid::Px { Utils::standardControlHeight } };
        grid.columnGap = grid.rowGap = Grid::Px { Utils::padding };
        grid.templateColumns = { Grid::TrackInfo { Grid::Fr { 1 } },
                                 Grid::TrackInfo { Grid::Fr { 1 } } };
        grid.templateRows = { shortRow,
                              tallRow,
                              tallRow,
                              shortRow,
                              shortRow,
                              shortRow,
                              shortRow,
                              shortRow,
                              shortRow,
                              tallRow };
        auto* canSetControl = editable == Editable::yes ? static_cast<Component*> (&canSet)
                                                        : static_cast<Component*> (&canSetField);
        grid.items = { GridItem { nameLabel }, GridItem { name },
                       GridItem { mediaTypesLabel }, GridItem { mediaTypes },
                       GridItem { schemaLabel }, GridItem { schema },
                       GridItem { canSetLabel }, GridItem { canSetControl },
                       GridItem { canGet }, GridItem { canSubscribe },
                       GridItem { requireResId }, GridItem { canAsciiEncode },
                       GridItem { canMcoded7Encode }, GridItem { canZipMcoded7Encode },
                       GridItem { listLabel }.withArea ({}, GridItem::Span { 2 }),
                       GridItem { canPaginate }.withArea ({}, GridItem::Span { 2 }),
                       GridItem { columnsLabel }, GridItem { columns } };
        grid.performLayout (getLocalBounds().reduced (Utils::padding));
    }

private:
    void refresh()
    {
        const auto currentState = *state;

        if (auto* item = state->properties.getSelected())
            name.set (item->name);
    }

    void updateStateFromUI()
    {
        if constexpr (editable == Editable::yes)
        {
            auto updated = *state;
            auto& props = updated.properties;

            if (auto* item = props.getSelected())
            {
                auto cachedData = item->value;
                *item = getInfoFromUI();
                item->value = cachedData;

                state = std::move (updated);
            }
        }
    }

    Model::Property getInfoFromUI() const
    {
        Model::Property result;
        result.name = name.getText();
        result.schema = JSON::fromString (schema.getText());
        auto lines = StringArray::fromLines (mediaTypes.getText() + "\n");
        lines.removeEmptyStrings();
        result.mediaTypes = std::vector<String> (lines.begin(), lines.end());
        result.columns = JSON::fromString (columns.getText());

        result.encodings = [&]
        {
            std::set<ci::Encoding> encodings;

            if (canAsciiEncode.getToggleState())
                encodings.insert (ci::Encoding::ascii);

            if (canMcoded7Encode.getToggleState())
                encodings.insert (ci::Encoding::mcoded7);

            if (canZipMcoded7Encode.getToggleState())
                encodings.insert (ci::Encoding::zlibAndMcoded7);

            return encodings;
        }();

        result.canSet = (Model::CanSet) canSet.getSelectedItemIndex();
        result.canGet = canGet.getToggleState();
        result.canSubscribe = canSubscribe.getToggleState();
        result.requireResId = requireResId.getToggleState();
        result.canPaginate = canPaginate.getToggleState();
        return result;
    }

    void updateUIFromState()
    {
        refresh();

        const auto currentState = *state;

        if (auto* item = state->properties.getSelected())
        {
            schema.setText (JSON::toString (item->schema), false);

            const auto pairs = { std::tuple (&canAsciiEncode, ci::Encoding::ascii),
                                 std::tuple (&canMcoded7Encode, ci::Encoding::mcoded7),
                                 std::tuple (&canZipMcoded7Encode, ci::Encoding::zlibAndMcoded7) };

            for (const auto& [button, encoding] : pairs)
            {
                button->setToggleState (item->encodings.count (encoding) != 0,
                                        dontSendNotification);
            }

            mediaTypes.setText (StringArray (item->mediaTypes.data(),
                                             (int) item->mediaTypes.size()).joinIntoString ("\n"),
                                false);

            columns.setText (JSON::toString (item->columns), false);

            canSetField.set (Model::CanSetUtils::toString (item->canSet));
            canSet.setSelectedItemIndex ((int) item->canSet, dontSendNotification);
            canGet.setToggleState (item->canGet, dontSendNotification);
            canSubscribe.setToggleState (item->canSubscribe, dontSendNotification);
            requireResId.setToggleState (item->requireResId, dontSendNotification);
            canPaginate.setToggleState (item->canPaginate, dontSendNotification);

            const auto list = item->name.endsWith ("List");
            canPaginate.setEnabled (list);
            columnsLabel.setEnabled (list);
            columns.setEnabled (list);
        }
    }

    State<Model::Properties> state;
    Label nameLabel          { "", "Name" },
          schemaLabel        { "", "Schema" },
          mediaTypesLabel    { "", "Media Types" },
          columnsLabel       { "", "Columns (json array)" },
          canSetLabel        { "", "Can Set" },
          listLabel          { "", "List Properties (only valid when Name ends with \"List\")" };

    TextField<editable> name, canSetField;
    MonospaceEditor schema, mediaTypes, columns;
    ComboBox canSet;
    ToggleButton canGet                 { "Can Get" },
                 canSubscribe           { "Can Subscribe" },
                 canPaginate            { "Can Paginate" },
                 requireResId           { "Require Res ID" },
                 canAsciiEncode         { "Can ASCII Encode" },
                 canMcoded7Encode       { "Can Mcoded7 Encode" },
                 canZipMcoded7Encode    { "Can zlib+Mcoded7 Encode" };

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        updateUIFromState();
    });
};

template <Editable>
class PropertyEditPanel;

template <>
class PropertyEditPanel<Editable::yes> : public Component
{
public:
    explicit PropertyEditPanel (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (tabs);
        tabs.addTab ("Info",
                      findColour (DocumentWindow::backgroundColourId),
                      &info,
                      false);
        tabs.addTab ("Value",
                     findColour (DocumentWindow::backgroundColourId),
                     &value,
                     false);
        tabs.addTab ("Subscribers",
                     findColour (DocumentWindow::backgroundColourId),
                     &subscribers,
                     false);
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds());
    }

private:
    State<Model::App> state;
    PropertyInfoPanel<Editable::yes> info { state[&Model::App::saved][&Model::Saved::properties] };
    PropertyValuePanel<Editable::yes> value { state[&Model::App::saved]
                                                   [&Model::Saved::properties] };
    PropertySubscribersPanel<Editable::yes> subscribers { state };
    TabbedComponent tabs { TabbedButtonBar::Orientation::TabsAtTop };
};

template <>
class PropertyEditPanel<Editable::no> : public Component
{
public:
    explicit PropertyEditPanel (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (tabs);
        tabs.addTab ("Info",
                     findColour (DocumentWindow::backgroundColourId),
                     &info,
                     false);
        tabs.addTab ("Value",
                     findColour (DocumentWindow::backgroundColourId),
                     &value,
                     false);

        value.onSetFullRequested    = Utils::forwardFunction (onSetFullRequested);
        value.onSetPartialRequested = Utils::forwardFunction (onSetPartialRequested);
        value.onGetRequested        = Utils::forwardFunction (onGetRequested);
        value.onSubscribeRequested  = Utils::forwardFunction (onSubscribeRequested);
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds());
    }

    std::function<void()> onGetRequested, onSubscribeRequested;
    std::function<void (Span<const std::byte>)> onSetFullRequested, onSetPartialRequested;

private:
    State<Model::Device> getDeviceState() const
    {
        const auto selected = (size_t) state->transient.devices.selection;
        return state[&Model::App::transient]
                    [&Model::Transient::devices]
                    [&Model::ListWithSelection<Model::Device>::items]
                    [selected];
    }

    State<Model::App> state;
    PropertyInfoPanel<Editable::no> info { getDeviceState()[&Model::Device::properties] };
    PropertyValuePanel<Editable::no> value
    {
        getDeviceState()[&Model::Device::properties],
        getDeviceState()[&Model::Device::subscriptions]
    };
    TabbedComponent tabs { TabbedButtonBar::Orientation::TabsAtTop };
};

template <Editable editable>
class PropertyConfigPanel : public Component
{
public:
    explicit PropertyConfigPanel (State<Model::App> s)
        : state (s)
    {
        setSize (1, 700);

        addAndMakeVisible (list);
    }

    void resized() override
    {
        Utils::doTwoColumnLayout (getLocalBounds().reduced (Utils::padding),
                                  list,
                                  GridItem { info.has_value() ? &*info : nullptr });
    }

    std::function<void()> onGetRequested, onSubscribeRequested;
    std::function<void (Span<const std::byte>)> onSetFullRequested, onSetPartialRequested;

private:
    State<Model::Properties> getPropertyState() const
    {
        if constexpr (editable == Editable::yes)
        {
            return state[&Model::App::saved][&Model::Saved::properties];
        }
        else
        {
            const auto selected = (size_t) state->transient.devices.selection;
            return state[&Model::App::transient]
                        [&Model::Transient::devices]
                        [&Model::ListWithSelection<Model::Device>::items]
                        [selected]
                        [&Model::Device::properties];
        }
    }

    State<Model::App> state;
    PropertyList<editable> list { getPropertyState() };
    std::optional<PropertyEditPanel<editable>> info;

    ErasedScopeGuard listener = state.observe ([this] (auto)
    {
        if (getPropertyState()->properties.getSelected() != nullptr)
        {
            if (! info.has_value())
            {
                addAndMakeVisible (info.emplace (state));

                if constexpr (editable == Editable::no)
                {
                    info->onSetFullRequested    = Utils::forwardFunction (onSetFullRequested);
                    info->onSetPartialRequested = Utils::forwardFunction (onSetPartialRequested);
                    info->onGetRequested        = Utils::forwardFunction (onGetRequested);
                    info->onSubscribeRequested  = Utils::forwardFunction (onSubscribeRequested);
                }

                resized();
            }
        }
        else
        {
            info.reset();
        }
    });
};

class LocalConfigurationPanel : public Component
{
public:
    LocalConfigurationPanel (State<ci::MUID> m, State<Model::App> s)
        : muidState (m), state (s)
    {
        addAndMakeVisible (list);

        list.addItem (basicsHeader);
        list.addItem (discovery);
        list.addItem (profilesHeader);
        list.addItem (profiles);
        list.addItem (propertiesHeader);
        list.addItem (properties);
    }

    void resized() override
    {
        list.setBounds (getLocalBounds());
    }

private:
    State<ci::MUID> muidState;
    State<Model::App> state;
    SectionHeader basicsHeader { "Basics" };
    ToggleSectionHeader profilesHeader { "Profiles",
                                         state[&Model::App::saved]
                                              [&Model::Saved::fundamentals]
                                              [&Model::DeviceInfo::profilesSupported] };
    PropertySectionHeader propertiesHeader { state[&Model::App::saved]
                                                  [&Model::Saved::fundamentals] };
    DiscoveryInfoPanel<Editable::yes> discovery { muidState,
                                                  state[&Model::App::saved]
                                                       [&Model::Saved::fundamentals] };
    ProfileConfigPanel<Editable::yes> profiles { state[&Model::App::saved]
                                                      [&Model::Saved::profiles] };
    PropertyConfigPanel<Editable::yes> properties { state };
    HeterogeneousListView list;

    ErasedScopeGuard listener = state[&Model::App::saved]
                                     [&Model::Saved::fundamentals].observe ([this] (auto)
    {
        const auto current = state->saved.fundamentals;
        profiles.setEnabled (current.profilesSupported);
        properties.setEnabled (current.propertiesSupported);
    });
};

class RemoteConfigurationPanel : public Component
{
public:
    explicit RemoteConfigurationPanel (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (list);
        profiles.onChannelsRequested = Utils::forwardFunction (onChannelsRequested);

        properties.onSetFullRequested    = Utils::forwardFunction (onPropertySetFullRequested);
        properties.onSetPartialRequested = Utils::forwardFunction (onPropertySetPartialRequested);
        properties.onGetRequested        = Utils::forwardFunction (onPropertyGetRequested);
        properties.onSubscribeRequested  = Utils::forwardFunction (onPropertySubscribeRequested);

        rebuildList();
    }

    void resized() override
    {
        list.setBounds (getLocalBounds());
    }

    std::function<void (uint16_t)> onChannelsRequested;
    std::function<void()> onPropertyGetRequested, onPropertySubscribeRequested;
    std::function<void (Span<const std::byte>)> onPropertySetFullRequested,
                                                onPropertySetPartialRequested;

private:
    void rebuildList()
    {
        list.clear();

        list.addItem (basicsHeader);
        list.addItem (discovery);

        if (getDeviceState()->info.profilesSupported)
        {
            list.addItem (profilesHeader);
            list.addItem (profiles);
        }

        if (getDeviceState()->info.propertiesSupported)
        {
            list.addItem (propertiesHeader);
            list.addItem (properties);
        }

        list.resized();
    }

    State<Model::Device> getDeviceState() const
    {
        const auto selected = (size_t) state->transient.devices.selection;
        return state[&Model::App::transient]
                    [&Model::Transient::devices]
                    [&Model::ListWithSelection<Model::Device>::items]
                    [selected];
    }

    State<Model::App> state;
    SectionHeader basicsHeader { "Basics" };
    DiscoveryInfoPanel<Editable::no> discovery { getDeviceState()[&Model::Device::muid],
                                                 getDeviceState()[&Model::Device::info] };
    SectionHeader profilesHeader { "Profiles" }, propertiesHeader { "Properties" };
    ProfileConfigPanel<Editable::no> profiles { getDeviceState()[&Model::Device::profiles] };
    PropertyConfigPanel<Editable::no> properties { state };
    HeterogeneousListView list;

    ErasedScopeGuard listener = getDeviceState().observe ([this] (const auto& old)
    {
        const auto transactions = (int) getDeviceState()->info.numPropertyExchangeTransactions;
        const auto plural = transactions == 1 ? "transaction" : "transactions";
        propertiesHeader.set ("Properties ("
                              + String (transactions)
                              + " simultaneous "
                              + String (plural)
                              + " supported)");

        const auto& newInfo = getDeviceState()->info;

        if (std::tie (old.info.propertiesSupported, old.info.profilesSupported)
            != std::tie (newInfo.propertiesSupported, newInfo.profilesSupported))
        {
            rebuildList();
        }
    });
};

class DiscoveryPanel : public Component
{
public:
    explicit DiscoveryPanel (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (discoveryButton);
        discoveryButton.onClick = Utils::forwardFunction (onDiscover);

        addAndMakeVisible (deviceCombo);
        deviceCombo.setTextWhenNoChoicesAvailable ("Press Discover Devices to find new devices");
        deviceCombo.setTextWhenNothingSelected ("No device selected");
    }

    void resized() override
    {
        const auto headerHeight = Utils::standardControlHeight + 2 * Utils::padding;

        auto b = getLocalBounds();

        Utils::doTwoColumnLayout (b.removeFromTop (headerHeight).reduced (Utils::padding),
                                  discoveryButton,
                                  deviceCombo);

        if (configPanel.has_value())
            configPanel->setBounds (b);
    }

    std::function<void (uint16_t)> onChannelsRequested;
    std::function<void()> onDiscover, onPropertyGetRequested, onPropertySubscribeRequested;
    std::function<void (Span<const std::byte>)> onPropertySetFullRequested,
                                                onPropertySetPartialRequested;

private:
    State<Model::App> state;
    TextButton discoveryButton { "Discover Devices" };
    ComboBox deviceCombo;
    std::optional<RemoteConfigurationPanel> configPanel;
    ErasedScopeGuard listener = state[&Model::App::transient]
                                     [&Model::Transient::devices].observe ([this] (const auto& old)
    {
        if (old.items != state->transient.devices.items)
        {
            deviceCombo.getRootMenu()->clear();

            auto indexState = state[&Model::App::transient]
                                   [&Model::Transient::devices]
                                   [&Model::ListWithSelection<Model::Device>::selection];

            auto index = 0;
            for (auto& dev : state->transient.devices.items)
            {
                const auto suffix = [&]() -> String
                {
                    if (const auto readable = dev.properties.getReadableDeviceInfo())
                    {
                        String result;

                        if (readable->model.isNotEmpty())
                            result << " " << readable->model;

                        if (readable->manufacturer.isNotEmpty())
                            result << " (" << readable->manufacturer << ")";

                        return result;
                    }

                    return {};
                }();

                PopupMenu::Item item;
                item.action = [indexState, i = index++]() mutable { indexState = i; };
                item.text = String::toHexString (dev.muid.get()) + suffix;
                item.itemID = index;
                deviceCombo.getRootMenu()->addItem (std::move (item));
            }
        }

        deviceCombo.setSelectedItemIndex (state->transient.devices.selection, dontSendNotification);

        if (state->transient.devices.getSelected() != nullptr)
        {
            const auto newIndex = state->transient.devices.selection;

            if (old.selection != newIndex)
            {
                addAndMakeVisible (configPanel.emplace (state));
                configPanel->onChannelsRequested =
                    Utils::forwardFunction (onChannelsRequested);
                configPanel->onPropertySetFullRequested =
                    Utils::forwardFunction (onPropertySetFullRequested);
                configPanel->onPropertySetPartialRequested =
                    Utils::forwardFunction (onPropertySetPartialRequested);
                configPanel->onPropertyGetRequested =
                    Utils::forwardFunction (onPropertyGetRequested);
                configPanel->onPropertySubscribeRequested =
                    Utils::forwardFunction (onPropertySubscribeRequested);
            }
        }
        else
        {
            configPanel.reset();
        }

        resized();
    });
};

/** Accumulates incoming MIDI messages on the MIDI thread, and passes these messages off to other
    Consumers on the main thread.

    This is useful because the ci::Device is only intended for single-threaded use. It is an error
    to call ci::Device::receiveMessage concurrently with any other member function on a particular
    instance of Device.

    This implementation uses a mutex to protect access to the accumulated packets, and an
    AsyncUpdater to signal the main thread to wake up and process the packets. Alternative
    approaches (e.g. using notify_one and wait from std::atomic_flag, or putting messages into a
    queue and polling periodically on the main thread) may be more suitable in production apps.
*/
class MessageForwarder : public MidiInputCallback,
                         private AsyncUpdater
{
public:
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        if (! message.isSysEx())
            return;

        {
            const std::scoped_lock lock { mutex };
            messages.push_back (message);
        }

        triggerAsyncUpdate();
    }

    [[nodiscard]] ErasedScopeGuard addConsumer (ci::DeviceMessageHandler& c)
    {
        return ErasedScopeGuard { [this, it = consumers.insert (&c).first] { consumers.erase (it); } };
    }

private:
    void handleAsyncUpdate() override
    {
        const std::scoped_lock lock { mutex };

        for (auto* c : consumers)
            for (const auto& message : messages)
                c->processMessage ({ 0, message.getSysExDataSpan() });

        messages.clear();
    }

    std::set<ci::DeviceMessageHandler*> consumers;

    std::mutex mutex;
    std::vector<MidiMessage> messages;
};

class LoggingModel : public TableListBoxModel
{
public:
    enum Columns
    {
        messageTime = 1,
        group,
        direction,
        from,
        to,
        version,
        channel,
        description,
    };

    explicit LoggingModel (State<Model::App> s) : state (s) {}

    Component* refreshComponentForCell (int rowNumber,
                                        int columnId,
                                        bool,
                                        Component* existingComponentToUpdate) override
    {
        auto owned = rawToUniquePtr (existingComponentToUpdate);
        const auto filtered = getFiltered();

        if (! isPositiveAndBelow (rowNumber, filtered.size()))
            return nullptr;

        auto ownedLabel = [&owned]
        {
            auto* label = dynamic_cast<Label*> (owned.get());

            if (label == nullptr)
                return Utils::makeListRowLabel ("", false);

            owned.release();
            return rawToUniquePtr (label);
        }();

        const auto& row = filtered[(size_t) rowNumber];

        const auto text = [&]() -> String
        {
            const auto parsed = ci::Parser::parse (row.message);

            if (! parsed.has_value())
                return {};

            switch (columnId)
            {
                case messageTime:
                    return row.time.toString (false, true, true, true);

                case group:
                    return String (row.group);

                case direction:
                    return row.kind == Model::MessageKind::incoming ? "in" : "out";

                case from:
                    return String::toHexString (parsed->header.source.get());

                case to:
                    return String::toHexString (parsed->header.destination.get());

                case version:
                    return String::toHexString (parsed->header.version);

                case channel:
                    return ci::ChannelInGroupUtils::toString (parsed->header.deviceID);

                case description:
                    return state->saved.logView.mode == Model::DataViewMode::ascii
                         ? ci::Parser::getMessageDescription (*parsed)
                         : String::toHexString (row.message.data(), (int) row.message.size());
            }

            return {};
        }();

        ownedLabel->setText (text, dontSendNotification);
        return ownedLabel.release();
    }

    int getNumRows() override { return (int) getFiltered().size(); }

    void paintRowBackground (Graphics&, int, int, int, bool) override {}
    void paintCell (Graphics&, int, int, int, int, bool) override {}

private:
    std::deque<Model::LogEntry> getFiltered() const
    {
        const auto& filter = state->saved.logView.filter;
        auto copy = state->transient.logEntries;

        if (! filter.has_value())
            return copy;

        const auto iter = std::remove_if (copy.begin(),
                                          copy.end(),
                                          [&] (const auto& e) { return e.kind != *filter; });
        copy.erase (iter, copy.end());
        return copy;
    }

    State<Model::App> state;
};

class LoggingList : public Component,
                    private Timer
{
public:
    explicit LoggingList (State<Model::App> s)
        : state (s)
    {
        addAndMakeVisible (list);

        auto& header = list.getHeader();
        header.addColumn ("Time",
                          LoggingModel::Columns::messageTime,
                          100,
                          100);
        header.addColumn ("Group",
                          LoggingModel::Columns::group,
                          50,
                          50);
        header.addColumn ("IO",
                          LoggingModel::Columns::direction,
                          50,
                          50);
        header.addColumn ("From",
                          LoggingModel::Columns::from,
                          60,
                          50);
        header.addColumn ("To",
                          LoggingModel::Columns::to,
                          60,
                          50);
        header.addColumn ("Version",
                          LoggingModel::Columns::version,
                          50,
                          50);
        header.addColumn ("Channel",
                          LoggingModel::Columns::channel,
                          100,
                          50);
        header.addColumn ("Description",
                          LoggingModel::Columns::description,
                          300,
                          50);
    }

    void resized() override
    {
        list.setBounds (getLocalBounds());
    }

    void updateContent()
    {
        // Using a timer here means that we only repaint the UI after there haven't been any
        // new messages for a while, which avoids doing redundant expensive list-layouts.
        startTimer (16);
    }

private:
    void timerCallback() override
    {
        const auto& vbar = list.getVerticalScrollBar();
        const auto endShowing = vbar.getCurrentRange().getEnd() >= vbar.getMaximumRangeLimit();

        stopTimer();
        list.updateContent();

        if (endShowing)
            list.scrollToEnsureRowIsOnscreen (list.getNumRows() - 1);
    }

    State<Model::App> state;
    LoggingModel model { state };
    TableListBox list { "Logs", &model };
    ErasedScopeGuard listener = state[&Model::App::transient]
                                     [&Model::Transient::logEntries].observe ([this] (auto)
                                                                              {
                                                                                 updateContent();
                                                                              });
};

class LoggingPanel : public Component
{
public:
    explicit LoggingPanel (State<Model::App> stateIn)
        : state (stateIn)
    {
        addAndMakeVisible (list);

        addAndMakeVisible (onlyIncoming);
        onlyIncoming.onClick = [this]
        {
            auto s = state[&Model::App::saved][&Model::Saved::logView][&Model::LogView::filter];
            s = *s == Model::MessageKind::incoming ? std::nullopt
                                                   : std::optional (Model::MessageKind::incoming);
        };

        addAndMakeVisible (onlyOutgoing);
        onlyOutgoing.onClick = [this]
        {
            auto s = state[&Model::App::saved][&Model::Saved::logView][&Model::LogView::filter];
            s = *s == Model::MessageKind::outgoing ? std::nullopt
                                                   : std::optional (Model::MessageKind::outgoing);
        };

        addAndMakeVisible (readable);
        readable.onClick = [this]
        {
            auto s = state[&Model::App::saved][&Model::Saved::logView][&Model::LogView::mode];
            s = readable.getToggleState() ? Model::DataViewMode::ascii : Model::DataViewMode::hex;
        };

        addAndMakeVisible (clearButton);
        clearButton.onClick = [this]
        {
            state[&Model::App::transient]
                 [&Model::Transient::logEntries] = std::deque<Model::LogEntry>();
        };
    }

    void resized() override
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.items = { FlexItem{}.withHeight (Utils::standardControlHeight),
                     FlexItem{}.withHeight (Utils::padding),
                     FlexItem { list }.withFlex (1) };
        fb.performLayout (getLocalBounds().reduced (Utils::padding));

        Utils::doColumnLayout (fb.items.getFirst().currentBounds.getSmallestIntegerContainer(),
                               onlyIncoming,
                               onlyOutgoing,
                               readable,
                               clearButton);
    }

private:
    State<Model::App> state;
    LoggingList list { state };
    ToggleButton onlyIncoming { "Only Incoming" }, onlyOutgoing { "Only Outgoing" };
    ToggleButton readable { "Human-Readable" };
    TextButton clearButton { "Clear" };
    ErasedScopeGuard listener = state[&Model::App::saved]
                                     [&Model::Saved::logView].observe ([this] (auto)
    {
        list.updateContent();

        onlyIncoming.setToggleState (state->saved.logView.filter == Model::MessageKind::incoming,
                                     dontSendNotification);
        onlyOutgoing.setToggleState (state->saved.logView.filter == Model::MessageKind::outgoing,
                                     dontSendNotification);
        readable.setToggleState (state->saved.logView.mode == Model::DataViewMode::ascii,
                                 dontSendNotification);
    });
};

class CapabilityInquiryDemo : public Component,
                              private Timer
{
public:
    CapabilityInquiryDemo()
    {
        PropertiesFile::Options options;
        options.applicationName     = "CapabilityInquiryDemo";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Application Support";
        applicationProperties.setStorageParameters (options);

        if (auto* userSettings = applicationProperties.getUserSettings())
            setSavedState (JSON::parse (userSettings->getValue ("savedState")));

        forwarder.addConsumer (inputHandler).release();

        setSize (800, 800);

        addAndMakeVisible (tabs);
        tabs.addTab ("MIDI IO",
                     findColour (DocumentWindow::backgroundColourId),
                     &lists,
                     false);
        tabs.addTab ("Local Configuration",
                     findColour (DocumentWindow::backgroundColourId),
                     &local,
                     false);
        tabs.addTab ("Discovery",
                     findColour (DocumentWindow::backgroundColourId),
                     &discovery,
                     false);
        tabs.addTab ("Logging",
                     findColour (DocumentWindow::backgroundColourId),
                     &logging,
                     false);

        addAndMakeVisible (loadButton);
        loadButton.onClick = [this] { loadState(); };

        addAndMakeVisible (saveButton);
        saveButton.onClick = [this] { saveState(); };

        discovery.onDiscover = [this] { discoverDevices(); };
        discovery.onPropertyGetRequested = [this] { getProperty(); };
        discovery.onPropertySubscribeRequested = [this] { subscribeToProperty(); };
        discovery.onChannelsRequested = [this] (auto channels) { setProfileChannels (channels); };
        discovery.onPropertySetFullRequested = [this] (Span<const std::byte> bytes)
        {
            setPropertyFull (bytes);
        };
        discovery.onPropertySetPartialRequested = [this] (Span<const std::byte> bytes)
        {
            setPropertyPartial (bytes);
        };

        startTimer (2'000);
    }

    ~CapabilityInquiryDemo() override
    {
        stopTimer();

        // In a production app, it'd be a bit risky to write to a file from a destructor as it's
        // bad karma to throw an exception inside a destructor!
        if (auto* userSettings = applicationProperties.getUserSettings())
            userSettings->setValue ("savedState", JSON::toString (getSavedState()));

        if (auto* p = getPeer())
            p->setHasChangedSinceSaved (false);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto buttonStrip = bounds.getWidth() < 650 ? bounds.removeFromTop (tabs.getTabBarDepth())
                                                   : getLocalBounds().removeFromTop (tabs.getTabBarDepth());

        tabs.setBounds (bounds);

        const auto buttonBounds = buttonStrip.removeFromTop (tabs.getTabBarDepth())
                                             .removeFromRight (300)
                                             .reduced (2);
        Utils::doColumnLayout (buttonBounds, loadButton, saveButton);
    }

private:
    void timerCallback() override
    {
        if (device.has_value())
            device->sendPendingMessages();
    }

    std::optional<std::tuple<ci::MUID, String>> getPropertyRequestInfo() const
    {
        auto* selectedDevice = appState->transient.devices.getSelected();

        if (selectedDevice == nullptr)
            return {};

        auto* selectedProperty = selectedDevice->properties.properties.getSelected();

        if (selectedProperty == nullptr)
            return {};

        return std::tuple (selectedDevice->muid, selectedProperty->name);
    }

    void discoverDevices()
    {
        if (device.has_value())
            device->sendDiscovery();
    }

    void loadState()
    {
        fileChooser.launchAsync (FileBrowserComponent::canSelectFiles | FileBrowserComponent::openMode,
                                 [this] (const auto& fc)
                                 {
                                     if (fc.getResults().isEmpty())
                                         return;

                                     auto stream = fc.getResult().createInputStream();

                                     if (stream == nullptr)
                                         return;

                                     setSavedState (JSON::parse (*stream));
                                 });
    }

    void saveState()
    {
        const auto toWrite = JSON::toString (getSavedState());
        fileChooser.launchAsync (FileBrowserComponent::canSelectFiles | FileBrowserComponent::saveMode,
                                 [this, toWrite] (const auto& fc)
                                 {
                                     if (! fc.getResults().isEmpty())
                                     {
                                         fc.getResult().replaceWithText (toWrite);

                                         if (auto* p = getPeer())
                                             p->setHasChangedSinceSaved (false);
                                     }
                                 });
    }

    void setProfileChannels (uint16_t numChannels)
    {
        if (! device.has_value())
            return;

        if (auto* selectedDevice = appState->transient.devices.getSelected())
        {
            if (const auto selected = selectedDevice->profiles.getSelectedProfileAtAddress())
            {
                device->sendProfileEnablement (selectedDevice->muid,
                                               selected->address.getChannel(),
                                               selected->profile,
                                               numChannels);
            }
        }
    }

    void setPropertyFull (Span<const std::byte> bytes)
    {
        if (! device.has_value())
            return;

        if (const auto details = getPropertyRequestInfo())
        {
            const auto& [muid, propName] = *details;

            const auto encodingToUse = [&, muidCopy = muid, propNameCopy = propName]() -> std::optional<ci::Encoding>
            {
                if (auto* deviceResource = findDeviceResource (appState->transient, muidCopy, propNameCopy))
                    return deviceResource->getBestCommonEncoding();

                return {};
            }();

            if (! encodingToUse.has_value())
            {
                // We can't set property data because we don't have any encodings in common with the other device.
                jassertfalse;
                return;
            }

            ci::PropertyRequestHeader header;
            header.resource = propName;
            header.mutualEncoding = *encodingToUse;
            device->sendPropertySetInquiry (muid, header, bytes, [] (const auto&)
            {
                // Could do error handling here, e.g. retry the request if the responder is busy
            });
        }
    }

    void setPropertyPartial (Span<const std::byte> bytes)
    {
        if (! device.has_value())
            return;

        if (const auto details = getPropertyRequestInfo())
        {
            const auto& [muid, propName] = *details;

            const auto encodingToUse = [&, muidCopy = muid, propNameCopy = propName]() -> std::optional<ci::Encoding>
            {
                if (auto* deviceResource = findDeviceResource (appState->transient, muidCopy, propNameCopy))
                    return deviceResource->getBestCommonEncoding();

                return {};
            }();

            if (! encodingToUse.has_value())
            {
                // We can't set property data because we don't have any encodings in common with the other device.
                jassertfalse;
                return;
            }

            ci::PropertyRequestHeader header;
            header.resource = propName;
            header.mutualEncoding = *encodingToUse;
            header.setPartial = true;
            device->sendPropertySetInquiry (muid, header, bytes, [] (const auto&)
            {
                // Could do error handling here, e.g. retry the request if the responder is busy
            });
        }
    }

    void getProperty()
    {
        if (! device.has_value())
            return;

        if (const auto details = getPropertyRequestInfo())
        {
            const auto& [muid, propName] = *details;
            requestPropertyData (muid, propName);
        }
    }

    void subscribeToProperty()
    {
        if (! device.has_value())
            return;

        auto* selectedDevice = appState->transient.devices.getSelected();

        if (selectedDevice == nullptr)
            return;

        const auto details = getPropertyRequestInfo();

        if (! details.has_value())
            return;

        const auto& [muid, propName] = *details;

        // Find the subscription for this resource, if any
        const auto existingToken = [&, propNameCopy = propName]() -> std::optional<ci::SubscriptionKey>
        {
            const auto ongoing = device->getOngoingSubscriptions();

            for (const auto& o : ongoing)
                if (propNameCopy == device->getResourceForKey (o))
                    return o;

            return std::nullopt;
        }();

        // If we're already subscribed, end that subscription.
        // Otherwise, begin a new subscription to this resource.
        const auto changedToken = [this,
                                   propNameCopy = propName,
                                   muidCopy = muid,
                                   existingTokenCopy = existingToken]() -> std::optional<ci::SubscriptionKey>
        {
            // We're not subscribed, so begin a new subscription
            if (! existingTokenCopy.has_value())
            {
                ci::PropertySubscriptionHeader header;
                header.resource = propNameCopy;
                header.command = ci::PropertySubscriptionCommand::start;
                return device->beginSubscription (muidCopy, header);
            }

            device->endSubscription (*existingTokenCopy);
            return existingTokenCopy;
        }();

        if (changedToken.has_value())
            deviceListener.propertySubscriptionChanged (*changedToken);
    }

    template <typename Transient>
    static auto findDeviceResourceImpl (Transient& transient, ci::MUID device, String resource)
        -> decltype (transient.devices.items.front().properties.properties.items.data())
    {
        auto& knownDevices = transient.devices.items;
        const auto deviceIter = std::find_if (knownDevices.begin(),
                                              knownDevices.end(),
                                              [&] (const auto& d) { return d.muid == device; });

        if (deviceIter == knownDevices.end())
            return nullptr;

        auto& props = deviceIter->properties.properties.items;
        const auto propIter = std::find_if (props.begin(), props.end(), [&] (const auto& prop)
        {
            return prop.name == resource;
        });

        if (propIter == props.end())
            return nullptr;

        return &*propIter;
    }

    static Model::Property* findDeviceResource (Model::Transient& transient,
                                                ci::MUID device,
                                                String resource)
    {
        return findDeviceResourceImpl (transient, device, resource);
    }

    static const Model::Property* findDeviceResource (const Model::Transient& transient,
                                                      ci::MUID device,
                                                      String resource)
    {
        return findDeviceResourceImpl (transient, device, resource);
    }

    void requestPropertyData (ci::MUID target, String propertyName)
    {
        const auto encodingToUse = [&]() -> std::optional<ci::Encoding>
        {
            if (auto* deviceResource = findDeviceResource (appState->transient, target, propertyName))
                return deviceResource->getBestCommonEncoding();

            return {};
        }();

        if (! encodingToUse.has_value())
        {
            // We can't request property data because we don't have any encodings in common with the other device.
            jassertfalse;
            return;
        }

        ci::PropertyRequestHeader header;
        header.resource = propertyName;
        header.mutualEncoding = *encodingToUse;

        device->sendPropertyGetInquiry (target, header, [this, target, propertyName] (const auto& response)
        {
            if (response.getError().has_value())
                return;

            auto updated = *appState;

            if (auto* deviceResource = findDeviceResource (updated.transient, target, propertyName))
            {
                deviceResource->value.bytes = std::vector<std::byte> (response.getBody().begin(),
                                                                      response.getBody().end());
                appState = std::move (updated);
            }
        });
    }

    void setSavedState (var json)
    {
        if (auto newState = FromVar::convert<Model::Saved> (json))
            appState[&Model::App::saved] = std::move (*newState);

        if (auto* p = getPeer())
            p->setHasChangedSinceSaved (false);
    }

    var getSavedState() const
    {
        if (auto json = ToVar::convert (appState->saved))
            return *json;

        return {};
    }

    ci::ProfileHost* getProfileHost()
    {
        if (device.has_value())
            return device->getProfileHost();

        return nullptr;
    }

    ci::PropertyHost* getPropertyHost()
    {
        if (device.has_value())
            return device->getPropertyHost();

        return nullptr;
    }

    [[nodiscard]] std::optional<MidiDeviceInfo> getInputInfo() const
    {
        if (input != nullptr)
            return input->getDeviceInfo();

        return std::nullopt;
    }

    [[nodiscard]] std::optional<MidiDeviceInfo> getOutputInfo() const
    {
        if (output != nullptr)
            return output->getDeviceInfo();

        return std::nullopt;
    }

    [[nodiscard]] Model::IOSelection getIOSelectionFromDevices() const
    {
        return { getInputInfo(), getOutputInfo() };
    }

    static bool isStillConnected (const MidiInput& x)
    {
        const auto devices = MidiInput::getAvailableDevices();
        return std::any_of (devices.begin(),
                            devices.end(),
                            [info = x.getDeviceInfo()] (const auto& d) { return d == info; });
    }

    static bool isStillConnected (const MidiOutput& x)
    {
        const auto devices = MidiOutput::getAvailableDevices();
        return std::any_of (devices.begin(),
                            devices.end(),
                            [info = x.getDeviceInfo()] (const auto& d) { return d == info; });
    }

    [[nodiscard]] static bool isStillConnected (const std::unique_ptr<MidiInput>& x)
    {
        if (x == nullptr)
            return false;

        return isStillConnected (*x);
    }

    [[nodiscard]] static bool isStillConnected (const std::unique_ptr<MidiOutput>& x)
    {
        if (x == nullptr)
            return false;

        return isStillConnected (*x);
    }

    void setDeviceProfileState (ci::ProfileAtAddress profileAtAddress,
                                ci::SupportedAndActive state)
    {
        if (auto* h = getProfileHost())
        {
            if (state.supported == 0)
                h->removeProfile (profileAtAddress);
            else
                h->addProfile (profileAtAddress, state.supported);

            h->setProfileEnablement (profileAtAddress, state.active);
        }
    }

    void notifySubscribersForProperty (StringRef propertyName)
    {
        if (! device.has_value())
            return;

        const auto& subscribers = appState->transient.subscribers;
        const auto iter = subscribers.find (propertyName);

        if (iter == subscribers.end())
            return;

        for (const auto& [receiver, subscriptions] : iter->second)
        {
            for (const auto& subId : subscriptions)
            {
                if (auto* host = getPropertyHost())
                {
                    ci::PropertySubscriptionHeader header;
                    header.command = ci::PropertySubscriptionCommand::notify;
                    header.subscribeId = subId;
                    header.resource = propertyName;
                    host->sendSubscriptionUpdate (receiver, header, {}, {});
                }
            }
        }
    }

    void addLogEntry (ump::BytesOnGroup entry,
                      Model::MessageKind kind,
                      Time time = Time::getCurrentTime())
    {
        static constexpr size_t maxNum = 1000;

        auto entries = appState[&Model::App::transient][&Model::Transient::logEntries];
        auto updated = *entries;
        updated.emplace (updated.end(), Model::LogEntry { { entry.bytes.begin(),
                                                            entry.bytes.end() },
                                                          entry.group,
                                                          time,
                                                          kind });

        while (updated.size() > maxNum)
            updated.pop_front();

        entries = std::move (updated);
    }

    static Model::App normalise (const Model::App& older, Model::App&& newer)
    {
        auto modified = std::move (newer);

        if (older.saved.fundamentals != modified.saved.fundamentals)
        {
            modified.transient.devices.items.clear();
            modified.transient.devices.selection = -1;
            modified.transient.subscribers.clear();
        }

        modified.syncSubscribers();

        return modified;
    }

    class DeviceListener : public ci::DeviceListener
    {
    public:
        explicit DeviceListener (CapabilityInquiryDemo& d) : demo (d) {}

        void deviceAdded (ci::MUID added) override
        {
            auto updated = *demo.appState;

            auto& devices = updated.transient.devices;
            auto& deviceVec = devices.items;
            const auto iterForMuid = [&] (auto m)
            {
                return std::find_if (deviceVec.begin(),
                                     deviceVec.end(),
                                     [&] (const auto& d) { return d.muid == m; });
            };

            const auto iter = iterForMuid (added);
            auto& toUpdate = iter != deviceVec.end() ? *iter : deviceVec.emplace_back();
            toUpdate.muid = added;

            if (devices.getSelected() == nullptr)
                devices.selection = (int) std::distance (deviceVec.begin(), iterForMuid (added));

            const auto response = demo.device.has_value() ? demo.device->getDiscoveryInfoForMuid (added)
                                                          : std::nullopt;

            if (! response.has_value())
                return;

            const ci::DeviceFeatures features { response->capabilities };
            toUpdate.info.profilesSupported = features.isProfileConfigurationSupported();
            toUpdate.info.propertiesSupported = features.isPropertyExchangeSupported();
            toUpdate.info.deviceInfo = response->device;
            toUpdate.info.maxSysExSize = response->maximumSysexSize;
            toUpdate.info.numPropertyExchangeTransactions = 0;

            demo.appState = updated;

            if (! demo.device.has_value())
                return;

            if (features.isProfileConfigurationSupported())
                demo.device->sendProfileInquiry (added, ci::ChannelInGroup::wholeBlock);

            if (features.isPropertyExchangeSupported())
                demo.device->sendPropertyCapabilitiesInquiry (added);
        }

        void deviceRemoved (ci::MUID gone) override
        {
            auto updated = *demo.appState;

            auto& subs = updated.transient.subscribers;

            for (auto& u : subs)
                u.second.erase (gone);

            auto& devices = updated.transient.devices;
            const auto selectedMuid = [&]() -> std::optional<ci::MUID>
            {
                if (auto* item = devices.getSelected())
                    return item->muid;

                return {};
            }();

            devices.items.erase (std::remove_if (devices.items.begin(),
                                                 devices.items.end(),
                                                 [&] (const auto& d) { return d.muid == gone; }),
                                 devices.items.end());

            const auto iter = std::find_if (devices.items.begin(),
                                            devices.items.end(),
                                            [&] (const auto& d) { return d.muid == selectedMuid; });
            devices.selection = iter != devices.items.end()
                                ? (int) std::distance (devices.items.begin(), iter)
                                : -1;

            demo.appState = updated;
        }

        void endpointReceived (ci::MUID, ci::Message::EndpointInquiryResponse) override
        {
            // No special handling
        }

        void messageNotAcknowledged (ci::MUID, ci::Message::NAK) override
        {
            // No special handling
        }

        void profileStateReceived (ci::MUID muid,
                                   ci::ChannelInGroup) override
        {
            updateProfilesForMuid (muid);
        }

        void profilePresenceChanged (ci::MUID muid,
                                     ci::ChannelInGroup,
                                     ci::Profile,
                                     bool) override
        {
            updateProfilesForMuid (muid);
        }

        void profileEnablementChanged (ci::MUID muid,
                                       ci::ChannelInGroup,
                                       ci::Profile,
                                       int) override
        {
            updateProfilesForMuid (muid);
        }

        void profileDetailsReceived (ci::MUID,
                                     ci::ChannelInGroup,
                                     ci::Profile,
                                     std::byte,
                                     Span<const std::byte>) override
        {
            // No special handling
        }

        void profileSpecificDataReceived (ci::MUID,
                                          ci::ChannelInGroup,
                                          ci::Profile,
                                          Span<const std::byte>) override
        {
            // No special handling
        }

        void propertyExchangeCapabilitiesReceived (ci::MUID muid) override
        {
            auto updated = *demo.appState;

            auto& devices = updated.transient.devices;
            const auto iter = std::find_if (devices.items.begin(),
                                            devices.items.end(),
                                            [&] (const auto& d) { return d.muid == muid; });

            if (iter == devices.items.end() || ! demo.device.has_value())
                return;

            const auto transactions = demo.device->getNumPropertyExchangeRequestsSupportedForMuid (muid);

            if (! transactions.has_value())
                return;

            iter->info.numPropertyExchangeTransactions = (uint8_t) *transactions;

            if (const auto resourceList = demo.device->getResourceListForMuid (muid); resourceList != var{})
            {
                if (auto* list = resourceList.getArray())
                {
                    auto& items = updated.transient.devices.items;
                    const auto found = std::find_if (items.begin(),
                                                     items.end(),
                                                     [&] (const auto& dev) { return dev.muid == muid; });

                    if (found != updated.transient.devices.items.end())
                    {
                        found->properties.properties = {};
                        auto& propItems = found->properties.properties.items;

                        Model::Property resourceListProp;
                        resourceListProp.name = "ResourceList";
                        resourceListProp.canSet = Model::CanSet::none;
                        resourceListProp.value.bytes = ci::Encodings::jsonTo7BitText (resourceList);
                        propItems.push_back (resourceListProp);

                        for (auto& entry : *list)
                            propItems.push_back (Model::Property::fromResourceListEntry (entry));
                    }
                }
            }

            if (const auto deviceInfo = demo.device->getDeviceInfoForMuid (muid); deviceInfo != var{})
            {
                if (auto* deviceResource = findDeviceResource (updated.transient, muid, "DeviceInfo"))
                {
                    deviceResource->value.bytes = ci::Encodings::jsonTo7BitText (deviceInfo);
                }
            }

            demo.appState = std::move (updated);
        }

        void propertySubscriptionDataReceived (ci::MUID muid,
                                               const ci::PropertySubscriptionData& subscription) override
        {
            const auto resource = [&]
            {
                const auto ongoing = demo.device->getOngoingSubscriptions();

                for (const auto& o : ongoing)
                {
                    if (subscription.header.subscribeId == demo.device->getSubscribeIdForKey (o))
                        return demo.device->getResourceForKey (o).value_or (String{});
                }

                return String{};
            }();

            if (resource.isEmpty())
            {
                // Got a subscription message for a subscription that's no longer ongoing
                jassertfalse;
                return;
            }

            auto devicesState = demo.appState[&Model::App::transient]
                                             [&Model::Transient::devices]
                                             [&Model::ListWithSelection<Model::Device>::items];
            auto copiedDevices = *devicesState;

            const auto matchingDevice = [&]
            {
                const auto iter = std::find_if (copiedDevices.begin(),
                                                copiedDevices.end(),
                                                [&] (const auto& d) { return d.muid == muid; });
                return iter != copiedDevices.end() ? &*iter : nullptr;
            }();

            if (matchingDevice == nullptr)
            {
                // Got a subscription message for a device that we haven't recorded
                jassertfalse;
                return;
            }

            auto& propertyList = matchingDevice->properties.properties.items;

            const auto matchingProperty = [&]
            {
                const auto iter = std::find_if (propertyList.begin(),
                                                propertyList.end(),
                                                [&] (const auto& p) { return p.name == resource; });
                return iter != propertyList.end() ? &*iter : nullptr;
            }();

            if (matchingProperty == nullptr)
            {
                // Got a subscription message for a property that we haven't recorded
                jassertfalse;
                return;
            }

            switch (subscription.header.command)
            {
                case ci::PropertySubscriptionCommand::partial:
                {
                    auto [updated, error] = Utils::attemptSetPartial (std::move (*matchingProperty),
                                                                      subscription);
                    *matchingProperty = std::move (updated);
                    jassert (error.isEmpty()); // Inspect 'error' to see what went wrong
                    break;
                }

                case ci::PropertySubscriptionCommand::full:
                {
                    matchingProperty->value.bytes = std::vector<std::byte> (subscription.body.begin(),
                                                                            subscription.body.end());
                    matchingProperty->value.mediaType = subscription.header.mediaType;
                    break;
                }

                case ci::PropertySubscriptionCommand::notify:
                {
                    demo.requestPropertyData (muid, resource);
                    break;
                }

                case ci::PropertySubscriptionCommand::end:
                    break;

                case ci::PropertySubscriptionCommand::start:
                    jassertfalse;
                    return;
            }

            devicesState = std::move (copiedDevices);
        }

        void propertySubscriptionChanged (ci::SubscriptionKey key, const std::optional<String>&) override
        {
            propertySubscriptionChanged (key);
        }

        void propertySubscriptionChanged (ci::SubscriptionKey key)
        {
            auto updated = *demo.appState;

            auto& knownDevices = updated.transient.devices.items;
            const auto deviceIter = std::find_if (knownDevices.begin(),
                                                  knownDevices.end(),
                                                  [target = key.getMuid()] (const auto& d) { return d.muid == target; });

            if (deviceIter == knownDevices.end())
            {
                // The device has gone away?
                jassertfalse;
                return;
            }

            if (const auto resource = demo.device->getResourceForKey (key))
                deviceIter->subscriptions.emplace (key, ci::Subscription { demo.device->getSubscribeIdForKey (key).value_or (String{}), *resource });
            else
                deviceIter->subscriptions.erase (key);

            demo.appState = std::move (updated);
        }

    private:
        void updateProfilesForMuid (ci::MUID muid)
        {
            if (! demo.device.has_value())
                return;

            auto innerState = demo.appState[&Model::App::transient][&Model::Transient::devices];
            auto updated = *innerState;

            const auto iter = std::find_if (updated.items.begin(),
                                            updated.items.end(),
                                            [&] (const auto& d) { return d.muid == muid; });

            if (iter == updated.items.end())
                return;

            auto& profiles = iter->profiles;

            const auto lastSelectedProfile = [&]() -> std::optional<ci::Profile>
            {
                if (auto* ptr = profiles.profiles.getSelected())
                    return *ptr;

                return {};
            }();

            profiles.profileMode = Model::ProfileMode::use;
            profiles.profiles.items = [&]
            {
                std::set<ci::Profile> uniqueProfiles;
                Utils::forAllChannelAddresses ([&] (auto address)
                {
                    if (auto* state = demo.device->getProfileStateForMuid (muid, address))
                        for (const auto& p : *state)
                            uniqueProfiles.insert (p.profile);
                });
                return std::vector<ci::Profile> (uniqueProfiles.begin(), uniqueProfiles.end());
            }();
            profiles.profiles.selection = [&]
            {
                if (! lastSelectedProfile.has_value())
                    return -1;

                const auto foundMuid = std::find (profiles.profiles.items.begin(),
                                                  profiles.profiles.items.end(),
                                                  *lastSelectedProfile);

                if (foundMuid == profiles.profiles.items.end())
                    return -1;

                return (int) std::distance (profiles.profiles.items.begin(), foundMuid);
            }();
            profiles.channels = [&]
            {
                std::map<ci::ProfileAtAddress, ci::SupportedAndActive> result;

                Utils::forAllChannelAddresses ([&] (auto address)
                {
                    if (auto* state = demo.device->getProfileStateForMuid (muid, address))
                        for (const auto& p : *state)
                            result[{ p.profile, address }] = p.state;
                });

                return result;
            }();
            profiles.selectedChannel = [&]() -> std::optional<ci::ChannelAddress>
            {
                if (profiles.profileMode == Model::ProfileMode::edit)
                    return profiles.selectedChannel;

                const auto profileAtAddress = profiles.getSelectedProfileAtAddress();

                if (! profileAtAddress.has_value())
                    return std::nullopt;

                const auto found = profiles.channels.find (*profileAtAddress);

                if (found == profiles.channels.end() || found->second.supported == 0)
                    return std::nullopt;

                return profiles.selectedChannel;
            }();

            innerState = std::move (updated);
        }

        CapabilityInquiryDemo& demo;
    };

    class InputHandler : public ci::DeviceMessageHandler
    {
    public:
        explicit InputHandler (CapabilityInquiryDemo& d) : demo (d) {}

        void processMessage (ump::BytesOnGroup msg) override
        {
            demo.addLogEntry (msg, Model::MessageKind::incoming);

            if (demo.device.has_value())
                demo.device->processMessage (msg);
        }

    private:
        CapabilityInquiryDemo& demo;
    };

    class OutputHandler : public ci::DeviceMessageHandler
    {
    public:
        explicit OutputHandler (CapabilityInquiryDemo& d) : demo (d) {}

        void processMessage (ump::BytesOnGroup msg) override
        {
            SafePointer weak { &demo };
            std::vector<std::byte> bytes (msg.bytes.begin(), msg.bytes.end());
            auto group = msg.group;
            auto time = Time::getCurrentTime();

            MessageManager::callAsync ([weak, movedBytes = std::move (bytes), group, time]
            {
                // This call is async because we may send messages in direct response to model updates.
                if (weak != nullptr)
                    weak->addLogEntry ({ group, movedBytes }, Model::MessageKind::outgoing, time);
            });

            if (auto* out = demo.output.get())
                out->sendMessageNow (MidiMessage::createSysExMessage (msg.bytes));
        }

    private:
        CapabilityInquiryDemo& demo;
    };

    class ProfileDelegate : public ci::ProfileDelegate
    {
    public:
        explicit ProfileDelegate (CapabilityInquiryDemo& d) : demo (d) {}

        void profileEnablementRequested (ci::MUID,
                                         ci::ProfileAtAddress profileAtAddress,
                                         int numChannels,
                                         bool enabled) override
        {
            auto state = demo.appState[&Model::App::saved][&Model::Saved::profiles];
            auto profiles = *state;

            if (auto* host = demo.getProfileHost())
            {
                const auto count = enabled ? jmax (1, numChannels) : 0;
                host->setProfileEnablement (profileAtAddress, count);
                profiles.channels[profileAtAddress].active = (uint16_t) count;

                state = profiles;
            }
        }

    private:
        CapabilityInquiryDemo& demo;
    };

    class PropertyDelegate : public ci::PropertyDelegate
    {
    public:
        explicit PropertyDelegate (CapabilityInquiryDemo& d) : demo (d) {}

        uint8_t getNumSimultaneousRequestsSupported() const override
        {
            return demo.appState->saved.fundamentals.numPropertyExchangeTransactions;
        }

        ci::PropertyReplyData propertyGetDataRequested (ci::MUID,
                                                        const ci::PropertyRequestHeader& header) override
        {
            auto allProperties = demo.appState->saved.properties.properties.items;
            allProperties.insert (allProperties.begin(), getDeviceInfo());

            if (header.resource == "ResourceList")
                return generateResourceListReply (allProperties);

            for (const auto& prop : allProperties)
                if (auto reply = generateReply (header, prop))
                    return *reply;

            ci::PropertyReplyData result;
            result.header.status = 404;
            result.header.message = "Unable to locate resource " + header.resource;
            return result;
        }

        ci::PropertyReplyHeader propertySetDataRequested (ci::MUID,
                                                          const ci::PropertyRequestData& request) override
        {
            const auto makeErrorHeader = [] (auto str, int code = 400)
            {
                ci::PropertyReplyHeader header;
                header.status = code;
                header.message = str;
                return header;
            };

            if (request.header.resource == "ResourceList")
                return makeErrorHeader ("Unable to set ResourceList");

            auto state = demo.appState[&Model::App::saved]
                                      [&Model::Saved::properties]
                                      [&Model::Properties::properties];
            auto props = *state;

            for (auto& prop : props.items)
            {
                if (request.header.resource != prop.name)
                    continue;

                if (prop.canSet == Model::CanSet::none)
                    return makeErrorHeader ("Unable to set resource " + prop.name);

                if (request.header.setPartial)
                {
                    if (prop.canSet != Model::CanSet::partial)
                        return makeErrorHeader ("Resource " + prop.name + " does not support setPartial");

                    auto [updatedProp, error] = Utils::attemptSetPartial (std::move (prop), request);
                    prop = std::move (updatedProp);

                    if (error.isNotEmpty())
                        return makeErrorHeader (error);

                    state = props;
                    return {};
                }

                prop.value.bytes = std::vector<std::byte> (request.body.begin(), request.body.end());
                prop.value.mediaType = request.header.mediaType;

                state = props;

                return {};
            }

            return makeErrorHeader ("Unable to locate resource " + request.header.resource, 404);
        }

        bool subscriptionStartRequested (ci::MUID, const ci::PropertySubscriptionHeader& header) override
        {
            const auto props = demo.appState->saved.properties.properties.items;

            return std::any_of (props.begin(), props.end(), [&] (const auto& p)
            {
                return p.name == header.resource;
            });
        }

        void subscriptionDidStart (ci::MUID initiator,
                                   const String& token,
                                   const ci::PropertySubscriptionHeader& header) override
        {
            auto transient = demo.appState[&Model::App::transient];
            auto updated = *transient;
            updated.subscribers[header.resource][initiator].insert (token);
            transient = updated;
        }

        void subscriptionWillEnd (ci::MUID initiator, const ci::Subscription& subscription) override
        {
            auto transient = demo.appState[&Model::App::transient];
            auto updated = *transient;
            updated.subscribers[subscription.resource][initiator].erase (subscription.subscribeId);
            transient = updated;
        }

    private:
        Model::Property getDeviceInfo() const
        {
            Model::Property result;
            result.name = "DeviceInfo";

            auto obj = std::make_unique<DynamicObject>();

            const auto varFromByteArray = [] (const auto& r)
            {
                return Model::toVarArray (r, [] (auto b) { return (int) b; });
            };

            obj->setProperty ("manufacturerId",
                              varFromByteArray (demo.device->getOptions().getDeviceInfo().manufacturer));
            obj->setProperty ("manufacturer",
                              "JUCE");
            obj->setProperty ("familyId",
                              varFromByteArray (demo.device->getOptions().getDeviceInfo().family));
            obj->setProperty ("family",
                              "MIDI Software");
            obj->setProperty ("modelId",
                              varFromByteArray (demo.device->getOptions().getDeviceInfo().modelNumber));
            obj->setProperty ("model",
                              "Capability Inquiry Demo");
            obj->setProperty ("versionId",
                              varFromByteArray (demo.device->getOptions().getDeviceInfo().revision));
            obj->setProperty ("version",
                              ProjectInfo::versionString);

            result.value.bytes = ci::Encodings::jsonTo7BitText (obj.release());
            return result;
        }

        ci::PropertyReplyData generateResourceListReply (Span<const Model::Property> allProperties) const
        {
            Array<var> resourceList;

            for (const auto& prop : allProperties)
                resourceList.add (prop.getResourceListEntry());

            return ci::PropertyReplyData { {},
                                           ci::Encodings::jsonTo7BitText (std::move (resourceList)) };
        }

        std::optional<ci::PropertyReplyData> generateReply (const ci::PropertyRequestHeader& header,
                                                            const Model::Property& info) const
        {
            if (header.resource != info.name)
                return {};

            const auto encodingToUse = [&]() -> std::optional<ci::Encoding>
            {
                if (info.encodings.count (header.mutualEncoding) != 0)
                    return header.mutualEncoding;

                if (! info.encodings.empty())
                    return *info.encodings.begin();

                return {};
            }();

            if (! encodingToUse.has_value())
            {
                // If this is hit, we don't declare any supported encodings for this property!
                jassertfalse;
                return {};
            }

            const auto basicReplyHeader = [&]
            {
                ci::PropertyReplyHeader h;
                h.mutualEncoding = *encodingToUse;
                h.mediaType = info.value.mediaType;
                return h;
            }();

            const auto [replyHeader, unencoded] = [&]
            {
                if (info.name.endsWith ("List")
                    && info.canPaginate
                    && info.value.mediaType == "application/json")
                {
                    const auto jsonToSend = ci::Encodings::jsonFrom7BitText (info.value.bytes);

                    if (const auto* array = jsonToSend.getArray())
                    {
                        const auto updatedHeader = [&]
                        {
                            auto h = basicReplyHeader;
                            h.extended["totalCount"] = array->size();
                            return h;
                        }();

                        if (const auto pagination = header.pagination)
                        {
                            const auto realOffset = jlimit (0, array->size(), pagination->offset);
                            const auto realLimit = jlimit (0,
                                                           array->size() - realOffset,
                                                           pagination->limit);
                            const auto* data = array->data() + realOffset;
                            const Array<var> slice (data, realLimit);
                            return std::tuple (updatedHeader, ci::Encodings::jsonTo7BitText (slice));
                        }

                        return std::tuple (updatedHeader, info.value.bytes);
                    }
                }

                return std::tuple (basicReplyHeader, info.value.bytes);
            }();

            return ci::PropertyReplyData { replyHeader, unencoded };
        }

        CapabilityInquiryDemo& demo;
    };

    ApplicationProperties applicationProperties;

    State<ci::MUID> ourMuid { ci::MUID::makeUnchecked (0) };
    State<Model::App> appState { Model::App{}, normalise };

    IOPickerLists lists { appState[&Model::App::saved][&Model::Saved::ioSelection] };
    LocalConfigurationPanel local { ourMuid, appState };
    DiscoveryPanel discovery { appState };
    LoggingPanel logging { appState };
    TabbedComponent tabs { TabbedButtonBar::Orientation::TabsAtTop };
    TextButton loadButton { "Load State..." }, saveButton { "Save State..."};

    MessageForwarder forwarder;

    std::unique_ptr<MidiInput> input;
    std::unique_ptr<MidiOutput> output;
    std::optional<ci::Device> device;

    FileChooser fileChooser { "Pick State JSON File", {}, "*.json", true, false, this };

    InputHandler inputHandler { *this };
    OutputHandler outputHandler { *this };
    DeviceListener deviceListener { *this };
    ProfileDelegate profileDelegate { *this };
    PropertyDelegate propertyDelegate { *this };

    std::vector<ErasedScopeGuard> listeners = Utils::makeVector
    (
        appState[&Model::App::saved][&Model::Saved::fundamentals].observe ([this] (auto)
        {
            Random random;
            random.setSeedRandomly();

            const auto fullState = *appState;
            const auto info = fullState.saved.fundamentals;
            const auto features = ci::DeviceFeatures()
                    .withProfileConfigurationSupported (info.profilesSupported)
                    .withPropertyExchangeSupported (info.propertiesSupported);

            const auto options = ci::DeviceOptions()
                    .withOutputs ({ &outputHandler })
                    .withDeviceInfo (info.deviceInfo)
                    .withFeatures (features)
                    .withMaxSysExSize (info.maxSysExSize)
                    .withProductInstanceId (ci::DeviceOptions::makeProductInstanceId (random))
                    .withPropertyDelegate (&propertyDelegate)
                    .withProfileDelegate (&profileDelegate);

            device.emplace (options);
            device->addListener (deviceListener);

            for (const auto& [addressAndProfile, channels] : fullState.saved.profiles.channels)
                setDeviceProfileState (addressAndProfile, channels);

            ourMuid = device->getMuid();
        }),
        appState[&Model::App::saved]
                [&Model::Saved::profiles]
                [&Model::Profiles::channels].observe ([this] (auto)
        {
            if (auto* host = getProfileHost())
            {
                std::map<ci::ProfileAtAddress, ci::SupportedAndActive> deviceState;

                Utils::forAllChannelAddresses ([&] (auto address)
                {
                    if (auto* state = host->getProfileStates().getStateForDestination (address))
                        for (const auto& p : *state)
                            deviceState[{ p.profile, address }] = p.state;
                });

                const auto requestedState = appState->saved.profiles.channels;

                std::vector<std::pair<ci::ProfileAtAddress, ci::SupportedAndActive>> removed;
                const auto compare = [] (const auto& a, const auto& b)
                {
                    return a.first < b.first;
                };
                std::set_difference (deviceState.begin(),
                                     deviceState.end(),
                                     requestedState.begin(),
                                     requestedState.end(),
                                     std::back_inserter (removed),
                                     compare);

                for (const auto& p : removed)
                    host->removeProfile (p.first);

                for (const auto& [addressAndProfile, channels] : requestedState)
                    setDeviceProfileState (addressAndProfile, channels);
            }
        }),
        appState[&Model::App::saved]
                [&Model::Saved::ioSelection]
                [&Model::IOSelection::input].observe ([this] (auto)
        {
            if (input != nullptr)
                input->stop();

            input.reset();

            if (const auto selection = appState->saved.ioSelection.input)
            {
                input = MidiInput::openDevice (selection->identifier, &forwarder);

                if (input != nullptr)
                    input->start();
            }
        }),
        appState[&Model::App::saved]
                [&Model::Saved::ioSelection]
                [&Model::IOSelection::output].observe ([this] (auto)
        {
            output.reset();

            if (const auto selection = appState->saved.ioSelection.output)
                output = MidiOutput::openDevice (selection->identifier);
        }),
        appState[&Model::App::saved].observe ([this] (auto)
        {
            if (auto* p = getPeer())
                p->setHasChangedSinceSaved (true);
        }),
        appState[&Model::App::transient]
                [&Model::Transient::subscribers].observe ([this] (const auto& oldSubscribers)
        {
            // Send a subscription end message for any subscribed properties that have been
            // completely removed
            const auto newSubscribers = appState->transient.subscribers;

            if (oldSubscribers == newSubscribers || ! device.has_value())
                return;

            std::set<String> removed;
            for (const auto& p : oldSubscribers)
                if (newSubscribers.count (p.first) == 0)
                    removed.insert (p.first);

            if (auto* host = getPropertyHost())
                for (const auto& r : removed)
                    for (const auto& [dest, subIds] : oldSubscribers.find (r)->second)
                        for (const auto& id : subIds)
                            host->terminateSubscription (dest, id);
        }),
        appState[&Model::App::saved]
                [&Model::Saved::properties]
                [&Model::Properties::properties].observe ([this] (const auto& oldProperties)
        {
            const auto makePropertyMap = [] (const auto& props)
            {
                std::map<String, Model::PropertyValue> result;

                for (const auto& p : props.items)
                    result.emplace (p.name, p.value);

                return result;
            };

            const auto& newProperties = appState->saved.properties.properties;
            const auto oldMap = makePropertyMap (oldProperties);
            const auto newMap = makePropertyMap (newProperties);

            for (const auto& [name, value] : newMap)
            {
                const auto iter = oldMap.find (name);

                if (iter != oldMap.end() && iter->second != value)
                    notifySubscribersForProperty (name);
            }
        })
    );

    MidiDeviceListConnection connection = MidiDeviceListConnection::make ([this]
    {
        if (! isStillConnected (input))
        {
            if (input != nullptr)
                input->stop();

            input.reset();
        }

        if (! isStillConnected (output))
            output.reset();

        appState[&Model::App::saved][&Model::Saved::ioSelection] = getIOSelectionFromDevices();
    });
};
