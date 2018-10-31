#include "juce_AudioProcessorEditorARAExtension.h"

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension(AudioProcessor* audioProcessor)
: araProcessorExtension(dynamic_cast<AudioProcessorARAExtension*>(audioProcessor))
{}

ARA::PlugIn::EditorView* AudioProcessorEditorARAExtension::getARAEditorView() const noexcept
{
    return araProcessorExtension ? araProcessorExtension->getARAEditorView() : nullptr;
}

bool AudioProcessorEditorARAExtension::isARAEditorView() const noexcept
{
    return getARAEditorView() != nullptr;
}