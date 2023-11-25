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

#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

bool ARARenderer::processBlock ([[maybe_unused]] AudioBuffer<double>& buffer,
                                [[maybe_unused]] AudioProcessor::Realtime realtime,
                                [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    // If you hit this assertion then either the caller called the double
    // precision version of processBlock on a processor which does not support it
    // (i.e. supportsDoublePrecisionProcessing() returns false), or the implementation
    // of the ARARenderer forgot to override the double precision version of this method
    jassertfalse;

    return false;
}

void ARARenderer::prepareToPlay ([[maybe_unused]] double sampleRate,
                                 [[maybe_unused]] int maximumSamplesPerBlock,
                                 [[maybe_unused]] int numChannels,
                                 [[maybe_unused]] AudioProcessor::ProcessingPrecision precision,
                                 [[maybe_unused]] AlwaysNonRealtime alwaysNonRealtime) {}

//==============================================================================
#if ARA_VALIDATE_API_CALLS
void ARAPlaybackRenderer::addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    if (araExtension)
        ARA_VALIDATE_API_STATE (! araExtension->isPrepared);

    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (playbackRegionRef);
}

void ARAPlaybackRenderer::removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    if (araExtension)
        ARA_VALIDATE_API_STATE (! araExtension->isPrepared);

    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (playbackRegionRef);
}
#endif

bool ARAPlaybackRenderer::processBlock ([[maybe_unused]] AudioBuffer<float>& buffer,
                                        [[maybe_unused]] AudioProcessor::Realtime realtime,
                                        [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    return false;
}

//==============================================================================
bool ARAEditorRenderer::processBlock ([[maybe_unused]] AudioBuffer<float>& buffer,
                                      [[maybe_unused]] AudioProcessor::Realtime isNonRealtime,
                                      [[maybe_unused]] const AudioPlayHead::PositionInfo& positionInfo) noexcept
{
    return true;
}

//==============================================================================
void ARAEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* viewSelection) noexcept
{
    listeners.call ([&] (Listener& l)
    {
        l.onNewSelection (*viewSelection);
    });
}

void ARAEditorView::doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept
{
    listeners.call ([&] (Listener& l)
    {
        l.onHideRegionSequences (ARA::vector_cast<ARARegionSequence*> (regionSequences));
    });
}

void ARAEditorView::addListener (Listener* l)
{
    listeners.add (l);
}

void ARAEditorView::removeListener (Listener* l)
{
    listeners.remove (l);
}

void ARAEditorView::Listener::onNewSelection ([[maybe_unused]] const ARAViewSelection& viewSelection) {}
void ARAEditorView::Listener::onHideRegionSequences ([[maybe_unused]] const std::vector<ARARegionSequence*>& regionSequences) {}

} // namespace juce
