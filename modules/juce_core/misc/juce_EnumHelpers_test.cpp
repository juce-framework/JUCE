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
