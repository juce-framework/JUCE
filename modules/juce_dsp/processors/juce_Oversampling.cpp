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


//===============================================================================
/** Abstract class for the provided oversampling engines used internally in
    the Oversampling class.
*/
template <typename SampleType>
class OversamplingEngine
{
public:
    //===============================================================================
    OversamplingEngine (size_t newFactor) { factor = newFactor; }
    virtual ~OversamplingEngine() {}

    //===============================================================================
    virtual SampleType getLatencyInSamples() = 0;
    size_t getFactor() { return factor; }

    virtual void initProcessing (size_t maximumNumberOfSamplesBeforeOversampling)
    {
        buffer.setSize (1, static_cast<int> (maximumNumberOfSamplesBeforeOversampling * factor));
    }

    virtual void reset()
    {
        buffer.clear();
    }

    SampleType* getProcessedSamples() { return buffer.getWritePointer (0); }
    size_t getNumProcessedSamples()   { return static_cast<size_t> (buffer.getNumSamples()); }

    virtual void processSamplesUp (SampleType *samples, size_t numSamples) = 0;
    virtual void processSamplesDown (SampleType *samples, size_t numSamples) = 0;

protected:
    //===============================================================================
    AudioBuffer<SampleType> buffer;
    size_t factor;
};


//===============================================================================
/** Dummy oversampling engine class which simply copies and pastes the input
    signal, which could be equivalent to a "one time" oversampling processing.
*/
template <typename SampleType>
class OversamplingDummy : public OversamplingEngine<SampleType>
{
public:
    //===============================================================================
    OversamplingDummy() : OversamplingEngine<SampleType>(1) {}
    ~OversamplingDummy() {}

    //===============================================================================
    SampleType getLatencyInSamples() override
    {
        return 0.f;
    }

    void processSamplesUp (SampleType *samples, size_t numSamples) override
    {
        auto bufferSamples = this->buffer.getWritePointer (0);

        for (size_t i = 0; i < numSamples; i++)
            bufferSamples[i] = samples[i];
    }

    void processSamplesDown (SampleType *samples, size_t numSamples) override
    {
        auto bufferSamples = OversamplingEngine<SampleType>::buffer.getWritePointer (0);

        for (size_t i = 0; i < numSamples; i++)
            samples[i] = bufferSamples[i];
    }

private:
    //===============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingDummy)
};

//===============================================================================
/**
    Oversampling engine class performing 2 times oversampling using the Filter
    Design FIR Equiripple method. The resulting filter is linear phase,
    symmetric, and has every two samples but the middle one equal to zero,
    leading to specific processing optimizations.
*/
template <typename SampleType>
class Oversampling2TimesEquirippleFIR : public OversamplingEngine<SampleType>
{
public:
    //===============================================================================
    Oversampling2TimesEquirippleFIR (SampleType normalizedTransitionWidthUp,
                                     SampleType stopbandAttenuationdBUp,
                                     SampleType normalizedTransitionWidthDown,
                                     SampleType stopbandAttenuationdBDown) : OversamplingEngine<SampleType> (2)
    {
        coefficientsUp = *dsp::FilterDesign<SampleType>::designFIRLowpassHalfBandEquirippleMethod (normalizedTransitionWidthUp, stopbandAttenuationdBUp);
        coefficientsDown = *dsp::FilterDesign<SampleType>::designFIRLowpassHalfBandEquirippleMethod (normalizedTransitionWidthDown, stopbandAttenuationdBDown);

        auto N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;

        stateUp.setSize (1, static_cast<int> (coefficientsUp.getFilterOrder() + 1));
        stateDown.setSize (1, static_cast<int> (N));
        stateDown2.setSize (1, static_cast<int> (Ndiv4));
    }

    ~Oversampling2TimesEquirippleFIR() {}

    //===============================================================================
    SampleType getLatencyInSamples() override
    {
        return static_cast<SampleType> (coefficientsUp.getFilterOrder() + coefficientsDown.getFilterOrder());
    }

    void reset() override
    {
        OversamplingEngine<SampleType>::reset();

        stateUp.clear();
        stateDown.clear();
        stateDown2.clear();

        position = 0;
    }

