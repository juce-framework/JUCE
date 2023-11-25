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

namespace juce
{

class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    using ARA::PlugIn::DocumentController::DocumentController;

    template <typename Document_t = ARADocument>
    Document_t* getDocument() const noexcept { return ARA::PlugIn::DocumentController::getDocument<Document_t>(); }

    //==============================================================================
    /** @internal */
    virtual void internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource) = 0;

    /** @internal */
    virtual void internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress) = 0;

    /** @internal */
    virtual void internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource) = 0;

    /** @internal */
    virtual void internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource,
                                                               ARAAudioSource::ARAAnalysisProgressState state,
                                                               float progress) = 0;

    //==============================================================================
    /** @internal */
    virtual void internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource,
                                                          ARAContentUpdateScopes scopeFlags,
                                                          bool notifyARAHost) = 0;

    /** @internal */
    virtual void internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification,
                                                                ARAContentUpdateScopes scopeFlags,
                                                                bool notifyARAHost) = 0;

    /** @internal */
    virtual void internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion,
                                                             ARAContentUpdateScopes scopeFlags,
                                                             bool notifyARAHost) = 0;

    friend class ARAPlaybackRegionReader;
};

} // namespace juce
