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

namespace juce::detail
{

#if JUCE_UNIT_TESTS

template <typename T, typename = void>
constexpr auto hasGetStartFunction = false;

template <typename T>
constexpr auto hasGetStartFunction<T, std::void_t<decltype (std::declval<T>().getStart())>> = true;

template <typename RangeType, typename std::enable_if<hasGetStartFunction<RangeType>, int>::type = 0>
std::ostream& operator<< (std::ostream& os, const RangeType& range)
{
    os << "[" << range.getStart() << ", " << range.getEnd() << ")";
    return os;
}

static String& operator<< (String& s, Range<int64> r)
{
    return s += "[" + String { r.getStart() } + ", " + String { r.getEnd() } + ")";
}

template <typename ValueType>
static auto getCumulativeRangeLengths (const RangedValues<ValueType>& rv)
{
    int64 totalLength{};

    for (size_t i = 0; i < rv.size(); ++i)
        totalLength += rv.getItem (i).range.getLength();

    return totalLength;
}

template <typename ValueType>
static auto toString (const RangedValues<ValueType>& rv)
{
    String s {};

    for (size_t i = 0; i < rv.size(); ++i)
    {
        auto item = rv.getItem (i);
        s << item.range << ": " << item.value << "\n";
    }

    return s;
}

class RangesTestsBase : public UnitTest
{
public:
    using UnitTest::UnitTest;

    void expectRange (Range<int64> actual, Range<int64> expected)
    {
        String failureMessage { "range " };
        failureMessage << actual << " did not equal expected range " << expected;
        expect (actual == expected, failureMessage);
    }
};

class RangesTests : public RangesTestsBase
{
public:
    RangesTests() : RangesTestsBase ("Ranges", UnitTestCategories::containers) {}

    void runTest() override
    {
        beginTest ("Ranges::set() - basics");
        {
            Ranges ranges;

            ranges.set ({ -3, 14 });
            expectRange (ranges.get (0), { -3, 14 });

            ranges.set ({ 7, 20 });
            expectRange (ranges.get (0), { -3, 7 });
            expectRange (ranges.get (1), { 7, 20 });
        }

        beginTest ("Ranges::set() - neighbouring ranges extents are modified");
        {
            Ranges ranges;
            ranges.set ({ -3, 14 });
            ranges.set ({ 19, 30 });
            ranges.set ({ 10, 25 });

            // size_t doesn't always map to an existing overload for String::operator<< on all platforms
            expectEquals ((int64) ranges.size(), (int64)  3);
            expectRange (ranges.get (0), { -3, 10 });
            expectRange (ranges.get (1), { 10, 25 });
            expectRange (ranges.get (2), { 25, 30 });
        }

        beginTest ("Ranges::set() - setting a range inside another one splits that range");
        {
            Ranges ranges;
            ranges.set ({ -3, 14 });

            expectEquals ((int64) ranges.size(), (int64) 1);

            //==============================================================================
            ranges.set ({ 3, 7 });

            expectEquals ((int64) ranges.size(), (int64) 3);
            expectRange (ranges.get (0), { -3, 3 });
            expectRange (ranges.get (1), { 3, 7 });
            expectRange (ranges.get (2), { 7, 14 });
        }

        beginTest ("Ranges::set() - old ranges falling within the bounds of a newly set are erased");
        {
            Ranges ranges;
            ranges.set ({ 0, 5 });
            ranges.set ({ 5, 10 });
            ranges.set ({ 15, 20 });
            ranges.set ({ 25, 30 });
            ranges.set ({ 35, 50 });

            expectEquals ((int64) ranges.size(), (int64) 5);

            //==============================================================================
            ranges.set ({ 4, 36 });

            expectEquals ((int64) ranges.size(), (int64) 3);
            expectRange (ranges.get (0), { 0, 4 });
            expectRange (ranges.get (1), { 4, 36 });
            expectRange (ranges.get (2), { 36, 50 });
        }

        beginTest ("Ranges::set() - setting an empty range should be a no-op");
        {
            Ranges ranges;

            auto ops = ranges.set ({ 0, 0 });
            expect (ranges.isEmpty());
            expect (ops.empty());
        }

        beginTest ("Ranges::set() - setting a range inside another range");
        {
            Ranges ranges;

            ranges.set ({ 0, 48 });
            ranges.set ({ 48, 127 });
            ranges.set ({ 49, 94 });

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 49, 94 });
            expectRange (ranges.get (3), { 94, 127 });
        }

