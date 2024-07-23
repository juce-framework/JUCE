/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    Allows serialisation functions to be attached to a specific type without having to modify the
    declaration of that type.

    A specialisation of SerialisationTraits must include:
    - A static constexpr data member named 'marshallingVersion' with a value that is convertible
      to std::optional<int>.
    - Either:
        - Normally, a single function with the following signature:
          @code
          template <typename Archive, typename Item>
          static void serialise (Archive& archive, Item& item);
          @endcode
        - For types that must do slightly different work when loading and saving, you may supply two
          functions with the following signatures, where "T" is a placeholder for the type on which
          SerialisationTraits is specialised:
          @code
          template <typename Archive>
          static void load (Archive& archive, T& item);

          template <typename Archive>
          static void save (Archive& archive, const T& item);
          @endcode

    If the marshallingVersion converts to a null optional, then all versioning information will be
    ignored when marshalling the type. Otherwise, if the value converts to a non-null optional, this
    versioning information will be included when serialising the type.

    Inside serialise() and load() you may call archive.getVersion() to find the detected version
    of the object being deserialised. archive.getVersion() will return an std::optional<int>,
    where 'nullopt' indicates that no versioning information was detected.

    Marshalling functions can also be specified directly inside the type to be marshalled. This
    approach may be preferable as it is more concise. Internal marshalling functions are written
    in exactly the same way as external ones; i.e. the type must include a marshallingVersion,
    and either a single serialise function, or a load/save pair of functions, as specified above.

    @tags{Core}
*/
template <typename T> struct SerialisationTraits
{
    /*  Intentionally left blank. */
};

#define JUCE_COMPARISON_OPS X(==) X(!=) X(<) X(<=) X(>) X(>=)

/**
    Combines an object with a name.

    Instances of Named have reference-like semantics. That is, Named stores a reference
    to a wrapped value, rather than storing the value internally.

    @tparam T   the type of reference that is wrapped. Passing "const T" will cause the Named
                instance to hold a "const T&"; passing "T" will cause the Named instance to
                hold a "T&".

    @see named()

    @tags{Core}
*/
template <typename T>
struct Named
{
   #define X(op) auto operator op (const Named& other) const { return value op other.value; }
    JUCE_COMPARISON_OPS
   #undef X

    std::string_view name; ///< A name that corresponds to the value
    T& value;              ///< A reference to a value to wrap
};

/** Produces a Named instance that holds a mutable reference. */
template <typename T> constexpr auto named (std::string_view c, T& t)       { return Named<T>       { c, t }; }

/** Produces a Named instance that holds an immutable reference. */
template <typename T> constexpr auto named (std::string_view c, const T& t) { return Named<const T> { c, t }; }

/**
    Holds a reference to some kind of size value, used to indicate that an object being marshalled
    is of variable size (e.g. Array, vector, map, set, etc.).

    If you need to write your own serialisation routines for a dynamically-sized type, ensure
    that you archive an instance of SerialisationSize before any of the contents of the container.

    @tparam T   the (probably numeric) type of the size value

    @see serialisztionSize()

    @tags{Core}
*/
template <typename T>
struct SerialisationSize
{
   #define X(op) auto operator op (const SerialisationSize& other) const { return size op other.size; }
    JUCE_COMPARISON_OPS
   #undef X

    T& size;
};

/** Produces a SerialisationSize instance that holds a mutable reference to a size value. */
template <typename T> constexpr auto serialisationSize       (T& t) -> std::enable_if_t<std::is_integral_v<T>, SerialisationSize<T>>       { return { t }; }

/** Produces a SerialisationSize instance that holds an immutable reference to a size value. */
template <typename T> constexpr auto serialisationSize (const T& t) -> std::enable_if_t<std::is_integral_v<T>, SerialisationSize<const T>> { return { t }; }

#undef JUCE_COMPARISON_OPS

//==============================================================================
/*
    The following are specialisations of SerialisationTraits for commonly-used types.
*/

#ifndef DOXYGEN

template <typename... Ts>
struct SerialisationTraits<std::vector<Ts...>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));
        t.resize (size);

        for (auto& element : t)
            archive (element);
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        archive (serialisationSize (t.size()));

        for (auto& element : t)
            archive (element);
    }
};

template <typename Element, typename Mutex, int minSize>
struct SerialisationTraits<Array<Element, Mutex, minSize>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));
        t.resize (size);

        for (auto& element : t)
            archive (element);
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        archive (serialisationSize (t.size()));

        for (auto& element : t)
            archive (element);
    }
};

template <>
struct SerialisationTraits<StringArray>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (t.strings);
    }
};

template <typename... Ts>
struct SerialisationTraits<std::pair<Ts...>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("first", t.first), named ("second", t.second));
    }
};

