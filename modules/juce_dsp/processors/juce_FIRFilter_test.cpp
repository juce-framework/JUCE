/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

class FIRFilterTest final : public UnitTest
{
    template <typename Type>
    struct Helpers
    {
        static void fillRandom (Random& random, Type* buffer, size_t n)
        {
            for (size_t i = 0; i < n; ++i)
                buffer[i] = (2.0f * random.nextFloat()) - 1.0f;
        }

        static bool checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept
        {
            for (size_t i = 0; i < n; ++i)
                if (std::abs (a[i] - b[i]) > 1e-6f)
                    return false;

            return true;
        }
    };

   #if JUCE_USE_SIMD
    template <typename Type>
    struct Helpers<SIMDRegister<Type>>
    {
        static void fillRandom (Random& random, SIMDRegister<Type>* buffer, size_t n)
        {
            Helpers<Type>::fillRandom (random, reinterpret_cast<Type*> (buffer), n * SIMDRegister<Type>::size());
        }

        static bool checkArrayIsSimilar (SIMDRegister<Type>* a, SIMDRegister<Type>* b, size_t n) noexcept
        {
            return Helpers<Type>::checkArrayIsSimilar (reinterpret_cast<Type*> (a),
                                                       reinterpret_cast<Type*> (b),
                                                       n * SIMDRegister<Type>::size());
        }
    };
   #endif

    template <typename Type>
    static void fillRandom (Random& random, Type* buffer, size_t n) { Helpers<Type>::fillRandom (random, buffer, n); }

    template <typename Type>
    static bool checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept { return Helpers<Type>::checkArrayIsSimilar (a, b, n); }

    //==============================================================================
    // reference implementation of an FIR
    template <typename SampleType, typename NumericType>
    static void reference (const NumericType* firCoefficients, size_t numCoefficients,
                           const SampleType* input, SampleType* output, size_t n) noexcept
    {
        if (numCoefficients == 0)
        {
            zeromem (output, sizeof (SampleType) * n);
            return;
        }

        HeapBlock<SampleType> scratchBuffer (numCoefficients
                                            #if JUCE_USE_SIMD
                                             + (SIMDRegister<NumericType>::SIMDRegisterSize / sizeof (SampleType))
                                            #endif
                                             );
       #if JUCE_USE_SIMD
        SampleType* buffer = reinterpret_cast<SampleType*> (SIMDRegister<NumericType>::getNextSIMDAlignedPtr (reinterpret_cast<NumericType*> (scratchBuffer.getData())));
       #else
        SampleType* buffer = scratchBuffer.getData();
       #endif

        zeromem (buffer, sizeof (SampleType) * numCoefficients);

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = (numCoefficients - 1); j >= 1; --j)
                buffer[j] = buffer[j-1];

            buffer[0] = input[i];

            SampleType sum (0);

            for (size_t j = 0; j < numCoefficients; ++j)
                sum += buffer[j] * firCoefficients[j];

            output[i] = sum;
        }
    }

    //==============================================================================
    struct LargeBlockTest
    {
        template <typename FloatType>
        static void run (FIR::Filter<FloatType>& filter, FloatType* src, FloatType* dst, size_t n)
        {
            AudioBlock<const FloatType> input (&src, 1, n);
            AudioBlock<FloatType> output (&dst, 1, n);
            ProcessContextNonReplacing<FloatType> context (input, output);

            filter.process (context);
        }
    };

    struct SampleBySampleTest
    {
        template <typename FloatType>
        static void run (FIR::Filter<FloatType>& filter, FloatType* src, FloatType* dst, size_t n)
        {
            for (size_t i = 0; i < n; ++i)
                dst[i] = filter.processSample (src[i]);
        }
    };

    struct SplitBlockTest
    {
        template <typename FloatType>
        static void run (FIR::Filter<FloatType>& filter, FloatType* input, FloatType* output, size_t n)
        {
            size_t len = 0;
            for (size_t i = 0; i < n; i += len)
            {
                len = jmin (n - i, n / 3);
                auto* src = input + i;
                auto* dst = output + i;

                AudioBlock<const FloatType> inBlock (&src, 1, len);
                AudioBlock<FloatType> outBlock (&dst, 1, len);
                ProcessContextNonReplacing<FloatType> context (inBlock, outBlock);

                filter.process (context);
            }
        }
    };

    //==============================================================================
    template <typename TheTest, typename SampleType, typename NumericType>
    void runTestForType()
    {
        Random random (8392829);

        for (auto size : {1, 2, 4, 8, 12, 13, 25})
        {
            constexpr size_t n = 813;

            HeapBlock<char> inputBuffer, outputBuffer, refBuffer;
            AudioBlock<SampleType> input (inputBuffer, 1, n), output (outputBuffer, 1, n), ref (refBuffer, 1, n);
            fillRandom (random, input.getChannelPointer (0), n);

            HeapBlock<char> firBlock;
            AudioBlock<NumericType> fir (firBlock, 1, static_cast<size_t> (size));
            fillRandom (random, fir.getChannelPointer (0), static_cast<size_t> (size));

            FIR::Filter<SampleType> filter (*new FIR::Coefficients<NumericType> (fir.getChannelPointer (0), static_cast<size_t> (size)));
            ProcessSpec spec {0.0, n, 1};
            filter.prepare (spec);

            reference<SampleType, NumericType> (fir.getChannelPointer (0), static_cast<size_t> (size),
                                                input.getChannelPointer (0), ref.getChannelPointer (0), n);

            TheTest::template run<SampleType> (filter, input.getChannelPointer (0), output.getChannelPointer (0), n);
            expect (checkArrayIsSimilar (output.getChannelPointer (0), ref.getChannelPointer (0), n));
        }
    }

    template <typename TheTest>
    void runTestForAllTypes (const char* unitTestName)
    {
        beginTest (unitTestName);

        runTestForType<TheTest, float, float>();
        runTestForType<TheTest, double, double>();
       #if JUCE_USE_SIMD
        runTestForType<TheTest, SIMDRegister<float>, float>();
        runTestForType<TheTest, SIMDRegister<double>, double>();
       #endif
    }


public:
    FIRFilterTest()
        : UnitTest ("FIR Filter", UnitTestCategories::dsp)
    {}

    void runTest() override
    {
        runTestForAllTypes<LargeBlockTest> ("Large Blocks");
        runTestForAllTypes<SampleBySampleTest> ("Sample by Sample");
        runTestForAllTypes<SplitBlockTest> ("Split Block");
    }
};

static FIRFilterTest firFilterUnitTest;

} // namespace juce::dsp
