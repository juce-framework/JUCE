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

class ARADocumentController;
class ARADocument;
class ARAMusicalContext;
class ARARegionSequence;
class ARAAudioSource;
class ARAAudioModification;
class ARAPlaybackRegion;

/** Base class used by the JUCE ARA model objects to provide listenable interfaces.

    @tags{ARA}
*/
template <class ListenerType>
class JUCE_API  ARAListenableModelClass
{
public:
    /** Constructor. */
    ARAListenableModelClass() = default;

    /** Destructor. */
    virtual ~ARAListenableModelClass() = default;

    /** Subscribe \p l to notified by changes to the object.
        @param l The listener instance.
    */
    void addListener (ListenerType* l) { listeners.add (l); }

    /** Unsubscribe \p l from object notifications.
        @param l The listener instance.
    */
    void removeListener (ListenerType* l) { listeners.remove (l); }

    /** Call the provided callback for each of the added listeners. */
    template <typename Callback>
    void notifyListeners (Callback&& callback)
    {
        listeners.call (callback);
    }

private:
    ListenerList<ListenerType> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAListenableModelClass)
};

/** Create a derived implementation of this class and pass it to ARAObject::visit() to retrieve the
    concrete type of a model object.

    Combined with ARAObject::traverse() on the ARADocument object it is possible to discover the
    entire model graph.

    Note that the references passed to the visit member functions are only guaranteed to live for
    the duration of the function call, so don't store pointers to these objects!

    @tags{Audio}
*/
class ARAObjectVisitor
{
public:
    /** Destructor. */
    virtual ~ARAObjectVisitor() = default;

    /** Called when visiting an ARADocument object. */
    virtual void visitDocument (juce::ARADocument&) {}

    /** Called when visiting an ARAMusicalContext object. */
    virtual void visitMusicalContext (juce::ARAMusicalContext&) {}

    /** Called when visiting an ARARegionSequence object. */
    virtual void visitRegionSequence (juce::ARARegionSequence&) {}

    /** Called when visiting an ARAPlaybackRegion object. */
    virtual void visitPlaybackRegion (juce::ARAPlaybackRegion&) {}

    /** Called when visiting an ARAAudioModification object. */
    virtual void visitAudioModification (juce::ARAAudioModification&) {}

    /** Called when visiting an ARAAudioSource object. */
    virtual void visitAudioSource (juce::ARAAudioSource&) {}
};

/** Common base class for all JUCE ARA model objects to aid with the discovery and traversal of the
    entire ARA model graph.

    @tags{ARA}
*/
class ARAObject
{
public:
    /** Destructor. */
    virtual ~ARAObject() = default;

    /** Returns the number of ARA model objects aggregated by this object. Objects that are merely
        referred to, but not aggregated by the current object are not included in this count, e.g.
        a referenced RegionSequence does not count as a child of the referring PlaybackRegion.

        See the ARA documentation's  ARA Model Graph Overview for more details.
    */
    virtual size_t getNumChildren() const noexcept = 0;

    /** Returns the child object associated with the given index.

        The index should be smaller than the value returned by getNumChildren().

        Note that the index of a particular object may change when the ARA model graph is edited.
    */
    virtual ARAObject* getChild (size_t index) = 0;

    /** Returns the ARA model object that aggregates this object.

        Returns nullptr for the ARADocument root object.
     */
    virtual ARAObject* getParent() = 0;

    /** Implements a depth first traversal of the ARA model graph starting from the current object,
        and visiting its children recursively.

        The provided function should accept a single ARAObject& parameter.
    */
    template <typename Fn>
    void traverse (Fn&& fn)
    {
        fn (*this);

        for (size_t i = 0; i < getNumChildren(); ++i)
        {
            getChild (i)->traverse (fn);
        }
    }

    /** Allows the retrieval of the concrete type of a model object.

        To use this, create a new class derived from ARAObjectVisitor and override its functions
        depending on which concrete types you are interested in.

        Calling this function inside the function passed to ARAObject::traverse() allows you to
        map the entire ARA model graph.
    */
    virtual void visit (ARAObjectVisitor& visitor) = 0;
};

