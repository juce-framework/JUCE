#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
// Forward declare listener types
class ARARegionSequenceUpdateListener;
class ARAAudioSourceUpdateListener;

// juce ARA document controller implementation
class ARADocumentController: public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController () noexcept {}
    virtual ~ARADocumentController () noexcept {}

    void addRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener);
    void removeRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener);

    void addAudioSourceUpdateListener (ARAAudioSourceUpdateListener* updateListener);
    void removeAudioSourceUpdateListener (ARAAudioSourceUpdateListener* updateListener);
    //==============================================================================
    // Override document controller methods here
protected:

    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept override;
    void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    // needed for ARA AudioFormatReaders to be thread-safe and work properly!
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) noexcept override;
    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    // region sequence update overrides
    // TODO JUCE_ARA we do this so we can notify ARARegionSequenceUpdateListeners, but subclasses of
    // juce::ARADocumentController will have to manually call these functions to get the listener behaviour to work
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void didReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void willReorderRegionSequences () noexcept override;
    void didReorderRegionSequences () noexcept override;

private:
    std::vector<ARARegionSequenceUpdateListener*> regionSequenceUpdateListeners;
    std::vector<ARAAudioSourceUpdateListener*> audioSourceUpdateListeners;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

// Region Sequence update listener
// TODO if we decide to continue with this type of pattern, 
// perhaps we should override all creation functions in ARADocumentController
// so we can forward the creation notifications to our listeners
class ARARegionSequenceUpdateListener
{
public:
    // listener lifetime takes care of adding and removing us from the document controller
    ARARegionSequenceUpdateListener (ARA::PlugIn::DocumentController* documentController);
    virtual ~ARARegionSequenceUpdateListener ();

    ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

    // these are called from the document controller overrides of the same name
    virtual void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept {}
    virtual void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept {}
    virtual void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept {}
    virtual void willReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept {}
    virtual void didReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept {}
    virtual void willReorderRegionSequences () noexcept {}
    virtual void didReorderRegionSequences () noexcept {}

    ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END

    //==============================================================================
private:
    ARADocumentController* araDocumentController;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARARegionSequenceUpdateListener)
};

class ARAAudioSourceUpdateListener
{
public:
    ARAAudioSourceUpdateListener (ARA::PlugIn::DocumentController* documentController);
    virtual ~ARAAudioSourceUpdateListener ();

    ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

    // listener lifetime takes care of adding and removing us from the document controller
    virtual void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) noexcept {}
    virtual void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept {}
    virtual void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept {}
    virtual void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept {}
    virtual void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept {}
    virtual void doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept {}
    virtual void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept {}

    ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END

        //==============================================================================
private:
    ARADocumentController* araDocumentController;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioSourceUpdateListener)
};


} // namespace juce