    void processSamplesUp (SampleType *samples, size_t numSamples) override
    {
        // Initialization
        auto bufferSamples = OversamplingEngine<SampleType>::buffer.getWritePointer (0);
        auto fir = coefficientsUp.getRawCoefficients();
        auto buf = stateUp.getWritePointer (0);

        auto N = coefficientsUp.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;

        // Processing
        for (size_t i = 0; i < numSamples; i++)
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
            for (size_t k = 0; k < N - 2; k+=2)
                buf[k] = buf[k + 2];
        }
    }

    void processSamplesDown (SampleType *samples, size_t numSamples) override
    {
        // Initialization
        auto bufferSamples = OversamplingEngine<SampleType>::buffer.getWritePointer (0);
        auto fir = coefficientsDown.getRawCoefficients();
        auto buf = stateDown.getWritePointer (0);
        auto buf2 = stateDown2.getWritePointer (0);

        auto N = coefficientsDown.getFilterOrder() + 1;
        auto Ndiv2 = N / 2;
        auto Ndiv4 = Ndiv2 / 2;

        // Processing
        for (size_t i = 0; i < numSamples; i++)
        {
            // Input
            buf[N - 1] = bufferSamples[2 * i];

            // Convolution
            auto out = static_cast<SampleType> (0.0);
            for (size_t k = 0; k < Ndiv2; k += 2)
                out += (buf[k] + buf[N - k - 1]) * fir[k];

            // Output
            out += buf2[position] * fir[Ndiv2];
            buf2[position] = bufferSamples[2 * i + 1];

            samples[i] = out;

            // Shift data
            for (size_t k = 0; k < N - 2; k++)
                buf[k] = buf[k + 2];

            // Circular buffer
            position = (position == 0 ? Ndiv4 - 1 : position - 1);
        }
    }

private:
    //===============================================================================
    dsp::FIR::Coefficients<SampleType> coefficientsUp, coefficientsDown;
    AudioBuffer<SampleType> stateUp, stateDown, stateDown2;
    size_t position;

    //===============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling2TimesEquirippleFIR)
};


//===============================================================================
/** Oversampling engine class performing 2 times oversampling using the Filter
    Design IIR Polyphase Allpass Cascaded method. The resulting filter is minimum
    phase, and provided with a method to get the exact resulting latency.
*/
template <typename SampleType>
class Oversampling2TimesPolyphaseIIR : public OversamplingEngine<SampleType>
{
public:
    //===============================================================================
    Oversampling2TimesPolyphaseIIR (SampleType normalizedTransitionWidthUp,
                                    SampleType stopbandAttenuationdBUp,
                                    SampleType normalizedTransitionWidthDown,
                                    SampleType stopbandAttenuationdBDown) : OversamplingEngine<SampleType> (2)
    {
        auto structureUp = dsp::FilterDesign<SampleType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (normalizedTransitionWidthUp, stopbandAttenuationdBUp);
        dsp::IIR::Coefficients<SampleType> coeffsUp = getCoefficients (structureUp);
        latency = static_cast<SampleType> (-(coeffsUp.getPhaseForFrequency (0.0001, 1.0)) / (0.0001 * 2 * double_Pi));

        auto structureDown = dsp::FilterDesign<SampleType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (normalizedTransitionWidthDown, stopbandAttenuationdBDown);
        dsp::IIR::Coefficients<SampleType> coeffsDown = getCoefficients (structureDown);
        latency += static_cast<SampleType> (-(coeffsDown.getPhaseForFrequency (0.0001, 1.0)) / (0.0001 * 2 * double_Pi));

        for (auto i = 0; i < structureUp.directPath.size(); i++)
            coefficientsUp.add (structureUp.directPath[i].coefficients[0]);

        for (auto i = 1; i < structureUp.delayedPath.size(); i++)
            coefficientsUp.add (structureUp.delayedPath[i].coefficients[0]);

        for (auto i = 0; i < structureDown.directPath.size(); i++)
            coefficientsDown.add (structureDown.directPath[i].coefficients[0]);

        for (auto i = 1; i < structureDown.delayedPath.size(); i++)
            coefficientsDown.add (structureDown.delayedPath[i].coefficients[0]);

        v1Up.resize (coefficientsUp.size());
        v1Down.resize (coefficientsDown.size());
    }

    ~Oversampling2TimesPolyphaseIIR() {}

    //===============================================================================
    SampleType getLatencyInSamples() override
    {
        return latency;
    }

