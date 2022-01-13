/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

//==============================================================================
template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer()
    : DryWetMixer (0)
{
}

template <typename SampleType>
DryWetMixer<SampleType>::DryWetMixer (int maximumWetLatencyInSamplesIn)
    : dryDelayLine (maximumWetLatencyInSamplesIn),
      maximumWetLatencyInSamples (maximumWetLatencyInSamplesIn)
{
    dryDelayLine.setDelay (0);

    update();
    reset();
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::setMixingRule (MixingRule newRule)
{
    currentMixingRule = newRule;
    update();
}

template <typename SampleType>
void DryWetMixer<SampleType>::setWetMixProportion (SampleType newWetMixProportion)
{
    jassert (isPositiveAndNotGreaterThan (newWetMixProportion, 1.0));

    mix = jlimit (static_cast<SampleType> (0.0), static_cast<SampleType> (1.0), newWetMixProportion);
    update();
}

template <typename SampleType>
void DryWetMixer<SampleType>::setWetLatency (SampleType wetLatencySamples)
{
    dryDelayLine.setDelay (wetLatencySamples);
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    dryDelayLine.prepare (spec);
    bufferDry.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize, false, false, true);

    update();
    reset();
}

template <typename SampleType>
void DryWetMixer<SampleType>::reset()
{
    dryVolume.reset (sampleRate, 0.05);
    wetVolume.reset (sampleRate, 0.05);

    dryDelayLine.reset();

    fifo = SingleThreadedAbstractFifo (nextPowerOfTwo (bufferDry.getNumSamples()));
    bufferDry.setSize (bufferDry.getNumChannels(), fifo.getSize(), false, false, true);
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::pushDrySamples (const AudioBlock<const SampleType> drySamples)
{
    jassert (drySamples.getNumChannels() <= (size_t) bufferDry.getNumChannels());
    jassert (drySamples.getNumSamples() <= (size_t) fifo.getRemainingSpace());

    auto offset = 0;

    for (const auto& range : fifo.write ((int) drySamples.getNumSamples()))
    {
        if (range.getLength() == 0)
            continue;

        auto block = AudioBlock<SampleType> (bufferDry).getSubsetChannelBlock (0, drySamples.getNumChannels())
                                                       .getSubBlock ((size_t) range.getStart(), (size_t) range.getLength());

        auto inputBlock = drySamples.getSubBlock ((size_t) offset, (size_t) range.getLength());

        if (maximumWetLatencyInSamples == 0)
            block.copyFrom (inputBlock);
        else
            dryDelayLine.process (ProcessContextNonReplacing<SampleType> (inputBlock, block));

        offset += range.getLength();
    }
}

template <typename SampleType>
void DryWetMixer<SampleType>::mixWetSamples (AudioBlock<SampleType> inOutBlock)
{
    inOutBlock.multiplyBy (wetVolume);

    jassert (inOutBlock.getNumSamples() <= (size_t) fifo.getNumReadable());

    auto offset = 0;

    for (const auto& range : fifo.read ((int) inOutBlock.getNumSamples()))
    {
        if (range.getLength() == 0)
            continue;

        auto block = AudioBlock<SampleType> (bufferDry).getSubsetChannelBlock (0, inOutBlock.getNumChannels())
                                                       .getSubBlock ((size_t) range.getStart(), (size_t) range.getLength());
        block.multiplyBy (dryVolume);
        inOutBlock.getSubBlock ((size_t) offset).add (block);

        offset += range.getLength();
    }
}

//==============================================================================
template <typename SampleType>
void DryWetMixer<SampleType>::update()
{
    SampleType dryValue, wetValue;

    switch (currentMixingRule)
    {
        case MixingRule::balanced:
            dryValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = static_cast<SampleType> (2.0) * jmin (static_cast<SampleType> (0.5), mix);
            break;

        case MixingRule::linear:
            dryValue = static_cast<SampleType> (1.0) - mix;
            wetValue = mix;
            break;

        case MixingRule::sin3dB:
            dryValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)));
            wetValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * mix));
            break;

        case MixingRule::sin4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * mix), 1.5));
            break;

        case MixingRule::sin6dB:
            dryValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - mix)), 2.0));
            wetValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * mix), 2.0));
            break;

        case MixingRule::squareRoot3dB:
            dryValue = std::sqrt (static_cast<SampleType> (1.0) - mix);
            wetValue = std::sqrt (mix);
            break;

        case MixingRule::squareRoot4p5dB:
            dryValue = static_cast<SampleType> (std::pow (std::sqrt (1.0 - mix), 1.5));
            wetValue = static_cast<SampleType> (std::pow (std::sqrt (mix), 1.5));
            break;

        default:
            dryValue = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - mix);
            wetValue = jmin (static_cast<SampleType> (0.5), mix);
            break;
    }

    dryVolume.setTargetValue (dryValue);
    wetVolume.setTargetValue (wetValue);
}

