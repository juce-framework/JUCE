#pragma once

#include "juce_AudioProcessorARAExtension.h"

namespace juce
{

class AudioProcessor;
class ARAEditorView;
class ARADocumentController;

class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension();

    ARAEditorView* getARAEditorView() const noexcept;

    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    ARADocumentController* getARADocumentController() const noexcept;

private:
    AudioProcessorARAExtension* araProcessorExtension = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
