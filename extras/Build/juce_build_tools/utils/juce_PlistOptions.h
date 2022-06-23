/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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