template <typename T>
struct SerialisationTraits<std::optional<T>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive>
    static void load (Archive& archive, std::optional<T>& t)
    {
        bool engaged = false;

        archive (named ("engaged", engaged));

        if (! engaged)
            return;

        t.emplace();
        archive (named ("value", *t));
    }

    template <typename Archive>
    static void save (Archive& archive, const std::optional<T>& t)
    {
        archive (named ("engaged", t.has_value()));

        if (t.has_value())
            archive (named ("value", *t));
    }
};

template <>
struct SerialisationTraits<std::string>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive>
    static void load (Archive& archive, std::string& t)
    {
        String temporary;
        archive (temporary);
        t = temporary.toStdString();
    }

    template <typename Archive>
    static void save (Archive& archive, const std::string& t)
    {
        archive (String (t));
    }
};

template <typename... Ts>
struct SerialisationTraits<std::map<Ts...>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));

        for (auto i = (decltype (size)) 0; i < size; ++i)
        {
            std::pair<typename T::key_type, typename T::mapped_type> element;
            archive (element);
            t.insert (element);
        }
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));

        for (const auto& element : t)
            archive (element);
    }
};

template <typename... Ts>
struct SerialisationTraits<std::set<Ts...>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));

        for (auto i = (decltype (size)) 0; i < size; ++i)
        {
            typename T::value_type element;
            archive (element);
            t.insert (element);
        }
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        auto size = t.size();
        archive (serialisationSize (size));

        for (const auto& element : t)
            archive (element);
    }
};

template <size_t N>
struct SerialisationTraits<char[N]>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t) { archive (String (t, N)); }
};

template <typename Element, size_t N>
struct SerialisationTraits<Element[N]>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = N;
        archive (serialisationSize (size));

        for (auto& element : t)
            archive (element);
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        const auto size = N;
        archive (serialisationSize (size));

        for (auto& element : t)
            archive (element);
    }
};

template <typename Element, size_t N>
struct SerialisationTraits<std::array<Element, N>>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void load (Archive& archive, T& t)
    {
        auto size = N;
        archive (serialisationSize (size));

        for (auto& element : t)
            archive (element);
    }

    template <typename Archive, typename T>
    static void save (Archive& archive, const T& t)
    {
        const auto size = N;
        archive (serialisationSize (size));

        for (auto& element : t)
            archive (element);
    }
};

/*
    This namespace holds utilities for detecting and using serialisation functions.

    The contents of this namespace are private, and liable to change, so you shouldn't use any of
    the contents directly.
*/
namespace detail
{
    struct DummyArchive
    {
        template <typename... Ts>
        bool operator() (Ts&&...);

        std::optional<int> getVersion() const { return {}; }
    };

    template <typename T, typename = void>
    constexpr auto hasInternalVersion = false;

    template <typename T>
    constexpr auto hasInternalVersion<T, std::void_t<decltype (T::marshallingVersion)>> = true;

    template <typename Traits, typename T, typename = void>
    constexpr auto hasInternalSerialise = false;

    template <typename Traits, typename T>
    constexpr auto hasInternalSerialise<Traits, T, std::void_t<decltype (Traits::serialise (std::declval<DummyArchive&>(), std::declval<T&>()))>> = true;

    template <typename Traits, typename T, typename = void>
    constexpr auto hasInternalLoad = false;

    template <typename Traits, typename T>
    constexpr auto hasInternalLoad<Traits, T, std::void_t<decltype (Traits::load (std::declval<DummyArchive&>(), std::declval<T&>()))>> = true;

    template <typename Traits, typename T, typename = void>
    constexpr auto hasInternalSave = false;

    template <typename Traits, typename T>
    constexpr auto hasInternalSave<Traits, T, std::void_t<decltype (Traits::save (std::declval<DummyArchive&>(), std::declval<const T&>()))>> = true;

    template <typename T>
    struct SerialisedTypeTrait { using type = T; };

    template <typename T>
    struct SerialisedTypeTrait<SerialisationTraits<T>> { using type = T; };

    template <typename T>
    using SerialisedType = typename SerialisedTypeTrait<T>::type;

    template <typename T>
    constexpr auto hasSerialisation = hasInternalVersion<SerialisedType<T>>
                                   || hasInternalSerialise<T, SerialisedType<T>>
                                   || hasInternalLoad<T, SerialisedType<T>>
                                   || hasInternalSave<T, SerialisedType<T>>;

    /*  Different kinds of serialisation function. */
    enum class SerialisationKind
    {
        none,       // The type doesn't have any serialisation
        primitive,  // The type has serialisation handling defined directly on the archiver. enums will be converted to equivalent integral values
        internal,   // The type has internally-defined serialisation utilities
        external,   // The type has an external specialisation of SerialisationTraits
    };

