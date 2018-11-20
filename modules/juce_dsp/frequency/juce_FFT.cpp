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

struct FFT::Instance
{
    virtual ~Instance() {}
    virtual void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept = 0;
    virtual void performRealOnlyForwardTransform (float*, bool) const noexcept = 0;
    virtual void performRealOnlyInverseTransform (float*) const noexcept = 0;
};

struct FFT::Engine
{
    Engine (int priorityToUse) : enginePriority (priorityToUse)
    {
        auto& list = getEngines();
        list.add (this);
        std::sort (list.begin(), list.end(), [] (Engine* a, Engine* b) { return b->enginePriority < a->enginePriority; });
    }

    virtual ~Engine() {}

    virtual FFT::Instance* create (int order) const = 0;

    //==============================================================================
    static FFT::Instance* createBestEngineForPlatform (int order)
    {
        for (auto* engine : getEngines())
            if (auto* instance = engine->create (order))
                return instance;

        jassertfalse;  // This should never happen as the fallback engine should always work!
        return nullptr;
    }

private:
    static Array<Engine*>& getEngines()
    {
        static Array<Engine*> engines;
        return engines;
    }

    int enginePriority; // used so that faster engines have priority over slower ones
};

template <typename InstanceToUse>
struct FFT::EngineImpl  : public FFT::Engine
{
    EngineImpl() : FFT::Engine (InstanceToUse::priority)        {}
    FFT::Instance* create (int order) const override            { return InstanceToUse::create (order); }
};

//==============================================================================
//==============================================================================
struct FFTFallback  : public FFT::Instance
{
    // this should have the least priority of all engines
    static constexpr int priority = -1;

    static FFTFallback* create (int order)
    {
        return new FFTFallback (order);
    }

    FFTFallback (int order)
    {
        configForward.reset (new FFTConfig (1 << order, false));
        configInverse.reset (new FFTConfig (1 << order, true));

        size = 1 << order;
    }

    void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept override
    {
        if (size == 1)
        {
            *output = *input;
            return;
        }

        const SpinLock::ScopedLockType sl(processLock);

        jassert (configForward != nullptr);

        if (inverse)
        {
            configInverse->perform (input, output);

            const float scaleFactor = 1.0f / size;

            for (int i = 0; i < size; ++i)
                output[i] *= scaleFactor;
        }
        else
        {
            configForward->perform (input, output);
        }
    }

    const size_t maxFFTScratchSpaceToAlloca = 256 * 1024;

    void performRealOnlyForwardTransform (float* d, bool) const noexcept override
    {
        if (size == 1)
            return;

        const size_t scratchSize = 16 + sizeof (Complex<float>) * (size_t) size;

        if (scratchSize < maxFFTScratchSpaceToAlloca)
        {
            performRealOnlyForwardTransform (static_cast<Complex<float>*> (alloca (scratchSize)), d);
        }
        else
        {
            HeapBlock<char> heapSpace (scratchSize);
            performRealOnlyForwardTransform (reinterpret_cast<Complex<float>*> (heapSpace.getData()), d);
        }
    }

    void performRealOnlyInverseTransform (float* d) const noexcept override
    {
        if (size == 1)
            return;

        const size_t scratchSize = 16 + sizeof (Complex<float>) * (size_t) size;

        if (scratchSize < maxFFTScratchSpaceToAlloca)
        {
            performRealOnlyInverseTransform (static_cast<Complex<float>*> (alloca (scratchSize)), d);
        }
        else
        {
            HeapBlock<char> heapSpace (scratchSize);
            performRealOnlyInverseTransform (reinterpret_cast<Complex<float>*> (heapSpace.getData()), d);
        }
    }

    void performRealOnlyForwardTransform (Complex<float>* scratch, float* d) const noexcept
    {
        for (int i = 0; i < size; ++i)
            scratch[i] = { d[i], 0 };

        perform (scratch, reinterpret_cast<Complex<float>*> (d), false);
    }

    void performRealOnlyInverseTransform (Complex<float>* scratch, float* d) const noexcept
    {
        auto* input = reinterpret_cast<Complex<float>*> (d);

        for (auto i = size >> 1; i < size; ++i)
            input[i] = std::conj (input[size - i]);

        perform (input, scratch, true);

        for (int i = 0; i < size; ++i)
        {
            d[i] = scratch[i].real();
            d[i + size] = scratch[i].imag();
        }
    }

