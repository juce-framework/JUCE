/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_XcodeProjectParser.h"

//==============================================================================
namespace
{
    static const char* const iOSDefaultVersion = "9.3";
    static const StringArray iOSVersions { "7.0", "7.1", "8.0", "8.1", "8.2", "8.3", "8.4",
                                           "9.0", "9.1", "9.2", "9.3", "10.0", "10.1", "10.2", "10.3",
                                           "11.0", "12.0", "13.0" };

    static const int oldestDeploymentTarget  = 7;
    static const int defaultDeploymentTarget = 11;
    static const int oldestSDKVersion        = 11;
    static const int currentSDKVersion       = 15;
    static const int minimumAUv3SDKVersion   = 11;

    static String getVersionName    (int version)  { return "10." + String (version); }
    static String getSDKDisplayName (int version)  { return getVersionName (version) + " SDK"; }
    static String getSDKRootName    (int version)  { return "macosx" + getVersionName (version); }

    template<class ContainerType>
    static ContainerType getSDKChoiceList (int oldestVersion, bool displayName)
    {
        ContainerType container;

        for (int v = oldestVersion; v <= currentSDKVersion; ++v)
            container.add (displayName ? getSDKDisplayName (v) : getVersionName (v));

        return container;
    }

    static const char* const osxArch_Default        = "default";
    static const char* const osxArch_Native         = "Native";
    static const char* const osxArch_32BitUniversal = "32BitUniversal";
    static const char* const osxArch_64BitUniversal = "64BitUniversal";
    static const char* const osxArch_64Bit          = "64BitIntel";
}

//==============================================================================
class XcodeProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameMac()                         { return "Xcode (MacOSX)"; }
    static const char* getNameiOS()                         { return "Xcode (iOS)"; }
    static const char* getValueTreeTypeName (bool iOS)      { return iOS ? "XCODE_IPHONE" : "XCODE_MAC"; }

    //==============================================================================
    XcodeProjectExporter (Project& p, const ValueTree& t, const bool isIOS)
        : ProjectExporter (p, t),
          xcodeCanUseDwarf (true),
          iOS (isIOS),
          customPListValue                             (settings, Ids::customPList,                             getUndoManager()),
          pListPrefixHeaderValue                       (settings, Ids::pListPrefixHeader,                       getUndoManager()),
          pListPreprocessValue                         (settings, Ids::pListPreprocess,                         getUndoManager()),
          subprojectsValue                             (settings, Ids::xcodeSubprojects,                        getUndoManager()),
          extraFrameworksValue                         (settings, Ids::extraFrameworks,                         getUndoManager()),
          frameworkSearchPathsValue                    (settings, Ids::frameworkSearchPaths,                    getUndoManager()),
          extraCustomFrameworksValue                   (settings, Ids::extraCustomFrameworks,                   getUndoManager()),
          embeddedFrameworksValue                      (settings, Ids::embeddedFrameworks,                      getUndoManager()),
          postbuildCommandValue                        (settings, Ids::postbuildCommand,                        getUndoManager()),
          prebuildCommandValue                         (settings, Ids::prebuildCommand,                         getUndoManager()),
          duplicateAppExResourcesFolderValue           (settings, Ids::duplicateAppExResourcesFolder,           getUndoManager(), true),
          iosDeviceFamilyValue                         (settings, Ids::iosDeviceFamily,                         getUndoManager(), "1,2"),
          iPhoneScreenOrientationValue                 (settings, Ids::iPhoneScreenOrientation,                 getUndoManager(), "portraitlandscape"),
          iPadScreenOrientationValue                   (settings, Ids::iPadScreenOrientation,                   getUndoManager(), "portraitlandscape"),
          customXcodeResourceFoldersValue              (settings, Ids::customXcodeResourceFolders,              getUndoManager()),
          customXcassetsFolderValue                    (settings, Ids::customXcassetsFolder,                    getUndoManager()),
          appSandboxValue                              (settings, Ids::appSandbox,                              getUndoManager()),
          appSandboxOptionsValue                       (settings, Ids::appSandboxOptions,                       getUndoManager(), Array<var>(), ","),
          hardenedRuntimeValue                         (settings, Ids::hardenedRuntime,                         getUndoManager()),
          hardenedRuntimeOptionsValue                  (settings, Ids::hardenedRuntimeOptions,                  getUndoManager(), Array<var>(), ","),
          microphonePermissionNeededValue              (settings, Ids::microphonePermissionNeeded,              getUndoManager()),
          microphonePermissionsTextValue               (settings, Ids::microphonePermissionsText,               getUndoManager(),
                                                        "This app requires audio input. If you do not have an audio interface connected it will use the built-in microphone."),
          cameraPermissionNeededValue                  (settings, Ids::cameraPermissionNeeded,                  getUndoManager()),
          cameraPermissionTextValue                    (settings, Ids::cameraPermissionText,                    getUndoManager(),
                                                        "This app requires access to the camera to function correctly."),
          iosBluetoothPermissionNeededValue            (settings, Ids::iosBluetoothPermissionNeeded,            getUndoManager()),
          iosBluetoothPermissionTextValue              (settings, Ids::iosBluetoothPermissionText,              getUndoManager(),
                                                        "This app requires access to Bluetooth to function correctly."),
          uiFileSharingEnabledValue                    (settings, Ids::UIFileSharingEnabled,                    getUndoManager()),
          uiSupportsDocumentBrowserValue               (settings, Ids::UISupportsDocumentBrowser,               getUndoManager()),
          uiStatusBarHiddenValue                       (settings, Ids::UIStatusBarHidden,                       getUndoManager()),
          documentExtensionsValue                      (settings, Ids::documentExtensions,                      getUndoManager()),
          iosInAppPurchasesValue                       (settings, Ids::iosInAppPurchases,                       getUndoManager()),
          iosBackgroundAudioValue                      (settings, Ids::iosBackgroundAudio,                      getUndoManager()),
          iosBackgroundBleValue                        (settings, Ids::iosBackgroundBle,                        getUndoManager()),
          iosPushNotificationsValue                    (settings, Ids::iosPushNotifications,                    getUndoManager()),
          iosAppGroupsValue                            (settings, Ids::iosAppGroups,                            getUndoManager()),
          iCloudPermissionsValue                       (settings, Ids::iCloudPermissions,                       getUndoManager()),
          iosDevelopmentTeamIDValue                    (settings, Ids::iosDevelopmentTeamID,                    getUndoManager()),
          iosAppGroupsIDValue                          (settings, Ids::iosAppGroupsId,                          getUndoManager()),
          keepCustomXcodeSchemesValue                  (settings, Ids::keepCustomXcodeSchemes,                  getUndoManager()),
          useHeaderMapValue                            (settings, Ids::useHeaderMap,                            getUndoManager()),
          customLaunchStoryboardValue                  (settings, Ids::customLaunchStoryboard,                  getUndoManager()),
          exporterBundleIdentifierValue                (settings, Ids::bundleIdentifier,                        getUndoManager())
    {
        name = iOS ? getNameiOS() : getNameMac();

        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderForExporter (getValueTreeTypeName (isIOS)));
    }

    static XcodeProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName (false)))  return new XcodeProjectExporter (projectToUse, settingsToUse, false);
        if (settingsToUse.hasType (getValueTreeTypeName (true)))   return new XcodeProjectExporter (projectToUse, settingsToUse, true);

        return nullptr;
    }

    //==============================================================================
    String getPListToMergeString() const               { return customPListValue.get(); }
    String getPListPrefixHeaderString() const          { return pListPrefixHeaderValue.get(); }
    bool isPListPreprocessEnabled() const              { return pListPreprocessValue.get(); }

    String getSubprojectsString() const                { return subprojectsValue.get(); }

    String getExtraFrameworksString() const            { return extraFrameworksValue.get(); }
    String getFrameworkSearchPathsString() const       { return frameworkSearchPathsValue.get(); }
    String getExtraCustomFrameworksString() const      { return extraCustomFrameworksValue.get(); }
    String getEmbeddedFrameworksString() const         { return embeddedFrameworksValue.get(); }

    String getPostBuildScript() const                  { return postbuildCommandValue.get(); }
    String getPreBuildScript() const                   { return prebuildCommandValue.get(); }

    bool shouldDuplicateAppExResourcesFolder() const   { return duplicateAppExResourcesFolderValue.get(); }

    String getDeviceFamilyString() const               { return iosDeviceFamilyValue.get(); }

    String getiPhoneScreenOrientationString() const    { return iPhoneScreenOrientationValue.get(); }
    String getiPadScreenOrientationString() const      { return iPadScreenOrientationValue.get(); }

    String getCustomResourceFoldersString() const      { return customXcodeResourceFoldersValue.get().toString().replaceCharacters ("\r\n", "::"); }
    String getCustomXcassetsFolderString() const       { return customXcassetsFolderValue.get(); }
    String getCustomLaunchStoryboardString() const     { return customLaunchStoryboardValue.get(); }
    bool shouldAddStoryboardToProject() const          { return getCustomLaunchStoryboardString().isNotEmpty() || getCustomXcassetsFolderString().isEmpty(); }

    bool isHardenedRuntimeEnabled() const              { return hardenedRuntimeValue.get(); }
    Array<var> getHardenedRuntimeOptions() const       { return *hardenedRuntimeOptionsValue.get().getArray(); }

    bool isAppSandboxEnabled() const                   { return appSandboxValue.get(); }
    Array<var> getAppSandboxOptions() const            { return *appSandboxOptionsValue.get().getArray(); }

    bool isMicrophonePermissionEnabled() const         { return microphonePermissionNeededValue.get(); }
    String getMicrophonePermissionsTextString() const  { return microphonePermissionsTextValue.get(); }

    bool isCameraPermissionEnabled() const             { return cameraPermissionNeededValue.get(); }
    String getCameraPermissionTextString() const       { return cameraPermissionTextValue.get(); }

    bool isBluetoothPermissionEnabled() const          { return iosBluetoothPermissionNeededValue.get(); }
    String getBluetoothPermissionTextString() const    { return iosBluetoothPermissionTextValue.get(); }

    bool isInAppPurchasesEnabled() const               { return iosInAppPurchasesValue.get(); }
    bool isBackgroundAudioEnabled() const              { return iosBackgroundAudioValue.get(); }
    bool isBackgroundBleEnabled() const                { return iosBackgroundBleValue.get(); }
    bool isPushNotificationsEnabled() const            { return iosPushNotificationsValue.get(); }
    bool isAppGroupsEnabled() const                    { return iosAppGroupsValue.get(); }
    bool isiCloudPermissionsEnabled() const            { return iCloudPermissionsValue.get(); }
    bool isFileSharingEnabled() const                  { return uiFileSharingEnabledValue.get(); }
    bool isDocumentBrowserEnabled() const              { return uiSupportsDocumentBrowserValue.get(); }
    bool isStatusBarHidden() const                     { return uiStatusBarHiddenValue.get(); }

    String getDocumentExtensionsString() const         { return documentExtensionsValue.get(); }

    bool shouldKeepCustomXcodeSchemes() const          { return keepCustomXcodeSchemesValue.get(); }

    String getDevelopmentTeamIDString() const          { return iosDevelopmentTeamIDValue.get(); }
    String getAppGroupIdString() const                 { return iosAppGroupsIDValue.get(); }

    String getDefaultLaunchStoryboardName() const      { jassert (iOS); return "LaunchScreen"; }

    //==============================================================================
    bool usesMMFiles() const override                       { return true; }
    bool canCopeWithDuplicateFiles() override               { return true; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                           { return true; }
    bool isVisualStudio() const override                    { return false; }
    bool isCodeBlocks() const override                      { return false; }
    bool isMakefile() const override                        { return false; }
    bool isAndroidStudio() const override                   { return false; }
    bool isCLion() const override                           { return false; }

    bool isAndroid() const override                         { return false; }
    bool isWindows() const override                         { return false; }
    bool isLinux() const override                           { return false; }
    bool isOSX() const override                             { return ! iOS; }
    bool isiOS() const override                             { return iOS; }

    bool supportsTargetType (ProjectType::Target::Type type) const override
    {
        switch (type)
        {
            case ProjectType::Target::AudioUnitv3PlugIn:
            case ProjectType::Target::StandalonePlugIn:
            case ProjectType::Target::GUIApp:
            case ProjectType::Target::StaticLibrary:
            case ProjectType::Target::DynamicLibrary:
            case ProjectType::Target::SharedCodeTarget:
            case ProjectType::Target::AggregateTarget:
                return true;
            case ProjectType::Target::ConsoleApp:
            case ProjectType::Target::VSTPlugIn:
            case ProjectType::Target::VST3PlugIn:
            case ProjectType::Target::AAXPlugIn:
            case ProjectType::Target::RTASPlugIn:
            case ProjectType::Target::AudioUnitPlugIn:
            case ProjectType::Target::UnityPlugIn:
                return ! iOS;
            default:
                break;
        }

        return false;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        if (iOS)
        {
            props.add (new TextPropertyComponent (customXcassetsFolderValue, "Custom Xcassets Folder", 128, false),
                       "If this field is not empty, your Xcode project will use the custom xcassets folder specified here "
                       "for the app icons and launchimages, and will ignore the Icon files specified above. This will also prevent "
                       "a launch storyboard from being used.");

            props.add (new TextPropertyComponent (customLaunchStoryboardValue, "Custom Launch Storyboard", 256, false),
                       "If this field is not empty then the specified launch storyboard file will be added to the project as an Xcode "
                       "resource and will be used for the app's launch screen, otherwise a default blank launch storyboard will be used. "
                       "The file path should be relative to the project folder.");
        }

        props.add (new TextPropertyComponent (customXcodeResourceFoldersValue, "Custom Xcode Resource Folders", 8192, true),
                   "You can specify a list of custom resource folders here (separated by newlines or whitespace). "
                   "References to these folders will then be added to the Xcode resources. "
                   "This way you can specify them for OS X and iOS separately, and modify the content of the resource folders "
                   "without re-saving the Projucer project.");

        if (getProject().getProjectType().isAudioPlugin())
            props.add (new ChoicePropertyComponent (duplicateAppExResourcesFolderValue, "Add Duplicate Resources Folder to App Extension"),
                       "Disable this to prevent the Projucer from creating a duplicate resources folder for AUv3 app extensions.");

        if (iOS)
        {
            props.add (new ChoicePropertyComponent (iosDeviceFamilyValue, "Device Family",
                                                    { "iPhone", "iPad", "Universal" },
                                                    { "1",      "2",    "1,2" }),
                       "The device family to target.");

            {
                StringArray orientationStrings { "Portrait and Landscape", "Portrait", "Landscape" };
                Array<var> orientationValues   { "portraitlandscape",      "portrait", "landscape"};

                props.add (new ChoicePropertyComponent (iPhoneScreenOrientationValue, "iPhone Screen Orientation",
                                                        orientationStrings, orientationValues),
                           "The screen orientations that this app should support on iPhones.");

                props.add (new ChoicePropertyComponent (iPadScreenOrientationValue,   "iPad Screen Orientation",
                                                        orientationStrings, orientationValues),
                           "The screen orientations that this app should support on iPads.");
            }

            props.add (new ChoicePropertyComponent (uiFileSharingEnabledValue, "File Sharing Enabled"),
                       "Enable this to expose your app's files to iTunes.");

            props.add (new ChoicePropertyComponent (uiSupportsDocumentBrowserValue, "Support Document Browser"),
                       "Enable this to allow the user to access your app documents from a native file chooser.");

            props.add (new ChoicePropertyComponent (uiStatusBarHiddenValue, "Status Bar Hidden"),
                       "Enable this to disable the status bar in your app.");
        }
        else if (projectType.isGUIApplication())
        {
            props.add (new TextPropertyComponent (documentExtensionsValue, "Document File Extensions", 128, false),
                       "A comma-separated list of file extensions for documents that your app can open. "
                       "Using a leading '.' is optional, and the extensions are not case-sensitive.");
        }

        if (isOSX())
        {
            props.add (new ChoicePropertyComponent (appSandboxValue, "Use App Sandbox"),
                       "Enable this to use the app sandbox.");

            std::vector<std::pair<String, String>> sandboxOptions
            {
                { "Network: Incoming Connections (Server)",         "network.server" },
                { "Network: Outgoing Connections (Client)",         "network.client" },

                { "Hardware: Camera",                               "device.camera" },
                { "Hardware: Microphone",                           "device.microphone" },
                { "Hardware: USB",                                  "device.usb" },
                { "Hardware: Printing",                             "print" },
                { "Hardware: Bluetooth",                            "device.bluetooth" },

                { "App Data: Contacts",                             "personal-information.addressbook" },
                { "App Data: Location",                             "personal-information.location" },
                { "App Data: Calendar",                             "personal-information.calendars" },

                { "File Access: User Selected File (Read Only)",    "files.user-selected.read-only" },
                { "File Access: User Selected File (Read/Write)",   "files.user-selected.read-write" },
                { "File Access: Downloads Folder (Read Only)",      "files.downloads.read-only" },
                { "File Access: Downloads Folder (Read/Write)",     "files.downloads.read-write" },
                { "File Access: Pictures Folder (Read Only)",       "files.pictures.read-only" },
                { "File Access: Pictures Folder (Read/Write)",      "files.pictures.read-write" },
                { "File Access: Music Folder (Read Only)",          "assets.music.read-only" },
                { "File Access: Music Folder (Read/Write)",         "assets.music.read-write" },
                { "File Access: Movies Folder (Read Only)",         "assets.movies.read-only" },
                { "File Access: Movies Folder (Read/Write)",        "assets.movies.read-write" }
            };

            StringArray sandboxKeys;
            Array<var> sanboxValues;

            for (auto& opt : sandboxOptions)
            {
                sandboxKeys.add (opt.first);
                sanboxValues.add ("com.apple.security." + opt.second);
            }

            props.add (new MultiChoicePropertyComponentWithEnablement (appSandboxOptionsValue,
                                                                       appSandboxValue,
                                                                       "App Sandbox Options",
                                                                       sandboxKeys,
                                                                       sanboxValues));

            props.add (new ChoicePropertyComponent (hardenedRuntimeValue, "Use Hardened Runtime"),
                       "Enable this to use the hardened runtime required for app notarization.");

            std::vector<std::pair<String, String>> hardeningOptions
            {
                { "Runtime Exceptions: Allow Execution of JIT-compiled Code", "cs.allow-jit" },
                { "Runtime Exceptions: Allow Unsigned Executable Memory",     "cs.allow-unsigned-executable-memory" },
                { "Runtime Exceptions: Allow DYLD Environment Variables",     "cs.allow-dyld-environment-variables" },
                { "Runtime Exceptions: Disable Library Validation",           "cs.disable-library-validation" },
                { "Runtime Exceptions: Disable Executable Memory Protection", "cs.disable-executable-page-protection" },
                { "Runtime Exceptions: Debugging Tool",                       "cs.debugger" },

                { "Resource Access: Audio Input",                             "device.audio-input" },
                { "Resource Access: Camera",                                  "device.camera" },
                { "Resource Access: Location",                                "personal-information.location" },
                { "Resource Access: Address Book",                            "personal-information.addressbook" },
                { "Resource Access: Calendar",                                "personal-information.calendars" },
                { "Resource Access: Photos Library",                          "personal-information.photos-library" },
                { "Resource Access: Apple Events",                            "automation.apple-events" }
            };

            StringArray hardeningKeys;
            Array<var> hardeningValues;

            for (auto& opt : hardeningOptions)
            {
                hardeningKeys.add (opt.first);
                hardeningValues.add ("com.apple.security." + opt.second);
            }

            props.add (new MultiChoicePropertyComponentWithEnablement (hardenedRuntimeOptionsValue,
                                                                       hardenedRuntimeValue,
                                                                       "Hardened Runtime Options",
                                                                       hardeningKeys,
                                                                       hardeningValues));
        }

        props.add (new ChoicePropertyComponent (microphonePermissionNeededValue, "Microphone Access"),
                   "Enable this to allow your app to use the microphone. "
                   "The user of your app will be prompted to grant microphone access permissions.");

        props.add (new TextPropertyComponentWithEnablement (microphonePermissionsTextValue, microphonePermissionNeededValue,
                                                            "Microphone Access Text", 1024, false),
                   "A short description of why your app requires microphone access.");

        props.add (new ChoicePropertyComponent (cameraPermissionNeededValue, "Camera Access"),
                   "Enable this to allow your app to use the camera. "
                   "The user of your app will be prompted to grant camera access permissions.");

        props.add (new TextPropertyComponentWithEnablement (cameraPermissionTextValue, cameraPermissionNeededValue,
                                                            "Camera Access Text", 1024, false),
                   "A short description of why your app requires camera access.");

        if (iOS)
        {
            props.add (new ChoicePropertyComponent (iosBluetoothPermissionNeededValue, "Bluetooth Access"),
                       "Enable this to allow your app to use Bluetooth on iOS 13.0 and above. "
                       "The user of your app will be prompted to grant Bluetooth access permissions.");

            props.add (new TextPropertyComponentWithEnablement (iosBluetoothPermissionTextValue, iosBluetoothPermissionNeededValue,
                                                                "Bluetooth Access Text", 1024, false),
                       "A short description of why your app requires Bluetooth access.");
        }

        props.add (new ChoicePropertyComponent (iosInAppPurchasesValue, "In-App Purchases Capability"),
                   "Enable this to grant your app the capability for in-app purchases. "
                   "This option requires that you specify a valid Development Team ID.");

        if (iOS)
        {
            props.add (new ChoicePropertyComponent (iosBackgroundAudioValue, "Audio Background Capability"),
                       "Enable this to grant your app the capability to access audio when in background mode. "
                       "This permission is required if your app creates a MIDI input or output device.");

            props.add (new ChoicePropertyComponent (iosBackgroundBleValue, "Bluetooth MIDI Background Capability"),
                       "Enable this to grant your app the capability to connect to Bluetooth LE devices when in background mode.");

            props.add (new ChoicePropertyComponent (iosAppGroupsValue, "App Groups Capability"),
                       "Enable this to grant your app the capability to share resources between apps using the same app group ID.");

            props.add (new ChoicePropertyComponent (iCloudPermissionsValue, "iCloud Permissions"),
                       "Enable this to grant your app the capability to use native file load/save browser windows on iOS.");
        }

        props.add (new ChoicePropertyComponent (iosPushNotificationsValue, "Push Notifications Capability"),
                   "Enable this to grant your app the capability to receive push notifications.");

        props.add (new TextPropertyComponent (customPListValue, "Custom PList", 8192, true),
                   "You can paste the contents of an XML PList file in here, and the settings that it contains will override any "
                   "settings that the Projucer creates. BEWARE! When doing this, be careful to remove from the XML any "
                   "values that you DO want the Projucer to change!");

        props.add (new ChoicePropertyComponent (pListPreprocessValue, "PList Preprocess"),
                   "Enable this to preprocess PList file. This will allow you to set values to preprocessor defines,"
                   " for instance if you define: #define MY_FLAG 1 in a prefix header file (see PList prefix header), you can have"
                   " a key with MY_FLAG value and it will be replaced with 1.");

        props.add (new TextPropertyComponent (pListPrefixHeaderValue, "PList Prefix Header", 512, false),
                   "Header file containing definitions used in plist file (see PList Preprocess).");

        props.add (new TextPropertyComponent (extraFrameworksValue, "Extra System Frameworks", 2048, false),
                   "A comma-separated list of extra system frameworks that should be added to the build. "
                   "(Don't include the .framework extension in the name)"
                   " The frameworks are expected to be located in /System/Library/Frameworks");

        props.add (new TextPropertyComponent (frameworkSearchPathsValue, "Framework Search Paths", 8192, true),
                   "A set of paths to search for custom frameworks (one per line).");

        props.add (new TextPropertyComponent (extraCustomFrameworksValue, "Extra Custom Frameworks", 8192, true),
                   "Paths to custom frameworks that should be added to the build (one per line). "
                   "You will probably need to add an entry to the Framework Search Paths for each unique directory.");

        props.add (new TextPropertyComponent (embeddedFrameworksValue, "Embedded Frameworks", 8192, true),
                   "Paths to frameworks to be embedded with the app (one per line). "
                   "If you are adding a framework here then you do not need to specify it in Extra Custom Frameworks too. "
                   "You will probably need to add an entry to the Framework Search Paths for each unique directory.");

        props.add (new TextPropertyComponent (subprojectsValue, "Xcode Subprojects", 8192, true),
                   "Paths to Xcode projects that should be added to the build (one per line). "
                   "The names of the required build products can be specified after a colon, comma seperated, "
                   "e.g. \"path/to/MySubProject.xcodeproj: MySubProject, OtherTarget\". "
                   "If no build products are specified, all build products associated with a subproject will be added.");

        props.add (new TextPropertyComponent (prebuildCommandValue, "Pre-Build Shell Script", 32768, true),
                   "Some shell-script that will be run before a build starts.");

        props.add (new TextPropertyComponent (postbuildCommandValue, "Post-Build Shell Script", 32768, true),
                   "Some shell-script that will be run after a build completes.");

        props.add (new TextPropertyComponent (exporterBundleIdentifierValue, "Exporter Bundle Identifier", 256, false),
                   "Use this to override the project bundle identifier for this exporter. "
                   "This is useful if you want to use different bundle identifiers for Mac and iOS exporters in the same project.");

        props.add (new TextPropertyComponent (iosDevelopmentTeamIDValue, "Development Team ID", 10, false),
                   "The Development Team ID to be used for setting up code-signing your app. This is a ten-character "
                   "string (for example, \"S7B6T5XJ2Q\") that describes the distribution certificate Apple issued to you. "
                   "You can find this string in the OS X app Keychain Access under \"Certificates\".");

        if (iOS)
            props.add (new TextPropertyComponentWithEnablement (iosAppGroupsIDValue, iosAppGroupsValue, "App Group ID", 256, false),
                       "The App Group ID to be used for allowing multiple apps to access a shared resource folder. Multiple IDs can be "
                       "added separated by a semicolon.");

        props.add (new ChoicePropertyComponent (keepCustomXcodeSchemesValue, "Keep Custom Xcode Schemes"),
                   "Enable this to keep any Xcode schemes you have created for debugging or running, e.g. to launch a plug-in in"
                   "various hosts. If disabled, all schemes are replaced by a default set.");

        props.add (new ChoicePropertyComponent (useHeaderMapValue, "USE_HEADERMAP"),
                   "Enable this to make Xcode search all the projects folders for include files. This means you can be lazy "
                   "and not bother using relative paths to include your headers, but it means your code won't be "
                   "compatible with other build systems");
    }

    bool launchProject() override
    {
       #if JUCE_MAC
        return getProjectBundle().startAsProcess();
       #else
        return false;
       #endif
    }

    bool canLaunchProject() override
    {
       #if JUCE_MAC
        return true;
       #else
        return false;
       #endif
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        for (auto& target : targets)
            if (target->shouldCreatePList())
                target->infoPlistFile = getTargetFolder().getChildFile (target->getInfoPlistName());

        menuNibFile = getTargetFolder().getChildFile ("RecentFilesMenuTemplate.nib");

        createIconFile();

        auto projectBundle = getProjectBundle();
        createDirectoryOrThrow (projectBundle);

        createObjects();

        auto projectFile = projectBundle.getChildFile ("project.pbxproj");

        {
            MemoryOutputStream mo;
            writeProjectFile (mo);
            overwriteFileIfDifferentOrThrow (projectFile, mo);
        }

        writeInfoPlistFiles();

        // This forces the project to use the legacy build system to workaround Xcode 10 issues,
        // hopefully these will be fixed in the future and this can be removed...
        writeWorkspaceSettings();

        // Deleting the .rsrc files can be needed to force Xcode to update the version number.
        deleteRsrcFiles (getTargetFolder().getChildFile ("build"));
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        callForAllSupportedTargets ([this] (ProjectType::Target::Type targetType)
                                    {
                                        if (auto* target = new XcodeTarget (targetType, *this))
                                        {
                                            if (targetType == ProjectType::Target::AggregateTarget)
                                                targets.insert (0, target);
                                            else
                                                targets.add (target);
                                        }
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    void updateDeprecatedProjectSettingsInteractively() override
    {
        if (hasInvalidPostBuildScript())
        {
            String alertWindowText = iOS ? "Your Xcode (iOS) Exporter settings use an invalid post-build script. Click 'Update' to remove it."
                                         : "Your Xcode (OSX) Exporter settings use a pre-JUCE 4.2 post-build script to move the plug-in binaries to their plug-in install folders.\n\n"
                                           "Since JUCE 4.2, this is instead done using \"AU/VST/VST2/AAX/RTAS Binary Location\" in the Xcode (OS X) configuration settings.\n\n"
                                           "Click 'Update' to remove the script (otherwise your plug-in may not compile correctly).";

            if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                              "Project settings: " + project.getDocumentTitle(),
                                              alertWindowText, "Update", "Cancel", nullptr, nullptr))
                postbuildCommandValue.resetToDefault();
        }
    }

    bool hasInvalidPostBuildScript() const
    {
        // check whether the script is identical to the old one that the Introjucer used to auto-generate
        return (MD5 (getPostBuildScript().toUTF8()).toHexString() == "265ac212a7e734c5bbd6150e1eae18a1");
    }

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                        getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::osx), TargetOS::osx);

        vst3PathValueWrapper.init ({ settings, Ids::vst3Folder, nullptr },
                                   getAppSettings().getStoredPath (Ids::vst3Path, TargetOS::osx), TargetOS::osx);

        aaxPathValueWrapper.init ({ settings, Ids::aaxFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::aaxPath,  TargetOS::osx), TargetOS::osx);

        rtasPathValueWrapper.init ({ settings, Ids::rtasFolder, nullptr },
                                   getAppSettings().getStoredPath (Ids::rtasPath, TargetOS::osx), TargetOS::osx);
    }