/** A base class for listeners that want to know about changes to an ARADocument object.

    Use ARADocument::addListener() to register your listener with an ARADocument.

    @tags{ARA}
*/
class JUCE_API  ARADocumentListener
{
public:
    /** Destructor */
    virtual ~ARADocumentListener() = default;

    /** Called before the document enters an editing state.
        @param document The document about to enter an editing state.
    */
    virtual void willBeginEditing (ARADocument* document);

    /** Called after the document exits an editing state.
        @param document The document about exit an editing state.
    */
    virtual void didEndEditing (ARADocument* document);

    /** Called before sending model updates do the host.
        @param document The document whose model updates are about to be sent.
    */
    virtual void willNotifyModelUpdates (ARADocument* document);

    /** Called after sending model updates do the host.
        @param document The document whose model updates have just been sent.
    */
    virtual void didNotifyModelUpdates (ARADocument* document);

    /** Called before the document's properties are updated.
        @param document The document whose properties will be updated.
        @param newProperties The document properties that will be assigned to \p document.
    */
    virtual void willUpdateDocumentProperties (ARADocument* document,
                                               ARA::PlugIn::PropertiesPtr<ARA::ARADocumentProperties> newProperties);

    /** Called after the document's properties are updated.
        @param document The document whose properties were updated.
    */
    virtual void didUpdateDocumentProperties (ARADocument* document);

    /** Called after a musical context is added to the document.
        @param document The document that \p musicalContext was added to.
        @param musicalContext The musical context that was added to \p document.
    */
    virtual void didAddMusicalContextToDocument (ARADocument* document, ARAMusicalContext* musicalContext);

    /** Called before a musical context is removed from the document.
        @param document The document that \p musicalContext will be removed from.
        @param musicalContext The musical context that will be removed from \p document.
    */
    virtual void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext);

    /** Called after the musical contexts are reordered in an ARA document

        Musical contexts are sorted by their order index -
        this callback signals a change in this ordering within the document.

        @param document The document with reordered musical contexts.
    */
    virtual void didReorderMusicalContextsInDocument (ARADocument* document);

    /** Called after a region sequence is added to the document.
        @param document The document that \p regionSequence was added to.
        @param regionSequence The region sequence that was added to \p document.
    */
    virtual void didAddRegionSequenceToDocument (ARADocument* document, ARARegionSequence* regionSequence);

    /** Called before a region sequence is removed from the document.
        @param document The document that \p regionSequence will be removed from.
        @param regionSequence The region sequence that will be removed from \p document.
    */
    virtual void willRemoveRegionSequenceFromDocument (ARADocument* document, ARARegionSequence* regionSequence);

    /** Called after the region sequences are reordered in an ARA document

        Region sequences are sorted by their order index -
        this callback signals a change in this ordering within the document.

        @param document The document with reordered region sequences.
    */
    virtual void didReorderRegionSequencesInDocument (ARADocument* document);

    /** Called after an audio source is added to the document.
        @param document The document that \p audioSource was added to.
        @param audioSource The audio source that was added to \p document.
    */
    virtual void didAddAudioSourceToDocument (ARADocument* document, ARAAudioSource* audioSource);

    /** Called before an audio source is removed from the document.
        @param document The document that \p audioSource will be removed from .
        @param audioSource The audio source that will be removed from \p document.
    */
    virtual void willRemoveAudioSourceFromDocument (ARADocument* document, ARAAudioSource* audioSource);

    /** Called before the document is destroyed by the ARA host.
        @param document The document that will be destroyed.
    */
    virtual void willDestroyDocument (ARADocument* document);
};