        beginTest ("Ranges::split()");
        {
            Ranges ranges;

            ranges.set ({ 0, 48 });
            ranges.set ({ 48, 127 });

            auto ops = ranges.split (47);

            expectEquals ((int64) ops.size(), (int64) 1, "");
            const auto op0 = std::get_if<Ranges::Ops::Split> (&ops[0]);
            expect (op0 != nullptr);

            if (op0 != nullptr)
                expectEquals ((int) op0->index, 0, "The 0th element should be split");

            expectEquals ((int64) ranges.size(), (int64) 3, "");
            expectRange (ranges.get (0), { 0, 47 });
            expectRange (ranges.get (1), { 47, 48 });
            expectRange (ranges.get (2), { 48, 127 });
        }

        beginTest ("Ranges::split() - splitting has no effect when no range begins before and ends after the location");
        {
            Ranges ranges;

            ranges.set ({ 0, 48 });
            ranges.set ({ 48, 127 });

            auto ops = ranges.split (48);
            expectEquals ((int64) ops.size(), (int64) 0, "");

            expectEquals ((int64) ranges.size(), (int64) 2, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 127 });
        }

        beginTest ("Ranges::insert() - basics");
        {
            Ranges ranges;

            ranges.insert ({ -3, 14 });
            expectRange (ranges.get (0), { -3, 14 });

            ranges.insert ({ 7, 20 });
            expectRange (ranges.get (0), { -3, 7 });
            expectRange (ranges.get (1), { 7, 20 });
            expectRange (ranges.get (2), { 20, 27 });
        }

        beginTest ("Ranges::insert() - inserting shifts all following ranges");
        {
            Ranges ranges;

            ranges.insert ({ 10, 11 });
            ranges.insert ({ 0, 1 });

            expectRange (ranges.get (0), { 0, 1 });
            expectRange (ranges.get (1), { 11, 12 });
        }

        beginTest ("Ranges::insert() - inserting an empty range should be a no-op");
        {
            Ranges ranges;

            auto ops = ranges.insert ({ 0, 0 });
            expect (ranges.isEmpty());
            expect (ops.empty());
        }

        const auto getTestRanges = []
        {
            Ranges ranges;

            ranges.set ({ 0, 48 });
            ranges.set ({ 48, 49 });
            ranges.set ({ 55, 94 });
            ranges.set ({ 94, 127 });

            return ranges;
        };

        beginTest ("Ranges::eraseFrom() - erasing beyond all ranges has no effect");
        {
            auto ranges = getTestRanges();
            ranges.eraseFrom (ranges.get (ranges.size() - 1).getEnd() + 5);

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 55, 94 });
            expectRange (ranges.get (3), { 94, 127 });
        }

        beginTest ("Ranges::eraseFrom() - erasing modifies the range that encloses the starting index");
        {
            auto ranges = getTestRanges();
            ranges.eraseFrom (122);

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 55, 94 });
            expectRange (ranges.get (3), { 94, 122 });
        }

        beginTest ("Ranges::eraseFrom() - ranges starting after the erased index are deleted entirely");
        {
            auto ranges = getTestRanges();
            ranges.eraseFrom (60);

            expectEquals ((int64) ranges.size(), (int64) 3, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 55, 60 });
        }

        beginTest ("Ranges::eraseFrom() - erasing from a location outside any ranges will still drop subsequent ranges");
        {
            auto ranges = getTestRanges();
            ranges.eraseFrom (51);

            expectEquals ((int64) ranges.size(), (int64) 2, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
        }

        beginTest ("Ranges::erase() - erasing a zero length range has no effect");
        {
            auto ranges = getTestRanges();

            ranges.erase ({ 30, 30 });

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 55, 94 });
            expectRange (ranges.get (3), { 94, 127 });
        }

        beginTest ("Ranges::erase() - erasing inside a range splits that range");
        {
            auto ranges = getTestRanges();

            ranges.erase ({ 30, 31 });

            expectEquals ((int64) ranges.size(), (int64) 5, "");
            expectRange (ranges.get (0), { 0, 30 });
            expectRange (ranges.get (1), { 31, 48 });
            expectRange (ranges.get (2), { 48, 49 });
            expectRange (ranges.get (3), { 55, 94 });
            expectRange (ranges.get (4), { 94, 127 });
        }

        beginTest ("Ranges::erase() - erasing a range that completely overlaps existing ranges erases those");
        {
            auto ranges = getTestRanges();

            ranges.erase ({ 30, 70 });

            expectEquals ((int64) ranges.size(), (int64) 3, "");
            expectRange (ranges.get (0), { 0, 30 });
            expectRange (ranges.get (1), { 70, 94 });
            expectRange (ranges.get (2), { 94, 127 });
        }

        beginTest ("Ranges::erase() - erasing a range that no previous ranges covered has no effect");
        {
            auto ranges = getTestRanges();

            ranges.erase ({ 51, 53 });

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 55, 94 });
            expectRange (ranges.get (3), { 94, 127 });
        }

        beginTest ("Ranges::erase() - erasing over a range beyond all limits clears all ranges");
        {
            auto ranges = getTestRanges();

            ranges.erase ({ -1000, 1000 });

            expect (ranges.isEmpty());
        }

        beginTest ("Ranges::drop() - dropping a range shifts all following ranges downward");
        {
            auto ranges = getTestRanges();

            ranges.drop ({ 48, 49 });

            expectEquals ((int64) ranges.size(), (int64) 3, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 54, 93 });
            expectRange (ranges.get (2), { 93, 126 });
        }

        beginTest ("Ranges::drop() - dropping a range shifts all following ranges downward even if no range covered the dropped range previously");
        {
            auto ranges = getTestRanges();

            ranges.drop ({ 51, 53 });

            expectEquals ((int64) ranges.size(), (int64) 4, "");
            expectRange (ranges.get (0), { 0, 48 });
            expectRange (ranges.get (1), { 48, 49 });
            expectRange (ranges.get (2), { 53, 92 });
            expectRange (ranges.get (3), { 92, 125 });
        }

        beginTest ("Ranges::drop() - dropping a range covering all other ranges empties the collection");
        {
            auto ranges = getTestRanges();

            ranges.drop ({ -1000, 1000 });
            expect (ranges.isEmpty());
        }
    }
};

