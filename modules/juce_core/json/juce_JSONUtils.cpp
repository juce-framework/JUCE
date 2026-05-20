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

var JSONUtils::makeObject (const std::map<Identifier, var>& source)
{
    auto result = std::make_unique<DynamicObject>();

    for (const auto& [name, value] : source)
        result->setProperty (name, value);

    return var (result.release());
}

var JSONUtils::makeObjectWithKeyFirst (const std::map<Identifier, var>& source,
                                       Identifier key)
{
    auto result = std::make_unique<DynamicObject>();

    if (const auto iter = source.find (key); iter != source.end())
        result->setProperty (key, iter->second);

    for (const auto& [name, value] : source)
        if (name != key)
            result->setProperty (name, value);

    return var (result.release());
}

static var* locateProperty (const var& v, const Identifier& name)
{
    auto* object = v.getDynamicObject();

    if (object == nullptr)
        return nullptr;

    return object->getProperties().getVarPointer (name);
}

static var* locateProperty (var& v, const Identifier& name)
{
    auto* object = v.getDynamicObject();

    if (object == nullptr)
        return nullptr;

    if (! object->getProperties().contains (name))
        object->getProperties().set (name, var());

    return object->getProperties().getVarPointer (name);
}

static const var* locateIndex (const var& v, int index)
{
    auto* array = v.getArray();

    if (array == nullptr)
        return nullptr;

    if (! isPositiveAndBelow (index, array->size()))
        return nullptr;

    return array->data() + index;
}

static var* locateIndex (var& v, int index)
{
    auto* array = v.getArray();

    if (array == nullptr)
        return nullptr;

    const auto correctedIndex = index == -1 ? array->size() : index;

    if (correctedIndex == array->size())
        array->add (var());

    if (! isPositiveAndBelow (correctedIndex, array->size()))
        return nullptr;

    return array->data() + correctedIndex;
}

template <typename Var>
Var* locate (Var& v, String pointer)
{
    if (pointer.isEmpty())
        return &v;

    if (! pointer.startsWith ("/"))
    {
        // This is not a well-formed JSON pointer
        jassertfalse;
        return nullptr;
    }

    const auto findResult = pointer.indexOfChar (1, '/');
    const auto pos = findResult < 0 ? pointer.length() : findResult;

    const auto head = String { pointer.begin() + 1, pointer.begin() + pos }.replace ("~1", "/").replace ("~0", "~");
    const String tail { pointer.begin() + pos, pointer.end() };

    if (auto* prop = locateProperty (v, head))
        return locate (*prop, tail);

    const auto index = std::invoke ([&]() -> std::optional<int>
    {
        if (head == "0")
            return 0;

        if (head == "-")
            return -1;

        if (head.startsWith ("0") || ! head.containsOnly ("0123456789"))
            return {};

        return head.getIntValue();
    });

    if (! index.has_value())
        return nullptr;

    if (auto* element = locateIndex (v, *index))
        return locate (*element, tail);

    return nullptr;
}

std::optional<var> JSONUtils::getPointer (const var& v, String pointer)
{
    if (auto* result = locate (v, pointer))
        return *result;

    return {};
}

std::optional<var> JSONUtils::setPointer (const var& v, String pointer, const var& newValue)
{
    auto clone = v.clone();
    auto* leaf = locate (clone, pointer);

    if (leaf == nullptr)
        return {};

    *leaf = newValue;
    return clone;
}

