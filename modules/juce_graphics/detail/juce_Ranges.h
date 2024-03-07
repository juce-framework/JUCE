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

inline std::optional<Range<int64>> getRangeIntersectionWith (Range<int64> r1, Range<int64> r2)
{
    auto intersection = r1.getIntersectionWith (r2);

    if (intersection.getLength() == 0)
        return std::nullopt;

    return Range<int64> { intersection };
}

inline size_t clampCast (int64 v)
{
    return v < 0 ? 0 : (size_t) v;
}

/*  Used in Ranges::getAffectedElements(). Yes means that if a change occurs to range i, we also
    want ranges i - 1 and i + 1 included in the affected elements.
*/
enum class IncludeAdjacentRanges
{
    no,
    yes
};

struct Ranges final
{
    struct Ops
    {
        Ops() = delete;

        struct Erased     { Range<size_t> range; };
        struct Inserted   { size_t index; };
        struct Reinserted { size_t index; };
        struct Changed    { size_t index; };
    };

    using Op = std::variant<Ops::Erased, Ops::Inserted, Ops::Reinserted, Ops::Changed>;

    using Operations = std::vector<Op>;

    Ranges() = default;

    explicit Ranges (std::vector<Range<int64>> rangesIn)
        : ranges (std::move (rangesIn))
    {
       #if JUCE_DEBUG
        jassert (std::is_sorted (ranges.cbegin(), ranges.cend(), [] (const auto& a, const auto& b)
        {
            return a.getEnd() < b.getStart();
        }));
       #endif
    }

    void clear()
    {
        ranges.clear();
    }

    /*  Returns the ranges that have an intersection with the provided range. */
    std::vector<Range<int64>> getIntersectionsWith (Range<int64> r) const
    {
        std::vector<Range<int64>> result;

        const auto firstOverlapping = [&]
        {
            auto it = std::lower_bound (ranges.cbegin(),
                                        ranges.cend(),
                                        r,
                                        [] (auto& elem, auto& value)
                                        { return elem.getEnd() <= value.getStart(); });

            return it;
        }();

        const auto lastOverlapping = [&]
        {
            auto it = std::lower_bound (firstOverlapping,
                                        ranges.cend(),
                                        r,
                                        [] (auto& elem, auto& value)
                                        { return elem.getEnd() < value.getEnd(); });

            return it != std::cend (ranges) ? it + 1 : it;
        }();

        for (auto it = firstOverlapping; it != lastOverlapping; ++it)
            if (auto intersection = getRangeIntersectionWith (Range<int64> { *it }, r))
                result.push_back (*intersection);

        return result;
    }

