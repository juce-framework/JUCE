#include "juce_ARAPlugInInstanceRoles.h"

namespace juce
{

//==============================================================================

void ARARendererBase::prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock)
{
    sampleRate = newSampleRate;
    numChannels = newNumChannels;
    maxSamplesPerBlock = newMaxSamplesPerBlock;
    prepared = true;
}

void ARARendererBase::releaseResources()
{
    prepared = false;
}

//==============================================================================

void ARAPlaybackRenderer::prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool mayBeRealtime)
{
    ARARendererBase::prepareToPlay (newSampleRate, newNumChannels, newMaxSamplesPerBlock);
    preparedForRealtime = mayBeRealtime;
}

bool ARAPlaybackRenderer::processBlock (AudioBuffer<float>& buffer, int64 /*timeInSamples*/, bool /*isPlayingBack*/, bool isNonRealtime)
{
    jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());
    jassert (isNonRealtime || isPreparedForRealtime());
    buffer.clear();
    return true;
}

void ARAPlaybackRenderer::releaseResources()
{
    preparedForRealtime = false;
    ARARendererBase::releaseResources();
}

void ARAPlaybackRenderer::addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

void ARAPlaybackRenderer::removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
}

#if ARA_VALIDATE_API_CALLS
void ARAPlaybackRenderer::addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    ARA_VALIDATE_API_STATE (! isPrepared());
    ARA::PlugIn::PlaybackRenderer::addPlaybackRegion (playbackRegionRef);
}

void ARAPlaybackRenderer::removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept
{
    ARA_VALIDATE_API_STATE (! isPrepared());
    ARA::PlugIn::PlaybackRenderer::removePlaybackRegion (playbackRegionRef);
}
#endif

//==============================================================================

void ARAEditorRenderer::prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock)
{
    ARARendererBase::prepareToPlay (newSampleRate, newNumChannels, newMaxSamplesPerBlock);
}

bool ARAEditorRenderer::processBlock (AudioBuffer<float>& buffer, int64 /*timeInSamples*/, bool /*isPlayingBack*/)
{
    jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());
    return true;
}

void ARAEditorRenderer::releaseResources()
{
    ARAEditorRenderer::ARARendererBase::releaseResources();
}

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
        l.onHideRegionSequences (ARA::vector_cast<ARARegionSequence> (regionSequences));
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
