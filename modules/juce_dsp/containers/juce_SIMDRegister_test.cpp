/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

namespace SIMDRegister_test_internal
{
    template <typename type>
    static void fillRandom (type* dst, const int size, Random& random)
    {
        bool is_signed = std::is_signed<type>::value;

        for (int i = 0; i < size; ++i)
        {
            if (is_signed)
            {
                *dst++ = static_cast<type> ((random.nextFloat() * 16.0) - 8.0);
            }
            else
            {
                *dst++ = static_cast<type> (random.nextFloat() * 8.0);
            }
        }
    }

    template <typename type>
    static void fillRandom (std::complex<type>* dst, const int size, Random& random)
    {
        for (int i = 0; i < size; ++i)
        {
            type real, imag;

            real = static_cast<type> ((random.nextFloat() * 16.0) - 8.0);
            imag = static_cast<type> ((random.nextFloat() * 16.0) - 8.0);

            *dst++ = std::complex<type> (real, imag);
        }
    }

    // Avoid visual studio warning
    template <typename type>
    static type safeAbs (type a)
    {
        return static_cast<type> (fabs ((double) a));
    }

    template <typename type>
    static type safeAbs (std::complex<type> a)
    {
        return abs (a);
    }

    template <typename type>
    static double difference (type a)
    {
        return static_cast<double> (safeAbs (a));
    }

    template <typename type>
    static double difference (type a, type b)
    {
        return difference (a - b);
    }
}

// These tests need to be strictly run on all platforms supported by JUCE as the
// SIMD code is highly platform dependant.

class SIMDRegisterUnitTests   : public UnitTest
{
public:
    SIMDRegisterUnitTests()  : UnitTest ("SIMDRegister UnitTests") {}

    //==============================================================================
    // Some helper classes
    template <typename type>
    static bool allValuesEqualTo (const SIMDRegister<type>& vec, const type scalar)
    {
        // as we do not want to rely on the access operator we cast this to a primitive pointer
        const type* ptr = reinterpret_cast<const type*> (&vec);
        for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
            if (ptr[i] != scalar) return false;

        return true;
    }

    template <typename type>
    static bool vecEqualToArray (const SIMDRegister<type>& vec, const type* array)
    {
        const type* ptr = reinterpret_cast<const type*> (&vec);
        for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
        {
            double delta = SIMDRegister_test_internal::difference (ptr[i], array[i]);
            if (delta > 1e-4)
            {
                DBG ("a: " << SIMDRegister_test_internal::difference (ptr[i]) << " b: " << SIMDRegister_test_internal::difference (array[i]) << " difference: " << delta);
                return false;
            }
        }

        return true;
    }

    template <typename type>
    static void copy (SIMDRegister<type>& vec, const type* ptr)
    {
        for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
            vec[i] = ptr[i];
    }