protected:
    //==============================================================================
    class XcodeBuildConfiguration  : public BuildConfiguration
    {
    public:
        XcodeBuildConfiguration (Project& p, const ValueTree& t, const bool isIOS, const ProjectExporter& e)
            : BuildConfiguration (p, t, e),
              iOS (isIOS),
              osxSDKVersion                (config, Ids::osxSDK,                       getUndoManager()),
              osxDeploymentTarget          (config, Ids::osxCompatibility,             getUndoManager(), getSDKDisplayName (defaultDeploymentTarget)),
              iosDeploymentTarget          (config, Ids::iosCompatibility,             getUndoManager(), iOSDefaultVersion),
              osxArchitecture              (config, Ids::osxArchitecture,              getUndoManager(), osxArch_Default),
              customXcodeFlags             (config, Ids::customXcodeFlags,             getUndoManager()),
              plistPreprocessorDefinitions (config, Ids::plistPreprocessorDefinitions, getUndoManager()),
              codeSignIdentity             (config, Ids::codeSigningIdentity,          getUndoManager()),
              fastMathEnabled              (config, Ids::fastMath,                     getUndoManager()),
              stripLocalSymbolsEnabled     (config, Ids::stripLocalSymbols,            getUndoManager()),
              pluginBinaryCopyStepEnabled  (config, Ids::enablePluginBinaryCopyStep,   getUndoManager(), true),
              vstBinaryLocation            (config, Ids::vstBinaryLocation,            getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/VST/"),
              vst3BinaryLocation           (config, Ids::vst3BinaryLocation,           getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/VST3/"),
              auBinaryLocation             (config, Ids::auBinaryLocation,             getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/Components/"),
              rtasBinaryLocation           (config, Ids::rtasBinaryLocation,           getUndoManager(), "/Library/Application Support/Digidesign/Plug-Ins/"),
              aaxBinaryLocation            (config, Ids::aaxBinaryLocation,            getUndoManager(), "/Library/Application Support/Avid/Audio/Plug-Ins/"),
              unityPluginBinaryLocation    (config, Ids::unityPluginBinaryLocation,    getUndoManager())
        {
            updateOldPluginBinaryLocations();
            updateOldSDKDefaults();

            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        //==============================================================================
        void createConfigProperties (PropertyListBuilder& props) override
        {
            addXcodePluginInstallPathProperties (props);
            addRecommendedLLVMCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            if (iOS)
            {
                Array<var> iOSVersionVars;

                for (auto& s : iOSVersions)
                    iOSVersionVars.add (s);

                props.add (new ChoicePropertyComponent (iosDeploymentTarget, "iOS Deployment Target", iOSVersions, iOSVersionVars),
                           "The minimum version of iOS that the target binary will run on.");
            }
            else
            {
                props.add (new ChoicePropertyComponent (osxSDKVersion, "OSX Base SDK Version", getSDKChoiceList<StringArray> (oldestSDKVersion, true),
                                                                                               getSDKChoiceList<Array<var>>  (oldestSDKVersion, true)),
                           "The version of OSX to link against in the Xcode build. If \"Default\" is selected then the field will be left "
                           "empty and the Xcode default will be used.");

                props.add (new ChoicePropertyComponent (osxDeploymentTarget, "OSX Deployment Target", getSDKChoiceList<StringArray> (oldestDeploymentTarget, false),
                                                                                                      getSDKChoiceList<Array<var>>  (oldestDeploymentTarget, true)),
                           "The minimum version of OSX that the target binary will be compatible with.");

                props.add (new ChoicePropertyComponent (osxArchitecture, "OSX Architecture",
                                                        { "Native architecture of build machine", "Universal Binary (32-bit)", "Universal Binary (32/64-bit)", "64-bit Intel" },
                                                        { osxArch_Native,                          osxArch_32BitUniversal,      osxArch_64BitUniversal,         osxArch_64Bit }),
                           "The type of OSX binary that will be produced.");
            }

            props.add (new TextPropertyComponent (customXcodeFlags, "Custom Xcode Flags", 8192, true),
                       "A comma-separated list of custom Xcode setting flags which will be appended to the list of generated flags, "
                       "e.g. MACOSX_DEPLOYMENT_TARGET_i386 = 10.5, VALID_ARCHS = \"ppc i386 x86_64\"");

            props.add (new TextPropertyComponent (plistPreprocessorDefinitions, "PList Preprocessor Definitions", 2048, true),
                       "Preprocessor definitions used during PList preprocessing (see PList Preprocess).");

            props.add (new TextPropertyComponent (codeSignIdentity, "Code-Signing Identity", 1024, false),
                       "The name of a code-signing identity for Xcode to apply.");

            props.add (new ChoicePropertyComponent (fastMathEnabled, "Relax IEEE Compliance"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new ChoicePropertyComponent (stripLocalSymbolsEnabled, "Strip Local Symbols"),
                       "Enable this to strip any locally defined symbols resulting in a smaller binary size. Enabling this "
                       "will also remove any function names from crash logs. Must be disabled for static library projects.");
        }

        String getModuleLibraryArchName() const override
        {
            return "${CURRENT_ARCH}";
        }

        //==============================================================================
        String getOSXArchitectureString() const                 { return osxArchitecture.get(); }
        String getPListPreprocessorDefinitionsString() const    { return plistPreprocessorDefinitions.get(); }

        bool isFastMathEnabled() const                          { return fastMathEnabled.get(); }

        bool isStripLocalSymbolsEnabled() const                 { return stripLocalSymbolsEnabled.get(); }

        String getCustomXcodeFlagsString() const                { return customXcodeFlags.get(); }

        String getOSXSDKVersionString() const                   { return osxSDKVersion.get(); }
        String getOSXDeploymentTargetString() const             { return osxDeploymentTarget.get(); }

        String getCodeSignIdentityString() const                { return codeSignIdentity.get(); }

        String getiOSDeploymentTargetString() const             { return iosDeploymentTarget.get(); }

        bool isPluginBinaryCopyStepEnabled() const              { return pluginBinaryCopyStepEnabled.get(); }
        String getVSTBinaryLocationString() const               { return vstBinaryLocation.get(); }
        String getVST3BinaryLocationString() const              { return vst3BinaryLocation.get(); }
        String getAUBinaryLocationString() const                { return auBinaryLocation.get(); }
        String getRTASBinaryLocationString() const              { return rtasBinaryLocation.get();}
        String getAAXBinaryLocationString() const               { return aaxBinaryLocation.get();}
        String getUnityPluginBinaryLocationString() const       { return unityPluginBinaryLocation.get(); }

    private:
        //==============================================================================
        bool iOS;

        ValueWithDefault osxSDKVersion, osxDeploymentTarget, iosDeploymentTarget, osxArchitecture,
                         customXcodeFlags, plistPreprocessorDefinitions, codeSignIdentity,
                         fastMathEnabled, stripLocalSymbolsEnabled, pluginBinaryCopyStepEnabled,
                         vstBinaryLocation, vst3BinaryLocation, auBinaryLocation, rtasBinaryLocation,
                         aaxBinaryLocation, unityPluginBinaryLocation;

        //==============================================================================
        void addXcodePluginInstallPathProperties (PropertyListBuilder& props)
        {
            auto isBuildingAnyPlugins = (project.shouldBuildVST() || project.shouldBuildVST3() || project.shouldBuildAU()
                                         || project.shouldBuildRTAS() || project.shouldBuildAAX() || project.shouldBuildUnityPlugin());

            if (isBuildingAnyPlugins)
                props.add (new ChoicePropertyComponent (pluginBinaryCopyStepEnabled, "Enable Plugin Copy Step"),
                           "Enable this to copy plugin binaries to the specified folder after building.");

            if (project.shouldBuildVST3())
                props.add (new TextPropertyComponentWithEnablement (vst3BinaryLocation, pluginBinaryCopyStepEnabled, "VST3 Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled VST3 binary should be placed.");

            if (project.shouldBuildAU())
                props.add (new TextPropertyComponentWithEnablement (auBinaryLocation, pluginBinaryCopyStepEnabled, "AU Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled AU binary should be placed.");

            if (project.shouldBuildRTAS())
                props.add (new TextPropertyComponentWithEnablement (rtasBinaryLocation, pluginBinaryCopyStepEnabled, "RTAS Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled RTAS binary should be placed.");

            if (project.shouldBuildAAX())
                props.add (new TextPropertyComponentWithEnablement (aaxBinaryLocation, pluginBinaryCopyStepEnabled, "AAX Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled AAX binary should be placed.");

            if (project.shouldBuildUnityPlugin())
                props.add (new TextPropertyComponentWithEnablement (unityPluginBinaryLocation, pluginBinaryCopyStepEnabled, "Unity Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled Unity plugin binary and associated C# GUI script should be placed.");

            if (project.shouldBuildVST())
                props.add (new TextPropertyComponentWithEnablement (vstBinaryLocation, pluginBinaryCopyStepEnabled, "VST Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled legacy VST binary should be placed.");
        }

        void updateOldPluginBinaryLocations()
        {
            if (! config ["xcodeVstBinaryLocation"].isVoid())        vstBinaryLocation  = config ["xcodeVstBinaryLocation"];
            if (! config ["xcodeVst3BinaryLocation"].isVoid())       vst3BinaryLocation = config ["xcodeVst3BinaryLocation"];
            if (! config ["xcodeAudioUnitBinaryLocation"].isVoid())  auBinaryLocation   = config ["xcodeAudioUnitBinaryLocation"];
            if (! config ["xcodeRtasBinaryLocation"].isVoid())       rtasBinaryLocation = config ["xcodeRtasBinaryLocation"];
            if (! config ["xcodeAaxBinaryLocation"].isVoid())        aaxBinaryLocation  = config ["xcodeAaxBinaryLocation"];
        }

        void updateOldSDKDefaults()
        {
            if (iosDeploymentTarget.get() == "default")    iosDeploymentTarget.resetToDefault();
            if (osxArchitecture.get() == "default")        osxArchitecture.resetToDefault();
            if (osxSDKVersion.get() == "default")          osxSDKVersion.resetToDefault();
            if (osxDeploymentTarget.get() == "default")    osxDeploymentTarget.resetToDefault();
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return *new XcodeBuildConfiguration (project, v, iOS, *this);
    }

public:
    //==============================================================================
    /* The numbers for these enum values are defined by Xcode for the different
       possible destinations of a "copy files" post-build step.
    */
    enum XcodeCopyFilesDestinationIDs
    {
        kWrapperFolder          = 1,
        kExecutablesFolder      = 6,
        kResourcesFolder        = 7,
        kFrameworksFolder       = 10,
        kSharedFrameworksFolder = 11,
        kSharedSupportFolder    = 12,
        kPluginsFolder          = 13,
        kJavaResourcesFolder    = 15,
        kXPCServicesFolder      = 16
    };

    //==============================================================================
    struct XcodeTarget : ProjectType::Target
    {
        //==============================================================================
        XcodeTarget (ProjectType::Target::Type targetType, const XcodeProjectExporter& exporter)
            : ProjectType::Target (targetType),
              owner (exporter)
        {
            switch (type)
            {
                case GUIApp:
                    xcodePackageType = "APPL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case ConsoleApp:
                    xcodeFileType = "compiled.mach-o.executable";
                    xcodeBundleExtension = String();
                    xcodeProductType = "com.apple.product-type.tool";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case StaticLibrary:
                    xcodeFileType = "archive.ar";
                    xcodeBundleExtension = ".a";
                    xcodeProductType = "com.apple.product-type.library.static";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case DynamicLibrary:
                    xcodeFileType = "compiled.mach-o.dylib";
                    xcodeProductType = "com.apple.product-type.library.dynamic";
                    xcodeBundleExtension = ".dylib";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case VSTPlugIn:
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case VST3PlugIn:
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst3";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case AudioUnitPlugIn:
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".component";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    addExtraAudioUnitTargetSettings();
                    break;

                case StandalonePlugIn:
                    xcodePackageType = "APPL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case AudioUnitv3PlugIn:
                    xcodePackageType = "XPC!";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.app-extension";
                    xcodeBundleExtension = ".appex";
                    xcodeBundleIDSubPath = "AUv3";
                    xcodeProductType = "com.apple.product-type.app-extension";
                    xcodeCopyToProductInstallPathAfterBuild = false;

                    addExtraAudioUnitv3PlugInTargetSettings();
                    break;

                case AAXPlugIn:
                    xcodePackageType = "TDMw";
                    xcodeBundleSignature = "PTul";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".aaxplugin";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case RTASPlugIn:
                    xcodePackageType = "TDMw";
                    xcodeBundleSignature = "PTul";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".dpm";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case UnityPlugIn:
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".bundle";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case SharedCodeTarget:
                    xcodeFileType = "archive.ar";
                    xcodeBundleExtension = ".a";
                    xcodeProductType = "com.apple.product-type.library.static";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case AggregateTarget:
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                default:
                    // unknown target type!
                    jassertfalse;
                    break;
            }
        }

        String getXcodeSchemeName() const
        {
            return owner.projectName + " - " + getName();
        }

        String getID() const
        {
            return owner.createID (String ("__target") + getName());
        }

        String getInfoPlistName() const
        {
            return String ("Info-") + String (getName()).replace (" ", "_") + String (".plist");
        }

        String xcodePackageType, xcodeBundleSignature, xcodeBundleExtension;
        String xcodeProductType, xcodeFileType;
        String xcodeOtherRezFlags, xcodeBundleIDSubPath;
        bool xcodeCopyToProductInstallPathAfterBuild;
        StringArray xcodeFrameworks, xcodeLibs;
        Array<XmlElement> xcodeExtraPListEntries;

        StringArray frameworkIDs, buildPhaseIDs, configIDs, sourceIDs, rezFileIDs;
        StringArray frameworkNames;
        String dependencyID, mainBuildProductID;
        File infoPlistFile;

        struct SourceFileInfo
        {
            RelativePath path;
            bool shouldBeCompiled = false;
        };

        Array<SourceFileInfo> getSourceFilesInfo (const Project::Item& projectItem) const
        {
            Array<SourceFileInfo> result;

            auto targetType = (owner.getProject().getProjectType().isAudioPlugin() ? type : SharedCodeTarget);

            if (projectItem.isGroup())
            {
                for (int i = 0; i < projectItem.getNumChildren(); ++i)
                    result.addArray (getSourceFilesInfo (projectItem.getChild (i)));
            }
            else if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (owner)
                     && owner.getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType)
            {
                SourceFileInfo info;

                info.path = RelativePath (projectItem.getFile(), owner.getTargetFolder(), RelativePath::buildTargetFolder);

                jassert (info.path.getRoot() == RelativePath::buildTargetFolder);

                if (targetType == SharedCodeTarget || projectItem.shouldBeCompiled())
                    info.shouldBeCompiled = projectItem.shouldBeCompiled();

                result.add (info);
            }

            return result;
        }

        //==============================================================================
        void addMainBuildProduct() const
        {
            jassert (xcodeFileType.isNotEmpty());
            jassert (xcodeBundleExtension.isEmpty() || xcodeBundleExtension.startsWithChar ('.'));

            if (ProjectExporter::BuildConfiguration::Ptr config = owner.getConfiguration(0))
            {
                auto productName = owner.replacePreprocessorTokens (*config, config->getTargetBinaryNameString (type == UnityPlugIn));

                if (xcodeFileType == "archive.ar")
                    productName = getStaticLibbedFilename (productName);
                else
                    productName += xcodeBundleExtension;

                addBuildProduct (xcodeFileType, productName);
            }
        }

        //==============================================================================
        void addBuildProduct (const String& fileType, const String& binaryName) const
        {
            auto* v = new ValueTree (owner.createID (String ("__productFileID") + getName()));
            v->setProperty ("isa", "PBXFileReference", nullptr);
            v->setProperty ("explicitFileType", fileType, nullptr);
            v->setProperty ("includeInIndex", (int) 0, nullptr);
            v->setProperty ("path", sanitisePath (binaryName), nullptr);
            v->setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", nullptr);
            owner.pbxFileReferences.add (v);
        }

        //==============================================================================
        void addDependency()
        {
            jassert (dependencyID.isEmpty());

            dependencyID = owner.createID (String ("__dependency") + getName());
            auto* v = new ValueTree (dependencyID);

            v->setProperty ("isa", "PBXTargetDependency", nullptr);
            v->setProperty ("target", getID(), nullptr);

            owner.misc.add (v);
        }

        String getDependencyID() const
        {
            jassert (dependencyID.isNotEmpty());

            return dependencyID;
        }

        //==============================================================================
        void addTargetConfig (const String& configName, const StringArray& buildSettings)
        {
            auto configID = owner.createID (String ("targetconfigid_") + getName() + String ("_") + configName);

            auto* v = new ValueTree (configID);
            v->setProperty ("isa", "XCBuildConfiguration", nullptr);
            v->setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
            v->setProperty (Ids::name, configName, nullptr);

            configIDs.add (configID);
            owner.targetConfigs.add (v);
        }

        //==============================================================================
        String getTargetAttributes() const
        {
            auto attributes = getID() + " = { ";

            auto developmentTeamID = owner.getDevelopmentTeamIDString();

            if (developmentTeamID.isNotEmpty())
            {
                attributes << "DevelopmentTeam = " << developmentTeamID << "; ";
                attributes << "ProvisioningStyle = Automatic; ";
            }

            auto appGroupsEnabled      = (owner.iOS && owner.isAppGroupsEnabled()) ? 1 : 0;
            auto inAppPurchasesEnabled = owner.isInAppPurchasesEnabled() ? 1 : 0;
            auto interAppAudioEnabled  = (owner.iOS
                                          && type == Target::StandalonePlugIn
                                          && owner.getProject().shouldEnableIAA()) ? 1 : 0;

            auto pushNotificationsEnabled = owner.isPushNotificationsEnabled() ? 1 : 0;
            auto sandboxEnabled = ((type == Target::AudioUnitv3PlugIn) || owner.isAppSandboxEnabled()) ? 1 : 0;
            auto hardendedRuntimeEnabled = owner.isHardenedRuntimeEnabled() ? 1 : 0;

            attributes << "SystemCapabilities = {";
            attributes << "com.apple.ApplicationGroups.iOS = { enabled = " << appGroupsEnabled << "; }; ";
            attributes << "com.apple.InAppPurchase = { enabled = " << inAppPurchasesEnabled << "; }; ";
            attributes << "com.apple.InterAppAudio = { enabled = " << interAppAudioEnabled << "; }; ";
            attributes << "com.apple.Push = { enabled = " << pushNotificationsEnabled << "; }; ";
            attributes << "com.apple.Sandbox = { enabled = " << sandboxEnabled << "; }; ";
            attributes << "com.apple.HardenedRuntime = { enabled = " << hardendedRuntimeEnabled << "; }; ";

            if (owner.iOS && owner.isiCloudPermissionsEnabled())
                attributes << "com.apple.iCloud = { enabled = 1; }; ";

            attributes << "}; };";

            return attributes;
        }

        //==============================================================================
        ValueTree& addBuildPhase (const String& buildPhaseType, const StringArray& fileIds, const StringRef humanReadableName = StringRef())
        {
            auto buildPhaseName = buildPhaseType + "_" + getName() + "_" + (humanReadableName.isNotEmpty() ? String (humanReadableName) : String ("resbuildphase"));
            auto buildPhaseId (owner.createID (buildPhaseName));

            int n = 0;
            while (buildPhaseIDs.contains (buildPhaseId))
                buildPhaseId = owner.createID (buildPhaseName + String (++n));

            buildPhaseIDs.add (buildPhaseId);

            auto* v = new ValueTree (buildPhaseId);
            v->setProperty ("isa", buildPhaseType, nullptr);
            v->setProperty ("buildActionMask", "2147483647", nullptr);
            v->setProperty ("files", indentParenthesisedList (fileIds), nullptr);
            v->setProperty ("runOnlyForDeploymentPostprocessing", (int) 0, nullptr);

            if (humanReadableName.isNotEmpty())
                v->setProperty ("name", String (humanReadableName), nullptr);

            owner.misc.add (v);
            return *v;
        }

        bool shouldCreatePList() const
        {
            auto fileType = getTargetFileType();
            return (fileType == executable && type != ConsoleApp) || fileType == pluginBundle || fileType == macOSAppex;
        }

        //==============================================================================
        bool shouldAddEntitlements() const
        {
            if (owner.isPushNotificationsEnabled()
             || owner.isAppGroupsEnabled()
             || owner.isAppSandboxEnabled()
             || owner.isHardenedRuntimeEnabled()
             || (owner.isiOS() && owner.isiCloudPermissionsEnabled()))
                return true;

            if (owner.project.getProjectType().isAudioPlugin()
                && (   (owner.isOSX() && type == Target::AudioUnitv3PlugIn)
                    || (owner.isiOS() && type == Target::StandalonePlugIn && owner.getProject().shouldEnableIAA())))
                return true;

            return false;
        }

        String getBundleIdentifier() const
        {
            auto exporterBundleIdentifier = owner.exporterBundleIdentifierValue.get().toString();
            auto bundleIdentifier = exporterBundleIdentifier.isNotEmpty() ? exporterBundleIdentifier
                                                                          : owner.project.getBundleIdentifierString();

            if (xcodeBundleIDSubPath.isNotEmpty())
            {
                auto bundleIdSegments = StringArray::fromTokens (bundleIdentifier, ".", StringRef());

                jassert (bundleIdSegments.size() > 0);
                bundleIdentifier += String (".") + bundleIdSegments[bundleIdSegments.size() - 1] + xcodeBundleIDSubPath;
            }

            return bundleIdentifier;
        }

        //==============================================================================
        StringPairArray getTargetSettings (const XcodeBuildConfiguration& config) const
        {
            StringPairArray s;

            if (type == AggregateTarget && ! owner.isiOS())
            {
                // the aggregate target needs to have the deployment target set for
                // pre-/post-build scripts
                s.set ("MACOSX_DEPLOYMENT_TARGET", getOSXDeploymentTarget (config.getOSXDeploymentTargetString()));

                auto sdkRoot = getOSXSDKVersion (config.getOSXSDKVersionString());

                if (sdkRoot.isNotEmpty())
                    s.set ("SDKROOT", sdkRoot);

                return s;
            }

            s.set ("PRODUCT_NAME", owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString (type == UnityPlugIn)).quoted());
            s.set ("PRODUCT_BUNDLE_IDENTIFIER", getBundleIdentifier());

            auto arch = (! owner.isiOS() && type == Target::AudioUnitv3PlugIn) ? osxArch_64Bit
                                                                               : config.getOSXArchitectureString();

            if      (arch == osxArch_Native)           s.set ("ARCHS", "\"$(NATIVE_ARCH_ACTUAL)\"");
            else if (arch == osxArch_32BitUniversal)   s.set ("ARCHS", "\"$(ARCHS_STANDARD_32_BIT)\"");
            else if (arch == osxArch_64BitUniversal)   s.set ("ARCHS", "\"$(ARCHS_STANDARD_32_64_BIT)\"");
            else if (arch == osxArch_64Bit)            s.set ("ARCHS", "\"$(ARCHS_STANDARD_64_BIT)\"");

            StringArray headerPaths (getHeaderSearchPaths (config));
            headerPaths.add ("\"$(inherited)\"");
            s.set ("HEADER_SEARCH_PATHS", indentParenthesisedList (headerPaths, 1));
            s.set ("USE_HEADERMAP", String (static_cast<bool> (config.exporter.settings.getProperty ("useHeaderMap")) ? "YES" : "NO"));

            auto frameworkSearchPaths = getFrameworkSearchPaths (config);

            if (! frameworkSearchPaths.isEmpty())
                s.set ("FRAMEWORK_SEARCH_PATHS", String ("(") + frameworkSearchPaths.joinIntoString (", ") + ", \"$(inherited)\")");

            s.set ("GCC_OPTIMIZATION_LEVEL", config.getGCCOptimisationFlag());

            if (shouldCreatePList())
            {
                s.set ("INFOPLIST_FILE", infoPlistFile.getFileName());

                if (owner.getPListPrefixHeaderString().isNotEmpty())
                    s.set ("INFOPLIST_PREFIX_HEADER", owner.getPListPrefixHeaderString());

                s.set ("INFOPLIST_PREPROCESS", (owner.isPListPreprocessEnabled() ? String ("YES") : String ("NO")));

                auto plistDefs = parsePreprocessorDefs (config.getPListPreprocessorDefinitionsString());
                StringArray defsList;

                for (int i = 0; i < plistDefs.size(); ++i)
                {
                    auto def = plistDefs.getAllKeys()[i];
                    auto value = plistDefs.getAllValues()[i];

                    if (value.isNotEmpty())
                        def << "=" << value.replace ("\"", "\\\\\\\"");

                    defsList.add ("\"" + def + "\"");
                }

                if (defsList.size() > 0)
                    s.set ("INFOPLIST_PREPROCESSOR_DEFINITIONS", indentParenthesisedList (defsList, 1));
            }

            if (config.isLinkTimeOptimisationEnabled())
                s.set ("LLVM_LTO", "YES");

            if (config.isFastMathEnabled())
                s.set ("GCC_FAST_MATH", "YES");


            auto flags = (config.getRecommendedCompilerWarningFlags().joinIntoString (" ")
                             + " " + owner.getExtraCompilerFlagsString()).trim();
            flags = owner.replacePreprocessorTokens (config, flags);

            if (flags.isNotEmpty())
                s.set ("OTHER_CPLUSPLUSFLAGS", flags.quoted());

            auto installPath = getInstallPathForConfiguration (config);

            if (installPath.startsWith ("~"))
                installPath = installPath.replace ("~", "$(HOME)");

            if (installPath.isNotEmpty())
            {
                s.set ("INSTALL_PATH", installPath.quoted());

                if (type == Target::SharedCodeTarget)
                    s.set ("SKIP_INSTALL", "YES");

                if (! owner.embeddedFrameworkIDs.isEmpty())
                    s.set ("LD_RUNPATH_SEARCH_PATHS", "\"$(inherited) @executable_path/Frameworks @executable_path/../Frameworks\"");

                if (xcodeCopyToProductInstallPathAfterBuild)
                {
                    s.set ("DEPLOYMENT_LOCATION", "YES");
                    s.set ("DSTROOT", "/");
                }
            }

            if (getTargetFileType() == pluginBundle)
            {
                s.set ("LIBRARY_STYLE", "Bundle");
                s.set ("WRAPPER_EXTENSION", xcodeBundleExtension.substring (1));
                s.set ("GENERATE_PKGINFO_FILE", "YES");
            }

            if (xcodeOtherRezFlags.isNotEmpty())
                s.set ("OTHER_REZFLAGS", "\"" + xcodeOtherRezFlags + "\"");

            String configurationBuildDir ("$(PROJECT_DIR)/build/$(CONFIGURATION)");

            if (config.getTargetBinaryRelativePathString().isNotEmpty())
            {
                // a target's position can either be defined via installPath + xcodeCopyToProductInstallPathAfterBuild
                // (= for audio plug-ins) or using a custom binary path (for everything else), but not both (= conflict!)
                jassert (! xcodeCopyToProductInstallPathAfterBuild);

                RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
                configurationBuildDir = sanitisePath (binaryPath.rebased (owner.projectFolder, owner.getTargetFolder(), RelativePath::buildTargetFolder)
                                                                .toUnixStyle());
            }

            s.set ("CONFIGURATION_BUILD_DIR", addQuotesIfRequired (configurationBuildDir));

            if (owner.isHardenedRuntimeEnabled())
                s.set ("ENABLE_HARDENED_RUNTIME", "YES");

            String gccVersion ("com.apple.compilers.llvm.clang.1_0");

            if (owner.iOS)
            {
                s.set ("ASSETCATALOG_COMPILER_APPICON_NAME", "AppIcon");

                if (! owner.shouldAddStoryboardToProject())
                    s.set ("ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME", "LaunchImage");
            }
            else
            {
                s.set ("MACOSX_DEPLOYMENT_TARGET", getOSXDeploymentTarget (config.getOSXDeploymentTargetString()));

                auto sdkRoot = getOSXSDKVersion (config.getOSXSDKVersionString());

                if (sdkRoot.isNotEmpty())
                    s.set ("SDKROOT", sdkRoot);
            }

            s.set ("GCC_VERSION", gccVersion);
            s.set ("CLANG_LINK_OBJC_RUNTIME", "NO");

            auto codeSigningIdentity = owner.getCodeSigningIdentity (config);
            s.set (owner.iOS ? "\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\"" : "CODE_SIGN_IDENTITY",
                   codeSigningIdentity.quoted());

            if (codeSigningIdentity.isNotEmpty())
                s.set ("PROVISIONING_PROFILE_SPECIFIER", "\"\"");

            if (owner.getDevelopmentTeamIDString().isNotEmpty())
                s.set ("DEVELOPMENT_TEAM", owner.getDevelopmentTeamIDString());

            if (shouldAddEntitlements())
                s.set ("CODE_SIGN_ENTITLEMENTS", owner.getEntitlementsFileName().quoted());

            {
                auto cppStandard = owner.project.getCppStandardString();

                if (cppStandard == "latest")
                    cppStandard = "17";

                s.set ("CLANG_CXX_LANGUAGE_STANDARD", (String (owner.shouldUseGNUExtensions() ? "gnu++"
                                                                                              : "c++") + cppStandard).quoted());
            }

            s.set ("CLANG_CXX_LIBRARY", "\"libc++\"");

            s.set ("COMBINE_HIDPI_IMAGES", "YES");

            {
                StringArray linkerFlags, librarySearchPaths;
                getLinkerSettings (config, linkerFlags, librarySearchPaths);

                if (linkerFlags.size() > 0)
                    s.set ("OTHER_LDFLAGS", linkerFlags.joinIntoString (" ").quoted());

                librarySearchPaths.addArray (config.getLibrarySearchPaths());
                librarySearchPaths = getCleanedStringArray (librarySearchPaths);

                if (librarySearchPaths.size() > 0)
                {
                    StringArray libPaths;
                    libPaths.add ("\"$(inherited)\"");

                    for (auto& p : librarySearchPaths)
                        libPaths.add ("\"\\\"" + p + "\\\"\"");

                    s.set ("LIBRARY_SEARCH_PATHS", indentParenthesisedList (libPaths, 1));

                }
            }

            StringPairArray defines;

            if (config.isDebug())
            {
                defines.set ("_DEBUG", "1");
                defines.set ("DEBUG", "1");
                s.set ("COPY_PHASE_STRIP", "NO");
                s.set ("GCC_DYNAMIC_NO_PIC", "NO");
            }
            else
            {
                defines.set ("_NDEBUG", "1");
                defines.set ("NDEBUG", "1");
                s.set ("GCC_GENERATE_DEBUGGING_SYMBOLS", "NO");
                s.set ("GCC_SYMBOLS_PRIVATE_EXTERN", "YES");
                s.set ("DEAD_CODE_STRIPPING", "YES");
            }

            if (type != Target::SharedCodeTarget && type != Target::StaticLibrary && type != Target::DynamicLibrary
                  && config.isStripLocalSymbolsEnabled())
            {
                s.set ("STRIPFLAGS", "\"-x\"");
                s.set ("DEPLOYMENT_POSTPROCESSING", "YES");
                s.set ("SEPARATE_STRIP", "YES");
            }

            if (owner.isInAppPurchasesEnabled())
                defines.set ("JUCE_IN_APP_PURCHASES", "1");

            if (owner.isPushNotificationsEnabled())
                defines.set ("JUCE_PUSH_NOTIFICATIONS", "1");

            defines = mergePreprocessorDefs (defines, owner.getAllPreprocessorDefs (config, type));

            StringArray defsList;

            for (int i = 0; i < defines.size(); ++i)
            {
                auto def = defines.getAllKeys()[i];
                auto value = defines.getAllValues()[i];
                if (value.isNotEmpty())
                    def << "=" << value.replace ("\"", "\\\\\\\"");

                defsList.add ("\"" + def + "\"");
            }

            s.set ("GCC_PREPROCESSOR_DEFINITIONS", indentParenthesisedList (defsList, 1));

            StringArray customFlags;
            customFlags.addTokens (config.getCustomXcodeFlagsString(), ",", "\"'");
            customFlags.removeEmptyStrings();

            for (auto flag : customFlags)
            {
                s.set (flag.upToFirstOccurrenceOf ("=", false, false).trim(),
                       flag.fromFirstOccurrenceOf ("=", false, false).trim().quoted());
            }

            return s;
        }

        String getInstallPathForConfiguration (const XcodeBuildConfiguration& config) const
        {
            switch (type)
            {
                case GUIApp:            return "$(HOME)/Applications";
                case ConsoleApp:        return "/usr/bin";
                case VSTPlugIn:         return config.isPluginBinaryCopyStepEnabled() ? config.getVSTBinaryLocationString() : String();
                case VST3PlugIn:        return config.isPluginBinaryCopyStepEnabled() ? config.getVST3BinaryLocationString() : String();
                case AudioUnitPlugIn:   return config.isPluginBinaryCopyStepEnabled() ? config.getAUBinaryLocationString() : String();
                case RTASPlugIn:        return config.isPluginBinaryCopyStepEnabled() ? config.getRTASBinaryLocationString() : String();
                case AAXPlugIn:         return config.isPluginBinaryCopyStepEnabled() ? config.getAAXBinaryLocationString() : String();
                case UnityPlugIn:       return config.isPluginBinaryCopyStepEnabled() ? config.getUnityPluginBinaryLocationString() : String();
                case SharedCodeTarget:  return owner.isiOS() ? "@executable_path/Frameworks" : "@executable_path/../Frameworks";
                default:                return {};
            }
        }

        //==============================================================================
        void getLinkerSettings (const BuildConfiguration& config, StringArray& flags, StringArray& librarySearchPaths) const
        {
            if (getTargetFileType() == pluginBundle)
                flags.add (owner.isiOS() ? "-bitcode_bundle" : "-bundle");

            if (type != Target::SharedCodeTarget)
            {
                Array<RelativePath> extraLibs;

                addExtraLibsForTargetType (config, extraLibs);

                for (auto& lib : extraLibs)
                {
                    flags.add (getLinkerFlagForLib (lib.getFileNameWithoutExtension()));
                    librarySearchPaths.add (owner.getSearchPathForStaticLibrary (lib));
                }

                if (owner.project.getProjectType().isAudioPlugin())
                {
                    if (owner.getTargetOfType (Target::SharedCodeTarget) != nullptr)
                    {
                        auto productName = getStaticLibbedFilename (owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

                        RelativePath sharedCodelib (productName, RelativePath::buildTargetFolder);
                        flags.add (getLinkerFlagForLib (sharedCodelib.getFileNameWithoutExtension()));
                    }
                }

                flags.add (owner.getExternalLibraryFlags (config));

                auto libs = owner.xcodeLibs;
                libs.addArray (xcodeLibs);

                for (auto& l : libs)
                    flags.add (getLinkerFlagForLib (l));
            }

            flags.add (owner.replacePreprocessorTokens (config, owner.getExtraLinkerFlagsString()));
            flags = getCleanedStringArray (flags);
        }

        //========================================================================== c
        void writeInfoPlistFile() const
        {
            if (! shouldCreatePList())
                return;

            auto plist = parseXML (owner.getPListToMergeString());

            if (plist == nullptr || ! plist->hasTagName ("plist"))
                plist.reset (new XmlElement ("plist"));

            auto* dict = plist->getChildByName ("dict");

            if (dict == nullptr)
                dict = plist->createNewChildElement ("dict");

            if (owner.isMicrophonePermissionEnabled())
                addPlistDictionaryKey (dict, "NSMicrophoneUsageDescription", owner.getMicrophonePermissionsTextString());

            if (owner.isCameraPermissionEnabled())
                addPlistDictionaryKey (dict, "NSCameraUsageDescription", owner.getCameraPermissionTextString());

            if (owner.iOS)
            {
                if (owner.isBluetoothPermissionEnabled())
                {
                    addPlistDictionaryKey (dict, "NSBluetoothAlwaysUsageDescription", owner.getBluetoothPermissionTextString());
                    addPlistDictionaryKey (dict, "NSBluetoothPeripheralUsageDescription", owner.getBluetoothPermissionTextString()); // needed for pre iOS 13.0
                }

                addPlistDictionaryKeyBool (dict, "LSRequiresIPhoneOS", true);

                if (type != AudioUnitv3PlugIn)
                    addPlistDictionaryKeyBool (dict, "UIViewControllerBasedStatusBarAppearance", false);

                if (owner.shouldAddStoryboardToProject())
                {
                    auto customStoryboard = owner.getCustomLaunchStoryboardString();

                    addPlistDictionaryKey (dict, "UILaunchStoryboardName", customStoryboard.isNotEmpty() ? customStoryboard.fromLastOccurrenceOf ("/", false, false)
                                                                                                                           .upToLastOccurrenceOf (".storyboard", false, false)
                                                                                                         : owner.getDefaultLaunchStoryboardName());
                }
            }

            addPlistDictionaryKey (dict, "CFBundleExecutable",          "${EXECUTABLE_NAME}");

            if (! owner.iOS) // (NB: on iOS this causes error ITMS-90032 during publishing)
                addPlistDictionaryKey (dict, "CFBundleIconFile", owner.iconFile.exists() ? owner.iconFile.getFileName() : String());

            addPlistDictionaryKey (dict, "CFBundleIdentifier",          getBundleIdentifier());
            addPlistDictionaryKey (dict, "CFBundleName",                owner.projectName);

            // needed by NSExtension on iOS
            addPlistDictionaryKey (dict, "CFBundleDisplayName",         owner.projectName);
            addPlistDictionaryKey (dict, "CFBundlePackageType",         xcodePackageType);
            addPlistDictionaryKey (dict, "CFBundleSignature",           xcodeBundleSignature);
            addPlistDictionaryKey (dict, "CFBundleShortVersionString",  owner.project.getVersionString());
            addPlistDictionaryKey (dict, "CFBundleVersion",             owner.project.getVersionString());
            addPlistDictionaryKey (dict, "NSHumanReadableCopyright",    owner.project.getCompanyCopyrightString());
            addPlistDictionaryKeyBool (dict, "NSHighResolutionCapable", true);

            auto documentExtensions = StringArray::fromTokens (replacePreprocessorDefs (owner.getAllPreprocessorDefs(),
                                                                                        owner.getDocumentExtensionsString()), ",", {});
            documentExtensions.trim();
            documentExtensions.removeEmptyStrings (true);

            if (documentExtensions.size() > 0 && type != AudioUnitv3PlugIn)
            {
                dict->createNewChildElement ("key")->addTextElement ("CFBundleDocumentTypes");
                auto* dict2 = dict->createNewChildElement ("array")->createNewChildElement ("dict");
                XmlElement* arrayTag = nullptr;

                for (auto ex : documentExtensions)
                {
                    if (ex.startsWithChar ('.'))
                        ex = ex.substring (1);

                    if (arrayTag == nullptr)
                    {
                        dict2->createNewChildElement ("key")->addTextElement ("CFBundleTypeExtensions");
                        arrayTag = dict2->createNewChildElement ("array");

                        addPlistDictionaryKey (dict2, "CFBundleTypeName", ex);
                        addPlistDictionaryKey (dict2, "CFBundleTypeRole", "Editor");
                        addPlistDictionaryKey (dict2, "CFBundleTypeIconFile", "Icon");
                        addPlistDictionaryKey (dict2, "NSPersistentStoreTypeKey", "XML");
                    }

                    arrayTag->createNewChildElement ("string")->addTextElement (ex);
                }
            }

            if (owner.isFileSharingEnabled() && type != AudioUnitv3PlugIn)
                addPlistDictionaryKeyBool (dict, "UIFileSharingEnabled", true);

            if (owner.isDocumentBrowserEnabled())
                addPlistDictionaryKeyBool (dict, "UISupportsDocumentBrowser", true);

            if (owner.isStatusBarHidden() && type != AudioUnitv3PlugIn)
                addPlistDictionaryKeyBool (dict, "UIStatusBarHidden", true);

            if (owner.iOS)
            {
                if (type != AudioUnitv3PlugIn)
                {
                    // Forcing full screen disables the split screen feature and prevents error ITMS-90475
                    addPlistDictionaryKeyBool (dict, "UIRequiresFullScreen", true);
                    addPlistDictionaryKeyBool (dict, "UIStatusBarHidden", true);

                    addIosScreenOrientations (dict);
                    addIosBackgroundModes (dict);
                }

                if (type == StandalonePlugIn && owner.getProject().shouldEnableIAA())
                {
                    XmlElement audioComponentsPlistKey ("key");
                    audioComponentsPlistKey.addTextElement ("AudioComponents");

                    dict->addChildElement (new XmlElement (audioComponentsPlistKey));

                    XmlElement audioComponentsPlistEntry ("array");
                    auto* audioComponentsDict = audioComponentsPlistEntry.createNewChildElement ("dict");

                    addPlistDictionaryKey    (audioComponentsDict, "name",         owner.project.getIAAPluginName());
                    addPlistDictionaryKey    (audioComponentsDict, "manufacturer", owner.project.getPluginManufacturerCodeString().substring (0, 4));
                    addPlistDictionaryKey    (audioComponentsDict, "type",         owner.project.getIAATypeCode());
                    addPlistDictionaryKey    (audioComponentsDict, "subtype",      owner.project.getPluginCodeString().substring (0, 4));
                    addPlistDictionaryKeyInt (audioComponentsDict, "version",      owner.project.getVersionAsHexInteger());

                    dict->addChildElement (new XmlElement (audioComponentsPlistEntry));
                }
            }

            for (auto& e : xcodeExtraPListEntries)
                dict->addChildElement (new XmlElement (e));

            MemoryOutputStream mo;
            XmlElement::TextFormat format;
            format.dtd = "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">";
            plist->writeTo (mo, format);
            overwriteFileIfDifferentOrThrow (infoPlistFile, mo);
        }

        //==============================================================================
        void addIosScreenOrientations (XmlElement* dict) const
        {
            String screenOrientations[2] = { owner.getiPhoneScreenOrientationString(), owner.getiPadScreenOrientationString() };
            String plistSuffix[2]        = { "", "~ipad" };
            auto orientationsAreTheSame  = ( screenOrientations[0] == screenOrientations[1] );

            for (int i = 0; i < (orientationsAreTheSame ? 1 : 2); ++i)
            {
                StringArray iOSOrientations;

                if (screenOrientations[i].contains ("portrait"))   { iOSOrientations.add ("UIInterfaceOrientationPortrait"); }
                if (screenOrientations[i].contains ("landscape"))  { iOSOrientations.add ("UIInterfaceOrientationLandscapeLeft");  iOSOrientations.add ("UIInterfaceOrientationLandscapeRight"); }

                addArrayToPlist (dict, String ("UISupportedInterfaceOrientations") + plistSuffix[i], iOSOrientations);
            }

        }

        //==============================================================================
        void addIosBackgroundModes (XmlElement* dict) const
        {
            StringArray iosBackgroundModes;
            if (owner.isBackgroundAudioEnabled())     iosBackgroundModes.add ("audio");
            if (owner.isBackgroundBleEnabled())       iosBackgroundModes.add ("bluetooth-central");
            if (owner.isPushNotificationsEnabled())   iosBackgroundModes.add ("remote-notification");

            addArrayToPlist (dict, "UIBackgroundModes", iosBackgroundModes);
        }

        //==============================================================================
        static void addArrayToPlist (XmlElement* dict, String arrayKey, const StringArray& arrayElements)
        {
            dict->createNewChildElement ("key")->addTextElement (arrayKey);
            auto* plistStringArray = dict->createNewChildElement ("array");

            for (auto& e : arrayElements)
                plistStringArray->createNewChildElement ("string")->addTextElement (e);
        }

        //==============================================================================
        void addShellScriptBuildPhase (const String& phaseName, const String& script)
        {
            if (script.trim().isNotEmpty())
            {
                auto& v = addBuildPhase ("PBXShellScriptBuildPhase", {});
                v.setProperty (Ids::name, phaseName, nullptr);
                v.setProperty ("shellPath", "/bin/sh", nullptr);
                v.setProperty ("shellScript", script.replace ("\\", "\\\\")
                                                    .replace ("\"", "\\\"")
                                                    .replace ("\r\n", "\\n")
                                                    .replace ("\n", "\\n"), nullptr);
            }
        }

        void addCopyFilesPhase (const String& phaseName, const StringArray& files, XcodeCopyFilesDestinationIDs dst)
        {
            auto& v = addBuildPhase ("PBXCopyFilesBuildPhase", files, phaseName);
            v.setProperty ("dstPath", "", nullptr);
            v.setProperty ("dstSubfolderSpec", (int) dst, nullptr);
        }

        //==============================================================================
        void sanitiseAndEscapeSearchPaths (const BuildConfiguration& config, StringArray& paths) const
        {
            paths = getCleanedStringArray (paths);

            for (auto& path : paths)
            {
                // Xcode 10 can't deal with search paths starting with "~" so we need to replace them here...
                path = owner.replacePreprocessorTokens (config, sanitisePath (path));

                if (path.containsChar (' '))
                    path = "\"\\\"" + path + "\\\"\""; // crazy double quotes required when there are spaces..
                else
                    path = "\"" + path + "\"";
            }
        }

        StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
        {
            StringArray paths (owner.extraSearchPaths);
            paths.addArray (config.getHeaderSearchPaths());
            paths.addArray (getTargetExtraHeaderSearchPaths());

            if (owner.project.getEnabledModules().isModuleEnabled ("juce_audio_plugin_client"))
            {
                // Needed to compile .r files
                paths.add (owner.getModuleFolderRelativeToProject ("juce_audio_plugin_client")
                                .rebased (owner.projectFolder, owner.getTargetFolder(), RelativePath::buildTargetFolder)
                                .toUnixStyle());
            }

            sanitiseAndEscapeSearchPaths (config, paths);
            return paths;
        }

        StringArray getFrameworkSearchPaths (const BuildConfiguration& config) const
        {
            auto paths = getSearchPathsFromString (owner.getFrameworkSearchPathsString());
            sanitiseAndEscapeSearchPaths (config, paths);
            return paths;
        }

    private:
        //==============================================================================
        void addExtraAudioUnitTargetSettings()
        {
            xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                                 " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                 " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\""
                                 " -I \\\"$(DEVELOPER_DIR)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AudioUnit.framework/Headers\\\"";

            xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

            XmlElement plistKey ("key");
            plistKey.addTextElement ("AudioComponents");

            XmlElement plistEntry ("array");
            auto* dict = plistEntry.createNewChildElement ("dict");

            auto pluginManufacturerCode = owner.project.getPluginManufacturerCodeString().substring (0, 4);
            auto pluginSubType          = owner.project.getPluginCodeString().substring (0, 4);

            if (pluginManufacturerCode.toLowerCase() == pluginManufacturerCode)
            {
                throw SaveError ("AudioUnit plugin code identifiers invalid!\n\n"
                                 "You have used only lower case letters in your AU plugin manufacturer identifier. "
                                 "You must have at least one uppercase letter in your AU plugin manufacturer "
                                 "identifier code.");
            }

            addPlistDictionaryKey (dict, "name", owner.project.getPluginManufacturerString()
                                                   + ": " + owner.project.getPluginNameString());
            addPlistDictionaryKey (dict, "description", owner.project.getPluginDescriptionString());
            addPlistDictionaryKey (dict, "factoryFunction", owner.project.getPluginAUExportPrefixString() + "Factory");
            addPlistDictionaryKey (dict, "manufacturer", pluginManufacturerCode);
            addPlistDictionaryKey (dict, "type", owner.project.getAUMainTypeString().removeCharacters ("'"));
            addPlistDictionaryKey (dict, "subtype", pluginSubType);
            addPlistDictionaryKeyInt (dict, "version", owner.project.getVersionAsHexInteger());

            if (owner.project.isAUSandBoxSafe())
            {
                addPlistDictionaryKeyBool (dict, "sandboxSafe", true);
            }
            else
            {
                dict->createNewChildElement ("key")->addTextElement ("resourceUsage");
                auto* resourceUsageDict = dict->createNewChildElement ("dict");

                addPlistDictionaryKeyBool (resourceUsageDict, "network.client", true);
                addPlistDictionaryKeyBool (resourceUsageDict, "temporary-exception.files.all.read-write", true);
            }

            xcodeExtraPListEntries.add (plistKey);
            xcodeExtraPListEntries.add (plistEntry);
        }

        void addExtraAudioUnitv3PlugInTargetSettings()
        {
            if (owner.isiOS())
                xcodeFrameworks.addTokens ("CoreAudioKit AVFoundation", false);
            else
                xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit AVFoundation", false);

            XmlElement plistKey ("key");
            plistKey.addTextElement ("NSExtension");

            XmlElement plistEntry ("dict");

            addPlistDictionaryKey (&plistEntry, "NSExtensionPrincipalClass", owner.project.getPluginAUExportPrefixString() + "FactoryAUv3");
            addPlistDictionaryKey (&plistEntry, "NSExtensionPointIdentifier", "com.apple.AudioUnit-UI");
            plistEntry.createNewChildElement ("key")->addTextElement ("NSExtensionAttributes");

            auto* dict = plistEntry.createNewChildElement ("dict");
            dict->createNewChildElement ("key")->addTextElement ("AudioComponents");
            auto* componentArray = dict->createNewChildElement ("array");

            auto* componentDict = componentArray->createNewChildElement ("dict");

            addPlistDictionaryKey (componentDict, "name", owner.project.getPluginManufacturerString()
                                                            + ": " + owner.project.getPluginNameString());
            addPlistDictionaryKey (componentDict, "description", owner.project.getPluginDescriptionString());
            addPlistDictionaryKey (componentDict, "factoryFunction",owner.project. getPluginAUExportPrefixString() + "FactoryAUv3");
            addPlistDictionaryKey (componentDict, "manufacturer", owner.project.getPluginManufacturerCodeString().substring (0, 4));
            addPlistDictionaryKey (componentDict, "type", owner.project.getAUMainTypeString().removeCharacters ("'"));
            addPlistDictionaryKey (componentDict, "subtype", owner.project.getPluginCodeString().substring (0, 4));
            addPlistDictionaryKeyInt (componentDict, "version", owner.project.getVersionAsHexInteger());
            addPlistDictionaryKeyBool (componentDict, "sandboxSafe", true);

            componentDict->createNewChildElement ("key")->addTextElement ("tags");
            auto* tagsArray = componentDict->createNewChildElement ("array");

            tagsArray->createNewChildElement ("string")
                ->addTextElement (static_cast<bool> (owner.project.isPluginSynth()) ? "Synth" : "Effects");

            xcodeExtraPListEntries.add (plistKey);
            xcodeExtraPListEntries.add (plistEntry);
        }

        void addExtraLibsForTargetType  (const BuildConfiguration& config, Array<RelativePath>& extraLibs) const
        {
            if (type == AAXPlugIn)
            {
                auto aaxLibsFolder = RelativePath (owner.getAAXPathString(), RelativePath::projectFolder).getChildFile ("Libs");

                String libraryPath (config.isDebug() ? "Debug" : "Release");
                libraryPath += "/libAAXLibrary_libcpp.a";

                extraLibs.add   (aaxLibsFolder.getChildFile (libraryPath));
            }
            else if (type == RTASPlugIn)
            {
                RelativePath rtasFolder (owner.getRTASPathString(), RelativePath::projectFolder);

                extraLibs.add (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
                extraLibs.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));
            }
        }

        StringArray getTargetExtraHeaderSearchPaths() const
        {
            StringArray targetExtraSearchPaths;

            if (type == RTASPlugIn)
            {
                RelativePath rtasFolder (owner.getRTASPathString(), RelativePath::projectFolder);

                targetExtraSearchPaths.add ("$(DEVELOPER_DIR)/Headers/FlatCarbon");
                targetExtraSearchPaths.add ("$(SDKROOT)/Developer/Headers/FlatCarbon");

                static const char* p[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/CoreClasses",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/DSPClasses",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/MacBuild",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/Meters",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses/Interfaces",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/RTASP_Adapt",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/Utilities",
                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ViewClasses",
                    "AlturaPorts/TDMPlugIns/DSPManager/**",
                    "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/Encryption",
                    "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/GraphicsExtensions",
                    "AlturaPorts/TDMPlugIns/common/**",
                    "AlturaPorts/TDMPlugIns/common/PI_LibInterface",
                    "AlturaPorts/TDMPlugIns/PACEProtection/**",
                    "AlturaPorts/TDMPlugIns/SignalProcessing/**",
                    "AlturaPorts/OMS/Headers",
                    "AlturaPorts/Fic/Interfaces/**",
                    "AlturaPorts/Fic/Source/SignalNets",
                    "AlturaPorts/DSIPublicInterface/PublicHeaders",
                    "DAEWin/Include",
                    "AlturaPorts/DigiPublic/Interfaces",
                    "AlturaPorts/DigiPublic",
                    "AlturaPorts/NewFileLibs/DOA",
                    "AlturaPorts/NewFileLibs/Cmn",
                    "xplat/AVX/avx2/avx2sdk/inc",
                    "xplat/AVX/avx2/avx2sdk/utils" };

                for (auto* path : p)
                    owner.addProjectPathToBuildPathList (targetExtraSearchPaths, rtasFolder.getChildFile (path));
            }

            return targetExtraSearchPaths;
        }

        String getOSXDeploymentTarget (const String& deploymentTarget) const
        {
            auto minVersion = (type == Target::AudioUnitv3PlugIn ? minimumAUv3SDKVersion : oldestDeploymentTarget);

            for (int v = minVersion; v <= currentSDKVersion; ++v)
                if (deploymentTarget == getSDKDisplayName (v))
                    return getVersionName (v);

            return getVersionName (minVersion);
        }

        String getOSXSDKVersion (const String& sdkVersion) const
        {
            for (int v = oldestSDKVersion; v <= currentSDKVersion; ++v)
                if (sdkVersion == getSDKDisplayName (v))
                    return getSDKRootName (v);

            return {};
        }

        //==============================================================================
        const XcodeProjectExporter& owner;

        Target& operator= (const Target&) = delete;
    };

    mutable StringArray xcodeFrameworks;
    StringArray xcodeLibs;

private:
    //==============================================================================
    friend class CLionProjectExporter;

    bool xcodeCanUseDwarf;
    OwnedArray<XcodeTarget> targets;

    mutable OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, pbxGroups, misc, projectConfigs, targetConfigs;
    mutable StringArray resourceIDs, sourceIDs, targetIDs;
    mutable StringArray frameworkFileIDs, embeddedFrameworkIDs, rezFileIDs, resourceFileRefs, subprojectFileIDs;
    mutable Array<std::pair<String, String>> subprojectReferences;
    mutable File menuNibFile, iconFile;
    mutable StringArray buildProducts;

    const bool iOS;

    ValueWithDefault customPListValue, pListPrefixHeaderValue, pListPreprocessValue,
                     subprojectsValue,
                     extraFrameworksValue, frameworkSearchPathsValue, extraCustomFrameworksValue, embeddedFrameworksValue,
                     postbuildCommandValue, prebuildCommandValue,
                     duplicateAppExResourcesFolderValue, iosDeviceFamilyValue, iPhoneScreenOrientationValue,
                     iPadScreenOrientationValue, customXcodeResourceFoldersValue, customXcassetsFolderValue,
                     appSandboxValue, appSandboxOptionsValue,
                     hardenedRuntimeValue, hardenedRuntimeOptionsValue,
                     microphonePermissionNeededValue, microphonePermissionsTextValue, cameraPermissionNeededValue, cameraPermissionTextValue, iosBluetoothPermissionNeededValue, iosBluetoothPermissionTextValue,
                     uiFileSharingEnabledValue, uiSupportsDocumentBrowserValue, uiStatusBarHiddenValue, documentExtensionsValue, iosInAppPurchasesValue,
                     iosBackgroundAudioValue, iosBackgroundBleValue, iosPushNotificationsValue, iosAppGroupsValue, iCloudPermissionsValue,
                     iosDevelopmentTeamIDValue, iosAppGroupsIDValue, keepCustomXcodeSchemesValue, useHeaderMapValue, customLaunchStoryboardValue,
                     exporterBundleIdentifierValue;

    static String sanitisePath (const String& path)
    {
        if (path.startsWithChar ('~'))
            return "$(HOME)" + path.substring (1);

        return path;
    }

    static String addQuotesIfRequired (const String& s)
    {
        return s.containsAnyOf (" $") ? s.quoted() : s;
    }

    File getProjectBundle() const                 { return getTargetFolder().getChildFile (project.getProjectFilenameRootString()).withFileExtension (".xcodeproj"); }

    //==============================================================================
    void createObjects() const
    {
        prepareTargets();

        // Must be called before adding embedded frameworks, as we want to
        // embed any frameworks found in subprojects.
        addSubprojects();

        addFrameworks();
        addCustomFrameworks();
        addEmbeddedFrameworks();

        addCustomResourceFolders();
        addPlistFileReferences();

        if (iOS && ! projectType.isStaticLibrary())
        {
            addXcassets();

            if (shouldAddStoryboardToProject())
            {
                auto customLaunchStoryboard = getCustomLaunchStoryboardString();

                if (customLaunchStoryboard.isEmpty())
                    writeDefaultLaunchStoryboardFile();
                else if (getProject().getProjectFolder().getChildFile (customLaunchStoryboard).existsAsFile())
                    addLaunchStoryboardFileReference (RelativePath (customLaunchStoryboard, RelativePath::projectFolder)
                                                          .rebased (getProject().getProjectFolder(), getTargetFolder(), RelativePath::buildTargetFolder)
                                                          .toUnixStyle());
            }
        }
        else
        {
            addNibFiles();
        }

        addIcons();
        addBuildConfigurations();

        addProjectConfigList (projectConfigs, createID ("__projList"));

        {
            StringArray topLevelGroupIDs;

            addFilesAndGroupsToProject (topLevelGroupIDs);
            addBuildPhases();
            addExtraGroupsToProject (topLevelGroupIDs);

            addGroup (createID ("__mainsourcegroup"), "Source", topLevelGroupIDs);
        }

        addProjectObject();
        removeMismatchedXcuserdata();
    }

    void prepareTargets() const
    {
        for (auto* target : targets)
        {
            if (target->type == XcodeTarget::AggregateTarget)
                continue;

            target->addMainBuildProduct();

            auto targetName = target->getName();
            auto fileID = createID (targetName + String ("__targetbuildref"));
            auto fileRefID = createID (String ("__productFileID") + targetName);

            auto* v = new ValueTree (fileID);
            v->setProperty ("isa", "PBXBuildFile", nullptr);
            v->setProperty ("fileRef", fileRefID, nullptr);

            target->mainBuildProductID = fileID;

            pbxBuildFiles.add (v);
            target->addDependency();
        }
    }

    void addPlistFileReferences() const
    {
        for (auto* target : targets)
        {
            if (target->type == XcodeTarget::AggregateTarget)
                continue;

            if (target->shouldCreatePList())
            {
                RelativePath plistPath (target->infoPlistFile, getTargetFolder(), RelativePath::buildTargetFolder);
                addFileReference (plistPath.toUnixStyle());
                resourceFileRefs.add (createFileRefID (plistPath));
            }
        }
    }

    void addNibFiles() const
    {
        MemoryOutputStream nib;
        nib.write (BinaryData::RecentFilesMenuTemplate_nib, BinaryData::RecentFilesMenuTemplate_nibSize);
        overwriteFileIfDifferentOrThrow (menuNibFile, nib);

        RelativePath menuNibPath (menuNibFile, getTargetFolder(), RelativePath::buildTargetFolder);
        addFileReference (menuNibPath.toUnixStyle());
        resourceIDs.add (addBuildFile (menuNibPath, false, false));
        resourceFileRefs.add (createFileRefID (menuNibPath));
    }

    void addIcons() const
    {
        if (iconFile.exists())
        {
            RelativePath iconPath (iconFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (iconPath.toUnixStyle());
            resourceIDs.add (addBuildFile (iconPath, false, false));
            resourceFileRefs.add (createFileRefID (iconPath));
        }
    }

    void addBuildConfigurations() const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            auto& xcodeConfig = dynamic_cast<const XcodeBuildConfiguration&> (*config);
            StringArray settingsLines;
            auto configSettings = getProjectSettings (xcodeConfig);

            for (auto& key : configSettings.getAllKeys())
                settingsLines.add (key + " = " + configSettings[key]);

            addProjectConfig (config->getName(), settingsLines);
        }
    }

    void addFilesAndGroupsToProject (StringArray& topLevelGroupIDs) const
    {
        addEntitlementsFile();

        for (auto& group : getAllGroups())
        {
            if (group.getNumChildren() > 0)
            {
                auto groupID = addProjectItem (group);

                if (groupID.isNotEmpty())
                    topLevelGroupIDs.add (groupID);
            }
        }
    }

    void addExtraGroupsToProject (StringArray& topLevelGroupIDs) const
    {
        {
            auto resourcesGroupID = createID ("__resources");
            addGroup (resourcesGroupID, "Resources", resourceFileRefs);
            topLevelGroupIDs.add (resourcesGroupID);
        }

        {
            auto frameworksGroupID = createID ("__frameworks");
            addGroup (frameworksGroupID, "Frameworks", frameworkFileIDs);
            topLevelGroupIDs.add (frameworksGroupID);
        }

        {
            auto productsGroupID = createID ("__products");
            addGroup (productsGroupID, "Products", buildProducts);
            topLevelGroupIDs.add (productsGroupID);
        }

        if (! subprojectFileIDs.isEmpty())
        {
            auto subprojectLibrariesGroupID = createID ("__subprojects");
            addGroup (subprojectLibrariesGroupID, "Subprojects", subprojectFileIDs);
            topLevelGroupIDs.add (subprojectLibrariesGroupID);
        }
    }

    void addBuildPhases() const
    {
        // add build phases
        for (auto* target : targets)
        {
            if (target->type != XcodeTarget::AggregateTarget)
                buildProducts.add (createID (String ("__productFileID") + String (target->getName())));

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto& xcodeConfig = dynamic_cast<const XcodeBuildConfiguration&> (*config);

                auto configSettings = target->getTargetSettings (xcodeConfig);
                StringArray settingsLines;

                for (auto& key : configSettings.getAllKeys())
                    settingsLines.add (key + " = " + configSettings.getValue (key, "\"\""));

                target->addTargetConfig (config->getName(), settingsLines);
            }

            addConfigList (*target, targetConfigs, createID (String ("__configList") + target->getName()));

            target->addShellScriptBuildPhase ("Pre-build script", getPreBuildScript());

            if (target->type != XcodeTarget::AggregateTarget)
            {
                auto skipAUv3 = (target->type == XcodeTarget::AudioUnitv3PlugIn && ! shouldDuplicateAppExResourcesFolder());

                if (! projectType.isStaticLibrary() && target->type != XcodeTarget::SharedCodeTarget && ! skipAUv3)
                    target->addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

                auto rezFiles = rezFileIDs;
                rezFiles.addArray (target->rezFileIDs);

                if (rezFiles.size() > 0)
                    target->addBuildPhase ("PBXRezBuildPhase", rezFiles);

                auto sourceFiles = target->sourceIDs;

                if (target->type == XcodeTarget::SharedCodeTarget
                     || (! project.getProjectType().isAudioPlugin()))
                    sourceFiles.addArray (sourceIDs);

                target->addBuildPhase ("PBXSourcesBuildPhase", sourceFiles);

                if (! projectType.isStaticLibrary() && target->type != XcodeTarget::SharedCodeTarget)
                    target->addBuildPhase ("PBXFrameworksBuildPhase", target->frameworkIDs);
            }

            target->addShellScriptBuildPhase ("Post-build script", getPostBuildScript());

            if (project.getProjectType().isAudioPlugin() && project.shouldBuildAUv3()
                && project.shouldBuildStandalonePlugin() && target->type == XcodeTarget::StandalonePlugIn)
                embedAppExtension();

            if (project.getProjectType().isAudioPlugin() && project.shouldBuildUnityPlugin()
                && target->type == XcodeTarget::UnityPlugIn)
                embedUnityScript();

            addTargetObject (*target);
        }
    }

    void embedAppExtension() const
    {
        if (auto* standaloneTarget = getTargetOfType (XcodeTarget::StandalonePlugIn))
        {
            if (auto* auv3Target   = getTargetOfType (XcodeTarget::AudioUnitv3PlugIn))
            {
                StringArray files;
                files.add (auv3Target->mainBuildProductID);
                standaloneTarget->addCopyFilesPhase ("Embed App Extensions", files, kPluginsFolder);
            }
        }
    }

    void embedUnityScript() const
    {
        if (auto* unityTarget = getTargetOfType (XcodeTarget::UnityPlugIn))
        {
            RelativePath scriptPath (getProject().getGeneratedCodeFolder().getChildFile (getProject().getUnityScriptName()),
                                     getTargetFolder(),
                                     RelativePath::buildTargetFolder);

            auto path = scriptPath.toUnixStyle();
            auto refID = addFileReference (path);
            auto fileID = addBuildFile (path, refID, false, false);

            resourceIDs.add (fileID);
            resourceFileRefs.add (refID);

            unityTarget->addCopyFilesPhase ("Embed Unity Script", fileID, kResourcesFolder);
        }
    }

    static Image fixMacIconImageSize (Drawable& image)
    {
        const int validSizes[] = { 16, 32, 64, 128, 256, 512, 1024 };

        auto w = image.getWidth();
        auto h = image.getHeight();

        int bestSize = 16;

        for (int size : validSizes)
        {
            if (w == h && w == size)
            {
                bestSize = w;
                break;
            }

            if (jmax (w, h) > size)
                bestSize = size;
        }

        return rescaleImageForIcon (image, bestSize);
    }

    //==============================================================================
    XcodeTarget* getTargetOfType (ProjectType::Target::Type type) const
    {
        for (auto& target : targets)
            if (target->type == type)
                return target;

        return nullptr;
    }

    void addTargetObject (XcodeTarget& target) const
    {
        auto targetName = target.getName();

        auto targetID = target.getID();
        auto* v = new ValueTree (targetID);
        v->setProperty ("isa", target.type == XcodeTarget::AggregateTarget ? "PBXAggregateTarget" : "PBXNativeTarget", nullptr);
        v->setProperty ("buildConfigurationList", createID (String ("__configList") + targetName), nullptr);

        v->setProperty ("buildPhases", indentParenthesisedList (target.buildPhaseIDs), nullptr);
        v->setProperty ("buildRules", "( )", nullptr);

        v->setProperty ("dependencies", indentParenthesisedList (getTargetDependencies (target)), nullptr);
        v->setProperty (Ids::name, target.getXcodeSchemeName(), nullptr);

        v->setProperty ("productName", projectName, nullptr);

        if (target.type != XcodeTarget::AggregateTarget)
        {
            v->setProperty ("productReference", createID (String ("__productFileID") + targetName), nullptr);

            jassert (target.xcodeProductType.isNotEmpty());
            v->setProperty ("productType", target.xcodeProductType, nullptr);
        }

        targetIDs.add (targetID);
        misc.add (v);
    }

    StringArray getTargetDependencies (const XcodeTarget& target) const
    {
        StringArray dependencies;

        if (project.getProjectType().isAudioPlugin())
        {
            if (target.type == XcodeTarget::StandalonePlugIn) // depends on AUv3 and shared code
            {
                if (auto* auv3Target = getTargetOfType (XcodeTarget::AudioUnitv3PlugIn))
                    dependencies.add (auv3Target->getDependencyID());

                if (auto* sharedCodeTarget = getTargetOfType (XcodeTarget::SharedCodeTarget))
                    dependencies.add (sharedCodeTarget->getDependencyID());
            }
            else if (target.type == XcodeTarget::AggregateTarget) // depends on all other targets
            {
                for (int i = 1; i < targets.size(); ++i)
                    dependencies.add (targets[i]->getDependencyID());
            }
            else if (target.type != XcodeTarget::SharedCodeTarget) // shared code doesn't depend on anything; all other targets depend only on the shared code
            {
                if (auto* sharedCodeTarget = getTargetOfType (XcodeTarget::SharedCodeTarget))
                    dependencies.add (sharedCodeTarget->getDependencyID());
            }
        }

        return dependencies;
    }

    static void writeIconData (MemoryOutputStream& out, const Image& image, const char* type)
    {
        MemoryOutputStream pngData;
        PNGImageFormat pngFormat;
        pngFormat.writeImageToStream (image, pngData);

        out.write (type, 4);
        out.writeIntBigEndian (8 + (int) pngData.getDataSize());
        out << pngData;
    }

    void writeIcnsFile (const OwnedArray<Drawable>& images, OutputStream& out) const
    {
        MemoryOutputStream data;
        auto smallest = std::numeric_limits<int>::max();
        Drawable* smallestImage = nullptr;

        for (int i = 0; i < images.size(); ++i)
        {
            auto image = fixMacIconImageSize (*images.getUnchecked (i));
            jassert (image.getWidth() == image.getHeight());

            if (image.getWidth() < smallest)
            {
                smallest = image.getWidth();
                smallestImage = images.getUnchecked(i);
            }

            switch (image.getWidth())
            {
                case 16:   writeIconData (data, image, "icp4"); break;
                case 32:   writeIconData (data, image, "icp5"); break;
                case 64:   writeIconData (data, image, "icp6"); break;
                case 128:  writeIconData (data, image, "ic07"); break;
                case 256:  writeIconData (data, image, "ic08"); break;
                case 512:  writeIconData (data, image, "ic09"); break;
                case 1024: writeIconData (data, image, "ic10"); break;
                default:   break;
            }
        }

        jassert (data.getDataSize() > 0); // no suitable sized images?

        // If you only supply a 1024 image, the file doesn't work on 10.8, so we need
        // to force a smaller one in there too..
        if (smallest > 512 && smallestImage != nullptr)
            writeIconData (data, rescaleImageForIcon (*smallestImage, 512), "ic09");

        out.write ("icns", 4);
        out.writeIntBigEndian ((int) data.getDataSize() + 8);
        out << data;
    }

    void getIconImages (OwnedArray<Drawable>& images) const
    {
        if (auto icon = getBigIcon())
            images.add (std::move (icon));

        if (auto icon = getSmallIcon())
            images.add (std::move (icon));
    }

    void createiOSIconFiles (File appIconSet) const
    {
        OwnedArray<Drawable> images;
        getIconImages (images);

        if (images.size() > 0)
        {
            for (auto& type : getiOSAppIconTypes())
            {
                auto image = rescaleImageForIcon (*images.getFirst(), type.size);

                if (image.hasAlphaChannel())
                {
                    Image background (Image::RGB, image.getWidth(), image.getHeight(), false);
                    Graphics g (background);
                    g.fillAll (Colours::white);

                    g.drawImageWithin (image, 0, 0, image.getWidth(), image.getHeight(),
                                       RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);

                    image = background;
                }

                MemoryOutputStream pngData;
                PNGImageFormat pngFormat;
                pngFormat.writeImageToStream (image, pngData);

                overwriteFileIfDifferentOrThrow (appIconSet.getChildFile (type.filename), pngData);
            }
        }
    }

    void createIconFile() const
    {
        OwnedArray<Drawable> images;
        getIconImages (images);

        if (images.size() > 0)
        {
            MemoryOutputStream mo;
            writeIcnsFile (images, mo);

            iconFile = getTargetFolder().getChildFile ("Icon.icns");
            overwriteFileIfDifferentOrThrow (iconFile, mo);
        }
    }

    void writeWorkspaceSettings() const
    {
        auto settingsFile = getProjectBundle().getChildFile ("project.xcworkspace")
                                              .getChildFile ("xcshareddata")
                                              .getChildFile ("WorkspaceSettings.xcsettings");

        MemoryOutputStream mo;
        mo.setNewLineString ("\n");

        mo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << newLine
           << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << newLine
           << "<plist version=\"1.0\">"                    << newLine
           << "<dict>"                                     << newLine
           << "\t" << "<key>BuildSystemType</key>"         << newLine
           << "\t" << "<string>Original</string>"          << newLine
           << "</dict>"                                    << newLine
           << "</plist>"                                   << newLine;

        overwriteFileIfDifferentOrThrow (settingsFile, mo);
    }

    void writeInfoPlistFiles() const
    {
        for (auto& target : targets)
           target->writeInfoPlistFile();
    }

    // Delete .rsrc files in folder but don't follow sym-links
    void deleteRsrcFiles (const File& folder) const
    {
        for (DirectoryIterator di (folder, false, "*", File::findFilesAndDirectories); di.next();)
        {
            auto& entry = di.getFile();

            if (! entry.isSymbolicLink())
            {
                if (entry.existsAsFile() && entry.getFileExtension().toLowerCase() == ".rsrc")
                    entry.deleteFile();
                else if (entry.isDirectory())
                    deleteRsrcFiles (entry);
            }
        }
    }

    static String getLinkerFlagForLib (String library)
    {
        if (library.substring (0, 3) == "lib")
            library = library.substring (3);

        return "-l" + library.replace (" ", "\\\\ ").replace ("\"", "\\\\\"").replace ("\'", "\\\\\'").upToLastOccurrenceOf (".", false, false);
    }

    String getSearchPathForStaticLibrary (const RelativePath& library) const
    {
        auto searchPath = library.toUnixStyle().upToLastOccurrenceOf ("/", false, false);

        if (! library.isAbsolute())
        {
            auto srcRoot = rebaseFromProjectFolderToBuildTarget (RelativePath (".", RelativePath::projectFolder)).toUnixStyle();

            if (srcRoot.endsWith ("/."))      srcRoot = srcRoot.dropLastCharacters (2);
            if (! srcRoot.endsWithChar ('/')) srcRoot << '/';

            searchPath = srcRoot + searchPath;
        }

        return sanitisePath (searchPath);
    }

    String getCodeSigningIdentity (const XcodeBuildConfiguration& config) const
    {
        auto identity = config.getCodeSignIdentityString();

        if (identity.isEmpty() && getDevelopmentTeamIDString().isNotEmpty())
            return iOS ? "iPhone Developer" : "Mac Developer";

        return identity;
    }

    StringPairArray getProjectSettings (const XcodeBuildConfiguration& config) const
    {
        StringPairArray s;

        s.set ("ALWAYS_SEARCH_USER_PATHS", "NO");
        s.set ("ENABLE_STRICT_OBJC_MSGSEND", "YES");
        s.set ("GCC_C_LANGUAGE_STANDARD", "c11");
        s.set ("GCC_NO_COMMON_BLOCKS", "YES");
        s.set ("GCC_MODEL_TUNING", "G5");
        s.set ("GCC_WARN_ABOUT_RETURN_TYPE", "YES");
        s.set ("GCC_WARN_CHECK_SWITCH_STATEMENTS", "YES");
        s.set ("GCC_WARN_UNUSED_VARIABLE", "YES");
        s.set ("GCC_WARN_MISSING_PARENTHESES", "YES");
        s.set ("GCC_WARN_NON_VIRTUAL_DESTRUCTOR", "YES");
        s.set ("GCC_WARN_TYPECHECK_CALLS_TO_PRINTF", "YES");
        s.set ("GCC_WARN_64_TO_32_BIT_CONVERSION", "YES");
        s.set ("GCC_WARN_UNDECLARED_SELECTOR", "YES");
        s.set ("GCC_WARN_UNINITIALIZED_AUTOS", "YES");
        s.set ("GCC_WARN_UNUSED_FUNCTION", "YES");
        s.set ("CLANG_ENABLE_OBJC_WEAK", "YES");
        s.set ("CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING", "YES");
        s.set ("CLANG_WARN_BOOL_CONVERSION", "YES");
        s.set ("CLANG_WARN_COMMA", "YES");
        s.set ("CLANG_WARN_CONSTANT_CONVERSION", "YES");
        s.set ("CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS", "YES");
        s.set ("CLANG_WARN_EMPTY_BODY", "YES");
        s.set ("CLANG_WARN_ENUM_CONVERSION", "YES");
        s.set ("CLANG_WARN_INFINITE_RECURSION", "YES");
        s.set ("CLANG_WARN_INT_CONVERSION", "YES");
        s.set ("CLANG_WARN_NON_LITERAL_NULL_CONVERSION", "YES");
        s.set ("CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF", "YES");
        s.set ("CLANG_WARN_OBJC_LITERAL_CONVERSION", "YES");
        s.set ("CLANG_WARN_RANGE_LOOP_ANALYSIS", "YES");
        s.set ("CLANG_WARN_STRICT_PROTOTYPES", "YES");
        s.set ("CLANG_WARN_SUSPICIOUS_MOVE", "YES");
        s.set ("CLANG_WARN_UNREACHABLE_CODE", "YES");
        s.set ("CLANG_WARN__DUPLICATE_METHOD_MATCH", "YES");
        s.set ("WARNING_CFLAGS", "\"-Wreorder\"");

        if (projectType.isStaticLibrary())
        {
            s.set ("GCC_INLINES_ARE_PRIVATE_EXTERN", "NO");
            s.set ("GCC_SYMBOLS_PRIVATE_EXTERN", "NO");
        }
        else
        {
            s.set ("GCC_INLINES_ARE_PRIVATE_EXTERN", "YES");
        }

        if (config.isDebug())
        {
            s.set ("ENABLE_TESTABILITY", "YES");

            if (config.getOSXArchitectureString() == osxArch_Default)
                s.set ("ONLY_ACTIVE_ARCH", "YES");
        }

        s.set (iOS ? "\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\"" : "CODE_SIGN_IDENTITY",
               getCodeSigningIdentity (config).quoted());

        if (iOS)
        {
            s.set ("SDKROOT", "iphoneos");
            s.set ("TARGETED_DEVICE_FAMILY", getDeviceFamilyString().quoted());
            s.set ("IPHONEOS_DEPLOYMENT_TARGET", config.getiOSDeploymentTargetString());
        }

        s.set ("ZERO_LINK", "NO");

        if (xcodeCanUseDwarf)
            s.set ("DEBUG_INFORMATION_FORMAT", "dwarf");

        s.set ("PRODUCT_NAME", replacePreprocessorTokens (config, config.getTargetBinaryNameString()).quoted());

        return s;
    }

    void addFrameworks() const
    {
        if (! projectType.isStaticLibrary())
        {
            if (isInAppPurchasesEnabled())
                xcodeFrameworks.addIfNotAlreadyThere ("StoreKit");

            if (iOS && isPushNotificationsEnabled())
                xcodeFrameworks.addIfNotAlreadyThere ("UserNotifications");

            if (isiOS() && project.getConfigFlag ("JUCE_USE_CAMERA").get())
                xcodeFrameworks.addIfNotAlreadyThere ("ImageIO");

            xcodeFrameworks.addTokens (getExtraFrameworksString(), ",;", "\"'");
            xcodeFrameworks.trim();

            auto s = xcodeFrameworks;

            for (auto& target : targets)
                s.addArray (target->xcodeFrameworks);

            if (! project.getConfigFlag ("JUCE_QUICKTIME").get())
                s.removeString ("QuickTime");

            s.trim();
            s.removeDuplicates (true);
            s.sort (true);

            for (auto& framework : s)
            {
                auto frameworkID = addFramework (framework);

                // find all the targets that are referring to this object
                for (auto& target : targets)
                {
                    if (xcodeFrameworks.contains (framework) || target->xcodeFrameworks.contains (framework))
                    {
                        target->frameworkIDs.add (frameworkID);
                        target->frameworkNames.add (framework);
                    }
                }
            }
        }
    }

    void addCustomFrameworks() const
    {
        StringArray customFrameworks;
        customFrameworks.addTokens (getExtraCustomFrameworksString(), true);
        customFrameworks.trim();

        for (auto& framework : customFrameworks)
        {
            auto frameworkID = addCustomFramework (framework);

            for (auto& target : targets)
            {
                target->frameworkIDs.add (frameworkID);
                target->frameworkNames.add (framework);
            }
        }
    }

    void addEmbeddedFrameworks() const
    {
        StringArray frameworks;
        frameworks.addTokens (getEmbeddedFrameworksString(), true);
        frameworks.trim();

        for (auto& framework : frameworks)
        {
            auto frameworkID = addEmbeddedFramework (framework);
            embeddedFrameworkIDs.add (frameworkID);

            for (auto& target : targets)
            {
                target->frameworkIDs.add (frameworkID);
                target->frameworkNames.add (framework);
            }
        }

        if (! embeddedFrameworkIDs.isEmpty())
            for (auto& target : targets)
                target->addCopyFilesPhase ("Embed Frameworks", embeddedFrameworkIDs, kFrameworksFolder);
    }

    void addCustomResourceFolders() const
    {
        StringArray folders;

        folders.addTokens (getCustomResourceFoldersString(), ":", "");
        folders.trim();
        folders.removeEmptyStrings();

        for (auto& crf : folders)
            addCustomResourceFolder (crf);
    }

    void addSubprojects() const
    {
        auto subprojectLines = StringArray::fromLines (getSubprojectsString());
        subprojectLines.removeEmptyStrings (true);

        Array<std::pair<String, StringArray>> subprojects;

        for (auto& line : subprojectLines)
        {
            String subprojectName (line.upToFirstOccurrenceOf (":", false, false));
            StringArray requestedBuildProducts (StringArray::fromTokens (line.fromFirstOccurrenceOf (":", false, false), ",;|", "\"'"));
            requestedBuildProducts.trim();
            subprojects.add ({ subprojectName, requestedBuildProducts });
        }

        for (const auto& subprojectInfo : subprojects)
        {
            String subprojectPath (subprojectInfo.first);

            if (! subprojectPath.endsWith (".xcodeproj"))
                subprojectPath += ".xcodeproj";

            File subprojectFile;

            if (File::isAbsolutePath (subprojectPath))
            {
                subprojectFile = subprojectPath;
            }
            else
            {
                subprojectFile = getProject().getProjectFolder().getChildFile (subprojectPath);

                RelativePath p (subprojectPath, RelativePath::projectFolder);
                subprojectPath = p.rebased (getProject().getProjectFolder(), getTargetFolder(), RelativePath::buildTargetFolder).toUnixStyle();
            }

            if (! subprojectFile.isDirectory())
                continue;

            auto availableBuildProducts = XcodeProjectParser::parseBuildProducts (subprojectFile);

            // If no build products have been specified then we'll take everything
            if (! subprojectInfo.second.isEmpty())
            {
                auto newEnd = std::remove_if (availableBuildProducts.begin(), availableBuildProducts.end(),
                                              [&subprojectInfo](const std::pair<String, String> &item)
                                              {
                                                  return ! subprojectInfo.second.contains (item.first);
                                              });
                availableBuildProducts.erase (newEnd, availableBuildProducts.end());
            }

            if (availableBuildProducts.empty())
                continue;

            auto subprojectFileType = getFileType (RelativePath (subprojectPath, RelativePath::projectFolder));
            auto subprojectFileID = addFileOrFolderReference (subprojectPath, "<group>", subprojectFileType);
            subprojectFileIDs.add (subprojectFileID);

            StringArray proxyIDs;

            for (auto& buildProduct : availableBuildProducts)
            {
                auto buildProductFileType = getFileType (RelativePath (buildProduct.second, RelativePath::projectFolder));

                auto containerID = addContainerItemProxy (subprojectFileID, buildProduct.first);
                auto proxyID = addReferenceProxy (containerID, buildProduct.second, buildProductFileType);
                proxyIDs.add (proxyID);

                if (buildProductFileType == "archive.ar" || buildProductFileType == "wrapper.framework")
                {
                    auto buildFileID = addBuildFile (buildProduct.second, proxyID, false, true);

                    for (auto& target : targets)
                        target->frameworkIDs.add (buildFileID);

                    if (buildProductFileType == "wrapper.framework")
                    {
                        auto fileID = createID (buildProduct.second + "buildref");

                        auto* v = new ValueTree (fileID);
                        v->setProperty ("isa", "PBXBuildFile", nullptr);
                        v->setProperty ("fileRef", proxyID, nullptr);
                        v->setProperty ("settings", "{ATTRIBUTES = (CodeSignOnCopy, RemoveHeadersOnCopy, ); }", nullptr);
                        pbxBuildFiles.add (v);

                        embeddedFrameworkIDs.add (fileID);
                    }
                }
            }

            auto productGroupID = createFileRefID (subprojectPath + "_products");
            addGroup (productGroupID, "Products", proxyIDs);

            subprojectReferences.add ({ productGroupID, subprojectFileID });
        }
    }

    void addXcassets() const
    {
        auto customXcassetsPath = getCustomXcassetsFolderString();

        if (customXcassetsPath.isEmpty())
            createXcassetsFolderFromIcons();
        else
            addCustomResourceFolder (customXcassetsPath, "folder.assetcatalog");
    }

    void addCustomResourceFolder (String folderPathRelativeToProjectFolder, const String fileType = "folder") const
    {
        auto folderPath = RelativePath (folderPathRelativeToProjectFolder, RelativePath::projectFolder)
                                       .rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder)
                                       .toUnixStyle();

        auto fileRefID = createFileRefID (folderPath);

        addFileOrFolderReference (folderPath, "<group>", fileType);

        resourceIDs.add (addBuildFile (folderPath, fileRefID, false, false));
        resourceFileRefs.add (createFileRefID (folderPath));
    }

    //==============================================================================
    void writeProjectFile (OutputStream& output) const
    {
        output << "// !$*UTF8*$!\n{\n"
                  "\tarchiveVersion = 1;\n"
                  "\tclasses = {\n\t};\n"
                  "\tobjectVersion = 46;\n"
                  "\tobjects = {\n";

        Array<ValueTree*> objects;
        objects.addArray (pbxBuildFiles);
        objects.addArray (pbxFileReferences);
        objects.addArray (pbxGroups);
        objects.addArray (targetConfigs);
        objects.addArray (projectConfigs);
        objects.addArray (misc);

        for (auto* o : objects)
        {
            output << "\t\t" << o->getType().toString() << " = {\n";

            for (int j = 0; j < o->getNumProperties(); ++j)
            {
                auto propertyName = o->getPropertyName(j);
                auto val = o->getProperty (propertyName).toString();

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-_@~\r\n\\#%^`*")
                                        && ! (val.trimStart().startsWithChar ('(')
                                                || val.trimStart().startsWithChar ('{'))))
                    val = "\"" + val + "\"";

                output << "\t\t\t" << propertyName.toString() << " = " << val << ";\n";
            }

            output << "\t\t};\n";
        }

        output << "\t};\n\trootObject = " << createID ("__root") << ";\n}\n";
    }

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings,
                         XcodeTarget* xcodeTarget = nullptr, String compilerFlags = {}) const
    {
        auto fileID = createID (path + "buildref");

        if (addToSourceBuildPhase)
        {
            if (xcodeTarget != nullptr)
                xcodeTarget->sourceIDs.add (fileID);
            else
                sourceIDs.add (fileID);
        }

        auto* v = new ValueTree (fileID);
        v->setProperty ("isa", "PBXBuildFile", nullptr);
        v->setProperty ("fileRef", fileRefID, nullptr);

        if (inhibitWarnings)
            compilerFlags += " -w";

        if (compilerFlags.isNotEmpty())
            v->setProperty ("settings", "{ COMPILER_FLAGS = \"" + compilerFlags.trim() + "\"; }", nullptr);

        pbxBuildFiles.add (v);
        return fileID;
    }

    String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings, XcodeTarget* xcodeTarget = nullptr) const
    {
        return addBuildFile (path.toUnixStyle(), createFileRefID (path), addToSourceBuildPhase, inhibitWarnings, xcodeTarget);
    }

    String addFileReference (String pathString) const
    {
        String sourceTree ("SOURCE_ROOT");
        RelativePath path (pathString, RelativePath::unknown);

        if (pathString.startsWith ("${"))
        {
            sourceTree = pathString.substring (2).upToFirstOccurrenceOf ("}", false, false);
            pathString = pathString.fromFirstOccurrenceOf ("}/", false, false);
        }
        else if (path.isAbsolute())
        {
            sourceTree = "<absolute>";
        }

        auto fileType = getFileType (path);

        return addFileOrFolderReference (pathString, sourceTree, fileType);
    }

    void checkAndAddFileReference (std::unique_ptr<ValueTree> v) const
    {
        auto existing = pbxFileReferences.indexOfSorted (*this, v.get());

        if (existing >= 0)
        {
            // If this fails, there's either a string hash collision, or the same file is being added twice (incorrectly)
            jassert (pbxFileReferences.getUnchecked (existing)->isEquivalentTo (*v));
        }
        else
        {
            pbxFileReferences.addSorted (*this, v.release());
        }
    }

    String addFileOrFolderReference (const String& pathString, String sourceTree, String fileType) const
    {
        auto fileRefID = createFileRefID (pathString);

        std::unique_ptr<ValueTree> v (new ValueTree (fileRefID));
        v->setProperty ("isa", "PBXFileReference", nullptr);
        v->setProperty ("lastKnownFileType", fileType, nullptr);
        v->setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), nullptr);
        v->setProperty ("path", pathString, nullptr);
        v->setProperty ("sourceTree", sourceTree, nullptr);

        checkAndAddFileReference (std::move (v));

        return fileRefID;
    }

    String addContainerItemProxy (const String& subprojectID, const String& itemName) const
    {
        auto uniqueString = subprojectID + "_" + itemName;
        auto fileRefID = createFileRefID (uniqueString);

        std::unique_ptr<ValueTree> v (new ValueTree (fileRefID));
        v->setProperty ("isa", "PBXContainerItemProxy", nullptr);
        v->setProperty ("containerPortal", subprojectID, nullptr);
        v->setProperty ("proxyType", 2, nullptr);
        v->setProperty ("remoteGlobalIDString", createFileRefID (uniqueString + "_global"), nullptr);
        v->setProperty ("remoteInfo", itemName, nullptr);

        checkAndAddFileReference (std::move (v));

        return fileRefID;
    }

    String addReferenceProxy (const String& containerItemID, const String& proxyPath, const String& fileType) const
    {
        auto fileRefID = createFileRefID (containerItemID + "_" + proxyPath);

        std::unique_ptr<ValueTree> v (new ValueTree (fileRefID));
        v->setProperty ("isa", "PBXReferenceProxy", nullptr);
        v->setProperty ("fileType", fileType, nullptr);
        v->setProperty ("path", proxyPath, nullptr);
        v->setProperty ("remoteRef", containerItemID, nullptr);
        v->setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", nullptr);

        checkAndAddFileReference (std::move (v));

        return fileRefID;
    }

public:
    static int compareElements (const ValueTree* first, const ValueTree* second)
    {
        return first->getType().getCharPointer().compare (second->getType().getCharPointer());
    }

private:
    static String getFileType (const RelativePath& file)
    {
        if (file.hasFileExtension (cppFileExtensions))      return "sourcecode.cpp.cpp";
        if (file.hasFileExtension (".mm"))                  return "sourcecode.cpp.objcpp";
        if (file.hasFileExtension (".m"))                   return "sourcecode.c.objc";
        if (file.hasFileExtension (".c"))                   return "sourcecode.c.c";
        if (file.hasFileExtension (headerFileExtensions))   return "sourcecode.c.h";
        if (file.hasFileExtension (asmFileExtensions))      return "sourcecode.c.asm";
        if (file.hasFileExtension (".framework"))           return "wrapper.framework";
        if (file.hasFileExtension (".jpeg;.jpg"))           return "image.jpeg";
        if (file.hasFileExtension ("png;gif"))              return "image" + file.getFileExtension();
        if (file.hasFileExtension ("html;htm"))             return "text.html";
        if (file.hasFileExtension ("xml;zip;wav"))          return "file" + file.getFileExtension();
        if (file.hasFileExtension ("txt;rtf"))              return "text" + file.getFileExtension();
        if (file.hasFileExtension ("plist"))                return "text.plist.xml";
        if (file.hasFileExtension ("entitlements"))         return "text.plist.xml";
        if (file.hasFileExtension ("app"))                  return "wrapper.application";
        if (file.hasFileExtension ("component;vst;plugin")) return "wrapper.cfbundle";
        if (file.hasFileExtension ("xcodeproj"))            return "wrapper.pb-project";
        if (file.hasFileExtension ("a"))                    return "archive.ar";
        if (file.hasFileExtension ("xcassets"))             return "folder.assetcatalog";

        return "file" + file.getFileExtension();
    }

    String addFile (const RelativePath& path, bool shouldBeCompiled, bool shouldBeAddedToBinaryResources,
                    bool shouldBeAddedToXcodeResources, bool inhibitWarnings, XcodeTarget* xcodeTarget, const String& compilerFlags) const
    {
        auto pathAsString = path.toUnixStyle();
        auto refID = addFileReference (path.toUnixStyle());

        if (shouldBeCompiled)
        {
            addBuildFile (pathAsString, refID, true, inhibitWarnings, xcodeTarget, compilerFlags);
        }
        else if (! shouldBeAddedToBinaryResources || shouldBeAddedToXcodeResources)
        {
            auto fileType = getFileType (path);

            if (shouldBeAddedToXcodeResources)
            {
                resourceIDs.add (addBuildFile (pathAsString, refID, false, false));
                resourceFileRefs.add (refID);
            }
        }

        return refID;
    }

    String addRezFile (const Project::Item& projectItem, const RelativePath& path) const
    {
        auto pathAsString = path.toUnixStyle();
        auto refID = addFileReference (path.toUnixStyle());

        if (projectItem.isModuleCode())
        {
            if (auto* xcodeTarget = getTargetOfType (getProject().getTargetTypeFromFilePath (projectItem.getFile(), false)))
            {
                auto rezFileID = addBuildFile (pathAsString, refID, false, false, xcodeTarget);
                xcodeTarget->rezFileIDs.add (rezFileID);

                return refID;
            }
        }

        return {};
    }

    String getEntitlementsFileName() const
    {
        return project.getProjectFilenameRootString() + String (".entitlements");
    }

    StringPairArray getEntitlements() const
    {
        StringPairArray entitlements;

        if (project.getProjectType().isAudioPlugin())
        {
            if (isiOS())
            {
                if (project.shouldEnableIAA())
                    entitlements.set ("inter-app-audio", "<true/>");
            }
            else
            {
                entitlements.set ("com.apple.security.app-sandbox", "<true/>");
            }
        }
        else
        {
            if (isPushNotificationsEnabled())
                entitlements.set (isiOS() ? "aps-environment"
                                          : "com.apple.developer.aps-environment",
                                            "<string>development</string>");
        }

        if (isAppGroupsEnabled())
        {
            auto appGroups = StringArray::fromTokens (getAppGroupIdString(), ";", {});
            auto groups = String ("<array>");

            for (auto group : appGroups)
                groups += "\n\t\t<string>" + group.trim() + "</string>";

            groups += "\n\t</array>";

            entitlements.set ("com.apple.security.application-groups", groups);
        }

        if (isHardenedRuntimeEnabled())
            for (auto& option : getHardenedRuntimeOptions())
                entitlements.set (option, "<true/>");

        if (isAppSandboxEnabled())
            for (auto& option : getAppSandboxOptions())
                entitlements.set (option, "<true/>");

        if (isiOS() && isiCloudPermissionsEnabled())
        {
            entitlements.set ("com.apple.developer.icloud-container-identifiers",
                              "<array>\n"
                              "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                              "    </array>");

            entitlements.set ("com.apple.developer.icloud-services",
                              "<array>\n"
                              "        <string>CloudDocuments</string>\n"
                              "    </array>");

            entitlements.set ("com.apple.developer.ubiquity-container-identifiers",
                              "<array>\n"
                              "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                              "    </array>");
        }

        return entitlements;
    }

    String addEntitlementsFile() const
    {
        String content =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n";

        auto entitlements = getEntitlements();
        auto keys = entitlements.getAllKeys();

        for (auto& key : keys)
        {
            content += "\t<key>" + key + "</key>\n"
                       "\t" + entitlements[key] + "\n";
        }
        content += "</dict>\n"
                   "</plist>\n";

        auto entitlementsFile = getTargetFolder().getChildFile (getEntitlementsFileName());
        overwriteFileIfDifferentOrThrow (entitlementsFile, content);

        RelativePath plistPath (entitlementsFile, getTargetFolder(), RelativePath::buildTargetFolder);
        return addFile (plistPath, false, false, false, false, nullptr, {});
    }

    String addProjectItem (const Project::Item& projectItem) const
    {
        if (modulesGroup != nullptr && projectItem.getParent() == *modulesGroup)
            return addFileReference (rebaseFromProjectFolderToBuildTarget (getModuleFolderRelativeToProject (projectItem.getName())).toUnixStyle());

        if (projectItem.isGroup())
        {
            StringArray childIDs;
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
            {
                auto child = projectItem.getChild (i);

                auto childID = addProjectItem (child);

                if (childID.isNotEmpty() && ! child.shouldBeAddedToXcodeResources())
                    childIDs.add (childID);
            }

            if (childIDs.isEmpty())
                return {};

            return addGroup (projectItem, childIDs);
        }

        if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (*this))
        {
            auto itemPath = projectItem.getFilePath();
            RelativePath path;

            if (itemPath.startsWith ("${"))
                path = RelativePath (itemPath, RelativePath::unknown);
            else
                path = RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            if (path.hasFileExtension (".r"))
                return addRezFile (projectItem, path);

            XcodeTarget* xcodeTarget = nullptr;
            if (projectItem.isModuleCode() && projectItem.shouldBeCompiled())
                xcodeTarget = getTargetOfType (project.getTargetTypeFromFilePath (projectItem.getFile(), false));

            return addFile (path, projectItem.shouldBeCompiled(),
                            projectItem.shouldBeAddedToBinaryResources(),
                            projectItem.shouldBeAddedToXcodeResources(),
                            projectItem.shouldInhibitWarnings(),
                            xcodeTarget,
                            compilerFlagSchemesMap[projectItem.getCompilerFlagSchemeString()].get());
        }

        return {};
    }

    String addFramework (const String& frameworkName) const
    {
        auto path = frameworkName;
        auto isRelativePath = path.startsWith ("../");

        if (! File::isAbsolutePath (path) && ! isRelativePath)
            path = "System/Library/Frameworks/" + path;

        if (! path.endsWithIgnoreCase (".framework"))
            path << ".framework";

        auto fileRefID = createFileRefID (path);

        addFileReference (((File::isAbsolutePath (frameworkName) || isRelativePath) ? "" : "${SDKROOT}/") + path);
        frameworkFileIDs.add (fileRefID);

        return addBuildFile (path, fileRefID, false, false);
    }

    String addCustomFramework (String frameworkPath) const
    {
        if (! frameworkPath.endsWithIgnoreCase (".framework"))
            frameworkPath << ".framework";

        auto fileRefID = createFileRefID (frameworkPath);

        auto fileType = getFileType (RelativePath (frameworkPath, RelativePath::projectFolder));
        addFileOrFolderReference (frameworkPath, "<group>", fileType);

        frameworkFileIDs.add (fileRefID);

        return addBuildFile (frameworkPath, fileRefID, false, false);
    }

    String addEmbeddedFramework (const String& path) const
    {
        auto fileRefID = createFileRefID (path);

        auto fileType = getFileType (RelativePath (path, RelativePath::projectFolder));
        addFileOrFolderReference (path, "<group>", fileType);

        auto fileID = createID (path + "buildref");

        auto* v = new ValueTree (fileID);
        v->setProperty ("isa", "PBXBuildFile", nullptr);
        v->setProperty ("fileRef", fileRefID, nullptr);
        v->setProperty ("settings", "{ ATTRIBUTES = (CodeSignOnCopy, RemoveHeadersOnCopy, ); }", nullptr);
        pbxBuildFiles.add (v);

        frameworkFileIDs.add (fileRefID);

        return fileID;
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs) const
    {
        auto* v = new ValueTree (groupID);
        v->setProperty ("isa", "PBXGroup", nullptr);
        v->setProperty ("children", indentParenthesisedList (childIDs), nullptr);
        v->setProperty (Ids::name, groupName, nullptr);
        v->setProperty ("sourceTree", "<group>", nullptr);
        pbxGroups.add (v);
    }

    String addGroup (const Project::Item& item, StringArray& childIDs) const
    {
        auto groupName = item.getName();
        auto groupID = getIDForGroup (item);
        addGroup (groupID, groupName, childIDs);
        return groupID;
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings) const
    {
        auto* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", nullptr);
        v->setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
        v->setProperty (Ids::name, configName, nullptr);
        projectConfigs.add (v);
    }

    void addConfigList (XcodeTarget& target, const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        auto* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", indentParenthesisedList (target.configIDs), nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (auto* first = configsToUse.getFirst())
            v->setProperty ("defaultConfigurationName", first->getProperty (Ids::name), nullptr);

        misc.add (v);
    }

    void addProjectConfigList (const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        StringArray configIDs;

        for (auto* c : configsToUse)
            configIDs.add (c->getType().toString());

        auto* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", indentParenthesisedList (configIDs), nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (auto* first = configsToUse.getFirst())
            v->setProperty ("defaultConfigurationName", first->getProperty (Ids::name), nullptr);

        misc.add (v);
    }

    void addProjectObject() const
    {
        auto* v = new ValueTree (createID ("__root"));
        v->setProperty ("isa", "PBXProject", nullptr);
        v->setProperty ("buildConfigurationList", createID ("__projList"), nullptr);
        v->setProperty ("attributes", getProjectObjectAttributes(), nullptr);
        v->setProperty ("compatibilityVersion", "Xcode 3.2", nullptr);
        v->setProperty ("hasScannedForEncodings", (int) 0, nullptr);
        v->setProperty ("mainGroup", createID ("__mainsourcegroup"), nullptr);
        v->setProperty ("projectDirPath", "\"\"", nullptr);

        if (! subprojectReferences.isEmpty())
        {
            StringArray projectReferences;

            for (auto& reference : subprojectReferences)
                projectReferences.add (indentBracedList ({ "ProductGroup = " + reference.first, "ProjectRef = " + reference.second }, 1));

            v->setProperty ("projectReferences", indentParenthesisedList (projectReferences), nullptr);
        }

        v->setProperty ("projectRoot", "\"\"", nullptr);

        auto targetString = "(" + targetIDs.joinIntoString (", ") + ")";
        v->setProperty ("targets", targetString, nullptr);

        v->setProperty ("knownRegions", "(en, Base)", nullptr);

        misc.add (v);
    }

    //==============================================================================
    void removeMismatchedXcuserdata() const
    {
        if (shouldKeepCustomXcodeSchemes())
            return;

        auto xcuserdata = getProjectBundle().getChildFile ("xcuserdata");

        if (! xcuserdata.exists())
            return;

        if (! xcuserdataMatchesTargets (xcuserdata))
        {
            xcuserdata.deleteRecursively();
            getProjectBundle().getChildFile ("xcshareddata").getChildFile ("xcschemes").deleteRecursively();
            getProjectBundle().getChildFile ("project.xcworkspace").deleteRecursively();
        }
    }

    bool xcuserdataMatchesTargets (const File& xcuserdata) const
    {
        for (auto& plist : xcuserdata.findChildFiles (File::findFiles, true, "xcschememanagement.plist"))
            if (! xcschemeManagementPlistMatchesTargets (plist))
                return false;

        return true;
    }

    static StringArray parseNamesOfTargetsFromPlist (const XmlElement& dictXML)
    {
        forEachXmlChildElementWithTagName (dictXML, schemesKey, "key")
        {
            if (schemesKey->getAllSubText().trim().equalsIgnoreCase ("SchemeUserState"))
            {
                if (auto* dict = schemesKey->getNextElement())
                {
                    if (dict->hasTagName ("dict"))
                    {
                        StringArray names;

                        forEachXmlChildElementWithTagName (*dict, key, "key")
                            names.add (key->getAllSubText().upToLastOccurrenceOf (".xcscheme", false, false).trim());

                        names.sort (false);
                        return names;
                    }
                }
            }
        }

        return {};
    }

    StringArray getNamesOfTargets() const
    {
        StringArray names;

        for (auto& target : targets)
            names.add (target->getXcodeSchemeName());

        names.sort (false);
        return names;
    }

    bool xcschemeManagementPlistMatchesTargets (const File& plist) const
    {
        if (auto xml = parseXML (plist))
            if (auto* dict = xml->getChildByName ("dict"))
                return parseNamesOfTargetsFromPlist (*dict) == getNamesOfTargets();

        return false;
    }

    //==============================================================================
    struct AppIconType
    {
        const char* idiom;
        const char* sizeString;
        const char* filename;
        const char* scale;
        int size;
    };

    static Array<AppIconType> getiOSAppIconTypes()
    {
        AppIconType types[] =
        {
            { "iphone",          "20x20",     "Icon-Notification-20@2x.png",       "2x", 40   },
            { "iphone",          "20x20",     "Icon-Notification-20@3x.png",       "3x", 60   },
            { "iphone",          "29x29",     "Icon-29.png",                       "1x", 29   },
            { "iphone",          "29x29",     "Icon-29@2x.png",                    "2x", 58   },
            { "iphone",          "29x29",     "Icon-29@3x.png",                    "3x", 87   },
            { "iphone",          "40x40",     "Icon-Spotlight-40@2x.png",          "2x", 80   },
            { "iphone",          "40x40",     "Icon-Spotlight-40@3x.png",          "3x", 120  },
            { "iphone",          "57x57",     "Icon.png",                          "1x", 57   },
            { "iphone",          "57x57",     "Icon@2x.png",                       "2x", 114  },
            { "iphone",          "60x60",     "Icon-60@2x.png",                    "2x", 120  },
            { "iphone",          "60x60",     "Icon-@3x.png",                      "3x", 180  },
            { "ipad",            "20x20",     "Icon-Notifications-20.png",         "1x", 20   },
            { "ipad",            "20x20",     "Icon-Notifications-20@2x.png",      "2x", 40   },
            { "ipad",            "29x29",     "Icon-Small-1.png",                  "1x", 29   },
            { "ipad",            "29x29",     "Icon-Small@2x-1.png",               "2x", 58   },
            { "ipad",            "40x40",     "Icon-Spotlight-40.png",             "1x", 40   },
            { "ipad",            "40x40",     "Icon-Spotlight-40@2x-1.png",        "2x", 80   },
            { "ipad",            "50x50",     "Icon-Small-50.png",                 "1x", 50   },
            { "ipad",            "50x50",     "Icon-Small-50@2x.png",              "2x", 100  },
            { "ipad",            "72x72",     "Icon-72.png",                       "1x", 72   },
            { "ipad",            "72x72",     "Icon-72@2x.png",                    "2x", 144  },
            { "ipad",            "76x76",     "Icon-76.png",                       "1x", 76   },
            { "ipad",            "76x76",     "Icon-76@2x.png",                    "2x", 152  },
            { "ipad",            "83.5x83.5", "Icon-83.5@2x.png",                  "2x", 167  },
            { "ios-marketing",   "1024x1024", "Icon-AppStore-1024.png",            "1x", 1024 }
        };

        return Array<AppIconType> (types, numElementsInArray (types));
    }

    static String getiOSAppIconContents()
    {
        var images;

        for (auto& type : getiOSAppIconTypes())
        {
            DynamicObject::Ptr d (new DynamicObject());
            d->setProperty ("idiom",    type.idiom);
            d->setProperty ("size",     type.sizeString);
            d->setProperty ("filename", type.filename);
            d->setProperty ("scale",    type.scale);
            images.append (var (d.get()));
        }

        return getiOSAssetContents (images);
    }

    String getProjectObjectAttributes() const
    {
        String attributes;

        attributes << "{ LastUpgradeCheck = 1100; "
                   << "ORGANIZATIONNAME = " << getProject().getCompanyNameString().quoted()
                   <<"; ";

        if (projectType.isGUIApplication() || projectType.isAudioPlugin())
        {
            attributes << "TargetAttributes = { ";

            for (auto& target : targets)
                attributes << target->getTargetAttributes();

            attributes << " }; ";
        }

        attributes << "}";

        return attributes;
    }

    //==============================================================================
    struct ImageType
    {
        const char* orientation;
        const char* idiom;
        const char* subtype;
        const char* extent;
        const char* scale;
        const char* filename;
        int width;
        int height;
    };

    static Array<ImageType> getiOSLaunchImageTypes()
    {
        ImageType types[] =
        {
            { "portrait", "iphone", nullptr,      "full-screen", "2x", "LaunchImage-iphone-2x.png",         640, 960 },
            { "portrait", "iphone", "retina4",    "full-screen", "2x", "LaunchImage-iphone-retina4.png",    640, 1136 },
            { "portrait", "ipad",   nullptr,      "full-screen", "1x", "LaunchImage-ipad-portrait-1x.png",  768, 1024 },
            { "landscape","ipad",   nullptr,      "full-screen", "1x", "LaunchImage-ipad-landscape-1x.png", 1024, 768 },
            { "portrait", "ipad",   nullptr,      "full-screen", "2x", "LaunchImage-ipad-portrait-2x.png",  1536, 2048 },
            { "landscape","ipad",   nullptr,      "full-screen", "2x", "LaunchImage-ipad-landscape-2x.png", 2048, 1536 }
        };

        return Array<ImageType> (types, numElementsInArray (types));
    }

    static String getiOSLaunchImageContents()
    {
        var images;

        for (auto& type : getiOSLaunchImageTypes())
        {
            DynamicObject::Ptr d (new DynamicObject());
            d->setProperty ("orientation", type.orientation);
            d->setProperty ("idiom", type.idiom);
            d->setProperty ("extent",  type.extent);
            d->setProperty ("minimum-system-version", "7.0");
            d->setProperty ("scale", type.scale);
            d->setProperty ("filename", type.filename);

            if (type.subtype != nullptr)
                d->setProperty ("subtype", type.subtype);

            images.append (var (d.get()));
        }

        return getiOSAssetContents (images);
    }

    static void createiOSLaunchImageFiles (const File& launchImageSet)
    {
        for (auto& type : getiOSLaunchImageTypes())
        {
            Image image (Image::ARGB, type.width, type.height, true); // (empty black image)
            image.clear (image.getBounds(), Colours::black);

            MemoryOutputStream pngData;
            PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (image, pngData);
            overwriteFileIfDifferentOrThrow (launchImageSet.getChildFile (type.filename), pngData);
        }
    }

    //==============================================================================
    static String getiOSAssetContents (var images)
    {
        DynamicObject::Ptr v (new DynamicObject());

        var info (new DynamicObject());
        info.getDynamicObject()->setProperty ("version", 1);
        info.getDynamicObject()->setProperty ("author", "xcode");

        v->setProperty ("images", images);
        v->setProperty ("info", info);

        return JSON::toString (var (v.get()));
    }

    void writeDefaultLaunchStoryboardFile() const
    {
        auto storyboardFile = getTargetFolder().getChildFile (getDefaultLaunchStoryboardName() + ".storyboard");

        MemoryOutputStream mo;
        mo.setNewLineString ("\n");

        mo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                                                  << newLine
           << "<document type=\"com.apple.InterfaceBuilder3.CocoaTouch.Storyboard.XIB\" version=\"3.0\" toolsVersion=\"14460.31\" "
           << "targetRuntime=\"iOS.CocoaTouch\" propertyAccessControl=\"none\" useAutolayout=\"YES\" launchScreen=\"YES\" useTraitCollections=\"YES\" "
           << "useSafeAreas=\"YES\" colorMatched=\"YES\" initialViewController=\"01J-lp-oVM\">"                                                                             << newLine
           << "    <scenes>"                                                                                                                                                << newLine
           << "        <scene sceneID=\"EHf-IW-A2E\">"                                                                                                                      << newLine
           << "            <objects>"                                                                                                                                       << newLine
           << "                <placeholder placeholderIdentifier=\"IBFirstResponder\" id=\"iYj-Kq-Ea1\" userLabel=\"\" sceneMemberID=\"firstResponder\"/>"                 << newLine
           << "                <viewController id=\"01J-lp-oVM\" sceneMemberID=\"viewController\">"                                                                         << newLine
           << "                    <view key=\"view\" contentMode=\"scaleToFill\" id=\"Ze5-6b-2t3\">"                                                                       << newLine
           << "                        <autoresizingMask key=\"autoresizingMask\"/>"                                                                                        << newLine
           << "                        <color key=\"backgroundColor\" red=\"0\" green=\"0\" blue=\"0\" alpha=\"1\" colorSpace=\"custom\" customColorSpace=\"sRGB\"/>"       << newLine
           << "                    </view>"                                                                                                                                 << newLine
           << "                </viewController>"                                                                                                                           << newLine
           << "            </objects>"                                                                                                                                      << newLine
           << "        </scene>"                                                                                                                                            << newLine
           << "    </scenes>"                                                                                                                                               << newLine
           << "</document>"                                                                                                                                                 << newLine;

        overwriteFileIfDifferentOrThrow (storyboardFile, mo);

        addLaunchStoryboardFileReference (RelativePath (storyboardFile, getTargetFolder(), RelativePath::buildTargetFolder).toUnixStyle());
    }

    void addLaunchStoryboardFileReference (const String& relativePath) const
    {
        auto refID  = addFileReference (relativePath);
        auto fileID = addBuildFile (relativePath, refID, false, false);

        resourceIDs.add (fileID);
        resourceFileRefs.add (refID);
    }

    void createXcassetsFolderFromIcons() const
    {
        auto assets = getTargetFolder().getChildFile (project.getProjectFilenameRootString())
                                       .getChildFile ("Images.xcassets");
        auto iconSet = assets.getChildFile ("AppIcon.appiconset");
        auto launchImage = assets.getChildFile ("LaunchImage.launchimage");

        overwriteFileIfDifferentOrThrow (iconSet.getChildFile ("Contents.json"), getiOSAppIconContents());
        createiOSIconFiles (iconSet);

        overwriteFileIfDifferentOrThrow (launchImage.getChildFile ("Contents.json"), getiOSLaunchImageContents());
        createiOSLaunchImageFiles (launchImage);

        RelativePath assetsPath (assets, getTargetFolder(), RelativePath::buildTargetFolder);
        addFileReference (assetsPath.toUnixStyle());
        resourceIDs.add (addBuildFile (assetsPath, false, false));
        resourceFileRefs.add (createFileRefID (assetsPath));
    }

    //==============================================================================
    static String indentBracedList        (const StringArray& list, int depth = 0) { return indentList (list, '{', '}', ";", depth, true); }
    static String indentParenthesisedList (const StringArray& list, int depth = 0) { return indentList (list, '(', ')', ",", depth, false); }

    static String indentList (StringArray list, char openBracket, char closeBracket, const String& separator, int extraTabs, bool shouldSort)
    {
        if (list.size() == 0)
            return openBracket + String (" ") + closeBracket;

        auto tabs = "\n" + String::repeatedString ("\t", extraTabs + 4);

        if (shouldSort)
            list.sort (true);

        return openBracket + tabs + list.joinIntoString (separator + tabs) + separator
                   + "\n" + String::repeatedString ("\t", extraTabs + 3) + closeBracket;
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        rootString += project.getProjectUIDString();

        return MD5 (rootString.toUTF8()).toHexString().substring (0, 24).toUpperCase();
    }

    String createFileRefID (const RelativePath& path) const     { return createFileRefID (path.toUnixStyle()); }
    String createFileRefID (const String& path) const           { return createID ("__fileref_" + path); }
    String getIDForGroup (const Project::Item& item) const      { return createID (item.getID()); }

    bool shouldFileBeCompiledByDefault (const File& file) const override
    {
        return file.hasFileExtension (sourceFileExtensions);
    }

    JUCE_DECLARE_NON_COPYABLE (XcodeProjectExporter)
};
