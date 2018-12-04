#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARADocumentController;
class ARADocument;
class ARAMusicalContext;
class ARARegionSequence;
class ARAAudioSource;
class ARAAudioModification;
class ARAPlaybackRegion;

template<typename Listener>
class ARAModelClass
{
public:
    ARAModelClass() = default;
    virtual ~ARAModelClass() = default;

	inline void addListener(Listener* l) { listeners.add (l); }
	inline void removeListener(Listener* l) { listeners.remove (l); }

	template<typename Callback>
	inline void notifyListeners (Callback&& callback) { listeners.callExpectingUnregistration (callback); } \

private:
	ListenerList<Listener> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAModelClass)
};

// TODO JUCE_ARA
// Do we want to use this Listener pattern?
template<class ModelClassType>
class ARAModelClass_0
{
public:
    class Listener_0 
    {
    public:
        Listener_0() = default;
        virtual ~Listener_0 () = default;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener_0)
    };

    ARAModelClass_0() = default;
    virtual ~ARAModelClass_0() = default;

    inline void addListener(Listener_0* l) { listeners.add (l); }
    inline void removeListener(Listener_0* l) { listeners.remove (l); }

    template<typename Callback>
    inline void notifyListeners (Callback&& callback) { listeners.callExpectingUnregistration (callback); } \

private:
    ListenerList<Listener_0> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAModelClass_0)
};

class ARADocument: public ARA::PlugIn::Document,
                   public ARAModelClass_0<ARADocument>
{
public:
    ARADocument (ARADocumentController* documentController);

    class Listener: public ARAModelClass_0<ARADocument>::Listener_0
    {
    public:

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willBeginEditing (ARADocument* document) {}
        virtual void didEndEditing (ARADocument* document) {}
        virtual void willUpdateDocumentProperties (ARADocument* document, ARADocument::PropertiesPtr newProperties) {}
        virtual void didUpdateDocumentProperties (ARADocument* document) {}
        virtual void didReorderRegionSequencesInDocument (ARADocument* document) {}
        virtual void didAddMusicalContextToDocument (ARADocument* document, ARAMusicalContext* musicalContext) {}
        virtual void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext) {}
        virtual void didAddRegionSequenceToDocument (ARADocument* document, ARARegionSequence* regionSequence) {}
        virtual void willRemoveRegionSequenceFromDocument (ARADocument* document, ARARegionSequence* regionSequence) {}
        virtual void didAddAudioSourceToDocument (ARADocument* document, ARAAudioSource* audioSource) {}
        virtual void willRemoveAudioSourceFromDocument (ARADocument* document, ARAAudioSource* audioSource) {}
        virtual void willDestroyDocument (ARADocument* document) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocument)
};

//==============================================================================
class _ARAMusicalContextListener;
class ARAMusicalContext : public ARA::PlugIn::MusicalContext, 
                          public ARAModelClass<_ARAMusicalContextListener>
{
public:
    ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef);
    
    using Listener = _ARAMusicalContextListener;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAMusicalContext)
};

class _ARAMusicalContextListener
{
public:
    _ARAMusicalContextListener() = default;
    virtual ~_ARAMusicalContextListener() = default;

   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
    virtual void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) {}
    virtual void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext) {}
    virtual void doUpdateMusicalContextContent (ARAMusicalContext* musicalContext, ARAContentUpdateScopes scopeFlags) {}
    virtual void willDestroyMusicalContext (ARAMusicalContext* musicalContext) {}
   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_ARAMusicalContextListener)
};

//==============================================================================
class _ARARegionSequenceListener;
class ARARegionSequence : public ARA::PlugIn::RegionSequence,
                          public ARAModelClass<_ARARegionSequenceListener>
{
public:
    ARARegionSequence (ARADocument* document, ARA::ARARegionSequenceHostRef hostRef);

    using Listener = _ARARegionSequenceListener;

    // If all audio sources used by the playback regions in this region sequence have the
    // same sample rate, this rate is returned here, otherwise 0.0 is returned.
    // If the region sequence has no playback regions, this also returns 0.0.
    double getCommonSampleRate();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARARegionSequence)
};

