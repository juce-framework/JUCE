/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

bool ARARenderer::processBlock (AudioBuffer<double>& buffer,
                                AudioProcessor::Realtime realtime,
                                const AudioPlayHead::CurrentPositionInfo& positionInfo) noexcept
{
    ignoreUnused (buffer, realtime, positionInfo);

    // If you hit this assertion then either the caller called the double
    // precision version of processBlock on a processor which does not support it
    // (i.e. supportsDoublePrecisionProcessing() returns false), or the implementation
    // of the ARARenderer forgot to override the double precision version of this method
    jassertfalse;

    return false;
}

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

} // namespace juce
