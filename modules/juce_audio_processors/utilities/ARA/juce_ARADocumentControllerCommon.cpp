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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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
