/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class ARADocumentController  : public ARA::PlugIn::DocumentController
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