    void reset() override
    {
        OversamplingEngine<SampleType>::reset();

        v1Up.fill (0);
        v1Down.fill (0);
        delayDown = 0;
    }

    void processSamplesUp (SampleType *samples, size_t numSamples) override
    {
        // Initialization
        auto bufferSamples = OversamplingEngine<SampleType>::buffer.getWritePointer (0);
        auto coeffs = coefficientsUp.getRawDataPointer();
        auto lv1 = v1Up.getRawDataPointer();

        auto numStages = coefficientsUp.size();
        auto delayedStages = numStages / 2;
        auto directStages = numStages - delayedStages;

        // Processing
        for (size_t i = 0; i < numSamples; i++)
        {
            // Direct path cascaded allpass filters
            auto input = samples[i];
            for (auto n = 0; n < directStages; n++)
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
            for (auto n = directStages; n < numStages; n++)
            {
                auto alpha = coeffs[n];
                auto output = alpha * input + lv1[n];
                lv1[n] = input - alpha * output;
                input = output;
            }

            // Output
            bufferSamples[(i << 1) + 1] = input;
        }

        // Snap To Zero
        snapToZero (true);

    }

    void processSamplesDown (SampleType *samples, size_t numSamples) override
    {
        // Initialization
        auto bufferSamples = OversamplingEngine<SampleType>::buffer.getWritePointer (0);
        auto coeffs = coefficientsDown.getRawDataPointer();
        auto lv1 = v1Down.getRawDataPointer();

        auto numStages = coefficientsDown.size();
        auto delayedStages = numStages / 2;
        auto directStages = numStages - delayedStages;

        // Processing
        for (size_t i = 0; i < numSamples; i++)
        {
            // Direct path cascaded allpass filters
            auto input = bufferSamples[i << 1];
            for (auto n = 0; n < directStages; n++)
            {
                auto alpha = coeffs[n];
                auto output = alpha * input + lv1[n];
                lv1[n] = input - alpha * output;
                input = output;
            }
            auto directOut = input;

            // Delayed path cascaded allpass filters
            input = bufferSamples[(i << 1) + 1];
            for (auto n = directStages; n < numStages; n++)
            {
                auto alpha = coeffs[n];
                auto output = alpha * input + lv1[n];
                lv1[n] = input - alpha * output;
                input = output;
            }

            // Output
            samples[i] = (delayDown + directOut) * static_cast<SampleType> (0.5);
            delayDown = input;
        }

        // Snap To Zero
        snapToZero (false);
    }

