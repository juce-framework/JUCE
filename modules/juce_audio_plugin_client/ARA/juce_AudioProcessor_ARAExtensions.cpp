#include "juce_AudioProcessor_ARAExtensions.h"

namespace juce
{

//==============================================================================
bool AudioProcessorARAExtension::getTailLengthSecondsForARA (double& tailLength) const
{
    if (! isBoundToARA())
        return false;

    tailLength = 0.0;
    if (auto playbackRenderer = getPlaybackRenderer())
        for (const auto& playbackRegion : playbackRenderer->getPlaybackRegions())
            tailLength = jmax (tailLength, playbackRegion->getTailTime());
    return true;
}

bool AudioProcessorARAExtension::prepareToPlayForARA (double sampleRate, int samplesPerBlock, int numChannels)
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = true;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels);

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->prepareToPlay (sampleRate, samplesPerBlock, numChannels);

    return true;
}

bool AudioProcessorARAExtension::releaseResourcesForARA()
{
#if ARA_VALIDATE_API_CALLS
    isPrepared = false;
#endif

    if (! isBoundToARA())
        return false;

    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->releaseResources();

    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->releaseResources();

    return true;
}

bool AudioProcessorARAExtension::processBlockForARA (AudioBuffer<float>& buffer, bool isNonRealtime, const AudioPlayHead::CurrentPositionInfo& positionInfo)
{
    // validate that the host has prepared us before processing
    ARA_VALIDATE_API_STATE (isPrepared);

    if (! isBoundToARA())
        return false;

    // Render our ARA playback regions for this buffer.
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->processBlock (buffer, isNonRealtime, positionInfo);

    // Render our ARA editor regions and sequences for this buffer.
    // This example does not support editor rendering and thus uses the default implementation,
    // which is a no-op and could be omitted in actual plug-ins to optimize performance.
    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->processBlock (buffer, isNonRealtime, positionInfo);

    return true;
}

bool AudioProcessorARAExtension::processBlockForARA (AudioBuffer<float>& buffer, bool isNonRealtime, AudioPlayHead* playhead)
{
    AudioPlayHead::CurrentPositionInfo positionInfo;
    if (! isBoundToARA() || ! playhead || ! playhead->getCurrentPosition (positionInfo))
        positionInfo.resetToDefault();

    return processBlockForARA(buffer, isNonRealtime, positionInfo);
}

//==============================================================================
void AudioProcessorARAExtension::didBindToARA() noexcept
{
    // validate that the ARA binding is not established by the host while prepared to play
#if ARA_VALIDATE_API_CALLS
    ARA_VALIDATE_API_STATE (! isPrepared);
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->araExtension = this;
#endif

#if (! JUCE_DISABLE_ASSERTIONS)
    // validate proper subclassing of the instance role classes
    if (auto playbackRenderer = getPlaybackRenderer())
        jassert (dynamic_cast<ARAPlaybackRenderer*> (playbackRenderer) != nullptr);
    if (auto editorRenderer = getEditorRenderer())
        jassert (dynamic_cast<ARAEditorRenderer*> (editorRenderer) != nullptr);
    if (auto editorView = getEditorView())
        jassert (dynamic_cast<ARAEditorView*> (editorView) != nullptr);
#endif
}

//==============================================================================

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
    : araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (true);
}

AudioProcessorEditorARAExtension::~AudioProcessorEditorARAExtension()
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (false);
}

} // namespace juce
