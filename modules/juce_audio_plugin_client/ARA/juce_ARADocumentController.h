#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
// Forward declare listener types
class ARARegionSequenceUpdateListener;

// juce ARA document controller implementation
class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController() noexcept {}
    virtual ~ARADocumentController() noexcept {}

    void addRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener);
    void removeRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener);

    //==============================================================================
    // Override document controller methods here
protected:
    // needed for ARA AudioFormatReaders to be thread-safe and work properly!
    ARA::PlugIn::AudioSource* doCreateAudioSource       (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess  (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void willUpdateAudioSourceProperties (
        ARA::PlugIn::AudioSource*, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>) noexcept override;
    void didUpdateAudioSourceProperties     (ARA::PlugIn::AudioSource *audioSource) noexcept override;


    // region sequence update overrides
    virtual void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept override;
    virtual void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    virtual void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    virtual void willReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    virtual void didReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    virtual void willReorderRegionSequences () noexcept override;
    virtual void didReorderRegionSequences () noexcept override;

private:
    std::vector<ARARegionSequenceUpdateListener*> regionSequenceUpdateListeners;

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
    ~ARARegionSequenceUpdateListener ();

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

} // namespace juce