    //==============================================================================
    struct FFTConfig
    {
        FFTConfig (int sizeOfFFT, bool isInverse)
            : fftSize (sizeOfFFT), inverse (isInverse), twiddleTable ((size_t) sizeOfFFT)
        {
            auto inverseFactor = (inverse ? 2.0 : -2.0) * MathConstants<double>::pi / (double) fftSize;

            if (fftSize <= 4)
            {
                for (int i = 0; i < fftSize; ++i)
                {
                    auto phase = i * inverseFactor;

                    twiddleTable[i] = { (float) std::cos (phase),
                                        (float) std::sin (phase) };
                }
            }
            else
            {
                for (int i = 0; i < fftSize / 4; ++i)
                {
                    auto phase = i * inverseFactor;

                    twiddleTable[i] = { (float) std::cos (phase),
                                        (float) std::sin (phase) };
                }

                for (int i = fftSize / 4; i < fftSize / 2; ++i)
                {
                    auto other = twiddleTable[i - fftSize / 4];

                    twiddleTable[i] = { inverse ? -other.imag() :  other.imag(),
                                        inverse ?  other.real() : -other.real() };
                }

                twiddleTable[fftSize / 2].real (-1.0f);
                twiddleTable[fftSize / 2].imag (0.0f);

                for (int i = fftSize / 2; i < fftSize; ++i)
                {
                    auto index = fftSize / 2 - (i - fftSize / 2);
                    twiddleTable[i] = conj(twiddleTable[index]);
                }
            }

            auto root = (int) std::sqrt ((double) fftSize);
            int divisor = 4, n = fftSize;

            for (int i = 0; i < numElementsInArray (factors); ++i)
            {
                while ((n % divisor) != 0)
                {
                    if (divisor == 2)       divisor = 3;
                    else if (divisor == 4)  divisor = 2;
                    else                    divisor += 2;

                    if (divisor > root)
                        divisor = n;
                }

                n /= divisor;

                jassert (divisor == 1 || divisor == 2 || divisor == 4);
                factors[i].radix = divisor;
                factors[i].length = n;
            }
        }

        void perform (const Complex<float>* input, Complex<float>* output) const noexcept
        {
            perform (input, output, 1, 1, factors);
        }

        const int fftSize;
        const bool inverse;

        struct Factor { int radix, length; };
        Factor factors[32];
        HeapBlock<Complex<float>> twiddleTable;

        void perform (const Complex<float>* input, Complex<float>* output, int stride, int strideIn, const Factor* facs) const noexcept
        {
            auto factor = *facs++;
            auto* originalOutput = output;
            auto* outputEnd = output + factor.radix * factor.length;

            if (stride == 1 && factor.radix <= 5)
            {
                for (int i = 0; i < factor.radix; ++i)
                    perform (input + stride * strideIn * i, output + i * factor.length, stride * factor.radix, strideIn, facs);

                butterfly (factor, output, stride);
                return;
            }

            if (factor.length == 1)
            {
                do
                {
                    *output++ = *input;
                    input += stride * strideIn;
                }
                while (output < outputEnd);
            }
            else
            {
                do
                {
                    perform (input, output, stride * factor.radix, strideIn, facs);
                    input += stride * strideIn;
                    output += factor.length;
                }
                while (output < outputEnd);
            }

            butterfly (factor, originalOutput, stride);
        }

        void butterfly (const Factor factor, Complex<float>* data, int stride) const noexcept
        {
            switch (factor.radix)
            {
                case 1:   break;
                case 2:   butterfly2 (data, stride, factor.length); return;
                case 4:   butterfly4 (data, stride, factor.length); return;
                default:  jassertfalse; break;
            }

            auto* scratch = static_cast<Complex<float>*> (alloca (sizeof (Complex<float>) * (size_t) factor.radix));

            for (int i = 0; i < factor.length; ++i)
            {
                for (int k = i, q1 = 0; q1 < factor.radix; ++q1)
                {
                    scratch[q1] = data[k];
                    k += factor.length;
                }

                for (int k = i, q1 = 0; q1 < factor.radix; ++q1)
                {
                    int twiddleIndex = 0;
                    data[k] = scratch[0];

                    for (int q = 1; q < factor.radix; ++q)
                    {
                        twiddleIndex += stride * k;

                        if (twiddleIndex >= fftSize)
                            twiddleIndex -= fftSize;

                        data[k] += scratch[q] * twiddleTable[twiddleIndex];
                    }

                    k += factor.length;
                }
            }
        }

