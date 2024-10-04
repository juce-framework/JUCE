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

struct TypeWithExternalUnifiedSerialisation
{
    int a;
    std::string b;
    std::vector<int> c;
    std::map<std::string, int> d;

    auto operator== (const TypeWithExternalUnifiedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithExternalUnifiedSerialisation& other) const { return ! operator== (other); }
};

template <>
struct SerialisationTraits<TypeWithExternalUnifiedSerialisation>
{
    static constexpr auto marshallingVersion = 2;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a),
                 named ("b", t.b),
                 named ("c", t.c),
                 named ("d", t.d));
    }
};

// Now that the serialiser trait is visible, it should be detected
static_assert (detail::serialisationKind<TypeWithExternalUnifiedSerialisation> == detail::SerialisationKind::external);

struct TypeWithInternalUnifiedSerialisation
{
    double a;
    float b;
    String c;
    StringArray d;

    auto operator== (const TypeWithInternalUnifiedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithInternalUnifiedSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 5;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a),
                 named ("b", t.b),
                 named ("c", t.c),
                 named ("d", t.d));
    }
};

static_assert (detail::serialisationKind<TypeWithInternalUnifiedSerialisation> == detail::SerialisationKind::internal);

struct TypeWithExternalSplitSerialisation
{
    std::optional<String> a;
    Array<int> b;

    auto operator== (const TypeWithExternalSplitSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithExternalSplitSerialisation& other) const { return ! operator== (other); }
};

template <>
struct SerialisationTraits<TypeWithExternalSplitSerialisation>
{
    static constexpr auto marshallingVersion = 10;

    template <typename Archive>
    static void load (Archive& archive, TypeWithExternalSplitSerialisation& t)
    {
        std::optional<String> a;
        Array<String> hexStrings;
        archive (named ("a", a), named ("b", hexStrings));

        Array<int> b;

        for (auto& i : hexStrings)
            b.add (i.getHexValue32());

        t = { a, b };
    }

    template <typename Archive>
    static void save (Archive& archive, const TypeWithExternalSplitSerialisation& t)
    {
        Array<String> hexStrings;

        for (auto& i : t.b)
            hexStrings.add ("0x" + String::toHexString (i));

        archive (named ("a", t.a), named ("b", hexStrings));
    }
};

// Now that the serialiser trait is visible, it should be detected
static_assert (detail::serialisationKind<TypeWithExternalSplitSerialisation> == detail::SerialisationKind::external);

// Check that serialisation kinds are correctly detected for primitives
static_assert (detail::serialisationKind<bool>               == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<  int8_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< uint8_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< int16_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<uint16_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< int32_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<uint32_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind< int64_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<uint64_t>           == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<float>              == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<double>             == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<std::byte>          == detail::SerialisationKind::primitive);
static_assert (detail::serialisationKind<String>             == detail::SerialisationKind::primitive);

// Check that serialisation is disabled for types with no serialsation defined
static_assert (detail::serialisationKind<Logger>             == detail::SerialisationKind::none);
static_assert (detail::serialisationKind<CriticalSection>    == detail::SerialisationKind::none);

struct TypeWithInternalSplitSerialisation
{
    std::string a;
    Array<int> b;

    auto operator== (const TypeWithInternalSplitSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b); };
        return tie (*this) == tie (other);
    }

    auto operator!= (const TypeWithInternalSplitSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 1;

    template <typename Archive>
    static void load (Archive& archive, TypeWithInternalSplitSerialisation& t)
    {
        std::string a;
        Array<String> hexStrings;
        archive (named ("a", a), named ("b", hexStrings));

        Array<int> b;

        for (auto& i : hexStrings)
            b.add (i.getHexValue32());

        t = { a, b };
    }

    template <typename Archive>
    static void save (Archive& archive, const TypeWithInternalSplitSerialisation& t)
    {
        Array<String> hexStrings;

        for (auto& i : t.b)
            hexStrings.add ("0x" + String::toHexString (i));

        archive (named ("a", t.a), named ("b", hexStrings));
    }
};

static_assert (detail::serialisationKind<TypeWithInternalSplitSerialisation> == detail::SerialisationKind::internal);

struct TypeWithBrokenObjectSerialisation
{
    int a;
    int b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        // Archiving a named value will start reading/writing an object
        archive (named ("a", t.a));
        // Archiving a non-named value will assume that the current node is convertible
        archive (t.b);
    }
};

