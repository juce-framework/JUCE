/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

RangedAudioParameter::RangedAudioParameter (const String& parameterID,
                                            const String& parameterName,
                                            const String& parameterLabel,
                                            Category parameterCategory)
    : AudioProcessorParameterWithID (parameterID, parameterName, parameterLabel, parameterCategory)
{
}

int RangedAudioParameter::getNumSteps() const
{
    const auto& range = getNormalisableRange();

    if (range.interval > 0)
        return (static_cast<int> ((range.end - range.start) / range.interval) + 1);

    return AudioProcessor::getDefaultNumParameterSteps();
}

float RangedAudioParameter::convertTo0to1 (float v) const noexcept
{
    const auto& range = getNormalisableRange();
    return range.convertTo0to1 (range.snapToLegalValue (v));
}

float RangedAudioParameter::convertFrom0to1 (float v) const noexcept
{
    const auto& range = getNormalisableRange();
    return range.snapToLegalValue (range.convertFrom0to1 (jlimit (0.0f, 1.0f, v)));
}

} // namespace juce
