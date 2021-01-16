#pragma once

#include <juce_audio_plugin_client/ARA/juce_ARADocumentController.h>

//==============================================================================
// The document controller is the central point of communication between the ARA host and our plug-in.
// While we're not customizing anything in this example, actual plug-ins will do a lot of work here.
class ARAPluginDemoDocumentController     : public juce::ARADocumentController
{
public:
    ARAPluginDemoDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
        :  ARADocumentController (instance)
    {}

    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept override;

    bool doRestoreObjectsFromStream (juce::ARAInputStream& input, const juce::ARARestoreObjectsFilter* filter) noexcept override;
    bool doStoreObjectsToStream (juce::ARAOutputStream& output, const juce::ARAStoreObjectsFilter* filter) noexcept override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoDocumentController)
};
