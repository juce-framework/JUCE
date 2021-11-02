#include "PlaybackRegionView.h"
#include "DocumentView.h"
#include "ARAPluginDemoAudioModification.h"
#include "ARAPluginDemoPlaybackRenderer.h"

//==============================================================================
PlaybackRegionView::PlaybackRegionView (RegionSequenceViewContainer& viewContainer, juce::ARAPlaybackRegion* region)
    : regionSequenceViewContainer (viewContainer),
      documentView (regionSequenceViewContainer.getDocumentView()),
      playbackRegion (region),
      audioThumb (128, documentView.getAudioFormatManger(), *sharedAudioThumbnailCache)
{
    audioThumb.addChangeListener (this);

    documentView.getARAEditorView()->addListener (this);
    onNewSelection (documentView.getARAEditorView()->getViewSelection());

    playbackRegion->getRegionSequence()->getDocument()->addListener (this);
    playbackRegion->getAudioModification()->addListener (this);
    playbackRegion->getAudioModification()->getAudioSource()->addListener (this);
    playbackRegion->addListener (this);

    recreatePlaybackRegionReader();
}

PlaybackRegionView::~PlaybackRegionView()
{
    documentView.getARAEditorView()->removeListener (this);

    playbackRegion->removeListener (this);
    playbackRegion->getAudioModification()->removeListener (this);
    playbackRegion->getAudioModification()->getAudioSource()->removeListener (this);
    playbackRegion->getRegionSequence()->getDocument()->removeListener (this);

    destroyPlaybackRegionReader();
    audioThumb.removeChangeListener (this);
}

void PlaybackRegionView::mouseDoubleClick (const juce::MouseEvent& /*event*/)
{
    // set the dim flag on our region's audio modification when double clicked
    auto audioModification = playbackRegion->getAudioModification<ARAPluginDemoAudioModification>();
    audioModification->setDimmed (! audioModification->isDimmed());

    // send a content change notification for the modification and all associated playback regions
    audioModification->notifyContentChanged (juce::ARAContentUpdateScopes::samplesAreAffected(), true);
    for (auto region : audioModification->getPlaybackRegions())
        region->notifyContentChanged (juce::ARAContentUpdateScopes::samplesAreAffected(), true);
}

void PlaybackRegionView::updateBounds()
{
    const auto regionTimeRange = getTimeRange();
    const auto& regionSequenceHeaderView = regionSequenceViewContainer.getRegionSequenceHeaderView();
    const int startX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getStart());
    const int endX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getEnd());
    const int width = juce::jmax (1, endX - startX);
    setBounds (startX, regionSequenceHeaderView.getY(), width, regionSequenceHeaderView.getHeight());
}

//==============================================================================
void PlaybackRegionView::paint (juce::Graphics& g)
{
    auto rect = getLocalBounds();

    if (rect.getWidth() > 2)
    {
        g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
        g.drawRect (rect);
        rect.reduce (1, 1);
    }

    const juce::Colour regionColour = juce::convertOptionalARAColour (playbackRegion->getEffectiveColor(), juce::Colours::black);
    g.setColour (regionColour);
    g.fillRect (rect);

    auto audioModification = playbackRegion->getAudioModification<ARAPluginDemoAudioModification>();
    if (audioModification->getAudioSource()->isSampleAccessEnabled())
    {
        auto clipBounds = g.getClipBounds();
        if (clipBounds.getWidth() > 0)
        {
            const auto convertedBounds = clipBounds + getBoundsInParent().getPosition();
            const double startTime = documentView.getPlaybackRegionsViewsTimeForX (convertedBounds.getX());
            const double endTime = documentView.getPlaybackRegionsViewsTimeForX (convertedBounds.getRight());

            const auto regionTimeRange = getTimeRange();

            auto drawBounds = getBounds() - getPosition();
            drawBounds.setHorizontalRange (clipBounds.getHorizontalRange());
            g.setColour (regionColour.contrasting (audioModification->isDimmed() ? 0.55f : 0.7f));
            audioThumb.drawChannels (g, drawBounds, startTime - regionTimeRange.getStart(), endTime - regionTimeRange.getStart(), 1.0f);
        }
    }
    else
    {
        g.setColour (regionColour.contrasting (1.0f));
        g.setFont (juce::Font (12.0f));
        g.drawText ("Access Disabled", getBounds(), juce::Justification::centred);
    }

    g.setColour (regionColour.contrasting (1.0f));
    g.setFont (juce::Font (12.0f));
    g.drawText (juce::convertOptionalARAString (playbackRegion->getEffectiveName()), rect, juce::Justification::topLeft);

    if (audioModification->isDimmed())
        g.drawText ("DIMMED", rect, juce::Justification::bottomLeft);
}

