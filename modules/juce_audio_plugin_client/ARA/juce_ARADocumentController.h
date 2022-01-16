#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace juce
{

class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;
class ARAInputStream;
class ARAOutputStream;

//==============================================================================
/**
    Base class for implementing an ARA DocumentController in JUCE - refer to the 
    ARA SDK documentation for more details. 

    Your ARA plug-in must subclass ARADocumentController and define a global
    function called createARAFactory() which calls
    ARADocumentController::createARAFactory().

    Any `do` functions (i.e doRestoreObjectsFromStream(), doCreateAudioSource())
    can be overridden to implement custom document controller behavior. The 
    `will/did` methods found in ARA::PlugIn::DocumentController have been replaced
    by listener callbacks on our ARA model object / plug-in instance role classes. 

    @tags{ARA}
*/
class JUCE_API  ARADocumentController     : public ARA::PlugIn::DocumentController,
                                            private ARADocument::Listener,
                                            private ARAMusicalContext::Listener,
                                            private ARARegionSequence::Listener,
                                            private ARAAudioSource::Listener,
                                            private ARAAudioModification::Listener,
                                            private ARAPlaybackRegion::Listener,
                                            private juce::Timer
{
private:
    class FactoryConfig   : public ARA::PlugIn::FactoryConfig
    {
    public:
        FactoryConfig() noexcept;

        const char* getFactoryID() const noexcept override { return JucePlugin_ARAFactoryID; }
        const char* getPlugInName() const noexcept override { return JucePlugin_Name; }
        const char* getManufacturerName() const noexcept override { return JucePlugin_Manufacturer; }
        const char* getInformationURL() const noexcept override { return JucePlugin_ManufacturerWebsite; }
        const char* getVersion() const noexcept override { return JucePlugin_VersionString; }

        const char* getDocumentArchiveID() const noexcept override { return JucePlugin_ARADocumentArchiveID; }
        ARA::ARASize getCompatibleDocumentArchiveIDsCount() const noexcept override { return compatibleDocumentArchiveIDs.size(); }
        const ARA::ARAPersistentID* getCompatibleDocumentArchiveIDs() const noexcept override { return compatibleDocumentArchiveIDs.empty() ? nullptr : compatibleDocumentArchiveIDs.data(); }

        ARA::ARASize getAnalyzeableContentTypesCount() const noexcept override { return analyzeableContentTypes.size(); }
        const ARA::ARAContentType* getAnalyzeableContentTypes() const noexcept override { return analyzeableContentTypes.empty() ? nullptr : analyzeableContentTypes.data(); }

        ARA::ARAPlaybackTransformationFlags getSupportedPlaybackTransformationFlags() const noexcept override { return supportedPlaybackTransformationFlags; }

    private:
        juce::StringArray compatibleDocumentArchiveIDStrings;
        std::vector<ARA::ARAPersistentID> compatibleDocumentArchiveIDs;
        std::vector<ARA::ARAContentType> analyzeableContentTypes;
        ARA::ARAPlaybackTransformationFlags supportedPlaybackTransformationFlags;
    };

public:
    //==============================================================================
    /** Entry point for your customization:
        Similar to createPluginFilter() for your audio processor class, you should
        implement a global function called createARAFactory() which simply calls this
        function with your custom subclass of ARADocumentController as template argument.
     */
    template<typename CustomSubclass>
    static inline const ARA::ARAFactory* createARAFactory()
    {
        static_assert (std::is_base_of<ARADocumentController, CustomSubclass>::value, "Must provide subclass of ARADocumentController");
        return ARA::PlugIn::PlugInEntry::getPlugInEntry<FactoryConfig, CustomSubclass> ()->getFactory ();
    }

    ARADocumentController (const ARA::PlugIn::PlugInEntry* entry, const ARA::ARADocumentControllerHostInstance* instance) noexcept
        : ARA::PlugIn::DocumentController::DocumentController (entry, instance)
    {
        internalAnalysisProgressIsSynced.test_and_set (std::memory_order_release);
    }

    // overloading inherited templated getters to default to juce versions of the returned classes
    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept { return ARA::PlugIn::DocumentController::getDocument<Document_t>(); }
    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    std::vector<PlaybackRenderer_t*> const& getPlaybackRenderers () const noexcept { return ARA::PlugIn::DocumentController::getPlaybackRenderers<PlaybackRenderer_t>(); }
    template <typename EditorRenderer_t = ARAEditorRenderer>
    std::vector<EditorRenderer_t*> const& getEditorRenderers () const noexcept { return ARA::PlugIn::DocumentController::getEditorRenderers<EditorRenderer_t>(); }
    template <typename EditorView_t = ARAEditorView>
    std::vector<EditorView_t*> const& getEditorViews () const noexcept { return ARA::PlugIn::DocumentController::getEditorViews<EditorView_t>(); }

protected:
    //==============================================================================
    // Override document controller methods here
    // These functions are called by the host through the ARA API.
    // If you are subclassing ARADocumentController, make sure to call the base class
    // implementations for any will/did method overridden here you're overriding again.
    // For do methods or for methods inherited directly from ARA::PlugIn::DocumentController
    // you do not need to call the ingerited when overriding.

    /** Read an ARADocument archive from a juce::InputStream.
    @param input Data stream containing previously persisted data to be used when restoring the ARADocument
    @param filter A filter to be applied to the stream
    */
    virtual bool doRestoreObjectsFromStream (ARAInputStream& input, const ARARestoreObjectsFilter* filter) noexcept = 0;

    /** Write an ARADocument archive to a juce::OutputStream.
    @param output Data stream that should be used to write the persistent ARADocument data
    @param filter A filter to be applied to the stream
    */
    virtual bool doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept = 0;

    // Model object creation - override as needed to return custom subclasses of the base ARA model types.
    ARA::PlugIn::Document* doCreateDocument () noexcept override;
    ARA::PlugIn::MusicalContext* doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    // PlugIn instance role creation - override as needed to return custom subclasses of the base ARA plug-in instance roles.
    friend class ARAPlaybackRegionReader;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override = 0;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer() noexcept override;
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;

    // ARADocument::Listener callbacks
    using ARADocument::Listener::willBeginEditing;
    using ARADocument::Listener::didEndEditing;
    using ARADocument::Listener::willNotifyModelUpdates;
    using ARADocument::Listener::didNotifyModelUpdates;
    using ARADocument::Listener::willUpdateDocumentProperties;
    using ARADocument::Listener::didUpdateDocumentProperties;
    using ARADocument::Listener::didAddMusicalContextToDocument;
    using ARADocument::Listener::willRemoveMusicalContextFromDocument;
    using ARADocument::Listener::didReorderMusicalContextsInDocument;
    using ARADocument::Listener::didAddRegionSequenceToDocument;
    using ARADocument::Listener::willRemoveRegionSequenceFromDocument;
    using ARADocument::Listener::didReorderRegionSequencesInDocument;
    using ARADocument::Listener::didAddAudioSourceToDocument;
    using ARADocument::Listener::willRemoveAudioSourceFromDocument;
    using ARADocument::Listener::willDestroyDocument;

    // ARAMusicalContext::Listener callbacks
    using ARAMusicalContext::Listener::willUpdateMusicalContextProperties;
    using ARAMusicalContext::Listener::didUpdateMusicalContextProperties;
    using ARAMusicalContext::Listener::doUpdateMusicalContextContent;
    using ARAMusicalContext::Listener::didAddRegionSequenceToMusicalContext;
    using ARAMusicalContext::Listener::willRemoveRegionSequenceFromMusicalContext;
    using ARAMusicalContext::Listener::didReorderRegionSequencesInMusicalContext;
    using ARAMusicalContext::Listener::willDestroyMusicalContext;

    // ARARegionSequence::Listener callbacks
    using ARARegionSequence::Listener::willUpdateRegionSequenceProperties;
    using ARARegionSequence::Listener::didUpdateRegionSequenceProperties;
    using ARARegionSequence::Listener::didAddPlaybackRegionToRegionSequence;
    using ARARegionSequence::Listener::willRemovePlaybackRegionFromRegionSequence;
    using ARARegionSequence::Listener::willDestroyRegionSequence;

    // ARAAudioSource::Listener callbacks
    using ARAAudioSource::Listener::willUpdateAudioSourceProperties;
    using ARAAudioSource::Listener::didUpdateAudioSourceProperties;
    using ARAAudioSource::Listener::doUpdateAudioSourceContent;
    using ARAAudioSource::Listener::willEnableAudioSourceSamplesAccess;
    using ARAAudioSource::Listener::didEnableAudioSourceSamplesAccess;
    using ARAAudioSource::Listener::didAddAudioModificationToAudioSource;
    using ARAAudioSource::Listener::willRemoveAudioModificationFromAudioSource;
    using ARAAudioSource::Listener::willDeactivateAudioSourceForUndoHistory;
    using ARAAudioSource::Listener::didDeactivateAudioSourceForUndoHistory;
    using ARAAudioSource::Listener::willDestroyAudioSource;

    // ARAAudioModification::Listener callbacks
    using ARAAudioModification::Listener::willUpdateAudioModificationProperties;
    using ARAAudioModification::Listener::didUpdateAudioModificationProperties;
    using ARAAudioModification::Listener::didAddPlaybackRegionToAudioModification;
    using ARAAudioModification::Listener::willRemovePlaybackRegionFromAudioModification;
    using ARAAudioModification::Listener::willDeactivateAudioModificationForUndoHistory;
    using ARAAudioModification::Listener::didDeactivateAudioModificationForUndoHistory;
    using ARAAudioModification::Listener::willDestroyAudioModification;

    // ARAPlaybackRegion::Listener callbacks
    using ARAPlaybackRegion::Listener::willUpdatePlaybackRegionProperties;
    using ARAPlaybackRegion::Listener::didUpdatePlaybackRegionProperties;
    using ARAPlaybackRegion::Listener::willDestroyPlaybackRegion;

    // ARAAudioSource content access
    using ARA::PlugIn::DocumentController::doIsAudioSourceContentAvailable;
    using ARA::PlugIn::DocumentController::doGetAudioSourceContentGrade;
    using ARA::PlugIn::DocumentController::doCreateAudioSourceContentReader;

    // ARAAudioModification content access
    using ARA::PlugIn::DocumentController::doIsAudioModificationContentAvailable;
    using ARA::PlugIn::DocumentController::doGetAudioModificationContentGrade;
    using ARA::PlugIn::DocumentController::doCreateAudioModificationContentReader;

    // ARAPlaybackRegion content access
    using ARA::PlugIn::DocumentController::doIsPlaybackRegionContentAvailable;
    using ARA::PlugIn::DocumentController::doGetPlaybackRegionContentGrade;
    using ARA::PlugIn::DocumentController::doCreatePlaybackRegionContentReader;

    // ARAAudioSource analysis
    using ARA::PlugIn::DocumentController::doIsAudioSourceContentAnalysisIncomplete;
    using ARA::PlugIn::DocumentController::doRequestAudioSourceContentAnalysis;

    // Analysis Algorithm selection
    using ARA::PlugIn::DocumentController::doGetProcessingAlgorithmsCount;
    using ARA::PlugIn::DocumentController::doGetProcessingAlgorithmProperties;
    using ARA::PlugIn::DocumentController::doGetProcessingAlgorithmForAudioSource;
    using ARA::PlugIn::DocumentController::doRequestProcessingAlgorithmForAudioSource;

#ifndef DOXYGEN

    // overridden to forward doRestoreObjectsFromStream()/doStoreObjectsToStream(), typically not overridden further
    bool doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    // Document notifications, typically not overridden further - instead, override the ARADocument::Listener callbacks below
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

    // MusicalContext notifications, typically not overridden further - instead, override the ARAMusicalContext::Listener callbacks below
    void willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept override;
    void didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    void didAddRegionSequenceToMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willRemoveRegionSequenceFromMusicalContext (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didReorderRegionSequencesInMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    // RegionSequence notifications, typically not overridden further - instead, override the ARARegionSequence::Listener callbacks below
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;

    // AudioSource notifications, typically not overridden further - instead, override the ARAAudioSource::Listener callbacks below
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

    // AudioModification notifications, typically not overridden further - instead, override the ARAAudioModification::Listener callbacks below
    void willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void didDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    // PlaybackRegion notifications, typically not overridden further - instead, override the ARAPlaybackRegion::Listener callbacks below
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    //==============================================================================
    // juce::Timer overrides
    void timerCallback() override;

public:

    //==============================================================================
    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource);
    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress);
    /** @internal */
    void internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource);
    /** @internal */
    void internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource, ARAAudioSource::ARAAnalysisProgressState state, float progress);

    //==============================================================================
    /** @internal */
    void internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags, bool notifyARAHost);
    /** @internal */
    void internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags, bool notifyARAHost);
    /** @internal */
    void internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags, bool notifyARAHost);

