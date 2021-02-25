/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

%%aradocumentcontroller_headers%%

//==============================================================================
%%aradocumentcontroller_class_name%%::~%%aradocumentcontroller_class_name%%()
{
}

//==============================================================================
bool %%aradocumentcontroller_class_name%%::doRestoreObjectsFromStream (juce::ARAInputStream& input, const juce::ARARestoreObjectsFilter* filter) noexcept
{
    // You should use this method to read any persistent data associated with
    // your ARA model graph stored in an archive using the supplied ARAInputStream. 
    // Be sure to check the ARARestoreObjectsFilter to determine which objects to restore.
    return true;
}

bool %%aradocumentcontroller_class_name%%::doStoreObjectsToStream (juce::ARAOutputStream& output, const juce::ARAStoreObjectsFilter* filter) noexcept
{
    // You should use this method to write any persistent data associated with
    // your ARA model graph into the an archive using the supplied ARAOutputStream. 
    // Be sure to check the ARAStoreObjectsFilter to determine which objects to store.
    return true;
}

//==============================================================================
// This creates the static ARAFactory instances for the plugin.
const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory()
{
    return juce::ARADocumentController::createARAFactory<%%aradocumentcontroller_class_name%%>();
}
