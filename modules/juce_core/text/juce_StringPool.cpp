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

static const int minNumberOfStringsForGarbageCollection = 300;
static const uint32 garbageCollectionInterval = 30000;

StringPool::StringPool() noexcept
: lastGarbageCollectionTime(0)
{
}

struct StartEndString {
    StartEndString(String::CharPointerType s, String::CharPointerType e) noexcept
    : start(s),
      end(e)
    {
    }
    operator String() const
    {
        return String(start, end);
    }

    String::CharPointerType start, end;
};

static int compareStrings(const String& s1, const String& s2) noexcept
{
    return s1.compare(s2);
}
static int compareStrings(CharPointer_UTF8 s1, const String& s2) noexcept
{
    return s1.compare(s2.getCharPointer());
}

static int compareStrings(const StartEndString& string1, const String& string2) noexcept
{
    String::CharPointerType s1(string1.start), s2(string2.getCharPointer());

    for (;;) {
        const int c1 = s1 < string1.end ? (int)s1.getAndAdvance() : 0;
        const int c2 = (int)s2.getAndAdvance();
        const int diff = c1 - c2;

        if (diff != 0)
            return diff < 0 ? -1 : 1;
        if (c1 == 0)
            break;
    }

    return 0;
}

template <typename NewStringType>
static String addPooledString(Array<String>& strings, const NewStringType& newString)
{
    int start = 0;
    int end = strings.size();

    while (start < end) {
        const String& startString = strings.getReference(start);
        const int startComp = compareStrings(newString, startString);

        if (startComp == 0)
            return startString;

        const int halfway = (start + end) / 2;

        if (halfway == start) {
            if (startComp > 0)
                ++start;

            break;
        }

        const String& halfwayString = strings.getReference(halfway);
        const int halfwayComp = compareStrings(newString, halfwayString);

        if (halfwayComp == 0)
            return halfwayString;

        if (halfwayComp > 0)
            start = halfway;
        else
            end = halfway;
    }

    strings.insert(start, newString);
    return strings.getReference(start);
}

String StringPool::getPooledString(const char* const newString)
{
    if (newString == nullptr || *newString == 0)
        return {};

    const ScopedLock sl(lock);
    garbageCollectIfNeeded();
    return addPooledString(strings, CharPointer_UTF8(newString));
}

String StringPool::getPooledString(String::CharPointerType start, String::CharPointerType end)
{
    if (start.isEmpty() || start == end)
        return {};

    const ScopedLock sl(lock);
    garbageCollectIfNeeded();
    return addPooledString(strings, StartEndString(start, end));
}

String StringPool::getPooledString(StringRef newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl(lock);
    garbageCollectIfNeeded();
    return addPooledString(strings, newString.text);
}

String StringPool::getPooledString(const String& newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl(lock);
    garbageCollectIfNeeded();
    return addPooledString(strings, newString);
}

void StringPool::garbageCollectIfNeeded()
{
    if (strings.size() > minNumberOfStringsForGarbageCollection
        && Time::getApproximateMillisecondCounter() > lastGarbageCollectionTime + garbageCollectionInterval)
        garbageCollect();
}

void StringPool::garbageCollect()
{
    const ScopedLock sl(lock);

    for (int i = strings.size(); --i >= 0;)
        if (strings.getReference(i).getReferenceCount() == 1)
            strings.remove(i);

    lastGarbageCollectionTime = Time::getApproximateMillisecondCounter();
}

StringPool& StringPool::getGlobalPool() noexcept
{
    static StringPool pool;
    return pool;
}

void StringPool::ensureAdditionalStorageAllocated(int numStringsNeeded)
{
    const ScopedLock sl(lock);
    strings.ensureStorageAllocated(strings.size() + numStringsNeeded);
}

Array<Identifier> StringPool::addSortedStrings(const Array<String>& stringsToAdd)
{
    if (stringsToAdd.isEmpty()) {
        return {};
    }

    // Ensure the input array is sorted and contains no duplicates
    jassert(std::is_sorted(stringsToAdd.begin(), stringsToAdd.end(), [](const auto& a, const auto& b) {
        return compareStrings(a, b) < 0;
    }));
    jassert(std::adjacent_find(stringsToAdd.begin(),
                               stringsToAdd.end(),
                               [](const auto& a, const auto& b) { return compareStrings(a, b) == 0; })
            == stringsToAdd.end());

    // Ensure the existing pool is sorted
//    jassert(std::is_sorted(
//        strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) < 0; }));


    const ScopedLock sl(lock);

    Array<Identifier> result;
    result.resize(stringsToAdd.size());

    int start = 0;
    const int end = stringsToAdd.size();


    while (start < end) {
        const auto& startString = stringsToAdd.getReference(start);
        auto [found, insertion_index] = getIndex(startString, 0, strings.size());

        if (found) {
            result.set(start, Identifier(strings.getReference(insertion_index), true));
            start++;
//            pool_start_search_index = insertion_index;
            continue;
        }

        //        // Determine the number of elements that can be inserted at insertion_index (using binary search)
        //        auto rangeEnd =
        //            std::upper_bound(strings.begin() + insertion_index + 1,
        //                             strings.end(),
        //                             insertion_index,
        //                             [pool_end_search_index, this](const auto& expected_insertion_index, const auto& inString) {
        //                                 auto [already_in, current_insertion_index] =
        //                                     getIndex(inString, expected_insertion_index, pool_end_search_index);
        //
        //                                 return already_in ? true : current_insertion_index != expected_insertion_index;
        //                             });
        //                auto num_elems = (int)std::distance(strings.begin() + insertion_index, rangeEnd);

        int num_elems = 1;
        // TODO: This is a linear search, but it should be possible to do a binary search here
        while (num_elems < end - start) {
            auto [new_found, new_insertion_index] =
                getIndex(stringsToAdd.getReference(start + num_elems), insertion_index, strings.size());

            if (new_found || new_insertion_index != insertion_index) {
                break;
            } else {
                num_elems++;
            }
        }


        // Insert the elements
        strings.insertArray(insertion_index, stringsToAdd.begin() + start, num_elems);

        for (int i = 0; i < num_elems; i++) {
            result.set(i + start, Identifier(strings.getReference(insertion_index + i), true));
        }

        start += num_elems;
    }

//    jassert(std::is_sorted(
//        strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) < 0; }));
//
//    jassert(std::adjacent_find(
//                strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) == 0; })
//            == strings.end());

    return result;
}

std::pair<bool, int> StringPool::getIndex(const String& newString, int startIndex, int endIndex) const
{
    int start = startIndex;
    int end = endIndex;

    while (start < end) {
        const int halfway = (start + end) / 2;
        const String& halfwayString = strings.getReference(halfway);
        const int halfwayComp = compareStrings(newString, halfwayString);

        if (halfwayComp == 0)
            return { true, halfway };

        if (halfwayComp > 0)
            start = halfway + 1;
        else
            end = halfway;
    }

    return { false, start };
}

} // namespace juce