//==============================================================================
/** Base class representing an ARA document.

    @tags{ARA}
*/
class JUCE_API  ARADocument   : public ARA::PlugIn::Document,
                                public ARAListenableModelClass<ARADocumentListener>,
                                public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARADocumentProperties>;
    using Listener = ARADocumentListener;

    using ARA::PlugIn::Document::Document;

    /** Returns the result of ARA::PlugIn::Document::getAudioSources() with the pointers within
        cast to ARAAudioSource*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateAudioSource(), then
        you can use the template parameter to cast the pointers to your subclass of ARAAudioSource.
    */
    template <typename AudioSource_t = ARAAudioSource>
    const std::vector<AudioSource_t*>& getAudioSources() const noexcept
    {
        return ARA::PlugIn::Document::getAudioSources<AudioSource_t>();
    }

    /** Returns the result of ARA::PlugIn::Document::getMusicalContexts() with the pointers within
        cast to ARAMusicalContext*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateMusicalContext(), then
        you can use the template parameter to cast the pointers to your subclass of ARAMusicalContext.
    */
    template <typename MusicalContext_t = ARAMusicalContext>
    const std::vector<MusicalContext_t*>& getMusicalContexts() const noexcept
    {
        return ARA::PlugIn::Document::getMusicalContexts<MusicalContext_t>();
    }

    /** Returns the result of ARA::PlugIn::Document::getRegionSequences() with the pointers within
        cast to ARARegionSequence*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateRegionSequence(), then
        you can use the template parameter to cast the pointers to your subclass of ARARegionSequence.
    */
    template <typename RegionSequence_t = ARARegionSequence>
    const std::vector<RegionSequence_t*>& getRegionSequences() const noexcept
    {
        return ARA::PlugIn::Document::getRegionSequences<RegionSequence_t>();
    }

    size_t getNumChildren() const noexcept override;

    ARAObject* getChild (size_t index) override;

    ARAObject* getParent() override { return nullptr; }

    void visit (ARAObjectVisitor& visitor) override { visitor.visitDocument (*this); }
};

/** A base class for listeners that want to know about changes to an ARAMusicalContext object.

    Use ARAMusicalContext::addListener() to register your listener with an ARAMusicalContext.

    @tags{ARA}
*/
class JUCE_API  ARAMusicalContextListener
{
public:
    virtual ~ARAMusicalContextListener() = default;

    /** Called before the musical context's properties are updated.
        @param musicalContext The musical context whose properties will be updated.
        @param newProperties The musical context properties that will be assigned to \p musicalContext.
    */
    virtual void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext,
                                                     ARA::PlugIn::PropertiesPtr<ARA::ARAMusicalContextProperties> newProperties);

    /** Called after the musical context's properties are updated by the host.
        @param musicalContext The musical context whose properties were updated.
    */
    virtual void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext);

    /** Called when the musical context's content (i.e tempo entries or chords) changes.
        @param musicalContext The musical context with updated content.
        @param scopeFlags The scope of the content update indicating what has changed.
    */
    virtual void doUpdateMusicalContextContent (ARAMusicalContext* musicalContext, ARAContentUpdateScopes scopeFlags);

    /** Called after a region sequence is added to the musical context.
        @param musicalContext The musical context that \p regionSequence was added to.
        @param regionSequence The region sequence that was added to \p musicalContext.
    */
    virtual void didAddRegionSequenceToMusicalContext (ARAMusicalContext* musicalContext, ARARegionSequence* regionSequence);

    /** Called before a region sequence is removed from the musical context.
        @param musicalContext The musical context that \p regionSequence will be removed from.
        @param regionSequence The region sequence that will be removed from \p musicalContext.
    */
    virtual void willRemoveRegionSequenceFromMusicalContext (ARAMusicalContext* musicalContext,
                                                             ARARegionSequence* regionSequence);

    /** Called after the region sequences are reordered in an ARA MusicalContext

        Region sequences are sorted by their order index -
        this callback signals a change in this ordering within the musical context.

        @param musicalContext The musical context with reordered region sequences.
    */
    virtual void didReorderRegionSequencesInMusicalContext (ARAMusicalContext* musicalContext);

    /** Called before the musical context is destroyed.
        @param musicalContext The musical context that will be destroyed.
    */
    virtual void willDestroyMusicalContext (ARAMusicalContext* musicalContext);
};

