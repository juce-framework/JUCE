#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

class ARAPluginDemoAudioModification  : public juce::ARAAudioModification
{
public:
    ARAPluginDemoAudioModification (juce::ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef,
                                       const juce::ARAAudioModification* optionalModificationToClone)
        : ARAAudioModification (audioSource, hostRef, optionalModificationToClone)
    {
        if (auto toClone = static_cast<const ARAPluginDemoAudioModification*> (optionalModificationToClone))
            dimmed = toClone->dimmed;
    }

    bool isDimmed() const { return dimmed; }
    void setDimmed (bool shouldDim) { dimmed = shouldDim; }

private:
    bool dimmed { false };
};
