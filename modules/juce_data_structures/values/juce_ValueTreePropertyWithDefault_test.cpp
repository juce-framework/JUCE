/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class ValueTreePropertyWithDefaultTests  : public UnitTest
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