bool JSONUtils::deepEqual (const var& a, const var& b)
{
    const auto compareObjects = [] (const DynamicObject& x, const DynamicObject& y)
    {
        if (x.getProperties().size() != y.getProperties().size())
            return false;

        for (const auto& [key, value] : x.getProperties())
        {
            if (! y.hasProperty (key))
                return false;

            if (! deepEqual (value, y.getProperty (key)))
                return false;
        }

        return true;
    };

    if (auto* i = a.getDynamicObject())
        if (auto* j = b.getDynamicObject())
            return compareObjects (*i, *j);

    if (auto* i = a.getArray())
        if (auto* j = b.getArray())
            return std::equal (i->begin(), i->end(), j->begin(), j->end(), [] (const var& x, const var& y) { return deepEqual (x, y); });

    return a == b;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class JSONUtilsTests final : public UnitTest
{
public:
    JSONUtilsTests() : UnitTest ("JSONUtils", UnitTestCategories::json) {}

    void runTest() override
    {
        auto expectEqualsFunc = [] (const std::optional<var>& a, const std::optional<var>& b)
        {
            return a.has_value() && b.has_value() && deepEqual (*a, *b);
        };

        beginTest ("Specification 6901 tests");
        {
            const auto rfc = JSON::parse (R"({
                "foo": ["bar", "baz"],
                "a/b": 1,
                "c%d": 2,
                "e^f": 3,
                "g|h": 4,
                "i\\j": 5,
                "k\"l": 6,
                " ": 7,
                "m~n": 8,
                "obj":
                {
                    "key": "value",
                    "array": [0, 1, 2]
                },
                "nested": { "foo/bar": { "foo/bar": "first" },
                            "foo~bar": { "foo~bar": "second" } },
                "01": "foo"
            })");

            expectEqualsFunc (JSONUtils::getPointer (rfc, ""), rfc);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/foo"), JSON::parse (R"(["bar", "baz"])"));
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/foo/0"), "bar");
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/a~1b"), 1);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/c%d"), 2);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/e^f"), 3);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/g|h"), 4);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/i\\j"), 5);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/k\"l"), 6);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/ "), 7);
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/m~0n"), 8);

            expectEqualsFunc (JSONUtils::getPointer (rfc, "/nested/foo~1bar/foo~1bar"), "first");
            expectEqualsFunc (JSONUtils::getPointer (rfc, "/nested/foo~0bar/foo~0bar"), "second");

            beginTest ("Zero leading key is valid key");
            {
                expectEqualsFunc (JSONUtils::getPointer (JSON::parse (R"({ "01": "foo" })"), "/01"), "foo");
            }

            beginTest ("Null terminator is not 'End of String'");
            {
                const auto nulJson = JSON::parse (String::fromUTF8 ("{ \"foo\0bar\": \"bang\" }", 22));
                expectEqualsFunc (JSONUtils::getPointer (nulJson, String::fromUTF8 ("/foo\0bar", 9)), "bang");
            }
        }

        const auto obj = JSON::parse (R"({ "name":           "PIANO 4"
                                         , "lfoSpeed":       30
                                         , "lfoWaveform":    "triangle"
                                         , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50, 51, 52, 53] }
                                         })");

        beginTest ("getPointer");
        {
            expectEqualsFunc (JSONUtils::getPointer (obj, "/name"), "PIANO 4");
            expectEqualsFunc (JSONUtils::getPointer (obj, "/lfoSpeed"), var (30));
            expectEqualsFunc (JSONUtils::getPointer (obj, "/pitchEnvelope/rates/1"), var (67));
            expectEqualsFunc (JSONUtils::getPointer (obj, "/pitchEnvelope/levels/2"), var (50));
            expectEqualsFunc (JSONUtils::getPointer (obj, "/pitchEnvelope/levels/10"), var());
        }

        beginTest ("setPointer");
        {
            expectDeepEqual (JSONUtils::setPointer (obj, "", "hello world"), var ("hello world"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/lfoWaveform/foobar", "str"), std::nullopt);
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"foo":0,"bar":1})"), "/foo", 2), JSON::parse (R"({"foo":2,"bar":1})"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"foo":0,"bar":1})"), "/baz", 2), JSON::parse (R"({"foo":0,"bar":1,"baz":2})"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"foo":{},"bar":{}})"), "/foo/bar", 2), JSON::parse (R"({"foo":{"bar":2},"bar":{}})"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/rates/01", "str"), std::nullopt);
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/rates/10", "str"), std::nullopt);
            expectDeepEqual (JSONUtils::setPointer (obj, "/lfoSpeed", 10), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                           , "lfoSpeed":       10
                                                                                           , "lfoWaveform":    "triangle"
                                                                                           , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50,51,52,53] }
                                                                                           })"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"([0,1,2])"), "/0", "bang"), JSON::parse (R"(["bang",1,2])"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"([0,1,2])"), "/0", "bang"), JSON::parse (R"(["bang",1,2])"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"/":"fizz"})"), "/~1", "buzz"), JSON::parse (R"({"/":"buzz"})"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"~":"fizz"})"), "/~0", "buzz"), JSON::parse (R"({"~":"buzz"})"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/rates/0", 80), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                        , "lfoSpeed":       30
                                                                                                        , "lfoWaveform":    "triangle"
                                                                                                        , "pitchEnvelope":  { "rates": [80,67,95,60], "levels": [50,51,52,53] }
                                                                                                        })"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/levels/0", 80), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                         , "lfoSpeed":       30
                                                                                                         , "lfoWaveform":    "triangle"
                                                                                                         , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [80,51,52,53] }
                                                                                                         })"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/levels/-", 100), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                          , "lfoSpeed":       30
                                                                                                          , "lfoWaveform":    "triangle"
                                                                                                          , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50,51,52,53,100] }
                                                                                                          })"));
        }
    }

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

static JSONUtilsTests jsonUtilsTests;

#endif

} // namespace juce
