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

#include "juce_AudioProcessor_ARAExtensions.h"

namespace juce
{

//==============================================================================
bool AudioProcessorARAExtension::getTailLengthSecondsForARA (double& tailLength) const
{
    if (! isBoundToARA())
        return false;

    tailLength = 0.0;

    if (auto playbackRenderer = getPlaybackRenderer())
        for (const auto& playbackRegion : playbackRenderer->getPlaybackRegions())
            tailLength = jmax (tailLength, playbackRegion->getTailTime());

    return true;
}

bool AudioProcessorARAExtension::prepareToPlayForARA (double sampleRate,
                                                      int samplesPerBlock,
                                                      int numChannels,
                                                      AudioProcessor::ProcessingPrecision precision)
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = true;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels, precision);

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels, precision);

    return true;
}

bool AudioProcessorARAExtension::releaseResourcesForARA()
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = false;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->releaseResources();

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->releaseResources();

    return true;
}

bool AudioProcessorARAExtension::processBlockForARA (AudioBuffer<float>& buffer,
                                                     AudioProcessor::Realtime realtime,
                                                     const AudioPlayHead::PositionInfo& positionInfo)
{
    // validate that the host has prepared us before processing
    ARA_VALIDATE_API_STATE (isPrepared);

    if (! isBoundToARA())
        return false;

    // Render our ARA playback regions for this buffer.
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->processBlock (buffer, realtime, positionInfo);

    // Render our ARA editor regions and sequences for this buffer.
    // This example does not support editor rendering and thus uses the default implementation,
    // which is a no-op and could be omitted in actual plug-ins to optimize performance.
    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->processBlock (buffer, realtime, positionInfo);

    return true;
}

bool AudioProcessorARAExtension::processBlockForARA (AudioBuffer<float>& buffer,
                                                     juce::AudioProcessor::Realtime realtime,
                                                     AudioPlayHead* playhead)
{
    return processBlockForARA (buffer,
                               realtime,
                               playhead != nullptr ? playhead->getPosition().orFallback (AudioPlayHead::PositionInfo{})
                                                   : AudioPlayHead::PositionInfo{});
}

//==============================================================================
void AudioProcessorARAExtension::didBindToARA() noexcept
{
    // validate that the ARA binding is not established by the host while prepared to play
#if ARA_VALIDATE_API_CALLS
    ARA_VALIDATE_API_STATE (! isPrepared);
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->araExtension = this;
#endif

#if (! JUCE_DISABLE_ASSERTIONS)
    // validate proper subclassing of the instance role classes
    if (auto playbackRenderer = getPlaybackRenderer())
        jassert (dynamic_cast<ARAPlaybackRenderer*> (playbackRenderer) != nullptr);
    if (auto editorRenderer = getEditorRenderer())
        jassert (dynamic_cast<ARAEditorRenderer*> (editorRenderer) != nullptr);
    if (auto editorView = getEditorView())
        jassert (dynamic_cast<ARAEditorView*> (editorView) != nullptr);
#endif
}

//==============================================================================

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
    : araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (true);
}

AudioProcessorEditorARAExtension::~AudioProcessorEditorARAExtension()
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (false);
}

} // namespace juce
