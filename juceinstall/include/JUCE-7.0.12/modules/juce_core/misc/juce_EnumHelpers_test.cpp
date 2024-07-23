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
enum class TestEnum
{
    one    = 1 << 0,
    four   = 1 << 1,
    other  = 1 << 2
};

JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (TestEnum)
}

class EnumHelperTest final : public UnitTest
{
public:
    EnumHelperTest() : UnitTest ("EnumHelpers", UnitTestCategories::containers) {}

    void runTest() override
    {
        using detail::TestEnum;

        TestEnum e = {};

        beginTest ("Default initialised enum is 'none'");
        {
            expect (e == TestEnum{});
            expect (! hasBitValueSet (e, TestEnum{}));
        }

        beginTest ("withBitValueSet sets correct bit on empty enum");
        {
            e = withBitValueSet (e, TestEnum::other);
            expect (e == TestEnum::other);
            expect (hasBitValueSet (e, TestEnum::other));
        }

        beginTest ("withBitValueSet sets correct bit on non-empty enum");
        {
            e = withBitValueSet (e, TestEnum::one);
            expect (hasBitValueSet (e, TestEnum::one));
        }

        beginTest ("withBitValueCleared clears correct bit");
        {
            e = withBitValueCleared (e, TestEnum::one);
            expect (e != TestEnum::one);
            expect (hasBitValueSet (e, TestEnum::other));
            expect (! hasBitValueSet (e, TestEnum::one));
        }

        beginTest ("operators work as expected");
        {
            e = TestEnum::one;
            expect ((e & TestEnum::one) != TestEnum{});
            e |= TestEnum::other;
            expect ((e & TestEnum::other) != TestEnum{});

            e &= ~TestEnum::one;
            expect ((e & TestEnum::one) == TestEnum{});
            expect ((e & TestEnum::other) != TestEnum{});
        }
    }
};

static EnumHelperTest enumHelperTest;

} // namespace juce