    Operations set (Range<int64> newRange)
    {
        if (newRange.isEmpty())
            return {};

        Operations ops;

        const auto firstStartingBeforeNewRange = [&]
        {
            auto it = std::lower_bound (ranges.begin(),
                                        ranges.end(),
                                        newRange,
                                        [] (auto& elem, auto& value)
                                        { return elem.getStart() < value.getStart(); });

            if (! ranges.empty() && it != ranges.begin())
                return it - 1;

            return ranges.end();
        }();

        const auto getFirstEndingAfterNewRange = [&]
        {
            auto it = std::lower_bound (ranges.begin(),
                                        ranges.end(),
                                        newRange,
                                        [] (auto& elem, auto& value)
                                        { return elem.getEnd() <= value.getEnd(); });

            return it;
        };

        auto firstEndingAfterNewRange = getFirstEndingAfterNewRange();

        // This variable helps with handling the corner case, when the newValue to be set lies
        // entirely inside an existing range. The set() operation in this case is expected to split
        // the existing range.
        auto remainderRangeBecauseOfSplit = [&]() -> std::optional<Range<int64>>
        {
            if (firstStartingBeforeNewRange == ranges.end() || firstStartingBeforeNewRange != firstEndingAfterNewRange)
                return std::nullopt;

            return Range<int64> { std::max (newRange.getEnd(), firstEndingAfterNewRange->getStart()),
                                  firstEndingAfterNewRange->getEnd() };
        }();

        if (firstStartingBeforeNewRange != ranges.end())
        {
            const auto oldEnd = firstStartingBeforeNewRange->getEnd();
            const auto newEnd = std::min (oldEnd, newRange.getStart());

            firstStartingBeforeNewRange->setEnd (newEnd);

            if (oldEnd != newEnd)
                ops.push_back (Ops::Changed { getIndex (firstStartingBeforeNewRange) });
        }

        if (! remainderRangeBecauseOfSplit.has_value()
            && firstEndingAfterNewRange != ranges.end())
        {
            const auto oldStart = firstEndingAfterNewRange->getStart();
            const auto newStart = std::max (firstEndingAfterNewRange->getStart(), newRange.getEnd());

            firstEndingAfterNewRange->setStart (newStart);

            if (oldStart != newStart)
                ops.push_back (Ops::Changed { getIndex (firstStartingBeforeNewRange) });
        }

        const auto firstToDelete = std::lower_bound (ranges.begin(),
                                                     ranges.end(),
                                                     newRange,
                                                     [] (auto& elem, auto& value)
                                                     { return elem.getStart() < value.getStart(); });

        firstEndingAfterNewRange = getFirstEndingAfterNewRange();

        if (firstToDelete != ranges.end() && firstToDelete != firstEndingAfterNewRange)
            ops.push_back (Ops::Erased { { getIndex (firstToDelete),
                                           getIndex (firstEndingAfterNewRange) } });

        const auto beyondLastRemoved = ranges.erase (firstToDelete, firstEndingAfterNewRange);

        const auto insertIt = ranges.insert (beyondLastRemoved, newRange);
        ops.push_back (Ops::Inserted { getIndex (insertIt) });

        if (remainderRangeBecauseOfSplit.has_value())
        {
            const auto it = ranges.insert (insertIt + 1, *remainderRangeBecauseOfSplit);
            ops.push_back (Ops::Reinserted { getIndex (it) });
        }

        return ops;
    }

    size_t getIndex (std::vector<Range<int64>>::const_iterator it) const
    {
        return (size_t) std::distance (ranges.cbegin(), it);
    }

    Operations insert (Range<int64> newRange)
    {
        if (newRange.isEmpty())
            return {};

        Operations ops;

        auto it = std::lower_bound (ranges.begin(),
                                    ranges.end(),
                                    newRange,
                                    [] (auto& elem, auto& value)
                                    { return elem.getEnd() <= value.getStart(); });

        if (it != ranges.end() && it->getStart() < newRange.getStart())
        {
            const auto oldEnd = it->getEnd();
            it->setEnd (newRange.getStart());
            ops.push_back (Ops::Changed { getIndex (it) });

            Range<int64> newItems[] = { newRange,
                                        { newRange.getEnd(), newRange.getEnd() + oldEnd - it->getEnd() } };

            it = ranges.insert (it + 1, std::begin (newItems), std::end (newItems));

            ops.push_back (Ops::Inserted { getIndex (it) });
            ops.push_back (Ops::Inserted { getIndex (it + 1) });

            ++it;
        }
        else
        {
            it = ranges.insert (it, newRange);

            ops.push_back (Ops::Inserted { getIndex (it) });
        }

        for (auto& range : makeRange (std::next (it), ranges.end()))
             range += newRange.getLength();

        return ops;
    }

    Operations split (int64 i)
    {
        Operations ops;

        const auto elemIndex = getIndexForEnclosingRange (i);

        if (! elemIndex.has_value())
            return ops;

        auto& elem = ranges[*elemIndex];

        if (elem.getStart() == i)
            return ops;

        const auto oldLength = elem.getLength();
        elem.setEnd (i);
        ops.push_back (Ops::Changed { *elemIndex });

        auto setOps = set (Range<int64> { Range<int64> { i, i + oldLength - elem.getLength() } });
        ops.insert (ops.end(), setOps.begin(), setOps.end());

        return ops;
    }

