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

MPESynthesiserBase::MPESynthesiserBase()
    : instrument (defaultInstrument)
{
    instrument.addListener (this);
}

MPESynthesiserBase::MPESynthesiserBase (MPEInstrument& inst)
    : instrument (inst)
{
    instrument.addListener (this);
}

//==============================================================================
MPEZoneLayout MPESynthesiserBase::getZoneLayout() const noexcept
{
    return instrument.getZoneLayout();
}

void MPESynthesiserBase::setZoneLayout (MPEZoneLayout newLayout)
{
    instrument.setZoneLayout (newLayout);
}

//==============================================================================
void MPESynthesiserBase::enableLegacyMode (int pitchbendRange, Range<int> channelRange)
{
    instrument.enableLegacyMode (pitchbendRange, channelRange);
}

bool MPESynthesiserBase::isLegacyModeEnabled() const noexcept
{
    return instrument.isLegacyModeEnabled();
}

Range<int> MPESynthesiserBase::getLegacyModeChannelRange() const noexcept
{
    return instrument.getLegacyModeChannelRange();
}

void MPESynthesiserBase::setLegacyModeChannelRange (Range<int> channelRange)
{
    instrument.setLegacyModeChannelRange (channelRange);
}

int MPESynthesiserBase::getLegacyModePitchbendRange() const noexcept
{
    return instrument.getLegacyModePitchbendRange();
}

void MPESynthesiserBase::setLegacyModePitchbendRange (int pitchbendRange)
{
    instrument.setLegacyModePitchbendRange (pitchbendRange);
}

//==============================================================================
void MPESynthesiserBase::setPressureTrackingMode (TrackingMode modeToUse)
{
    instrument.setPressureTrackingMode (modeToUse);
}

void MPESynthesiserBase::setPitchbendTrackingMode (TrackingMode modeToUse)
{
    instrument.setPitchbendTrackingMode (modeToUse);
}

void MPESynthesiserBase::setTimbreTrackingMode (TrackingMode modeToUse)
{
    instrument.setTimbreTrackingMode (modeToUse);
}

//==============================================================================
void MPESynthesiserBase::handleMidiEvent (const MidiMessage& m)
{
    instrument.processNextMidiEvent (m);
}

//==============================================================================
template <typename floatType>
void MPESynthesiserBase::renderNextBlock (AudioBuffer<floatType>& outputAudio,
                                          const MidiBuffer& inputMidi,
                                          int startSample,
                                          int numSamples)
{
    // you must set the sample rate before using this!
    jassert (! approximatelyEqual (sampleRate, 0.0));

    const ScopedLock sl (noteStateLock);

    auto prevSample = startSample;
    const auto endSample = startSample + numSamples;

    for (auto it = inputMidi.findNextSamplePosition (startSample); it != inputMidi.cend(); ++it)
    {
        const auto metadata = *it;

        if (metadata.samplePosition >= endSample)
            break;

        const auto smallBlockAllowed = (prevSample == startSample && ! subBlockSubdivisionIsStrict);
        const auto thisBlockSize = smallBlockAllowed ? 1 : minimumSubBlockSize;

        if (metadata.samplePosition >= prevSample + thisBlockSize)
        {
            renderNextSubBlock (outputAudio, prevSample, metadata.samplePosition - prevSample);
            prevSample = metadata.samplePosition;
        }

        handleMidiEvent (metadata.getMessage());
    }

    if (prevSample < endSample)
        renderNextSubBlock (outputAudio, prevSample, endSample - prevSample);
}

// explicit instantiation for supported float types:
template void MPESynthesiserBase::renderNextBlock<float> (AudioBuffer<float>&, const MidiBuffer&, int, int);
template void MPESynthesiserBase::renderNextBlock<double> (AudioBuffer<double>&, const MidiBuffer&, int, int);

//==============================================================================
void MPESynthesiserBase::setCurrentPlaybackSampleRate (const double newRate)
{
    if (! approximatelyEqual (sampleRate, newRate))
    {
        const ScopedLock sl (noteStateLock);
        instrument.releaseAllNotes();
        sampleRate = newRate;
    }
}

//==============================================================================
void MPESynthesiserBase::setMinimumRenderingSubdivisionSize (int numSamples, bool shouldBeStrict) noexcept
{
    jassert (numSamples > 0); // it wouldn't make much sense for this to be less than 1
    minimumSubBlockSize = numSamples;
    subBlockSubdivisionIsStrict = shouldBeStrict;
}

#if JUCE_UNIT_TESTS

namespace
{
    class MpeSynthesiserBaseTests final : public UnitTest
    {
        enum class CallbackKind { process, midi };

        struct StartAndLength
        {
            StartAndLength (int s, int l) : start (s), length (l) {}

            int start  = 0;
            int length = 0;

            std::tuple<const int&, const int&> tie() const noexcept { return std::tie (start, length); }

            bool operator== (const StartAndLength& other) const noexcept { return tie() == other.tie(); }
            bool operator!= (const StartAndLength& other) const noexcept { return tie() != other.tie(); }

            bool operator< (const StartAndLength& other) const noexcept { return tie() < other.tie(); }
        };

        struct Events
        {
            std::vector<StartAndLength> blocks;
            std::vector<MidiMessage> messages;
            std::vector<CallbackKind> order;
        };

        class MockSynthesiser final : public MPESynthesiserBase
        {
        public:
            Events events;

