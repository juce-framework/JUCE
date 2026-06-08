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

bool AudioPlayHead::FrameRate::operator== (const FrameRate& other) const
{
    const auto tie = [] (const FrameRate& x) { return std::tie (x.base, x.drop, x.pulldown); };
    return tie (*this) == tie (other);
}

bool AudioPlayHead::TimeSignature::operator== (const TimeSignature& other) const
{
    const auto tie = [] (auto& x) { return std::tie (x.numerator, x.denominator); };
    return tie (*this) == tie (other);
}

bool AudioPlayHead::LoopPoints::operator== (const LoopPoints& other) const
{
    const auto tie = [] (auto& x) { return std::tie (x.ppqStart, x.ppqEnd); };
    return tie (*this) == tie (other);
}

bool AudioPlayHead::CurrentPositionInfo::operator== (const CurrentPositionInfo& other) const noexcept
{
    const auto tie = [] (const CurrentPositionInfo& i)
    {
        return std::tie (i.timeInSamples,
                         i.ppqPosition,
                         i.editOriginTime,
                         i.ppqPositionOfLastBarStart,
                         i.frameRate,
                         i.isPlaying,
                         i.isRecording,
                         i.bpm,
                         i.timeSigNumerator,
                         i.timeSigDenominator,
                         i.ppqLoopStart,
                         i.ppqLoopEnd,
                         i.isLooping);
    };

    return tie (*this) == tie (other);
}

bool AudioPlayHead::PositionInfo::operator== (const PositionInfo& other) const noexcept
{
    const auto tie = [] (const PositionInfo& i)
    {
        return std::make_tuple (i.getTimeInSamples(),
                                i.getTimeInSeconds(),
                                i.getPpqPosition(),
                                i.getEditOriginTime(),
                                i.getPpqPositionOfLastBarStart(),
                                i.getFrameRate(),
                                i.getBarCount(),
                                i.getTimeSignature(),
                                i.getBpm(),
                                i.getLoopPoints(),
                                i.getHostTimeNs(),
                                i.getIsPlaying(),
                                i.getIsRecording(),
                                i.getIsLooping());
    };

    return tie (*this) == tie (other);
}

bool AudioPlayHead::canControlTransport()                                          { return false; }
void AudioPlayHead::transportPlay ([[maybe_unused]] bool shouldStartPlaying)       {}
void AudioPlayHead::transportRecord ([[maybe_unused]] bool shouldStartRecording)   {}
void AudioPlayHead::transportRewind()                                              {}

} // namespace juce
