#include "juce_AudioProcessor_ARAExtensions.h"

namespace juce
{

void AudioProcessorARAExtension::didBindToARA() noexcept
{
#if (! JUCE_DISABLE_ASSERTIONS)
    // validate proper subclassing of the instance role classes
    if (isARAPlaybackRenderer())
        jassert (dynamic_cast<ARAPlaybackRenderer*> (getARAPlaybackRenderer()) != nullptr);
    if (isARAEditorRenderer())
        jassert (dynamic_cast<ARAEditorRenderer*> (getARAEditorRenderer()) != nullptr);
    if (isARAEditorView())
        jassert (dynamic_cast<ARAEditorView*> (getARAEditorView()) != nullptr);
#endif

    if (isARAPlaybackRenderer())
        getARAPlaybackRenderer()->setAudioProcessor (dynamic_cast<AudioProcessor*> (this));
    if (isARAEditorRenderer())
        getARAEditorRenderer()->setAudioProcessor (dynamic_cast<AudioProcessor*> (this));
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