    Operations eraseFrom (int64 i)
    {
        Operations ops;

        const auto elemIndex = getIndexForEnclosingRange (i);

        if (elemIndex.has_value())
        {
            ranges[*elemIndex].setEnd (i);
            ops.push_back (Ops::Changed { *elemIndex });
        }

        const auto firstToDelete = std::lower_bound (ranges.begin(),
                                                     ranges.end(),
                                                     i,
                                                     [] (auto& elem, auto& value)
                                                     { return elem.getStart() < value; });

        if (firstToDelete != ranges.end())
            ops.push_back (Ops::Erased { { getIndex (firstToDelete), getIndex (ranges.end()) } });

        ranges.erase (firstToDelete, ranges.end());

        return ops;
    }

    Operations merge (Range<size_t> elements)
    {
        jassert (elements.getEnd() <= ranges.size());

        Operations ops;

        for (auto i = elements.getStart(), j = i + 1; j < elements.getEnd(); ++j)
        {
            const auto inLastIteration = j == elements.getEnd() - 1;

            if (inLastIteration || ranges[j].getEnd() != ranges[j + 1].getStart())
            {
                ranges[i].setEnd (ranges[j].getEnd());
                ranges.erase (ranges.begin() + (int) i + 1, ranges.begin() + (int) j + 1);

                // I like that the merging algorithm works regardless of i being equal to j, so
                // I didn't add this if earlier. No need to handle a corner case where there is
                // none. However, I don't want to omit events if nothing changed.
                if (i != j)
                {
                    ops.push_back (Ops::Changed { i });
                    ops.push_back (Ops::Erased { { i + 1, j + 1 } });
                }

                const auto numItemsDeleted = j - i;
                elements.setEnd (elements.getEnd() - numItemsDeleted);
                i = j + 1 - numItemsDeleted;
                j = i;
            }
        }

        return ops;
    }

    std::optional<size_t> getIndexForEnclosingRange (int64 positionInTextRange) const
    {
        auto it = std::lower_bound (ranges.begin(),
                                    ranges.end(),
                                    positionInTextRange,
                                    [] (auto& elem, auto& value) { return elem.getEnd() <= value; });

        if (it != ranges.end() && it->getStart() <= positionInTextRange)
            return getIndex (it);

        return std::nullopt;
    }

    Range<int64> get (size_t rangeIndex) const
    {
        return ranges[rangeIndex];
    }

    size_t size() const
    {
        return ranges.size();
    }

    bool isEmpty() const
    {
        return ranges.empty();
    }

    auto& operator[] (size_t rangeIndex)
    {
        return ranges[rangeIndex];
    }

    auto& operator[] (size_t rangeIndex) const
    {
        return ranges[rangeIndex];
    }

    [[nodiscard]] Range<size_t> getAffectedElements (const Ranges::Operations& ops,
                                                    IncludeAdjacentRanges includeAdjacent = IncludeAdjacentRanges::yes) const
    {
        if (ops.empty())
            return {};

        int64 start = std::numeric_limits<int64>::max();
        int64 end = std::numeric_limits<int64>::min();

        const auto startIsValid = [&start] { return start != std::numeric_limits<int64>::max(); };

        const int64 includeAdjacentOffset = includeAdjacent == IncludeAdjacentRanges::yes ? 1 : 0;

        const auto adjacentOffsetFor = [includeAdjacent, this] (size_t index, int64 offset) -> int64
        {
            if (includeAdjacent == IncludeAdjacentRanges::no)
                return 0;

            const auto adjacentRangeIndex = (int64) index + offset;

            if (! isPositiveAndBelow (adjacentRangeIndex, (int64) ranges.size()))
                return 0;

            if (offset < 0)
                return ranges[(size_t) adjacentRangeIndex].getEnd() == ranges[index].getStart() ? offset : 0;

            return ranges[index].getEnd() == ranges[(size_t) adjacentRangeIndex].getStart() ? offset : 0;
        };

        for (auto& op : ops)
        {
            if (auto* inserted = std::get_if<Ranges::Ops::Inserted> (&op))
            {
                if (startIsValid() && (int64) inserted->index < start)
                    start += 1;

                if ((int64) inserted->index < end)
                    end += 1;

                start = std::min (start, (int64) inserted->index + adjacentOffsetFor (inserted->index, -1));
                end = std::max (end, (int64) inserted->index + adjacentOffsetFor (inserted->index, 1) + 1);
            }
            else if (auto* reinserted = std::get_if<Ranges::Ops::Reinserted> (&op))
            {
                if (startIsValid() && (int64) reinserted->index < start)
                    start += 1;

                if ((int64) reinserted->index < end)
                    end += 1;

                start = std::min (start, (int64) reinserted->index + adjacentOffsetFor (reinserted->index, -1));
                end = std::max (end, (int64) reinserted->index + adjacentOffsetFor (reinserted->index, 1) + 1);
            }
            else if (auto* erased = std::get_if<Ranges::Ops::Erased> (&op))
            {
                const auto eraseStart = (int64) erased->range.getStart();

                if (startIsValid() && eraseStart < start)
                    start -= (int64) erased->range.getLength();

                if (eraseStart < end - 1)
                    end -= (int64) erased->range.getLength();
            }
            else if (auto* changed = std::get_if<Ranges::Ops::Changed> (&op))
            {
                start = std::min (start, (int64) changed->index - includeAdjacentOffset);
                end = std::max (end, (int64) changed->index + includeAdjacentOffset);
            }
        }

        return { clampCast (start), std::min (clampCast (end), ranges.size()) };
    }