        void butterfly2 (Complex<float>* data, const int stride, const int length) const noexcept
        {
            auto* dataEnd = data + length;
            auto* tw = twiddleTable.getData();

            for (int i = length; --i >= 0;)
            {
                auto s = *dataEnd;
                s *= (*tw);
                tw += stride;
                *dataEnd++ = *data - s;
                *data++ += s;
            }
        }

        void butterfly4 (Complex<float>* data, const int stride, const int length) const noexcept
        {
            auto lengthX2 = length * 2;
            auto lengthX3 = length * 3;

            auto strideX2 = stride * 2;
            auto strideX3 = stride * 3;

            auto* twiddle1 = twiddleTable.getData();
            auto* twiddle2 = twiddle1;
            auto* twiddle3 = twiddle1;

            for (int i = length; --i >= 0;)
            {
                auto s0 = data[length]   * *twiddle1;
                auto s1 = data[lengthX2] * *twiddle2;
                auto s2 = data[lengthX3] * *twiddle3;
                auto s3 = s0;             s3 += s2;
                auto s4 = s0;             s4 -= s2;
                auto s5 = *data;          s5 -= s1;

                *data += s1;
                data[lengthX2] = *data;
                data[lengthX2] -= s3;
                twiddle1 += stride;
                twiddle2 += strideX2;
                twiddle3 += strideX3;
                *data += s3;

                if (inverse)
                {
                    data[length] = { s5.real() - s4.imag(),
                                     s5.imag() + s4.real() };

                    data[lengthX3] = { s5.real() + s4.imag(),
                                       s5.imag() - s4.real() };
                }
                else
                {
                    data[length] = { s5.real() + s4.imag(),
                                     s5.imag() - s4.real() };

                    data[lengthX3] = { s5.real() - s4.imag(),
                                       s5.imag() + s4.real() };
                }

                ++data;
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFTConfig)
    };

    //==============================================================================
    SpinLock processLock;
    std::unique_ptr<FFTConfig> configForward, configInverse;
    int size;
};

FFT::EngineImpl<FFTFallback> fftFallback;

//==============================================================================
//==============================================================================
#if (JUCE_MAC || JUCE_IOS) && JUCE_USE_VDSP_FRAMEWORK
struct AppleFFT  : public FFT::Instance
{
    static constexpr int priority = 5;

    static AppleFFT* create (int order)
    {
        return new AppleFFT (order);
    }

    AppleFFT (int orderToUse)
        : order (static_cast<vDSP_Length> (orderToUse)),
          fftSetup (vDSP_create_fftsetup (order, 2)),
          forwardNormalisation (0.5f),
          inverseNormalisation (1.0f / static_cast<float> (1 << order))
    {}

    ~AppleFFT() override
    {
        if (fftSetup != nullptr)
        {
            vDSP_destroy_fftsetup (fftSetup);
            fftSetup = nullptr;
        }
    }

    void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept override
    {
        auto size = (1 << order);

        DSPSplitComplex splitInput  (toSplitComplex (const_cast<Complex<float>*> (input)));
        DSPSplitComplex splitOutput (toSplitComplex (output));

        vDSP_fft_zop (fftSetup, &splitInput,  2, &splitOutput, 2,
                      order, inverse ?  kFFTDirection_Inverse : kFFTDirection_Forward);

        float factor = (inverse ? inverseNormalisation : forwardNormalisation * 2.0f);
        vDSP_vsmul ((float*) output, 1, &factor, (float*) output, 1, static_cast<size_t> (size << 1));
    }

    void performRealOnlyForwardTransform (float* inoutData, bool ignoreNegativeFreqs) const noexcept override
    {
        auto size = (1 << order);
        auto* inout = reinterpret_cast<Complex<float>*> (inoutData);
        auto splitInOut (toSplitComplex (inout));

        inoutData[size] = 0.0f;
        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Forward);
        vDSP_vsmul (inoutData, 1, &forwardNormalisation, inoutData, 1, static_cast<size_t> (size << 1));

        mirrorResult (inout, ignoreNegativeFreqs);
    }

    void performRealOnlyInverseTransform (float* inoutData) const noexcept override
    {
        auto* inout = reinterpret_cast<Complex<float>*> (inoutData);
        auto size = (1 << order);
        auto splitInOut (toSplitComplex (inout));

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        if (size != 1)
            inout[0] = Complex<float> (inout[0].real(), inout[size >> 1].real());

        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Inverse);
        vDSP_vsmul (inoutData, 1, &inverseNormalisation, inoutData, 1, static_cast<size_t> (size << 1));
        vDSP_vclr (inoutData + size, 1, static_cast<size_t> (size));
    }

