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

//==============================================================================
inline constexpr auto dynamicExtent = std::numeric_limits<size_t>::max();

namespace detail
{
    //==============================================================================
    template <typename, typename = void>
    constexpr auto hasToAddress = false;

    template <typename T>
    constexpr auto hasToAddress<T, Void<decltype (std::pointer_traits<T>::to_address (std::declval<T>()))>> = true;

    template <typename, typename = void>
    constexpr auto hasDataAndSize = false;

    template <typename T>
    constexpr auto hasDataAndSize<T,
                                  Void<decltype (std::data (std::declval<T>())),
                                       decltype (std::size (std::declval<T>()))>> = true;

    template <size_t Extent>
    struct NumBase
    {
        constexpr NumBase() = default;

        constexpr explicit NumBase (size_t) {}

        constexpr size_t size() const { return Extent; }
    };

    template <>
    struct NumBase<dynamicExtent>
    {
        constexpr NumBase() = default;

        constexpr explicit NumBase (size_t arg)
            : num (arg) {}

        constexpr size_t size() const { return num; }

        size_t num{};
    };

    template <typename T>
    constexpr T* toAddress (T* p)
    {
        return p;
    }

    template <typename It>
    constexpr auto toAddress (const It& it)
    {
        if constexpr (detail::hasToAddress<It>)
            return std::pointer_traits<It>::to_address (it);
        else
            return toAddress (it.operator->());
    }
}

//==============================================================================
/**
    A non-owning view over contiguous objects stored in an Array or vector
    or other similar container.

    This is a bit like std::span from C++20, but with a more limited interface.

    @tags{Core}
*/
template <typename Value, size_t Extent = dynamicExtent>
class Span : private detail::NumBase<Extent> // for empty-base optimisation
{
    using Base = detail::NumBase<Extent>;

public:
    static constexpr auto extent = Extent;

    template <size_t e = extent, std::enable_if_t<e == 0 || e == dynamicExtent, int> = 0>
    constexpr Span() {}

    template <typename It>
    constexpr Span (It it, size_t end)
        : Base (end), ptr (detail::toAddress (it)) {}

    template <typename Range, std::enable_if_t<detail::hasDataAndSize<Range>, int> = 0>
    constexpr Span (Range&& range)
        : Base (std::size (range)), ptr (std::data (range)) {}

    constexpr Span (const Span&) = default;

    constexpr Span& operator= (const Span&) = default;

    using Base::size;

    constexpr Value* begin() const { return ptr; }
    constexpr Value* end()   const { return ptr + size(); }

    constexpr auto& front() const { return ptr[0]; }
    constexpr auto& back()  const { return ptr[size() - 1]; }

    constexpr auto& operator[] (size_t index) const { return ptr[index]; }
    constexpr Value* data() const { return ptr; }

    constexpr bool empty() const { return size() == 0; }

private:
    Value* ptr = nullptr;
};

template <typename T, typename End>
Span (T, End) -> Span<std::remove_pointer_t<decltype (detail::toAddress (std::declval<T>()))>>;

template <typename T, size_t N>
Span (T (&) [N]) -> Span<T, N>;

template <typename T, size_t N>
Span (std::array<T, N>&) -> Span<T, N>;

template <typename T, size_t N>
Span (const std::array<T, N>&) -> Span<const T, N>;

template <typename Range>
Span (Range&& r) -> Span<std::remove_pointer_t<decltype (std::data (r))>>;


} // namespace juce