    void snapToZero (bool snapUpProcessing)
    {
        if (snapUpProcessing)
        {
            auto lv1 = v1Up.getRawDataPointer();
            auto numStages = coefficientsUp.size();

            for (auto n = 0; n < numStages; n++)
                JUCE_SNAP_TO_ZERO (lv1[n]);
        }
        else
        {
            auto lv1 = v1Down.getRawDataPointer();
            auto numStages = coefficientsDown.size();

            for (auto n = 0; n < numStages; n++)
                JUCE_SNAP_TO_ZERO (lv1[n]);
        }
    }

private:
    //===============================================================================
    /** This function calculates the equivalent high order IIR filter of a given
        polyphase cascaded allpass filters structure.
    */
    const dsp::IIR::Coefficients<SampleType> getCoefficients (typename dsp::FilterDesign<SampleType>::IIRPolyphaseAllpassStructure &structure) const
    {
        dsp::Polynomial<SampleType> numerator1 ({ static_cast<SampleType> (1.0) });
        dsp::Polynomial<SampleType> denominator1 ({ static_cast<SampleType> (1.0) });
        dsp::Polynomial<SampleType> numerator2 ({ static_cast<SampleType> (1.0) });
        dsp::Polynomial<SampleType> denominator2 ({ static_cast<SampleType> (1.0) });

        dsp::Polynomial<SampleType> temp;

        for (auto n = 0; n < structure.directPath.size(); n++)
        {
            auto *coeffs = structure.directPath.getReference (n).getRawCoefficients();

            if (structure.directPath[n].getFilterOrder() == 1)
            {
                temp = dsp::Polynomial<SampleType> ({ coeffs[0], coeffs[1] });
                numerator1 = numerator1.getProductWith (temp);

                temp = dsp::Polynomial<SampleType> ({ static_cast<SampleType> (1.0), coeffs[2] });
                denominator1 = denominator1.getProductWith (temp);
            }
            else
            {
                temp = dsp::Polynomial<SampleType> ({ coeffs[0], coeffs[1], coeffs[2] });
                numerator1 = numerator1.getProductWith (temp);

                temp = dsp::Polynomial<SampleType> ({ static_cast<SampleType> (1.0), coeffs[3], coeffs[4] });
                denominator1 = denominator1.getProductWith (temp);
            }
        }

        for (auto n = 0; n < structure.delayedPath.size(); n++)
        {
            auto *coeffs = structure.delayedPath.getReference (n).getRawCoefficients();

            if (structure.delayedPath[n].getFilterOrder() == 1)
            {
                temp = dsp::Polynomial<SampleType> ({ coeffs[0], coeffs[1] });
                numerator2 = numerator2.getProductWith (temp);

                temp = dsp::Polynomial<SampleType> ({ static_cast<SampleType> (1.0), coeffs[2] });
                denominator2 = denominator2.getProductWith (temp);
            }
            else
            {
                temp = dsp::Polynomial<SampleType> ({ coeffs[0], coeffs[1], coeffs[2] });
                numerator2 = numerator2.getProductWith (temp);

                temp = dsp::Polynomial<SampleType> ({ static_cast<SampleType> (1.0), coeffs[3], coeffs[4] });
                denominator2 = denominator2.getProductWith (temp);
            }
        }

        dsp::Polynomial<SampleType> numeratorf1 = numerator1.getProductWith (denominator2);
        dsp::Polynomial<SampleType> numeratorf2 = numerator2.getProductWith (denominator1);
        dsp::Polynomial<SampleType> numerator = numeratorf1.getSumWith (numeratorf2);
        dsp::Polynomial<SampleType> denominator = denominator1.getProductWith (denominator2);

        dsp::IIR::Coefficients<SampleType> coeffs;

        coeffs.coefficients.clear();
        auto inversion = static_cast<SampleType> (1.0) / denominator[0];

        for (auto i = 0; i <= numerator.getOrder(); i++)
            coeffs.coefficients.add (numerator[i] * inversion);

        for (auto i = 1; i <= denominator.getOrder(); i++)
            coeffs.coefficients.add (denominator[i] * inversion);

        return coeffs;
    }

    //===============================================================================
    Array<SampleType> coefficientsUp, coefficientsDown;
    SampleType latency;

    Array<SampleType> v1Up, v1Down;
    SampleType delayDown;

    //===============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling2TimesPolyphaseIIR)
};


//===============================================================================
template <typename SampleType>
Oversampling<SampleType>::Oversampling (size_t newNumChannels, size_t newFactor, FilterType newType, bool newMaxQuality)
{
    jassert (newFactor >= 0 && newFactor <= 4 && newNumChannels > 0);

    factorOversampling = (size_t) 1 << newFactor;
    isMaximumQuality = newMaxQuality;
    type = newType;
    numChannels = newNumChannels;

    if (newFactor == 0)
    {
        for (size_t channel = 0; channel < numChannels; channel++)
            engines.add (new OversamplingDummy<SampleType>());

        numStages = 1;
    }
    else if (type == FilterType::filterHalfBandPolyphaseIIR)
    {
        numStages = newFactor;

        for (size_t channel = 0; channel < numChannels; channel++)
            for (size_t n = 0; n < numStages; n++)
            {
                auto tw1 = (isMaximumQuality ? 0.10f : 0.12f);
                auto tw2 = (isMaximumQuality ? 0.12f : 0.15f);

                engines.add (new Oversampling2TimesPolyphaseIIR<SampleType> (tw1, -75.f + 10.f * n, tw2, -70.f + 10.f * n));
            }
    }
    else if (type == FilterType::filterHalfBandFIREquiripple)
    {
        numStages = newFactor;

        for (size_t channel = 0; channel < numChannels; channel++)
            for (size_t n = 0; n < numStages; n++)
            {
                auto tw1 = (isMaximumQuality ? 0.10f : 0.12f);
                auto tw2 = (isMaximumQuality ? 0.12f : 0.15f);

                engines.add (new Oversampling2TimesEquirippleFIR<SampleType> (tw1, -90.f + 10.f * n, tw2, -70.f + 10.f * n));
            }
    }
}