    //==============================================================================
    // Someuseful operations to test
    struct Addition
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a += b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a + b;
        }
    };

    struct Subtraction
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a -= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a - b;
        }
    };

    struct Multiplication
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a *= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a * b;
        }
    };

    struct BitAND
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a &= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a & b;
        }
    };

    struct BitOR
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a |= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a | b;
        }
    };

    struct BitXOR
    {
        template <typename typeOne, typename typeTwo>
        static void inplace (typeOne& a, const typeTwo& b)
        {
            a ^= b;
        }

        template <typename typeOne, typename typeTwo>
        static typeOne outofplace (const typeOne& a, const typeTwo& b)
        {
            return a ^ b;
        }
    };

    //==============================================================================
    // the individual tests
    struct InitializationTest
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            u.expect (allValuesEqualTo<type> (SIMDRegister<type>::expand (static_cast<type> (23)), 23));

            {
                SIMDRegister<type> a;

                type* ptr = reinterpret_cast<type*>(&a);
                SIMDRegister_test_internal::fillRandom (ptr, SIMDRegister<type>::SIMDNumElements, random);

                u.expect (vecEqualToArray (SIMDRegister<type> (a), ptr));
            }
        }
    };

    struct AccessTest
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            // set-up
            SIMDRegister<type> a;
            type array [SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillRandom (array, SIMDRegister<type>::SIMDNumElements, random);

            // Test non-const access operator
            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                a[i] = array[i];

            u.expect (vecEqualToArray (a, array));

            // Test const access operator
            const SIMDRegister<type>& b = a;

            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                u.expect (b[i] == array[i]);
        }
    };

    template <class Operation>
    struct OperatorTests
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            for (int n = 0; n < 100; ++n)
            {
                // set-up
                SIMDRegister<type> a, b, c;
                type array_a [SIMDRegister<type>::SIMDNumElements];
                type array_b [SIMDRegister<type>::SIMDNumElements];
                type array_c [SIMDRegister<type>::SIMDNumElements];

                SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_c, SIMDRegister<type>::SIMDNumElements, random);

                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test in-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    Operation::template inplace<type, type> (array_a[i], array_b[i]);

                Operation::template inplace<SIMDRegister<type>, SIMDRegister<type> > (a, b);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));

                SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_c, SIMDRegister<type>::SIMDNumElements, random);

                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test in-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    Operation::template inplace<type, type> (array_b[i], static_cast<type> (2));

                Operation::template inplace<SIMDRegister<type>, type> (b, 2);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));

                // set-up again
                SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_c, SIMDRegister<type>::SIMDNumElements, random);
                copy (a, array_a); copy (b, array_b); copy (c, array_c);

                // test out-of-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<type, type> (array_a[i], array_b[i]);

                c = Operation::template outofplace<SIMDRegister<type>, SIMDRegister<type> > (a, b);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, array_c));

                // test out-of-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<type, type> (array_b[i], static_cast<type> (2));

                c = Operation::template outofplace<SIMDRegister<type>, type> (b, 2);

                u.expect (vecEqualToArray (a, array_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, array_c));
            }
        }
    };

    template <class Operation>
    struct BitOperatorTests
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            typedef typename SIMDRegister<type>::vMaskType vMaskType;
            typedef typename SIMDRegister<type>::MaskType MaskType;


            for (int n = 0; n < 100; ++n)
            {
                // Check flip sign bit and using as a union
                {
                    type array_a [SIMDRegister<type>::SIMDNumElements];

                    union
                    {
                        SIMDRegister<type> floatVersion;
                        vMaskType intVersion;
                    } a, b;

                    vMaskType bitmask = vMaskType::expand (static_cast<MaskType> (1) << (sizeof (MaskType) - 1));
                    SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
                    copy (a.floatVersion, array_a);
                    copy (b.floatVersion, array_a);

                    Operation::template inplace<SIMDRegister<type>, vMaskType> (a.floatVersion, bitmask);
                    Operation::template inplace<vMaskType, vMaskType> (b.intVersion, bitmask);

                    u.expect (vecEqualToArray (a.floatVersion, reinterpret_cast<const type*> (&b.floatVersion)));
                }

                // set-up
                SIMDRegister<type> a, c;
                vMaskType b;

                MaskType array_a [SIMDRegister<MaskType>::SIMDNumElements];
                MaskType array_b [SIMDRegister<MaskType>::SIMDNumElements];
                MaskType array_c [SIMDRegister<MaskType>::SIMDNumElements];

                type* conv_a = reinterpret_cast<type*> (array_a);
                type* conv_c = reinterpret_cast<type*> (array_c);

                SIMDRegister_test_internal::fillRandom (conv_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<MaskType>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (conv_c, SIMDRegister<type>::SIMDNumElements, random);
                copy (a, conv_a); copy (b, array_b); copy (c, conv_c);

                // test in-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    Operation::template inplace<MaskType, MaskType> (array_a[i], array_b[i]);

                Operation::template inplace<SIMDRegister<type>, vMaskType> (a, b);

                u.expect (vecEqualToArray (a, conv_a));
                u.expect (vecEqualToArray (b, array_b));

                SIMDRegister_test_internal::fillRandom (conv_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<MaskType>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (conv_c, SIMDRegister<type>::SIMDNumElements, random);
                copy (a, conv_a); copy (b, array_b); copy (c, conv_c);

                // test in-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    Operation::template inplace<MaskType, MaskType> (array_a[i], static_cast<MaskType> (9));

                Operation::template inplace<SIMDRegister<type>, MaskType> (a, static_cast<MaskType> (9));

                u.expect (vecEqualToArray (a, conv_a));
                u.expect (vecEqualToArray (b, array_b));

                // set-up again
                SIMDRegister_test_internal::fillRandom (conv_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<MaskType>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (conv_c, SIMDRegister<type>::SIMDNumElements, random);
                copy (a, conv_a); copy (b, array_b); copy (c, conv_c);

                // test out-of-place with both params being vectors
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                {
                    array_c[i] =
                        Operation::template outofplace<MaskType, MaskType> (array_a[i], array_b[i]);
                }

                c = Operation::template outofplace<SIMDRegister<type>, vMaskType> (a, b);

                u.expect (vecEqualToArray (a, conv_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, conv_c));

                // test out-of-place with one param being scalar
                for (size_t i = 0; i < SIMDRegister<MaskType>::SIMDNumElements; ++i)
                    array_c[i] = Operation::template outofplace<MaskType, MaskType> (array_a[i], static_cast<MaskType> (9));

                c = Operation::template outofplace<SIMDRegister<type>, MaskType> (a, static_cast<MaskType> (9));

                u.expect (vecEqualToArray (a, conv_a));
                u.expect (vecEqualToArray (b, array_b));
                u.expect (vecEqualToArray (c, conv_c));
            }
        }
    };

    struct CheckComparisonOps
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            typedef typename SIMDRegister<type>::vMaskType vMaskType;
            typedef typename SIMDRegister<type>::MaskType MaskType;

            for (int i = 0; i < 100; ++i)
            {
                // set-up
                type array_a   [SIMDRegister<type>::SIMDNumElements];
                type array_b   [SIMDRegister<type>::SIMDNumElements];
                MaskType array_eq  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_neq [SIMDRegister<type>::SIMDNumElements];
                MaskType array_lt  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_le  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_gt  [SIMDRegister<type>::SIMDNumElements];
                MaskType array_ge  [SIMDRegister<type>::SIMDNumElements];


                SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
                SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<type>::SIMDNumElements, random);

                // do check
                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_eq  [j] = (array_a[j] == array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_neq [j] = (array_a[j] != array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_lt  [j] = (array_a[j] <  array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_le  [j] = (array_a[j] <= array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_gt  [j] = (array_a[j] >  array_b[j]) ? static_cast<MaskType> (-1) : 0;
                    array_ge  [j] = (array_a[j] >= array_b[j]) ? static_cast<MaskType> (-1) : 0;
                }

                SIMDRegister<type> a, b;
                vMaskType eq, neq, lt, le, gt, ge;

                copy (a, array_a);
                copy (b, array_b);

                eq  = SIMDRegister<type>::equal              (a, b);
                neq = SIMDRegister<type>::notEqual           (a, b);
                lt  = SIMDRegister<type>::lessThan           (a, b);
                le  = SIMDRegister<type>::lessThanOrEqual    (a, b);
                gt  = SIMDRegister<type>::greaterThan        (a, b);
                ge  = SIMDRegister<type>::greaterThanOrEqual (a, b);

                u.expect (vecEqualToArray (eq,  array_eq ));
                u.expect (vecEqualToArray (neq, array_neq));
                u.expect (vecEqualToArray (lt,  array_lt ));
                u.expect (vecEqualToArray (le,  array_le ));
                u.expect (vecEqualToArray (gt,  array_gt ));
                u.expect (vecEqualToArray (ge,  array_ge ));
            }
        }
    };

    struct CheckMultiplyAdd
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            // set-up
            type array_a [SIMDRegister<type>::SIMDNumElements];
            type array_b [SIMDRegister<type>::SIMDNumElements];
            type array_c [SIMDRegister<type>::SIMDNumElements];
            type array_d [SIMDRegister<type>::SIMDNumElements];

            SIMDRegister_test_internal::fillRandom (array_a, SIMDRegister<type>::SIMDNumElements, random);
            SIMDRegister_test_internal::fillRandom (array_b, SIMDRegister<type>::SIMDNumElements, random);
            SIMDRegister_test_internal::fillRandom (array_c, SIMDRegister<type>::SIMDNumElements, random);
            SIMDRegister_test_internal::fillRandom (array_d, SIMDRegister<type>::SIMDNumElements, random);

            // check
            for (size_t i = 0; i < SIMDRegister<type>::SIMDNumElements; ++i)
                array_d[i] = array_a[i] + (array_b[i] * array_c[i]);

            SIMDRegister<type> a, b, c, d;

            copy (a, array_a);
            copy (b, array_b);
            copy (c, array_c);

            d = SIMDRegister<type>::multiplyAdd (a, b, c);

            u.expect (vecEqualToArray (d, array_d));
        }
    };

    struct CheckMinMax
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            for (int i = 0; i < 100; ++i)
            {
                type array_a [SIMDRegister<type>::SIMDNumElements];
                type array_b [SIMDRegister<type>::SIMDNumElements];
                type array_min [SIMDRegister<type>::SIMDNumElements];
                type array_max [SIMDRegister<type>::SIMDNumElements];

                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_a[j] = static_cast<type> (random.nextInt (127));
                    array_b[j] = static_cast<type> (random.nextInt (127));
                }

                for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
                {
                    array_min[j] = (array_a[j] < array_b[j]) ? array_a[j] : array_b[j];
                    array_max[j] = (array_a[j] > array_b[j]) ? array_a[j] : array_b[j];
                }

                SIMDRegister<type> a, b, vMin, vMax;

                copy (a, array_a);
                copy (b, array_b);

                vMin = jmin (a, b);
                vMax = jmax (a, b);

                u.expect (vecEqualToArray (vMin, array_min));
                u.expect (vecEqualToArray (vMax, array_max));

                copy (vMin, array_a);
                copy (vMax, array_a);

                vMin = SIMDRegister<type>::min (a, b);
                vMax = SIMDRegister<type>::max (a, b);

                u.expect (vecEqualToArray (vMin, array_min));
                u.expect (vecEqualToArray (vMax, array_max));
            }
        }
    };

    struct CheckSum
    {
        template <typename type>
        static void run (UnitTest& u, Random& random)
        {
            type array [SIMDRegister<type>::SIMDNumElements];
            type sumCheck = 0;

            SIMDRegister_test_internal::fillRandom (array, SIMDRegister<type>::SIMDNumElements, random);

            for (size_t j = 0; j < SIMDRegister<type>::SIMDNumElements; ++j)
            {
                sumCheck += array[j];
            }

            SIMDRegister<type> a;
            copy (a, array);

            u.expect (SIMDRegister_test_internal::difference (sumCheck, a.sum()) < 1e-4);
        }
    };

    //==============================================================================
    template <class TheTest>
    void runTestForAllTypes (const char* unitTestName)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::template run<float>   (*this, random);
        TheTest::template run<double>  (*this, random);
        TheTest::template run<int8_t>  (*this, random);
        TheTest::template run<uint8_t> (*this, random);
        TheTest::template run<int16_t> (*this, random);
        TheTest::template run<uint16_t>(*this, random);
        TheTest::template run<int32_t> (*this, random);
        TheTest::template run<uint32_t>(*this, random);
        TheTest::template run<int64_t> (*this, random);
        TheTest::template run<uint64_t>(*this, random);
        TheTest::template run<std::complex<float> >   (*this, random);
        TheTest::template run<std::complex<double>>  (*this, random);
    }

    template <class TheTest>
    void runTestNonComplex (const char* unitTestName)
    {
        beginTest (unitTestName);

        Random random = getRandom();

        TheTest::template run<float>   (*this, random);
        TheTest::template run<double>  (*this, random);
        TheTest::template run<int8_t>  (*this, random);
        TheTest::template run<uint8_t> (*this, random);
        TheTest::template run<int16_t> (*this, random);
        TheTest::template run<uint16_t>(*this, random);
        TheTest::template run<int32_t> (*this, random);
        TheTest::template run<uint32_t>(*this, random);
        TheTest::template run<int64_t> (*this, random);
        TheTest::template run<uint64_t>(*this, random);
    }

    void runTest()
    {
        runTestForAllTypes<InitializationTest> ("InitializationTest");

        runTestForAllTypes<AccessTest> ("AccessTest");

        runTestForAllTypes<OperatorTests<Addition> > ("AdditionOperators");
        runTestForAllTypes<OperatorTests<Subtraction> > ("SubtractionOperators");
        runTestForAllTypes<OperatorTests<Multiplication> > ("MultiplicationOperators");

        runTestForAllTypes<BitOperatorTests<BitAND> > ("BitANDOperators");
        runTestForAllTypes<BitOperatorTests<BitOR> > ("BitOROperators");
        runTestForAllTypes<BitOperatorTests<BitXOR> > ("BitXOROperators");

        runTestNonComplex<CheckComparisonOps> ("CheckComparisons");
        runTestNonComplex<CheckMinMax> ("CheckMinMax");

        runTestForAllTypes<CheckMultiplyAdd> ("CheckMultiplyAdd");
        runTestForAllTypes<CheckSum> ("CheckSum");
    }
};

static SIMDRegisterUnitTests SIMDRegisterUnitTests;

} // namespace dsp
} // namespace juce