class RangedValuesTests : public UnitTest
{
public:
    RangedValuesTests() : UnitTest ("RangedValues", UnitTestCategories::containers) {}

    template <typename ItemType>
    void expectRangedValuesItem (ItemType item, Range<int64> range, char value)
    {
        {
            String failureMessage { "range " };
            failureMessage << item.range << " did not equal expected range " << range;
            expect (item.range == range, failureMessage);
        }

        {
            String failureMessage { "value '" };
            failureMessage << item.value << "' in range " << range << " did not equal expected value '" << value << "'";
            expect (item.value == value, failureMessage);
        }
    }

    void runTest() override
    {
        auto random = getRandom();

        const auto createRangedValuesObject = [&]
        {
            RangedValues<char> rangedValues;

            rangedValues.set ({ 0, 10 }, 'a');
            rangedValues.set ({ 11, 20 }, 'b');
            rangedValues.set ({ 23, 30 }, 'c');

            return rangedValues;
        };

        beginTest ("RangedValues::set() with distinct value overlapping other ranges");
        {
            auto rangedValues = createRangedValuesObject();

            rangedValues.set ({ 5, 15 }, 'd');

            expect (! rangedValues.isEmpty());

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 5 }, 'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 5, 15 }, 'd');
            expectRangedValuesItem (rangedValues.getItem (2), { 15, 20 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (3), { 23, 30 }, 'c');

            rangedValues.set ({ 19, 24 }, 'e');

            expectRangedValuesItem (rangedValues.getItem (2), { 15, 19 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (3), { 19, 24 }, 'e');
            expectRangedValuesItem (rangedValues.getItem (4), { 24, 30 }, 'c');
        }

        beginTest ("RangedValues::set() with distinct value in corner cases");
        {
            auto rangedValues = createRangedValuesObject();

            rangedValues.set ({ -1, 0 }, 'd');

            expectRangedValuesItem (rangedValues.getItem (0), { -1, 0 }, 'd');
            expectRangedValuesItem (rangedValues.getItem (1), { 0, 10 }, 'a');
        }

        beginTest ("RangedValues::set() with same value with merging disallowed");
        {
            auto rangedValues = createRangedValuesObject();

            rangedValues.set<MergeEqualItems::no> ({ 5, 15 }, 'b');

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 5 },   'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 5, 15 },  'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 15, 20 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (3), { 23, 30 }, 'c');
        }

