#pragma once

#include "JuceHeader.h"

namespace juce
{

//==============================================================================
/**
    Base class for implementing an ARA DocumentController in JUCE - refer to the 
    ARA SDK documentation for more details. 

    Your ARA plug-in must subclass ARADocumentController and declare a valid
    ARA::PlugIn::DocumentController::doCreateDocumentController implementation
    that returns an instance of your ARADocumentController subclass.

    Any `do` functions (i.e doRestoreObjectsFromStream(), doCreateAudioSource())
    can be overridden to implement custom document controller behavior. The 
    `will/did` methods found in ARA::PlugIn::DocumentController have been replaced
    by listener callbacks on our ARA model object / plug-in instance role classes. 

    @tags{ARA}
*/
class JUCE_API  ARADocumentController  : public ARA::PlugIn::DocumentController,
                                         private juce::Timer
{
public:
    explicit ARADocumentController (const ARA::ARADocumentControllerHostInstance* instance);

protected:
    //==============================================================================
    // Override document controller methods here
    // These functions are called by the host through the ARA API.
    // If you are subclassing ARADocumentController, make sure to call the base class
    // implementations for any will/did method overridden here you're overriding again.
    // For do methods or for methods inherited directly from ARA::PlugIn::DocumentController
    // you do not need to call the ingerited when overriding.
    class ARAInputStream;
    class ARAOutputStream;

    /** Read an ARADocument archive from a juce::InputStream.
    @param input Data stream containing previously persisted data to be used when restoring the ARADocument
    @param filter A filter to be applied to the stream
    */
    virtual bool doRestoreObjectsFromStream (ARAInputStream& input, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept;

    /** Write an ARADocument archive to a juce::OutputStream.
    @param output Data stream that should be used to write the persistent ARADocument data
    @param filter A filter to be applied to the stream
    */
    virtual bool doStoreObjectsToStream (ARAOutputStream& output, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept;

#ifndef DOXYGEN

    // Model object creation
    // these are typically overridden with custom types which are inherit from our ARA model classes
    ARA::PlugIn::Document* doCreateDocument () noexcept override;
    ARA::PlugIn::MusicalContext* doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;

    // PlugIn instance role creation
    // these are typically overridden with custom types which are inherit from our ARA instance role classes
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer() noexcept override;
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;

    // overridden to forward doRestoreObjectsFromStream()/doStoreObjectsToStream(), typically not overridden further
    bool doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    // overridden to invoke the ARA::PlugIn::DocumentController base implementations and forward content changes to listeners, typically not overridden further
    void updateMusicalContextContent (ARA::ARAMusicalContextRef musicalContextRef, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;
    void updateAudioSourceContent (ARA::ARAAudioSourceRef audioSourceRef, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes flags) noexcept override;

    // Document notifications, typically not overridden further
    void willBeginEditing() noexcept override;
    void didEndEditing() noexcept override;
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

    // MusicalContext notifications, typically not overridden further
    void willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept override;
    void didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    // RegionSequence notifications, typically not overridden further
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;

    // AudioSource notifications, typically not overridden further
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didAddAudioModificationToAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willRemoveAudioModificationFromAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void willDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    // AudioModification notifications, typically not overridden further
    void willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void didDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    // PlaybackRegion notifications, typically not overridden further
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

#endif

protected:

    //==============================================================================
    /**
        Used to read persisted ARA archives - see doRestoreObjectsFromStream() for details. 
    
        @tags{ARA}
    */
    class ARAInputStream     : public InputStream
    {
    public:
        ARAInputStream (ARA::PlugIn::HostArchiveReader*);

        int64 getPosition() override { return (int64) position; }
        int64 getTotalLength() override { return (int64) size; }

        int read (void*, int) override;
        bool setPosition (int64) override;
        bool isExhausted() override;

        bool failed() const { return failure; }

    private:
        ARA::PlugIn::HostArchiveReader* archiveReader;
        size_t position { 0 };
        size_t size;
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

        int64 getPosition() override { return (int64) position; }
        void flush() override {}

        bool write (const void*, size_t) override;
        bool setPosition (int64) override;

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
