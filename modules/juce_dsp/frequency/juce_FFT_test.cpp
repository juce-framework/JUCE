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

namespace juce::dsp
{

struct FFTUnitTest final : public UnitTest
{
    FFTUnitTest()
        : UnitTest ("FFT", UnitTestCategories::dsp)
    {}

    static void fillRandom (Random& random, Complex<float>* buffer, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            buffer[i] = Complex<float> ((2.0f * random.nextFloat()) - 1.0f,
                                             (2.0f * random.nextFloat()) - 1.0f);
    }

    static void fillRandom (Random& random, float* buffer, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
            buffer[i] = (2.0f * random.nextFloat()) - 1.0f;
    }

    static Complex<float> freqConvolution (const Complex<float>* in, float freq, size_t n)
    {
        Complex<float> sum (0.0, 0.0);
        for (size_t i = 0; i < n; ++i)
            sum += in[i] * exp (Complex<float> (0, static_cast<float> (i) * freq));

        return sum;
    }

    static void performReferenceFourier (const Complex<float>* in, Complex<float>* out,
                                         size_t n, bool reverse)
    {
        auto base_freq = static_cast<float> (((reverse ? 1.0 : -1.0) * MathConstants<double>::twoPi)
                                               / static_cast<float> (n));

        for (size_t i = 0; i < n; ++i)
            out[i] = freqConvolution (in, static_cast<float> (i) * base_freq, n);
    }

    static void performReferenceFourier (const float* in, Complex<float>* out,
                                         size_t n, bool reverse)
    {
        HeapBlock<Complex<float>> buffer (n);

        for (size_t i = 0; i < n; ++i)
            buffer.getData()[i] = Complex<float> (in[i], 0.0f);

        float base_freq = static_cast<float> (((reverse ? 1.0 : -1.0) * MathConstants<double>::twoPi)
                                                / static_cast<float> (n));

        for (size_t i = 0; i < n; ++i)
            out[i] = freqConvolution (buffer.getData(), static_cast<float> (i) * base_freq, n);
    }


    //==============================================================================
    template <typename Type>
    static bool checkArrayIsSimilar (Type* a, Type* b, size_t n) noexcept
    {
        for (size_t i = 0; i < n; ++i)
            if (std::abs (a[i] - b[i]) > 1e-3f)
                return false;

        return true;
    }

    struct RealTest
    {
        static void run (FFTUnitTest& u)
        {
            Random random (378272);

            for (size_t order = 0; order <= 8; ++order)
            {
                auto n = (1u << order);

                FFT fft ((int) order);

                HeapBlock<float> input (n);
                HeapBlock<Complex<float>> reference (n), output (n);

                fillRandom (random, input.getData(), n);
                performReferenceFourier (input.getData(), reference.getData(), n, false);

                // fill only first half with real numbers
                zeromem (output.getData(), n * sizeof (Complex<float>));
                memcpy (reinterpret_cast<float*> (output.getData()), input.getData(), n * sizeof (float));

                fft.performRealOnlyForwardTransform ((float*) output.getData());
                u.expect (checkArrayIsSimilar (reference.getData(), output.getData(), n));

                // fill only first half with real numbers
                zeromem (output.getData(), n * sizeof (Complex<float>));
                memcpy (reinterpret_cast<float*> (output.getData()), input.getData(), n * sizeof (float));

                fft.performRealOnlyForwardTransform ((float*) output.getData(), true);
                std::fill (reference.getData() + ((n >> 1) + 1), reference.getData() + n, std::complex<float> (0.0f));
                u.expect (checkArrayIsSimilar (reference.getData(), output.getData(), (n >> 1) + 1));

                memcpy (output.getData(), reference.getData(), n * sizeof (Complex<float>));
                fft.performRealOnlyInverseTransform ((float*) output.getData());
                u.expect (checkArrayIsSimilar ((float*) output.getData(), input.getData(), n));
            }
        }
    };

    struct FrequencyOnlyTest
    {
        static void run (FFTUnitTest& u)
        {
            Random random (378272);
            for (size_t order = 0; order <= 8; ++order)
            {
                auto n = (1u << order);

                FFT fft ((int) order);

                std::vector<float> inout ((size_t) n << 1), reference ((size_t) n << 1);
                std::vector<Complex<float>> frequency (n);

                fillRandom (random, inout.data(), n);
                zeromem (reference.data(), sizeof (float) * ((size_t) n << 1));
                performReferenceFourier (inout.data(), frequency.data(), n, false);

                for (size_t i = 0; i < n; ++i)
                    reference[i] = std::abs (frequency[i]);

                for (auto ignoreNegative : { false, true })
                {
                    auto inoutCopy = inout;
                    fft.performFrequencyOnlyForwardTransform (inoutCopy.data(), ignoreNegative);
                    auto numMatching = ignoreNegative ? (n / 2) + 1 : n;
                    u.expect (checkArrayIsSimilar (inoutCopy.data(), reference.data(), numMatching));
                }
            }
        }
    };

    struct ComplexTest
    {
        static void run (FFTUnitTest& u)
        {
            Random random (378272);

            for (size_t order = 0; order <= 7; ++order)
            {
                auto n = (1u << order);

                FFT fft ((int) order);

                HeapBlock<Complex<float>> input (n), buffer (n), output (n), reference (n);

                fillRandom (random, input.getData(), n);
                performReferenceFourier (input.getData(), reference.getData(), n, false);

                memcpy (buffer.getData(), input.getData(), sizeof (Complex<float>) * n);
                fft.perform (buffer.getData(), output.getData(), false);

                u.expect (checkArrayIsSimilar (output.getData(), reference.getData(), n));

                memcpy (buffer.getData(), reference.getData(), sizeof (Complex<float>) * n);
                fft.perform (buffer.getData(), output.getData(), true);


                u.expect (checkArrayIsSimilar (output.getData(), input.getData(), n));
            }
        }
    };

    template <class TheTest>
    void runTestForAllTypes (const char* unitTestName)
    {
        beginTest (unitTestName);

        TheTest::run (*this);
    }

    void runTest() override
    {
        runTestForAllTypes<RealTest> ("Real input numbers Test");
        runTestForAllTypes<FrequencyOnlyTest> ("Frequency only Test");
        runTestForAllTypes<ComplexTest> ("Complex input numbers Test");
    }
};

static FFTUnitTest fftUnitTest;

} // namespace juce::dsp
