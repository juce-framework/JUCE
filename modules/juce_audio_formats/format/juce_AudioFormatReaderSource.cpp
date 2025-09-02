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

namespace juce
{

AudioFormatReaderSource::AudioFormatReaderSource (AudioFormatReader* const r,
                                                  const bool deleteReaderWhenThisIsDeleted)
    : reader (r, deleteReaderWhenThisIsDeleted),
      nextPlayPos (0),
      looping (false)
{
    jassert (reader != nullptr);
}

AudioFormatReaderSource::~AudioFormatReaderSource() {}

int64 AudioFormatReaderSource::getTotalLength() const                   { return reader->lengthInSamples; }
void AudioFormatReaderSource::setNextReadPosition (int64 newPosition)   { nextPlayPos = newPosition; }
void AudioFormatReaderSource::setLooping (bool shouldLoop)              { looping = shouldLoop; }

int64 AudioFormatReaderSource::getNextReadPosition() const
{
    return looping ? nextPlayPos % reader->lengthInSamples
                   : nextPlayPos;
}

void AudioFormatReaderSource::prepareToPlay (int /*samplesPerBlockExpected*/, double /*sampleRate*/) {}
void AudioFormatReaderSource::releaseResources() {}

void AudioFormatReaderSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples <= 0)
        return;

    for (auto destOffset = 0; destOffset < info.numSamples;)
    {
        const auto readFrom = looping ? nextPlayPos % reader->lengthInSamples : nextPlayPos;

        const auto numSamplesToRead = jlimit ((int64) 0,
                                              (int64) (info.numSamples - destOffset),
                                              reader->lengthInSamples - readFrom);

        reader->read (info.buffer, info.startSample + destOffset,
                      (int) numSamplesToRead, readFrom, true, true);

        destOffset += (int) numSamplesToRead;
        nextPlayPos += numSamplesToRead;

        if (! looping)
        {
            const auto numSamplesToClear = info.numSamples - destOffset;
            info.buffer->clear (info.startSample + destOffset, numSamplesToClear);

            destOffset += numSamplesToClear;
            nextPlayPos += numSamplesToClear;
        }
    }
}

#if JUCE_UNIT_TESTS

struct AudioFormatReaderSourceTests : public UnitTest
{
    AudioFormatReaderSourceTests()
        : UnitTest ("AudioFormatReaderSource", UnitTestCategories::audio)
    {}

    //==============================================================================
    struct GetNextAudioBlockTestParams
    {
        int audioFormatReaderLength;
        int readFrom;
        int numSamplesToRead;
        bool enableLooping;
    };

    static void mockReadSamples (float* dest, int64 audioFormatReaderLength, int64 readFrom, int numSamplesToRead)
    {
        for (auto i = readFrom; i < readFrom + numSamplesToRead; ++i)
        {
            *dest = i < audioFormatReaderLength ? 0.001f * (float) i : 0.0f;
            ++dest;
        }
    }

    static void createGetNextAudioBlockExpectedOutput (const GetNextAudioBlockTestParams& params,
                                                       std::vector<float>& expected)
    {
        for (auto i = params.readFrom, end = i + params.numSamplesToRead; i < end; ++i)
        {
            const auto expectedResult = params.enableLooping || i < params.audioFormatReaderLength
                                      ? 0.001f * (float) (i % params.audioFormatReaderLength)
                                      : 0.0f;

            expected.push_back (expectedResult);
        }
    }

    //==============================================================================
    struct TestAudioFormatReader : public AudioFormatReader
    {
        explicit TestAudioFormatReader (int audioFormatReaderLength)
            : AudioFormatReader { nullptr, "test_format" }
        {
            jassert (audioFormatReaderLength < 1000);

            lengthInSamples = (int64) audioFormatReaderLength;
            numChannels = 1;
            usesFloatingPointData = true;
            bitsPerSample = 32;
        }

        void readMaxLevels (int64, int64, Range<float>*, int) override { jassertfalse; }
        void readMaxLevels (int64, int64, float&, float&, float&, float&) override { jassertfalse; }

        AudioChannelSet getChannelLayout() override
        {
            return AudioChannelSet::mono();
        }

        bool readSamples (int* const* destChannels,
                          [[maybe_unused]] int numDestChannels,
                          int startOffsetInDestBuffer,
                          int64 startSampleInFile,
                          int numSamples) override
        {
            jassert (numDestChannels == 1);
            mockReadSamples (reinterpret_cast<float*> (*destChannels + startOffsetInDestBuffer),
                             lengthInSamples,
                             startSampleInFile,
                             numSamples);
            return true;
        }
    };

