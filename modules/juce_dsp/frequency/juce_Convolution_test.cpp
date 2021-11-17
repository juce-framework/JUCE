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

#if JUCE_ENABLE_ALLOCATION_HOOKS
#define JUCE_FAIL_ON_ALLOCATION_IN_SCOPE const UnitTestAllocationChecker checker (*this)
#else
#define JUCE_FAIL_ON_ALLOCATION_IN_SCOPE
#endif

namespace juce
{
namespace dsp
{
namespace
{

class ConvolutionTest  : public UnitTest
{
    template <typename Callback>
    static void nTimes (int n, Callback&& callback)
    {
        for (auto i = 0; i < n; ++i)
            callback();
    }

    static AudioBuffer<float> makeRamp (int length)
    {
        AudioBuffer<float> result (1, length);
        result.clear();

        const auto writePtr = result.getWritePointer (0);
        std::fill (writePtr, writePtr + length, 1.0f);
        result.applyGainRamp (0, length, 1.0f, 0.0f);

        return result;
    }

    static AudioBuffer<float> makeStereoRamp (int length)
    {
        AudioBuffer<float> result (2, length);
        result.clear();

        auto** channels = result.getArrayOfWritePointers();
        std::for_each (channels, channels + result.getNumChannels(), [length] (auto* channel)
        {
            std::fill (channel, channel + length, 1.0f);
        });

        result.applyGainRamp (0, 0, length, 1.0f, 0.0f);
        result.applyGainRamp (1, 0, length, 0.0f, 1.0f);

        return result;
    }

    static void addDiracImpulse (const AudioBlock<float>& block)
    {
        block.clear();

        for (size_t channel = 0; channel != block.getNumChannels(); ++channel)
            block.setSample ((int) channel, 0, 1.0f);
    }

    void checkForNans (const AudioBlock<float>& block)
    {
        for (size_t channel = 0; channel != block.getNumChannels(); ++channel)
            for (size_t sample = 0; sample != block.getNumSamples(); ++sample)
                expect (! std::isnan (block.getSample ((int) channel, (int) sample)));
    }

    void checkAllChannelsNonZero (const AudioBlock<float>& block)
    {
        for (size_t i = 0; i != block.getNumChannels(); ++i)
        {
            const auto* channel = block.getChannelPointer (i);

            expect (std::any_of (channel, channel + block.getNumSamples(), [] (float sample)
            {
                return sample != 0.0f;
            }));
        }
    }

    template <typename T>
    void nonAllocatingExpectWithinAbsoluteError (const T& a, const T& b, const T& error)
    {
        expect (std::abs (a - b) < error);
    }

    enum class InitSequence { prepareThenLoad, loadThenPrepare };

    void checkLatency (const Convolution& convolution, const Convolution::Latency& latency)
    {
        const auto reportedLatency = convolution.getLatency();

        if (latency.latencyInSamples == 0)
            expect (reportedLatency == 0);

        expect (reportedLatency >= latency.latencyInSamples);
    }

    void checkLatency (const Convolution&, const Convolution::NonUniform&) {}

