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

/*  This is to get rid of the warning where advance isn't of type difference_type.
*/
template <typename Iterator, typename Value>
auto iteratorWithAdvance (Iterator&& it, Value advance)
{
    auto outIt = std::move (it);
    std::advance (outIt, static_cast<typename std::iterator_traits<Iterator>::difference_type> (advance));
    return outIt;
}

struct Ranges final
{
    struct Ops
    {
        Ops() = delete;

        struct New
        {
            explicit New (size_t x) : index { x } {}

            size_t index;
        };

        struct Split
        {
            Split (size_t x, Range<int64> leftRangeIn, Range<int64> rightRangeIn)
                : index { x },
                  leftRange { leftRangeIn },
                  rightRange { rightRangeIn }
            {}

            size_t index;
            Range<int64> leftRange;
            Range<int64> rightRange;
        };

        struct Erase
        {
            explicit Erase (Range<size_t> x) : range { x } {}

            Range<size_t> range;
        };

        struct Change
        {
            Change (size_t x, Range<int64> oldRangeIn, Range<int64> newRangeIn)
                : index { x },
                  oldRange { oldRangeIn },
                  newRange { newRangeIn }
            {}

            size_t index;
            Range<int64> oldRange;
            Range<int64> newRange;
        };
    };

    using Op = std::variant<Ops::New, Ops::Split, Ops::Erase, Ops::Change>;

    using Operations = std::vector<Op>;

    static auto withOperationsFrom (const Operations& ops, const Operations& newOps)
    {
        auto result = ops;
        result.insert (result.end(), newOps.begin(), newOps.end());
        return result;
    }

    static auto withOperationsFrom (const Operations& ops, Op newOp)
    {
        auto result = ops;
        result.insert (result.end(), newOp);
        return result;
    }

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

    //==============================================================================
    // Basic operations
    Operations split (int64 i)
    {
        Operations ops;

        const auto elemIndex = getIndexForEnclosingRange (i);

        if (! elemIndex.has_value())
            return {};

        auto& elem = ranges[*elemIndex];

        if (elem.getStart() == i)
            return {};

        ops = withOperationsFrom (ops, Ops::Split { *elemIndex,
                                                    elem.withEnd (i),
                                                    elem.withStart (i) });

        const auto oldLength = elem.getLength();
        elem.setEnd (i);

        ranges.insert (iteratorWithAdvance (ranges.begin(), *elemIndex + 1),
                       { i, i + oldLength - elem.getLength() });

        return ops;
    }

    Operations erase (Range<int64> r)
    {
        if (r.isEmpty())
            return {};

        Operations ops;

        for (auto i : { r.getStart(), r.getEnd() })
            ops = withOperationsFrom (ops, split (i));

        const auto firstToDelete = std::lower_bound (ranges.begin(),
                                                     ranges.end(),
                                                     r.getStart(),
                                                     [] (auto& elem, auto& value)
                                                     { return elem.getStart() < value; });

        const auto beyondLastToDelete = std::lower_bound (firstToDelete,
                                                          ranges.end(),
                                                          r.getEnd(),
                                                          [] (auto& elem, auto& value)
                                                          { return elem.getStart() < value; });

        if (firstToDelete != ranges.end())
            ops = withOperationsFrom (ops, Ops::Erase { { getIndex (firstToDelete), getIndex (beyondLastToDelete) } });

        ranges.erase (firstToDelete, beyondLastToDelete);

        return ops;
    }

    Operations drop (Range<int64> r)
    {
        auto ops = erase (r);
        ops = withOperationsFrom (ops, shift (r.getEnd(), -r.getLength()));
        return ops;
    }

    /*  Shift all ranges starting at or beyond the specified from parameter, by the specified amount.
    */
    Operations shift (int64 from, int64 amount)
    {
        if (amount == 0)
            return {};

        const auto shiftStartingFrom = std::lower_bound (ranges.begin(),
                                                         ranges.end(),
                                                         from,
                                                         [] (auto& elem, auto& value)
                                                         { return elem.getStart() < value; });

        Operations ops;

        for (auto it = shiftStartingFrom; it < ranges.end(); ++it)
        {
            const auto oldRange = *it;
            *it += amount;
            ops = withOperationsFrom (ops, Ops::Change { getIndex (it), oldRange, *it });
        }

        return ops;
    }

