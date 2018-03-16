/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

%%editor_headers%%

//==============================================================================
/**
*/
class %%editor_class_name%%  : public AudioProcessorEditor
{
public:
    %%editor_class_name%% (%%filter_class_name%%&);
    ~%%editor_class_name%%();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    %%filter_class_name%%& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%editor_class_name%%)
};
