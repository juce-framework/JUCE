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

/**
    Classes for FIR filter processing.
*/
namespace juce::dsp::FIR
{
    template <typename NumericType>
    struct Coefficients;

    //==============================================================================
    /**
        A processing class that can perform FIR filtering on an audio signal, in the
        time domain.

        Using FIRFilter is fast enough for FIRCoefficients with a size lower than 128
        samples. For longer filters, it might be more efficient to use the class
        Convolution instead, which does the same processing in the frequency domain
        thanks to FFT.

        @see FIRFilter::Coefficients, Convolution, FFT

        @tags{DSP}
    */
    template <typename SampleType>
    class Filter
    {
    public:
        /** The NumericType is the underlying primitive type used by the SampleType (which
            could be either a primitive or vector)
        */
        using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

        /** A typedef for a ref-counted pointer to the coefficients object */
        using CoefficientsPtr = typename Coefficients<NumericType>::Ptr;

        //==============================================================================
        /** This will create a filter which will produce silence. */
        Filter() : coefficients (new Coefficients<NumericType>)                                     { reset(); }

        /** Creates a filter with a given set of coefficients. */
        Filter (CoefficientsPtr coefficientsToUse)  : coefficients (std::move (coefficientsToUse))   { reset(); }

        Filter (const Filter&) = default;
        Filter (Filter&&) = default;
        Filter& operator= (const Filter&) = default;
        Filter& operator= (Filter&&) = default;

        //==============================================================================
        /** Prepare this filter for processing. */
        inline void prepare ([[maybe_unused]] const ProcessSpec& spec) noexcept
        {
            // This class can only process mono signals. Use the ProcessorDuplicator class
            // to apply this filter on a multi-channel audio stream.
            jassert (spec.numChannels == 1);
            reset();
        }

        /** Resets the filter's processing pipeline, ready to start a new stream of data.

            Note that this clears the processing state, but the type of filter and
            its coefficients aren't changed. To disable the filter, call setEnabled (false).
        */
        void reset()
        {
            if (coefficients != nullptr)
            {
                auto newSize = coefficients->getFilterOrder() + 1;

                if (newSize != size)
                {
                    memory.malloc (1 + jmax (newSize, size, static_cast<size_t> (128)));

                    fifo = snapPointerToAlignment (memory.getData(), sizeof (SampleType));
                    size = newSize;
                }

                for (size_t i = 0; i < size; ++i)
                    fifo[i] = SampleType {0};
            }
        }

        //==============================================================================
        /** The coefficients of the FIR filter. It's up to the caller to ensure that
            these coefficients are modified in a thread-safe way.

            If you change the order of the coefficients then you must call reset after
            modifying them.
        */
        typename Coefficients<NumericType>::Ptr coefficients;

        //==============================================================================
        /** Processes a block of samples */
        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept
        {
            static_assert (std::is_same_v<typename ProcessContext::SampleType, SampleType>,
                           "The sample-type of the FIR filter must match the sample-type supplied to this process callback");
            check();

            auto&& inputBlock  = context.getInputBlock();
            auto&& outputBlock = context.getOutputBlock();

            // This class can only process mono signals. Use the ProcessorDuplicator class
            // to apply this filter on a multi-channel audio stream.
            jassert (inputBlock.getNumChannels()  == 1);
            jassert (outputBlock.getNumChannels() == 1);

            auto numSamples = inputBlock.getNumSamples();
            auto* src = inputBlock .getChannelPointer (0);
            auto* dst = outputBlock.getChannelPointer (0);

            auto* fir = coefficients->getRawCoefficients();
            size_t p = pos;

            if (context.isBypassed)
            {
                for (size_t i = 0; i < numSamples; ++i)
                {
                    fifo[p] = dst[i] = src[i];
                    p = (p == 0 ? size - 1 : p - 1);
                }
            }
            else
            {
                for (size_t i = 0; i < numSamples; ++i)
                    dst[i] = processSingleSample (src[i], fifo, fir, size, p);
            }

            pos = p;
        }


