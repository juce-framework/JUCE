/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace build_tools
{
    class PlistOptions final
    {
    public:
        void write (const File& infoPlistFile) const;

        //==============================================================================
        ProjectType::Target::Type type          = ProjectType::Target::Type::GUIApp;

        String executableName;
        String bundleIdentifier;

        String plistToMerge;

        bool iOS                                = false;

        bool microphonePermissionEnabled        = false;
        String microphonePermissionText;

        bool cameraPermissionEnabled            = false;
        String cameraPermissionText;

        bool bluetoothPermissionEnabled         = false;
        String bluetoothPermissionText;

        bool sendAppleEventsPermissionEnabled   = false;
        String sendAppleEventsPermissionText;

        bool shouldAddStoryboardToProject       = false;
        String storyboardName;

        File iconFile;
        String projectName;
        String marketingVersion;
        String currentProjectVersion;
        String companyCopyright;

        String applicationCategory;

        StringPairArray allPreprocessorDefs;
        String documentExtensions;

        bool fileSharingEnabled                 = false;
        bool documentBrowserEnabled             = false;
        bool statusBarHidden                    = false;
        bool requiresFullScreen                 = false;
        bool backgroundAudioEnabled             = false;
        bool backgroundBleEnabled               = false;
        bool pushNotificationsEnabled           = false;

        bool enableIAA                          = false;
        String IAAPluginName;
        String pluginManufacturerCode;
        String IAATypeCode;
        String pluginCode;

        StringArray iPhoneScreenOrientations;
        StringArray iPadScreenOrientations;

        String pluginName;
        String pluginManufacturer;
        String pluginDescription;
        String pluginAUExportPrefix;
        String auMainType;
        bool isAuSandboxSafe                    = false;
        bool isPluginSynth                      = false;
        bool suppressResourceUsage              = false;
        bool isPluginARAEffect                  = false;

    private:
        void write (MemoryOutputStream&) const;
        std::unique_ptr<XmlElement> createXML() const;
        void addIosScreenOrientations (XmlElement&) const;
        void addIosBackgroundModes (XmlElement&) const;
        Array<XmlElement> createExtraAudioUnitTargetPlistOptions() const;
        Array<XmlElement> createExtraAudioUnitV3TargetPlistOptions() const;
    };
}
}
