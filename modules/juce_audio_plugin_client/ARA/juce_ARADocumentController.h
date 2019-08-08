#pragma once

#include "JuceHeader.h"

namespace juce
{

class ARAAudioSourceReader;
class ARAPlaybackRegionReader;
class ARARegionSequenceReader;

class JUCE_API  ARADocumentController  : public ARA::PlugIn::DocumentController,
                                         private juce::Timer
{
public:
    explicit ARADocumentController (const ARA::ARADocumentControllerHostInstance* instance);

    //==============================================================================
    // Analysis progress notifications
    // Internal helper - instead of calling these functions directly, rather use
    // ARAAudioSource::notifyAnalysisProgress() which will forward here as appropriate.
    
    void notifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource);
    void notifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress);
    void notifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource);

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
    @param filter An optional filter to be applied to the stream; nullptr if no filtering is required

    The optional \p filter parameter can be used to restore a subsection of the document, in which case
    it will not be nullptr. 

    Overriding this method is the preferred way of handling ARA document persistence, but you can also
    override ARA::PlugIn::DocumentController::doRestoreObjectsFromArchive to deal with an ARA archive 
    ARA::PlugIn::HostArchiveReader directly. 
    */
    virtual bool doRestoreObjectsFromStream (InputStream& input, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept;

    /** Write an ARADocument archive to a juce::OutputStream.
    @param output Data stream that should be used to write the persistent ARADocument data
    @param filter An optional filter to be applied to the stream; nullptr if no filtering is required

    The optional \p filter parameter can be used to store a subsection of the document, in which case
    it will not be nullptr. 

    Overriding this method is the preferred way of handling ARA document persistence, but you can also
    override ARA::PlugIn::DocumentController::doStoreObjectsToArchive to deal with an ARA archive 
    ARA::PlugIn::HostArchiveWriter directly. 
    */
    virtual bool doStoreObjectsToStream (OutputStream& output, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept;

    // Model object creation
    // these can be overridden with custom types so long as
    // they inherit from our ARA model classes
    ARA::PlugIn::Document* doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept override;
    ARA::PlugIn::MusicalContext* doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    // PlugIn instance role creation
    // these can be overridden with custom types so long as
    // they inherit from our ARA instance role classes
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer () noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer () noexcept override;
    ARA::PlugIn::EditorView* doCreateEditorView () noexcept override;

#ifndef DOXYGEN

    // overridden to forward doRestoreObjectsFromStream()/doStoreObjectsToStream()
    bool doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    // overridden to invoke the ARA::PlugIn::DocumentController base implementations and forward content changes to listeners
    void updateMusicalContextContent (ARA::ARAMusicalContextRef musicalContextRef, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    void updateAudioSourceContent (ARA::ARAAudioSourceRef audioSourceRef, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;

    // Model Update Management
    void willBeginEditing () noexcept override;
    void didEndEditing () noexcept override;

    // Document notifications
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

    // MusicalContext notifications
    void willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept override;
    void didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    // RegionSequence notifications
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;

    // AudioSource notifications
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didAddAudioModificationToAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willRemoveAudioModificationFromAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    // AudioModification notifications
    void willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void didDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    // PlaybackRegion notifications
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    //==============================================================================
    // juce::Timer overrides
    void timerCallback() override;

#endif

protected:

    //==============================================================================
    class ArchiveReader     : public InputStream
    {
    public:
        ArchiveReader (ARA::PlugIn::HostArchiveReader*);

        int64 getPosition() override { return (int64) position; }
        int64 getTotalLength() override { return (int64) size; }

        bool setPosition (int64) override;
        bool isExhausted() override;
        int read (void*, int) override;

    private:
        ARA::PlugIn::HostArchiveReader* archiveReader;
        size_t position { 0 };
        size_t size;
    };

    //==============================================================================
    class ArchiveWriter     : public OutputStream
    {
    public:
        ArchiveWriter (ARA::PlugIn::HostArchiveWriter*);

        int64 getPosition() override { return (int64) position; }
        void flush() override {}

        bool setPosition (int64) override;
        bool write (const void*, size_t) override;

    private:
        ARA::PlugIn::HostArchiveWriter* archiveWriter;
        size_t position { 0 };
    };

private:
    // this flag is used automatically trigger content update if a property change implies this
    bool currentPropertyUpdateAffectsContent { false };

    std::atomic_flag internalAnalysisProgressIsSynced { true };

    ScopedJuceInitialiser_GUI libraryInitialiser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