//==============================================================================
void PlaybackRegionView::changeListenerCallback (juce::ChangeBroadcaster* /*broadcaster*/)
{
    // our thumb nail has changed
    repaint();
}

void PlaybackRegionView::onNewSelection (const juce::ARAViewSelection& viewSelection)
{
    bool selected = ARA::contains (viewSelection.getPlaybackRegions(), playbackRegion);
    if (selected != isSelected)
    {
        isSelected = selected;
        repaint();
    }
}

void PlaybackRegionView::didEndEditing (juce::ARADocument* /*document*/)
{
    // our reader will pick up any changes in audio samples or region time range
    if (playbackRegionReader ==  nullptr || ! playbackRegionReader->isValid())
    {
        recreatePlaybackRegionReader();
        updateBounds();
        repaint();
    }
}

void PlaybackRegionView::willEnableAudioSourceSamplesAccess (juce::ARAAudioSource* /*audioSource*/, bool enable)
{
    // AudioThumbnail does not handle "pausing" access, so we clear it if any data is still pending, and recreate it when access is reenabled
    if (! enable && ! audioThumb.isFullyLoaded())
        destroyPlaybackRegionReader();
}

void PlaybackRegionView::didEnableAudioSourceSamplesAccess (juce::ARAAudioSource* /*audioSource*/, bool enable)
{
    // check whether we need to recreate the thumb data because it hadn't been loaded completely when access was disabled
    // (if we're inside a host edit cycle, we'll wait until it has completed to catch all changes in one update)
    if (enable && playbackRegionReader == nullptr && ! playbackRegion->getDocumentController()->isHostEditingDocument())
        recreatePlaybackRegionReader();

    repaint();
}

void PlaybackRegionView::willUpdateAudioSourceProperties (juce::ARAAudioSource* audioSource, juce::ARAAudioSource::PropertiesPtr newProperties)
{
    if (playbackRegion->getName() == nullptr && playbackRegion->getAudioModification()->getName() == nullptr && newProperties->name != audioSource->getName())
        repaint();
}

void PlaybackRegionView::willUpdateAudioModificationProperties (juce::ARAAudioModification* audioModification, juce::ARAAudioModification::PropertiesPtr newProperties)
{
    if (playbackRegion->getName() == nullptr && newProperties->name != audioModification->getName())
        repaint();
}

void PlaybackRegionView::willUpdatePlaybackRegionProperties (juce::ARAPlaybackRegion* /*playbackRegion*/, juce::ARAPlaybackRegion::PropertiesPtr newProperties)
{
    if (playbackRegion->getName() != newProperties->name || playbackRegion->getColor() != newProperties->color)
        repaint();

    if (playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime ||
        playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime)
        documentView.invalidateTimeRange();
}

void PlaybackRegionView::didUpdatePlaybackRegionContent (juce::ARAPlaybackRegion* /*playbackRegion*/, juce::ARAContentUpdateScopes scopeFlags)
{
    // Our reader catches this too, but we only check for its validity after host edits.
    // If the update is triggered inside the plug-in, we need to update the view from this call
    // (unless we're within a host edit already).
    if (scopeFlags.affectSamples() && ! playbackRegion->getDocumentController()->isHostEditingDocument())
    {
        recreatePlaybackRegionReader();
        repaint();
    }
}

//==============================================================================
void PlaybackRegionView::destroyPlaybackRegionReader()
{
    if (playbackRegionReader == nullptr)
        return;

    sharedAudioThumbnailCache->removeThumb (reinterpret_cast<intptr_t> (playbackRegionReader));
    audioThumb.clear();
    playbackRegionReader = nullptr;
}

void PlaybackRegionView::recreatePlaybackRegionReader()
{
    destroyPlaybackRegionReader();

    // Create a playback region reader for our region for our audio thumb
    playbackRegionReader = new juce::ARAPlaybackRegionReader (playbackRegion);
    audioThumb.setReader (playbackRegionReader, reinterpret_cast<intptr_t> (playbackRegionReader));

    // TODO JUCE_ARA see juce_AudioThumbnail.cpp, line 122: AudioThumbnail handles zero-length sources
    // by deleting the reader, therefore we must clear our "weak" pointer to the reader in this case.
    if (playbackRegionReader->lengthInSamples <= 0)
        playbackRegionReader = nullptr;

    // Update tooltip whenever updating the reader
    setTooltip ("Playback range " + juce::String (playbackRegion->getStartInPlaybackTime(), 3) + " .. " + juce::String (playbackRegion->getEndInPlaybackTime(), 3) + juce::newLine +
                "Audio Modification range " + juce::String (playbackRegion->getStartInAudioModificationTime(), 3) + " .. " + juce::String (playbackRegion->getEndInAudioModificationTime(), 3));
}
