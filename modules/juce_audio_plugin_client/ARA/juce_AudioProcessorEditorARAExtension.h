#pragma once

#include "juce_AudioProcessorARAExtension.h"

namespace juce
{

class AudioProcessor;

class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension(AudioProcessor* audioProcessor);

    ARA::PlugIn::EditorView* getARAEditorView() const noexcept;

    bool isARAEditorView() const noexcept;

private:
    AudioProcessorARAExtension* araProcessorExtension;
};

} // namespace juce