//==============================================================================
/** Base class representing an ARA musical context.

    @tags{ARA}
*/
class JUCE_API  ARAMusicalContext     : public ARA::PlugIn::MusicalContext,
                                        public ARAListenableModelClass<ARAMusicalContextListener>,
                                        public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARAMusicalContextProperties>;
    using Listener = ARAMusicalContextListener;

    using ARA::PlugIn::MusicalContext::MusicalContext;

    /** Returns the result of ARA::PlugIn::MusicalContext::getDocument() with the pointer cast
        to ARADocument*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateDocument(), then you
        can use the template parameter to cast the pointers to your subclass of ARADocument.
    */
    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept
    {
        return ARA::PlugIn::MusicalContext::getDocument<Document_t>();
    }

    /** Returns the result of ARA::PlugIn::MusicalContext::getRegionSequences() with the pointers
        within cast to ARARegionSequence*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateRegionSequence(), then you
        can use the template parameter to cast the pointers to your subclass of ARARegionSequence.
    */
    template <typename RegionSequence_t = ARARegionSequence>
    const std::vector<RegionSequence_t*>& getRegionSequences() const noexcept
    {
        return ARA::PlugIn::MusicalContext::getRegionSequences<RegionSequence_t>();
    }

    size_t getNumChildren() const noexcept override { return 0; }

    ARAObject* getChild (size_t) override { return nullptr; }

    ARAObject* getParent() override { return getDocument(); }

    void visit (ARAObjectVisitor& visitor) override { visitor.visitMusicalContext (*this); }
};

/** A base class for listeners that want to know about changes to an ARAPlaybackRegion object.

    Use ARAPlaybackRegion::addListener() to register your listener with an ARAPlaybackRegion.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRegionListener
{
public:
    /** Destructor. */
    virtual ~ARAPlaybackRegionListener() = default;

    /** Called before the playback region's properties are updated.
        @param playbackRegion The playback region whose properties will be updated.
        @param newProperties The playback region properties that will be assigned to \p playbackRegion.
    */
    virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion,
                                                     ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties);

    /** Called after the playback region's properties are updated.
        @param playbackRegion The playback region whose properties were updated.
    */
    virtual void didUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion);

    /** Called when the playback region's content (i.e. samples or notes) changes.
        @param playbackRegion The playback region with updated content.
        @param scopeFlags The scope of the content update.
    */
    virtual void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion,
                                                 ARAContentUpdateScopes scopeFlags);

    /** Called before the playback region is destroyed.
        @param playbackRegion The playback region that will be destroyed.
    */
    virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion);
};

//==============================================================================
/** Base class representing an ARA playback region.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRegion     : public ARA::PlugIn::PlaybackRegion,
                                        public ARAListenableModelClass<ARAPlaybackRegionListener>,
                                        public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties>;
    using Listener = ARAPlaybackRegionListener;

    using ARA::PlugIn::PlaybackRegion::PlaybackRegion;

    /** Returns the result of ARA::PlugIn::PlaybackRegion::getAudioModification() with the pointer cast
        to ARAAudioModification*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateAudioModification(), then you
        can use the template parameter to cast the pointers to your subclass of ARAAudioModification.
    */
    template <typename AudioModification_t = ARAAudioModification>
    AudioModification_t* getAudioModification() const noexcept
    {
        return ARA::PlugIn::PlaybackRegion::getAudioModification<AudioModification_t>();
    }

    /** Returns the result of ARA::PlugIn::PlaybackRegion::getRegionSequence() with the pointer cast
        to ARARegionSequence*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateRegionSequence(), then you
        can use the template parameter to cast the pointers to your subclass of ARARegionSequence.
    */
    template <typename RegionSequence_t = ARARegionSequence>
    RegionSequence_t* getRegionSequence() const noexcept
    {
        return ARA::PlugIn::PlaybackRegion::getRegionSequence<RegionSequence_t>();
    }

    size_t getNumChildren() const noexcept override { return 0; }

    ARAObject* getChild (size_t) override { return nullptr; }

    ARAObject* getParent() override;

    void visit (ARAObjectVisitor& visitor) override { visitor.visitPlaybackRegion (*this); }

    /** Used in getTimeRange to indicate whether the head and tail times should be included in the result.
    */
    enum class IncludeHeadAndTail { no, yes };

    /** Returns the playback time range of this playback region.
        @param includeHeadAndTail Whether or not the range includes the head and tail
                                  time of all playback regions in the sequence.
    */
    Range<double> getTimeRange (IncludeHeadAndTail includeHeadAndTail = IncludeHeadAndTail::no) const;

    /** Returns the playback sample range of this playback region.
        @param sampleRate         The rate at which the sample position should be calculated from
                                  the time range.
        @param includeHeadAndTail Whether or not the range includes the head and tail
                                  time of all playback regions in the sequence.
    */
    Range<int64> getSampleRange (double sampleRate, IncludeHeadAndTail includeHeadAndTail = IncludeHeadAndTail::no) const;

    /** Get the head length in seconds before the start of the region's playback time. */
    double getHeadTime() const;

    /** Get the tail length in seconds after the end of the region's playback time. */
    double getTailTime() const;

    /** Notify the ARA host and any listeners of a content update initiated by the plug-in.
        This must be called by the plug-in model management code on the message thread whenever updating
        the internal content representation, such as after the user edited the pitch of a note in the
        underlying audio modification.
        Listeners will be notified immediately. If \p notifyARAHost is true, a notification to the host
        will be enqueued and sent out the next time it polls for updates.

        @param scopeFlags The scope of the content update.
        @param notifyARAHost If true, the ARA host will be notified of the content change.
    */
    void notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost);
};