private:
    //==============================================================================
    void mirrorResult (Complex<float>* out, bool ignoreNegativeFreqs) const noexcept
    {
        auto size = (1 << order);
        auto i = size >> 1;

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        out[i++] = { out[0].imag(), 0.0 };
        out[0]   = { out[0].real(), 0.0 };

        if (! ignoreNegativeFreqs)
            for (; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    static DSPSplitComplex toSplitComplex (Complex<float>* data) noexcept
    {
        // this assumes that Complex interleaves real and imaginary parts
        // and is tightly packed.
        return { reinterpret_cast<float*> (data),
                 reinterpret_cast<float*> (data) + 1};
    }

    //==============================================================================
    vDSP_Length order;
    FFTSetup fftSetup;
    float forwardNormalisation, inverseNormalisation;
};

FFT::EngineImpl<AppleFFT> appleFFT;
#endif

//==============================================================================
//==============================================================================
#if JUCE_DSP_USE_SHARED_FFTW || JUCE_DSP_USE_STATIC_FFTW

#if JUCE_DSP_USE_STATIC_FFTW
extern "C"
{
    void* fftwf_plan_dft_1d     (int, void*, void*, int, int);
    void* fftwf_plan_dft_r2c_1d (int, void*, void*, int);
    void* fftwf_plan_dft_c2r_1d (int, void*, void*, int);
    void fftwf_destroy_plan     (void*);
    void fftwf_execute_dft      (void*, void*, void*);
    void fftwf_execute_dft_r2c  (void*, void*, void*);
    void fftwf_execute_dft_c2r  (void*, void*, void*);
}
#endif

struct FFTWImpl  : public FFT::Instance
{
   #if JUCE_DSP_USE_STATIC_FFTW
    // if the JUCE developer has gone through the hassle of statically
    // linking in fftw, they probably want to use it
    static constexpr int priority = 10;
   #else
    static constexpr int priority = 3;
   #endif

    struct FFTWPlan;
    using FFTWPlanRef = FFTWPlan*;

    enum
    {
        measure   = 0,
        unaligned = (1 << 1),
        estimate  = (1 << 6)
    };

    struct Symbols
    {
        FFTWPlanRef (*plan_dft_fftw) (unsigned, Complex<float>*, Complex<float>*, int, unsigned);
        FFTWPlanRef (*plan_r2c_fftw) (unsigned, float*, Complex<float>*, unsigned);
        FFTWPlanRef (*plan_c2r_fftw) (unsigned, Complex<float>*, float*, unsigned);
        void (*destroy_fftw) (FFTWPlanRef);

        void (*execute_dft_fftw) (FFTWPlanRef, const Complex<float>*, Complex<float>*);
        void (*execute_r2c_fftw) (FFTWPlanRef, float*, Complex<float>*);
        void (*execute_c2r_fftw) (FFTWPlanRef, Complex<float>*, float*);

       #if JUCE_DSP_USE_STATIC_FFTW
        template <typename FuncPtr, typename ActualSymbolType>
        static bool symbol (FuncPtr& dst, ActualSymbolType sym)
        {
            dst = reinterpret_cast<FuncPtr> (sym);
            return true;
        }
       #else
        template <typename FuncPtr>
        static bool symbol (DynamicLibrary& lib, FuncPtr& dst, const char* name)
        {
            dst = reinterpret_cast<FuncPtr> (lib.getFunction (name));
            return (dst != nullptr);
        }
       #endif
    };

    static FFTWImpl* create (int order)
    {
        DynamicLibrary lib;

      #if ! JUCE_DSP_USE_STATIC_FFTW
       #if JUCE_MAC
        auto libName = "libfftw3f.dylib";
       #elif JUCE_WINDOWS
        auto libName = "libfftw3f.dll";
       #else
        auto libName = "libfftw3f.so";
       #endif

        if (lib.open (libName))
      #endif
        {
            Symbols symbols;

           #if JUCE_DSP_USE_STATIC_FFTW
            if (! Symbols::symbol (symbols.plan_dft_fftw, fftwf_plan_dft_1d))     return nullptr;
            if (! Symbols::symbol (symbols.plan_r2c_fftw, fftwf_plan_dft_r2c_1d)) return nullptr;
            if (! Symbols::symbol (symbols.plan_c2r_fftw, fftwf_plan_dft_c2r_1d)) return nullptr;
            if (! Symbols::symbol (symbols.destroy_fftw,  fftwf_destroy_plan))    return nullptr;

            if (! Symbols::symbol (symbols.execute_dft_fftw, fftwf_execute_dft))     return nullptr;
            if (! Symbols::symbol (symbols.execute_r2c_fftw, fftwf_execute_dft_r2c)) return nullptr;
            if (! Symbols::symbol (symbols.execute_c2r_fftw, fftwf_execute_dft_c2r)) return nullptr;
           #else
            if (! Symbols::symbol (lib, symbols.plan_dft_fftw, "fftwf_plan_dft_1d"))     return nullptr;
            if (! Symbols::symbol (lib, symbols.plan_r2c_fftw, "fftwf_plan_dft_r2c_1d")) return nullptr;
            if (! Symbols::symbol (lib, symbols.plan_c2r_fftw, "fftwf_plan_dft_c2r_1d")) return nullptr;
            if (! Symbols::symbol (lib, symbols.destroy_fftw,  "fftwf_destroy_plan"))    return nullptr;

            if (! Symbols::symbol (lib, symbols.execute_dft_fftw, "fftwf_execute_dft"))     return nullptr;
            if (! Symbols::symbol (lib, symbols.execute_r2c_fftw, "fftwf_execute_dft_r2c")) return nullptr;
            if (! Symbols::symbol (lib, symbols.execute_c2r_fftw, "fftwf_execute_dft_c2r")) return nullptr;
           #endif

            return new FFTWImpl (static_cast<size_t> (order), static_cast<DynamicLibrary&&> (lib), symbols);
        }

        return nullptr;
    }

    FFTWImpl (size_t orderToUse, DynamicLibrary&& libraryToUse, const Symbols& symbols)
        : fftwLibrary (std::move (libraryToUse)), fftw (symbols), order (static_cast<size_t> (orderToUse))
    {
        ScopedLock lock (getFFTWPlanLock());

        auto n = (1u << order);
        HeapBlock<Complex<float>> in (n), out (n);

        c2cForward = fftw.plan_dft_fftw (n, in.getData(), out.getData(), -1, unaligned | estimate);
        c2cInverse = fftw.plan_dft_fftw (n, in.getData(), out.getData(), +1, unaligned | estimate);

        r2c = fftw.plan_r2c_fftw (n, (float*) in.getData(), in.getData(), unaligned | estimate);
        c2r = fftw.plan_c2r_fftw (n, in.getData(), (float*) in.getData(), unaligned | estimate);
    }

    ~FFTWImpl() override
    {
        ScopedLock lock (getFFTWPlanLock());

        fftw.destroy_fftw (c2cForward);
        fftw.destroy_fftw (c2cInverse);
        fftw.destroy_fftw (r2c);
        fftw.destroy_fftw (c2r);
    }

    void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept override
    {
        if (inverse)
        {
            auto n = (1u << order);
            fftw.execute_dft_fftw (c2cInverse, input, output);
            FloatVectorOperations::multiply ((float*) output, 1.0f / static_cast<float> (n), (int) n << 1);
        }
        else
        {
            fftw.execute_dft_fftw (c2cForward, input, output);
        }
    }

    void performRealOnlyForwardTransform (float* inputOutputData, bool ignoreNegativeFreqs) const noexcept override
    {
        if (order == 0)
            return;

        auto* out = reinterpret_cast<Complex<float>*> (inputOutputData);

        fftw.execute_r2c_fftw (r2c, inputOutputData, out);

        auto size = (1 << order);

        if (! ignoreNegativeFreqs)
            for (auto i = size >> 1; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    void performRealOnlyInverseTransform (float* inputOutputData) const noexcept override
    {
        auto n = (1u << order);

        fftw.execute_c2r_fftw (c2r, (Complex<float>*) inputOutputData, inputOutputData);
        FloatVectorOperations::multiply ((float*) inputOutputData, 1.0f / static_cast<float> (n), (int) n);
    }

    //==============================================================================
    // fftw's plan_* and destroy_* methods are NOT thread safe. So we need to share
    // a lock between all instances of FFTWImpl
    static CriticalSection& getFFTWPlanLock() noexcept
    {
        static CriticalSection cs;
        return cs;
    }

    //==============================================================================
    DynamicLibrary fftwLibrary;
    Symbols fftw;
    size_t order;

    FFTWPlanRef c2cForward, c2cInverse, r2c, c2r;
};

FFT::EngineImpl<FFTWImpl> fftwEngine;
#endif

//==============================================================================
//==============================================================================
#if JUCE_DSP_USE_INTEL_MKL
struct IntelFFT  : public FFT::Instance
{
    static constexpr int priority = 8;

    static bool succeeded (MKL_LONG status) noexcept        { return status == 0; }

    static IntelFFT* create (int orderToUse)
    {
        DFTI_DESCRIPTOR_HANDLE mklc2c, mklc2r;

        if (DftiCreateDescriptor (&mklc2c, DFTI_SINGLE, DFTI_COMPLEX, 1, 1 << orderToUse) == 0)
        {
            if (succeeded (DftiSetValue (mklc2c, DFTI_PLACEMENT, DFTI_NOT_INPLACE))
                 && succeeded (DftiSetValue (mklc2c, DFTI_BACKWARD_SCALE, 1.0f / static_cast<float> (1 << orderToUse)))
                 && succeeded (DftiCommitDescriptor (mklc2c)))
            {
                if (succeeded (DftiCreateDescriptor (&mklc2r, DFTI_SINGLE, DFTI_REAL, 1, 1 << orderToUse)))
                {
                    if (succeeded (DftiSetValue (mklc2r, DFTI_PLACEMENT, DFTI_INPLACE))
                         && succeeded (DftiSetValue (mklc2r, DFTI_BACKWARD_SCALE, 1.0f / static_cast<float> (1 << orderToUse)))
                         && succeeded (DftiCommitDescriptor (mklc2r)))
                    {
                        return new IntelFFT (static_cast<size_t> (orderToUse), mklc2c, mklc2r);
                    }

                    DftiFreeDescriptor (&mklc2r);
                }
            }

            DftiFreeDescriptor (&mklc2c);
        }

        return {};
    }

    IntelFFT (size_t orderToUse, DFTI_DESCRIPTOR_HANDLE c2cToUse, DFTI_DESCRIPTOR_HANDLE cr2ToUse)
        : order (orderToUse), c2c (c2cToUse), c2r (cr2ToUse)
    {}

    ~IntelFFT()
    {
        DftiFreeDescriptor (&c2c);
        DftiFreeDescriptor (&c2r);
    }

    void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept override
    {
        if (inverse)
            DftiComputeBackward (c2c, (void*) input, output);
        else
            DftiComputeForward (c2c, (void*) input, output);
    }

    void performRealOnlyForwardTransform (float* inputOutputData, bool ignoreNegativeFreqs) const noexcept override
    {
        if (order == 0)
            return;

        DftiComputeForward (c2r, inputOutputData);

        auto* out = reinterpret_cast<Complex<float>*> (inputOutputData);
        auto size = (1 << order);

        if (! ignoreNegativeFreqs)
            for (auto i = size >> 1; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    void performRealOnlyInverseTransform (float* inputOutputData) const noexcept override
    {
        DftiComputeBackward (c2r, inputOutputData);
    }

    size_t order;
    DFTI_DESCRIPTOR_HANDLE c2c, c2r;
};

FFT::EngineImpl<IntelFFT> fftwEngine;
#endif

//==============================================================================
//==============================================================================
FFT::FFT (int order)
    : engine (FFT::Engine::createBestEngineForPlatform (order)),
      size (1 << order)
{
}

FFT::~FFT() {}

void FFT::perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept
{
    if (engine != nullptr)
        engine->perform (input, output, inverse);
}

void FFT::performRealOnlyForwardTransform (float* inputOutputData, bool ignoreNeagtiveFreqs) const noexcept
{
    if (engine != nullptr)
        engine->performRealOnlyForwardTransform (inputOutputData, ignoreNeagtiveFreqs);
}

void FFT::performRealOnlyInverseTransform (float* inputOutputData) const noexcept
{
    if (engine != nullptr)
        engine->performRealOnlyInverseTransform (inputOutputData);
}

void FFT::performFrequencyOnlyForwardTransform (float* inputOutputData) const noexcept
{
    if (size == 1)
        return;

    performRealOnlyForwardTransform (inputOutputData);
    auto* out = reinterpret_cast<Complex<float>*> (inputOutputData);

    for (auto i = 0; i < size; ++i)
        inputOutputData[i] = std::abs (out[i]);

    zeromem (&inputOutputData[size], sizeof (float) * static_cast<size_t> (size));
}

} // namespace dsp
} // namespace juce
