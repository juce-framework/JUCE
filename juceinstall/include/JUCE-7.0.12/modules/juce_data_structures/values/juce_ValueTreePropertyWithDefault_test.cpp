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

namespace juce
{

class ValueTreePropertyWithDefaultTests final : public UnitTest
{
public:
    ValueTreePropertyWithDefaultTests()
        : UnitTest ("ValueTreePropertyWithDefault", UnitTestCategories::values)
    {}

    void runTest() override
    {
        beginTest ("default constructor");
        {
            ValueTreePropertyWithDefault value;
            expect (value.isUsingDefault());
            expect (value.get() == var());
        }

        beginTest ("missing property");
        {
            ValueTree t ("root");
            ValueTreePropertyWithDefault value (t, "testKey", nullptr, "default");

            expect (value.isUsingDefault());
            expectEquals (value.get().toString(), String ("default"));
        }

        beginTest ("non-empty property");
        {
            ValueTree t ("root");
            t.setProperty ("testKey", "non-default", nullptr);

            ValueTreePropertyWithDefault value (t, "testKey", nullptr, "default");

            expect (! value.isUsingDefault());
            expectEquals (value.get().toString(), String ("non-default"));
        }

        beginTest ("set default");
        {
            ValueTree t ("root");

            ValueTreePropertyWithDefault value (t, "testkey", nullptr);
            value.setDefault ("default");

            expect (value.isUsingDefault());
            expectEquals (value.get().toString(), String ("default"));
        }

        beginTest ("set value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            ValueTreePropertyWithDefault value (t, "testkey", nullptr, "default");
            value = "newvalue";

            expect (! value.isUsingDefault());
            expectEquals (t["testkey"].toString(), String ("newvalue"));

            value.resetToDefault();

            expect (value.isUsingDefault());
            expect (t["testkey"] == var());
        }
    }
};

static ValueTreePropertyWithDefaultTests valueTreePropertyWithDefaultTests;

} // namespace juce
