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
