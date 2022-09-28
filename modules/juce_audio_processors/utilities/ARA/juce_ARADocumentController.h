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

#pragma once

namespace juce
{

class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;
class ARAInputStream;
class ARAOutputStream;

/** This class contains the customisation points for the JUCE provided ARA document controller
    implementation.

    Every ARA enabled plugin must provide its own document controller implementation. To do this,
    inherit from this class, and override its protected methods as needed. Then you need to
    implement a global function somewhere in your module called createARAFactory(). This function
    must return an ARAFactory* that will instantiate document controller objects using your
    specialisation. There are helper functions inside ARADocumentControllerSpecialisation, so the
    implementation of createARAFactory() can always be a simple one-liner. For example

    @code
    class MyDocumentController : public ARADocumentControllerSpecialisation
    {
        //...
    };

    const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory()
    {
        return juce::ARADocumentControllerSpecialisation::createARAFactory<MyDocumentController>();
    }
    @endcode

    Most member functions have a default implementation so you can build up your required feature
    set gradually. The protected functions of this class fall in three distinct groups:
    - interactive editing and playback,
    - analysis features provided by the plugin and utilised by the host, and
    - maintaining the ARA model graph.

    On top of the pure virtual functions, you will probably want to override
    doCreatePlaybackRenderer() at the very least if you want your plugin to play any sound. This
    function belongs to the first group.

    If your plugin has analysis capabilities and wants to allow the host to access these, functions
    in the second group should be overridden.

    The default implementation of the ARA model object classes - i.e. ARADocument, ARAMusicalContext,
    ARARegionSequence, ARAAudioSource, ARAAudioModification, ARAPlaybackRegion - should be sufficient
    for maintaining a representation of the ARA model graph, hence overriding the model object
    creation functions e.g. doCreateMusicalContext() is considered an advanced use case. Hence you
    should be able to get a lot done without overriding functions in the third group.

    In order to react to the various ARA state changes you can override any of the ARA model object
    Listener functions that ARADocumentControllerSpecialisation inherits from. Such listener
    functions can be attached to one particular model objects instance, but the listener functions
    inside ARADocumentControllerSpecialisation will respond to the events of all instances of the
    model objects.

    @tags{ARA}
*/
class ARADocumentControllerSpecialisation  : public ARADocument::Listener,
                                             public ARAMusicalContext::Listener,
                                             public ARARegionSequence::Listener,
                                             public ARAAudioSource::Listener,
                                             public ARAAudioModification::Listener,
                                             public ARAPlaybackRegion::Listener
{
public:
    //==============================================================================
    /** Constructor. Used internally by the ARAFactory implementation.
    */
    ARADocumentControllerSpecialisation (const ARA::PlugIn::PlugInEntry* entry,
                                         const ARA::ARADocumentControllerHostInstance* instance);

    /** Destructor. */
    virtual ~ARADocumentControllerSpecialisation();

    /** Returns the underlying DocumentController object that references this specialisation.
    */
    ARA::PlugIn::DocumentController* getDocumentController() noexcept;

    /** Helper function for implementing the global createARAFactory() function.

        For example

        @code
        class MyDocumentController : public ARADocumentControllerSpecialisation
        {
            //...
        };

        const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory()
        {
            return juce::ARADocumentControllerSpecialisation::createARAFactory<MyDocumentController>();
        }
        @endcode
    */
    template <typename SpecialisationType>
    static const ARA::ARAFactory* createARAFactory()
    {
        static_assert (std::is_base_of_v<ARADocumentControllerSpecialisation, SpecialisationType>,
                       "DocumentController specialization types must inherit from ARADocumentControllerSpecialisation");
        return ARA::PlugIn::PlugInEntry::getPlugInEntry<FactoryConfig<SpecialisationType>>()->getFactory();
    }

    /** Returns a pointer to the ARADocumentControllerSpecialisation instance that is referenced
        by the provided DocumentController. You can use this function to access your specialisation
        from anywhere where you have access to ARA::PlugIn::DocumentController*.
    */
    template <typename Specialisation = ARADocumentControllerSpecialisation>
    static Specialisation* getSpecialisedDocumentController (ARA::PlugIn::DocumentController* dc)
    {
        return static_cast<Specialisation*> (getSpecialisedDocumentControllerImpl (dc));
    }

    /** Returns a pointer to the ARA document root maintained by this document controller. */
    template <typename DocumentType = ARADocument>
    DocumentType* getDocument()
    {
        return static_cast<DocumentType*> (getDocumentImpl());
    }

protected:
    //==============================================================================
    /** Read an ARADocument archive from a juce::InputStream.

        @param input Data stream containing previously persisted data to be used when restoring the ARADocument
        @param filter A filter to be applied to the stream

        Return true if the operation is successful.

        @see ARADocumentControllerInterface::restoreObjectsFromArchive
    */
    virtual bool doRestoreObjectsFromStream (ARAInputStream& input, const ARARestoreObjectsFilter* filter) = 0;

    /** Write an ARADocument archive to a juce::OutputStream.

        @param output Data stream that should be used to write the persistent ARADocument data
        @param filter A filter to be applied to the stream

        Returns true if the operation is successful.

        @see ARADocumentControllerInterface::storeObjectsToArchive
    */
    virtual bool doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) = 0;

