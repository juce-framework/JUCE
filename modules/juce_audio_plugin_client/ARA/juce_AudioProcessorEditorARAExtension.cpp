#include "juce_AudioProcessorEditorARAExtension.h"

namespace juce
{

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
: araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{}

ARA::PlugIn::EditorView* AudioProcessorEditorARAExtension::getARAEditorView() const noexcept
{
    return araProcessorExtension ? araProcessorExtension->getARAEditorView() : nullptr;
}

} // namespace juce