/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

// Diagnostic counters describing Wayland peer state, readable when JUCE_WAYLAND_PEER_DIAGNOSTICS is enabled.
struct WaylandPeerDiagnostics
{
    struct TextInput
    {
        bool available = false;
        bool active = false;
        bool hasSurroundingText = false;
        uint32_t commits = 0;
        int surroundingBytes = 0;
        int cursor = 0;
        int anchor = 0;
        Rectangle<int> caret;
        int compositionLength = 0;
    };

    bool isWaylandBackend = false;
    bool registryGlobalsBound = false;
    bool firstConfigureReceived = false;
    bool frameCallbackFired = false;
    double surfaceScale = 1.0;
    bool fractionalScaleActive = false;
    int lastPreferredScale120 = 0;

    uint64 repaintsRequested = 0;
    uint64 commitsSubmitted = 0;
    uint64 commitsDeferredNoBuffer = 0;
    uint64 frameCallbacksReceived = 0;
    uint64 frameCallbackOnlyCommits = 0;

    int bufferPoolSize = 0;
    int busyBuffers = 0;
    uint64 buffersCreatedTotal = 0;

    int lastCommitDamageRectCount = 0;
    int64 lastCommitDamageArea = 0;
    int lastCommitBufferWidth = 0;
    int lastCommitBufferHeight = 0;

    uint64 configuresReceived = 0;
    int lastConfigureWidth = 0;
    int lastConfigureHeight = 0;
    bool lastConfigureActivated = false;
    bool lastConfigureFullScreen = false;
    bool lastConfigureSuspended = false;

    // A zxdg_toplevel_decoration_v1 mode value, or 0 while no decoration configure has arrived.
    uint32_t lastDecorationMode = 0;
    bool toplevelUsesLibdecor = false;

    int boundOutputs = 0;
    int enteredOutputs = 0;

    TextInput textInput;

    bool seatBound = false;
    bool keyboardBound = false;
    bool pointerBound = false;
    bool touchBound = false;

    uint64 unmapCommits = 0;

    uint64 openGLBoundsUpdates = 0;
    int openGLCropWidth = 0;
    int openGLCropHeight = 0;

    bool usesPopupRole = false;
    bool usesSubsurfaceRole = false;
    bool popupHasGrab = false;
    uint64 popupDoneEvents = 0;
    uint64 popupConfigures = 0;
};

#if JUCE_WAYLAND_PEER_DIAGNOSTICS
std::optional<WaylandPeerDiagnostics> getWaylandPeerDiagnostics (ComponentPeer*);
#endif

} // namespace juce::detail
