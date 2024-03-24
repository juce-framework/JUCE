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

std::optional<var> JSONUtils::setPointer (const var& v,
                                          String pointer,
                                          const var& newValue)
{
    if (pointer.isEmpty())
        return newValue;

    if (! pointer.startsWith ("/"))
    {
        // This is not a well-formed JSON pointer
        jassertfalse;
        return {};
    }

    const auto findResult = pointer.indexOfChar (1, '/');
    const auto pos = findResult < 0 ? pointer.length() : findResult;
    const String head (pointer.begin() + 1, pointer.begin() + pos);
    const String tail (pointer.begin() + pos, pointer.end());

    const auto unescaped = head.replace ("~1", "/").replace ("~0", "~");

    if (auto* object = v.getDynamicObject())
    {
        if (const auto newProperty = setPointer (object->getProperty (unescaped), tail, newValue))
        {
            auto cloned = object->clone();
            cloned->setProperty (unescaped, *newProperty);
            return var (cloned.release());
        }
    }
    else if (auto* array = v.getArray())
    {
        const auto index = [&]() -> size_t
        {
            if (unescaped == "-")
                return (size_t) array->size();

            if (unescaped == "0")
                return 0;

            if (! unescaped.startsWith ("0"))
                return (size_t) unescaped.getLargeIntValue();

            return std::numeric_limits<size_t>::max();
        }();

        if (const auto newIndex = setPointer ((*array)[(int) index], tail, newValue))
        {
            auto copied = *array;

            if ((int) index == copied.size())
                copied.add ({});

            if (isPositiveAndBelow (index, copied.size()))
            {
                copied.getReference ((int) index) = *newIndex;
                return var (copied);
            }
        }
    }

    return {};
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
        beginTest ("JSON pointers");
        {
            const auto obj = JSON::parse (R"({ "name":           "PIANO 4"
                                             , "lfoSpeed":       30
                                             , "lfoWaveform":    "triangle"
                                             , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50,50,50,50] }
                                             })");
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
                                                                                           , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50,50,50,50] }
                                                                                           })"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"([0,1,2])"), "/0", "bang"), JSON::parse (R"(["bang",1,2])"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"([0,1,2])"), "/0", "bang"), JSON::parse (R"(["bang",1,2])"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"/":"fizz"})"), "/~1", "buzz"), JSON::parse (R"({"/":"buzz"})"));
            expectDeepEqual (JSONUtils::setPointer (JSON::parse (R"({"~":"fizz"})"), "/~0", "buzz"), JSON::parse (R"({"~":"buzz"})"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/rates/0", 80), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                        , "lfoSpeed":       30
                                                                                                        , "lfoWaveform":    "triangle"
                                                                                                        , "pitchEnvelope":  { "rates": [80,67,95,60], "levels": [50,50,50,50] }
                                                                                                        })"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/levels/0", 80), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                         , "lfoSpeed":       30
                                                                                                         , "lfoWaveform":    "triangle"
                                                                                                         , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [80,50,50,50] }
                                                                                                         })"));
            expectDeepEqual (JSONUtils::setPointer (obj, "/pitchEnvelope/levels/-", 100), JSON::parse (R"({ "name":           "PIANO 4"
                                                                                                          , "lfoSpeed":       30
                                                                                                          , "lfoWaveform":    "triangle"
                                                                                                          , "pitchEnvelope":  { "rates": [94,67,95,60], "levels": [50,50,50,50,100] }
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
