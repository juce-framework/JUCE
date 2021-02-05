/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

#include <juce_audio_plugin_client/ARA/juce_ARADocumentController.h>

//==============================================================================
/**
*/
class %%aradocumentcontroller_class_name%%  : public juce::ARADocumentController
{
public:
    using juce::ARADocumentController::ARADocumentController;

    //==============================================================================
    %%aradocumentcontroller_class_name%%(const ARA::ARADocumentControllerHostInstance* instance);
    ~%%aradocumentcontroller_class_name%%();

protected:
    //==============================================================================
    // Override document controller customization methods here

    bool doRestoreObjectsFromStream (juce::ARAInputStream& input, const juce::ARARestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToStream (juce::ARAOutputStream& output, const juce::ARAStoreObjectsFilter* filter) noexcept override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%aradocumentcontroller_class_name%%)
};
