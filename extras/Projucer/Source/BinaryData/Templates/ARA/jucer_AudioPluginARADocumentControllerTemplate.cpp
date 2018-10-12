/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

%%aradocumentcontroller_headers%%

//==============================================================================
%%aradocumentcontroller_class_name%%::%%aradocumentcontroller_class_name%%()
{
}

%%aradocumentcontroller_class_name%%::~%%aradocumentcontroller_class_name%%()
{
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController ()
{
    return new %%aradocumentcontroller_class_name%%();
};