    Operations set (Range<int64> newRange)
    {
        if (newRange.isEmpty())
            return {};

        Operations ops;

        ops = withOperationsFrom (ops, erase (newRange));

        const auto insertBefore = std::lower_bound (ranges.begin(),
                                                    ranges.end(),
                                                    newRange.getStart(),
                                                    [] (auto& elem, auto& value)
                                                    { return elem.getStart() < value; });

        ops = withOperationsFrom (ops, Ops::New { getIndex (insertBefore) });
        ranges.insert (insertBefore, newRange);

        return ops;
    }

    Operations insert (Range<int64> newRange)
    {
        if (newRange.isEmpty())
            return {};

        Operations ops;

        ops = withOperationsFrom (ops, split (newRange.getStart()));
        ops = withOperationsFrom (ops, shift (newRange.getStart(), newRange.getLength()));

        const auto insertBefore = std::lower_bound (ranges.begin(),
                                                    ranges.end(),
                                                    newRange.getStart(),
                                                    [] (auto& elem, auto& value)
                                                    { return elem.getStart() < value; });

        const auto insertBeforeIndex = getIndex (insertBefore);

        ranges.insert (insertBefore, newRange);
        ops = withOperationsFrom (ops, Ops::New { insertBeforeIndex });

        return ops;
    }

    //==============================================================================
    // Convenience functions
    void clear()
    {
        ranges.clear();
    }

    Operations eraseFrom (int64 i)
    {
        if (ranges.empty())
            return {};

        return erase ({ i, ranges.back().getEnd() });
    }

    /*  Merges neighbouring ranges backwards if they form a contiguous range.
    */
    Operations mergeBack (size_t i)
    {
        jassert (isPositiveAndBelow (i, ranges.size()));

        if (i == 0 || i >= ranges.size())
            return {};

        const auto start = i - 1;
        const auto end = i;

        if (ranges[start].getEnd() != ranges[end].getStart())
            return {};

        Operations ops;

        const auto oldRange = ranges[start];
        ranges[start].setEnd (ranges[end].getEnd());
        ops = withOperationsFrom (ops, Ops::Change { start, oldRange, ranges[start] });

        ops = withOperationsFrom (ops, Ops::Erase { { end, end + 1 } });

        ranges.erase (iteratorWithAdvance (ranges.begin(), end),
                      iteratorWithAdvance (ranges.begin(), end + 1));

        return ops;
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

    //==============================================================================
    size_t size() const
    {
        return ranges.size();
    }

    bool isEmpty() const
    {
        return ranges.empty();
    }

    Range<int64> get (size_t rangeIndex) const
    {
        return ranges[rangeIndex];
    }

    //==============================================================================
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

    auto* data() const
    {
        return ranges.data();
    }

    /* Returns an iterator for the Range element which includes the provided value. */
    auto find (int64 i) const
    {
        const auto it = std::lower_bound (cbegin(),
                                          cend(),
                                          i,
                                          [] (auto& elem, auto& value) { return elem.getEnd() <= value; });

        return it != cend() && it->getStart() <= i ? it : cend();
    }

    std::optional<size_t> getIndexForEnclosingRange (int64 positionInTextRange) const
    {
        const auto iter = find (positionInTextRange);
        return iter != ranges.end() ? std::make_optional (getIndex (iter)) : std::nullopt;
    }

    /* Returns true if this object covers each element in the provided range. For empty ranges it
       returns true if the start value is covered.
    */
    bool covers (Range<int64> range) const
    {
        for (auto curr = find (range.getStart()), prev = curr; curr != cend(); ++curr)
        {
            if (prev != curr && prev->getEnd() != curr->getStart())
                return false;

            if (range.getEnd() <= curr->getEnd())
                return true;

            prev = curr;
        }

        return false;
    }

private:
    size_t getIndex (std::vector<Range<int64>>::const_iterator it) const
    {
        return (size_t) std::distance (ranges.cbegin(), it);
    }

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

//==============================================================================
template <typename T>
struct RangedValuesIteratorItem
{
    Range<int64> range;
    T& value;
};

//==============================================================================
template <typename T>
class RangedValuesIterator
{
private:
    using InternalIterator = const Range<int64>*;

public:
    using value_type = std::pair<Range<int64>, T>;
    using difference_type = std::ptrdiff_t;
    using reference = RangedValuesIteratorItem<T>;

    struct PointerProxy
    {
        PointerProxy (reference r) : ref { r } {}

        auto operator->() const { return &ref; }

        reference ref;
    };

    using pointer = PointerProxy;
    using iterator_category = std::random_access_iterator_tag;

    RangedValuesIterator (T* valuesIn, InternalIterator iteratorBaseIn, InternalIterator iteratorIn)
        : values { valuesIn },
          iteratorBase { iteratorBaseIn },
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
    reference makeReference (const InternalIterator& it) const
    {
        const auto valueIt = values + (size_t) std::distance (iteratorBase, it);

        return { *it, *valueIt };
    }

    T* values{};
    InternalIterator iteratorBase, iterator;
};

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
    static auto makeIterator (RangedValuesType* rv, const Range<int64>* base, const Range<int64>* iterator)
    {
        return RangedValuesIterator<std::remove_pointer_t<decltype (rv->values.data())>> (rv->values.data(), base, iterator);
    }

    //==============================================================================
    auto begin()
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto begin() const
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto cbegin() const
    {
        return makeIterator (this, ranges.data(), ranges.data());
    }

    auto end()
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
    }

