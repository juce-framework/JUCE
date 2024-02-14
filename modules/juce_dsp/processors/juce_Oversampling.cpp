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

/** Abstract class for the provided oversampling stages used internally in
    the Oversampling class.
*/
template <typename SampleType>
struct Oversampling<SampleType>::OversamplingStage
{
    OversamplingStage (size_t numChans, size_t newFactor)  : numChannels (numChans), factor (newFactor) {}
    virtual ~OversamplingStage() {}

    //==============================================================================
    virtual SampleType getLatencyInSamples() const = 0;

    virtual void initProcessing (size_t maximumNumberOfSamplesBeforeOversampling)
    {
        buffer.setSize (static_cast<int> (numChannels),
                        static_cast<int> (maximumNumberOfSamplesBeforeOversampling * factor),
                        false, false, true);
    }

    virtual void reset()
    {
        buffer.clear();
    }

    AudioBlock<SampleType> getProcessedSamples (size_t numSamples)
    {
        return AudioBlock<SampleType> (buffer).getSubBlock (0, numSamples);
    }

    virtual void processSamplesUp   (const AudioBlock<const SampleType>&) = 0;
    virtual void processSamplesDown (AudioBlock<SampleType>&) = 0;

    AudioBuffer<SampleType> buffer;
    size_t numChannels, factor;
};


//==============================================================================
/** Dummy oversampling stage class which simply copies and pastes the input
    signal, which could be equivalent to a "one time" oversampling processing.
*/
template <typename SampleType>
struct OversamplingDummy final : public Oversampling<SampleType>::OversamplingStage
{
    using ParentType = typename Oversampling<SampleType>::OversamplingStage;

    OversamplingDummy (size_t numChans) : ParentType (numChans, 1) {}

    //==============================================================================
    SampleType getLatencyInSamples() const override
    {
        return 0;
    }