/** A base class for listeners that want to know about changes to an ARARegionSequence object.

    Use ARARegionSequence::addListener() to register your listener with an ARARegionSequence.

    @tags{ARA}
*/
class JUCE_API  ARARegionSequenceListener
{
public:
    /** Destructor. */
    virtual ~ARARegionSequenceListener() = default;

    /** Called before the region sequence's properties are updated.
        @param regionSequence The region sequence whose properties will be updated.
        @param newProperties The region sequence properties that will be assigned to \p regionSequence.
    */
    virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence,
                                                     ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties);

    /** Called after the region sequence's properties are updated.
        @param regionSequence The region sequence whose properties were updated.
    */
    virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence);

    /** Called before a playback region is removed from the region sequence.
        @param regionSequence The region sequence that \p playbackRegion will be removed from.
        @param playbackRegion The playback region that will be removed from \p regionSequence.
    */
    virtual void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence,
                                                             ARAPlaybackRegion* playbackRegion);

    /** Called after a playback region is added to the region sequence.
        @param regionSequence The region sequence that \p playbackRegion was added to.
        @param playbackRegion The playback region that was added to \p regionSequence.
    */
    virtual void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence,
                                                       ARAPlaybackRegion* playbackRegion);

    /** Called before the region sequence is destroyed.
        @param regionSequence The region sequence that will be destroyed.
    */
    virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence);
};

//==============================================================================
/** Base class representing an ARA region sequence.

    @tags{ARA}
*/
class JUCE_API  ARARegionSequence     : public ARA::PlugIn::RegionSequence,
                                        public ARAListenableModelClass<ARARegionSequenceListener>,
                                        public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties>;
    using Listener = ARARegionSequenceListener;

    using ARA::PlugIn::RegionSequence::RegionSequence;

    /** Returns the result of ARA::PlugIn::RegionSequence::getDocument() with the pointer cast
        to ARADocument*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateDocument(), then you
        can use the template parameter to cast the pointers to your subclass of ARADocument.
    */
    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept
    {
        return ARA::PlugIn::RegionSequence::getDocument<Document_t>();
    }

    /** Returns the result of ARA::PlugIn::RegionSequence::getMusicalContext() with the pointer cast
        to ARAMusicalContext*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateMusicalContext(), then you
        can use the template parameter to cast the pointers to your subclass of ARAMusicalContext.
    */
    template <typename MusicalContext_t = ARAMusicalContext>
    MusicalContext_t* getMusicalContext() const noexcept
    {
        return ARA::PlugIn::RegionSequence::getMusicalContext<MusicalContext_t>();
    }

    /** Returns the result of ARA::PlugIn::RegionSequence::getPlaybackRegions() with the pointers within cast
        to ARAPlaybackRegion*.

        If you have overridden ARADocumentControllerSpecialisation::doCreatePlaybackRegion(), then you
        can use the template parameter to cast the pointers to your subclass of ARAPlaybackRegion.
    */
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    const std::vector<PlaybackRegion_t*>& getPlaybackRegions() const noexcept
    {
        return ARA::PlugIn::RegionSequence::getPlaybackRegions<PlaybackRegion_t>();
    }

    size_t getNumChildren() const noexcept override;

    ARAObject* getChild (size_t index) override;

    ARAObject* getParent() override { return getDocument(); }

    void visit (ARAObjectVisitor& visitor) override { visitor.visitRegionSequence (*this); }

    /** Returns the playback time range covered by the regions in this sequence.
        @param includeHeadAndTail Whether or not the range includes the playback region's head and tail time.
    */
    Range<double> getTimeRange (ARAPlaybackRegion::IncludeHeadAndTail includeHeadAndTail = ARAPlaybackRegion::IncludeHeadAndTail::no) const;

    /** If all audio sources used by the playback regions in this region sequence have the same
        sample rate, this rate is returned here, otherwise 0.0 is returned.

        If the region sequence has no playback regions, this also returns 0.0.
    */
    double getCommonSampleRate() const;
};

