/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#include "PluginARADocumentController.h"

void ARASampleProjectPlaybackRenderer::didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
    if (_audioSourceMap.count (audioSource) == 0)
    {
        if (_audioReadingThread == nullptr)
        {
            _audioReadingThread.reset (new TimeSliceThread("ARA Audio source buffering thread"));
            _audioReadingThread->startThread();
        }
        _audioSourceMap.emplace (audioSource, audioSource->createBufferingAudioSource (*_audioReadingThread.get(), 4096));
        _audioSourceMap[audioSource]->prepareToPlay (128, audioSource->getSampleRate());
    }
}

void ARASampleProjectPlaybackRenderer::renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack)
{
    if (!isPlayingBack)
    {
        for (int c = 0; c < buffer.getNumChannels(); c++)
            FloatVectorOperations::clear(buffer.getArrayOfWritePointers()[c], buffer.getNumSamples());
    }
    else
    {
        using namespace ARA;
        ARA::ARASamplePosition sampleEnd = samplePosition + buffer.getNumSamples();
        for (ARA::PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions())
        {
            ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
            if (_audioSourceMap.count (audioSource) == 0)
                continue;

            // render silence if access is currently disabled
            if (!audioSource->isSampleAccessEnabled())
                continue;

            // this simplified test code "rendering" only produces audio if sample rate and channel count match
            if ((audioSource->getChannelCount() != buffer.getNumChannels()) || (audioSource->getSampleRate() != sampleRate))
                continue;

            // evaluate region borders in song time, calculate sample range to copy in song time
            ARASamplePosition regionStartSample = playbackRegion->getStartInPlaybackSamples (sampleRate);
            if (sampleEnd <= regionStartSample)
                continue;

            ARASamplePosition regionEndSample = playbackRegion->getEndInPlaybackSamples (sampleRate);
            if (regionEndSample <= samplePosition)
                continue;

            ARASamplePosition startSongSample = std::max (regionStartSample, samplePosition);
            ARASamplePosition endSongSample = std::min (regionEndSample, sampleEnd);

            // calculate offset between song and audio source samples, clip at region borders in audio source samples
            // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
            ARASamplePosition offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

            ARASamplePosition startAvailableSourceSamples = std::max ((ARASamplePosition) 0, playbackRegion->getStartInAudioModificationSamples());
            ARASamplePosition endAvailableSourceSamples = std::min (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

            startSongSample = std::max (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
            endSongSample = std::min (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

            AudioSourceChannelInfo channelInfo (&buffer, startSongSample - samplePosition, endSongSample - startSongSample);
            _audioSourceMap[audioSource]->setNextReadPosition(startSongSample + offsetToPlaybackRegion);
            _audioSourceMap[audioSource]->getNextAudioBlock(channelInfo);
        }
    }
}

//==============================================================================

ARA::PlugIn::EditorView* ARASampleProjectDocumentController::doCreateEditorView() noexcept
{
    return new ARASampleProjectEditor (this);
}

ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this);
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
};


ARASampleProjectEditor::ARASampleProjectEditor (ARA::PlugIn::DocumentController* ctrl) noexcept
: ARA::PlugIn::EditorView (ctrl)
{
}

void ARASampleProjectEditor::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
{
    const ScopedLock lock (selectionLock);

    removeAllChildren();
    maxRegionSequenceLength = 0.0;
    regionSequenceViews.clear();
    for (auto regionSequence : getDocumentController()->getDocument()->getRegionSequences())
    {
        auto regSeqView = new AudioView (*regionSequence);
        // shows all RegionSequences, highlight ones in current selection.
        for (auto selectedRegionSequence : currentSelection->getRegionSequences())
        {
            if (regionSequence == selectedRegionSequence)
            {
                regSeqView->setIsSelected (true);
                break;
            }
        }
        addAndMakeVisible (regSeqView);
        regionSequenceViews.add (regSeqView);
        maxRegionSequenceLength = std::max (maxRegionSequenceLength, regSeqView->getStartInSecs() + regSeqView->getLengthInSecs());
    }
    resized();
}

void ARASampleProjectEditor::resized()
{
    int i = 0;
    const int width = getParentWidth();
    const int height = 80;
    for (auto v : regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs() / maxRegionSequenceLength;
        double normalizedLength = v->getLengthInSecs() / maxRegionSequenceLength;
        jassert(normalizedStartPos+normalizedLength <= 1.0);
        v->setBounds ((int) (width * normalizedStartPos), height * i, (int) (width * normalizedLength), height);
        i++;
    }
    setBounds (0, 0, width, height * i);
}