    [[nodiscard]] Range<int64> getSpannedRange (Range<size_t> r) const
    {
        auto start = (int64) r.getStart();
        auto end = (int64) r.getEnd();

        jassert (start < (int64) ranges.size() && end <= (int64) ranges.size());

        return { ranges[(size_t) start].getStart(),
                 ranges[(size_t) std::max (start, end - 1)].getEnd() };
    }

    auto begin() const
    {
        return ranges.cbegin();
    }

    auto cbegin() const
    {
        return ranges.cbegin();
    }

    auto end() const
    {
        return ranges.cend();
    }

    auto cend() const
    {
        return ranges.cend();
    }

private:
    std::vector<Range<int64>> ranges;
};

//==============================================================================
enum class MergeEqualItems
{
    no,
    yes
};

template <typename T, typename = void>
constexpr auto hasEqualityOperator = false;

template <typename T>
constexpr auto hasEqualityOperator<T, std::void_t<decltype (std::declval<T>() == std::declval<T>())>> = true;

/*  This is to get rid of the warning where advance isn't of type difference_type.
 */
template <typename Iterator, typename Value>
auto iteratorWithAdvance (Iterator&& it, Value advance)
{
    auto outIt = std::move (it);
    std::advance (outIt, static_cast<typename std::iterator_traits<Iterator>::difference_type> (advance));
    return outIt;
}

/*  Data structure for storing values associated with non-overlapping ranges.

    Has set() and insert() operations with the optional merging of ranges that contain equal values.
    These operations emit a sequence of simpler operations, that are more easily executable on an
    external container to bring it in sync with the ranges and values stored in this class.

    These operations are communicating things such as
    - "the range associated with element 5 of your container have changed"
    - "a new element was inserted at index 5, that refers to new data"

    The second example operation also implies, that all elements beyond the new index 5 element
    still refer to the same data, since when the data associated with a given element changes, a
    change operation is emitted.
*/
template <typename T>
class RangedValues
{
    template <typename Self>
    static auto getItemWithEnclosingRangeImpl (Self& self, int64 i)
    {
        const auto j = self.ranges.getIndexForEnclosingRange (i);
        return j ? std::make_optional (self.getItem (*j)) : std::nullopt;
    }

public:
    static constexpr bool canMergeEqualItems = hasEqualityOperator<T>;

    template <typename RangedValuesType>
    class RangedValuesIterator
    {
    private:
        using InternalIterator = decltype (std::declval<RangedValuesType>().ranges.cbegin());

    public:
        using value_type = std::pair<Range<int64>, T>;
        using difference_type = typename std::iterator_traits<typename std::vector<T>::iterator>::difference_type;
        using reference = decltype (std::declval<RangedValuesType>().getItem (0));

        struct PointerProxy
        {
            PointerProxy (reference r) : ref { r } {}

            auto operator->() const { return &ref; }

            reference ref;
        };

        using pointer = PointerProxy;
        using iterator_category = std::random_access_iterator_tag;