    static auto createTestAudioFormatReaderSource (const GetNextAudioBlockTestParams& params)
    {
        return AudioFormatReaderSource { new TestAudioFormatReader (params.audioFormatReaderLength), true };
    }

    static void getNextAudioBlock (AudioFormatReaderSource& source,
                                   const GetNextAudioBlockTestParams& params,
                                   std::vector<float>& result)
    {
        source.setLooping (params.enableLooping);
        source.setNextReadPosition (params.readFrom);

        AudioBuffer<float> buffer { 1, params.numSamplesToRead };
        AudioSourceChannelInfo info { &buffer, 0, buffer.getNumSamples() };

        source.getNextAudioBlock (info);

        result.insert (result.end(),
                       buffer.getReadPointer (0),
                       buffer.getReadPointer (0) + buffer.getNumSamples());
    }

    static auto createFailureMessage (const GetNextAudioBlockTestParams& params)
    {
        return String { "AudioFormatReaderSource::getNextAudioBlock() failed for "
                        "audioFormatReaderLength=%audioFormatReaderLength%, "
                        "readFrom=%readFrom%, "
                        "numSamplesToRead=%numSamplesToRead%, "
                        "enableLooping=%enableLooping%" }
                            .replace ("%audioFormatReaderLength%", String { params.audioFormatReaderLength })
                            .replace ("%readFrom%", String { params.readFrom })
                            .replace ("%numSamplesToRead%", String { params.numSamplesToRead })
                            .replace ("%enableLooping%", params.enableLooping ? "true" : "false");
    }

    void runTest() override
    {
        const auto predicate = [] (auto a, auto b) { return exactlyEqual (a, b); };

        const auto testGetNextAudioBlock = [this, &predicate] (const GetNextAudioBlockTestParams& params)
        {
            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<float> actual;
            getNextAudioBlock (uut, params, actual);

            std::vector<float> expected;
            createGetNextAudioBlockExpectedOutput (params, expected);

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params));
        };

        beginTest ("A buffer without looping is played once and followed by silence");
        {
            GetNextAudioBlockTestParams testParams { 32, 0, 48, false };
            testGetNextAudioBlock (testParams);
        }

        beginTest ("A buffer with looping is played multiple times");
        {
            GetNextAudioBlockTestParams params { 32, 0, 24, true };

            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<float> actual;
            std::vector<float> expected;
            const auto numReads = 4;

            for (auto i = 0; i < numReads; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params) + " numReads=" + String { numReads });
        }

        beginTest ("A buffer with looping, loops even if the blockSize is greater than the internal buffer");
        {
            GetNextAudioBlockTestParams testParams { 32, 16, 128, true };
            testGetNextAudioBlock (testParams);
        }

        beginTest ("Behavioural invariants hold even if we turn on looping after prior reads");
        {
            GetNextAudioBlockTestParams params { 32, 0, 24, false };

            auto uut = createTestAudioFormatReaderSource (params);
            std::vector<float> actual;
            std::vector<float> expected;

            for (auto i = 0; i < 4; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            params.enableLooping = true;

            for (auto i = 0; i < 4; ++i)
            {
                getNextAudioBlock (uut, params, actual);
                createGetNextAudioBlockExpectedOutput (params, expected);
                params.readFrom += params.numSamplesToRead;
            }

            expect (std::equal (expected.begin(), expected.end(), actual.begin(), actual.end(), predicate),
                    createFailureMessage (params));
        }

        beginTest ("Fuzzing: getNextAudioBlock() should return correct results for all possible inputs");
        {
            for (auto params : { GetNextAudioBlockTestParams { 32, 0,  32,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 32,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 32,  true  },
                                 GetNextAudioBlockTestParams { 32, 16, 48,  false },
                                 GetNextAudioBlockTestParams { 32, 16, 128, false },
                                 GetNextAudioBlockTestParams { 32, 16, 48,  true  },
                                 GetNextAudioBlockTestParams { 32, 16, 128, true  } })
            {
                testGetNextAudioBlock (params);
            }

            const Range<int> audioFormatReaderLengthRange { 16, 128 };
            const Range<int> startFromRange { 0, 128 };
            const Range<int> numSamplesRange { 0, 128 };

            auto r = getRandom();

            for (int i = 0; i < 100; ++i)
            {
                GetNextAudioBlockTestParams params { r.nextInt (audioFormatReaderLengthRange),
                                                     r.nextInt (startFromRange),
                                                     r.nextInt (numSamplesRange),
                                                     r.nextBool() };

                testGetNextAudioBlock (params);
            }
        }
    }
};

static AudioFormatReaderSourceTests audioFormatReaderSourceTests;

#endif

} // namespace juce