struct TypeWithBrokenPrimitiveSerialisation
{
    int a;
    int b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        // Archiving a non-named value will assume that the current node is convertible
        archive (t.a);
        // Archiving a named value will fail if the current node holds a non-object type
        archive (named ("b", t.b));
    }
};

struct TypeWithBrokenArraySerialisation
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T&)
    {
        size_t size = 5;
        archive (size);

        // serialisationSize should always be serialised first!
        archive (serialisationSize (size));
    }
};

struct TypeWithBrokenNestedSerialisation
{
    int a;
    TypeWithBrokenObjectSerialisation b;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a), named ("b", t.b));
    }
};

struct TypeWithBrokenDynamicSerialisation
{
    std::vector<TypeWithBrokenObjectSerialisation> a;

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (t.a);
    }
};

struct TypeWithVersionedSerialisation
{
    int a{}, b{}, c{}, d{};

    bool operator== (const TypeWithVersionedSerialisation& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.a, x.b, x.c, x.d); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const TypeWithVersionedSerialisation& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = 3;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("a", t.a));

        if (archive.getVersion() >= 1)
            archive (named ("b", t.b));

        if (archive.getVersion() >= 2)
            archive (named ("c", t.c));

        if (archive.getVersion() >= 3)
            archive (named ("d", t.d));
    }
};

struct TypeWithRawVarLast
{
    int status = 0;
    String message;
    var extended;

    bool operator== (const TypeWithRawVarLast& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.status, x.message, x.extended); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const TypeWithRawVarLast& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("status", t.status),
                 named ("message", t.message),
                 named ("extended", t.extended));
    }
};

struct TypeWithRawVarFirst
{
    int status = 0;
    String message;
    var extended;

    bool operator== (const TypeWithRawVarFirst& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.status, x.message, x.extended); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const TypeWithRawVarFirst& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("extended", t.extended),
                 named ("status", t.status),
                 named ("message", t.message));
    }
};

struct TypeWithInnerVar
{
    int eventId = 0;
    var payload;

    bool operator== (const TypeWithInnerVar& other) const
    {
        const auto tie = [] (const auto& x) { return std::tie (x.eventId, x.payload); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const TypeWithInnerVar& other) const { return ! operator== (other); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& t)
    {
        archive (named ("eventId", t.eventId),
                 named ("payload", t.payload));
    }
};

class JSONSerialisationTest final : public UnitTest
{
public:
    JSONSerialisationTest() : UnitTest ("JSONSerialisation", UnitTestCategories::json) {}