class _ARARegionSequenceListener
{
public:
    _ARARegionSequenceListener() = default;
    virtual ~_ARARegionSequenceListener() = default;

   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
    virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) {}
    virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) {}
    virtual void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
    virtual void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
    virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence) {}
   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
       
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_ARARegionSequenceListener)
};

class _ARAAudioSourceListener;
class ARAAudioSource : public ARA::PlugIn::AudioSource,
                       public ARAModelClass<_ARAAudioSourceListener>
{
public:
    ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef);

    using Listener = _ARAAudioSourceListener;

    void notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllAudioModificationsAndPlaybackRegions = false);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioSource)
};

class _ARAAudioSourceListener
{
public:
    _ARAAudioSourceListener() = default;
    virtual ~_ARAAudioSourceListener() = default;

   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
    virtual void willUpdateAudioSourceProperties (ARAAudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) {}
    virtual void didUpdateAudioSourceProperties (ARAAudioSource* audioSource) {}
    virtual void doUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags) {}
    virtual void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) {}
    virtual void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) {}
    virtual void doDeactivateAudioSourceForUndoHistory (ARAAudioSource* audioSource, bool deactivate) {}
    virtual void didAddAudioModificationToAudioSource (ARAAudioSource* audioSource, ARAAudioModification* audioModification) {}
    virtual void willRemoveAudioModificationFromAudioSource (ARAAudioSource* audioSource, ARAAudioModification* audioModification) {}
    virtual void willDestroyAudioSource (ARAAudioSource* audioSource) {}
   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
       
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_ARAAudioSourceListener)
};

//==============================================================================
class _ARAAudioModificationListener;
class ARAAudioModification : public ARA::PlugIn::AudioModification,
                             public ARAModelClass<_ARAAudioModificationListener>
{
public:
    ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef);
    
    using Listener = _ARAAudioModificationListener;

    void notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions = false);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioModification)
};

class _ARAAudioModificationListener
{
public:
    _ARAAudioModificationListener() = default;
    virtual ~_ARAAudioModificationListener() = default;

   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
    virtual void willUpdateAudioModificationProperties (ARAAudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) {}
    virtual void didUpdateAudioModificationProperties (ARAAudioModification* audioModification) {}
    virtual void doUpdateAudioModificationContent (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags) {}
    virtual void doDeactivateAudioModificationForUndoHistory (ARAAudioModification* audioModification, bool deactivate) {}
    virtual void didAddPlaybackRegionToAudioModification (ARAAudioModification* audioModification, ARAPlaybackRegion* playbackRegion) {}
    virtual void willRemovePlaybackRegionFromAudioModification (ARAAudioModification* audioModification, ARAPlaybackRegion* playbackRegion) {}
    virtual void willDestroyAudioModification (ARAAudioModification* audioModification) {}
   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
       
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_ARAAudioModificationListener)
};

//==============================================================================
class _ARAPlaybackRegionListener;
class ARAPlaybackRegion: public ARA::PlugIn::PlaybackRegion,
                         public ARAModelClass<_ARAPlaybackRegionListener>
{
public:
    ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef);

    using Listener = _ARAPlaybackRegionListener;

    double getHeadTime() const { return headTime; }
    double getTailTime() const { return tailTime; }
    void setHeadTime (double newHeadTime);
    void setTailTime (double newTailTime);
    void setHeadAndTailTime (double newHeadTime, double newTailTime);

    void notifyContentChanged (ARAContentUpdateScopes scopeFlags);

private:
    double headTime = 0.0;
    double tailTime = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRegion)
};

class _ARAPlaybackRegionListener
{
public:
    _ARAPlaybackRegionListener() = default;
    virtual ~_ARAPlaybackRegionListener() = default;

   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
    virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) {}
    virtual void didUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion) {}
    virtual void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) {}
    virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) {}
   ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
       
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_ARAPlaybackRegionListener)
};

} // namespace juce
