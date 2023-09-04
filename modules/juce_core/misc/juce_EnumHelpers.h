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

//==============================================================================
/**
    Macro to enable bitwise operations for scoped enums (enum struct/class).

    To use this, add the line JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)
    after your enum declaration at file scope level.

    e.g. @code

    enum class MyEnum
    {
        one     = 1 << 0,
        two     = 1 << 1,
        three   = 1 << 2
    };

    JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (MyEnum)

    MyEnum e = MyEnum::one | MyEnum::two;

    bool hasTwo = (e & MyEnum::two) != MyEnum{}; // true
    bool hasTwo = hasBitValueSet (e, MyEnum::two); // true

    e = withBitValueCleared (e, MyEnum::two);

    bool hasTwo = hasBitValueSet (e, MyEnum::two); // false

    @endcode
*/
#define JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS(EnumType)               \
    static_assert (std::is_enum_v<EnumType>,                               \
                   "JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS "           \
                   "should only be used with enum types");                 \
    constexpr auto operator& (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) & base_type (b));      \
    }                                                                      \
    constexpr auto operator| (EnumType a, EnumType b)                      \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (base_type (a) | base_type (b));      \
    }                                                                      \
    constexpr auto operator~ (EnumType a)                                  \
    {                                                                      \
        using base_type = std::underlying_type<EnumType>::type;            \
        return static_cast<EnumType> (~base_type (a));                     \
    }                                                                      \
    constexpr auto& operator|= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a | b);                                                       \
        return a;                                                          \
    }                                                                      \
    constexpr auto& operator&= (EnumType& a, EnumType b)                   \
    {                                                                      \
        a = (a & b);                                                       \
        return a;                                                          \
    }


namespace juce
{

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr bool hasBitValueSet (EnumType enumValue, EnumType valueToLookFor) noexcept
{
    return (enumValue & valueToLookFor) != EnumType{};
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr EnumType withBitValueSet (EnumType enumValue, EnumType valueToAdd) noexcept
{
    return enumValue | valueToAdd;
}

template <typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, int> = 0>
constexpr EnumType withBitValueCleared (EnumType enumValue, EnumType valueToRemove) noexcept
{
    return enumValue & ~valueToRemove;
}
}
