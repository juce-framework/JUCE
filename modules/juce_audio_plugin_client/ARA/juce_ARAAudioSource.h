#pragma once

#include "juce_ARAUtils.h"

namespace juce
{

class ARAAudioSource : public ARA::PlugIn::AudioSource
{
public:
    ARAAudioSource (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef);
    virtual ~ARAAudioSource();

    AudioFormatReader* newReader();

    // Needs to be called in the document controller's `willUpdateAudioSourceProperties` method.
    void willUpdateProperties();

    // Needs to be called in the document controller's `didUpdateAudioSourceProperties` method.
    void didUpdateProperties();

    // Needs to be called in the document controller's `willEnableAudioSourceSamplesAccess` method.
    void willEnableSamplesAccess (bool enable);

    // Needs to be called in the document controller's `didEnableAudioSourceSamplesAccess` method.
    void didEnableSamplesAccess (bool enable);

private:
    void invalidateReaders();

    class Reader;
    typedef SafeRef<ARAAudioSource> Ref;

    Ref::Ptr ref_;

    // Active readers.
    std::vector<Reader*> readers_;

#if JUCE_DEBUG
    bool stateUpdateProperties = false, stateEnableSamplesAccess = false;
#endif
};

} // namespace juce