    void processSamplesUp (const AudioBlock<const SampleType>& inputBlock) override
    {
        jassert (inputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (inputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
            ParentType::buffer.copyFrom (static_cast<int> (channel), 0,
                inputBlock.getChannelPointer (channel), static_cast<int> (inputBlock.getNumSamples()));
    }

    void processSamplesDown (AudioBlock<SampleType>& outputBlock) override
    {
        jassert (outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        outputBlock.copyFrom (ParentType::getProcessedSamples (outputBlock.getNumSamples()));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingDummy)
};

//==============================================================================
/** Oversampling stage class performing 2 times oversampling using the Filter
    Design FIR Equiripple method. The resulting filter is linear phase,
    symmetric, and has every two samples but the middle one equal to zero,
    leading to specific processing optimizations.
*/
template <typename SampleType>
struct Oversampling2TimesEquirippleFIR final : public Oversampling<SampleType>::OversamplingStage
{
    using ParentType = typename Oversampling<SampleType>::OversamplingStage;

    Oversampling2TimesEquirippleFIR (size_t numChans,
                                     SampleType normalisedTransitionWidthUp,
                                     SampleType stopbandAmplitudedBUp,
                                     SampleType normalisedTransitionWidthDown,
                                     SampleType stopbandAmplitudedBDown)
        : ParentType (numChans, 2)
    {
        coefficientsUp   = *FilterDesign<SampleType>::designFIRLowpassHalfBandEquirippleMethod (normalisedTransitionWidthUp,   stopbandAmplitudedBUp);
        coefficientsDown = *FilterDesign<SampleType>::designFIRLowpassHalfBandEquirippleMethod (normalisedTransitionWidthDown, stopbandAmplitudedBDown);

        auto N = coefficientsUp.getFilterOrder() + 1;
        stateUp.setSize (static_cast<int> (this->numChannels), static_cast<int> (N));

        N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;

        stateDown.setSize  (static_cast<int> (this->numChannels), static_cast<int> (N));
        stateDown2.setSize (static_cast<int> (this->numChannels), static_cast<int> (Ndiv4 + 1));

        position.resize (static_cast<int> (this->numChannels));
    }

    //==============================================================================
    SampleType getLatencyInSamples() const override
    {
        return static_cast<SampleType> (coefficientsUp.getFilterOrder() + coefficientsDown.getFilterOrder()) * 0.5f;
    }

    void reset() override
    {
        ParentType::reset();

        stateUp.clear();
        stateDown.clear();
        stateDown2.clear();

        position.fill (0);
    }

    void processSamplesUp (const AudioBlock<const SampleType>& inputBlock) override
    {
        jassert (inputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (inputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto fir = coefficientsUp.getRawCoefficients();
        auto N = coefficientsUp.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto numSamples = inputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto buf = stateUp.getWritePointer (static_cast<int> (channel));
            auto samples = inputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Input
                buf[N - 1] = 2 * samples[i];

                // Convolution
                auto out = static_cast<SampleType> (0.0);

                for (size_t k = 0; k < Ndiv2; k += 2)
                    out += (buf[k] + buf[N - k - 1]) * fir[k];

                // Outputs
                bufferSamples[i << 1] = out;
                bufferSamples[(i << 1) + 1] = buf[Ndiv2 + 1] * fir[Ndiv2];

                // Shift data
                for (size_t k = 0; k < N - 2; k += 2)
                    buf[k] = buf[k + 2];
            }
        }
    }

    void processSamplesDown (AudioBlock<SampleType>& outputBlock) override
    {
        jassert (outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto fir = coefficientsDown.getRawCoefficients();
        auto N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;
        auto numSamples = outputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < outputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto buf = stateDown.getWritePointer (static_cast<int> (channel));
            auto buf2 = stateDown2.getWritePointer (static_cast<int> (channel));
            auto samples = outputBlock.getChannelPointer (channel);
            auto pos = position.getUnchecked (static_cast<int> (channel));

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Input
                buf[N - 1] = bufferSamples[i << 1];

                // Convolution
                auto out = static_cast<SampleType> (0.0);

                for (size_t k = 0; k < Ndiv2; k += 2)
                    out += (buf[k] + buf[N - k - 1]) * fir[k];

                // Output
                out += buf2[pos] * fir[Ndiv2];
                buf2[pos] = bufferSamples[(i << 1) + 1];

                samples[i] = out;

                // Shift data
                for (size_t k = 0; k < N - 2; ++k)
                    buf[k] = buf[k + 2];

                // Circular buffer
                pos = (pos == 0 ? Ndiv4 : pos - 1);
            }

            position.setUnchecked (static_cast<int> (channel), pos);
        }

    }

private:
    //==============================================================================
    FIR::Coefficients<SampleType> coefficientsUp, coefficientsDown;
    AudioBuffer<SampleType> stateUp, stateDown, stateDown2;
    Array<size_t> position;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling2TimesEquirippleFIR)
};


//==============================================================================
/** Oversampling stage class performing 2 times oversampling using the Filter
    Design IIR Polyphase Allpass Cascaded method. The resulting filter is minimum
    phase, and provided with a method to get the exact resulting latency.
*/
template <typename SampleType>
struct Oversampling2TimesPolyphaseIIR final : public Oversampling<SampleType>::OversamplingStage
{
    using ParentType = typename Oversampling<SampleType>::OversamplingStage;

    Oversampling2TimesPolyphaseIIR (size_t numChans,
                                    SampleType normalisedTransitionWidthUp,
                                    SampleType stopbandAmplitudedBUp,
                                    SampleType normalisedTransitionWidthDown,
                                    SampleType stopbandAmplitudedBDown)
        : ParentType (numChans, 2)
    {
        auto structureUp = FilterDesign<SampleType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (normalisedTransitionWidthUp, stopbandAmplitudedBUp);
        auto coeffsUp = getCoefficients (structureUp);
        latency = static_cast<SampleType> (-(coeffsUp.getPhaseForFrequency (0.0001, 1.0)) / (0.0001 * MathConstants<double>::twoPi));

        auto structureDown = FilterDesign<SampleType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (normalisedTransitionWidthDown, stopbandAmplitudedBDown);
        auto coeffsDown = getCoefficients (structureDown);
        latency += static_cast<SampleType> (-(coeffsDown.getPhaseForFrequency (0.0001, 1.0)) / (0.0001 * MathConstants<double>::twoPi));

        for (auto i = 0; i < structureUp.directPath.size(); ++i)
            coefficientsUp.add (structureUp.directPath.getObjectPointer (i)->coefficients[0]);

        for (auto i = 1; i < structureUp.delayedPath.size(); ++i)
            coefficientsUp.add (structureUp.delayedPath.getObjectPointer (i)->coefficients[0]);

        for (auto i = 0; i < structureDown.directPath.size(); ++i)
            coefficientsDown.add (structureDown.directPath.getObjectPointer (i)->coefficients[0]);

        for (auto i = 1; i < structureDown.delayedPath.size(); ++i)
            coefficientsDown.add (structureDown.delayedPath.getObjectPointer (i)->coefficients[0]);

        v1Up.setSize   (static_cast<int> (this->numChannels), coefficientsUp.size());
        v1Down.setSize (static_cast<int> (this->numChannels), coefficientsDown.size());
        delayDown.resize (static_cast<int> (this->numChannels));
    }

