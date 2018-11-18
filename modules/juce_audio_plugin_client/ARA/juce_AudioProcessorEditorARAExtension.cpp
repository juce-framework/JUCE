#include "juce_AudioProcessorEditorARAExtension.h"

namespace juce
{

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
: araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{
    if (isARAEditorView())
        getARAEditorView()->setEditorIsOpen (true);
}

AudioProcessorEditorARAExtension::~AudioProcessorEditorARAExtension()
{
    if (isARAEditorView())
        getARAEditorView()->setEditorIsOpen (false);
}

ARAEditorView* AudioProcessorEditorARAExtension::getARAEditorView() const noexcept
{
    return araProcessorExtension ? static_cast<ARAEditorView*>(araProcessorExtension->getARAEditorView()) : nullptr;
}

ARADocumentController* AudioProcessorEditorARAExtension::getARADocumentController() const noexcept
{
    return isARAEditorView() ? static_cast<ARADocumentController*>(getARAEditorView()->getDocumentController()) : nullptr;
}

} // namespace juce