    template <typename ConvolutionConfig>
    void testConvolution (const ProcessSpec& spec,
                          const ConvolutionConfig& config,
                          const AudioBuffer<float>& ir,
                          double irSampleRate,
                          Convolution::Stereo stereo,
                          Convolution::Trim trim,
                          Convolution::Normalise normalise,
                          const AudioBlock<const float>& expectedResult,
                          InitSequence initSequence)
    {
        AudioBuffer<float> buffer (static_cast<int> (spec.numChannels),
                                   static_cast<int> (spec.maximumBlockSize));
        AudioBlock<float> block { buffer };
        ProcessContextReplacing<float> context { block };

        const auto numBlocksPerSecond = (int) std::ceil (spec.sampleRate / spec.maximumBlockSize);
        const auto numBlocksForImpulse = (int) std::ceil ((double) expectedResult.getNumSamples() / spec.maximumBlockSize);

        AudioBuffer<float> outBuffer (static_cast<int> (spec.numChannels),
                                      numBlocksForImpulse * static_cast<int> (spec.maximumBlockSize));

        Convolution convolution (config);

        auto copiedIr = ir;

        if (initSequence == InitSequence::loadThenPrepare)
            convolution.loadImpulseResponse (std::move (copiedIr), irSampleRate, stereo, trim, normalise);

        convolution.prepare (spec);

        JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

        if (initSequence == InitSequence::prepareThenLoad)
            convolution.loadImpulseResponse (std::move (copiedIr), irSampleRate, stereo, trim, normalise);

        checkLatency (convolution, config);

        auto processBlocksWithDiracImpulse = [&]
        {
            for (auto i = 0; i != numBlocksForImpulse; ++i)
            {
                if (i == 0)
                    addDiracImpulse (block);
                else
                    block.clear();

                convolution.process (context);

                for (auto c = 0; c != static_cast<int> (spec.numChannels); ++c)
                {
                    outBuffer.copyFrom (c,
                                        i * static_cast<int> (spec.maximumBlockSize),
                                        block.getChannelPointer (static_cast<size_t> (c)),
                                        static_cast<int> (spec.maximumBlockSize));
                }
            }
        };

        // If we load an IR while the convolution is already running, we'll need to wait
        // for it to be loaded on a background thread
        if (initSequence == InitSequence::prepareThenLoad)
        {
            const auto time = Time::getMillisecondCounter();

            // Wait 10 seconds to load the impulse response
            while (Time::getMillisecondCounter() - time < 10'000)
            {
                processBlocksWithDiracImpulse();

                // Check if the impulse response was loaded
                if (block.getSample (0, 1) != 0.0f)
                    break;
            }
        }

        // At this point, our convolution should be loaded and the current IR size should
        // match the expected result size
        expect (convolution.getCurrentIRSize() == static_cast<int> (expectedResult.getNumSamples()));

        // Make sure we get any smoothing out of the way
        nTimes (numBlocksPerSecond, processBlocksWithDiracImpulse);

        nTimes (5, [&]
        {
            processBlocksWithDiracImpulse();

            const auto actualLatency = static_cast<size_t> (convolution.getLatency());

            // The output should be the same as the IR
            for (size_t c = 0; c != static_cast<size_t> (expectedResult.getNumChannels()); ++c)
            {
                for (size_t i = 0; i != static_cast<size_t> (expectedResult.getNumSamples()); ++i)
                {
                    const auto equivalentSample = i + actualLatency;

                    if (static_cast<int> (equivalentSample) >= outBuffer.getNumSamples())
                        continue;

                    nonAllocatingExpectWithinAbsoluteError (outBuffer.getSample ((int) c, (int) equivalentSample),
                                                            expectedResult.getSample ((int) c, (int) i),
                                                            0.01f);
                }
            }
        });
    }

    template <typename ConvolutionConfig>
    void testConvolution (const ProcessSpec& spec,
                          const ConvolutionConfig& config,
                          const AudioBuffer<float>& ir,
                          double irSampleRate,
                          Convolution::Stereo stereo,
                          Convolution::Trim trim,
                          Convolution::Normalise normalise,
                          const AudioBlock<const float>& expectedResult)
    {
        for (const auto sequence : { InitSequence::prepareThenLoad, InitSequence::loadThenPrepare })
            testConvolution (spec, config, ir, irSampleRate, stereo, trim, normalise, expectedResult, sequence);
    }

public:
    ConvolutionTest()
        : UnitTest ("Convolution", UnitTestCategories::dsp)
    {}

    void runTest() override
    {
        const ProcessSpec spec { 44100.0, 512, 2 };
        AudioBuffer<float> buffer (static_cast<int> (spec.numChannels),
                                   static_cast<int> (spec.maximumBlockSize));
        AudioBlock<float> block { buffer };
        ProcessContextReplacing<float> context { block };

        const auto impulseData = []
        {
            Random random;
            AudioBuffer<float> result (2, 1000);

            for (auto channel = 0; channel != result.getNumChannels(); ++channel)
                for (auto sample = 0; sample != result.getNumSamples(); ++sample)
                    result.setSample (channel, sample, random.nextFloat());

            return result;
        }();

        beginTest ("Impulse responses can be loaded without allocating on the audio thread");
        {
            Convolution convolution;
            convolution.prepare (spec);

            auto copy = impulseData;

            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            nTimes (100, [&]
            {
                convolution.loadImpulseResponse (std::move (copy),
                                                 1000,
                                                 Convolution::Stereo::yes,
                                                 Convolution::Trim::yes,
                                                 Convolution::Normalise::no);
                addDiracImpulse (block);
                convolution.process (context);
                checkForNans (block);
            });
        }

        beginTest ("Convolution can be reset without allocating on the audio thread");
        {
            Convolution convolution;
            convolution.prepare (spec);

            auto copy = impulseData;

            convolution.loadImpulseResponse (std::move (copy),
                                             1000,
                                             Convolution::Stereo::yes,
                                             Convolution::Trim::yes,
                                             Convolution::Normalise::yes);

            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            nTimes (100, [&]
            {
                addDiracImpulse (block);
                convolution.reset();
                convolution.process (context);
                convolution.reset();
            });

            checkForNans (block);
        }

        beginTest ("Completely empty IRs don't crash");
        {
            AudioBuffer<float> emptyBuffer;

            Convolution convolution;
            convolution.prepare (spec);

            auto copy = impulseData;

            convolution.loadImpulseResponse (std::move (copy),
                                             2000,
                                             Convolution::Stereo::yes,
                                             Convolution::Trim::yes,
                                             Convolution::Normalise::yes);

            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            nTimes (100, [&]
            {
                addDiracImpulse (block);
                convolution.reset();
                convolution.process (context);
                convolution.reset();
            });

            checkForNans (block);
        }

        beginTest ("Convolutions can cope with a change in samplerate and blocksize");
        {
            Convolution convolution;

            auto copy = impulseData;
            convolution.loadImpulseResponse (std::move (copy),
                                             2000,
                                             Convolution::Stereo::yes,
                                             Convolution::Trim::no,
                                             Convolution::Normalise::yes);

            const dsp::ProcessSpec specs[] = { { 96'000.0, 1024, 2 },
                                               { 48'000.0, 512, 2 },
                                               { 44'100.0, 256, 2 } };

            for (const auto& thisSpec : specs)
            {
                convolution.prepare (thisSpec);

                expectWithinAbsoluteError ((double) convolution.getCurrentIRSize(),
                                           thisSpec.sampleRate * 0.5,
                                           1.0);

                juce::AudioBuffer<float> thisBuffer ((int) thisSpec.numChannels,
                                                     (int) thisSpec.maximumBlockSize);
                AudioBlock<float> thisBlock { thisBuffer };
                ProcessContextReplacing<float> thisContext { thisBlock };

                nTimes (100, [&]
                {
                    addDiracImpulse (thisBlock);
                    convolution.process (thisContext);

                    checkForNans (thisBlock);
                    checkAllChannelsNonZero (thisBlock);
                });
            }
        }

        beginTest ("Short uniform convolutions work");
        {
            const auto ramp = makeRamp (static_cast<int> (spec.maximumBlockSize) / 2);
            testConvolution (spec,
                             Convolution::Latency { 0 },
                             ramp,
                             spec.sampleRate,
                             Convolution::Stereo::yes,
                             Convolution::Trim::yes,
                             Convolution::Normalise::no,
                             ramp);
        }

        beginTest ("Longer uniform convolutions work");
        {
            const auto ramp = makeRamp (static_cast<int> (spec.maximumBlockSize) * 8);
            testConvolution (spec,
                             Convolution::Latency { 0 },
                             ramp,
                             spec.sampleRate,
                             Convolution::Stereo::yes,
                             Convolution::Trim::yes,
                             Convolution::Normalise::no,
                             ramp);
        }

        beginTest ("Normalisation works");
        {
            const auto ramp = makeRamp (static_cast<int> (spec.maximumBlockSize) * 8);

            auto copy = ramp;
            const auto channels = copy.getArrayOfWritePointers();
            const auto numChannels = copy.getNumChannels();
            const auto numSamples = copy.getNumSamples();

            const auto factor = 0.125f / std::sqrt (std::accumulate (channels, channels + numChannels, 0.0f,
                                                                     [numSamples] (auto max, auto* channel)
            {
                return juce::jmax (max, std::accumulate (channel, channel + numSamples, 0.0f,
                                                         [] (auto sum, auto sample)
                {
                    return sum + sample * sample;
                }));
            }));

            std::for_each (channels, channels + numChannels, [factor, numSamples] (auto* channel)
            {
                FloatVectorOperations::multiply (channel, factor, numSamples);
            });

            testConvolution (spec,
                             Convolution::Latency { 0 },
                             ramp,
                             spec.sampleRate,
                             Convolution::Stereo::yes,
                             Convolution::Trim::yes,
                             Convolution::Normalise::yes,
                             copy);
        }

        beginTest ("Stereo convolutions work");
        {
            const auto ramp = makeStereoRamp (static_cast<int> (spec.maximumBlockSize) * 5);
            testConvolution (spec,
                             Convolution::Latency { 0 },
                             ramp,
                             spec.sampleRate,
                             Convolution::Stereo::yes,
                             Convolution::Trim::yes,
                             Convolution::Normalise::no,
                             ramp);
        }

        beginTest ("Stereo IRs only use first channel if stereo is disabled");
        {
            const auto length = static_cast<int> (spec.maximumBlockSize) * 5;
            const auto ramp = makeStereoRamp (length);

            const float* channels[] { ramp.getReadPointer (0), ramp.getReadPointer (0) };

            testConvolution (spec,
                             Convolution::Latency { 0 },
                             ramp,
                             spec.sampleRate,
                             Convolution::Stereo::no,
                             Convolution::Trim::yes,
                             Convolution::Normalise::no,
                             AudioBlock<const float> (channels, numElementsInArray (channels), length));
        }

        beginTest ("IRs with extra silence are trimmed appropriately");
        {
            const auto length = static_cast<int> (spec.maximumBlockSize) * 3;
            const auto ramp = makeRamp (length);
            AudioBuffer<float> paddedRamp (ramp.getNumChannels(), ramp.getNumSamples() * 2);
            paddedRamp.clear();

            const auto offset = (paddedRamp.getNumSamples() - ramp.getNumSamples()) / 2;

            for (auto channel = 0; channel != ramp.getNumChannels(); ++channel)
                paddedRamp.copyFrom (channel, offset, ramp.getReadPointer (channel), length);

            testConvolution (spec,
                             Convolution::Latency { 0 },
                             paddedRamp,
                             spec.sampleRate,
                             Convolution::Stereo::no,
                             Convolution::Trim::yes,
                             Convolution::Normalise::no,
                             ramp);
        }

        beginTest ("IRs are resampled if their sample rate is different to the playback rate");
        {
            for (const auto resampleRatio : { 0.1, 0.5, 2.0, 10.0 })
            {
                const auto length = static_cast<int> (spec.maximumBlockSize) * 2;
                const auto ramp = makeStereoRamp (length);

                const auto resampled = [&]
                {
                    AudioBuffer<float> original = ramp;
                    MemoryAudioSource memorySource (original, false);
                    ResamplingAudioSource resamplingSource (&memorySource, false, original.getNumChannels());

                    const auto finalSize = roundToInt (original.getNumSamples() / resampleRatio);
                    resamplingSource.setResamplingRatio (resampleRatio);
                    resamplingSource.prepareToPlay (finalSize, spec.sampleRate * resampleRatio);

                    AudioBuffer<float> result (original.getNumChannels(), finalSize);
                    resamplingSource.getNextAudioBlock ({ &result, 0, result.getNumSamples() });

                    result.applyGain ((float) resampleRatio);

                    return result;
                }();

                testConvolution (spec,
                                 Convolution::Latency { 0 },
                                 ramp,
                                 spec.sampleRate * resampleRatio,
                                 Convolution::Stereo::yes,
                                 Convolution::Trim::yes,
                                 Convolution::Normalise::no,
                                 resampled);
            }
        }

        beginTest ("Non-uniform convolutions work");
        {
            const auto ramp = makeRamp (static_cast<int> (spec.maximumBlockSize) * 8);

            for (auto headSize : { spec.maximumBlockSize / 2, spec.maximumBlockSize, spec.maximumBlockSize * 9 })
            {
                testConvolution (spec,
                                 Convolution::NonUniform { static_cast<int> (headSize) },
                                 ramp,
                                 spec.sampleRate,
                                 Convolution::Stereo::yes,
                                 Convolution::Trim::yes,
                                 Convolution::Normalise::no,
                                 ramp);
            }
        }

        beginTest ("Convolutions with latency work");
        {
            const auto ramp = makeRamp (static_cast<int> (spec.maximumBlockSize) * 8);
            using BlockSize = decltype (spec.maximumBlockSize);

            for (auto latency : { static_cast<BlockSize> (0),
                                  spec.maximumBlockSize / 3,
                                  spec.maximumBlockSize,
                                  spec.maximumBlockSize * 2,
                                  static_cast<BlockSize> (spec.maximumBlockSize * 2.5) })
            {
                testConvolution (spec,
                                 Convolution::Latency { static_cast<int> (latency) },
                                 ramp,
                                 spec.sampleRate,
                                 Convolution::Stereo::yes,
                                 Convolution::Trim::yes,
                                 Convolution::Normalise::no,
                                 ramp);
            }
        }
    }
};

ConvolutionTest convolutionUnitTest;

}
}
}

#undef JUCE_FAIL_ON_ALLOCATION_IN_SCOPE
