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

#pragma once

namespace juce
{

class AudioProcessor;
class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;

//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessor.

    Subclassing AudioProcessorARAExtension allows access to the three possible plugin instance
    roles as defined by the ARA SDK. Hosts can assign any subset of roles to each plugin instance.

    @tags{ARA}
*/
class JUCE_API  AudioProcessorARAExtension  : public ARA::PlugIn::PlugInExtension
{
public:
    AudioProcessorARAExtension() = default;

    //==============================================================================
    /** Returns the result of ARA::PlugIn::PlugInExtension::getPlaybackRenderer() with the pointer
        cast to ARAPlaybackRenderer*.

        If you have overridden ARADocumentControllerSpecialisation::doCreatePlaybackRenderer(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAPlaybackRenderer.
    */
    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getPlaybackRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getPlaybackRenderer<PlaybackRenderer_t>();
    }

    /** Returns the result of ARA::PlugIn::PlugInExtension::getEditorRenderer() with the pointer
        cast to ARAEditorRenderer*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateEditorRenderer(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAEditorRenderer.
    */
    template <typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getEditorRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorRenderer<EditorRenderer_t>();
    }

    /** Returns the result of ARA::PlugIn::PlugInExtension::getEditorView() with the pointer
        cast to ARAEditorView*.

        If you have overridden ARADocumentControllerSpecialisation::doCreateEditorView(),
        then you can use the template parameter to cast the pointers to your subclass of
        ARAEditorView.
    */
    template <typename EditorView_t = ARAEditorView>
    EditorView_t* getEditorView() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorView<EditorView_t>();
    }

    //==============================================================================
    /** Returns true if plugin instance fulfills the ARAPlaybackRenderer role. */
    bool isPlaybackRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getPlaybackRenderer() != nullptr;
    }

    /** Returns true if plugin instance fulfills the ARAEditorRenderer role. */
    bool isEditorRenderer() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorRenderer() != nullptr;
    }

    /** Returns true if plugin instance fulfills the ARAEditorView role. */
    bool isEditorView() const noexcept
    {
        return ARA::PlugIn::PlugInExtension::getEditorView() != nullptr;
    }

    //==============================================================================
#if ARA_VALIDATE_API_CALLS
    bool isPrepared { false };
#endif

protected:
    /** Implementation helper for AudioProcessor::getTailLengthSeconds().

        If bound to ARA, this traverses the instance roles to retrieve the respective tail time
        and returns true. Otherwise returns false and leaves tailLength unmodified.
    */
    bool getTailLengthSecondsForARA (double& tailLength) const;

    /** Implementation helper for AudioProcessor::prepareToPlay().

        If bound to ARA, this traverses the instance roles to prepare them for play and returns
        true. Otherwise returns false and does nothing.
    */
    bool prepareToPlayForARA (double sampleRate,
                              int samplesPerBlock,
                              int numChannels,
                              AudioProcessor::ProcessingPrecision precision);

    /** Implementation helper for AudioProcessor::releaseResources().

        If bound to ARA, this traverses the instance roles to let them release resources and returns
        true. Otherwise returns false and does nothing.
    */
    bool releaseResourcesForARA();

    /** Implementation helper for AudioProcessor::processBlock().

        If bound to ARA, this traverses the instance roles to let them process the block and returns
        true. Otherwise returns false and does nothing.

        Use this overload if your rendering code already has a current positionInfo available.
    */
    bool processBlockForARA (AudioBuffer<float>& buffer,
                             AudioProcessor::Realtime realtime,
                             const AudioPlayHead::PositionInfo& positionInfo);

    /** Implementation helper for AudioProcessor::processBlock().

        If bound to ARA, this traverses the instance roles to let them process the block and returns
        true. Otherwise returns false and does nothing.

        Use this overload if your rendering code does not have a current positionInfo available.
    */
    bool processBlockForARA (AudioBuffer<float>& buffer, AudioProcessor::Realtime isNonRealtime, AudioPlayHead* playhead);

    //==============================================================================
    /** Optional hook for derived classes to perform any additional initialization that may be needed.

        If overriding this, make sure you call the base class implementation from your override.
    */
    void didBindToARA() noexcept override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};

//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessorEditor.

    Subclassing AudioProcessorARAExtension allows access to the ARAEditorView instance role as
    described by the ARA SDK.

    @tags{ARA}
*/
class JUCE_API  AudioProcessorEditorARAExtension
{
public:
    /** Constructor. */
    explicit AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);

    /** \copydoc AudioProcessorARAExtension::getEditorView */
    template <typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept
    {
        return (this->araProcessorExtension != nullptr) ? this->araProcessorExtension->getEditorView<EditorView_t>()
                                                        : nullptr;
    }

    /** \copydoc AudioProcessorARAExtension::isEditorView */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

protected:
    /** Destructor. */
    ~AudioProcessorEditorARAExtension();

private:
    AudioProcessorARAExtension* araProcessorExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