//==============================================================================
template class DryWetMixer<float>;
template class DryWetMixer<double>;


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct DryWetMixerTests : public UnitTest
{
    DryWetMixerTests() : UnitTest ("DryWetMixer", UnitTestCategories::dsp) {}

    enum class Kind { down, up };

    static auto getRampBuffer (ProcessSpec spec, Kind kind)
    {
        AudioBuffer<float> buffer ((int) spec.numChannels, (int) spec.maximumBlockSize);

        for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
        {
            for (uint32_t channel = 0; channel < spec.numChannels; ++channel)
            {
                const auto ramp = kind == Kind::up ? sample : spec.maximumBlockSize - sample;

                buffer.setSample ((int) channel,
                                  (int) sample,
                                  jmap ((float) ramp, 0.0f, (float) spec.maximumBlockSize, 0.0f, 1.0f));
            }
        }

        return buffer;
    }

    void runTest() override
    {
        constexpr ProcessSpec spec { 44100.0, 512, 2 };
        constexpr auto numBlocks = 5;

        const auto wetBuffer = getRampBuffer (spec, Kind::up);
        const auto dryBuffer = getRampBuffer (spec, Kind::down);

        for (auto maxLatency : { 0, 100, 200, 512 })
        {
            beginTest ("Mixer can push multiple small buffers");
            {
                DryWetMixer<float> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push samples one-by-one
                    for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
                        mixer.pushDrySamples (AudioBlock<const float> (dryBuffer).getSubBlock (sample, 1));

                    // Mix wet samples in one go
                    auto outputBlock = wetBuffer;
                    mixer.mixWetSamples ({ outputBlock });

                    // The output block should contain the wet and dry samples averaged
                    for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        for (uint32_t channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((int) channel, (int) sample);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can pop multiple small buffers");
            {
                DryWetMixer<float> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push samples in one go
                    mixer.pushDrySamples ({ dryBuffer });

                    // Process wet samples one-by-one
                    for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        AudioBuffer<float> outputBlock ((int) spec.numChannels, 1);
                        AudioBlock<const float> (wetBuffer).getSubBlock (sample, 1).copyTo (outputBlock);
                        mixer.mixWetSamples ({ outputBlock });

                        // The output block should contain the wet and dry samples averaged
                        for (uint32_t channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((int) channel, 0);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can push and pop multiple small buffers");
            {
                DryWetMixer<float> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push dry samples and process wet samples one-by-one
                    for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        mixer.pushDrySamples (AudioBlock<const float> (dryBuffer).getSubBlock (sample, 1));

                        AudioBuffer<float> outputBlock ((int) spec.numChannels, 1);
                        AudioBlock<const float> (wetBuffer).getSubBlock (sample, 1).copyTo (outputBlock);
                        mixer.mixWetSamples ({ outputBlock });

                        // The output block should contain the wet and dry samples averaged
                        for (uint32_t channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((int) channel, 0);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }

            beginTest ("Mixer can push and pop full-sized blocks after encountering a shorter block");
            {
                DryWetMixer<float> mixer (maxLatency);
                mixer.setWetMixProportion (0.5f);
                mixer.prepare (spec);

                constexpr auto shortBlockLength = spec.maximumBlockSize / 2;
                AudioBuffer<float> shortBlock (spec.numChannels, shortBlockLength);
                mixer.pushDrySamples (AudioBlock<const float> (dryBuffer).getSubBlock (shortBlockLength));
                mixer.mixWetSamples ({ shortBlock });

                for (auto block = 0; block < numBlocks; ++block)
                {
                    // Push a full block of dry samples
                    mixer.pushDrySamples ({ dryBuffer });

                    // Mix a full block of wet samples
                    auto outputBlock = wetBuffer;
                    mixer.mixWetSamples ({ outputBlock });

                    // The output block should contain the wet and dry samples averaged
                    for (uint32_t sample = 0; sample < spec.maximumBlockSize; ++sample)
                    {
                        for (uint32_t channel = 0; channel < spec.numChannels; ++channel)
                        {
                            const auto outputValue = outputBlock.getSample ((int) channel, (int) sample);
                            expectWithinAbsoluteError (outputValue, 0.5f, 0.0001f);
                        }
                    }
                }
            }
        }
    }
};

static const DryWetMixerTests dryWetMixerTests;

#endif

} // namespace dsp
} // namespace juce
