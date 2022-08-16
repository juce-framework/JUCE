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

class ARADocumentControllerSpecialisation::ARADocumentControllerImpl  : public ARADocumentController,
                                                                        private juce::Timer
{
public:
    ARADocumentControllerImpl (const ARA::PlugIn::PlugInEntry* entry,
                               const ARA::ARADocumentControllerHostInstance* instance,
                               ARADocumentControllerSpecialisation* spec)
        : ARADocumentController (entry, instance), specialisation (spec)
    {
    }

    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    std::vector<PlaybackRenderer_t*> const& getPlaybackRenderers() const noexcept
    {
        return ARA::PlugIn::DocumentController::getPlaybackRenderers<PlaybackRenderer_t>();
    }

    template <typename EditorRenderer_t = ARAEditorRenderer>
    std::vector<EditorRenderer_t*> const& getEditorRenderers() const noexcept
    {
        return ARA::PlugIn::DocumentController::getEditorRenderers<EditorRenderer_t>();
    }

    template <typename EditorView_t = ARAEditorView>
    std::vector<EditorView_t*> const& getEditorViews() const noexcept
    {
        return ARA::PlugIn::DocumentController::getEditorViews<EditorView_t>();
    }

    auto getSpecialisation() { return specialisation; }

protected:
    //==============================================================================
    bool doRestoreObjectsFromStream (ARAInputStream& input, const ARARestoreObjectsFilter* filter) noexcept
    {
        return specialisation->doRestoreObjectsFromStream (input, filter);
    }

    bool doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept
    {
        return specialisation->doStoreObjectsToStream (output, filter);
    }

    //==============================================================================
    // Model object creation
    ARA::PlugIn::Document*          doCreateDocument          () noexcept override;
    ARA::PlugIn::MusicalContext*    doCreateMusicalContext    (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    ARA::PlugIn::RegionSequence*    doCreateRegionSequence    (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioSource*       doCreateAudioSource       (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    ARA::PlugIn::PlaybackRegion*    doCreatePlaybackRegion    (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    //==============================================================================
    // Plugin role implementation
    friend class ARAPlaybackRegionReader;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;
    ARA::PlugIn::EditorRenderer*   doCreateEditorRenderer() noexcept override;
    ARA::PlugIn::EditorView*       doCreateEditorView() noexcept override;

    //==============================================================================
    // ARAAudioSource content access
    bool doIsAudioSourceContentAvailable (const ARA::PlugIn::AudioSource* audioSource,
                                          ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetAudioSourceContentGrade (const ARA::PlugIn::AudioSource* audioSource,
                                                       ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreateAudioSourceContentReader (ARA::PlugIn::AudioSource* audioSource,
                                                                  ARA::ARAContentType type,
                                                                  const ARA::ARAContentTimeRange* range) noexcept override;

    //==============================================================================
    // ARAAudioModification content access
    bool doIsAudioModificationContentAvailable (const ARA::PlugIn::AudioModification* audioModification,
                                                ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetAudioModificationContentGrade (const ARA::PlugIn::AudioModification* audioModification,
                                                             ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreateAudioModificationContentReader (ARA::PlugIn::AudioModification* audioModification,
                                                                        ARA::ARAContentType type,
                                                                        const ARA::ARAContentTimeRange* range) noexcept override;

    //==============================================================================
    // ARAPlaybackRegion content access
    bool doIsPlaybackRegionContentAvailable (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                             ARA::ARAContentType type) noexcept override;
    ARA::ARAContentGrade doGetPlaybackRegionContentGrade (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                          ARA::ARAContentType type) noexcept override;
    ARA::PlugIn::ContentReader* doCreatePlaybackRegionContentReader (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                     ARA::ARAContentType type,
                                                                     const ARA::ARAContentTimeRange* range) noexcept override;

    //==============================================================================
    // ARAAudioSource analysis
    bool doIsAudioSourceContentAnalysisIncomplete (const ARA::PlugIn::AudioSource* audioSource,
                                                   ARA::ARAContentType type) noexcept override;
    void doRequestAudioSourceContentAnalysis (ARA::PlugIn::AudioSource* audioSource,
                                              std::vector<ARA::ARAContentType> const& contentTypes) noexcept override;

    //==============================================================================
    // Analysis Algorithm selection
    ARA::ARAInt32 doGetProcessingAlgorithmsCount() noexcept override;
    const ARA::ARAProcessingAlgorithmProperties* doGetProcessingAlgorithmProperties (ARA::ARAInt32 algorithmIndex) noexcept override;
    ARA::ARAInt32 doGetProcessingAlgorithmForAudioSource (const ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource,
                                                     ARA::ARAInt32 algorithmIndex) noexcept override;

#ifndef DOXYGEN

    //==============================================================================
    bool doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    //==============================================================================
    // Document notifications
    void willBeginEditing() noexcept override;
    void didEndEditing() noexcept override;
    void willNotifyModelUpdates() noexcept override;
    void didNotifyModelUpdates() noexcept override;
    void willUpdateDocumentProperties (ARA::PlugIn::Document* document, ARADocument::PropertiesPtr newProperties) noexcept override;
    void didUpdateDocumentProperties (ARA::PlugIn::Document* document) noexcept override;
    void didAddMusicalContextToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willRemoveMusicalContextFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void didReorderMusicalContextsInDocument (ARA::PlugIn::Document* document) noexcept override;
    void didAddRegionSequenceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willRemoveRegionSequenceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didReorderRegionSequencesInDocument (ARA::PlugIn::Document* document) noexcept override;
    void didAddAudioSourceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void willRemoveAudioSourceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void willDestroyDocument (ARA::PlugIn::Document* document) noexcept override;

    //==============================================================================
    // MusicalContext notifications
    void willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept override;
    void didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    void didAddRegionSequenceToMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willRemoveRegionSequenceFromMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didReorderRegionSequencesInMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    //==============================================================================
    // RegionSequence notifications, typically not overridden further
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;

    //==============================================================================
    // AudioSource notifications
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didAddAudioModificationToAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willRemoveAudioModificationFromAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    //==============================================================================
    // AudioModification notifications
    void willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void didDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    //==============================================================================
    // PlaybackRegion notifications
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    //==============================================================================
    // juce::Timer overrides
    void timerCallback() override;

public:
    //==============================================================================
    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource) override;

    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress) override;

    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource) override;

    /** @internal */
    void internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                       ARAAudioSource::ARAAnalysisProgressState state,
                                                       float progress) override;

    //==============================================================================
    /** @internal */
    void internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                  ARAContentUpdateScopes scopeFlags,
                                                  bool notifyARAHost) override;

    /** @internal */
    void internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                        ARAContentUpdateScopes scopeFlags,
                                                        bool notifyARAHost) override;

    /** @internal */
    void internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                     ARAContentUpdateScopes scopeFlags,
                                                     bool notifyARAHost) override;

#endif

private:
    //==============================================================================
    ARADocumentControllerSpecialisation* specialisation;
    std::atomic<bool> internalAnalysisProgressIsSynced { true };
    ScopedJuceInitialiser_GUI libraryInitialiser;
    int activeAudioSourcesCount = 0;

    //==============================================================================
    template <typename ModelObject, typename Function, typename... Ts>
    void notifyListeners (Function ModelObject::Listener::* function, ModelObject* modelObject, Ts... ts)
    {
        (specialisation->*function) (modelObject, ts...);
        modelObject->notifyListeners ([&] (auto& l)
                                       {
                                           try
                                           {
                                               (l.*function) (modelObject, ts...);
                                           }
                                           catch (...)
                                           {
                                           }
                                       });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentControllerImpl)
};

ARA::PlugIn::DocumentController* ARADocumentControllerSpecialisation::getDocumentController() noexcept
{
    return documentController.get();
}

//==============================================================================
void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressStarted, 0.0f))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressStarted (audioSource);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource,
                                                                                                                       float progress)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressUpdated,  progress))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressUpdated (audioSource, progress);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressCompleted, 1.0f))
        internalAnalysisProgressIsSynced.store (false, std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressCompleted (audioSource);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                                                                                   ARAAudioSource::ARAAnalysisProgressState state,
                                                                                                                   float progress)
{
    specialisation->didUpdateAudioSourceAnalysisProgress (audioSource, state, progress);
}