    /*  The SerialisationKind to use for the type T.

        Primitive serialisation is used for arithmetic types, enums, Strings, and vars.
        Internal serialisation is used for types that declare an internal marshallingVersion,
        serialise(), load(), or save().
        External serialisation is used in all other cases.
    */
    template <typename T>
    constexpr auto serialisationKind = []
    {
        if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, String> || std::is_same_v<T, var>)
            return SerialisationKind::primitive;
        else if constexpr (hasSerialisation<T>)
            return SerialisationKind::internal;
        else if constexpr (hasSerialisation<SerialisationTraits<T>>)
            return SerialisationKind::external;
        else
            return SerialisationKind::none;
    }();

    /*  This trait defines the serialisation utilities that are used for primitive types. */
    template <typename T, SerialisationKind kind = serialisationKind<T>>
    struct ForwardingSerialisationTraits
    {
        static constexpr auto marshallingVersion = std::nullopt;

        template <typename Archive, typename Primitive>
        static auto load (Archive& archive, Primitive& t)
        {
            if constexpr (std::is_enum_v<Primitive>)
                return archive (*reinterpret_cast<std::underlying_type_t<Primitive>*> (&t));
            else
                return archive (t);
        }

        template <typename Archive, typename Primitive>
        static auto save (Archive& archive, const Primitive& t)
        {
            if constexpr (std::is_enum_v<Primitive>)
                return archive (*reinterpret_cast<const std::underlying_type_t<Primitive>*> (&t));
            else
                return archive (t);
        }
    };

    /*  This specialisation will be used for types with internal serialisation.

        All members of ForwardingSerialisationTraits forward to the corresponding member of T.
    */
    template <typename T>
    struct ForwardingSerialisationTraits<T, SerialisationKind::internal>
    {
        static constexpr std::optional<int> marshallingVersion { T::marshallingVersion };

        template <typename Archive, typename Item>
        static auto serialise (Archive& archive, Item& t) -> decltype (Item::serialise (archive, t)) { return Item::serialise (archive, t); }

        template <typename Archive, typename Item>
        static auto load (Archive& archive, Item& t) -> decltype (Item::load (archive, t)) { return Item::load (archive, t); }

        template <typename Archive, typename Item>
        static auto save (Archive& archive, const Item& t) -> decltype (Item::save (archive, t)) { return Item::save (archive, t); }
    };

    /*  This specialisation will be used for types with external serialisation.

        @see SerialisationTraits
    */
    template <typename T>
    struct ForwardingSerialisationTraits<T, SerialisationKind::external> : SerialisationTraits<T> {};

    template <typename T, typename = void>
    constexpr auto hasSerialise = false;

    template <typename T>
    constexpr auto hasSerialise<T, std::void_t<decltype (ForwardingSerialisationTraits<T>::serialise (std::declval<DummyArchive&>(), std::declval<T&>()))>> = true;

    template <typename T, typename = void>
    constexpr auto hasLoad = false;

    template <typename T>
    constexpr auto hasLoad<T, std::void_t<decltype (ForwardingSerialisationTraits<T>::load (std::declval<DummyArchive&>(), std::declval<T&>()))>> = true;

    template <typename T, typename = void>
    constexpr auto hasSave = false;

    template <typename T>
    constexpr auto hasSave<T, std::void_t<decltype (ForwardingSerialisationTraits<T>::save (std::declval<DummyArchive&>(), std::declval<const T&>()))>> = true;

    template <typename T>
    constexpr auto delayStaticAssert = false;

    /*  Calls the correct function (serialise or save) to save the argument t to the archive.
    */
    template <typename Archive, typename T>
    auto doSave (Archive& archive, const T& t)
    {
        if constexpr (serialisationKind<T> == SerialisationKind::none)
            static_assert (delayStaticAssert<T>, "No serialisation function found or marshallingVersion unset");
        else if constexpr (hasSerialise<T> && ! hasSave<T>)
            return ForwardingSerialisationTraits<T>::serialise (archive, t);
        else if constexpr (! hasSerialise<T> && hasSave<T>)
            return ForwardingSerialisationTraits<T>::save (archive, t);
        else
            static_assert (delayStaticAssert<T>, "Multiple serialisation functions found");
    }

    /*  Calls the correct function (serialise or load) to load the argument t from the archive.
    */
    template <typename Archive, typename T>
    auto doLoad (Archive& archive, T& t)
    {
        if constexpr (serialisationKind<T> == SerialisationKind::none)
            static_assert (delayStaticAssert<T>, "No serialisation function found or marshallingVersion unset");
        else if constexpr (hasSerialise<T> && ! hasLoad<T>)
            return ForwardingSerialisationTraits<T>::serialise (archive, t);
        else if constexpr (! hasSerialise<T> && hasLoad<T>)
            return ForwardingSerialisationTraits<T>::load (archive, t);
        else
            static_assert (delayStaticAssert<T>, "Multiple serialisation functions found");
    }
} // namespace detail

#endif

} // namespace juce