    //==============================================================================
    /** Override to return a custom subclass instance of ARAPlaybackRenderer. */
    virtual ARAPlaybackRenderer* doCreatePlaybackRenderer();

    /** Override to return a custom subclass instance of ARAEditorRenderer. */
    virtual ARAEditorRenderer*   doCreateEditorRenderer();

    /** Override to return a custom subclass instance of ARAEditorView. */
    virtual ARAEditorView*       doCreateEditorView();

    //==============================================================================
    // ARAAudioSource content access

    /** Override to implement isAudioSourceContentAvailable() for all your supported content types -
        the default implementation always returns false, preventing any calls to doGetAudioSourceContentGrade()
        and doCreateAudioSourceContentReader().

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doIsAudioSourceContentAvailable.
    */
    virtual bool                        doIsAudioSourceContentAvailable            (const ARA::PlugIn::AudioSource* audioSource,
                                                                                    ARA::ARAContentType type);

    /** Override to implement getAudioSourceContentGrade() for all your supported content types.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetAudioSourceContentGrade.
    */
    virtual ARA::ARAContentGrade        doGetAudioSourceContentGrade               (const ARA::PlugIn::AudioSource* audioSource,
                                                                                    ARA::ARAContentType type);

    /** Override to implement createAudioSourceContentReader() for all your supported content types,
        returning a custom subclass instance of ContentReader providing data of the requested type.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doCreateAudioSourceContentReader.
    */
    virtual ARA::PlugIn::ContentReader* doCreateAudioSourceContentReader           (ARA::PlugIn::AudioSource* audioSource,
                                                                                    ARA::ARAContentType type,
                                                                                    const ARA::ARAContentTimeRange* range);

    //==============================================================================
    // ARAAudioModification content access

    /** Override to implement isAudioModificationContentAvailable() for all your supported content types -
        the default implementation always returns false.
        For read-only data directly inherited from the underlying audio source you can just delegate the
        call to the audio source, but user-editable modification data must be specifically handled here.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doIsAudioModificationContentAvailable.
    */
    virtual bool                        doIsAudioModificationContentAvailable      (const ARA::PlugIn::AudioModification* audioModification,
                                                                                    ARA::ARAContentType type);

    /** Override to implement getAudioModificationContentGrade() for all your supported content types.
        For read-only data directly inherited from the underlying audio source you can just delegate the
        call to the audio source, but user-editable modification data must be specifically handled here.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetAudioModificationContentGrade.
    */
    virtual ARA::ARAContentGrade        doGetAudioModificationContentGrade         (const ARA::PlugIn::AudioModification* audioModification,
                                                                                    ARA::ARAContentType type);

    /** Override to implement createAudioModificationContentReader() for all your supported content types,
        returning a custom subclass instance of ContentReader providing data of the requested \p type.
        For read-only data directly inherited from the underlying audio source you can just delegate the
        call to the audio source, but user-editable modification data must be specifically handled here.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doCreateAudioModificationContentReader.
    */
    virtual ARA::PlugIn::ContentReader* doCreateAudioModificationContentReader     (ARA::PlugIn::AudioModification* audioModification,
                                                                                    ARA::ARAContentType type,
                                                                                    const ARA::ARAContentTimeRange* range);

    //==============================================================================
    // ARAPlaybackRegion content access

    /** Override to implement isPlaybackRegionContentAvailable() for all your supported content types -
        the default implementation always returns false.
        Typically, this call can directly delegate to the underlying audio modification, since most
        plug-ins will apply their modification data to the playback region with a transformation that
        does not affect content availability.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doIsPlaybackRegionContentAvailable.
    */
    virtual bool                        doIsPlaybackRegionContentAvailable         (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                    ARA::ARAContentType type);

    /** Override to implement getPlaybackRegionContentGrade() for all your supported content types.
        Typically, this call can directly delegate to the underlying audio modification, since most
        plug-ins will apply their modification data to the playback region with a transformation that
        does not affect content grade.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetPlaybackRegionContentGrade.
    */
    virtual ARA::ARAContentGrade        doGetPlaybackRegionContentGrade            (const ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                    ARA::ARAContentType type);

