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
namespace IIR
{

#ifndef DOXYGEN

//==============================================================================
template <typename SampleType>
Filter<SampleType>::Filter()
    : coefficients (new Coefficients<typename Filter<SampleType>::NumericType> (1, 0, 1, 0))
{
    reset();
}

template <typename SampleType>
Filter<SampleType>::Filter (CoefficientsPtr c)  : coefficients (static_cast<CoefficientsPtr&&> (c))
{
    reset();
}

template <typename SampleType>
void Filter<SampleType>::reset (SampleType resetToValue)
{
    auto newOrder = coefficients->getFilterOrder();

    if (newOrder != order)
    {
        memory.malloc (jmax (order, newOrder, static_cast<size_t> (3)) + 1);
        state = snapPointerToAlignment (memory.getData(), sizeof (SampleType));
        order = newOrder;
    }

    for (size_t i = 0; i < order; ++i)
        state[i] = resetToValue;
}

template <typename SampleType>
void Filter<SampleType>::prepare (const ProcessSpec&) noexcept     { reset(); }


template <typename SampleType>
template <typename ProcessContext, bool bypassed>
void Filter<SampleType>::processInternal (const ProcessContext& context) noexcept
{
    static_assert (std::is_same<typename ProcessContext::SampleType, SampleType>::value,
                   "The sample-type of the IIR filter must match the sample-type supplied to this process callback");
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
    auto* coeffs = coefficients->getRawCoefficients();

    // we need to copy this template parameter into a constexpr
    // otherwise MSVC will moan that the tenary expressions below
    // are constant conditional expressions
    constexpr bool isBypassed = bypassed;

    switch (order)
    {
        case 1:
        {
            auto b0 = coeffs[0];
            auto b1 = coeffs[1];
            auto a1 = coeffs[2];

            auto lv1 = state[0];

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto in = src[i];
                auto out = in * b0 + lv1;

                dst[i] = isBypassed ? in : out;

                lv1 = (in * b1) - (out * a1);
            }

            util::snapToZero (lv1); state[0] = lv1;
        }
        break;

        case 2:
        {
            auto b0 = coeffs[0];
            auto b1 = coeffs[1];
            auto b2 = coeffs[2];
            auto a1 = coeffs[3];
            auto a2 = coeffs[4];

            auto lv1 = state[0];
            auto lv2 = state[1];

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto in = src[i];
                auto out = (in * b0) + lv1;
                dst[i] = isBypassed ? in : out;

                lv1 = (in * b1) - (out * a1) + lv2;
                lv2 = (in * b2) - (out * a2);
            }

            util::snapToZero (lv1); state[0] = lv1;
            util::snapToZero (lv2); state[1] = lv2;
        }
        break;

        case 3:
        {
            auto b0 = coeffs[0];
            auto b1 = coeffs[1];
            auto b2 = coeffs[2];
            auto b3 = coeffs[3];
            auto a1 = coeffs[4];
            auto a2 = coeffs[5];
            auto a3 = coeffs[6];

            auto lv1 = state[0];
            auto lv2 = state[1];
            auto lv3 = state[2];

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto in = src[i];
                auto out = (in * b0) + lv1;
                dst[i] = isBypassed ? in : out;

                lv1 = (in * b1) - (out * a1) + lv2;
                lv2 = (in * b2) - (out * a2) + lv3;
                lv3 = (in * b3) - (out * a3);
            }

            util::snapToZero (lv1); state[0] = lv1;
            util::snapToZero (lv2); state[1] = lv2;
            util::snapToZero (lv3); state[2] = lv3;
        }
        break;

        default:
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                auto in = src[i];
                auto out = (in * coeffs[0]) + state[0];
                dst[i] = isBypassed ? in : out;

                for (size_t j = 0; j < order - 1; ++j)
                    state[j] = (in * coeffs[j + 1]) - (out * coeffs[order + j + 1]) + state[j + 1];

                state[order - 1] = (in * coeffs[order]) - (out * coeffs[order * 2]);
            }

            snapToZero();
        }
    }
}

template <typename SampleType>
SampleType JUCE_VECTOR_CALLTYPE Filter<SampleType>::processSample (SampleType sample) noexcept
{
    check();
    auto* c = coefficients->getRawCoefficients();

    auto out = (c[0] * sample) + state[0];

    for (size_t j = 0; j < order - 1; ++j)
        state[j] = (c[j + 1] * sample) - (c[order + j + 1] * out) + state[j + 1];

    state[order - 1] = (c[order] * sample) - (c[order * 2] * out);

    return out;
}

template <typename SampleType>
void Filter<SampleType>::snapToZero() noexcept
{
    for (size_t i = 0; i < order; ++i)
        util::snapToZero (state[i]);
}

template <typename SampleType>
void Filter<SampleType>::check()
{
    jassert (coefficients != nullptr);

    if (order != coefficients->getFilterOrder())
        reset();
}

#endif

} // namespace IIR
} // namespace dsp
} // namespace juce
