/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

%%app_headers%%

//==============================================================================
/**
*/
class %%aradocumentcontroller_class_name%%  : public juce::ARADocumentController
{
public:
    //==============================================================================
    %%aradocumentcontroller_class_name%%(const ARA::ARADocumentControllerHostInstance* instance);
    ~%%aradocumentcontroller_class_name%%();

    //==============================================================================
    bool doRestoreObjectsFromStream (juce::ARAInputStream& input, const juce::ARARestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToStream (juce::ARAOutputStream& output, const juce::ARAStoreObjectsFilter* filter) noexcept override;

//==============================================================================
// Override document controller methods here
protected:


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%aradocumentcontroller_class_name%%)
};