            void handleMidiEvent (const MidiMessage& m) override
            {
                events.messages.emplace_back (m);
                events.order.emplace_back (CallbackKind::midi);
            }

        private:
            using MPESynthesiserBase::renderNextSubBlock;

            void renderNextSubBlock (AudioBuffer<float>&,
                                     int startSample,
                                     int numSamples) override
            {
                events.blocks.push_back ({ startSample, numSamples });
                events.order.emplace_back (CallbackKind::process);
            }
        };

        static MidiBuffer makeTestBuffer (const int bufferLength)
        {
            MidiBuffer result;

            for (int i = 0; i != bufferLength; ++i)
                result.addEvent ({}, i);

            return result;
        }

    public:
        MpeSynthesiserBaseTests()
            : UnitTest ("MPE Synthesiser Base", UnitTestCategories::midi) {}

        void runTest() override
        {
            const auto sumBlockLengths = [] (const std::vector<StartAndLength>& b)
            {
                const auto addBlock = [] (int acc, const StartAndLength& info) { return acc + info.length; };
                return std::accumulate (b.begin(), b.end(), 0, addBlock);
            };

            beginTest ("Rendering sparse subblocks works");
            {
                const int blockSize = 512;
                const auto midi = [&] { MidiBuffer b; b.addEvent ({}, blockSize / 2); return b; }();
                AudioBuffer<float> audio (1, blockSize);

                const auto processEvents = [&] (int start, int length)
                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (1, false);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, midi, start, length);
                    return synth.events;
                };

                {
                    const auto e = processEvents (0, blockSize);
                    expect (e.blocks.size() == 2);
                    expect (e.messages.size() == 1);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == blockSize);
                    expect (e.order == std::vector<CallbackKind> { CallbackKind::process,
                                                                   CallbackKind::midi,
                                                                   CallbackKind::process });
                }
            }

            beginTest ("Rendering subblocks processes only contained midi events");
            {
                const int blockSize = 512;
                const auto midi = makeTestBuffer (blockSize);
                AudioBuffer<float> audio (1, blockSize);

                const auto processEvents = [&] (int start, int length)
                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (1, false);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, midi, start, length);
                    return synth.events;
                };

                {
                    const int subBlockLength = 0;
                    const auto e = processEvents (0, subBlockLength);
                    expect (e.blocks.size() == 0);
                    expect (e.messages.size() == 0);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                }

                {
                    const int subBlockLength = 0;
                    const auto e = processEvents (1, subBlockLength);
                    expect (e.blocks.size() == 0);
                    expect (e.messages.size() == 0);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                }

                {
                    const int subBlockLength = 1;
                    const auto e = processEvents (1, subBlockLength);
                    expect (e.blocks.size() == 1);
                    expect (e.messages.size() == 1);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == subBlockLength);
                    expect (e.order == std::vector<CallbackKind> { CallbackKind::midi,
                                                                   CallbackKind::process });
                }

                {
                    const auto e = processEvents (0, blockSize);
                    expect (e.blocks.size() == blockSize);
                    expect (e.messages.size() == blockSize);
                    expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                    expect (sumBlockLengths (e.blocks) == blockSize);
                    expect (e.order.front() == CallbackKind::midi);
                }
            }

            beginTest ("Subblocks respect their minimum size");
            {
                const int blockSize = 512;
                const auto midi = makeTestBuffer (blockSize);
                AudioBuffer<float> audio (1, blockSize);

                const auto blockLengthsAreValid = [] (const std::vector<StartAndLength>& info, int minLength, bool strict)
                {
                    if (info.size() <= 1)
                        return true;

                    const auto lengthIsValid = [&] (const StartAndLength& s) { return minLength <= s.length; };
                    const auto begin = strict ? info.begin() : std::next (info.begin());
                    // The final block is allowed to be shorter than the minLength
                    return std::all_of (begin, std::prev (info.end()), lengthIsValid);
                };

                for (auto strict : { false, true })
                {
                    for (auto subblockSize : { 1, 16, 32, 64, 1024 })
                    {
                        MockSynthesiser synth;
                        synth.setMinimumRenderingSubdivisionSize (subblockSize, strict);
                        synth.setCurrentPlaybackSampleRate (44100);
                        synth.renderNextBlock (audio, midi, 0, blockSize);

                        const auto& e = synth.events;
                        expectWithinAbsoluteError (float (e.blocks.size()),
                                                   std::ceil ((float) blockSize / (float) subblockSize),
                                                   1.0f);
                        expect (e.messages.size() == blockSize);
                        expect (std::is_sorted (e.blocks.begin(), e.blocks.end()));
                        expect (sumBlockLengths (e.blocks) == blockSize);
                        expect (blockLengthsAreValid (e.blocks, subblockSize, strict));
                    }
                }

                {
                    MockSynthesiser synth;
                    synth.setMinimumRenderingSubdivisionSize (32, true);
                    synth.setCurrentPlaybackSampleRate (44100);
                    synth.renderNextBlock (audio, MidiBuffer{}, 0, 16);

                    expect (synth.events.blocks == std::vector<StartAndLength> { { 0, 16 } });
                    expect (synth.events.order == std::vector<CallbackKind> { CallbackKind::process });
                    expect (synth.events.messages.empty());
                }
            }
        }
    };

    MpeSynthesiserBaseTests mpeSynthesiserBaseTests;
}

#endif

} // namespace juce