#endif

private:
    std::atomic_flag internalAnalysisProgressIsSynced;

    ScopedJuceInitialiser_GUI libraryInitialiser;

    int activeAudioSourcesCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};


//==============================================================================
/**
    Used to read persisted ARA archives - see doRestoreObjectsFromStream() for details. 
    
    @tags{ARA}
*/
class ARAInputStream  : public InputStream
{
public:
    ARAInputStream (ARA::PlugIn::HostArchiveReader*);

    int64 getPosition() override { return position; }
    int64 getTotalLength() override { return size; }

    int read (void*, int) override;
    bool setPosition (int64) override;
    bool isExhausted() override;

    bool failed() const { return failure; }

private:
    ARA::PlugIn::HostArchiveReader* archiveReader;
    int64 position { 0 };
    int64 size;
    bool failure { false };
};


//==============================================================================
/**
    Used to write persistent ARA archives - see doStoreObjectsToStream() for details.

    @tags{ARA}
*/
class ARAOutputStream     : public OutputStream
{
public:
    ARAOutputStream (ARA::PlugIn::HostArchiveWriter*);

    int64 getPosition() override { return position; }
    void flush() override {}

    bool write (const void*, size_t) override;
    bool setPosition (int64) override;

private:
    ARA::PlugIn::HostArchiveWriter* archiveWriter;
    int64 position { 0 };
};
} // namespace juce
