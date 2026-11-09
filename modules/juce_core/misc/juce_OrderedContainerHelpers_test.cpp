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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

class OrderedContainerHelpersTests final : public UnitTest
{
public:
    OrderedContainerHelpersTests() : UnitTest ("OrderedContainerHelpersTests",
                                                UnitTestCategories::containers) {}

    void runTest() override
    {
        using Helper = OrderedContainerHelpers;

        beginTest ("find - element found");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            auto iter = Helper::find (sorted, 5);
            expect (iter != sorted.end());
            expectEquals (*iter, 5);

            iter = Helper::find (sorted, 1);
            expect (iter != sorted.end());
            expectEquals (*iter, 1);

            iter = Helper::find (sorted, 9);
            expect (iter != sorted.end());
            expectEquals (*iter, 9);
        }

        beginTest ("find - element not found");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            auto iter = Helper::find (sorted, 4);
            expect (iter == sorted.end());

            iter = Helper::find (sorted, 0);
            expect (iter == sorted.end());

            iter = Helper::find (sorted, 10);
            expect (iter == sorted.end());
        }

        beginTest ("find - empty container");
        {
            std::vector<int> empty;
            auto iter = Helper::find (empty, 5);
            expect (iter == empty.end());
        }

        beginTest ("find - custom comparator");
        {
            std::vector<int> descending { 9, 7, 5, 3, 1 };
            auto desc = [] (int a, int b) { return a > b; };

            auto iter = Helper::find (descending, 5, desc);
            expect (iter != descending.end());
            expectEquals (*iter, 5);

            iter = Helper::find (descending, 4, desc);
            expect (iter == descending.end());
        }

        beginTest ("contains - element exists");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (Helper::contains (sorted, 5));
            expect (Helper::contains (sorted, 1));
            expect (Helper::contains (sorted, 9));
        }

        beginTest ("contains - element does not exist");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (! Helper::contains (sorted, 4));
            expect (! Helper::contains (sorted, 0));
            expect (! Helper::contains (sorted, 10));
        }

        beginTest ("insertOrAssign - insert new element");
        {
            std::vector<int> sorted { 1, 3, 7, 9 };

            Helper::insertOrAssign (sorted, 5);
            expect (sorted == std::vector { 1, 3, 5, 7, 9 });
        }

        beginTest ("insertOrAssign - assign existing element");
        {
            struct Pair
            {
                int key;
                String value;

                auto tie() const { return std::tie (key, value); }

                bool operator== (const Pair& other) const { return tie() == other.tie(); }
                bool operator< (const Pair& other) const { return key < other.key; }
            };

            std::vector<Pair> sorted { { 1, "one" }, { 3, "three" }, { 5, "five" } };

            Helper::insertOrAssign (sorted, Pair { 3, "THREE" });
            expect (sorted == std::vector { Pair { 1, "one"   },
                                            Pair { 3, "THREE" },
                                            Pair { 5, "five"  } });

            beginTest ("find - search by value using only key");
            {
                auto iter = Helper::find (sorted, Pair { 3, "ignored" });
                expect (iter->value == "THREE");
            }
        }

        beginTest ("insertOrAssign - insert at beginning");
        {
            std::vector<int> sorted { 3, 5, 7 };
            Helper::insertOrAssign (sorted, 1);
            expect (sorted == std::vector { 1, 3, 5, 7 });
        }

        beginTest ("insertOrAssign - insert at end");
        {
            std::vector<int> sorted { 1, 3, 5 };
            Helper::insertOrAssign (sorted, 7);
            expect (sorted == std::vector { 1, 3, 5, 7 });
        }

        beginTest ("insertOrAssign - empty container");
        {
            std::vector<int> empty;
            Helper::insertOrAssign (empty, 5);
            expect (empty == std::vector { 5 });
        }

        beginTest ("remove - element exists");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (Helper::remove (sorted, 5));
            expect (! Helper::contains (sorted, 5));
            expect (sorted == std::vector { 1, 3, 7, 9 });
        }

        beginTest ("remove - element does not exist");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (! Helper::remove (sorted, 4));
            expect (sorted == std::vector { 1, 3, 5, 7, 9 });
        }

        beginTest ("remove - first element");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (Helper::remove (sorted, 1));
            expect (sorted == std::vector<int> { 3, 5, 7, 9 });
        }

        beginTest ("remove - last element");
        {
            std::vector<int> sorted { 1, 3, 5, 7, 9 };

            expect (Helper::remove (sorted, 9));
            expect (sorted == std::vector<int> { 1, 3, 5, 7 });
        }

        beginTest ("remove - empty container");
        {
            std::vector<int> empty;

            expect (! Helper::remove (empty, 5));
            expect (empty.empty());
        }

        beginTest ("remove - single element");
        {
            std::vector<int> single { 5 };

            expect (Helper::remove (single, 5));
            expect (single.empty());
        }
    }
};

static OrderedContainerHelpersTests orderedContainerHelpersTests;

} // namespace juce
