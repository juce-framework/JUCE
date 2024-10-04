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

static const int minNumberOfStringsForGarbageCollection = 300;
static const uint32 garbageCollectionInterval = 30000;


StringPool::StringPool() noexcept  : lastGarbageCollectionTime (0) {}

struct StartEndString
{
    StartEndString (String::CharPointerType s, String::CharPointerType e) noexcept : start (s), end (e) {}
    operator String() const   { return String (start, end); }

    String::CharPointerType start, end;
};

static int compareStrings (const String& s1, const String& s2) noexcept     { return s1.compare (s2); }
static int compareStrings (CharPointer_UTF8 s1, const String& s2) noexcept  { return s1.compare (s2.getCharPointer()); }

static int compareStrings (const StartEndString& string1, const String& string2) noexcept
{
    String::CharPointerType s1 (string1.start), s2 (string2.getCharPointer());

    for (;;)
    {
        const int c1 = s1 < string1.end ? (int) s1.getAndAdvance() : 0;
        const int c2 = (int) s2.getAndAdvance();
        const int diff = c1 - c2;

        if (diff != 0)  return diff < 0 ? -1 : 1;
        if (c1 == 0)    break;
    }

    return 0;
}

template <typename NewStringType>
static String addPooledString (Array<String>& strings, const NewStringType& newString)
{
    int start = 0;
    int end = strings.size();

    while (start < end)
    {
        const String& startString = strings.getReference (start);
        const int startComp = compareStrings (newString, startString);

        if (startComp == 0)
            return startString;

        const int halfway = (start + end) / 2;

        if (halfway == start)
        {
            if (startComp > 0)
                ++start;

            break;
        }

        const String& halfwayString = strings.getReference (halfway);
        const int halfwayComp = compareStrings (newString, halfwayString);

        if (halfwayComp == 0)
            return halfwayString;

        if (halfwayComp > 0)
            start = halfway;
        else
            end = halfway;
    }

    strings.insert (start, newString);
    return strings.getReference (start);
}

String StringPool::getPooledString (const char* const newString)
{
    if (newString == nullptr || *newString == 0)
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, CharPointer_UTF8 (newString));
}

String StringPool::getPooledString (String::CharPointerType start, String::CharPointerType end)
{
    if (start.isEmpty() || start == end)
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, StartEndString (start, end));
}

String StringPool::getPooledString (StringRef newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, newString.text);
}

String StringPool::getPooledString (const String& newString)
{
    if (newString.isEmpty())
        return {};

    const ScopedLock sl (lock);
    garbageCollectIfNeeded();
    return addPooledString (strings, newString);
}

void StringPool::garbageCollectIfNeeded()
{
    if (strings.size() > minNumberOfStringsForGarbageCollection
         && Time::getApproximateMillisecondCounter() > lastGarbageCollectionTime + garbageCollectionInterval)
        garbageCollect();
}

void StringPool::garbageCollect()
{
    const ScopedLock sl (lock);

    for (int i = strings.size(); --i >= 0;)
        if (strings.getReference (i).getReferenceCount() == 1)
            strings.remove (i);

    lastGarbageCollectionTime = Time::getApproximateMillisecondCounter();
}

StringPool& StringPool::getGlobalPool() noexcept
{
    static StringPool pool;
    return pool;
}

Array<Identifier> StringPool::addSortedStrings (const Array<String>& stringsToAdd)
{
    // Assert that this is the global pool
    jassert (this == &getGlobalPool());

    if (stringsToAdd.isEmpty())
        return {};

    // Ensure the input array is sorted and contains no duplicates
    jassert (std::is_sorted (stringsToAdd.begin(), stringsToAdd.end(), [](const auto& a, const auto& b) {
        return compareStrings (a, b) < 0;
    }));

    jassert(std::adjacent_find (stringsToAdd.begin(),
                               stringsToAdd.end(),
                               [](const auto& a, const auto& b) { return compareStrings (a, b) == 0; })
            == stringsToAdd.end());

    // Ensure the existing pool is sorted (very long check)
    //    jassert(std::is_sorted(
    //        strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) < 0; }));

    const ScopedLock sl (lock);

    Array<Identifier> result;
    result.resize (stringsToAdd.size());

    int start = 0;
    const int end = stringsToAdd.size();

    while (start < end) {
        const auto& startString = stringsToAdd.getReference (start);
        auto [found, insertionIndex] = locateOrGetInsertIndex (startString, 0, strings.size());

        if (found) {
            result.set (start, Identifier (strings.getReference (insertionIndex), true));
            start++;
            continue;
        }

        int numElems;
        auto low = start + 1;
        auto high = end;

        auto [endFound, endInsertionIndex] = locateOrGetInsertIndex (stringsToAdd.getLast(), insertionIndex, strings.size());

        // Check if all the remaining elements can be inserted at insertion_index
        if (!endFound && endInsertionIndex == insertionIndex) {

            numElems = end - start;

        } else {
            // Binary search to find the last element that can be inserted at insertion_index
            while (low < high) {
                auto mid = low + (high - low) / 2;
                auto [newFound, newInsertionIndex] =
                    locateOrGetInsertIndex (stringsToAdd.getReference (mid), insertionIndex, strings.size());

                if (newFound || newInsertionIndex != insertionIndex)
                    high = mid;
                else
                    low = mid + 1;
            }

            numElems = low - start;
        }

        // Insert the elements
        strings.insertArray (insertionIndex, stringsToAdd.begin() + start, numElems);

        for (int i = 0; i < numElems; i++)
            result.set (i + start, Identifier (strings.getReference (insertionIndex + i), true));

        start += numElems;
    }

    // Ensure the pool is still sorted and contains no duplicates (very long checks)
    //    jassert(std::is_sorted(
    //        strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) < 0; }));
    //
    //    jassert(std::adjacent_find(
    //                strings.begin(), strings.end(), [](const auto& a, const auto& b) { return compareStrings(a, b) == 0; })
    //            == strings.end());

    return result;
}

std::pair<bool, int> StringPool::locateOrGetInsertIndex (const String& newString, int startIndex, int endIndex) const
{
    int start = startIndex;
    int end = endIndex;

    while (start < end) {
        const int halfway = (start + end) / 2;
        const String& halfwayString = strings.getReference (halfway);
        const int halfwayComp = compareStrings (newString, halfwayString);

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