/** A base class for listeners that want to know about changes to an ARAAudioSource object.

    Use ARAAudioSource::addListener() to register your listener with an ARAAudioSource.

    @tags{ARA}
*/
class JUCE_API  ARAAudioSourceListener
{
public:
    /** Destructor. */
    virtual ~ARAAudioSourceListener() = default;

    /** Called before the audio source's properties are updated.
        @param audioSource The audio source whose properties will be updated.
        @param newProperties The audio source properties that will be assigned to \p audioSource.
    */
    virtual void willUpdateAudioSourceProperties (ARAAudioSource* audioSource,
                                                  ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties);

    /** Called after the audio source's properties are updated.
        @param audioSource The audio source whose properties were updated.
    */
    virtual void didUpdateAudioSourceProperties (ARAAudioSource* audioSource);

    /** Called when the audio source's content (i.e. samples or notes) changes.
        @param audioSource The audio source with updated content.
        @param scopeFlags The scope of the content update.
    */
    virtual void doUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags);

    /** Called to notify progress when an audio source is being analyzed.
        @param audioSource The audio source being analyzed.
        @param state Indicates start, intermediate update or completion of the analysis.
        @param progress Progress normalized to the 0..1 range.
    */
    virtual void didUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                       ARA::ARAAnalysisProgressState state,
                                                       float progress);

    /** Called before access to an audio source's samples is enabled or disabled.
        @param audioSource The audio source whose sample access state will be changed.
        @param enable A bool indicating whether or not sample access will be enabled or disabled.
    */
    virtual void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource,
                                                     bool enable);

    /** Called after access to an audio source's samples is enabled or disabled.
        @param audioSource The audio source whose sample access state was changed.
        @param enable A bool indicating whether or not sample access was enabled or disabled.
    */
    virtual void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource,
                                                    bool enable);

    /** Called before an audio source is activated or deactivated when being removed / added from the host's undo history.
        @param audioSource The audio source that will be activated or deactivated
        @param deactivate A bool indicating whether \p audioSource was deactivated or activated.
    */
    virtual void willDeactivateAudioSourceForUndoHistory (ARAAudioSource* audioSource,
                                                          bool deactivate);

    /** Called after an audio source is activated or deactivated when being removed / added from the host's undo history.
        @param audioSource The audio source that was activated or deactivated
        @param deactivate A bool indicating whether \p audioSource was deactivated or activated.
    */
    virtual void didDeactivateAudioSourceForUndoHistory (ARAAudioSource* audioSource,
                                                         bool deactivate);

    /** Called after an audio modification is added to the audio source.
        @param audioSource The region sequence that \p audioModification was added to.
        @param audioModification The playback region that was added to \p audioSource.
    */
    virtual void didAddAudioModificationToAudioSource (ARAAudioSource* audioSource,
                                                       ARAAudioModification* audioModification);

    /** Called before an audio modification is removed from the audio source.
        @param audioSource The audio source that \p audioModification will be removed from.
        @param audioModification The audio modification that will be removed from \p audioSource.
    */
    virtual void willRemoveAudioModificationFromAudioSource (ARAAudioSource* audioSource,
                                                             ARAAudioModification* audioModification);

    /** Called before the audio source is destroyed.
        @param audioSource The audio source that will be destroyed.
    */
    virtual void willDestroyAudioSource (ARAAudioSource* audioSource);
};

