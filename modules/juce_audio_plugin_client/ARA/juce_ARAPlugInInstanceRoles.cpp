#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

//==============================================================================

void ARAPlaybackRenderer::addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

void ARAPlaybackRenderer::removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

//==============================================================================

ARAEditorView::ARAEditorView (ARA::PlugIn::DocumentController* documentController) noexcept
: ARA::PlugIn::EditorView (documentController)
{}

void ARAEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
{
    for (Listener* l : listeners)
        l->onNewSelection (*currentSelection);
}

void ARAEditorView::doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept 
{
    for (Listener* l : listeners)
        l->onHideRegionSequences (reinterpret_cast<std::vector<ARARegionSequence*> const&> (regionSequences));
}

void ARAEditorView::addListener (Listener* l) 
{ 
    listeners.push_back (l); 
}

void ARAEditorView::removeListener (Listener* l) 
{ 
    ARA::find_erase (listeners, l); 
}

} // namespace juce