    //==============================================================================
    SampleType getLatencyInSamples() const override
    {
        return latency;
    }

    void reset() override
    {
        ParentType::reset();
        v1Up.clear();
        v1Down.clear();
        delayDown.fill (0);
    }

    void processSamplesUp (const AudioBlock<const SampleType>& inputBlock) override
    {
        jassert (inputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (inputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto coeffs = coefficientsUp.getRawDataPointer();
        auto numStages = coefficientsUp.size();
        auto delayedStages = numStages / 2;
        auto directStages = numStages - delayedStages;
        auto numSamples = inputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto lv1 = v1Up.getWritePointer (static_cast<int> (channel));
            auto samples = inputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Direct path cascaded allpass filters
                auto input = samples[i];

                for (auto n = 0; n < directStages; ++n)
                {
                    auto alpha = coeffs[n];
                    auto output = alpha * input + lv1[n];
                    lv1[n] = input - alpha * output;
                    input = output;
                }

                // Output
                bufferSamples[i << 1] = input;

                // Delayed path cascaded allpass filters
                input = samples[i];

                for (auto n = directStages; n < numStages; ++n)
                {
                    auto alpha = coeffs[n];
                    auto output = alpha * input + lv1[n];
                    lv1[n] = input - alpha * output;
                    input = output;
                }

                // Output
                bufferSamples[(i << 1) + 1] = input;
            }
        }

       #if JUCE_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero (true);
       #endif
    }

    void processSamplesDown (AudioBlock<SampleType>& outputBlock) override
    {
        jassert (outputBlock.getNumChannels() <= static_cast<size_t> (ParentType::buffer.getNumChannels()));
        jassert (outputBlock.getNumSamples() * ParentType::factor <= static_cast<size_t> (ParentType::buffer.getNumSamples()));

        // Initialization
        auto coeffs = coefficientsDown.getRawDataPointer();
        auto numStages = coefficientsDown.size();
        auto delayedStages = numStages / 2;
        auto directStages = numStages - delayedStages;
        auto numSamples = outputBlock.getNumSamples();

        // Processing
        for (size_t channel = 0; channel < outputBlock.getNumChannels(); ++channel)
        {
            auto bufferSamples = ParentType::buffer.getWritePointer (static_cast<int> (channel));
            auto lv1 = v1Down.getWritePointer (static_cast<int> (channel));
            auto samples = outputBlock.getChannelPointer (channel);
            auto delay = delayDown.getUnchecked (static_cast<int> (channel));

            for (size_t i = 0; i < numSamples; ++i)
            {
                // Direct path cascaded allpass filters
                auto input = bufferSamples[i << 1];

                for (auto n = 0; n < directStages; ++n)
                {
                    auto alpha = coeffs[n];
                    auto output = alpha * input + lv1[n];
                    lv1[n] = input - alpha * output;
                    input = output;
                }

                auto directOut = input;

                // Delayed path cascaded allpass filters
                input = bufferSamples[(i << 1) + 1];

                for (auto n = directStages; n < numStages; ++n)
                {
                    auto alpha = coeffs[n];
                    auto output = alpha * input + lv1[n];
                    lv1[n] = input - alpha * output;
                    input = output;
                }

                // Output
                samples[i] = (delay + directOut) * static_cast<SampleType> (0.5);
                delay = input;
            }

            delayDown.setUnchecked (static_cast<int> (channel), delay);
        }

       #if JUCE_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero (false);
       #endif
    }

