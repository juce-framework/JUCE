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

namespace juce
{

//==============================================================================
size_t ARADocument::getNumChildren() const noexcept
{
    return getMusicalContexts().size() + getRegionSequences().size() + getAudioSources().size();
}

ARAObject* ARADocument::getChild (size_t index)
{
    auto& musicalContexts = getMusicalContexts();

    if (index < musicalContexts.size())
        return musicalContexts[index];

    const auto numMusicalContexts = musicalContexts.size();
    auto& regionSequences = getRegionSequences();

    if (index < numMusicalContexts + regionSequences.size())
        return regionSequences[index - numMusicalContexts];

    const auto numMusicalContextsAndRegionSequences = numMusicalContexts + regionSequences.size();
    auto& audioSources = getAudioSources();

    if (index < numMusicalContextsAndRegionSequences + audioSources.size())
        return getAudioSources()[index - numMusicalContextsAndRegionSequences];

    return nullptr;
}

//==============================================================================
size_t ARARegionSequence::getNumChildren() const noexcept
{
    return 0;
}

ARAObject* ARARegionSequence::getChild (size_t)
{
    return nullptr;
}

Range<double> ARARegionSequence::getTimeRange (ARAPlaybackRegion::IncludeHeadAndTail includeHeadAndTail) const
{
    if (getPlaybackRegions().empty())
        return {};

    auto startTime = std::numeric_limits<double>::max();
    auto endTime = std::numeric_limits<double>::lowest();
    for (const auto& playbackRegion : getPlaybackRegions())
    {
        const auto regionTimeRange = playbackRegion->getTimeRange (includeHeadAndTail);
        startTime = jmin (startTime, regionTimeRange.getStart());
        endTime = jmax (endTime, regionTimeRange.getEnd());
    }
    return { startTime, endTime };
}

double ARARegionSequence::getCommonSampleRate() const
{
    const auto getSampleRate = [] (auto* playbackRegion)
    {
        return playbackRegion->getAudioModification()->getAudioSource()->getSampleRate();
    };

    const auto range = getPlaybackRegions();
    const auto sampleRate = range.size() > 0 ? getSampleRate (range.front()) : 0.0;

    if (std::any_of (range.begin(), range.end(), [&] (auto& x) { return getSampleRate (x) != sampleRate; }))
        return 0.0;

    return sampleRate;
}

//==============================================================================
size_t ARAAudioSource::getNumChildren() const noexcept
{
    return getAudioModifications().size();
}

ARAObject* ARAAudioSource::getChild (size_t index)
{
    return getAudioModifications()[index];
}

void ARAAudioSource::notifyAnalysisProgressStarted()
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressStarted (this);
}

void ARAAudioSource::notifyAnalysisProgressUpdated (float progress)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressUpdated (this, progress);
}

void ARAAudioSource::notifyAnalysisProgressCompleted()
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceAnalysisProgressCompleted (this);
}

void ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioSourceContentChanged (this,
                                                                                             scopeFlags,
                                                                                             notifyARAHost);
}

//==============================================================================
size_t ARAAudioModification::getNumChildren() const noexcept
{
    return getPlaybackRegions().size();
}

ARAObject* ARAAudioModification::getChild (size_t index)
{
    return getPlaybackRegions()[index];
}

void ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyAudioModificationContentChanged (this,
                                                                                                   scopeFlags,
                                                                                                   notifyARAHost);
}

//==============================================================================
ARAObject* ARAPlaybackRegion::getParent() { return getAudioModification(); }

Range<double> ARAPlaybackRegion::getTimeRange (IncludeHeadAndTail includeHeadAndTail) const
{
    auto startTime = getStartInPlaybackTime();
    auto endTime = getEndInPlaybackTime();

    if (includeHeadAndTail == IncludeHeadAndTail::yes)
    {
        ARA::ARATimeDuration headTime {}, tailTime {};
        getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
        startTime -= headTime;
        endTime += tailTime;
    }

    return { startTime, endTime };
}

Range<int64> ARAPlaybackRegion::getSampleRange (double sampleRate, IncludeHeadAndTail includeHeadAndTail) const
{
    const auto timeRange = getTimeRange (includeHeadAndTail);

    return { ARA::samplePositionAtTime (timeRange.getStart(), sampleRate),
             ARA::samplePositionAtTime (timeRange.getEnd(), sampleRate) };
}

double ARAPlaybackRegion::getHeadTime() const
{
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return headTime;
}

double ARAPlaybackRegion::getTailTime() const
{
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return tailTime;
}

void ARAPlaybackRegion::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController<ARADocumentController>()->internalNotifyPlaybackRegionContentChanged (this,
                                                                                                scopeFlags,
                                                                                                notifyARAHost);
}