    auto end() const
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
    }

    auto cend() const
    {
        return makeIterator (this, ranges.data(), ranges.data() + ranges.size());
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

    //==============================================================================
    // Basic operations
    template <MergeEqualItems mergeEquals = MergeEqualItems::yes>
    auto set (Range<int64> r, T v)
    {
        static_assert (mergeEquals == MergeEqualItems::no || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        Ranges::Operations ops;

        ops = Ranges::withOperationsFrom (ops, ranges.set (r));
        applyOperations (ops, std::move (v));

        if constexpr (mergeEquals == MergeEqualItems::yes)
        {
            ops = Ranges::withOperationsFrom (ops, mergeEqualItems (r.getStart()));
            ops = Ranges::withOperationsFrom (ops, mergeEqualItems (r.getEnd()));
        }

        return ops;
    }

    template <MergeEqualItems mergeEquals = MergeEqualItems::yes>
    auto insert (Range<int64> r, T v)
    {
        static_assert (mergeEquals == MergeEqualItems::no || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        auto ops = ranges.insert (r);
        applyOperations (ops, std::move (v));

        if constexpr (mergeEquals == MergeEqualItems::yes)
        {
            ops = Ranges::withOperationsFrom (ops, mergeEqualItems (r.getStart()));
            ops = Ranges::withOperationsFrom (ops, mergeEqualItems (r.getEnd()));
        }

        return ops;
    }

    // erase will always cause a discontinuity and thus, there is no opportunity to merge
    auto erase (Range<int64> r)
    {
        auto ops = ranges.erase (r);
        applyOperations (ops);
        return ops;
    }

    // drop moves subsequent ranges downward, and can end up in these ranges forming a contiguous
    // range with the ones on the left side of the drop. Hence, it makes sense to ask if we want
    // merging behaviour.
    template <MergeEqualItems mergeEquals = MergeEqualItems::yes>
    auto drop (Range<int64> r)
    {
        static_assert (mergeEquals == MergeEqualItems::no || canMergeEqualItems,
                       "You can't use MergeEqualItems::yes if your type doesn't have operator==.");

        auto ops = ranges.drop (r);
        applyOperations (ops);

        if constexpr (mergeEquals == MergeEqualItems::yes)
            ops = Ranges::withOperationsFrom (ops, mergeEqualItems (r.getStart()));

        return ops;
    }

    //==============================================================================
    void clear()
    {
        ranges.clear();
        values.clear();
    }

    Ranges::Operations shift (int64 from, int64 amount)
    {
        return ranges.shift (from, amount);
    }

    Ranges::Operations eraseFrom (int64 i)
    {
        if (ranges.isEmpty())
            return {};

        return erase ({ i, ranges.get (ranges.size() - 1).getEnd() });
    }

    Ranges::Operations eraseUpTo (int64 i)
    {
        if (ranges.isEmpty())
            return {};

        return erase ({ ranges.get (0).getStart(), i });
    }

    /** Create a RangedValues object from non-overlapping ranges. */
    template<MergeEqualItems mergeEquals, typename Iterable>
    auto setForEach (Iterable begin, Iterable end)
    {
        Ranges::Operations ops;

        for (auto it = begin; it != end; ++it)
        {
            const auto& [range, value] = *it;
            ops = Ranges::withOperationsFrom (ops, set<mergeEquals> (range, value));
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

    // Finds the item whose range encloses the provided value
    template <typename Self>
    static auto findImpl (Self& self, int64 i)
    {
        return iteratorWithAdvance (self.begin(),
                                    std::distance (self.ranges.cbegin(), self.ranges.find (i)));
    }

    auto find (int64 i) { return findImpl (*this, i); }
    auto find (int64 i) const { return findImpl (*this, i); }

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
    RangedValues<T> getIntersectionsWith (Range<int64> r) const
    {
        const auto intersections = ranges.getIntersectionsWith (r);

        RangedValues<T> result;

        for (const auto& is : intersections)
        {
            auto valueIndex = ranges.getIndexForEnclosingRange (is.getStart());
            jassert (valueIndex.has_value());
            result.template set<MergeEqualItems::no> (is, values[*valueIndex]);
        }

        return result;
    }

    RangedValues<T> getIntersectionsStartingAtZeroWith (Range<int64> r) const
    {
        auto result = getIntersectionsWith (r);
        result.drop ({ (int64) 0, r.getStart() });
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
    Ranges::Operations mergeEqualItems (int64 i)
    {
        const auto endOpt = ranges.getIndexForEnclosingRange (i);

        if (! endOpt.has_value() || *endOpt == 0)
            return {};

        const auto end = *endOpt;
        const auto start = end - 1;

        if (! exactlyEqual (values[start], values[end]))
            return {};

        const auto ops = ranges.mergeBack (end);
        applyOperations (ops);

        return ops;
    }

    void applyOperation (const Ranges::Op& op)
    {
        if (auto* split = std::get_if<Ranges::Ops::Split> (&op))
        {
            values.insert (iteratorWithAdvance (values.begin(), split->index), values[split->index]);
            return;
        }

        if (auto* erase = std::get_if<Ranges::Ops::Erase> (&op))
        {
            values.erase (iteratorWithAdvance (values.begin(), erase->range.getStart()),
                          iteratorWithAdvance (values.begin(), erase->range.getEnd()));
            return;
        }

        // This function can't handle New.
        jassert (std::get_if<Ranges::Ops::New> (&op) == nullptr);

        // This entire type doesn't have to do anything to handle Ranges::Ops::Change.
    }

    void applyOperation (const Ranges::Op& op, T v)
    {
        if (auto* newOp = std::get_if<Ranges::Ops::New> (&op))
        {
            values.insert (iteratorWithAdvance (values.begin(), newOp->index), std::move (v));
        }
        else
        {
            applyOperation (op);
        }
    }

    void applyOperations (const Ranges::Operations& ops)
    {
        for (const auto& op : ops)
            applyOperation (op);
    }

    void applyOperations (const Ranges::Operations& ops, T v)
    {
        for (const auto& op : ops)
            applyOperation (op, v);
    }

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
    RangedIteratorWrapper (RangedValuesIterator<T> iteratorIn, RangedValuesIterator<T> endIn)
        : iterator { std::move (iteratorIn) },
          end { std::move (endIn) }
    {}

    //==============================================================================
    Range<int64> getRange() const override { return iterator->range; }
    bool isValid() const override { return iterator != end; }
    void operator++() override { ++iterator; }

    //==============================================================================
    const T& getValue() const { return iterator->value; }

private:
    RangedValuesIterator<T> iterator, end;
};

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
class IntersectingRangedValues
{
public:
    static_assert (sizeof...(Values) > 0, "IntersectingRangedValues() must wrap at least one RangedValues object");

    /*  This constructor takes a pointer rather than a reference to make it clearer that the pointed-to
        objects must outlive the IntersectingRangedValues instance. Passing a pointer also makes
        it harder to accidentally reference a temporary when constructing IntersectingRangedValues.
    */
    template <typename... Iterables>
    explicit IntersectingRangedValues (Iterables*... iterable)
        : iteratorWrappers { std::make_tuple (RangedIteratorWrapper<Values> { iterable->begin(), iterable->end() }...) }
    {
    }

    struct IntersectionIteratorSentinel {};

    using IteratorWrappersType = std::tuple<RangedIteratorWrapper<Values>...>;

    struct IntersectionIterator
    {
        using reference  = std::tuple<Range<int64>, const Values&...>;
        using iterator_category = std::forward_iterator_tag;

        explicit IntersectionIterator (IteratorWrappersType wrappers)
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
        return IntersectionIterator { iteratorWrappers };
    }

    auto end() const
    {
        return IntersectionIteratorSentinel{};
    }

private:
    IteratorWrappersType iteratorWrappers;
};

/*  See IntersectingRangedValues.
*/
template <typename... Iterables>
[[nodiscard]] auto makeIntersectingRangedValues (Iterables*... iterables)
{
    static_assert (sizeof...(Iterables) > 0, "makeIntersectingRangedValues() requires at least one parameter");

    return IntersectingRangedValues<std::remove_reference_t<decltype (std::declval<Iterables>().begin()->value)>...> { iterables... };
}

} // namespace juce::detail
