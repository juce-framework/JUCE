#pragma once

#include "JuceHeader.h"
#include "juce_ARAModelObjects.h"

namespace juce
{

class ARAAudioSourceReader;
class ARAPlaybackRegionReader;
class ARARegionSequenceReader;

class ARADocumentController  : public ARA::PlugIn::DocumentController
{
public:
    using ARA::PlugIn::DocumentController::DocumentController;

    //==============================================================================
    // notify the host about content changes
    // Note that while the ARA API allows for specifying update time ranges, this feature is not yet
    // supported in our current plug-in implementation, since most hosts do not evaluate it anyways.
    // Typically, instead of calling these functions directly, rather use the respective model object's
    // notifyContentChanged() which will forward here as appropriate.
    void notifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags);
    void notifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags);
    void notifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags);

    //==============================================================================
    // Override document controller methods here
    // If you are subclassing ARADocumentController, make sure to call the base class
    // implementations of any overridden function, except for any doCreate...()
    // or where explicitly specified otherwise. Be careful whether you place the call to the
    // base class implementation before or after your additions, this depends on context!

private:
    // some helper macros to ease repeated declaration & implementation of notification functions below:

    // no notification arguments
   #define OVERRIDE_TO_NOTIFY_1(function, ModelObjectPtrType, modelObject) \
    void function (ARA::PlugIn::ModelObjectPtrType modelObject) noexcept override \
    { \
        auto object = static_cast<ARA##ModelObjectPtrType> (modelObject); \
        object->notifyListeners ([&] (std::remove_pointer<ARA##ModelObjectPtrType>::type::Listener& l) { l.function (object); }); \
    }

    // single notification argument, model object version
   #define OVERRIDE_TO_NOTIFY_2(function, ModelObjectPtrType, modelObject, ArgumentType, argument) \
    void function (ARA::PlugIn::ModelObjectPtrType modelObject, ARA::PlugIn::ArgumentType argument) noexcept override \
    { \
        auto object = static_cast<ARA##ModelObjectPtrType> (modelObject); \
        object->notifyListeners ([&] (std::remove_pointer<ARA##ModelObjectPtrType>::type::Listener& l) { l.function (object, static_cast<ARA##ArgumentType> (argument)); }); \
    }

    // single notification argument, non-model object version
   #define OVERRIDE_TO_NOTIFY_3(function, ModelObjectPtrType, modelObject, ArgumentType, argument) \
    void function (ARA::PlugIn::ModelObjectPtrType modelObject, ArgumentType argument) noexcept override \
    { \
        auto object = static_cast<ARA##ModelObjectPtrType> (modelObject); \
        object->notifyListeners ([&] (std::remove_pointer<ARA##ModelObjectPtrType>::type::Listener& l) { l.function (object, argument); }); \
    }

protected:
    // Model Update Management
    void willBeginEditing() noexcept override;
    void didEndEditing() noexcept override;
    void doNotifyModelContentUpdates() noexcept override final;

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

    bool doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept override;

    // Document callbacks
    ARA::PlugIn::Document* doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willUpdateDocumentProperties, Document*, document, ARADocument::PropertiesPtr, newProperties)
    OVERRIDE_TO_NOTIFY_1 (didUpdateDocumentProperties, Document*, document)
    OVERRIDE_TO_NOTIFY_2 (didAddMusicalContextToDocument, Document*, document, MusicalContext*, musicalContext)
    OVERRIDE_TO_NOTIFY_2 (willRemoveMusicalContextFromDocument, Document*, document, MusicalContext*, musicalContext)
    OVERRIDE_TO_NOTIFY_1 (didReorderMusicalContextsInDocument, Document*, document)
    OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToDocument, Document*, document, RegionSequence*, regionSequence)
    OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromDocument, Document*, document, RegionSequence*, regionSequence)
    OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInDocument, Document*, document)
    OVERRIDE_TO_NOTIFY_2 (didAddAudioSourceToDocument, Document*, document, AudioSource*, audioSource)
    OVERRIDE_TO_NOTIFY_2 (willRemoveAudioSourceFromDocument, Document*, document, AudioSource*, audioSource)
    OVERRIDE_TO_NOTIFY_1 (willDestroyDocument, Document*, document)

    // MusicalContext callbacks
    ARA::PlugIn::MusicalContext* doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willUpdateMusicalContextProperties, MusicalContext*, musicalContext, ARAMusicalContext::PropertiesPtr, newProperties)
    OVERRIDE_TO_NOTIFY_1 (didUpdateMusicalContextProperties, MusicalContext*, musicalContext)
    /*OVERRIDE_TO_NOTIFY_3*/ void doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes scopeFlags) noexcept override;
    OVERRIDE_TO_NOTIFY_1 (willDestroyMusicalContext, MusicalContext*, musicalContext)

    // RegionSequence callbacks
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willUpdateRegionSequenceProperties, RegionSequence*, regionSequence, ARARegionSequence::PropertiesPtr, newProperties)
    OVERRIDE_TO_NOTIFY_1 (didUpdateRegionSequenceProperties, RegionSequence*, regionSequence)
    OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToRegionSequence, RegionSequence*, regionSequence, PlaybackRegion*, playbackRegion)
    OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromRegionSequence, RegionSequence*, regionSequence, PlaybackRegion*, playbackRegion)
    OVERRIDE_TO_NOTIFY_1 (willDestroyRegionSequence, RegionSequence*, regionSequence)

    // AudioSource callbacks
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willUpdateAudioSourceProperties, AudioSource*, audioSource, ARAAudioSource::PropertiesPtr, newProperties)
    OVERRIDE_TO_NOTIFY_1 (didUpdateAudioSourceProperties, AudioSource*, audioSource)
    /*OVERRIDE_TO_NOTIFY_3*/ void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ContentUpdateScopes scopeFlags) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
    OVERRIDE_TO_NOTIFY_3 (didEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
    OVERRIDE_TO_NOTIFY_2 (didAddAudioModificationToAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
    OVERRIDE_TO_NOTIFY_2 (willRemoveAudioModificationFromAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
    OVERRIDE_TO_NOTIFY_3 (doDeactivateAudioSourceForUndoHistory, AudioSource*, audioSource, bool, deactivate)
    OVERRIDE_TO_NOTIFY_1 (willDestroyAudioSource, AudioSource*, audioSource)

    // AudioModification callbacks
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;
    OVERRIDE_TO_NOTIFY_3 (willUpdateAudioModificationProperties, AudioModification*, audioModification, ARAAudioModification::PropertiesPtr, newProperties)
    OVERRIDE_TO_NOTIFY_1 (didUpdateAudioModificationProperties, AudioModification*, audioModification)
    OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToAudioModification, AudioModification*, audioModification, PlaybackRegion*, playbackRegion)
    OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromAudioModification, AudioModification*, audioModification, PlaybackRegion*, playbackRegion)
    OVERRIDE_TO_NOTIFY_3 (doDeactivateAudioModificationForUndoHistory, AudioModification*, audioModification, bool, deactivate)
    OVERRIDE_TO_NOTIFY_1 (willDestroyAudioModification, AudioModification*, audioModification)

    // PlaybackRegion callbacks
    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;
    /*OVERRIDE_TO_NOTIFY_2*/ void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    /*OVERRIDE_TO_NOTIFY_1*/ void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void doGetPlaybackRegionHeadAndTailTime (const ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::ARATimeDuration* headTime, ARA::ARATimeDuration* tailTime) noexcept override;
    OVERRIDE_TO_NOTIFY_1 (willDestroyPlaybackRegion, PlaybackRegion*, playbackRegion)

    // PlugIn instance role creation
    // these can be overridden with custom types so long as
    // they inherit from our ARA instance role classes
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer() noexcept override;
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;

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
        size_t position, size;
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
        size_t position;
    };

private:
   #undef OVERRIDE_TO_NOTIFY_1
   #undef OVERRIDE_TO_NOTIFY_2
   #undef OVERRIDE_TO_NOTIFY_3

private:
    // this flag is used automatically trigger content update if a property change implies this
    bool _currentPropertyUpdateAffectsContent = false;

    std::map<ARAAudioSource*, ARAContentUpdateScopes> audioSourceUpdates;
    std::map<ARAAudioModification*, ARAContentUpdateScopes> audioModificationUpdates;
    std::map<ARAPlaybackRegion*, ARAContentUpdateScopes> playbackRegionUpdates;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