    void snapToZero (bool snapUpProcessing)
    {
        if (snapUpProcessing)
        {
            for (auto channel = 0; channel < ParentType::buffer.getNumChannels(); ++channel)
            {
                auto lv1 = v1Up.getWritePointer (channel);
                auto numStages = coefficientsUp.size();

                for (auto n = 0; n < numStages; ++n)
                    util::snapToZero (lv1[n]);
            }
        }
        else
        {
            for (auto channel = 0; channel < ParentType::buffer.getNumChannels(); ++channel)
            {
                auto lv1 = v1Down.getWritePointer (channel);
                auto numStages = coefficientsDown.size();

                for (auto n = 0; n < numStages; ++n)
                    util::snapToZero (lv1[n]);
            }
        }
    }

private:
    //==============================================================================
    /** This function calculates the equivalent high order IIR filter of a given
        polyphase cascaded allpass filters structure.
    */
    IIR::Coefficients<SampleType> getCoefficients (typename FilterDesign<SampleType>::IIRPolyphaseAllpassStructure& structure) const
    {
        constexpr auto one = static_cast<SampleType> (1.0);

        Polynomial<SampleType> numerator1 ({ one }), denominator1 ({ one }),
                               numerator2 ({ one }), denominator2 ({ one });

        for (auto* i : structure.directPath)
        {
            auto coeffs = i->getRawCoefficients();

            if (i->getFilterOrder() == 1)
            {
                numerator1   = numerator1  .getProductWith (Polynomial<SampleType> ({ coeffs[0], coeffs[1] }));
                denominator1 = denominator1.getProductWith (Polynomial<SampleType> ({ one,       coeffs[2] }));
            }
            else
            {
                numerator1   = numerator1  .getProductWith (Polynomial<SampleType> ({ coeffs[0], coeffs[1], coeffs[2] }));
                denominator1 = denominator1.getProductWith (Polynomial<SampleType> ({ one,       coeffs[3], coeffs[4] }));
            }
        }

        for (auto* i : structure.delayedPath)
        {
            auto coeffs = i->getRawCoefficients();

            if (i->getFilterOrder() == 1)
            {
                numerator2   = numerator2  .getProductWith (Polynomial<SampleType> ({ coeffs[0], coeffs[1] }));
                denominator2 = denominator2.getProductWith (Polynomial<SampleType> ({ one,       coeffs[2] }));
            }
            else
            {
                numerator2   = numerator2  .getProductWith (Polynomial<SampleType> ({ coeffs[0], coeffs[1], coeffs[2] }));
                denominator2 = denominator2.getProductWith (Polynomial<SampleType> ({ one,       coeffs[3], coeffs[4] }));
            }
        }

        auto numeratorf1 = numerator1.getProductWith (denominator2);
        auto numeratorf2 = numerator2.getProductWith (denominator1);
        auto numerator   = numeratorf1.getSumWith (numeratorf2);
        auto denominator = denominator1.getProductWith (denominator2);

        IIR::Coefficients<SampleType> coeffs;

        coeffs.coefficients.clear();
        auto inversion = one / denominator[0];

        for (int i = 0; i <= numerator.getOrder(); ++i)
            coeffs.coefficients.add (numerator[i] * inversion);

        for (int i = 1; i <= denominator.getOrder(); ++i)
            coeffs.coefficients.add (denominator[i] * inversion);

        return coeffs;
    }

    //==============================================================================
    Array<SampleType> coefficientsUp, coefficientsDown;
    SampleType latency;

    AudioBuffer<SampleType> v1Up, v1Down;
    Array<SampleType> delayDown;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling2TimesPolyphaseIIR)
};


//==============================================================================
template <typename SampleType>
Oversampling<SampleType>::Oversampling (size_t newNumChannels)
    : numChannels (newNumChannels)
{
    jassert (numChannels > 0);

    addDummyOversamplingStage();
}