//==============================================================================
/** Base class representing an ARA audio source.

    @tags{ARA}
*/
class JUCE_API  ARAAudioSource    : public ARA::PlugIn::AudioSource,
                                    public ARAListenableModelClass<ARAAudioSourceListener>,
                                    public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>;
    using ARAAnalysisProgressState = ARA::ARAAnalysisProgressState;
    using Listener = ARAAudioSourceListener;

    using ARA::PlugIn::AudioSource::AudioSource;

    /** Returns the result of ARA::PlugIn::AudioSource::getDocument() with the pointer cast
        to ARADocument*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateDocument(), then you
        can use the template parameter to cast the pointers to your subclass of ARADocument.
    */
    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept
    {
        return ARA::PlugIn::AudioSource::getDocument<Document_t>();
    }

    /** Returns the result of ARA::PlugIn::AudioSource::getAudioModifications() with the pointers
        within cast to ARAAudioModification*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateAudioModification(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAAudioModification.
    */
    template <typename AudioModification_t = ARAAudioModification>
    const std::vector<AudioModification_t*>& getAudioModifications() const noexcept
    {
        return ARA::PlugIn::AudioSource::getAudioModifications<AudioModification_t>();
    }

    size_t getNumChildren() const noexcept override;

    ARAObject* getChild (size_t index) override;

    ARAObject* getParent() override { return getDocument(); }

    void visit (ARAObjectVisitor& visitor) override { visitor.visitAudioSource (*this); }

    /** Notify the ARA host and any listeners of analysis progress.
        Contrary to most ARA functions, this call can be made from any thread.
        The implementation will enqueue these notifications and later post them from the message thread.
        Calling code must ensure start and completion state are always balanced,
        and must send updates in ascending order.
    */
    void notifyAnalysisProgressStarted();

    /** \copydoc notifyAnalysisProgressStarted
        @param progress Progress normalized to the 0..1 range.
    */
    void notifyAnalysisProgressUpdated (float progress);

    /** \copydoc notifyAnalysisProgressStarted
    */
    void notifyAnalysisProgressCompleted();

    /** Notify the ARA host and any listeners of a content update initiated by the plug-in.
        This must be called by the plug-in model management code on the message thread whenever updating
        the internal content representation, such as after successfully analyzing a new tempo map.
        Listeners will be notified immediately. If \p notifyARAHost is true, a notification to the host
        will be enqueued and sent out the next time it polls for updates.
        \p notifyARAHost must be false if the update was triggered by the host via doUpdateAudioSourceContent().
        Furthermore, \p notifyARAHost must be false if the updated content is being restored from an archive.

        @param scopeFlags The scope of the content update.
        @param notifyARAHost If true, the ARA host will be notified of the content change.
    */
    void notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost);

public:
    friend ARADocumentController;
    ARA::PlugIn::AnalysisProgressTracker internalAnalysisProgressTracker;
};

/** A base class for listeners that want to know about changes to an ARAAudioModification object.

    Use ARAAudioModification::addListener() to register your listener with an ARAAudioModification.

    @tags{ARA}
*/
class JUCE_API  ARAAudioModificationListener
{
public:
    /** Destructor. */
    virtual ~ARAAudioModificationListener() = default;

    /** Called before the audio modification's properties are updated.
        @param audioModification The audio modification whose properties will be updated.
        @param newProperties The audio modification properties that will be assigned to \p audioModification.
    */
    virtual void willUpdateAudioModificationProperties (ARAAudioModification* audioModification,
                                                        ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties> newProperties);

    /** Called after the audio modification's properties are updated.
        @param audioModification The audio modification whose properties were updated.
    */
    virtual void didUpdateAudioModificationProperties (ARAAudioModification* audioModification);