//==============================================================================
void ARADocumentListener::willBeginEditing ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::didEndEditing ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::willNotifyModelUpdates ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::didNotifyModelUpdates ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::willUpdateDocumentProperties ([[maybe_unused]] ARADocument* document,
                                                        [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARADocumentProperties> newProperties) {}
void ARADocumentListener::didUpdateDocumentProperties ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::didAddMusicalContextToDocument ([[maybe_unused]] ARADocument* document,
                                                          [[maybe_unused]] ARAMusicalContext* musicalContext) {}
void ARADocumentListener::willRemoveMusicalContextFromDocument ([[maybe_unused]] ARADocument* document,
                                                                [[maybe_unused]] ARAMusicalContext* musicalContext) {}
void ARADocumentListener::didReorderMusicalContextsInDocument ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::didAddRegionSequenceToDocument ([[maybe_unused]] ARADocument* document,
                                                          [[maybe_unused]] ARARegionSequence* regionSequence) {}
void ARADocumentListener::willRemoveRegionSequenceFromDocument ([[maybe_unused]] ARADocument* document,
                                                                [[maybe_unused]] ARARegionSequence* regionSequence) {}
void ARADocumentListener::didReorderRegionSequencesInDocument ([[maybe_unused]] ARADocument* document) {}
void ARADocumentListener::didAddAudioSourceToDocument ([[maybe_unused]] ARADocument* document,
                                                       [[maybe_unused]] ARAAudioSource* audioSource) {}
void ARADocumentListener::willRemoveAudioSourceFromDocument ([[maybe_unused]] ARADocument* document,
                                                             [[maybe_unused]] ARAAudioSource* audioSource) {}
void ARADocumentListener::willDestroyDocument ([[maybe_unused]] ARADocument* document) {}

//==============================================================================
void ARAMusicalContextListener::willUpdateMusicalContextProperties ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAMusicalContextProperties> newProperties) {}
void ARAMusicalContextListener::didUpdateMusicalContextProperties ([[maybe_unused]] ARAMusicalContext* musicalContext) {}
void ARAMusicalContextListener::doUpdateMusicalContextContent ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                               [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
void ARAMusicalContextListener::didAddRegionSequenceToMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                      [[maybe_unused]] ARARegionSequence* regionSequence) {}
void ARAMusicalContextListener::willRemoveRegionSequenceFromMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext,
                                                                            [[maybe_unused]] ARARegionSequence* regionSequence) {}
void ARAMusicalContextListener::didReorderRegionSequencesInMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext) {}
void ARAMusicalContextListener::willDestroyMusicalContext ([[maybe_unused]] ARAMusicalContext* musicalContext) {}

//==============================================================================
void ARAPlaybackRegionListener::willUpdatePlaybackRegionProperties ([[maybe_unused]] ARAPlaybackRegion* playbackRegion,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) {}
void ARAPlaybackRegionListener::didUpdatePlaybackRegionProperties ([[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
void ARAPlaybackRegionListener::didUpdatePlaybackRegionContent ([[maybe_unused]] ARAPlaybackRegion* playbackRegion,
                                                                [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
void ARAPlaybackRegionListener::willDestroyPlaybackRegion ([[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}

//==============================================================================
void ARARegionSequenceListener::willUpdateRegionSequenceProperties ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                    [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) {}
void ARARegionSequenceListener::didUpdateRegionSequenceProperties ([[maybe_unused]] ARARegionSequence* regionSequence) {}
void ARARegionSequenceListener::willRemovePlaybackRegionFromRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                            [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
void ARARegionSequenceListener::didAddPlaybackRegionToRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence,
                                                                      [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
void ARARegionSequenceListener::willDestroyRegionSequence ([[maybe_unused]] ARARegionSequence* regionSequence) {}

//==============================================================================
void ARAAudioSourceListener::willUpdateAudioSourceProperties ([[maybe_unused]] ARAAudioSource* audioSource,
                                                              [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) {}
void ARAAudioSourceListener::didUpdateAudioSourceProperties ([[maybe_unused]] ARAAudioSource* audioSource) {}
void ARAAudioSourceListener::doUpdateAudioSourceContent ([[maybe_unused]] ARAAudioSource* audioSource,
                                                         [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
void ARAAudioSourceListener::didUpdateAudioSourceAnalysisProgress ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                   [[maybe_unused]] ARA::ARAAnalysisProgressState state,
                                                                   [[maybe_unused]] float progress) {}
void ARAAudioSourceListener::willEnableAudioSourceSamplesAccess ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                 [[maybe_unused]] bool enable) {}
void ARAAudioSourceListener::didEnableAudioSourceSamplesAccess ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                [[maybe_unused]] bool enable) {}
void ARAAudioSourceListener::willDeactivateAudioSourceForUndoHistory ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                      [[maybe_unused]] bool deactivate) {}
void ARAAudioSourceListener::didDeactivateAudioSourceForUndoHistory ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                     [[maybe_unused]] bool deactivate) {}
void ARAAudioSourceListener::didAddAudioModificationToAudioSource ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                   [[maybe_unused]] ARAAudioModification* audioModification) {}
void ARAAudioSourceListener::willRemoveAudioModificationFromAudioSource ([[maybe_unused]] ARAAudioSource* audioSource,
                                                                         [[maybe_unused]] ARAAudioModification* audioModification) {}
void ARAAudioSourceListener::willDestroyAudioSource ([[maybe_unused]] ARAAudioSource* audioSource) {}

//==============================================================================
void ARAAudioModificationListener::willUpdateAudioModificationProperties ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                          [[maybe_unused]] ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties> newProperties) {}
void ARAAudioModificationListener::didUpdateAudioModificationProperties ([[maybe_unused]] ARAAudioModification* audioModification) {}
void ARAAudioModificationListener::didUpdateAudioModificationContent ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                      [[maybe_unused]] ARAContentUpdateScopes scopeFlags) {}
void ARAAudioModificationListener::willDeactivateAudioModificationForUndoHistory ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                  [[maybe_unused]] bool deactivate) {}
void ARAAudioModificationListener::didDeactivateAudioModificationForUndoHistory ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                 [[maybe_unused]] bool deactivate) {}
void ARAAudioModificationListener::didAddPlaybackRegionToAudioModification ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                            [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
void ARAAudioModificationListener::willRemovePlaybackRegionFromAudioModification ([[maybe_unused]] ARAAudioModification* audioModification,
                                                                                  [[maybe_unused]] ARAPlaybackRegion* playbackRegion) {}
void ARAAudioModificationListener::willDestroyAudioModification ([[maybe_unused]] ARAAudioModification* audioModification) {}

} // namespace juce
