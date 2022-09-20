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

//==============================================================================
/**
    A parameter with functions that are useful for plugin hosts.

    @tags{Audio}
*/
struct HostedAudioProcessorParameter : public AudioProcessorParameter
{
    using AudioProcessorParameter::AudioProcessorParameter;

    /** Returns an ID that is unique to this parameter.

        Parameter indices are unstable across plugin versions, which means that the
        parameter found at a particular index in one version of a plugin might move
        to a different index in the subsequent version.

        Unlike the parameter index, the ID returned by this function should be
        somewhat stable (depending on the format of the plugin), so it is more
        suitable for storing/recalling automation data.
    */
    virtual String getParameterID() const = 0;
};

} // namespace juce