        /** Processes a single sample, without any locking.
            Use this if you need processing of a single value.
        */
        SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType sample) noexcept
        {
            check();
            return processSingleSample (sample, fifo, coefficients->getRawCoefficients(), size, pos);
        }

    private:
        //==============================================================================
        HeapBlock<SampleType> memory;
        SampleType* fifo = nullptr;
        size_t pos = 0, size = 0;

        //==============================================================================
        void check()
        {
            jassert (coefficients != nullptr);

            if (size != (coefficients->getFilterOrder() + 1))
                reset();
        }

        static SampleType JUCE_VECTOR_CALLTYPE processSingleSample (SampleType sample, SampleType* buf,
                                                                    const NumericType* fir, size_t m, size_t& p) noexcept
        {
            SampleType out (0);

            buf[p] = sample;

            size_t k;
            for (k = 0; k < m - p; ++k)
                out += buf[(p + k)] * fir[k];

            for (size_t j = 0; j < p; ++j)
                out += buf[j] * fir[j + k];

            p = (p == 0 ? m - 1 : p - 1);

            return out;
        }


        JUCE_LEAK_DETECTOR (Filter)
    };

    //==============================================================================
    /**
        A set of coefficients for use in an FIRFilter object.

        @see FIRFilter

        @tags{DSP}
    */
    template <typename NumericType>
    struct Coefficients  : public ProcessorState
    {
        //==============================================================================
        /** Creates a null set of coefficients (which will produce silence). */
        Coefficients()  : coefficients ({ NumericType() }) {}

        /** Creates a null set of coefficients of a given size. */
        Coefficients (size_t size)    { coefficients.resize ((int) size); }

        /** Creates a set of coefficients from an array of samples. */
        Coefficients (const NumericType* samples, size_t numSamples)   : coefficients (samples, (int) numSamples) {}

        Coefficients (const Coefficients&) = default;
        Coefficients (Coefficients&&) = default;
        Coefficients& operator= (const Coefficients&) = default;
        Coefficients& operator= (Coefficients&&) = default;

        /** The Coefficients structure is ref-counted, so this is a handy type that can be used
            as a pointer to one.
        */
        using Ptr = ReferenceCountedObjectPtr<Coefficients>;

        //==============================================================================
        /** Returns the filter order associated with the coefficients. */
        size_t getFilterOrder() const noexcept  { return static_cast<size_t> (coefficients.size()) - 1; }

        /** Returns the magnitude frequency response of the filter for a given frequency
            and sample rate.
        */
        double getMagnitudeForFrequency (double frequency, double sampleRate) const noexcept;

        /** Returns the magnitude frequency response of the filter for a given frequency array
            and sample rate.
        */
        void getMagnitudeForFrequencyArray (double* frequencies, double* magnitudes,
                                            size_t numSamples, double sampleRate) const noexcept;

        /** Returns the phase frequency response of the filter for a given frequency and
            sample rate.
        */
        double getPhaseForFrequency (double frequency, double sampleRate) const noexcept;

        /** Returns the phase frequency response of the filter for a given frequency array
            and sample rate.
        */
        void getPhaseForFrequencyArray (double* frequencies, double* phases,
                                        size_t numSamples, double sampleRate) const noexcept;

        /** Returns a raw data pointer to the coefficients. */
        NumericType* getRawCoefficients() noexcept              { return coefficients.getRawDataPointer(); }

        /** Returns a raw data pointer to the coefficients. */
        const NumericType* getRawCoefficients() const noexcept  { return coefficients.begin(); }

        //==============================================================================
        /** Scales the values of the FIR filter with the sum of the squared coefficients. */
        void normalise() noexcept;

        //==============================================================================
        /** The raw coefficients.
            You should leave these numbers alone unless you really know what you're doing.
        */
        Array<NumericType> coefficients;
    };

} // namespace juce::dsp::FIR
