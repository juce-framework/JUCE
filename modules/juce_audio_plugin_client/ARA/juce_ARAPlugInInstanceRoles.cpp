#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

ARAPlaybackRenderer::ARAPlaybackRenderer (ARADocumentController* documentController)
: ARA::PlugIn::PlaybackRenderer (documentController),
  sampleRate (44100),
  maxSamplesPerBlock (1024)
#if ! JUCE_DISABLE_ASSERTIONS
, isPreparedToPlay (false)
#endif
{}

void ARAPlaybackRenderer::prepareToPlay (double newSampleRate, int newMaxSamplesPerBlock)
{
    jassert (! isPreparedToPlay);
    sampleRate = newSampleRate;
    maxSamplesPerBlock = newMaxSamplesPerBlock;
#if ! JUCE_DISABLE_ASSERTIONS
    isPreparedToPlay = true;
#endif
}

void ARAPlaybackRenderer::releaseResources()
{
    jassert (isPreparedToPlay);
#if ! JUCE_DISABLE_ASSERTIONS
    isPreparedToPlay = false;
#endif
}

void ARAPlaybackRenderer::processBlock (AudioBuffer<float>& buffer, int64 /*timeInSamples*/, bool /*isPlayingBack*/)
{
    jassert (isPreparedToPlay);
    jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());
    for (int c = 0; c < buffer.getNumChannels(); c++)
        FloatVectorOperations::clear (buffer.getArrayOfWritePointers()[c], buffer.getNumSamples());
}

void ARAPlaybackRenderer::addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}
void ARAPlaybackRenderer::removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

//==============================================================================

ARAEditorRenderer::ARAEditorRenderer (ARADocumentController* documentController)
: ARA::PlugIn::EditorRenderer (documentController)
{}

void ARAEditorRenderer::addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::EditorRenderer::addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}
void ARAEditorRenderer::removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::EditorRenderer::removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

void ARAEditorRenderer::addRegionSequence (ARARegionSequence* regionSequence) noexcept
{
    ARA::PlugIn::EditorRenderer::addRegionSequence (ARA::PlugIn::toRef (regionSequence));
}
void ARAEditorRenderer::removeRegionSequence (ARARegionSequence* regionSequence) noexcept
{
    ARA::PlugIn::EditorRenderer::removeRegionSequence (ARA::PlugIn::toRef (regionSequence));
}

//==============================================================================

ARAEditorView::ARAEditorView (ARA::PlugIn::DocumentController* documentController) noexcept
: ARA::PlugIn::EditorView (documentController)
{}

void ARAEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
{
    for (Listener* l : listeners)
        l->onNewSelection (getViewSelection());
}

void ARAEditorView::doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept 
{
    for (Listener* l : listeners)
        l->onHideRegionSequences (reinterpret_cast<std::vector<ARARegionSequence*> const&> (regionSequences));
}

void ARAEditorView::addSelectionListener (Listener* l) 
{ 
    listeners.push_back (l); 
}

void ARAEditorView::removeSelectionListener (Listener* l) 
{ 
    ARA::find_erase (listeners, l); 
}

} // namespace juce