template <typename SampleType>
Oversampling<SampleType>::Oversampling (size_t newNumChannels, size_t newFactor,
                                        FilterType newType, bool isMaximumQuality,
                                        bool useIntegerLatency)
    : numChannels (newNumChannels), shouldUseIntegerLatency (useIntegerLatency)
{
    jassert (isPositiveAndBelow (newFactor, 5) && numChannels > 0);

    if (newFactor == 0)
    {
        addDummyOversamplingStage();
    }
    else if (newType == FilterType::filterHalfBandPolyphaseIIR)
    {
        for (size_t n = 0; n < newFactor; ++n)
        {
            auto twUp   = (isMaximumQuality ? 0.10f : 0.12f) * (n == 0 ? 0.5f : 1.0f);
            auto twDown = (isMaximumQuality ? 0.12f : 0.15f) * (n == 0 ? 0.5f : 1.0f);

            auto gaindBStartUp    = (isMaximumQuality ? -90.0f : -70.0f);
            auto gaindBStartDown  = (isMaximumQuality ? -75.0f : -60.0f);
            auto gaindBFactorUp   = (isMaximumQuality ? 10.0f  : 8.0f);
            auto gaindBFactorDown = (isMaximumQuality ? 10.0f  : 8.0f);

            addOversamplingStage (FilterType::filterHalfBandPolyphaseIIR,
                                  twUp, gaindBStartUp + gaindBFactorUp * (float) n,
                                  twDown, gaindBStartDown + gaindBFactorDown * (float) n);
        }
    }
    else if (newType == FilterType::filterHalfBandFIREquiripple)
    {
        for (size_t n = 0; n < newFactor; ++n)
        {
            auto twUp   = (isMaximumQuality ? 0.10f : 0.12f) * (n == 0 ? 0.5f : 1.0f);
            auto twDown = (isMaximumQuality ? 0.12f : 0.15f) * (n == 0 ? 0.5f : 1.0f);

            auto gaindBStartUp    = (isMaximumQuality ? -90.0f : -70.0f);
            auto gaindBStartDown  = (isMaximumQuality ? -75.0f : -60.0f);
            auto gaindBFactorUp   = (isMaximumQuality ? 10.0f  : 8.0f);
            auto gaindBFactorDown = (isMaximumQuality ? 10.0f  : 8.0f);

            addOversamplingStage (FilterType::filterHalfBandFIREquiripple,
                                  twUp, gaindBStartUp + gaindBFactorUp * (float) n,
                                  twDown, gaindBStartDown + gaindBFactorDown * (float) n);
        }
    }
}

template <typename SampleType>
Oversampling<SampleType>::~Oversampling()
{
    stages.clear();
}

//==============================================================================
template <typename SampleType>
void Oversampling<SampleType>::addDummyOversamplingStage()
{
    stages.add (new OversamplingDummy<SampleType> (numChannels));
}

template <typename SampleType>
void Oversampling<SampleType>::addOversamplingStage (FilterType type,
                                                     float normalisedTransitionWidthUp,
                                                     float stopbandAmplitudedBUp,
                                                     float normalisedTransitionWidthDown,
                                                     float stopbandAmplitudedBDown)
{
    if (type == FilterType::filterHalfBandPolyphaseIIR)
    {
        stages.add (new Oversampling2TimesPolyphaseIIR<SampleType> (numChannels,
                                                                    normalisedTransitionWidthUp,   stopbandAmplitudedBUp,
                                                                    normalisedTransitionWidthDown, stopbandAmplitudedBDown));
    }
    else
    {
        stages.add (new Oversampling2TimesEquirippleFIR<SampleType> (numChannels,
                                                                     normalisedTransitionWidthUp,   stopbandAmplitudedBUp,
                                                                     normalisedTransitionWidthDown, stopbandAmplitudedBDown));
    }

    factorOversampling *= 2;
}

template <typename SampleType>
void Oversampling<SampleType>::clearOversamplingStages()
{
    stages.clear();
    factorOversampling = 1u;
}