        beginTest ("RangedValues::set() with same value with merging allowed");
        {
            auto rangedValues = createRangedValuesObject();

            rangedValues.set<MergeEqualItems::yes> ({ 5, 15 }, 'b');

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 5 },   'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 5, 20 },  'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 23, 30 }, 'c');
        }

        beginTest ("RangedValues::set() - setting an empty Range should be a no-op");
        {
            RangedValues<char> rangedValues;

            auto ops = rangedValues.set ({ 0, 0 }, 'a');
            expect (rangedValues.isEmpty());
            expect (ops.empty());
        }

        beginTest ("RangedValues::set() - setting a range inside another range");
        {
            RangedValues<char> rangedValues;

            rangedValues.set ({ 0, 48 }, 'a');
            rangedValues.set ({ 48, 127 }, 'b');
            rangedValues.set ({ 49, 94 }, 'c');

            expectEquals ((int64) rangedValues.size(), (int64) 4, "");

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 48 },   'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 48, 49 },  'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 49, 94 },  'c');
            expectRangedValuesItem (rangedValues.getItem (3), { 94, 127 }, 'b');
        }

        beginTest ("RangedValues::getIntersectionsWith()");
        {
            auto rangedValues = createRangedValuesObject();

            {
                const auto intersections = rangedValues.getIntersectionsWith ({ 5, 43 });

                expectRangedValuesItem (intersections[0], { 5, 10 }, 'a');
                expectRangedValuesItem (intersections[1], { 11, 20 }, 'b');
                expectRangedValuesItem (intersections[2], { 23, 30 }, 'c');
            }

            {
                const auto intersections = rangedValues.getIntersectionsWith ({ -10, 3 });

                expectRangedValuesItem (intersections[0], { 0, 3 }, 'a');
            }
        }

        beginTest ("RangedValues::insert() fuzzing - insert always increases the total covered range");
        {
            for (auto i = 0; i != 100; ++i)
            {
                auto rangedValuesNotMerged = createRangedValuesObject();
                auto rangedValuesMerged = createRangedValuesObject();

                const auto totalLengthBeforeInsert = getCumulativeRangeLengths (rangedValuesNotMerged);

                const auto beginInsertionAt = (int64) random.nextInt (100) - 50;
                const auto numElemsToInsert = (int64) random.nextInt (1000);

                rangedValuesNotMerged.insert<MergeEqualItems::no> ({ Range<int64>::withStartAndLength (beginInsertionAt, numElemsToInsert) },
                                                                   'a' + (char) random.nextInt (25));

                expectEquals (getCumulativeRangeLengths (rangedValuesNotMerged) - totalLengthBeforeInsert, numElemsToInsert);

                rangedValuesMerged.insert<MergeEqualItems::yes> ({ Range<int64>::withStartAndLength (beginInsertionAt, numElemsToInsert) },
                                                                 'a' + (char) random.nextInt (25));

                expectEquals (getCumulativeRangeLengths (rangedValuesMerged) - totalLengthBeforeInsert, numElemsToInsert);
            }
        }

        beginTest ("RangedValues::insert() with distinct value inside another range");
        {
            auto rangedValues = createRangedValuesObject();

            expectEquals ((int64) rangedValues.size(), (int64) 3);

            rangedValues.insert ({ 2, 4 }, 'd');

            expectEquals ((int64) rangedValues.size(), (int64) 5);

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 2 },   'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 2, 4 },   'd');
            expectRangedValuesItem (rangedValues.getItem (2), { 4, 12 },  'a');
            expectRangedValuesItem (rangedValues.getItem (3), { 13, 22 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (4), { 25, 32 }, 'c');
        }

        beginTest ("RangedValues::insert() with same value inside another range");
        {
            {
                auto rangedValues = createRangedValuesObject();

                expectEquals ((int64) rangedValues.size(), (int64) 3);

                rangedValues.insert<MergeEqualItems::yes> ({ 2, 4 }, 'a');

                expectEquals ((int64) rangedValues.size(), (int64) 3);

                expectRangedValuesItem (rangedValues.getItem (0), { 0, 12 }, 'a');
                expectRangedValuesItem (rangedValues.getItem (1), { 13, 22 }, 'b');
                expectRangedValuesItem (rangedValues.getItem (2), { 25, 32 }, 'c');
            }
            {
                auto rangedValues = createRangedValuesObject();

                expectEquals ((int64) rangedValues.size(), (int64) 3);

                rangedValues.insert<MergeEqualItems::no> ({ 2, 4 }, 'a');

                expectEquals ((int64) rangedValues.size(), (int64) 5);

                expectRangedValuesItem (rangedValues.getItem (0), { 0, 2 },   'a');
                expectRangedValuesItem (rangedValues.getItem (1), { 2, 4 },   'a');
                expectRangedValuesItem (rangedValues.getItem (2), { 4, 12 },  'a');
                expectRangedValuesItem (rangedValues.getItem (3), { 13, 22 }, 'b');
                expectRangedValuesItem (rangedValues.getItem (4), { 25, 32 }, 'c');
            }
        }

        beginTest ("RangedValues::insert() - inserting an empty Range should be a no-op");
        {
            {
                RangedValues<char> emptyRangedValues;

                auto ops = emptyRangedValues.insert ({ 0, 0 }, 'a');
                expect (emptyRangedValues.isEmpty());
                expect (ops.empty());
            }

            {
                RangedValues<char> rangedValues;
                rangedValues.set ({ 0, 10 }, 'a');

                auto ops = rangedValues.insert ({ 0, 0 }, 'a');
                expect (ops.empty());
                expectEquals ((int64) rangedValues.size(), (int64) 1);
                expectRangedValuesItem (rangedValues.getItem (0), { 0, 10 },   'a');
            }
        }

        const auto createRangedValuesObjectForErase = [&]
        {
            RangedValues<char> rangedValues;

            rangedValues.set ({ 0, 10 }, 'a');
            rangedValues.set ({ 11, 20 }, 'b');
            rangedValues.set ({ 23, 30 }, 'c');
            rangedValues.set ({ 35, 45 }, 'c');
            rangedValues.set ({ 45, 60 }, 'd');

            return rangedValues;
        };

        beginTest ("RangedValues::erase() - erase will not shift subsequent ranges downward");
        {
            auto rangedValues = createRangedValuesObjectForErase();

            rangedValues.erase ({ 15, 16 });

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 10 },  'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 11, 15 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 16, 20 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (3), { 23, 30 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (4), { 35, 45 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (5), { 45, 60 }, 'd');
        }

        beginTest ("RangedValues::drop() - drop shifts ranges downward on the right side");
        {
            auto rangedValues = createRangedValuesObjectForErase();

            rangedValues.drop<MergeEqualItems::no> ({ 15, 16 });

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 10 },  'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 11, 15 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 15, 19 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (3), { 22, 29 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (4), { 34, 44 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (5), { 44, 59 }, 'd');
        }

        beginTest ("RangedValues::drop() - drop can be used to merge equal values");
        {
            auto rangedValues = createRangedValuesObjectForErase();

            rangedValues.drop<MergeEqualItems::yes> ({ 15, 16 });

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 10 },  'a');
            expectRangedValuesItem (rangedValues.getItem (1), { 11, 19 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 22, 29 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (3), { 34, 44 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (4), { 44, 59 }, 'd');
        }

        beginTest ("RangedValues::drop() - the merge happens at the seam caused by the drop and does not extend beyond");
        {
            auto rangedValues = createRangedValuesObjectForErase();
            rangedValues.set<MergeEqualItems::no> ({ 20, 30 }, 'b');

            rangedValues.drop<MergeEqualItems::yes> ({ 15, 16 });

            expectRangedValuesItem (rangedValues.getItem (0), { 0, 10 },  'a');

            // These two items are not merged, even though they form a contiguous range, because
            // they were disjoint before the drop and they don't touch each other at the drop
            // seam of 15.
            expectRangedValuesItem (rangedValues.getItem (1), { 11, 19 }, 'b');
            expectRangedValuesItem (rangedValues.getItem (2), { 19, 29 }, 'b');

            expectRangedValuesItem (rangedValues.getItem (3), { 34, 44 }, 'c');
            expectRangedValuesItem (rangedValues.getItem (4), { 44, 59 }, 'd');
        }
    }
};

