#include "juce_AudioProcessor_ARAExtensions.h"

namespace juce
{

// ARARenderer draft note: only validation at this point, we could probably get rid of didBindToARA() now?
void AudioProcessorARAExtension::didBindToARA() noexcept
{
#if (! JUCE_DISABLE_ASSERTIONS)
    // validate proper subclassing of the instance role classes
    if (isPlaybackRenderer())
        jassert (dynamic_cast<ARAPlaybackRenderer*> (getPlaybackRenderer()) != nullptr);
    if (isEditorRenderer())
        jassert (dynamic_cast<ARAEditorRenderer*> (getEditorRenderer()) != nullptr);
    if (isEditorView())
        jassert (dynamic_cast<ARAEditorView*> (getEditorView()) != nullptr);
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