//==============================================================================
ARADocumentControllerSpecialisation* ARADocumentControllerSpecialisation::getSpecialisedDocumentControllerImpl (ARA::PlugIn::DocumentController* dc)
{
    return static_cast<ARADocumentControllerImpl*> (dc)->getSpecialisation();
}

ARADocument* ARADocumentControllerSpecialisation::getDocumentImpl()
{
    return documentController->getDocument();
}

//==============================================================================
// some helper macros to ease repeated declaration & implementation of notification functions below:
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments")

// no notification arguments
#define OVERRIDE_TO_NOTIFY_1(function, ModelObjectType, modelObject) \
    void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject)); \
    }

// single notification argument, model object version
#define OVERRIDE_TO_NOTIFY_2(function, ModelObjectType, modelObject, ArgumentType, argument) \
    void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject, ARA::PlugIn::ArgumentType argument) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject), static_cast<ARA##ArgumentType> (argument)); \
    }

// single notification argument, non-model object version
#define OVERRIDE_TO_NOTIFY_3(function, ModelObjectType, modelObject, ArgumentType, argument) \
    void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::function (ARA::PlugIn::ModelObjectType* modelObject, ArgumentType argument) noexcept \
    { \
        notifyListeners (&ARA##ModelObjectType::Listener::function, static_cast<ARA##ModelObjectType*> (modelObject), argument); \
    }

//==============================================================================
ARA::PlugIn::Document* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateDocument() noexcept
{
    auto* document = specialisation->doCreateDocument();

    // Your Document subclass must inherit from juce::ARADocument
    jassert (dynamic_cast<ARADocument*> (document));

    return document;
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willBeginEditing() noexcept
{
    notifyListeners (&ARADocument::Listener::willBeginEditing, static_cast<ARADocument*> (getDocument()));
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didEndEditing() noexcept
{
    notifyListeners (&ARADocument::Listener::didEndEditing, static_cast<ARADocument*> (getDocument()));

    if (isTimerRunning() && (activeAudioSourcesCount == 0))
        stopTimer();
    else if (! isTimerRunning() && (activeAudioSourcesCount > 0))
        startTimerHz (20);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willNotifyModelUpdates() noexcept
{
    notifyListeners (&ARADocument::Listener::willNotifyModelUpdates, static_cast<ARADocument*> (getDocument()));
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didNotifyModelUpdates() noexcept
{
    notifyListeners (&ARADocument::Listener::didNotifyModelUpdates, static_cast<ARADocument*> (getDocument()));
}

//==============================================================================
bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader,
                                                                                                  const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    ARAInputStream reader (archiveReader);
    return doRestoreObjectsFromStream (reader, filter);
}

bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter,
                                                                                              const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    ARAOutputStream writer (archiveWriter);
    return doStoreObjectsToStream (writer, filter);
}

//==============================================================================
OVERRIDE_TO_NOTIFY_3 (willUpdateDocumentProperties, Document, document, ARADocument::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateDocumentProperties, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddMusicalContextToDocument, Document, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_2 (willRemoveMusicalContextFromDocument, Document, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_1 (didReorderMusicalContextsInDocument, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToDocument, Document, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromDocument, Document, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInDocument, Document, document)
OVERRIDE_TO_NOTIFY_2 (didAddAudioSourceToDocument, Document, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioSourceFromDocument, Document, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_1 (willDestroyDocument, Document, document)

//==============================================================================
ARA::PlugIn::MusicalContext* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateMusicalContext (ARA::PlugIn::Document* document,
                                                                                                                     ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return specialisation->doCreateMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext,
                                                                                                    const ARA::ARAContentTimeRange*,
                                                                                                    ARA::ContentUpdateScopes flags) noexcept
{
    notifyListeners (&ARAMusicalContext::Listener::doUpdateMusicalContextContent,
                     static_cast<ARAMusicalContext*> (musicalContext),
                     flags);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateMusicalContextProperties, MusicalContext, musicalContext, ARAMusicalContext::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateMusicalContextProperties, MusicalContext, musicalContext)
OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToMusicalContext, MusicalContext, musicalContext, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromMusicalContext, MusicalContext, musicalContext, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInMusicalContext, MusicalContext, musicalContext)
OVERRIDE_TO_NOTIFY_1 (willDestroyMusicalContext, MusicalContext, musicalContext)

//==============================================================================
ARA::PlugIn::RegionSequence* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return specialisation->doCreateRegionSequence (static_cast<ARADocument*> (document), hostRef);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateRegionSequenceProperties, RegionSequence, regionSequence, ARARegionSequence::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateRegionSequenceProperties, RegionSequence, regionSequence)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToRegionSequence, RegionSequence, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromRegionSequence, RegionSequence, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyRegionSequence, RegionSequence, regionSequence)

//==============================================================================
ARA::PlugIn::AudioSource* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    ++activeAudioSourcesCount;
    return specialisation->doCreateAudioSource (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource,
                                                                                                 const ARA::ARAContentTimeRange*,
                                                                                                 ARA::ContentUpdateScopes flags) noexcept
{
    notifyListeners (&ARAAudioSource::Listener::doUpdateAudioSourceContent, static_cast<ARAAudioSource*> (audioSource), flags);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioSourceProperties, AudioSource, audioSource, ARAAudioSource::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioSourceProperties, AudioSource, audioSource)
OVERRIDE_TO_NOTIFY_3 (willEnableAudioSourceSamplesAccess, AudioSource, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_3 (didEnableAudioSourceSamplesAccess, AudioSource, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_2 (didAddAudioModificationToAudioSource, AudioSource, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioModificationFromAudioSource, AudioSource, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioSourceForUndoHistory, AudioSource, audioSource, bool, deactivate)

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource,
                                                                                                             bool deactivate) noexcept
{
    activeAudioSourcesCount += (deactivate ? -1 : 1);
    notifyListeners (&ARAAudioSource::Listener::didDeactivateAudioSourceForUndoHistory,
                     static_cast<ARAAudioSource*> (audioSource),
                     deactivate);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    if (! audioSource->isDeactivatedForUndoHistory())
        --activeAudioSourcesCount;

    notifyListeners (&ARAAudioSource::Listener::willDestroyAudioSource, static_cast<ARAAudioSource*> (audioSource));
}
//==============================================================================
ARA::PlugIn::AudioModification* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                           ARA::ARAAudioModificationHostRef hostRef,
                                                                                                                           const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return specialisation->doCreateAudioModification (static_cast<ARAAudioSource*> (audioSource),
                                                      hostRef,
                                                      static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioModificationProperties, AudioModification, audioModification, ARAAudioModification::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioModificationProperties, AudioModification, audioModification)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToAudioModification, AudioModification, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromAudioModification, AudioModification, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioModificationForUndoHistory, AudioModification, audioModification, bool, deactivate)
OVERRIDE_TO_NOTIFY_3 (didDeactivateAudioModificationForUndoHistory, AudioModification, audioModification, bool, deactivate)
OVERRIDE_TO_NOTIFY_1 (willDestroyAudioModification, AudioModification, audioModification)

//==============================================================================
ARA::PlugIn::PlaybackRegion* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification,
                                                                                                                     ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return specialisation->doCreatePlaybackRegion (static_cast<ARAAudioModification*> (modification), hostRef);
}

OVERRIDE_TO_NOTIFY_3 (willUpdatePlaybackRegionProperties, PlaybackRegion, playbackRegion, ARAPlaybackRegion::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdatePlaybackRegionProperties, PlaybackRegion, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyPlaybackRegion, PlaybackRegion, playbackRegion)

//==============================================================================
void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                                                                              ARAContentUpdateScopes scopeFlags,
                                                                                                              bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioSourceContentChanged (audioSource, scopeFlags);

    notifyListeners (&ARAAudioSource::Listener::doUpdateAudioSourceContent, audioSource, scopeFlags);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                                                                                    ARAContentUpdateScopes scopeFlags,
                                                                                                                    bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioModificationContentChanged (audioModification, scopeFlags);

    notifyListeners (&ARAAudioModification::Listener::didUpdateAudioModificationContent, audioModification, scopeFlags);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                                                                                 ARAContentUpdateScopes scopeFlags,
                                                                                                                 bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyPlaybackRegionContentChanged (playbackRegion, scopeFlags);

    notifyListeners (&ARAPlaybackRegion::Listener::didUpdatePlaybackRegionContent, playbackRegion, scopeFlags);
}

//==============================================================================
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#undef OVERRIDE_TO_NOTIFY_1
#undef OVERRIDE_TO_NOTIFY_2
#undef OVERRIDE_TO_NOTIFY_3

//==============================================================================
ARADocument* ARADocumentControllerSpecialisation::doCreateDocument()
{
    return new ARADocument (static_cast<ARADocumentControllerImpl*> (getDocumentController()));
}

ARAMusicalContext* ARADocumentControllerSpecialisation::doCreateMusicalContext (ARADocument* document,
                                                                                ARA::ARAMusicalContextHostRef hostRef)
{
    return new ARAMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

ARARegionSequence* ARADocumentControllerSpecialisation::doCreateRegionSequence (ARADocument* document,
                                                                                ARA::ARARegionSequenceHostRef hostRef)
{
    return new ARARegionSequence (static_cast<ARADocument*> (document), hostRef);
}

ARAAudioSource* ARADocumentControllerSpecialisation::doCreateAudioSource (ARADocument* document,
                                                                          ARA::ARAAudioSourceHostRef hostRef)
{
     return new ARAAudioSource (static_cast<ARADocument*> (document), hostRef);
}

ARAAudioModification* ARADocumentControllerSpecialisation::doCreateAudioModification (
    ARAAudioSource* audioSource,
    ARA::ARAAudioModificationHostRef hostRef,
    const ARAAudioModification* optionalModificationToClone)
{
    return new ARAAudioModification (static_cast<ARAAudioSource*> (audioSource),
                                     hostRef,
                                     static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

ARAPlaybackRegion*
    ARADocumentControllerSpecialisation::doCreatePlaybackRegion (ARAAudioModification* modification,
                                                                 ARA::ARAPlaybackRegionHostRef hostRef)
{
    return new ARAPlaybackRegion (static_cast<ARAAudioModification*> (modification), hostRef);
}

//==============================================================================
ARA::PlugIn::PlaybackRenderer* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRenderer() noexcept
{
    return specialisation->doCreatePlaybackRenderer();
}

ARA::PlugIn::EditorRenderer* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateEditorRenderer() noexcept
{
    return specialisation->doCreateEditorRenderer();
}

ARA::PlugIn::EditorView* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateEditorView() noexcept
{
    return specialisation->doCreateEditorView();
}

bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioSourceContentAvailable (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                      ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioSourceContentAvailable (audioSource, type);
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetAudioSourceContentGrade (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                                   ARA::ARAContentType type) noexcept
{
    return specialisation->doGetAudioSourceContentGrade (audioSource, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioSourceContentReader (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                              ARA::ARAContentType type,
                                                                                                                              const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreateAudioSourceContentReader (audioSource, type, range);
}

bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioModificationContentAvailable (const ARA::PlugIn::AudioModification* audioModification,
                                                                                                            ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioModificationContentAvailable (audioModification, type);
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetAudioModificationContentGrade (const ARA::PlugIn::AudioModification* audioModification,
                                                                                                                         ARA::ARAContentType type) noexcept
{
    return specialisation->doGetAudioModificationContentGrade (audioModification, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreateAudioModificationContentReader (ARA::PlugIn::AudioModification* audioModification,
                                                                                                                                    ARA::ARAContentType type,
                                                                                                                                    const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreateAudioModificationContentReader (audioModification, type, range);
}

bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsPlaybackRegionContentAvailable (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                         ARA::ARAContentType type) noexcept
{
    return specialisation->doIsPlaybackRegionContentAvailable (playbackRegion, type);
}

ARA::ARAContentGrade
    ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetPlaybackRegionContentGrade (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                     ARA::ARAContentType type) noexcept
{
    return specialisation->doGetPlaybackRegionContentGrade (playbackRegion, type);
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doCreatePlaybackRegionContentReader (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                                                 ARA::ARAContentType type,
                                                                                                                                 const ARA::ARAContentTimeRange* range) noexcept
{
    return specialisation->doCreatePlaybackRegionContentReader (playbackRegion, type, range);
}

bool ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doIsAudioSourceContentAnalysisIncomplete (const ARA::PlugIn::AudioSource* audioSource,
                                                                                                               ARA::ARAContentType type) noexcept
{
    return specialisation->doIsAudioSourceContentAnalysisIncomplete (audioSource, type);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRequestAudioSourceContentAnalysis (ARA::PlugIn::AudioSource* audioSource,
                                                                                                          std::vector<ARA::ARAContentType> const& contentTypes) noexcept
{
    specialisation->doRequestAudioSourceContentAnalysis (audioSource, contentTypes);
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmsCount() noexcept
{
    return specialisation->doGetProcessingAlgorithmsCount();
}

const ARA::ARAProcessingAlgorithmProperties* ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmProperties (ARA::ARAInt32 algorithmIndex) noexcept
{
    return specialisation->doGetProcessingAlgorithmProperties (algorithmIndex);
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doGetProcessingAlgorithmForAudioSource (const ARA::PlugIn::AudioSource* audioSource) noexcept
{
    return specialisation->doGetProcessingAlgorithmForAudioSource (audioSource);
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource,
                                                                                                                 ARA::ARAInt32 algorithmIndex) noexcept
{
    return specialisation->doRequestProcessingAlgorithmForAudioSource (audioSource, algorithmIndex);
}

//==============================================================================
// Helper code for ARADocumentControllerSpecialisation::ARADocumentControllerImpl::timerCallback() to
// rewire the host-related ARA SDK's progress tracker to our internal update mechanism.
namespace ModelUpdateControllerProgressAdapter
{
    using namespace ARA;

    static void ARA_CALL notifyAudioSourceAnalysisProgress (ARAModelUpdateControllerHostRef /*controllerHostRef*/,
                                                            ARAAudioSourceHostRef audioSourceHostRef, ARAAnalysisProgressState state, float value) noexcept
    {
        auto audioSource = reinterpret_cast<ARAAudioSource*> (audioSourceHostRef);
        audioSource->getDocumentController<ARADocumentController>()->internalDidUpdateAudioSourceAnalysisProgress (audioSource, state, value);
        audioSource->notifyListeners ([&] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceAnalysisProgress (audioSource, state, value); });
    }

    static void ARA_CALL notifyAudioSourceContentChanged (ARAModelUpdateControllerHostRef, ARAAudioSourceHostRef,
                                                          const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyAudioModificationContentChanged (ARAModelUpdateControllerHostRef, ARAAudioModificationHostRef,
                                                                const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyPlaybackRegionContentChanged (ARAModelUpdateControllerHostRef, ARAPlaybackRegionHostRef,
                                                             const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassertfalse;    // not to be called - this adapter only forwards analysis progress
    }

    static ARA::PlugIn::HostModelUpdateController* get()
    {
        static const auto modelUpdateControllerInterface = makeARASizedStruct (&ARA::ARAModelUpdateControllerInterface::notifyPlaybackRegionContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioSourceAnalysisProgress,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioSourceContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyAudioModificationContentChanged,
                                                                               ModelUpdateControllerProgressAdapter::notifyPlaybackRegionContentChanged);

        static const auto instance = makeARASizedStruct (&ARA::ARADocumentControllerHostInstance::playbackControllerInterface,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         nullptr,
                                                         &modelUpdateControllerInterface,
                                                         nullptr,
                                                         nullptr);

        static auto progressAdapter = ARA::PlugIn::HostModelUpdateController { &instance };
        return &progressAdapter;
    }
}

void ARADocumentControllerSpecialisation::ARADocumentControllerImpl::timerCallback()
{
    if (! internalAnalysisProgressIsSynced.exchange (true, std::memory_order_release))
        for (auto& audioSource : getDocument()->getAudioSources())
            audioSource->internalAnalysisProgressTracker.notifyProgress (ModelUpdateControllerProgressAdapter::get(),
                                                                         reinterpret_cast<ARA::ARAAudioSourceHostRef> (audioSource));
}

//==============================================================================
ARAInputStream::ARAInputStream (ARA::PlugIn::HostArchiveReader* reader)
    : archiveReader (reader),
      size ((int64) reader->getArchiveSize())
{}

int ARAInputStream::read (void* destBuffer, int maxBytesToRead)
{
    const auto bytesToRead = std::min ((int64) maxBytesToRead, size - position);

    if (bytesToRead > 0 && ! archiveReader->readBytesFromArchive ((ARA::ARASize) position, (ARA::ARASize) bytesToRead,
                                                                  static_cast<ARA::ARAByte*> (destBuffer)))
    {
        failure = true;
        return 0;
    }

    position += bytesToRead;
    return (int) bytesToRead;
}

bool ARAInputStream::setPosition (int64 newPosition)
{
    position = jlimit ((int64) 0, size, newPosition);
    return true;
}

bool ARAInputStream::isExhausted()
{
    return position >= size;
}

ARAOutputStream::ARAOutputStream (ARA::PlugIn::HostArchiveWriter* writer)
    : archiveWriter (writer)
{}

bool ARAOutputStream::write (const void* dataToWrite, size_t numberOfBytes)
{
    if (! archiveWriter->writeBytesToArchive ((ARA::ARASize) position, numberOfBytes, (const ARA::ARAByte*) dataToWrite))
        return false;

    position += (int64) numberOfBytes;
    return true;
}

bool ARAOutputStream::setPosition (int64 newPosition)
{
    position = newPosition;
    return true;
}

//==============================================================================
ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation (
    const ARA::PlugIn::PlugInEntry* entry,
    const ARA::ARADocumentControllerHostInstance* instance)
    : documentController (std::make_unique<ARADocumentControllerImpl> (entry, instance, this))
{
}

ARADocumentControllerSpecialisation::~ARADocumentControllerSpecialisation() = default;

ARAPlaybackRenderer* ARADocumentControllerSpecialisation::doCreatePlaybackRenderer()
{
    return new ARAPlaybackRenderer (getDocumentController());
}

ARAEditorRenderer* ARADocumentControllerSpecialisation::doCreateEditorRenderer()
{
    return new ARAEditorRenderer (getDocumentController());
}

ARAEditorView* ARADocumentControllerSpecialisation::doCreateEditorView()
{
    return new ARAEditorView (getDocumentController());
}

bool ARADocumentControllerSpecialisation::doIsAudioSourceContentAvailable (const ARA::PlugIn::AudioSource* audioSource,
                                                                           ARA::ARAContentType type)
{
    juce::ignoreUnused (audioSource, type);
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetAudioSourceContentGrade (const ARA::PlugIn::AudioSource* audioSource,
                                                                                        ARA::ARAContentType type)
{
    // Overriding doIsAudioSourceContentAvailable() requires overriding
    // doGetAudioSourceContentGrade() accordingly!
    jassertfalse;

    juce::ignoreUnused (audioSource, type);
    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreateAudioSourceContentReader (ARA::PlugIn::AudioSource* audioSource,
                                                                                                   ARA::ARAContentType type,
                                                                                                   const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsAudioSourceContentAvailable() requires overriding
    // doCreateAudioSourceContentReader() accordingly!
    jassertfalse;

    juce::ignoreUnused (audioSource, type, range);
    return nullptr;
}

bool ARADocumentControllerSpecialisation::doIsAudioModificationContentAvailable (const ARA::PlugIn::AudioModification* audioModification,
                                                                                 ARA::ARAContentType type)
{
    juce::ignoreUnused (audioModification, type);
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetAudioModificationContentGrade (const ARA::PlugIn::AudioModification* audioModification,
                                                                                              ARA::ARAContentType type)
{
    // Overriding doIsAudioModificationContentAvailable() requires overriding
    // doGetAudioModificationContentGrade() accordingly!
    jassertfalse;

    juce::ignoreUnused (audioModification, type);
    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreateAudioModificationContentReader (ARA::PlugIn::AudioModification* audioModification,
                                                                                                         ARA::ARAContentType type,
                                                                                                         const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsAudioModificationContentAvailable() requires overriding
    // doCreateAudioModificationContentReader() accordingly!
    jassertfalse;

    juce::ignoreUnused (audioModification, type, range);
    return nullptr;
}

bool ARADocumentControllerSpecialisation::doIsPlaybackRegionContentAvailable (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                              ARA::ARAContentType type)
{
    juce::ignoreUnused (playbackRegion, type);
    return false;
}

ARA::ARAContentGrade ARADocumentControllerSpecialisation::doGetPlaybackRegionContentGrade (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                           ARA::ARAContentType type)
{
    // Overriding doIsPlaybackRegionContentAvailable() requires overriding
    // doGetPlaybackRegionContentGrade() accordingly!
    jassertfalse;

    juce::ignoreUnused (playbackRegion, type);
    return ARA::kARAContentGradeInitial;
}

ARA::PlugIn::ContentReader* ARADocumentControllerSpecialisation::doCreatePlaybackRegionContentReader (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                                      ARA::ARAContentType type,
                                                                                                      const ARA::ARAContentTimeRange* range)
{
    // Overriding doIsPlaybackRegionContentAvailable() requires overriding
    // doCreatePlaybackRegionContentReader() accordingly!
    jassertfalse;

    juce::ignoreUnused (playbackRegion, type, range);
    return nullptr;
}

bool ARADocumentControllerSpecialisation::doIsAudioSourceContentAnalysisIncomplete (const ARA::PlugIn::AudioSource* audioSource,
                                                                                    ARA::ARAContentType type)
{
    juce::ignoreUnused (audioSource, type);
    return false;
}

void ARADocumentControllerSpecialisation::doRequestAudioSourceContentAnalysis (ARA::PlugIn::AudioSource* audioSource,
                                                                               std::vector<ARA::ARAContentType> const& contentTypes)
{
    juce::ignoreUnused (audioSource, contentTypes);
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::doGetProcessingAlgorithmsCount() { return 0; }

const ARA::ARAProcessingAlgorithmProperties*
    ARADocumentControllerSpecialisation::doGetProcessingAlgorithmProperties (ARA::ARAInt32 algorithmIndex)
{
    juce::ignoreUnused (algorithmIndex);
    return nullptr;
}

ARA::ARAInt32 ARADocumentControllerSpecialisation::doGetProcessingAlgorithmForAudioSource (const ARA::PlugIn::AudioSource* audioSource)
{
    // doGetProcessingAlgorithmForAudioSource() must be implemented if the supported
    // algorithm count is greater than zero.
    if (getDocumentController()->getProcessingAlgorithmsCount() > 0)
        jassertfalse;

    juce::ignoreUnused (audioSource);
    return 0;
}

void ARADocumentControllerSpecialisation::doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource,
                                                                                      ARA::ARAInt32 algorithmIndex)
{
    // doRequestProcessingAlgorithmForAudioSource() must be implemented if the supported
    // algorithm count is greater than zero.
    if (getDocumentController()->getProcessingAlgorithmsCount() > 0)
        jassertfalse;

    juce::ignoreUnused (audioSource, algorithmIndex);
}

} // namespace juce