    /** Override to implement createPlaybackRegionContentReader() for all your supported content types,
        returning a custom subclass instance of ContentReader providing data of the requested type.

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doCreatePlaybackRegionContentReader.
    */
    virtual ARA::PlugIn::ContentReader* doCreatePlaybackRegionContentReader        (ARA::PlugIn::PlaybackRegion* playbackRegion,
                                                                                    ARA::ARAContentType type,
                                                                                    const ARA::ARAContentTimeRange* range);

    //==============================================================================
    // ARAAudioSource analysis

    /** Override to implement isAudioSourceContentAnalysisIncomplete().

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doIsAudioSourceContentAnalysisIncomplete.
    */
    virtual bool                        doIsAudioSourceContentAnalysisIncomplete   (const ARA::PlugIn::AudioSource* audioSource,
                                                                                    ARA::ARAContentType type);

    /** Override to implement requestAudioSourceContentAnalysis().

        This function's called from
        ARA::PlugIn::DocumentControllerDelegate::doRequestAudioSourceContentAnalysis.
    */
    virtual void                        doRequestAudioSourceContentAnalysis        (ARA::PlugIn::AudioSource*  audioSource,
                                                                                    std::vector<ARA::ARAContentType> const& contentTypes);

    //==============================================================================
    // Analysis Algorithm selection

    /** Override to implement getProcessingAlgorithmsCount().

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetProcessingAlgorithmsCount.
    */
    virtual ARA::ARAInt32               doGetProcessingAlgorithmsCount             ();

    /** Override to implement getProcessingAlgorithmProperties().

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetProcessingAlgorithmProperties.
    */
    virtual const ARA::ARAProcessingAlgorithmProperties*
                                        doGetProcessingAlgorithmProperties         (ARA::ARAInt32 algorithmIndex);

    /** Override to implement getProcessingAlgorithmForAudioSource().

        This function's result is returned from
        ARA::PlugIn::DocumentControllerDelegate::doGetProcessingAlgorithmForAudioSource.
    */
    virtual ARA::ARAInt32               doGetProcessingAlgorithmForAudioSource     (const ARA::PlugIn::AudioSource* audioSource);

    /** Override to implement requestProcessingAlgorithmForAudioSource().

        This function's called from
        ARA::PlugIn::DocumentControllerDelegate::doRequestProcessingAlgorithmForAudioSource.
    */
    virtual void                        doRequestProcessingAlgorithmForAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::ARAInt32 algorithmIndex);

    //==============================================================================
    /** Override to return a custom subclass instance of ARADocument. */
    virtual ARADocument*          doCreateDocument();

    /** Override to return a custom subclass instance of ARAMusicalContext. */
    virtual ARAMusicalContext*    doCreateMusicalContext       (ARADocument* document,
                                                                ARA::ARAMusicalContextHostRef hostRef);

    /** Override to return a custom subclass instance of ARARegionSequence. */
    virtual ARARegionSequence*    doCreateRegionSequence       (ARADocument* document,
                                                                ARA::ARARegionSequenceHostRef hostRef);

    /** Override to return a custom subclass instance of ARAAudioSource. */
    virtual ARAAudioSource*       doCreateAudioSource          (ARADocument* document,
                                                                ARA::ARAAudioSourceHostRef hostRef);

    /** Override to return a custom subclass instance of ARAAudioModification. */
    virtual ARAAudioModification* doCreateAudioModification    (ARAAudioSource* audioSource,
                                                                ARA::ARAAudioModificationHostRef hostRef,
                                                                const ARAAudioModification* optionalModificationToClone);

    /** Override to return a custom subclass instance of ARAPlaybackRegion. */
    virtual ARAPlaybackRegion*    doCreatePlaybackRegion       (ARAAudioModification* modification,
                                                                ARA::ARAPlaybackRegionHostRef hostRef);

