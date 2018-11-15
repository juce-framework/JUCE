#pragma once

#include "juce_AudioProcessorARAExtension.h"

namespace juce
{

class AudioProcessor;
class ARAEditorView;

class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension ();

    // TODO JUCE_ARA return proper JUCE class here once we've added it to the framework to handle selection -
    // see static_cast<ARASampleProjectEditorView*> in ARASampleProjectAudioProcessorEditor
    ARAEditorView* getARAEditorView() const noexcept;

    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

private:
    AudioProcessorARAExtension* araProcessorExtension;
};

} // namespace juce
