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

/**
    This abstract base class is used by some AudioProcessorParameter helper classes.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API  AudioProcessorParameterWithID  : public HostedAudioProcessorParameter
{
public:
    /** The creation of this object requires providing a name and ID which will be
        constant for its lifetime.

        @param parameterID          Used to uniquely identify the parameter
        @param parameterName        The user-facing name of the parameter
        @param parameterLabel       An optional label for the parameter's value
        @param parameterCategory    The semantics of this parameter
        @param versionHint          See AudioProcessorParameter::getVersionHint()
    */
    AudioProcessorParameterWithID (const String& parameterID,
                                   const String& parameterName,
                                   const String& parameterLabel = {},
                                   Category parameterCategory = AudioProcessorParameter::genericParameter,
                                   int versionHint = 0);

    /** Provides access to the parameter's ID string. */
    const String paramID;

    /** Provides access to the parameter's name. */
    const String name;

    /** Provides access to the parameter's label. */
    const String label;

    /** Provides access to the parameter's category. */
    const Category category;

    String getName (int) const override;
    String getLabel() const override;
    Category getCategory() const override;

    String getParameterID() const override { return paramID; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterWithID)
};

} // namespace juce
