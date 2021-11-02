#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

//==============================================================================

// TODO JUCE_ARA while there is prepareToPlay()/releaseRessources() in AudioProcessor,
//       this state is not tracked and there is no getter for it.
//#if ARA_VALIDATE_API_CALLS
//void ARAPlaybackRenderer::addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
//{
//    ARA_VALIDATE_API_STATE (! getAudioProcessor->isPrepared());
//    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (playbackRegionRef);
//}
//
//void ARAPlaybackRenderer::removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
//{
//    ARA_VALIDATE_API_STATE (! getAudioProcessor->isPrepared());
//    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (playbackRegionRef);
//}
//#endif

//==============================================================================

void ARAEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* viewSelection) noexcept
{
    listeners.callExpectingUnregistration ([&] (Listener& l)
    {
        l.onNewSelection (*viewSelection);
    });
}

void ARAEditorView::doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept
{
    listeners.callExpectingUnregistration ([&] (Listener& l)
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