    void runTest() override
    {
        beginTest ("ToVar");
        {
            expectDeepEqual (ToVar::convert (false), false);
            expectDeepEqual (ToVar::convert (true), true);
            expectDeepEqual (ToVar::convert (1), 1);
            expectDeepEqual (ToVar::convert (5.0f), 5.0);
            expectDeepEqual (ToVar::convert (6LL), 6);
            expectDeepEqual (ToVar::convert ("hello world"), "hello world");
            expectDeepEqual (ToVar::convert (String ("hello world")), "hello world");
            expectDeepEqual (ToVar::convert (std::vector<int> { 1, 2, 3 }), Array<var> { 1, 2, 3 });
            expectDeepEqual (ToVar::convert (TypeWithExternalUnifiedSerialisation { 7,
                                                                                    "hello world",
                                                                                    { 5, 6, 7 },
                                                                                    { { "foo", 4 }, { "bar", 5 } } }),
                             JSONUtils::makeObject ({ { "__version__", 2 },
                                                      { "a", 7 },
                                                      { "b", "hello world" },
                                                      { "c", Array<var> { 5, 6, 7 } },
                                                      { "d",
                                                        Array<var> { JSONUtils::makeObject ({ { "first", "bar" },
                                                                                              { "second", 5 } }),
                                                                     JSONUtils::makeObject ({ { "first", "foo" },
                                                                                              { "second", 4 } }) } } }));
            expectDeepEqual (ToVar::convert (TypeWithInternalUnifiedSerialisation { 7.89,
                                                                                    4.321f,
                                                                                    "custom string",
                                                                                    { "foo", "bar", "baz" } }),
                             JSONUtils::makeObject ({ { "__version__", 5 },
                                                      { "a", 7.89 },
                                                      { "b", 4.321f },
                                                      { "c", "custom string" },
                                                      { "d", Array<var> { "foo", "bar", "baz" } } }));
            expectDeepEqual (ToVar::convert (TypeWithExternalSplitSerialisation { "string", { 1, 2, 3 } }),
                             JSONUtils::makeObject ({ { "__version__", 10 },
                                                      { "a", JSONUtils::makeObject ({ { "engaged", true }, { "value", "string" } }) },
                                                      { "b", Array<var> { "0x1", "0x2", "0x3" } } }));
            expectDeepEqual (ToVar::convert (TypeWithInternalSplitSerialisation { "string", { 16, 32, 48 } }),
                             JSONUtils::makeObject ({ { "__version__", 1 },
                                                      { "a", "string" },
                                                      { "b", Array<var> { "0x10", "0x20", "0x30" } } }));

            expect (ToVar::convert (TypeWithBrokenObjectSerialisation { 1, 2 }) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenPrimitiveSerialisation { 1, 2 }) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenArraySerialisation {}) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenNestedSerialisation {}) == std::nullopt);
            expect (ToVar::convert (TypeWithBrokenDynamicSerialisation { std::vector<TypeWithBrokenObjectSerialisation> (10) }) == std::nullopt);

            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }),
                             JSONUtils::makeObject ({ { "__version__", 3 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withVersionIncluded (false)),
                             JSONUtils::makeObject ({ { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            // Requested explicit version is higher than the type's declared version
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (4)),
                             std::nullopt);
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (3)),
                             JSONUtils::makeObject ({ { "__version__", 3 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 },
                                                      { "d", 4 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (2)),
                             JSONUtils::makeObject ({ { "__version__", 2 },
                                                      { "a", 1 },
                                                      { "b", 2 },
                                                      { "c", 3 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (1)),
                             JSONUtils::makeObject ({ { "__version__", 1 },
                                                      { "a", 1 },
                                                      { "b", 2 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (0)),
                             JSONUtils::makeObject ({ { "__version__", 0 },
                                                      { "a", 1 } }));
            expectDeepEqual (ToVar::convert (TypeWithVersionedSerialisation { 1, 2, 3, 4 }, ToVar::Options {}.withExplicitVersion (std::nullopt)),
                             JSONUtils::makeObject ({ { "a", 1 } }));

            expectDeepEqual (ToVar::convert (TypeWithRawVarLast { 200, "success", true }),
                             JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", true } }));
            expectDeepEqual (ToVar::convert (TypeWithRawVarLast { 200,
                                                                  "success",
                                                                  JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                           { "message", "failure" },
                                                                                           { "extended", true } }) }),
                             JSONUtils::makeObject ({ { "status", 200 },
                                                      { "message", "success" },
                                                      { "extended", JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                             { "message", "failure" },
                                                                                             { "extended", true } }) } }));

            expectDeepEqual (ToVar::convert (TypeWithRawVarFirst { 200, "success", true }),
                             JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", true } }));
            expectDeepEqual (ToVar::convert (TypeWithRawVarFirst { 200,
                                                                   "success",
                                                                   JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                            { "message", "failure" },
                                                                                            { "extended", true } }) }),
                             JSONUtils::makeObject ({ { "status", 200 },
                                                      { "message", "success" },
                                                      { "extended", JSONUtils::makeObject ({ { "status", 123.456 },
                                                                                             { "message", "failure" },
                                                                                             { "extended", true } }) } }));

            const auto payload = JSONUtils::makeObject ({ { "foo", 1 }, { "bar", 2 } });
            expectDeepEqual (ToVar::convert (TypeWithInnerVar { 404, payload }),
                             JSONUtils::makeObject ({ { "eventId", 404 }, { "payload", payload } }));
        }

        beginTest ("FromVar");
        {
            expect (FromVar::convert<bool> (JSON::fromString ("false")) == false);
            expect (FromVar::convert<bool> (JSON::fromString ("true")) == true);
            expect (FromVar::convert<bool> (JSON::fromString ("0")) == false);
            expect (FromVar::convert<bool> (JSON::fromString ("1")) == true);
            expect (FromVar::convert<int> (JSON::fromString ("1")) == 1);
            expect (FromVar::convert<float> (JSON::fromString ("5.0f")) == 5.0f);
            expect (FromVar::convert<int64> (JSON::fromString ("6")) == 6);
            expect (FromVar::convert<String> (JSON::fromString ("\"hello world\"")) == "hello world");
            expect (FromVar::convert<std::vector<int>> (JSON::fromString ("[1,2,3]")) == std::vector<int> { 1, 2, 3 });
            expect (FromVar::convert<TypeWithExternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "__version__", 2 },
                                                                                                     { "a", 7 },
                                                                                                     { "b", "hello world" },
                                                                                                     { "c", Array<var> { 5, 6, 7 } },
                                                                                                     { "d",
                                                                                                       Array<var> { JSONUtils::makeObject ({ { "first", "bar" },
                                                                                                                                             { "second", 5 } }),
                                                                                                                    JSONUtils::makeObject ({ { "first", "foo" },
                                                                                                                                             { "second", 4 } }) } } }))
                    == TypeWithExternalUnifiedSerialisation { 7,
                                                              "hello world",
                                                              { 5, 6, 7 },
                                                              { { "foo", 4 }, { "bar", 5 } } });

            expect (FromVar::convert<TypeWithInternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "__version__", 5 },
                                                                                                     { "a", 7.89 },
                                                                                                     { "b", 4.321f },
                                                                                                     { "c", "custom string" },
                                                                                                     { "d", Array<var> { "foo", "bar", "baz" } } }))
                    == TypeWithInternalUnifiedSerialisation { 7.89,
                                                              4.321f,
                                                              "custom string",
                                                              { "foo", "bar", "baz" } });

            expect (FromVar::convert<TypeWithExternalSplitSerialisation> (JSONUtils::makeObject ({ { "__version__", 10 },
                                                                                                   { "a", JSONUtils::makeObject ({ { "engaged", true }, { "value", "string" } }) },
                                                                                                   { "b", Array<var> { "0x1", "0x2", "0x3" } } }))
                    == TypeWithExternalSplitSerialisation { "string", { 1, 2, 3 } });
            expect (FromVar::convert<TypeWithInternalSplitSerialisation> (JSONUtils::makeObject ({ { "__version__", 1 },
                                                                                                   { "a", "string" },
                                                                                                   { "b", Array<var> { "0x10", "0x20", "0x30" } } }))
                    == TypeWithInternalSplitSerialisation { "string", { 16, 32, 48 } });

            expect (FromVar::convert<TypeWithBrokenObjectSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenPrimitiveSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenArraySerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenNestedSerialisation> (JSON::fromString ("null")) == std::nullopt);
            expect (FromVar::convert<TypeWithBrokenDynamicSerialisation> (JSON::fromString ("null")) == std::nullopt);

            expect (FromVar::convert<TypeWithInternalUnifiedSerialisation> (JSONUtils::makeObject ({ { "a", 7.89 },
                                                                                                     { "b", 4.321f } }))
                    == std::nullopt);

            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 3 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 },
                                                                                               { "d", 4 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 4 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 4 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 },
                                                                                               { "d", 4 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 4 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 2 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 },
                                                                                               { "c", 3 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 3, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 1 },
                                                                                               { "a", 1 },
                                                                                               { "b", 2 } }))
                    == TypeWithVersionedSerialisation { 1, 2, 0, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "__version__", 0 },
                                                                                               { "a", 1 } }))
                    == TypeWithVersionedSerialisation { 1, 0, 0, 0 });
            expect (FromVar::convert<TypeWithVersionedSerialisation> (JSONUtils::makeObject ({ { "a", 1 } }))
                    == TypeWithVersionedSerialisation { 1, 0, 0, 0 });

            const auto raw = JSONUtils::makeObject ({ { "status", 200 }, { "message", "success" }, { "extended", "another string" } });
            expect (FromVar::convert<TypeWithRawVarLast> (raw) == TypeWithRawVarLast { 200, "success", "another string" });
            expect (FromVar::convert<TypeWithRawVarFirst> (raw) == TypeWithRawVarFirst { 200, "success", "another string" });

            const var payloads[] { JSONUtils::makeObject ({ { "foo", 1 }, { "bar", 2 } }),
                                   var (Array<var> { 1, 2 }),
                                   var() };

            for (const auto& payload : payloads)
            {
                const auto objectWithPayload = JSONUtils::makeObject ({ { "eventId", 404 }, { "payload", payload } });
                expect (FromVar::convert<TypeWithInnerVar> (objectWithPayload) == TypeWithInnerVar { 404, payload });
            }
        }
    }

private:
    void expectDeepEqual (const std::optional<var>& a, const std::optional<var>& b)
    {
        const auto text = a.has_value() && b.has_value()
                        ? JSON::toString (*a) + " != " + JSON::toString (*b)
                        : String();
        expect (deepEqual (a, b), text);
    }

    static bool deepEqual (const std::optional<var>& a, const std::optional<var>& b)
    {
        if (a.has_value() && b.has_value())
            return JSONUtils::deepEqual (*a, *b);

        return a == b;
    }
};

static JSONSerialisationTest jsonSerialisationTest;

} // namespace juce
