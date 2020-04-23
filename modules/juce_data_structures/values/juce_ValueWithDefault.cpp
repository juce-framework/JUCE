/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_UNIT_TESTS

class ValueWithDefaultTests  : public UnitTest
{
public:
    ValueWithDefaultTests()
        : UnitTest ("ValueWithDefault", UnitTestCategories::values)
    {}

    void runTest() override
    {
        beginTest ("default constructor");
        {
            ValueWithDefault vwd;
            expect (vwd.isUsingDefault());
            expect (vwd.get() == var());
        }

        beginTest ("missing property");
        {
            ValueTree t ("root");
            ValueWithDefault vwd (t, "testKey", nullptr, "default");

            expect (vwd.isUsingDefault());
            expectEquals (vwd.get().toString(), String ("default"));
        }

        beginTest ("non-empty property");
        {
            ValueTree t ("root");
            t.setProperty ("testKey", "non-default", nullptr);

            ValueWithDefault vwd (t, "testKey", nullptr, "default");

            expect (! vwd.isUsingDefault());
            expectEquals (vwd.get().toString(), String ("non-default"));
        }

        beginTest ("set default");
        {
            ValueTree t ("root");

            ValueWithDefault vwd (t, "testkey", nullptr);
            vwd.setDefault ("default");

            expect (vwd.isUsingDefault());
            expectEquals (vwd.get().toString(), String ("default"));
        }

        beginTest ("set value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            ValueWithDefault vwd (t, "testkey", nullptr, "default");
            vwd = "newvalue";

            expect (! vwd.isUsingDefault());
            expectEquals (t["testkey"].toString(), String ("newvalue"));

            vwd.resetToDefault();

            expect (vwd.isUsingDefault());
            expect (t["testkey"] == var());
        }
    }
};

static ValueWithDefaultTests valueWithDefaultTests;

#endif

} // namespace juce
