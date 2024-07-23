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

namespace detail
{

template <typename T, typename = void>
constexpr auto canPreDecrement = false;

template <typename T>
constexpr auto canPreDecrement<T, std::void_t<decltype (--std::declval<T>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canAddAssign = false;

template <typename T, typename I>
constexpr auto canAddAssign<T, I, std::void_t<decltype (std::declval<T>() += std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canSubAssign = false;

template <typename T, typename I>
constexpr auto canSubAssign<T, I, std::void_t<decltype (std::declval<T>() -= std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canAdd = false;

template <typename T, typename I>
constexpr auto canAdd<T, I, std::void_t<decltype (std::declval<T>() + std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canSub = false;

template <typename T, typename I>
constexpr auto canSub<T, I, std::void_t<decltype (std::declval<T>() - std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canLessThan = false;

template <typename T, typename I>
constexpr auto canLessThan<T, I, std::void_t<decltype (std::declval<T>() < std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canLessThanEqual = false;

template <typename T, typename I>
constexpr auto canLessThanEqual<T, I, std::void_t<decltype (std::declval<T>() <= std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canGreaterThan = false;

template <typename T, typename I>
constexpr auto canGreaterThan<T, I, std::void_t<decltype (std::declval<T>() > std::declval<I>())>> = true;

template <typename T, typename I, typename = void>
constexpr auto canGreaterThanEqual = false;

template <typename T, typename I>
constexpr auto canGreaterThanEqual<T, I, std::void_t<decltype (std::declval<T>() >= std::declval<I>())>> = true;

namespace withAdlSize
{
    using std::size;

    template <typename Range>
    using AdlSize = decltype (size (std::declval<Range>()));

    template <typename Range>
    using AdlSignedSize = std::common_type_t<std::ptrdiff_t, std::make_signed_t<AdlSize<Range>>>;
}

} // namespace detail

/**
    Returned when dereferencing an EnumerateIterator.

    Allows querying the index associated with an element, along with a reference to the element
    itself.

    You should never need to construct an instance of this type yourself. Instead, use the
    enumerate() function to construct a range that can be enumerated.

    @see enumerate()
    @tags{Core}
*/
template <typename Index, typename Value>
struct Enumerated
{
    Index index;
    Value value;
};

/**
    An iterator that wraps some other iterator, keeping track of the relative position of that
    iterator based on calls to arithmetic operators such as
    operator++(), operator--(), operator+(), and operator-().

    You should never need to construct an instance of this type yourself. Instead, use the
    enumerate() function to construct a range that can be enumerated.

    @see enumerate()
    @tags{Core}
*/
template <typename Iter, typename Index = ptrdiff_t>
class EnumerateIterator
{
public:
    /** Default constructor. */
    constexpr EnumerateIterator() = default;

    /** Wraps the provided iterator, and sets the internal count to 0. */
    constexpr explicit EnumerateIterator (Iter iter)
        : EnumerateIterator (std::move (iter), Index{}) {}

    /** Wraps the provided iterator, and sets the internal count to the provided value. */
    constexpr EnumerateIterator (Iter iter, Index ind)
        : iterator (std::move (iter)), index (ind) {}

    /** Two EnumerateIterators are considered equal if the wrapped iterators are equal. */
    template <typename OtherIter, typename OtherInd>
    [[nodiscard]] constexpr bool operator== (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator == other.iterator;
    }

    /** @see operator==() */
    template <typename OtherIter, typename OtherInd>
    [[nodiscard]] constexpr bool operator!= (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return ! operator== (other);
    }

    /** Dereferencing the iterator produces an Enumerated instance *by value*. This type holds
        a copy of the iterator's current index, along with the result of dereferencing the
        wrapped iterator (normally a reference type).
    */
    [[nodiscard]] constexpr Enumerated<Index, decltype (*std::declval<Iter>())> operator*() const
    {
        return { index, *iterator };
    }

    /** Increments the iterator and the index. */
    constexpr EnumerateIterator& operator++()
    {
        ++iterator;
        ++index;
        return *this;
    }

    /** Increments the iterator and the index. */
    constexpr EnumerateIterator operator++ (int)
    {
        auto copy = *this;
        operator++();
        return copy;
    }

    /** Decrements the iterator and the index.
        Only participates in overload resolution if the iterator can be pre-decremented.
    */
    template <typename T = Iter, std::enable_if_t<detail::canPreDecrement<T>, int> = 0>
    constexpr EnumerateIterator& operator--()
    {
        --iterator;
        --index;
        return *this;
    }

    /** Decrements the iterator and the index.
        Only participates in overload resolution if the iterator can be pre-decremented.
    */
    template <typename T = Iter, std::enable_if_t<detail::canPreDecrement<T>, int> = 0>
    constexpr EnumerateIterator operator-- (int)
    {
        auto copy = *this;
        operator--();
        return copy;
    }

    /** Adds an integral value to both the iterator and the index.
        Only participates in overload resolution if the iterator can be add-assigned.
    */
    template <typename I, std::enable_if_t<detail::canAddAssign<Iter&, I>, int> = 0>
    constexpr EnumerateIterator& operator+= (I diff)
    {
        iterator += diff;
        index += static_cast<Index> (diff);
        return *this;
    }

    /** Subtracts an integral value from both the iterator and the index.
        Only participates in overload resolution if the iterator can be sub-assigned.
    */
    template <typename I, std::enable_if_t<detail::canSubAssign<Iter&, I>, int> = 0>
    constexpr EnumerateIterator& operator-= (I diff)
    {
        iterator -= diff;
        index -= static_cast<Index> (diff);
        return *this;
    }

    /** Subtracts another enumerate iterator from this one, producing the same result as
        subtracting the two wrapped iterators. For random-access iterators, this will normally
        return the distance between the two iterators.
        Only participates in overload resolution if the wrapped iterators can be subtracted.
    */
    template <typename OtherIter, typename OtherInd, std::enable_if_t<detail::canSub<Iter, OtherIter>, int> = 0>
    [[nodiscard]] constexpr auto operator- (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator - other.iterator;
    }

    /** Indexes into this iterator, equivalent to adding an integral value to this iterator and
        then dereferencing the result.
        Only participates in overload resolution if the wrapped iterator allows addition of
        integral values.
    */
    template <typename I, std::enable_if_t<detail::canAdd<EnumerateIterator, I>, int> = 0>
    [[nodiscard]] constexpr auto operator[] (I diff) const
    {
        return *(*this + diff);
    }

    /** Returns the result of comparing the two wrapped iterators.
        Only participates in overload resolution if the wrapped iterators are comparable.
    */
    template <typename OtherIter, typename OtherInd, std::enable_if_t<detail::canLessThan<Iter, OtherIter>, int> = 0>
    [[nodiscard]] constexpr bool operator< (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator < other.iterator;
    }

    /** Returns the result of comparing the two wrapped iterators.
        Only participates in overload resolution if the wrapped iterators are comparable.
    */
    template <typename OtherIter, typename OtherInd, std::enable_if_t<detail::canLessThanEqual<Iter, OtherIter>, int> = 0>
    [[nodiscard]] constexpr bool operator<= (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator <= other.iterator;
    }

    /** Returns the result of comparing the two wrapped iterators.
        Only participates in overload resolution if the wrapped iterators are comparable.
    */
    template <typename OtherIter, typename OtherInd, std::enable_if_t<detail::canGreaterThan<Iter, OtherIter>, int> = 0>
    [[nodiscard]] constexpr bool operator> (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator > other.iterator;
    }

    /** Returns the result of comparing the two wrapped iterators.
        Only participates in overload resolution if the wrapped iterators are comparable.
    */
    template <typename OtherIter, typename OtherInd, std::enable_if_t<detail::canGreaterThanEqual<Iter, OtherIter>, int> = 0>
    [[nodiscard]] constexpr bool operator>= (const EnumerateIterator<OtherIter, OtherInd>& other) const
    {
        return iterator >= other.iterator;
    }

    /** Returns the result of adding an integral value to this iterator.
        Only participates in overload resolution if addition is supported by the wrapped iterator.
    */
    template <typename I, std::enable_if_t<detail::canAddAssign<EnumerateIterator&, I>, int> = 0>
    constexpr friend auto operator+ (EnumerateIterator iter, I ind)
    {
        return iter += ind;
    }

    /** Returns the result of adding an integral value to this iterator.
        Only participates in overload resolution if addition is supported by the wrapped iterator.
    */
    template <typename I, std::enable_if_t<detail::canAddAssign<EnumerateIterator&, I>, int> = 0>
    constexpr friend auto operator+ (I ind, EnumerateIterator iter)
    {
        return iter += ind;
    }

    /** Returns the result of subtracting an integral value from this iterator.
        Only participates in overload resolution if subtraction is supported by the wrapped iterator.
    */
    template <typename I, std::enable_if_t<detail::canSubAssign<EnumerateIterator&, I>, int> = 0>
    constexpr friend auto operator- (EnumerateIterator iter, I ind)
    {
        return iter -= ind;
    }

private:
    Iter iterator{};
    Index index = 0;
};

//==============================================================================
/**
    Wraps a pair of iterators, providing member begin() and end() functions that return
    those iterators.
    This is useful in situations where you have an iterator pair, but want to use that
    pair somewhere that requires an iterable range, such as in a ranged-for loop.

    @see makeRange()
    @tags{Core}
*/
template <typename Begin, typename End>
class IteratorPair
{
public:
    /** Constructs a pair from a begin and end iterator.
        Instead of calling this directly, use makeRange().
    */
    constexpr IteratorPair (Begin bIn, End eIn)
        : b (std::move (bIn)), e (std::move (eIn)) {}

    /** Returns the begin iterator. */
    constexpr auto begin() const { return b; }

    /** Returns the end iterator. */
    constexpr auto end()   const { return e; }

private:
    Begin b;
    End e;
};

/**
    Given two iterators "begin" and "end", returns an IteratorPair with a member
    begin() and end() function. This pair can be used in contexts that expect an
    iterable range, the most significant of which is ranged-for loops.
    This automatically deduces the Begin and End types, so it is more concise to use than
    directly calling the IteratorPair constructor.
*/
template <typename Begin, typename End = Begin>
[[nodiscard]] constexpr auto makeRange (Begin begin, End end)
{
    return IteratorPair<Begin, End> { std::move (begin), std::move (end) };
}

//==============================================================================
/**
    Given a range and an optional starting offset, returns an IteratorPair that
    holds EnumerateIterators wrapping the begin() and end() of the range.

    This is useful in situations where you need to iterate some range, but also query
    the position of each item in the range.

    A simple usage might look like this:

    @code
    std::list<int> elements { 10, 20, 30, 40, 50 };

    for (const auto pair : enumerate (elements))
        std::cout << pair.index << ' ' << pair.value << ' ';

    // output: 0 10 1 20 2 30 3 40 4 50
    @endcode

    You can also use structured bindings to concisely destructure each Enumerated instance:

    @code
    for (const auto [index, value] : enumerate (elements))
        std::cout << index << ' ' << value << ' ';
    @endcode

    Note that the Enumerated instance is returned by value. This is because each Enumerated
    instance is created on-demand when the iterator is dereferenced. As a result, the following
    will result in a dangling reference, and will probably trigger a compiler warning:

    @code
    // BAD IDEA: creating a reference to a temporary Enumerated instance
    for (auto& [index, value] : enumerate (elements))
        ...
    @endcode

    The 'value' member of Enumerated automatically assumes the same type as dereferencing the
    wrapped iterator, which is normally a reference to an element of a container.
    In the following snippet, the type of '[index, value]' is 'const Enumerated<ptrdiff_t, int&>',
    the type of 'index' is 'ptrdiff_t', and the type of 'value' is 'int&'.

    @code
    std::vector<int> elements { 10, 20, 30, 40, 50 };
    for (const auto [index, value] : enumerate (elements))
        ...
    @endcode

    By default, the constness of pair.value will match the constness of the range passed to
    enumerate. If you pass a mutable lvalue reference to enumerate, then each value will also
    be mutable. If you pass a constant lvalue reference to enumerate, then each value will be
    const. If you know that you don't need the iterated elements to be mutable, it's good
    practice to wrap the range with std::as_const before passing it to enumerate:

    @code
    for (const auto [index, value] : enumerate (std::as_const (elements)))
    {
        // value is immutable here
    }
    @endcode
*/
template <typename Range, typename Index = detail::withAdlSize::AdlSignedSize<Range>>
[[nodiscard]] constexpr auto enumerate (Range&& range, Index startingValue = {})
{
    // This ensures argument-dependent lookup works properly for user-defined non-member begin/end
    using std::begin, std::end;
    return makeRange (EnumerateIterator { begin (range), startingValue },
                      EnumerateIterator { end   (range), startingValue });
}

} // namespace juce
