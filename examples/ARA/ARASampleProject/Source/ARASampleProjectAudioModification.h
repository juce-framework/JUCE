#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioModification    : public ARAAudioModification
{
public:
    using ARAAudioModification::ARAAudioModification;

    bool getReversePlayback() const { return reversePlayback; }
    void setReversePlayback (bool reverse) { reversePlayback = reverse; }

private:
    bool reversePlayback { false };
};