template <typename SampleType>
Oversampling<SampleType>::~Oversampling()
{
    engines.clear();
}

//===============================================================================
template <typename SampleType>
SampleType Oversampling<SampleType>::getLatencyInSamples() noexcept
{
    auto latency = static_cast<SampleType> (0);
    size_t order = 1;

    for (size_t n = 0; n < numStages; n++)
    {
        auto& engine = *engines[static_cast<int> (n)];

        order *= engine.getFactor();
        latency += engine.getLatencyInSamples() / std::pow (static_cast<SampleType> (2), static_cast<SampleType> (order));
    }

    return latency;
}

template <typename SampleType>
size_t Oversampling<SampleType>::getOversamplingFactor() noexcept
{
    return factorOversampling;
}

//===============================================================================
template <typename SampleType>
void Oversampling<SampleType>::initProcessing (size_t maximumNumberOfSamplesBeforeOversampling)
{
    jassert (engines.size() > 0);

    for (size_t channel = 0; channel < numChannels; channel++)
    {
        auto currentNumSamples = maximumNumberOfSamplesBeforeOversampling;
        auto offset = numStages * channel;

        for (size_t n = 0; n < numStages; n++)
        {
            auto& engine = *engines[static_cast<int> (n + offset)];

            engine.initProcessing (currentNumSamples);
            currentNumSamples *= engine.getFactor();
        }
    }
    isReady = true;

    reset();
}

template <typename SampleType>
void Oversampling<SampleType>::reset() noexcept
{
    jassert (engines.size() > 0);

    if (isReady)
        for (auto n = 0; n < engines.size(); n++)
            engines[n]->reset();
}

template <typename SampleType>
typename dsp::AudioBlock<SampleType> Oversampling<SampleType>::getProcessedSamples()
{
    jassert (engines.size() > 0);

    Array<SampleType*> arrayChannels;

    for (size_t channel = 0; channel < numChannels; channel++)
        arrayChannels.add (engines[static_cast<int> (((channel + 1) * numStages) - 1)]->getProcessedSamples());

    auto numSamples = engines[static_cast<int> (numStages - 1)]->getNumProcessedSamples();
    auto block = dsp::AudioBlock<SampleType> (arrayChannels.getRawDataPointer(), numChannels, numSamples);

    return block;
}

template <typename SampleType>
void Oversampling<SampleType>::processSamplesUp (dsp::AudioBlock<SampleType> &block) noexcept
{
    jassert (engines.size() > 0 && block.getNumChannels() <= numChannels);

    if (! isReady)
        return;

    for (size_t channel = 0; channel < jmin (numChannels, block.getNumChannels()); channel++)
    {
        SampleType* dataSamples = block.getChannelPointer (channel);
        auto currentNumSamples = block.getNumSamples();
        auto offset = numStages * channel;

        for (size_t n = 0; n < numStages; n++)
        {
            auto& engine = *engines[static_cast<int> (n + offset)];
            engine.processSamplesUp (dataSamples, currentNumSamples);

            currentNumSamples *= engine.getFactor();
            dataSamples = engine.getProcessedSamples();
        }
    }
}

template <typename SampleType>
void Oversampling<SampleType>::processSamplesDown (dsp::AudioBlock<SampleType> &block) noexcept
{
    jassert (engines.size() > 0 && block.getNumChannels() <= numChannels);

    if (! isReady)
        return;

    for (size_t channel = 0; channel < jmin (numChannels, block.getNumChannels()); channel++)
    {
        auto currentNumSamples = block.getNumSamples();
        auto offset = numStages * channel;

        for (size_t n = 0; n < numStages - 1; n++)
            currentNumSamples *= engines[static_cast<int> (n + offset)]->getFactor();

        for (size_t n = numStages - 1; n > 0; n--)
        {
            auto& engine = *engines[static_cast<int> (n + offset)];

            auto dataSamples = engines[static_cast<int> (n + offset - 1)]->getProcessedSamples();
            engine.processSamplesDown (dataSamples, currentNumSamples);

            currentNumSamples /= engine.getFactor();
        }

        engines[static_cast<int> (offset)]->processSamplesDown (block.getChannelPointer (channel), currentNumSamples);
    }

}

template class Oversampling<float>;
template class Oversampling<double>;
