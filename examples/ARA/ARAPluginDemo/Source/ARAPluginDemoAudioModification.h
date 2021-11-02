#pragma once

#include "JuceHeader.h"

class ARAPluginDemoAudioModification  : public ARAAudioModification
{
public:
    ARAPluginDemoAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef,
                                       const ARAAudioModification* optionalModificationToClone)
        : ARAAudioModification (audioSource, hostRef, optionalModificationToClone)
    {
        if (auto toClone = static_cast<const ARAPluginDemoAudioModification*> (optionalModificationToClone))
            reversePlayback = toClone->reversePlayback;
    }

    bool getReversePlayback() const { return reversePlayback; }
    void setReversePlayback (bool reverse) { reversePlayback = reverse; }

private:
    bool reversePlayback { false };
};
