/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_UNIT_TESTS

class CachedValueTests  : public UnitTest
{
public:
    CachedValueTests() : UnitTest ("CachedValues") {}

    void runTest() override
    {
        beginTest ("default constructor");
        {
            CachedValue<String> cv;
            expect (cv.isUsingDefault());
            expect (cv.get() == String());
        }

        beginTest ("without default value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            CachedValue<String> cv (t, "testkey", nullptr);

            expect (! cv.isUsingDefault());
            expect (cv.get() == "testvalue");

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv.get() == String());
        }

        beginTest ("with default value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "testvalue", nullptr);

            CachedValue<String> cv (t, "testkey", nullptr, "defaultvalue");

            expect (! cv.isUsingDefault());
            expect (cv.get() == "testvalue");

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv.get() == "defaultvalue");
        }

        beginTest ("with default value (int)");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", 23, nullptr);

            CachedValue<int> cv (t, "testkey", nullptr, 34);

            expect (! cv.isUsingDefault());
            expect (cv == 23);
            expectEquals (cv.get(), 23);

            cv.resetToDefault();

            expect (cv.isUsingDefault());
            expect (cv == 34);
        }

        beginTest ("with void value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", var(), nullptr);

            CachedValue<String> cv (t, "testkey", nullptr, "defaultvalue");

            expect (! cv.isUsingDefault());
            expect (cv == "");
            expectEquals (cv.get(), String());
        }

        beginTest ("with non-existent value");
        {
            ValueTree t ("root");

            CachedValue<String> cv (t, "testkey", nullptr, "defaultvalue");

            expect (cv.isUsingDefault());
            expect (cv == "defaultvalue");
            expect (cv.get() == "defaultvalue");
        }

        beginTest ("with value changing");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", "oldvalue", nullptr);

            CachedValue<String> cv (t, "testkey", nullptr, "defaultvalue");
            expect (cv == "oldvalue");

            t.setProperty ("testkey", "newvalue", nullptr);
            expect (cv != "oldvalue");
            expect (cv == "newvalue");
        }

        beginTest ("set value");
        {
            ValueTree t ("root");
            t.setProperty ("testkey", 23, nullptr);

            CachedValue<int> cv (t, "testkey", nullptr, 45);
            cv = 34;

            expectEquals ((int) t["testkey"], 34);

            cv.resetToDefault();
            expect (cv == 45);
            expectEquals (cv.get(), 45);

            expect (t["testkey"] == var());
        }

        beginTest ("reset value");
        {

        }
    }
};

static CachedValueTests cachedValueTests;

#endif