private:
    //==============================================================================
    template <typename SpecialisationType>
    class FactoryConfig  : public ARA::PlugIn::FactoryConfig
    {
    public:
        FactoryConfig() noexcept
        {
            const juce::String compatibleDocumentArchiveIDString = JucePlugin_ARACompatibleArchiveIDs;

            if (compatibleDocumentArchiveIDString.isNotEmpty())
            {
                compatibleDocumentArchiveIDStrings = juce::StringArray::fromLines (compatibleDocumentArchiveIDString);
                for (const auto& compatibleID : compatibleDocumentArchiveIDStrings)
                    compatibleDocumentArchiveIDs.push_back (compatibleID.toRawUTF8());
            }

            // Update analyzeable content types
            static constexpr std::array<ARA::ARAContentType, 6> contentTypes {
                ARA::kARAContentTypeNotes,
                ARA::kARAContentTypeTempoEntries,
                ARA::kARAContentTypeBarSignatures,
                ARA::kARAContentTypeStaticTuning,
                ARA::kARAContentTypeKeySignatures,
                ARA::kARAContentTypeSheetChords
            };

            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6313)

            for (size_t i = 0; i < contentTypes.size(); ++i)
                if (JucePlugin_ARAContentTypes & (1 << i))
                    analyzeableContentTypes.push_back (contentTypes[i]);

            JUCE_END_IGNORE_WARNINGS_MSVC

            // Update playback transformation flags
            static constexpr std::array<ARA::ARAPlaybackTransformationFlags, 4> playbackTransformationFlags {
                ARA::kARAPlaybackTransformationTimestretch,
                ARA::kARAPlaybackTransformationTimestretchReflectingTempo,
                ARA::kARAPlaybackTransformationContentBasedFadeAtTail,
                ARA::kARAPlaybackTransformationContentBasedFadeAtHead
            };

            supportedPlaybackTransformationFlags = 0;

            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6313)

            for (size_t i = 0; i < playbackTransformationFlags.size(); ++i)
                if (JucePlugin_ARATransformationFlags & (1 << i))
                    supportedPlaybackTransformationFlags |= playbackTransformationFlags[i];

            JUCE_END_IGNORE_WARNINGS_MSVC
        }

        const char* getFactoryID() const noexcept override         { return JucePlugin_ARAFactoryID; }
        const char* getPlugInName() const noexcept override        { return JucePlugin_Name; }
        const char* getManufacturerName() const noexcept override  { return JucePlugin_Manufacturer; }
        const char* getInformationURL() const noexcept override    { return JucePlugin_ManufacturerWebsite; }
        const char* getVersion() const noexcept override           { return JucePlugin_VersionString; }
        const char* getDocumentArchiveID() const noexcept override { return JucePlugin_ARADocumentArchiveID; }

        ARA::ARASize getCompatibleDocumentArchiveIDsCount() const noexcept override
        {
            return compatibleDocumentArchiveIDs.size();
        }

        const ARA::ARAPersistentID* getCompatibleDocumentArchiveIDs() const noexcept override
        {
            return compatibleDocumentArchiveIDs.empty() ? nullptr : compatibleDocumentArchiveIDs.data();
        }

        ARA::ARASize getAnalyzeableContentTypesCount() const noexcept override
        {
            return analyzeableContentTypes.size();
        }

        const ARA::ARAContentType* getAnalyzeableContentTypes() const noexcept override
        {
            return analyzeableContentTypes.empty() ? nullptr : analyzeableContentTypes.data();
        }

        ARA::ARAPlaybackTransformationFlags getSupportedPlaybackTransformationFlags() const noexcept override
        {
            return supportedPlaybackTransformationFlags;
        }

        ARA::PlugIn::DocumentController* createDocumentController (const ARA::PlugIn::PlugInEntry* entry,
                                                                   const ARA::ARADocumentControllerHostInstance* instance) const noexcept override
        {
            auto* spec = new SpecialisationType (entry, instance);
            return spec->getDocumentController();
        }

        void destroyDocumentController (ARA::PlugIn::DocumentController* controller) const noexcept override
        {
            delete getSpecialisedDocumentController (controller);
        }

    private:
        juce::StringArray compatibleDocumentArchiveIDStrings;
        std::vector<ARA::ARAPersistentID> compatibleDocumentArchiveIDs;
        std::vector<ARA::ARAContentType> analyzeableContentTypes;
        ARA::ARAPlaybackTransformationFlags supportedPlaybackTransformationFlags;
    };

    //==============================================================================
    static ARADocumentControllerSpecialisation* getSpecialisedDocumentControllerImpl (ARA::PlugIn::DocumentController*);

    ARADocument* getDocumentImpl();

    //==============================================================================
    class ARADocumentControllerImpl;
    std::unique_ptr<ARADocumentControllerImpl> documentController;
};

/** Used to read persisted ARA archives - see doRestoreObjectsFromStream() for details.

    @tags{ARA}
*/
class ARAInputStream  : public InputStream
{
public:
    explicit ARAInputStream (ARA::PlugIn::HostArchiveReader*);

    int64 getPosition() override { return position; }
    int64 getTotalLength() override { return size; }

    int read (void*, int) override;
    bool setPosition (int64) override;
    bool isExhausted() override;

    bool failed() const { return failure; }

private:
    ARA::PlugIn::HostArchiveReader* archiveReader;
    int64 position = 0;
    int64 size;
    bool failure = false;
};

/** Used to write persistent ARA archives - see doStoreObjectsToStream() for details.

    @tags{ARA}
*/
class ARAOutputStream  : public OutputStream
{
public:
    explicit ARAOutputStream (ARA::PlugIn::HostArchiveWriter*);

    int64 getPosition() override { return position; }
    void flush() override {}

    bool write (const void*, size_t) override;
    bool setPosition (int64) override;

private:
    ARA::PlugIn::HostArchiveWriter* archiveWriter;
    int64 position = 0;
};
} // namespace juce
