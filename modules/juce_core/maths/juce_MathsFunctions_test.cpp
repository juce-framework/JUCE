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

//==============================================================================
template <typename T>
String getTemplatedMathsFunctionUnitTestName (const String& functionName)
{
    const auto getTypeName = []() -> String
    {
        if constexpr (std::is_same_v<int, T>)
            return "int";

        if constexpr (std::is_same_v<float, T>)
            return "float";

        if constexpr (std::is_same_v<double, T>)
            return "double";

        if constexpr (std::is_same_v<long double, T>)
            return "long double";
    };

    return functionName + "<" + getTypeName() + ">";
}

template <typename T>
class ApproximatelyEqualTests final : public UnitTest
{
public:
    ApproximatelyEqualTests()
        : UnitTest { getTemplatedMathsFunctionUnitTestName<T> ("approximatelyEqual"), UnitTestCategories::maths }
    {}

    void runTest() final
    {
        using limits = std::numeric_limits<T>;

        constexpr auto zero = T{};
        constexpr auto one = T (1);
        constexpr auto min = limits::min();
        constexpr auto max = limits::max();
        constexpr auto epsilon = limits::epsilon();
        constexpr auto oneThird = one / (T) 3;

        beginTest ("Equal values are always equal");
        {
            expect (approximatelyEqual (zero, zero));
            expect (approximatelyEqual (zero, -zero));
            expect (approximatelyEqual (-zero, -zero));

            expect (approximatelyEqual (min, min));
            expect (approximatelyEqual (-min, -min));

            expect (approximatelyEqual (one, one));
            expect (approximatelyEqual (-one, -one));

            expect (approximatelyEqual (max, max));
            expect (approximatelyEqual (-max, -max));

            const Tolerance<T> zeroTolerance{};

            expect (approximatelyEqual (zero, zero, zeroTolerance));
            expect (approximatelyEqual (zero, -zero, zeroTolerance));
            expect (approximatelyEqual (-zero, -zero, zeroTolerance));

            expect (approximatelyEqual (min, min, zeroTolerance));
            expect (approximatelyEqual (-min, -min, zeroTolerance));

            expect (approximatelyEqual (one, one, zeroTolerance));
            expect (approximatelyEqual (-one, -one, zeroTolerance));

            expect (approximatelyEqual (max, max, zeroTolerance));
            expect (approximatelyEqual (-max, -max, zeroTolerance));
        }

        beginTest ("Comparing subnormal values to zero, returns true");
        {
            expect (! exactlyEqual     (zero, nextFloatUp (zero)));
            expect (approximatelyEqual (zero, nextFloatUp (zero)));

            expect (! exactlyEqual     (zero, nextFloatDown (zero)));
            expect (approximatelyEqual (zero, nextFloatDown (zero)));

            expect (! exactlyEqual     (zero, nextFloatDown (min)));
            expect (approximatelyEqual (zero, nextFloatDown (min)));

            expect (! exactlyEqual     (zero, nextFloatUp (-min)));
            expect (approximatelyEqual (zero, nextFloatUp (-min)));
        }

        beginTest ("Comparing the minimum normal value to zero, returns true");
        {
            expect (approximatelyEqual (zero, min));
            expect (approximatelyEqual (zero, -min));
        }

        beginTest ("Comparing normal values greater than the minimum to zero, returns true");
        {
            expect (! approximatelyEqual (zero, one));
            expect (! approximatelyEqual (zero, epsilon));
            expect (! approximatelyEqual (zero, nextFloatUp (min)));
            expect (! approximatelyEqual (zero, nextFloatDown (-min)));
        }

        beginTest ("Values with large ranges can be compared");
        {
            expect (! approximatelyEqual (zero, max));
            expect (  approximatelyEqual (zero, max, absoluteTolerance (max)));
            expect (  approximatelyEqual (zero, max, relativeTolerance (one)));
            expect (! approximatelyEqual (-one, max));
            expect (! approximatelyEqual (-max, max));
        }

        beginTest ("Larger values have a boundary that is a factor of the epsilon");
        {
            for (auto exponent = 0; exponent < 127; ++exponent)
            {
                const auto value = std::pow ((T) 2, (T) exponent);
                const auto boundaryValue = value * (one + epsilon);

                expect (juce_isfinite (value));
                expect (juce_isfinite (boundaryValue));

                expect (  approximatelyEqual (value, boundaryValue));
                expect (! approximatelyEqual (value, nextFloatUp (boundaryValue)));

                expect (  approximatelyEqual (-value, -boundaryValue));
                expect (! approximatelyEqual (-value, nextFloatDown (-boundaryValue)));
            }
        }

        beginTest ("Tolerances scale with the values being compared");
        {

            expect (approximatelyEqual ((T) 100'000'000'000'000.01,
                                        (T) 100'000'000'000'000.011));

            expect (! approximatelyEqual ((T) 100.01,
                                          (T) 100.011));

            expect (! approximatelyEqual ((T) 123'000, (T) 121'000, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123'000, (T) 122'000, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123'000, (T) 123'000, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123'000, (T) 124'000, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 123'000, (T) 125'000, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual ((T) 123, (T) 121, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123, (T) 122, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123, (T) 123, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 123, (T) 124, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 123, (T) 125, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual ((T) 12.3, (T) 12.1, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 12.3, (T) 12.2, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 12.3, (T) 12.3, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 12.3, (T) 12.4, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 12.3, (T) 12.5, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual ((T) 1.23, (T) 1.21, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 1.23, (T) 1.22, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 1.23, (T) 1.23, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 1.23, (T) 1.24, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 1.23, (T) 1.25, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual ((T) 0.123, (T) 0.121, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.123, (T) 0.122, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.123, (T) 0.123, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.123, (T) 0.124, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 0.123, (T) 0.125, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual ((T) 0.000123, (T) 0.000121, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.000123, (T) 0.000122, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.000123, (T) 0.000123, relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual ((T) 0.000123, (T) 0.000124, relativeTolerance ((T) 1e-2)));
            expect (! approximatelyEqual ((T) 0.000123, (T) 0.000125, relativeTolerance ((T) 1e-2)));
        }

        beginTest ("The square of the square root of 2 is approximately 2");
        {
            constexpr auto two = (T) 2;
            const auto sqrtOfTwo = std::sqrt (two);

            expect (approximatelyEqual (sqrtOfTwo * sqrtOfTwo, two));
            expect (approximatelyEqual (-sqrtOfTwo * sqrtOfTwo, -two));
            expect (approximatelyEqual (two / sqrtOfTwo, sqrtOfTwo));
        }

        if constexpr (limits::has_quiet_NaN)
        {
            beginTest ("Values are never equal to NaN");
            {
                const auto nan = limits::quiet_NaN();

                expect (! approximatelyEqual (nan, nan));

                const auto expectNotEqualTo = [&] (auto value)
                {
                    expect (! approximatelyEqual (value, nan));
                    expect (! approximatelyEqual (nan, value));
                };

                expectNotEqualTo (zero);
                expectNotEqualTo (-zero);
                expectNotEqualTo (min);
                expectNotEqualTo (-min);
                expectNotEqualTo (one);
                expectNotEqualTo (-one);
                expectNotEqualTo (max);
                expectNotEqualTo (-max);
            }
        }

        if constexpr (limits::has_infinity)
        {
            beginTest ("Only infinity is equal to infinity");
            {
                const auto inf = limits::infinity();
                expect (approximatelyEqual (inf, inf));
                expect (approximatelyEqual (-inf, -inf));
                expect (! approximatelyEqual (inf, -inf));
                expect (! approximatelyEqual (-inf, inf));

                const auto expectNotEqualTo = [&] (auto value)
                {
                    expect (! approximatelyEqual (value, inf));
                    expect (! approximatelyEqual (value, -inf));
                    expect (! approximatelyEqual (inf, value));
                    expect (! approximatelyEqual (-inf, value));
                };

                expectNotEqualTo (zero);
                expectNotEqualTo (-zero);
                expectNotEqualTo (min);
                expectNotEqualTo (-min);
                expectNotEqualTo (one);
                expectNotEqualTo (-one);
                expectNotEqualTo (max);
                expectNotEqualTo (-max);
            }
        }

        beginTest ("Can set an absolute tolerance");
        {
            constexpr std::array<T, 7> negativePowersOfTwo
            {
                (T) 0.5 /* 2^-1 */,
                (T) 0.25 /* 2^-2 */,
                (T) 0.125 /* 2^-3 */,
                (T) 0.0625 /* 2^-4 */,
                (T) 0.03125 /* 2^-5 */,
                (T) 0.015625 /* 2^-6 */,
                (T) 0.0078125 /* 2^-7 */
            };

            const auto testTolerance = [&] (auto tolerance)
            {
                const auto t = Tolerance<T>{}.withAbsolute ((T) tolerance);

                const auto testValue= [&] (auto value)
                {
                    const auto boundary = value + tolerance;

                    expect (approximatelyEqual (value, boundary, t));
                    expect (! approximatelyEqual (value, nextFloatUp (boundary), t));

                    expect (approximatelyEqual (-value, -boundary, t));
                    expect (! approximatelyEqual (-value, nextFloatDown (-boundary), t));
                };

                testValue (zero);
                testValue (min);
                testValue (epsilon);
                testValue (one);

                for (const auto value : negativePowersOfTwo)
                    testValue (value);
            };

            for (const auto toleranceValue : negativePowersOfTwo)
                testTolerance (toleranceValue);
        }

        beginTest ("Can set a relative tolerance");
        {
            expect (! approximatelyEqual (oneThird, (T) 0.34,  relativeTolerance ((T) 1e-2)));
            expect (  approximatelyEqual (oneThird, (T) 0.334, relativeTolerance ((T) 1e-2)));

            expect (! approximatelyEqual (oneThird, (T) 0.334,  relativeTolerance ((T) 1e-3)));
            expect (  approximatelyEqual (oneThird, (T) 0.3334, relativeTolerance ((T) 1e-3)));

            expect (! approximatelyEqual (oneThird, (T) 0.3334,  relativeTolerance ((T) 1e-4)));
            expect (  approximatelyEqual (oneThird, (T) 0.33334, relativeTolerance ((T) 1e-4)));

            expect (! approximatelyEqual (oneThird, (T) 0.33334,  relativeTolerance ((T) 1e-5)));
            expect (  approximatelyEqual (oneThird, (T) 0.333334, relativeTolerance ((T) 1e-5)));

            expect (! approximatelyEqual (oneThird, (T) 0.333334,  relativeTolerance ((T) 1e-6)));
            expect (  approximatelyEqual (oneThird, (T) 0.3333334, relativeTolerance ((T) 1e-6)));

            expect (! approximatelyEqual (oneThird, (T) 0.3333334,  relativeTolerance ((T) 1e-7)));
            expect (  approximatelyEqual (oneThird, (T) 0.33333334, relativeTolerance ((T) 1e-7)));

            expect (  approximatelyEqual ((T) 1e6, (T) 1e6 + (T) 1, relativeTolerance ((T) 1e-6)));
            expect (! approximatelyEqual ((T) 1e6, (T) 1e6 + (T) 1, relativeTolerance ((T) 1e-7)));

            expect (  approximatelyEqual ((T) -1e-6, (T) -1.0000009e-6, relativeTolerance ((T) 1e-6)));
            expect (! approximatelyEqual ((T) -1e-6, (T) -1.0000009e-6, relativeTolerance ((T) 1e-7)));

            const auto a = (T) 1.234567;
            const auto b = (T) 1.234568;

            for (auto exponent = 0; exponent < 39; ++exponent)
            {
                const auto m = std::pow ((T) 10, (T) exponent);
                expect (  approximatelyEqual (a * m, b * m, relativeTolerance ((T) 1e-6)));
                expect (! approximatelyEqual (a * m, b * m, relativeTolerance ((T) 1e-7)));
            }
        }

        beginTest ("A relative tolerance is always scaled by the maximum value");
        {
            expect (  approximatelyEqual ((T) 9, (T) 10, absoluteTolerance ((T) 10.0 * (T) 0.1)));
            expect (! approximatelyEqual ((T) 9, (T) 10, absoluteTolerance ((T)  9.0 * (T) 0.1)));

            expect (approximatelyEqual ((T)  9, (T) 10, relativeTolerance ((T) 0.1)));
            expect (approximatelyEqual ((T) 10, (T)  9, relativeTolerance ((T) 0.1)));
        }

        beginTest ("Documentation examples");
        {
            constexpr auto pi = MathConstants<T>::pi;

            expect (! approximatelyEqual (zero, std::sin (pi)));
            expect (  approximatelyEqual (zero, std::sin (pi), absoluteTolerance (std::sin (pi))));

            expect (  approximatelyEqual ((T) 100, (T) 95, relativeTolerance ((T) 0.05)));
            expect (! approximatelyEqual ((T) 100, (T) 94, relativeTolerance ((T) 0.05)));
        }
    }
};

template<>
class ApproximatelyEqualTests<int> final : public UnitTest
{
public:
    ApproximatelyEqualTests()
        : UnitTest { getTemplatedMathsFunctionUnitTestName<int> ("approximatelyEqual"), UnitTestCategories::maths }
    {}

    void runTest() final
    {
        beginTest ("Identical integers are always equal");
        {
            expect (approximatelyEqual ( 0,  0));
            expect (approximatelyEqual (-0, -0));

            expect (approximatelyEqual ( 1,  1));
            expect (approximatelyEqual (-1, -1));

            using limits = std::numeric_limits<int>;
            constexpr auto min = limits::min();
            constexpr auto max = limits::max();

            expect (approximatelyEqual (min, min));
            expect (approximatelyEqual (max, max));
        }

        beginTest ("Non-identical integers are never equal");
        {
            expect (! approximatelyEqual ( 0,  1));
            expect (! approximatelyEqual ( 0, -1));

            expect (! approximatelyEqual ( 1,  2));
            expect (! approximatelyEqual (-1, -2));

            using limits = std::numeric_limits<int>;
            constexpr auto min = limits::min();
            constexpr auto max = limits::max();

            expect (! approximatelyEqual (min, min + 1));
            expect (! approximatelyEqual (max, max - 1));
        }

        beginTest ("Zero is equal regardless of the sign");
        {
            expect (approximatelyEqual ( 0, -0));
            expect (approximatelyEqual (-0,  0));
        }
    }
};

template <typename T>
class IsFiniteTests final : public UnitTest
{
public:
    IsFiniteTests()
        : UnitTest { getTemplatedMathsFunctionUnitTestName<T> ("juce_isfinite"), UnitTestCategories::maths }
    {}

    void runTest() final
    {
        using limits = std::numeric_limits<T>;

        constexpr auto zero = T{};
        constexpr auto one = (T) 1;
        constexpr auto max = limits::max();
        constexpr auto inf = limits::infinity();
        constexpr auto nan = limits::quiet_NaN();

        beginTest ("Zero is finite");
        {
            expect (juce_isfinite (zero));
            expect (juce_isfinite (-zero));
        }

        beginTest ("Subnormals are finite");
        {
            expect (juce_isfinite (nextFloatUp (zero)));
            expect (juce_isfinite (nextFloatDown (zero)));
        }

        beginTest ("One is finite");
        {
            expect (juce_isfinite (one));
            expect (juce_isfinite (-one));
        }

        beginTest ("Max is finite");
        {
            expect (juce_isfinite (max));
            expect (juce_isfinite (-max));
        }

        beginTest ("Infinity is not finite");
        {
            expect (! juce_isfinite (inf));
            expect (! juce_isfinite (-inf));
        }

        beginTest ("NaN is not finite");
        {
            expect (! juce_isfinite (nan));
            expect (! juce_isfinite (-nan));
            expect (! juce_isfinite (std::sqrt ((T) -1)));
            expect (! juce_isfinite (inf * zero));
        }
    }
};

template <typename T>
class NextFloatTests final : public UnitTest
{
public:
    NextFloatTests()
        : UnitTest { getTemplatedMathsFunctionUnitTestName<T> ("nextFloat"), UnitTestCategories::maths }
    {}

    void runTest() final
    {
        using limits = std::numeric_limits<T>;

        constexpr auto zero = T{};
        constexpr auto one = T (1);
        constexpr auto min = limits::min();
        constexpr auto epsilon = limits::epsilon();

        beginTest ("nextFloat from zero is subnormal");
        {
            expect (juce_isfinite (nextFloatUp (zero)));
            expect (! exactlyEqual (zero, nextFloatUp (zero)));
            expect (! std::isnormal (nextFloatUp (zero)));

            expect (juce_isfinite (nextFloatDown (zero)));
            expect (! exactlyEqual (zero, nextFloatDown (zero)));
            expect (! std::isnormal (nextFloatDown (zero)));
        }

        beginTest ("nextFloat from min, towards zero, is subnormal");
        {
            expect (std::isnormal (min));
            expect (std::isnormal (-min));
            expect (! std::isnormal (nextFloatDown (min)));
            expect (! std::isnormal (nextFloatUp (-min)));
        }

        beginTest ("nextFloat from one matches epsilon");
        {
            expect (! exactlyEqual (one,           nextFloatUp (one)));
            expect (  exactlyEqual (one + epsilon, nextFloatUp (one)));

            expect (! exactlyEqual (-one,           nextFloatDown (-one)));
            expect (  exactlyEqual (-one - epsilon, nextFloatDown (-one)));
        }
    }
};

template <typename Type>
struct MathsFloatingPointFunctionsTests
{
    IsFiniteTests<Type> isFiniteTests;
    NextFloatTests<Type> nextFloatTests;
    ApproximatelyEqualTests<Type> approximatelyEqualTests;
};

template<>
struct MathsFloatingPointFunctionsTests<int>
{
    ApproximatelyEqualTests<int> approximatelyEqualTests;
};

struct MathsFunctionsTests
{
    MathsFloatingPointFunctionsTests<int> intFunctionTests;
    MathsFloatingPointFunctionsTests<float> floatFunctionTests;
    MathsFloatingPointFunctionsTests<double> doubleFunctionTests;
    MathsFloatingPointFunctionsTests<long double> longDoubleFunctionTests;
};

static MathsFunctionsTests mathsFunctionsTests;

} // namespace juce
