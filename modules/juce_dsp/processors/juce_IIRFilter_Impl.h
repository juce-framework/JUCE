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

namespace juce
{
namespace dsp
{
namespace IIR
{

#ifndef DOXYGEN

template <typename NumericType>
template <size_t Num>
Coefficients<NumericType>& Coefficients<NumericType>::assignImpl (const NumericType* values)
{
    static_assert (Num % 2 == 0, "Must supply an even number of coefficients");
    const auto a0Index = Num / 2;
    const auto a0 = values[a0Index];
    const auto a0Inv = ! approximatelyEqual (a0, NumericType())
                     ? static_cast<NumericType> (1) / values[a0Index]
                     : NumericType();

    coefficients.clearQuick();
    coefficients.ensureStorageAllocated ((int) jmax ((size_t) 8, Num));

    for (size_t i = 0; i < Num; ++i)
        if (i != a0Index)
            coefficients.add (values[i] * a0Inv);

    return *this;
}

//==============================================================================
template <typename SampleType>
Filter<SampleType>::Filter()
    : coefficients (new Coefficients<typename Filter<SampleType>::NumericType> (1, 0, 1, 0))
{
    reset();
}

template <typename SampleType>
Filter<SampleType>::Filter (CoefficientsPtr c)  : coefficients (std::move (c))
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
    static_assert (std::is_same_v<typename ProcessContext::SampleType, SampleType>,
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
                auto input = src[i];
                auto output = input * b0 + lv1;

                dst[i] = bypassed ? input : output;

                lv1 = (input * b1) - (output * a1);
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
                auto input = src[i];
                auto output = (input * b0) + lv1;
                dst[i] = bypassed ? input : output;

                lv1 = (input * b1) - (output* a1) + lv2;
                lv2 = (input * b2) - (output* a2);
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
                auto input = src[i];
                auto output = (input * b0) + lv1;
                dst[i] = bypassed ? input : output;

                lv1 = (input * b1) - (output* a1) + lv2;
                lv2 = (input * b2) - (output* a2) + lv3;
                lv3 = (input * b3) - (output* a3);
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
                auto input = src[i];
                auto output= (input * coeffs[0]) + state[0];
                dst[i] = bypassed ? input : output;

                for (size_t j = 0; j < order - 1; ++j)
                    state[j] = (input * coeffs[j + 1]) - (output* coeffs[order + j + 1]) + state[j + 1];

                state[order - 1] = (input * coeffs[order]) - (output* coeffs[order * 2]);
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

    auto output = (c[0] * sample) + state[0];

    for (size_t j = 0; j < order - 1; ++j)
        state[j] = (c[j + 1] * sample) - (c[order + j + 1] * output) + state[j + 1];

    state[order - 1] = (c[order] * sample) - (c[order * 2] * output);

    return output;
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