class IntersectingRangedValuesTests : public RangesTestsBase
{
public:
    IntersectingRangedValuesTests() : RangesTestsBase ("IntersectingRangedValues", UnitTestCategories::containers) {}

    void runTest() override
    {
        beginTest ("IntersectingRangedValuesTests - iterating over multiple RangedValues");
        {
            RangedValues<int> rv1;
            rv1.set ({ 3, 8},   1);
            rv1.set ({ 9, 16},  2);
            rv1.set ({ 30, 40}, 3);

            RangedValues<int> rv2;
            rv2.set ({ 0, 4},   7);
            rv2.set ({ 4, 6},   11);
            rv2.set ({ 6, 25},  13);
            rv2.set ({ 27, 55}, 17);

            RangedValues<int> rv3;
            rv3.set ({ -2, 10}, -1);
            rv3.set ({ 15, 19}, -2);
            rv3.set ({ 22, 36}, -3);

            int iteration = 0;

            for (const auto [range, v1, v2, v3] : makeIntersectingRangedValues (&rv1, &rv2, &rv3))
            {
                if (iteration == 0)
                {
                     expectRange (range, Range<int64> { 3, 4 });
                     expect (v1 == 1 && v2 == 7 && v3 == -1);
                }

                if (iteration == 1)
                {
                     expectRange (range, Range<int64> { 4, 6 });
                     expect (v1 == 1 && v2 == 11 && v3 == -1);
                }

                if (iteration == 2)
                {
                     expectRange (range, Range<int64> { 6, 8 });
                     expect (v1 == 1 && v2 == 13 && v3 == -1);
                }

                if (iteration == 3)
                {
                     expectRange (range, Range<int64> { 9, 10 });
                     expect (v1 == 2 && v2 == 13 && v3 == -1);
                }

                if (iteration == 4)
                {
                     expectRange (range, Range<int64> { 15, 16 });
                     expect (v1 == 2 && v2 == 13 && v3 == -2);
                }

                if (iteration == 5)
                {
                     expectRange (range, Range<int64> { 30, 36 });
                     expect (v1 == 3 && v2 == 17 && v3 == -3);
                }

                ++iteration;
            }

            expectEquals (iteration, 6);
        }
    }
};

static RangesTests rangesTests;
static RangedValuesTests rangedValuesTests;
static IntersectingRangedValuesTests intersectingRangedValuesTests;

#endif

} // namespace juce::detail