//==============================================================================
template <typename SampleType>
void Oversampling<SampleType>::setUsingIntegerLatency (bool useIntegerLatency) noexcept
{
    shouldUseIntegerLatency = useIntegerLatency;
}

template <typename SampleType>
SampleType Oversampling<SampleType>::getLatencyInSamples() const noexcept
{
    auto latency = getUncompensatedLatency();
    return shouldUseIntegerLatency ? latency + fractionalDelay : latency;
}

template <typename SampleType>
SampleType Oversampling<SampleType>::getUncompensatedLatency() const noexcept
{
    auto latency = static_cast<SampleType> (0);
    size_t order = 1;

    for (auto* stage : stages)
    {
        order *= stage->factor;
        latency += stage->getLatencyInSamples() / static_cast<SampleType> (order);
    }

    return latency;
}

template <typename SampleType>
size_t Oversampling<SampleType>::getOversamplingFactor() const noexcept
{
    return factorOversampling;
}

//==============================================================================
template <typename SampleType>
void Oversampling<SampleType>::initProcessing (size_t maximumNumberOfSamplesBeforeOversampling)
{
    jassert (! stages.isEmpty());
    auto currentNumSamples = maximumNumberOfSamplesBeforeOversampling;

    for (auto* stage : stages)
    {
        stage->initProcessing (currentNumSamples);
        currentNumSamples *= stage->factor;
    }

    ProcessSpec spec = { 0.0, (uint32) maximumNumberOfSamplesBeforeOversampling, (uint32) numChannels };
    delay.prepare (spec);
    updateDelayLine();

    isReady = true;
    reset();
}

template <typename SampleType>
void Oversampling<SampleType>::reset() noexcept
{
    jassert (! stages.isEmpty());

    if (isReady)
        for (auto* stage : stages)
           stage->reset();

    delay.reset();
}

template <typename SampleType>
AudioBlock<SampleType> Oversampling<SampleType>::processSamplesUp (const AudioBlock<const SampleType>& inputBlock) noexcept
{
    jassert (! stages.isEmpty());

    if (! isReady)
        return {};

    auto* firstStage = stages.getUnchecked (0);
    firstStage->processSamplesUp (inputBlock);
    auto block = firstStage->getProcessedSamples (inputBlock.getNumSamples() * firstStage->factor);

    for (int i = 1; i < stages.size(); ++i)
    {
        stages[i]->processSamplesUp (block);
        block = stages[i]->getProcessedSamples (block.getNumSamples() * stages[i]->factor);
    }

    return block;
}

template <typename SampleType>
void Oversampling<SampleType>::processSamplesDown (AudioBlock<SampleType>& outputBlock) noexcept
{
    jassert (! stages.isEmpty());

    if (! isReady)
        return;

    auto currentNumSamples = outputBlock.getNumSamples();

    for (int n = 0; n < stages.size() - 1; ++n)
        currentNumSamples *= stages.getUnchecked (n)->factor;

    for (int n = stages.size() - 1; n > 0; --n)
    {
        auto& stage = *stages.getUnchecked (n);
        auto audioBlock = stages.getUnchecked (n - 1)->getProcessedSamples (currentNumSamples);
        stage.processSamplesDown (audioBlock);

        currentNumSamples /= stage.factor;
    }

    stages.getFirst()->processSamplesDown (outputBlock);

    if (shouldUseIntegerLatency && fractionalDelay > static_cast<SampleType> (0.0))
    {
        auto context = ProcessContextReplacing<SampleType> (outputBlock);
        delay.process (context);
    }
}

template <typename SampleType>
void Oversampling<SampleType>::updateDelayLine()
{
    auto latency = getUncompensatedLatency();
    fractionalDelay = static_cast<SampleType> (1.0) - (latency - std::floor (latency));

    if (approximatelyEqual (fractionalDelay, static_cast<SampleType> (1.0)))
        fractionalDelay = static_cast<SampleType> (0.0);
    else if (fractionalDelay < static_cast<SampleType> (0.618))
        fractionalDelay += static_cast<SampleType> (1.0);

    delay.setDelay (fractionalDelay);
}

template class Oversampling<float>;
template class Oversampling<double>;

} // namespace juce::dsp