    /** Called when the audio modification's content (i.e. samples or notes) changes.
        @param audioModification The audio modification with updated content.
        @param scopeFlags The scope of the content update.
    */
    virtual void didUpdateAudioModificationContent (ARAAudioModification* audioModification,
                                                    ARAContentUpdateScopes scopeFlags);

    /** Called before an audio modification is activated or deactivated when being removed / added from the host's undo history.
        @param audioModification The audio modification that was activated or deactivated
        @param deactivate A bool indicating whether \p audioModification was deactivated or activated.
    */
    virtual void willDeactivateAudioModificationForUndoHistory (ARAAudioModification* audioModification,
                                                                bool deactivate);

    /** Called after an audio modification is activated or deactivated when being removed / added from the host's undo history.
        @param audioModification The audio modification that was activated or deactivated
        @param deactivate A bool indicating whether \p audioModification was deactivated or activated.
    */
    virtual void didDeactivateAudioModificationForUndoHistory (ARAAudioModification* audioModification,
                                                               bool deactivate);

    /** Called after a playback region is added to the audio modification.
        @param audioModification The audio modification that \p playbackRegion was added to.
        @param playbackRegion The playback region that was added to \p audioModification.
    */
    virtual void didAddPlaybackRegionToAudioModification (ARAAudioModification* audioModification,
                                                          ARAPlaybackRegion* playbackRegion);

    /** Called before a playback region is removed from the audio modification.
        @param audioModification The audio modification that \p playbackRegion will be removed from.
        @param playbackRegion The playback region that will be removed from \p audioModification.
    */
    virtual void willRemovePlaybackRegionFromAudioModification (ARAAudioModification* audioModification,
                                                                ARAPlaybackRegion* playbackRegion);

    /** Called before the audio modification is destroyed.
        @param audioModification The audio modification that will be destroyed.
    */
    virtual void willDestroyAudioModification (ARAAudioModification* audioModification);
};

//==============================================================================
/** Base class representing an ARA audio modification.

    @tags{ARA}
*/
class JUCE_API  ARAAudioModification  : public ARA::PlugIn::AudioModification,
                                        public ARAListenableModelClass<ARAAudioModificationListener>,
                                        public ARAObject
{
public:
    using PropertiesPtr = ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties>;
    using Listener = ARAAudioModificationListener;

    using ARA::PlugIn::AudioModification::AudioModification;

    /** Returns the result of ARA::PlugIn::AudioModification::getAudioSource() with the pointer cast
        to ARAAudioSource*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateAudioSource(), then you
        can use the template parameter to cast the pointers to your subclass of ARAAudioSource.
    */
    template <typename AudioSource_t = ARAAudioSource>
    AudioSource_t* getAudioSource() const noexcept
    {
        return ARA::PlugIn::AudioModification::getAudioSource<AudioSource_t>();
    }

    /** Returns the result of ARA::PlugIn::AudioModification::getPlaybackRegions() with the
        pointers within cast to ARAPlaybackRegion*.

        If you have overridden ARADocumentControllerSpecialisation::doCreatePlaybackRegion(), then
        you can use the template parameter to cast the pointers to your subclass of ARAPlaybackRegion.
    */
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    const std::vector<PlaybackRegion_t*>& getPlaybackRegions() const noexcept
    {
        return ARA::PlugIn::AudioModification::getPlaybackRegions<PlaybackRegion_t>();
    }

    size_t getNumChildren() const noexcept override;

    ARAObject* getChild (size_t index) override;

    ARAObject* getParent() override { return getAudioSource(); }

    void visit (ARAObjectVisitor& visitor) override { visitor.visitAudioModification (*this); }

    /** Notify the ARA host and any listeners of a content update initiated by the plug-in.
        This must be called by the plug-in model management code on the message thread whenever updating
        the internal content representation, such as after the user editing the pitch of a note.
        Listeners will be notified immediately. If \p notifyARAHost is true, a notification to the host
        will be enqueued and sent out the next time it polls for updates.
        \p notifyARAHost must be false if the updated content is being restored from an archive.

        @param scopeFlags The scope of the content update.
        @param notifyARAHost If true, the ARA host will be notified of the content change.
    */
    void notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost);
};

} // namespace juce
