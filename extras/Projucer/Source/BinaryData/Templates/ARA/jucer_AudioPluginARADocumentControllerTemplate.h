/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

%%app_headers%%

#include "juce_audio_plugin_client/ARA/juce_ARA_interface.h"

//==============================================================================
/**
*/
class %%aradocumentcontroller_class_name%%  : public ARA::PlugIn::DocumentController
{
public:
    %%aradocumentcontroller_class_name%%();
    ~%%aradocumentcontroller_class_name%%();

//==============================================================================
// Override document controller methods here
protected:


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%aradocumentcontroller_class_name%%)
};