        RangedValuesIterator (RangedValuesType* ownerIn, InternalIterator iteratorIn)
            : owner { ownerIn },
              iterator { iteratorIn }
        {}

        RangedValuesIterator& operator+= (difference_type distance)
        {
            iterator += distance;
            return *this;
        }

        friend RangedValuesIterator operator+ (RangedValuesIterator i, difference_type d) { return i += d; }
        friend RangedValuesIterator operator+ (difference_type d, RangedValuesIterator i) { return i += d; }

        RangedValuesIterator& operator-= (difference_type distance)
        {
            iterator -= distance;
            return *this;
        }

        friend RangedValuesIterator operator- (RangedValuesIterator i, difference_type d) { return i -= d; }

        reference makeReference (const InternalIterator& it) const
        {
            const auto valueIt = iteratorWithAdvance (owner->values.begin(), std::distance (owner->ranges.cbegin(), it));

            return { *it, *valueIt };
        }

        reference operator[] (difference_type d) const
        {
            auto it = iterator[d];
            return makeReference (it);
        }

        friend difference_type operator- (RangedValuesIterator a, RangedValuesIterator b)   { return a.iterator - b.iterator; }

        friend bool operator<  (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator <  b.iterator; }
        friend bool operator<= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator <= b.iterator; }
        friend bool operator>  (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator >  b.iterator; }
        friend bool operator>= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator >= b.iterator; }
        friend bool operator== (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator == b.iterator; }
        friend bool operator!= (RangedValuesIterator a, RangedValuesIterator b) { return a.iterator != b.iterator; }

        RangedValuesIterator& operator++()           { ++iterator; return *this; }
        RangedValuesIterator& operator--()           { --iterator; return *this; }
        RangedValuesIterator  operator++ (int) const { RangedValuesIterator copy (*this); ++(*this); return copy; }
        RangedValuesIterator  operator-- (int) const { RangedValuesIterator copy (*this); --(*this); return copy; }

        reference operator* () const { return makeReference (iterator); }
        pointer   operator->() const { return PointerProxy { makeReference (iterator) }; }

    private:
        RangedValuesType* owner{};
        InternalIterator iterator;
    };

    template <typename X, typename Y>
    static auto makeIterator (X* x, Y y) { return RangedValuesIterator<X> (x, y); }

    auto begin()
    {
        return makeIterator (this, ranges.cbegin());
    }

    auto begin() const
    {
        return makeIterator (this, ranges.cbegin());
    }

    auto cbegin() const
    {
        return makeIterator (this, ranges.cbegin());
    }

    auto end()
    {
        return makeIterator (this, ranges.cend());
    }

    auto end() const
    {
        return makeIterator (this, ranges.cend());
    }

    auto cend() const
    {
        return makeIterator (this, ranges.cend());
    }

    struct Item
    {
        Range<int64> range;
        T& value;
    };

    struct ConstItem
    {
        Range<int64> range;
        const T& value;
    };

    void clear()
    {
        ranges.clear();
        values.clear();
    }

    template <MergeEqualItems mergeEquals = MergeEqualItems::yes>
    auto set (Range<int64> r, T v)
    {
        static_assert (mergeEquals == MergeEqualItems::no || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        auto ops = ranges.set (r);

        // We use the copy constructor to avoid any dependency or restriction on the types default
        // constructor.
        T changedValue = v;

       #if JUCE_DEBUG
        int numInsert{};
       #endif

        for (auto& op : ops)
        {
            if (auto* changed = std::get_if<Ranges::Ops::Changed> (&op))
            {
                if (changed->index < values.size())
                    changedValue = values[changed->index];
            }
            else if (auto* inserted = std::get_if<Ranges::Ops::Inserted> (&op))
            {
               #if JUCE_DEBUG
                ++numInsert;
               #endif

                values.insert (iteratorWithAdvance (values.begin(), inserted->index), v);
            }
            else if (auto* reinserted = std::get_if<Ranges::Ops::Reinserted> (&op))
            {
                values.insert (iteratorWithAdvance (values.begin(), reinserted->index), changedValue);
            }
            else if (auto* erased = std::get_if<Ranges::Ops::Erased> (&op))
            {
                values.erase (iteratorWithAdvance (values.begin(), erased->range.getStart()),
                              iteratorWithAdvance (values.begin(), erased->range.getEnd()));
            }
        }

       #if JUCE_DEBUG
        jassert (numInsert <= 1);
       #endif

        if constexpr (mergeEquals == MergeEqualItems::yes)
        {
            const auto mergeOps = mergeEqualItems (ranges.getAffectedElements (ops));
            ops.insert (ops.begin(), mergeOps.begin(), mergeOps.end());
        }

        return ops;
    }

    /** Create a RangedValues object from non-overlapping ranges. */
    template<MergeEqualItems mergeEquals, typename Iterable>
    auto setForEach (Iterable begin, Iterable end)
    {
        Ranges::Operations ops;

        for (auto it = begin; it != end; ++it)
        {
            const auto& [range, value] = *it;
            const auto subOps = set<mergeEquals> (range, value);
            ops.insert (ops.end(), subOps.begin(), subOps.end());
        }

        return ops;
    }

    template <MergeEqualItems mergeEquals = MergeEqualItems::yes>
    auto insert (Range<int64> r, T v)
    {
        static_assert (mergeEquals == MergeEqualItems::no || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        auto ops = ranges.insert (r);

        std::optional<T> oldValue;
        auto newValueInserted = false;

        for (auto& op : ops)
        {
            if (auto* changed = std::get_if<Ranges::Ops::Changed> (&op))
            {
                oldValue = values[changed->index];
            }
            else if (auto* inserted = std::get_if<Ranges::Ops::Inserted> (&op))
            {
                if (! std::exchange (newValueInserted, true))
                {
                    values.insert (values.begin() + (int) inserted->index, v);
                }
                else
                {
                    jassert (oldValue.has_value());
                    values.insert (values.begin() + (int) inserted->index, *oldValue);
                    oldValue.reset();
                }
            }
            else
                jassertfalse;
        }

        if constexpr (mergeEquals == MergeEqualItems::yes)
        {
            const auto mergeOps = mergeEqualItems (ranges.getAffectedElements (ops));
            ops.insert (ops.begin(), mergeOps.begin(), mergeOps.end());
        }

        return ops;
    }

    auto eraseFrom (int64 i)
    {
        auto ops = ranges.eraseFrom (i);

        for (auto& op : ops)
        {
            if (auto* erased = std::get_if<Ranges::Ops::Erased> (&op))
            {
                values.erase (iteratorWithAdvance (values.begin(), erased->range.getStart()),
                              iteratorWithAdvance (values.begin(), erased->range.getEnd()));
            }
        }
    }

    Range<size_t> getEqualElements (Range<size_t> rangesToCheck)
    {
        if constexpr (canMergeEqualItems)
        {
            std::optional<size_t> start;
            size_t end{};

            for (auto i = rangesToCheck.getStart(); i < rangesToCheck.getEnd() - 1; ++i)
            {
                if (exactlyEqual (values[i], values[i + 1]))
                {
                    if (! start.has_value())
                        start = i;

                    end = i + 2;
                }
                else
                {
                    if (start.has_value())
                        break;
                }
            }

            return start.has_value() ? Range { *start, end }
                                     : Range { rangesToCheck.getStart(),
                                               rangesToCheck.getStart() };
        }
        else
        {
            return {};
        }
    }

    auto mergeEqualItems (Range<size_t> elements)
    {
        if (elements.isEmpty())
            return Ranges::Operations{};

        auto ops = ranges.merge (getEqualElements (elements));

        for (const auto& op : ops)
        {
            if (auto* erased = std::get_if<Ranges::Ops::Erased> (&op))
            {
                values.erase (values.begin() + (int) erased->range.getStart(),
                              values.begin() + (int) erased->range.getEnd());
            }
        }

        return ops;
    }

    auto getItemWithEnclosingRange (int64 i)
    {
        return getItemWithEnclosingRangeImpl (*this, i);
    }

    auto getItemWithEnclosingRange (int64 i) const
    {
        return getItemWithEnclosingRangeImpl (*this, i);
    }

    Item getItem (size_t i)
    {
        jassert (i < values.size());

        return { ranges.get (i), values[i] };
    }

    ConstItem getItem (size_t i) const
    {
        jassert (i < values.size());

        return { ranges.get (i), values[i] };
    }

    Item front()
    {
        jassert (! ranges.isEmpty());
        return getItem (0);
    }

    ConstItem front() const
    {
        jassert (! ranges.isEmpty());
        return getItem (0);
    }

    Item back()
    {
        jassert (! ranges.isEmpty());
        return getItem (values.size() - 1);
    }

    ConstItem back() const
    {
        jassert (! ranges.isEmpty());
        return getItem (values.size() - 1);
    }

    /*  Returns the stored values together with the overlapping range, that overlap with the
        provided range.
    */
    std::vector<ConstItem> getIntersectionsWith (Range<int64> r) const
    {
        const auto intersections = ranges.getIntersectionsWith (r);

        std::vector<ConstItem> result;
        result.reserve (intersections.size());

        for (const auto& is : intersections)
        {
            auto valueIndex = ranges.getIndexForEnclosingRange (is.getStart());
            jassert (valueIndex.has_value());
            result.push_back ({ is, values[*valueIndex] });
        }

        return result;
    }

    const auto& getRanges() const { return ranges; }

    size_t size() const
    {
        return ranges.size();
    }

    bool isEmpty() const
    {
        return ranges.isEmpty();
    }

private:
    Ranges ranges;
    std::vector<T> values;
};

struct RangedIterator
{
    RangedIterator() = default;
    RangedIterator (const RangedIterator&) = default;
    RangedIterator (RangedIterator&&) noexcept = default;
    RangedIterator& operator= (const RangedIterator&) = default;
    RangedIterator& operator= (RangedIterator&&) noexcept = default;

    virtual ~RangedIterator() = default;
    virtual Range<int64> getRange() const = 0;
    virtual bool isValid() const = 0;
    virtual void operator++() = 0;
};

template <typename T>
class RangedIteratorWrapper final : public RangedIterator
{
public:
    /*  We pass a pointer rather than a reference here to make it clearer that the pointed-to object
        must outlive the RangedIteratorWrapper, otherwise the wrapped iterators will dangle.
    */
    explicit RangedIteratorWrapper (const RangedValues<T>* rv)
        : iterator { rv->cbegin() },
          end { rv->cend() }
    {}

    //==============================================================================
    Range<int64> getRange() const override { return iterator->range; }
    bool isValid() const override { return iterator != end; }
    void operator++() override { ++iterator; }

    //==============================================================================
    const T& getValue() const { return iterator->value; }

private:
    decltype (std::declval<const RangedValues<T>&>().cbegin()) iterator, end;
};

template <typename... Values>
class IntersectingRangedValues;

/*  A wrapper type encapsulating multiple RangedValues objects and providing iterator support.

    The iterator will advance through Ranges that are intersections with homogeneous values in each
    respective RangedValues object.

    @code
    RangedValues<char> characters;
    characters.insert ({ -2, 12 }, 'a');
    characters.insert ({ 12, 44 }, 'b');
    characters.insert ({ 63, 81 }, 'c');

    RangedValues<int> numbers;
    numbers.insert ({ -1, 0 }, 99);
    numbers.insert ({ 9, 12 }, 823);
    numbers.insert ({ 14, 16 }, 112);

    for (auto [range, character, number] : makeIntersectingRangedValues (characters, numbers))
        std::cout << toString (range) << ", " << character << ", " << number << std::endl;

    //  Prints:
    //  [-1, 0), a, 99
    //  [9, 12), a, 823
    //  [14, 16), b, 112
    @endcode
*/
template <typename... Values>
class IntersectingRangedValues<RangedValues<Values>...>
{
private:
    static_assert (sizeof...(Values) > 0, "IntersectingRangedValues() must wrap at least one RangedValues object");

    static auto createIteratorWrappers (const RangedValues<Values>*... containers)
    {
        return std::make_tuple (RangedIteratorWrapper { containers }...);
    }

public:
    /*  This constructor takes a pointer rather than a reference to make it clearer that the pointed-to
        objects must outlive the IntersectingRangedValues instance. Passing a pointer also makes
        it harder to accidentally reference a temporary when constructing IntersectingRangedValues.
    */
    explicit IntersectingRangedValues (const RangedValues<Values>*... t)
        : items { t... }
    {
    }

    struct IntersectionIteratorSentinel {};

    struct IntersectionIterator
    {
        using reference  = std::tuple<Range<int64>, const Values&...>;
        using iterator_category = std::forward_iterator_tag;

        using IteratorWrappersType = decltype (createIteratorWrappers (std::declval<const RangedValues<Values>*>()...));

        explicit IntersectionIterator (IteratorWrappersType&& wrappers)
            : iteratorWrappers { std::move (wrappers) }
        {
            std::apply ([this] (auto&&... args)
                        {
                            iterators = std::list<RangedIterator*> { static_cast<RangedIterator*> (&args)... };
                        },
                        iteratorWrappers);

            if (! isValid())
                return;

            maxStart = std::accumulate (iterators.cbegin(),
                                        iterators.cend(),
                                        std::numeric_limits<int64>::min(),
                                        [] (auto acc, auto& item) { return std::max (acc, item->getRange().getStart()); });

            minEnd = std::accumulate (iterators.cbegin(),
                                      iterators.cend(),
                                      std::numeric_limits<int64>::max(),
                                      [] (auto acc, auto& item) { return std::min (acc, item->getRange().getEnd()); });

            iterators.sort ([] (auto& a, auto& b)
                            { return a->getRange().getEnd() < b->getRange().getEnd(); });

            if (Range<int64> { maxStart, minEnd }.isEmpty())
                advance();
        }

        friend bool operator!= (IntersectionIterator a, IntersectionIteratorSentinel) { return a.isValid(); }

        IntersectionIterator& operator++()           { advance(); return *this; }
        IntersectionIterator  operator++ (int) const { IntersectionIterator copy (*this); ++(*this); return copy; }

        reference operator* () const
        {
            return std::tuple_cat (std::make_tuple (Range { maxStart, minEnd }), makeReference());
        }

    private:
        auto makeReference() const
        {
            return std::apply ([] (auto&... args) { return std::tie (args.getValue()...); }, iteratorWrappers);
        }

        void advance()
        {
            do
            {
                minEnd = std::numeric_limits<int64>::max();

                for (auto it = iterators.begin(); it != iterators.end(); ++it)
                {
                    auto& elem = *(*it);

                    if (it == iterators.begin() || elem.getRange().getEnd() <= maxStart)
                    {
                        ++elem;

                        if (! elem.isValid())
                            return;

                        maxStart = std::max (maxStart, elem.getRange().getStart());
                    }

                    minEnd = std::min (minEnd, elem.getRange().getEnd());
                }

                iterators.sort ([] (auto& a, auto& b)
                                { return a->getRange().getEnd() < b->getRange().getEnd(); });
            }
            while (Range<int64> { maxStart, minEnd }.isEmpty());
        }

        bool isValid() const
        {
            return std::all_of (iterators.cbegin(), iterators.cend(), [] (auto& elem) { return elem->isValid(); });
        }

        IteratorWrappersType iteratorWrappers;
        std::list<RangedIterator*> iterators;

        int64 maxStart, minEnd;
    };

    auto begin() const
    {
        auto wrappers = std::apply ([](auto&&... args)
                                    { return createIteratorWrappers (std::forward<decltype (args)> (args)...); },
                                    items);

        return IntersectionIterator { std::move (wrappers) };
    }

    auto end() const
    {
        return IntersectionIteratorSentinel{};
    }

private:
    std::tuple<const RangedValues<Values>*...> items;
};

/*  See IntersectingRangedValues.
*/
template <typename... Values>
[[nodiscard]] auto makeIntersectingRangedValues (const RangedValues<Values>*... rvs)
{
    static_assert (sizeof...(Values) > 0, "makeIntersectingRangedValues() requires at least one parameter");

    return IntersectingRangedValues<RangedValues<Values>...> { rvs... };
}

} // namespace juce::detail
