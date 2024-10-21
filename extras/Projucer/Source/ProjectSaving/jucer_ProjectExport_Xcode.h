/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_XcodeProjectParser.h"

//==============================================================================
constexpr auto* macOSArch_Default        = "default";
constexpr auto* macOSArch_Native         = "Native";
constexpr auto* macOSArch_32BitUniversal = "32BitUniversal";
constexpr auto* macOSArch_64BitUniversal = "64BitUniversal";
constexpr auto* macOSArch_64Bit          = "64BitIntel";

//==============================================================================
inline String doubleQuoted (const String& text)
{
    return text.quoted();
}

inline String singleQuoted (const String& text)
{
    return text.quoted ('\'');
}

//==============================================================================
class ScriptBuilder
{
public:
    //==============================================================================
    ScriptBuilder() = default;
    explicit ScriptBuilder (int indentIn) : indent (indentIn) {}

    //==============================================================================
    template <typename... Args>
    ScriptBuilder& run (const String& command, Args&&... args)
    {
        const auto joined = StringArray { command, std::forward<Args> (args)... }.joinIntoString (" ");
        return echo ("Running " + joined).insertLine (joined);
    }

    ScriptBuilder& echo (const String& text)
    {
        return insertLine ("echo " + text.replace ("\"", "\\\""));
    }

    ScriptBuilder& remove (const String& path)
    {
        return run ("rm -rf", doubleQuoted (path));
    }

    ScriptBuilder& copy (const String& src, const String& dst)
    {
        return run ("ditto", doubleQuoted (src), doubleQuoted (dst));
    }

    ScriptBuilder& set (const String& variableName, const String& defaultValue = singleQuoted (""))
    {
        return insertLine (variableName + "=" + doubleQuoted (defaultValue));
    }

    //==============================================================================
    ScriptBuilder& ifThen (const String& condition, const String& then)
    {
        jassert (then.isNotEmpty());
        return insertLine ("if [[ " + condition + " ]]; then")
              .insertScript (ScriptBuilder { indent + 1 }.insertScript (then).toString())
              .insertLine ("fi")
              .insertLine();
    }

    ScriptBuilder& ifCompare (const String& lhs, const String& rhs, const String& comparison, const String& then)
    {
        return ifThen (StringArray { doubleQuoted (lhs), comparison, doubleQuoted (rhs) }.joinIntoString (" "), then);
    }

    ScriptBuilder& ifEqual (const String& lhs, const String& rhs, const String& then)
    {
        return ifCompare (lhs, rhs, "==", then);
    }

    ScriptBuilder& ifSet (const String& variable, const String& then)
    {
        return ifThen ("-n " + doubleQuoted ("${" + variable + "-}"), then);
    }

    //==============================================================================
    ScriptBuilder& insertLine (const String& line = {})
    {
        constexpr auto spacesPerIndent = 2;
        script.add ((String::repeatedString (" ", spacesPerIndent * indent) + line).trimEnd());
        return *this;
    }

    ScriptBuilder& insertLines (const StringArray& lines)
    {
        for (const auto& line : lines)
            insertLine (line);

        return *this;
    }

    ScriptBuilder& insertScript (const String& s)
    {
        return insertLines (StringArray::fromLines (s.trimEnd()));
    }

    //==============================================================================
    bool isEmpty() const
    {
        return script.isEmpty();
    }

    String toString() const
    {
        return script.joinIntoString ("\n") + "\n";
    }

    String toStringWithShellOptions (const String& options) const
    {
        if (isEmpty())
            return {};

        return ScriptBuilder{}.insertLine ("set " + options)
                              .insertLine()
                              .insertScript (toString())
                              .toString();
    }

    String toStringWithDefaultShellOptions() const
    {
        return toStringWithShellOptions ("-euo pipefail");
    }

private:
    StringArray script;
    int indent{};
};

//==============================================================================
class XcodeProjectExporter final : public ProjectExporter,
                                   private MessageBoxQueue::Listener
{
public:
    //==============================================================================
    static String getDisplayNameMac()        { return "Xcode (macOS)"; }
    static String getDisplayNameiOS()        { return "Xcode (iOS)"; }

    static String getTargetFolderNameMac()   { return "MacOSX"; }
    static String getTargetFolderNameiOS()   { return "iOS"; }

    static String getValueTreeTypeNameMac()  { return "XCODE_MAC"; }
    static String getValueTreeTypeNameiOS()  { return "XCODE_IPHONE"; }

    //==============================================================================
    XcodeProjectExporter (Project& p, const ValueTree& t, const bool isIOS)
        : ProjectExporter (p, t),
          xcodeCanUseDwarf (true),
          iOS (isIOS),
          applicationCategoryValue                     (settings, Ids::applicationCategory,                     getUndoManager(), ""),
          customPListValue                             (settings, Ids::customPList,                             getUndoManager()),
          pListPrefixHeaderValue                       (settings, Ids::pListPrefixHeader,                       getUndoManager()),
          pListPreprocessValue                         (settings, Ids::pListPreprocess,                         getUndoManager()),
          subprojectsValue                             (settings, Ids::xcodeSubprojects,                        getUndoManager()),
          validArchsValue                              (settings, Ids::xcodeValidArchs,                         getUndoManager(), getAllArchs(), ","),
          extraFrameworksValue                         (settings, Ids::extraFrameworks,                         getUndoManager()),
          frameworkSearchPathsValue                    (settings, Ids::frameworkSearchPaths,                    getUndoManager()),
          extraCustomFrameworksValue                   (settings, Ids::extraCustomFrameworks,                   getUndoManager()),
          embeddedFrameworksValue                      (settings, Ids::embeddedFrameworks,                      getUndoManager()),
          postbuildCommandValue                        (settings, Ids::postbuildCommand,                        getUndoManager()),
          prebuildCommandValue                         (settings, Ids::prebuildCommand,                         getUndoManager()),
          duplicateAppExResourcesFolderValue           (settings, Ids::duplicateAppExResourcesFolder,           getUndoManager(), true),
          iosDeviceFamilyValue                         (settings, Ids::iosDeviceFamily,                         getUndoManager(), "1,2"),
          iPhoneScreenOrientationValue                 (settings, Ids::iPhoneScreenOrientation,                 getUndoManager(), getDefaultScreenOrientations(), ","),
          iPadScreenOrientationValue                   (settings, Ids::iPadScreenOrientation,                   getUndoManager(), getDefaultScreenOrientations(), ","),
          customXcodeResourceFoldersValue              (settings, Ids::customXcodeResourceFolders,              getUndoManager()),
          customXcassetsFolderValue                    (settings, Ids::customXcassetsFolder,                    getUndoManager()),
          appSandboxValue                              (settings, Ids::appSandbox,                              getUndoManager()),
          appSandboxInheritanceValue                   (settings, Ids::appSandboxInheritance,                   getUndoManager()),
          appSandboxOptionsValue                       (settings, Ids::appSandboxOptions,                       getUndoManager(), Array<var>(), ","),
          appSandboxHomeDirROValue                     (settings, Ids::appSandboxHomeDirRO,                     getUndoManager()),
          appSandboxHomeDirRWValue                     (settings, Ids::appSandboxHomeDirRW,                     getUndoManager()),
          appSandboxAbsDirROValue                      (settings, Ids::appSandboxAbsDirRO,                      getUndoManager()),
          appSandboxAbsDirRWValue                      (settings, Ids::appSandboxAbsDirRW,                      getUndoManager()),
          appSandboxExceptionIOKitValue                (settings, Ids::appSandboxExceptionIOKit,                getUndoManager()),
          hardenedRuntimeValue                         (settings, Ids::hardenedRuntime,                         getUndoManager()),
          hardenedRuntimeOptionsValue                  (settings, Ids::hardenedRuntimeOptions,                  getUndoManager(), Array<var>(), ","),
          microphonePermissionNeededValue              (settings, Ids::microphonePermissionNeeded,              getUndoManager()),
          microphonePermissionsTextValue               (settings, Ids::microphonePermissionsText,               getUndoManager(),
                                                        "This app requires audio input. If you do not have an audio interface connected it will use the built-in microphone."),
          cameraPermissionNeededValue                  (settings, Ids::cameraPermissionNeeded,                  getUndoManager()),
          cameraPermissionTextValue                    (settings, Ids::cameraPermissionText,                    getUndoManager(),
                                                        "This app requires access to the camera to function correctly."),
          bluetoothPermissionNeededValue               (settings, Ids::iosBluetoothPermissionNeeded,            getUndoManager()),
          bluetoothPermissionTextValue                 (settings, Ids::iosBluetoothPermissionText,              getUndoManager(),
                                                        "This app requires access to Bluetooth to function correctly."),
          sendAppleEventsPermissionNeededValue         (settings, Ids::sendAppleEventsPermissionNeeded, getUndoManager()),
          sendAppleEventsPermissionTextValue           (settings, Ids::sendAppleEventsPermissionText, getUndoManager(),
                                                        "This app requires the ability to send Apple events to function correctly."),
          uiFileSharingEnabledValue                    (settings, Ids::UIFileSharingEnabled,                    getUndoManager()),
          uiSupportsDocumentBrowserValue               (settings, Ids::UISupportsDocumentBrowser,               getUndoManager()),
          uiStatusBarHiddenValue                       (settings, Ids::UIStatusBarHidden,                       getUndoManager()),
          uiRequiresFullScreenValue                    (settings, Ids::UIRequiresFullScreen,                    getUndoManager(), true),
          documentExtensionsValue                      (settings, Ids::documentExtensions,                      getUndoManager()),
          iosInAppPurchasesValue                       (settings, Ids::iosInAppPurchases,                       getUndoManager()),
          iosContentSharingValue                       (settings, Ids::iosContentSharing,                       getUndoManager(), true),
          iosBackgroundAudioValue                      (settings, Ids::iosBackgroundAudio,                      getUndoManager()),
          iosBackgroundBleValue                        (settings, Ids::iosBackgroundBle,                        getUndoManager()),
          iosPushNotificationsValue                    (settings, Ids::iosPushNotifications,                    getUndoManager()),
          iosAppGroupsValue                            (settings, Ids::iosAppGroups,                            getUndoManager()),
          iCloudPermissionsValue                       (settings, Ids::iCloudPermissions,                       getUndoManager()),
          networkingMulticastValue                     (settings, Ids::networkingMulticast,                     getUndoManager()),
          iosDevelopmentTeamIDValue                    (settings, Ids::iosDevelopmentTeamID,                    getUndoManager()),
          iosAppGroupsIDValue                          (settings, Ids::iosAppGroupsId,                          getUndoManager()),
          keepCustomXcodeSchemesValue                  (settings, Ids::keepCustomXcodeSchemes,                  getUndoManager()),
          useHeaderMapValue                            (settings, Ids::useHeaderMap,                            getUndoManager()),
          customLaunchStoryboardValue                  (settings, Ids::customLaunchStoryboard,                  getUndoManager()),
          exporterBundleIdentifierValue                (settings, Ids::bundleIdentifier,                        getUndoManager()),
          suppressPlistResourceUsageValue              (settings, Ids::suppressPlistResourceUsage,              getUndoManager()),
          useLegacyBuildSystemValue                    (settings, Ids::useLegacyBuildSystem,                    getUndoManager()),
          buildNumber                                  (settings, Ids::buildNumber,                             getUndoManager())
    {
        if (iOS)
        {
            name = getDisplayNameiOS();
            targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderNameiOS());
        }
        else
        {
            name = getDisplayNameMac();
            targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderNameMac());
        }

        if (needsDisplayMessageBox())
        {
            messageBoxQueueListenerScope = project.messageBoxQueue.addListener (*this);
        }
    }

    static XcodeProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeNameMac()))  return new XcodeProjectExporter (projectToUse, settingsToUse, false);
        if (settingsToUse.hasType (getValueTreeTypeNameiOS()))  return new XcodeProjectExporter (projectToUse, settingsToUse, true);

        return nullptr;
    }

    //==============================================================================
    String getApplicationCategoryString() const             { return applicationCategoryValue.get(); }

    String getPListToMergeString() const                    { return customPListValue.get(); }
    String getPListPrefixHeaderString() const               { return pListPrefixHeaderValue.get(); }
    bool isPListPreprocessEnabled() const                   { return pListPreprocessValue.get(); }

    String getSubprojectsString() const                     { return subprojectsValue.get(); }

    String getExtraFrameworksString() const                 { return extraFrameworksValue.get(); }
    String getFrameworkSearchPathsString() const            { return frameworkSearchPathsValue.get(); }
    String getExtraCustomFrameworksString() const           { return extraCustomFrameworksValue.get(); }
    String getEmbeddedFrameworksString() const              { return embeddedFrameworksValue.get(); }

    String getPostBuildScript() const                       { return postbuildCommandValue.get(); }
    String getPreBuildScript() const                        { return prebuildCommandValue.get(); }

    bool shouldDuplicateAppExResourcesFolder() const        { return duplicateAppExResourcesFolderValue.get(); }

    String getDeviceFamilyString() const                    { return iosDeviceFamilyValue.get(); }

    Array<var> getDefaultScreenOrientations() const         { return { "UIInterfaceOrientationPortrait",
                                                                       "UIInterfaceOrientationLandscapeLeft",
                                                                       "UIInterfaceOrientationLandscapeRight" }; }

    Array<var> getAllArchs() const                          { return { "i386", "x86_64", "arm64", "arm64e"}; }

    Array<var> getiPhoneScreenOrientations() const          { return *iPhoneScreenOrientationValue.get().getArray(); }
    Array<var> getiPadScreenOrientations() const            { return *iPadScreenOrientationValue.get().getArray(); }

    String getCustomResourceFoldersString() const           { return customXcodeResourceFoldersValue.get().toString().replaceCharacters ("\r\n", "::"); }
    String getCustomXcassetsFolderString() const            { return customXcassetsFolderValue.get(); }

    Optional<build_tools::RelativePath> getCustomXcassetsFolder() const
    {
        const auto customXcassetsPath = getCustomXcassetsFolderString();

        if (customXcassetsPath.isEmpty())
            return {};

        return build_tools::RelativePath { customXcassetsPath, build_tools::RelativePath::projectFolder };
    }

    String getCustomLaunchStoryboardString() const          { return customLaunchStoryboardValue.get(); }

    bool shouldAddStoryboardToProject() const               { return getCustomLaunchStoryboardString().isNotEmpty()
                                                                  || (! customXcassetsFolderContainsLaunchImage()); }

    bool isHardenedRuntimeEnabled() const                   { return hardenedRuntimeValue.get(); }
    Array<var> getHardenedRuntimeOptions() const            { return *hardenedRuntimeOptionsValue.get().getArray(); }

    bool isAppSandboxEnabled() const                        { return appSandboxValue.get(); }
    bool isAppSandboxInhertianceEnabled() const             { return appSandboxInheritanceValue.get(); }
    Array<var> getAppSandboxOptions() const                 { return *appSandboxOptionsValue.get().getArray(); }

    auto getAppSandboxTemporaryPaths() const
    {
        std::vector<build_tools::EntitlementOptions::KeyAndStringArray> result;

        for (const auto& entry : sandboxFileAccessProperties)
        {
            auto paths = getCommaOrWhitespaceSeparatedItems (entry.property.get());

            if (! paths.isEmpty())
                result.push_back ({ "com.apple.security.temporary-exception.files." + entry.key, std::move (paths) });
        }

        return result;
    }

    StringArray getAppSandboxExceptionIOKitClasses() const
    {
        return getCommaOrWhitespaceSeparatedItems (appSandboxExceptionIOKitValue.get());
    }

    Array<var> getValidArchs() const                        { return *validArchsValue.get().getArray(); }

    bool isMicrophonePermissionEnabled() const              { return microphonePermissionNeededValue.get(); }
    String getMicrophonePermissionsTextString() const       { return microphonePermissionsTextValue.get(); }

    bool isCameraPermissionEnabled() const                  { return cameraPermissionNeededValue.get(); }
    String getCameraPermissionTextString() const            { return cameraPermissionTextValue.get(); }

    bool isBluetoothPermissionEnabled() const               { return bluetoothPermissionNeededValue.get(); }
    String getBluetoothPermissionTextString() const         { return bluetoothPermissionTextValue.get(); }

    bool isSendAppleEventsPermissionEnabled() const         { return sendAppleEventsPermissionNeededValue.get(); }
    String getSendAppleEventsPermissionTextString() const   { return sendAppleEventsPermissionTextValue.get(); }

    bool isInAppPurchasesEnabled() const                    { return iosInAppPurchasesValue.get(); }
    bool isContentSharingEnabled() const                    { return iosContentSharingValue.get(); }
    bool isBackgroundAudioEnabled() const                   { return iosBackgroundAudioValue.get(); }
    bool isBackgroundBleEnabled() const                     { return iosBackgroundBleValue.get(); }
    bool isPushNotificationsEnabled() const                 { return iosPushNotificationsValue.get(); }
    bool isAppGroupsEnabled() const                         { return iosAppGroupsValue.get(); }
    bool isiCloudPermissionsEnabled() const                 { return iCloudPermissionsValue.get(); }
    bool isNetworkingMulticastEnabled() const               { return networkingMulticastValue.get(); }
    bool isFileSharingEnabled() const                       { return uiFileSharingEnabledValue.get(); }
    bool isDocumentBrowserEnabled() const                   { return uiSupportsDocumentBrowserValue.get(); }
    bool isStatusBarHidden() const                          { return uiStatusBarHiddenValue.get(); }
    bool requiresFullScreen() const                         { return uiRequiresFullScreenValue.get(); }

    bool getSuppressPlistResourceUsage() const              { return suppressPlistResourceUsageValue.get(); }

    bool shouldUseLegacyBuildSystem() const                 { return useLegacyBuildSystemValue.get(); }

    String getDocumentExtensionsString() const              { return documentExtensionsValue.get(); }

    bool shouldKeepCustomXcodeSchemes() const               { return keepCustomXcodeSchemesValue.get(); }

    String getDevelopmentTeamIDString() const               { return iosDevelopmentTeamIDValue.get(); }
    String getAppGroupIdString() const                      { return iosAppGroupsIDValue.get(); }

    String getBuildNumber() const
    {
        const auto buildNumberString = buildNumber.get().toString();
        return buildNumberString.isNotEmpty() ? buildNumberString : project.getVersionString();
    }

    String getDefaultLaunchStoryboardName() const           { return "LaunchScreen"; }

    //==============================================================================
    bool usesMMFiles() const override                       { return true; }
    bool canCopeWithDuplicateFiles() override               { return true; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                           { return true; }
    bool isVisualStudio() const override                    { return false; }
    bool isMakefile() const override                        { return false; }
    bool isAndroidStudio() const override                   { return false; }

    bool isAndroid() const override                         { return false; }
    bool isWindows() const override                         { return false; }
    bool isLinux() const override                           { return false; }
    bool isOSX() const override                             { return ! iOS; }
    bool isiOS() const override                             { return iOS; }

    Identifier getExporterIdentifier() const override
    {
        return iOS ? getValueTreeTypeNameiOS() : getValueTreeTypeNameMac();
    }

    bool supportsPrecompiledHeaders() const override        { return true; }

    String getNewLineString() const override                { return "\n"; }

    bool supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        using Target = build_tools::ProjectType::Target;

        switch (type)
        {
            case Target::AudioUnitv3PlugIn:
            case Target::StandalonePlugIn:
            case Target::GUIApp:
            case Target::StaticLibrary:
            case Target::DynamicLibrary:
            case Target::SharedCodeTarget:
            case Target::AggregateTarget:
                return true;
            case Target::ConsoleApp:
            case Target::VSTPlugIn:
            case Target::VST3PlugIn:
            case Target::AAXPlugIn:
            case Target::AudioUnitPlugIn:
            case Target::UnityPlugIn:
            case Target::LV2PlugIn:
            case Target::LV2Helper:
            case Target::VST3Helper:
                return ! iOS;
            case Target::unspecified:
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
                       "for the app icons, and will ignore the Icon files specified above. If the provided xcassets folder "
                       "contains a launchimage it will be used, unless a custom storyboard is specified.");

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

        if (getProject().isAudioPluginProject())
            props.add (new ChoicePropertyComponent (duplicateAppExResourcesFolderValue, "Add Duplicate Resources Folder to App Extension"),
                       "Disable this to prevent the Projucer from creating a duplicate resources folder for AUv3 app extensions.");

        props.add (new TextPropertyComponent (buildNumber, "Build Number", 128, false),
                   "The current version of the project. Used to disambiguate different builds of the same project on App Store Connect. "
                   "If this field is empty, the project's version will be used as the build number. "
                   "For more details about the difference between the project version and build version, see developer.apple.com/library/archive/technotes/tn2420/_index.html");

        if (iOS)
        {
            props.add (new ChoicePropertyComponent (iosDeviceFamilyValue, "Device Family",
                                                    { "iPhone", "iPad", "Universal" },
                                                    { "1",      "2",    "1,2" }),
                       "The device family to target.");

            {
                StringArray orientationStrings { "Portrait", "Portrait Upside Down",
                                                 "Landscape Left", "Landscape Right" };

                Array<var> orientationVars { "UIInterfaceOrientationPortrait", "UIInterfaceOrientationPortraitUpsideDown",
                                             "UIInterfaceOrientationLandscapeLeft", "UIInterfaceOrientationLandscapeRight" };

                props.add (new MultiChoicePropertyComponent (iPhoneScreenOrientationValue, "iPhone Screen Orientation", orientationStrings, orientationVars),
                           "The screen orientations that this app should support on iPhones.");

                props.add (new MultiChoicePropertyComponent (iPadScreenOrientationValue, "iPad Screen Orientation", orientationStrings, orientationVars),
                           "The screen orientations that this app should support on iPads.");
            }

            props.add (new ChoicePropertyComponent (uiFileSharingEnabledValue, "File Sharing Enabled"),
                       "Enable this to expose your app's files to iTunes.");

            props.add (new ChoicePropertyComponent (uiSupportsDocumentBrowserValue, "Support Document Browser"),
                       "Enable this to allow the user to access your app documents from a native file chooser.");

            props.add (new ChoicePropertyComponent (uiStatusBarHiddenValue, "Status Bar Hidden"),
                       "Enable this to disable the status bar in your app.");

            props.add (new ChoicePropertyComponent (uiRequiresFullScreenValue, "Requires Full Screen"),
                       "Disable this to enable non-fullscreen views such as Slide Over or Split View in your app. "
                       "You will also need to enable all orientations.");
        }
        else if (projectType.isGUIApplication())
        {
            props.add (new TextPropertyComponent (documentExtensionsValue, "Document File Extensions", 128, false),
                       "A comma-separated list of file extensions for documents that your app can open. "
                       "Using a leading '.' is optional, and the extensions are not case-sensitive.");
        }

        props.add (new ChoicePropertyComponent (useLegacyBuildSystemValue, "Use Legacy Build System"),
                   "Enable this to use the deprecated \"Legacy Build System\" in Xcode 10 and above. "
                   "This may fix build issues that were introduced with the new build system in Xcode 10 and subsequently fixed in Xcode 10.2, "
                   "however the new build system is recommended for apps targeting Apple silicon.");

        if (isOSX())
        {
            std::vector<std::pair<String, String>> appCategories
            {
                { "None",                 "" },
                { "Business",             "business" },
                { "Developer Tools",      "developer-tools" },
                { "Education",            "education" },
                { "Entertainment",        "entertainment" },
                { "Finance",              "finance" },
                { "Games",                "games" },
                { "Games - Action",       "action-games" },
                { "Games - Adventure",    "adventure-games" },
                { "Games - Arcade",       "arcade-games" },
                { "Games - Board",        "board-games" },
                { "Games - Card",         "card-games" },
                { "Games - Casino",       "casino-games" },
                { "Games - Dice",         "dice-games" },
                { "Games - Educational",  "educational-games" },
                { "Games - Family",       "family-games" },
                { "Games - Kids",         "kids-games" },
                { "Games - Music",        "music-games" },
                { "Games - Puzzle",       "puzzle-games" },
                { "Games - Racing",       "racing-games" },
                { "Games - Role Playing", "role-playing-games" },
                { "Games - Simulation",   "simulation-games" },
                { "Games - Sports",       "sports-games" },
                { "Games - Strategy",     "strategy-games" },
                { "Games - Trivia",       "trivia-games" },
                { "Games - Word",         "word-games" },
                { "Graphics Design",      "graphics-design" },
                { "Healthcare & Fitness", "healthcare-fitness" },
                { "Lifestyle",            "lifestyle" },
                { "Medial",               "medical" },
                { "Music",                "music" },
                { "News",                 "news" },
                { "Photography",          "photography" },
                { "Productivity",         "productivity" },
                { "Reference",            "reference" },
                { "Social Networking",    "social-networking" },
                { "Sports",               "sports" },
                { "Travel",               "travel" },
                { "Utilities",            "utilities" },
                { "Video",                "video" },
                { "Weather" ,             "weather" }
            };

            StringArray appCategoryKeys;
            Array<var> appCategoryValues;

            for (auto& opt : appCategories)
            {
                appCategoryKeys.add (opt.first);

                if (opt.second.isNotEmpty())
                    appCategoryValues.add ("public.app-category." + opt.second);
                else
                    appCategoryValues.add ("");
            }

            props.add (new ChoicePropertyComponent (applicationCategoryValue,
                                                    "App Category",
                                                    appCategoryKeys,
                                                    appCategoryValues),
                       "The application category.");

            props.add (new MultiChoicePropertyComponent (validArchsValue, "Valid Architectures", getAllArchs(), getAllArchs()),
                       "The full set of architectures which this project may target. "
                       "Each configuration will build for the intersection of this property, and the per-configuration macOS Architecture property");

            props.add (new ChoicePropertyComponent (appSandboxValue, "Use App Sandbox"),
                       "Enable this to use the app sandbox.");

            props.add (new ChoicePropertyComponentWithEnablement (appSandboxInheritanceValue, appSandboxValue, "App Sandbox Inheritance"),
                       "If app sandbox is enabled, this setting will configure a child process to inherit the sandbox of its parent. "
                       "Note that if you enable this and have specified any other app sandbox entitlements below, the child process "
                       "will fail to launch.");

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
                { "File Access: Movies Folder (Read/Write)",        "assets.movies.read-write" },

                { "Temporary Exception: Audio Unit Hosting",                       "temporary-exception.audio-unit-host" },
                { "Temporary Exception: Global Mach Service",                      "temporary-exception.mach-lookup.global-name" },
                { "Temporary Exception: Global Mach Service Dynamic Registration", "temporary-exception.mach-register.global-name" },
                { "Temporary Exception: Shared Preference Domain (Read Only)",     "temporary-exception.shared-preference.read-only" },
                { "Temporary Exception: Shared Preference Domain (Read/Write)",    "temporary-exception.shared-preference.read-write" }
            };

            StringArray sandboxKeys;
            Array<var> sandboxValues;

            for (auto& opt : sandboxOptions)
            {
                sandboxKeys.add (opt.first);
                sandboxValues.add ("com.apple.security." + opt.second);
            }

            props.add (new MultiChoicePropertyComponentWithEnablement (appSandboxOptionsValue,
                                                                       appSandboxValue,
                                                                       "App Sandbox Options",
                                                                       sandboxKeys,
                                                                       sandboxValues));

            for (const auto& entry : sandboxFileAccessProperties)
            {
                props.add (new TextPropertyComponentWithEnablement (entry.property,
                                                                    appSandboxValue,
                                                                    entry.label,
                                                                    8192,
                                                                    true),
                           "A list of the corresponding paths (separated by newlines or whitespace). "
                           "See Apple's File Access Temporary Exceptions documentation.");
            }

            props.add (new TextPropertyComponentWithEnablement (appSandboxExceptionIOKitValue,
                                                                appSandboxValue,
                                                                "App sandbox temporary exception: additional IOUserClient subclasses",
                                                                8192,
                                                                true),
                       "A list of IOUserClient subclasses to open or to set properties on. "
                       "See Apple's IOKit User Client Class Temporary Exception documentation.");

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

        props.add (new ChoicePropertyComponent (bluetoothPermissionNeededValue, "Bluetooth Access"),
                   "Enable this to allow your app to use Bluetooth on iOS 13.0 and above, and macOS 11.0 and above. "
                   "The user of your app will be prompted to grant Bluetooth access permissions.");

        props.add (new TextPropertyComponentWithEnablement (bluetoothPermissionTextValue, bluetoothPermissionNeededValue,
                                                            "Bluetooth Access Text", 1024, false),
                   "A short description of why your app requires Bluetooth access.");

        if (! iOS)
        {
            props.add (new ChoicePropertyComponent (sendAppleEventsPermissionNeededValue, "Send Apple Events"),
                       "Enable this to allow your app to send Apple events. "
                       "The user of your app will be prompted to grant permissions to control other apps.");

            props.add (new TextPropertyComponentWithEnablement (sendAppleEventsPermissionTextValue, sendAppleEventsPermissionNeededValue,
                                                                "Send Apple Events Text", 1024, false),
                       "A short description of why your app requires the ability to send Apple events.");
        }

        props.add (new ChoicePropertyComponent (iosInAppPurchasesValue, "In-App Purchases Capability"),
                   "Enable this to grant your app the capability for in-app purchases. "
                   "This option requires that you specify a valid Development Team ID.");

        if (iOS)
        {
            props.add (new ChoicePropertyComponent (iosContentSharingValue, "Content Sharing"),
                       "Enable this to allow your app to share content with other apps.");

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

        props.add (new ChoicePropertyComponent (networkingMulticastValue, "Networking Multicast Capability"),
                   "Your app must have this entitlement to send or receive IP multicast or broadcast. "
                   "You will also need permission from Apple to use this entitlement.");

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

        props.add (new ChoicePropertyComponent (suppressPlistResourceUsageValue, "Suppress AudioUnit Plist resourceUsage Key"),
                   "Suppress the resourceUsage key in the target's generated Plist. This is useful for AU"
                   " plugins that must access resources which cannot be declared in the resourceUsage block, such"
                   " as UNIX domain sockets. In particular, PACE-protected AU plugins may require this option to be enabled"
                   " in order for the plugin to load in GarageBand.");

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
                   "These can be absolute or relative to the build directory. "
                   "The names of the required build products can be specified after a colon, comma separated, "
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
                   "The Team ID to be used for setting up code-signing for your application. "
                   "This is a ten-character string (for example \"S7B6T5XJ2Q\") that can be found under the \"Organisational Unit\" "
                   "field of your developer certificate in Keychain Access or in the membership page of your account on developer.apple.com.");

        if (iOS)
            props.add (new TextPropertyComponentWithEnablement (iosAppGroupsIDValue, iosAppGroupsValue, "App Group ID", 256, false),
                       "The App Group ID to be used for allowing multiple apps to access a shared resource folder. Multiple IDs can be "
                       "added separated by a semicolon. The App Groups Capability setting must be enabled for this setting to have any effect.");

        props.add (new ChoicePropertyComponent (keepCustomXcodeSchemesValue, "Keep Custom Xcode Schemes"),
                   "Enable this to keep any Xcode schemes you have created for debugging or running, e.g. to launch a plug-in in "
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

        build_tools::writeStreamToFile (projectBundle.getChildFile ("project.pbxproj"),
                                        [this] (MemoryOutputStream& mo) { writeProjectFile (mo); });

        writeInfoPlistFiles();
        writeWorkspaceSettings();

        // Deleting the .rsrc files can be needed to force Xcode to update the version number.
        deleteRsrcFiles (getTargetFolder().getChildFile ("build"));
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) override
    {
        callForAllSupportedTargets ([this] (build_tools::ProjectType::Target::Type targetType)
                                    {
                                        targets.insert (targetType == build_tools::ProjectType::Target::AggregateTarget ? 0 : -1,
                                                        new XcodeTarget (targetType, *this));
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    void updateDeprecatedSettings() override
    {
        if (iOS)
            updateOldOrientationSettings();
    }

    bool hasInvalidPostBuildScript() const
    {
        // check whether the script is identical to the old one that the Introjucer used to auto-generate
        return    ! userAcknowledgedInvalidPostBuildScript
               && (MD5 (getPostBuildScript().toUTF8()).toHexString() == "265ac212a7e734c5bbd6150e1eae18a1");
    }

    bool hasDefunctIOKitSetting() const
    {
        auto v = appSandboxOptionsValue.get();

        if (! v.isArray())
        {
            jassertfalse;
            return false;
        }

        return    ! userAcknowledgedDefunctIOKitSetting
               && v.getArray()->contains ("com.apple.security.temporary-exception.iokit-user-client-class");
    }

    bool needsDisplayMessageBox() const
    {
        return hasInvalidPostBuildScript() || hasDefunctIOKitSetting();
    }

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                        getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::osx), TargetOS::osx);

        aaxPathValueWrapper.init ({ settings, Ids::aaxFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::aaxPath,  TargetOS::osx), TargetOS::osx);

        araPathValueWrapper.init ({ settings, Ids::araFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::araPath, TargetOS::osx), TargetOS::osx);
    }

protected:
    //==============================================================================
    class XcodeBuildConfiguration final : public BuildConfiguration,
                                          private ValueTree::Listener
    {
    public:
        XcodeBuildConfiguration (Project& p, const ValueTree& t, const bool isIOS, const ProjectExporter& e)
            : BuildConfiguration (p, t, e),
              iOS (isIOS),
              macOSBaseSDK                 (config, Ids::macOSBaseSDK,                 getUndoManager()),
              macOSDeploymentTarget        (config, Ids::macOSDeploymentTarget,        getUndoManager(), "10.13"),
              macOSArchitecture            (config, Ids::osxArchitecture,              getUndoManager(), macOSArch_Default),
              iosBaseSDK                   (config, Ids::iosBaseSDK,                   getUndoManager()),
              iosDeploymentTarget          (config, Ids::iosDeploymentTarget,          getUndoManager(), "12.0"),
              customXcodeFlags             (config, Ids::customXcodeFlags,             getUndoManager()),
              plistPreprocessorDefinitions (config, Ids::plistPreprocessorDefinitions, getUndoManager()),
              codeSignIdentity             (config, Ids::codeSigningIdentity,          getUndoManager()),
              fastMathEnabled              (config, Ids::fastMath,                     getUndoManager()),
              stripLocalSymbolsEnabled     (config, Ids::stripLocalSymbols,            getUndoManager()),
              pluginBinaryCopyStepEnabled  (config, Ids::enablePluginBinaryCopyStep,   getUndoManager(), true),
              vstBinaryLocation            (config, Ids::vstBinaryLocation,            getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/VST/"),
              vst3BinaryLocation           (config, Ids::vst3BinaryLocation,           getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/VST3/"),
              auBinaryLocation             (config, Ids::auBinaryLocation,             getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/Components/"),
              aaxBinaryLocation            (config, Ids::aaxBinaryLocation,            getUndoManager(), "/Library/Application Support/Avid/Audio/Plug-Ins/"),
              unityPluginBinaryLocation    (config, Ids::unityPluginBinaryLocation,    getUndoManager()),
              lv2BinaryLocation            (config, Ids::lv2BinaryLocation,            getUndoManager(), "$(HOME)/Library/Audio/Plug-Ins/LV2/")
        {
            updateOldPluginBinaryLocations();
            updateOldSDKDefaults();

            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);

            config.addListener (this);
        }

        //==============================================================================
        void createConfigProperties (PropertyListBuilder& props) override
        {
            if (project.isAudioPluginProject())
                addXcodePluginInstallPathProperties (props);

            addRecommendedLLVMCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            const String sdkInfoString ("\nThis must be in the format major.minor and contain only the numeric version number. "
                                        "If this is left empty then the default will be used."
                                        "\nThe minimum supported version is ");

            if (iOS)
            {
                props.add (new TextPropertyComponent (iosBaseSDK, "iOS Base SDK", 8, false),
                           "The version of the iOS SDK to link against." + sdkInfoString + "14.4.");

                props.add (new TextPropertyComponent (iosDeploymentTarget, "iOS Deployment Target", 8, false),
                           "The minimum version of iOS to target." + sdkInfoString + "12.0.");
            }
            else
            {
                props.add (new TextPropertyComponent (macOSBaseSDK, "macOS Base SDK", 8, false),
                           "The version of the macOS SDK to link against." + sdkInfoString + "11.1.");

                props.add (new TextPropertyComponent (macOSDeploymentTarget, "macOS Deployment Target", 8, false),
                           "The minimum version of macOS to target." + sdkInfoString + "10.11.");

                props.add (new ChoicePropertyComponent (macOSArchitecture, "macOS Architecture",
                                                        { "Native architecture of build machine", "Standard 32-bit",        "Standard 32/64-bit",     "Standard 64-bit" },
                                                        { macOSArch_Native,                       macOSArch_32BitUniversal, macOSArch_64BitUniversal, macOSArch_64Bit }),
                           "The type of macOS binary that will be produced.");
            }

            props.add (new TextPropertyComponent (customXcodeFlags, "Custom Xcode Flags", 8192, true),
                       "A comma-separated list of custom Xcode setting flags which will be appended to the list of generated flags, "
                       "e.g. MACOSX_DEPLOYMENT_TARGET_i386 = 10.5");

            props.add (new TextPropertyComponent (plistPreprocessorDefinitions, "PList Preprocessor Definitions", 2048, true),
                       "Preprocessor definitions used during PList preprocessing (see PList Preprocess).");

            props.add (new TextPropertyComponent (codeSignIdentity, "Code-Signing Identity", 1024, false),
                       "The name of a code-signing identity for Xcode to apply.");

            props.add (new ChoicePropertyComponent (fastMathEnabled, "Relax IEEE Compliance"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new ChoicePropertyComponent (stripLocalSymbolsEnabled, "Strip Local Symbols"),
                       "Enable this to strip any locally defined symbols resulting in a smaller binary size. Enabling this "
                       "will also remove any function names from crash logs. Must be disabled for static library projects. "
                       "Note that disabling this will not necessarily generate full debug symbols. For release configs, "
                       "you will also need to add the following to the \"Custom Xcode Flags\" field: "
                       "GCC_GENERATE_DEBUGGING_SYMBOLS = YES, STRIP_INSTALLED_PRODUCT = NO, COPY_PHASE_STRIP = NO");
        }

        String getModuleLibraryArchName() const override
        {
            return "${CURRENT_ARCH}";
        }

        //==============================================================================
        String getMacOSArchitectureString() const               { return macOSArchitecture.get(); }
        String getPListPreprocessorDefinitionsString() const    { return plistPreprocessorDefinitions.get(); }

        bool isFastMathEnabled() const                          { return fastMathEnabled.get(); }

        bool isStripLocalSymbolsEnabled() const                 { return stripLocalSymbolsEnabled.get(); }

        String getCustomXcodeFlagsString() const                { return customXcodeFlags.get(); }

        String getMacOSBaseSDKString() const                    { return macOSBaseSDK.get(); }
        String getMacOSDeploymentTargetString() const           { return macOSDeploymentTarget.get(); }

        String getCodeSignIdentityString() const                { return codeSignIdentity.get(); }

        String getiOSBaseSDKString() const                      { return iosBaseSDK.get(); }
        String getiOSDeploymentTargetString() const             { return iosDeploymentTarget.get(); }

        bool isPluginBinaryCopyStepEnabled() const              { return pluginBinaryCopyStepEnabled.get(); }
        String getVSTBinaryLocationString() const               { return vstBinaryLocation.get(); }
        String getVST3BinaryLocationString() const              { return vst3BinaryLocation.get(); }
        String getAUBinaryLocationString() const                { return auBinaryLocation.get(); }
        String getAAXBinaryLocationString() const               { return aaxBinaryLocation.get();}
        String getUnityPluginBinaryLocationString() const       { return unityPluginBinaryLocation.get(); }
        String getLV2PluginBinaryLocationString() const         { return lv2BinaryLocation.get(); }

    private:
        //==============================================================================
        bool iOS;

        ValueTreePropertyWithDefault macOSBaseSDK, macOSDeploymentTarget, macOSArchitecture, iosBaseSDK, iosDeploymentTarget,
                                     customXcodeFlags, plistPreprocessorDefinitions, codeSignIdentity,
                                     fastMathEnabled, stripLocalSymbolsEnabled, pluginBinaryCopyStepEnabled,
                                     vstBinaryLocation, vst3BinaryLocation, auBinaryLocation,
                                     aaxBinaryLocation, unityPluginBinaryLocation, lv2BinaryLocation;

        //==============================================================================
        void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
        {
            const auto updateOldSDKSetting = [this] (const Identifier& oldProperty,
                                                     const String& sdkString,
                                                     const String& sdkSuffix)
            {
                if (sdkString.isEmpty())
                    config.removeProperty (oldProperty, nullptr);
                else
                    config.setProperty (oldProperty, sdkString + sdkSuffix, nullptr);
            };

            if (property == Ids::macOSBaseSDK)
                updateOldSDKSetting (Ids::osxSDK, macOSBaseSDK.get(), " SDK");
            else if (property == Ids::macOSDeploymentTarget)
                updateOldSDKSetting (Ids::osxCompatibility, macOSDeploymentTarget.get(), " SDK");
            else if (property == Ids::iosDeploymentTarget)
                updateOldSDKSetting (Ids::iosCompatibility, iosDeploymentTarget.get(), {});
        }

        void addXcodePluginInstallPathProperties (PropertyListBuilder& props)
        {
            auto isBuildingAnyPlugins = (project.shouldBuildVST() || project.shouldBuildVST3() || project.shouldBuildAU()
                                         || project.shouldBuildAAX() || project.shouldBuildUnityPlugin());

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

            if (project.shouldBuildAAX())
                props.add (new TextPropertyComponentWithEnablement (aaxBinaryLocation, pluginBinaryCopyStepEnabled, "AAX Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled AAX binary should be placed.");

            if (project.shouldBuildLV2())
                props.add (new TextPropertyComponentWithEnablement (lv2BinaryLocation, pluginBinaryCopyStepEnabled, "LV2 Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled LV2 binary should be placed.");

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
            if (! config ["xcodeAaxBinaryLocation"].isVoid())        aaxBinaryLocation  = config ["xcodeAaxBinaryLocation"];
        }

        void updateOldSDKDefaults()
        {
            if (macOSArchitecture.get() == "default")
                macOSArchitecture.resetToDefault();

            const auto updateSDKString = [this] (const Identifier& propertyName,
                                                 ValueTreePropertyWithDefault& value,
                                                 const String& suffix)
            {
                auto sdkString = config[propertyName].toString();

                if (sdkString == "default")
                    value.resetToDefault();
                else if (sdkString.isNotEmpty() && sdkString.endsWith (suffix))
                    value = sdkString.upToLastOccurrenceOf (suffix, false, false);
            };

            updateSDKString (Ids::osxSDK, macOSBaseSDK, " SDK");
            updateSDKString (Ids::osxCompatibility, macOSDeploymentTarget, " SDK");
            updateSDKString (Ids::iosCompatibility, iosDeploymentTarget, {});
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
    struct XcodeTarget final : build_tools::ProjectType::Target
    {
        //==============================================================================
        XcodeTarget (Type targetType, const XcodeProjectExporter& exporter)
            : Target (targetType),
              owner (exporter)
        {
            switch (type)
            {
                case GUIApp:
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case ConsoleApp:
                case LV2Helper:
                case VST3Helper:
                    xcodeFileType = "compiled.mach-o.executable";
                    xcodeBundleExtension = String();
                    xcodeProductType = "com.apple.product-type.tool";
                    xcodeCopyToProductInstallPathAfterBuild = false;

                    if (type == VST3Helper)
                        xcodeFrameworks.add ("Cocoa");

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
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case VST3PlugIn:
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst3";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case AudioUnitPlugIn:
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".component";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    addExtraAudioUnitTargetSettings();
                    break;

                case StandalonePlugIn:
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case AudioUnitv3PlugIn:
                    xcodeFileType = "wrapper.app-extension";
                    xcodeBundleExtension = ".appex";
                    xcodeBundleIDSubPath = "AUv3";
                    xcodeProductType = "com.apple.product-type.app-extension";
                    xcodeCopyToProductInstallPathAfterBuild = false;

                    addExtraAudioUnitv3PlugInTargetSettings();
                    break;

                case AAXPlugIn:
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".aaxplugin";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case UnityPlugIn:
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".bundle";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;
                    break;

                case LV2PlugIn:
                    xcodeFileType = "compiled.mach-o.executable";
                    xcodeProductType = "com.apple.product-type.tool";
                    xcodeBundleExtension = ".so";
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

                case unspecified:
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

        String getEntitlementsFilename() const
        {
            return String (getName()).replace (" ", "_") + String (".entitlements");
        }

        String xcodeBundleExtension;
        String xcodeProductType, xcodeFileType;
        String xcodeOtherRezFlags, xcodeBundleIDSubPath;
        bool xcodeCopyToProductInstallPathAfterBuild;
        StringArray xcodeFrameworks, xcodeLibs;
        Array<XmlElement> xcodeExtraPListEntries;

        StringArray frameworkIDs, buildPhaseIDs, configIDs, sourceIDs, rezFileIDs, dependencyIDs;
        StringArray frameworkNames;
        String mainBuildProductID;
        File infoPlistFile;

        //==============================================================================
        void addMainBuildProduct() const
        {
            jassert (xcodeFileType.isNotEmpty());
            jassert (xcodeBundleExtension.isEmpty() || xcodeBundleExtension.startsWithChar ('.'));

            if (ProjectExporter::BuildConfiguration::Ptr config = owner.getConfiguration (0))
            {
                const auto productName = [&]() -> String
                {
                    const auto binaryName = owner.replacePreprocessorTokens (*config, config->getTargetBinaryNameString (type == UnityPlugIn));

                    if (xcodeFileType == "archive.ar")
                        return getStaticLibbedFilename (binaryName);

                    if (type == LV2Helper)
                        return Project::getLV2FileWriterName();

                    if (type == VST3Helper)
                        return Project::getVST3FileWriterName();

                    return binaryName + xcodeBundleExtension;
                }();

                addBuildProduct (xcodeFileType, productName);
            }
        }

        //==============================================================================
        void addBuildProduct (const String& fileType, const String& binaryName) const
        {
            ValueTree v (owner.createID (String ("__productFileID") + getName()) + " /* " + getName() + " */");
            v.setProperty ("isa", "PBXFileReference", nullptr);
            v.setProperty ("explicitFileType", fileType, nullptr);
            v.setProperty ("includeInIndex", (int) 0, nullptr);
            v.setProperty ("path", binaryName, nullptr);
            v.setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", nullptr);

            owner.addObject (v);
        }

        //==============================================================================
        String addDependencyFor (const XcodeTarget& dependentTarget)
        {
            auto dependencyID = owner.createID (String ("__dependency") + getName() + dependentTarget.getName());
            ValueTree v (dependencyID);
            v.setProperty ("isa", "PBXTargetDependency", nullptr);
            v.setProperty ("target", getID(), nullptr);

            owner.addObject (v);

            return dependencyID;
        }

        void addDependencies()
        {
            if (! owner.project.isAudioPluginProject())
                return;

            if (type == XcodeTarget::AggregateTarget) // depends on all other targets
            {
                for (auto* target : owner.targets)
                    if (target->type != XcodeTarget::AggregateTarget)
                        dependencyIDs.add (target->addDependencyFor (*this));

                return;
            }

            if (type == XcodeTarget::LV2Helper || type == XcodeTarget::VST3Helper)
            {
                return;
            }

            if (type != XcodeTarget::SharedCodeTarget) // everything else depends on the sharedCodeTarget
            {
                if (auto* sharedCodeTarget = owner.getTargetOfType (XcodeTarget::SharedCodeTarget))
                    dependencyIDs.add (sharedCodeTarget->addDependencyFor (*this));
            }

            if (type == LV2PlugIn)
            {
                if (auto* helperTarget = owner.getTargetOfType (LV2Helper))
                    dependencyIDs.add (helperTarget->addDependencyFor (*this));
            }

            if (type == VST3PlugIn)
            {
                if (auto* helperTarget = owner.getTargetOfType (VST3Helper))
                    dependencyIDs.add (helperTarget->addDependencyFor (*this));
            }

            if (type == XcodeTarget::StandalonePlugIn)
            {
                if (auto* auv3Target = owner.getTargetOfType (XcodeTarget::AudioUnitv3PlugIn))
                    dependencyIDs.add (auv3Target->addDependencyFor (*this));
            }
        }

        //==============================================================================
        void addTargetConfig (const String& configName, const StringArray& buildSettings)
        {
            auto configID = owner.createID (String ("targetconfigid_") + getName() + String ("_") + configName);

            ValueTree v (configID);
            v.setProperty ("isa", "XCBuildConfiguration", nullptr);
            v.setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
            v.setProperty (Ids::name, configName, nullptr);

            configIDs.add (configID);

            owner.addObject (v);
        }

        bool shouldUseHardenedRuntime() const
        {
            return type != VST3Helper && type != LV2Helper && owner.isHardenedRuntimeEnabled();
        }

        bool shouldUseAppSandbox() const
        {
            return type == Target::AudioUnitv3PlugIn
                || (type != VST3Helper && type != LV2Helper && owner.isAppSandboxEnabled());
        }

        //==============================================================================
        String getTargetAttributes() const
        {
            StringArray attributes;

            auto developmentTeamID = owner.getDevelopmentTeamIDString();

            if (developmentTeamID.isNotEmpty())
            {
                attributes.add ("DevelopmentTeam = " + developmentTeamID);
                attributes.add ("ProvisioningStyle = Automatic");
            }

            std::map<String, bool> capabilities;

            capabilities["ApplicationGroups.iOS"] = owner.iOS && owner.isAppGroupsEnabled();
            capabilities["InAppPurchase"]         = owner.isInAppPurchasesEnabled();
            capabilities["InterAppAudio"]         = owner.iOS && ((type == Target::StandalonePlugIn
                                                                   && owner.getProject().shouldEnableIAA())
                                                                  || owner.getProject().isAUPluginHost());
            capabilities["Push"]                  = owner.isPushNotificationsEnabled();
            capabilities["Sandbox"]               = shouldUseAppSandbox();
            capabilities["HardenedRuntime"]       = shouldUseHardenedRuntime();

            if (owner.iOS && owner.isiCloudPermissionsEnabled())
                capabilities["com.apple.iCloud"] = true;

            StringArray capabilitiesStrings;

            for (auto& capability : capabilities)
                capabilitiesStrings.add ("com.apple." + capability.first + " = " + indentBracedList ({ String ("enabled = ") + (capability.second ? "1" : "0") }, 4));

            attributes.add ("SystemCapabilities = " + indentBracedList (capabilitiesStrings, 3));

            attributes.sort (false);

            return getID() + " = " + indentBracedList (attributes, 2);
        }

        //==============================================================================
        ValueTree addBuildPhase (const String& buildPhaseType, const StringArray& fileIds, const StringRef humanReadableName = StringRef())
        {
            auto buildPhaseName = buildPhaseType + "_" + getName() + "_" + (humanReadableName.isNotEmpty() ? String (humanReadableName) : String ("resbuildphase"));
            auto buildPhaseId (owner.createID (buildPhaseName));

            int n = 0;
            while (buildPhaseIDs.contains (buildPhaseId))
                buildPhaseId = owner.createID (buildPhaseName + String (++n));

            buildPhaseIDs.add (buildPhaseId);

            ValueTree v (buildPhaseId);
            v.setProperty ("isa", buildPhaseType, nullptr);
            v.setProperty ("buildActionMask", "2147483647", nullptr);
            v.setProperty ("files", indentParenthesisedList (fileIds), nullptr);

            if (humanReadableName.isNotEmpty())
                v.setProperty ("name", String (humanReadableName), nullptr);

            v.setProperty ("runOnlyForDeploymentPostprocessing", (int) 0, nullptr);

            owner.addObject (v);

            return v;
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
             || shouldUseAppSandbox()
             || shouldUseHardenedRuntime()
             || owner.isNetworkingMulticastEnabled()
             || (owner.isiOS() && owner.isiCloudPermissionsEnabled())
             || (owner.isiOS() && owner.getProject().isAUPluginHost()))
                return true;

            if (owner.project.isAudioPluginProject()
                && ((owner.isOSX() && type == Target::AudioUnitv3PlugIn)
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

        StringPairArray getConfigPreprocessorDefs (const XcodeBuildConfiguration& config) const
        {
            StringPairArray defines;

            if (config.isDebug())
            {
                defines.set ("_DEBUG", "1");
                defines.set ("DEBUG", "1");
            }
            else
            {
                defines.set ("_NDEBUG", "1");
                defines.set ("NDEBUG", "1");
            }

            if (owner.isInAppPurchasesEnabled())
                defines.set ("JUCE_IN_APP_PURCHASES", "1");

            if (owner.iOS && owner.isContentSharingEnabled())
                defines.set ("JUCE_CONTENT_SHARING", "1");

            if (owner.isPushNotificationsEnabled())
                defines.set ("JUCE_PUSH_NOTIFICATIONS", "1");

            return mergePreprocessorDefs (defines, owner.getAllPreprocessorDefs (config, type));
        }

        String getConfigurationBuildDir (const XcodeBuildConfiguration& config) const
        {
            const String configurationBuildDir ("$(PROJECT_DIR)/build/$(CONFIGURATION)");

            if (config.getTargetBinaryRelativePathString().isEmpty())
                return configurationBuildDir;

            // a target's position can either be defined via installPath + xcodeCopyToProductInstallPathAfterBuild
            // (= for audio plug-ins) or using a custom binary path (for everything else), but not both (= conflict!)
            jassert (! xcodeCopyToProductInstallPathAfterBuild);

            build_tools::RelativePath binaryPath (config.getTargetBinaryRelativePathString(),
                                                  build_tools::RelativePath::projectFolder);

            return expandPath (binaryPath.rebased (owner.projectFolder,
                                                   owner.getTargetFolder(),
                                                   build_tools::RelativePath::buildTargetFolder).toUnixStyle());
        }

        String getLV2BundleName() const { return owner.project.getPluginNameString() + ".lv2"; }

        //==============================================================================
        StringPairArray getTargetSettings (const XcodeBuildConfiguration& config) const
        {
            StringPairArray s;

            if (type == AggregateTarget && ! owner.isiOS())
            {
                // the aggregate target needs to have the deployment target set for
                // pre-/post-build scripts
                s.set ("MACOSX_DEPLOYMENT_TARGET", config.getMacOSDeploymentTargetString());
                s.set ("SDKROOT", "macosx" + config.getMacOSBaseSDKString());

                return s;
            }

            const auto productName = [&]
            {
                if (type == LV2Helper)
                    return Project::getLV2FileWriterName().quoted();

                if (type == VST3Helper)
                    return Project::getVST3FileWriterName().quoted();

                return owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString (type == UnityPlugIn)).quoted();
            }();

            s.set ("PRODUCT_NAME", productName);
            s.set ("PRODUCT_BUNDLE_IDENTIFIER", getBundleIdentifier());

            auto arch = (! owner.isiOS() && type == Target::AudioUnitv3PlugIn) ? macOSArch_64Bit
                                                                               : config.getMacOSArchitectureString();

            const auto archString = [&]() -> const char*
            {
                if (arch == macOSArch_Native)           return "\"$(NATIVE_ARCH_ACTUAL)\"";
                if (arch == macOSArch_32BitUniversal)   return "\"$(ARCHS_STANDARD_32_BIT)\"";
                if (arch == macOSArch_64BitUniversal)   return "\"$(ARCHS_STANDARD_32_64_BIT)\"";
                if (arch == macOSArch_64Bit)            return "\"$(ARCHS_STANDARD_64_BIT)\"";

                return nullptr;
            }();

            if (archString != nullptr)
                s.set ("ARCHS", archString);

            if (! owner.isiOS())
            {
                const auto validArchs = owner.getValidArchs();

                if (! validArchs.isEmpty())
                {
                    const auto join = [] (const Array<var>& range)
                    {
                        return std::accumulate (range.begin(),
                                                range.end(),
                                                String(),
                                                [] (String str, const var& v) { return str + v.toString() + " "; }).trim().quoted();
                    };

                    s.set ("VALID_ARCHS", join (validArchs));

                    auto excludedArchs = owner.getAllArchs();
                    excludedArchs.removeIf ([&validArchs] (const auto& a) { return validArchs.contains (a); });

                    s.set ("EXCLUDED_ARCHS", join (excludedArchs));
                }
            }

            auto headerPaths = getHeaderSearchPaths (config);

            auto mtlHeaderPaths = headerPaths;

            for (auto& path : mtlHeaderPaths)
                path = path.unquoted();

            s.set ("MTL_HEADER_SEARCH_PATHS", "\"" + mtlHeaderPaths.joinIntoString (" ") + "\"");

            headerPaths.add ("\"$(inherited)\"");
            s.set ("HEADER_SEARCH_PATHS", indentParenthesisedList (headerPaths, 1));
            s.set ("USE_HEADERMAP", String (static_cast<bool> (config.exporter.settings.getProperty ("useHeaderMap")) ? "YES" : "NO"));

            auto frameworksToSkip = [this]() -> String
            {
                const String openGLFramework (owner.iOS ? "OpenGLES" : "OpenGL");

                if (owner.xcodeFrameworks.contains (openGLFramework))
                    return openGLFramework;

                return {};
            }();

            if (frameworksToSkip.isNotEmpty())
                s.set ("VALIDATE_WORKSPACE_SKIPPED_SDK_FRAMEWORKS", frameworksToSkip);

            auto frameworkSearchPaths = getFrameworkSearchPaths (config);

            if (! frameworkSearchPaths.isEmpty())
                s.set ("FRAMEWORK_SEARCH_PATHS", String ("(") + frameworkSearchPaths.joinIntoString (", ") + ", \"$(inherited)\")");

            s.set ("GCC_OPTIMIZATION_LEVEL", config.getGCCOptimisationFlag());

            if (config.shouldUsePrecompiledHeaderFile())
            {
                s.set ("GCC_PRECOMPILE_PREFIX_HEADER", "YES");

                auto pchFileContent = config.getPrecompiledHeaderFileContent();

                if (pchFileContent.isNotEmpty())
                {
                    auto pchFilename = config.getPrecompiledHeaderFilename() + ".h";

                    build_tools::writeStreamToFile (owner.getTargetFolder().getChildFile (pchFilename),
                                                    [&] (MemoryOutputStream& mo) { mo << pchFileContent; });

                    s.set ("GCC_PREFIX_HEADER", pchFilename);
                }
            }

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

            auto recommendedWarnings = config.getRecommendedCompilerWarningFlags();
            recommendedWarnings.common.addArray (recommendedWarnings.objc);
            recommendedWarnings.cpp.addArray (recommendedWarnings.common);

            struct XcodeWarningFlags
            {
                const StringArray& flags;
                const String variable;
            };

            for (const auto& xcodeFlags : { XcodeWarningFlags { recommendedWarnings.common, "OTHER_CFLAGS" },
                                            XcodeWarningFlags { recommendedWarnings.cpp,    "OTHER_CPLUSPLUSFLAGS" } })
            {
                const auto flags = owner.replacePreprocessorTokens (config,
                                                                    (xcodeFlags.flags.joinIntoString (" ")
                                                                     + " "
                                                                     + config.getAllCompilerFlagsString()).trim());

                if (flags.isNotEmpty())
                    s.set (xcodeFlags.variable, flags.quoted());
            }

            auto installPath = getInstallPathForConfiguration (config);

            if (installPath.startsWith ("~"))
                installPath = installPath.replace ("~", "$(HOME)");

            if (installPath.isNotEmpty())
            {
                s.set ("INSTALL_PATH", installPath.quoted());

                if (type == Target::SharedCodeTarget || type == Target::LV2PlugIn)
                    s.set ("SKIP_INSTALL", "YES");

                if (! owner.embeddedFrameworkIDs.isEmpty())
                    s.set ("LD_RUNPATH_SEARCH_PATHS", "\"$(inherited) @executable_path/Frameworks @executable_path/../Frameworks\"");
            }

            if (getTargetFileType() == pluginBundle)
            {
                s.set ("LIBRARY_STYLE", "Bundle");
                s.set ("WRAPPER_EXTENSION", xcodeBundleExtension.substring (1));
                s.set ("GENERATE_PKGINFO_FILE", "YES");
            }

            if (xcodeOtherRezFlags.isNotEmpty())
                s.set ("OTHER_REZFLAGS", "\"" + xcodeOtherRezFlags + "\"");

            const auto configurationBuildDir = getConfigurationBuildDir (config);
            const auto adjustedConfigBuildDir = type == LV2PlugIn ? configurationBuildDir + "/" + getLV2BundleName()
                                                                  : configurationBuildDir;

            s.set ("CONFIGURATION_BUILD_DIR", addQuotesIfRequired (adjustedConfigBuildDir));

            if (shouldUseHardenedRuntime())
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
                s.set ("MACOSX_DEPLOYMENT_TARGET", config.getMacOSDeploymentTargetString());
            }

            s.set ("GCC_VERSION", gccVersion);
            s.set ("CLANG_LINK_OBJC_RUNTIME", "NO");

            owner.addCodeSigningIdentity (config, s);

            if (owner.getCodeSigningIdentity (config).isNotEmpty())
            {
                s.set ("PROVISIONING_PROFILE_SPECIFIER", "\"\"");

                if (! owner.isUsingDefaultSigningIdentity (config))
                    s.set ("CODE_SIGN_STYLE", "Manual");
            }

            if (owner.getDevelopmentTeamIDString().isNotEmpty())
                s.set ("DEVELOPMENT_TEAM", owner.getDevelopmentTeamIDString());

            if (shouldAddEntitlements())
                s.set ("CODE_SIGN_ENTITLEMENTS", getEntitlementsFilename().quoted());

            {
                const auto cppStandard = [&]() -> String
                {
                    if (owner.project.getCppStandardString() == "latest")
                        return owner.project.getLatestNumberedCppStandardString();

                    return owner.project.getCppStandardString();
                }();

                s.set ("CLANG_CXX_LANGUAGE_STANDARD", (String (owner.shouldUseGNUExtensions() ? "gnu++"
                                                                                              : "c++") + cppStandard).quoted());
            }

            s.set ("CLANG_CXX_LIBRARY", "\"libc++\"");

            s.set ("COMBINE_HIDPI_IMAGES", "YES");

            {
                StringArray linkerFlags;
                getLinkerSettings (config, linkerFlags);

                if (linkerFlags.size() > 0)
                    s.set ("OTHER_LDFLAGS", linkerFlags.joinIntoString (" ").quoted());

                StringArray librarySearchPaths;
                librarySearchPaths.addArray (config.getLibrarySearchPaths());

                if (type == LV2PlugIn)
                    librarySearchPaths.add (configurationBuildDir);

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

            if (config.isDebug())
            {
                s.set ("COPY_PHASE_STRIP", "NO");
                s.set ("GCC_DYNAMIC_NO_PIC", "NO");
            }
            else
            {
                s.set ("GCC_GENERATE_DEBUGGING_SYMBOLS", "NO");
                s.set ("DEAD_CODE_STRIPPING", "YES");
            }

            if (type != Target::SharedCodeTarget && type != Target::StaticLibrary && type != Target::DynamicLibrary
                && config.isStripLocalSymbolsEnabled())
            {
                s.set ("STRIPFLAGS", "\"-x\"");
                s.set ("DEPLOYMENT_POSTPROCESSING", "YES");
                s.set ("SEPARATE_STRIP", "YES");
            }

            StringArray defsList;

            const auto defines = getConfigPreprocessorDefs (config);

            for (int i = 0; i < defines.size(); ++i)
            {
                auto def = defines.getAllKeys()[i];
                auto value = defines.getAllValues()[i];
                if (value.isNotEmpty())
                    def << "=" << value.replace ("\"", "\\\\\\\"").replace (" ", "\\\\ ").replace ("\'", "\\\\'");

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
                case AAXPlugIn:         return config.isPluginBinaryCopyStepEnabled() ? config.getAAXBinaryLocationString() : String();
                case UnityPlugIn:       return config.isPluginBinaryCopyStepEnabled() ? config.getUnityPluginBinaryLocationString() : String();
                case LV2PlugIn:         return config.isPluginBinaryCopyStepEnabled() ? config.getLV2PluginBinaryLocationString() : String();
                case SharedCodeTarget:  return owner.isiOS() ? "@executable_path/Frameworks" : "@executable_path/../Frameworks";
                case StaticLibrary:
                case LV2Helper:
                case VST3Helper:
                case DynamicLibrary:
                case AudioUnitv3PlugIn:
                case StandalonePlugIn:
                case AggregateTarget:
                case unspecified:
                default:                return {};
            }
        }

        //==============================================================================
        void getLinkerSettings (const BuildConfiguration& config, StringArray& flags) const
        {
            if (getTargetFileType() == pluginBundle)
                flags.add (owner.isiOS() ? "-bitcode_bundle" : "-bundle");

            if (type != Target::SharedCodeTarget && type != Target::LV2Helper && type != Target::VST3Helper)
            {
                if (owner.project.isAudioPluginProject())
                {
                    if (owner.getTargetOfType (Target::SharedCodeTarget) != nullptr)
                    {
                        auto productName = getStaticLibbedFilename (owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

                        build_tools::RelativePath sharedCodelib (productName, build_tools::RelativePath::buildTargetFolder);
                        flags.add (getLinkerFlagForLib (sharedCodelib.getFileNameWithoutExtension()));
                    }
                }

                flags.add (owner.getExternalLibraryFlags (config));

                auto libs = owner.xcodeLibs;
                libs.addArray (xcodeLibs);

                for (auto& l : libs)
                    flags.add (getLinkerFlagForLib (l));
            }

            flags.add (owner.replacePreprocessorTokens (config, config.getAllLinkerFlagsString()));
            flags = getCleanedStringArray (flags);
        }

        //==============================================================================
        void writeInfoPlistFile() const
        {
            if (! shouldCreatePList())
                return;

            build_tools::PlistOptions options;

            options.type                             = type;
            options.executableName                   = "${EXECUTABLE_NAME}";
            options.bundleIdentifier                 = getBundleIdentifier();
            options.applicationCategory              = owner.getApplicationCategoryString();
            options.plistToMerge                     = owner.getPListToMergeString();
            options.iOS                              = owner.iOS;
            options.microphonePermissionEnabled      = owner.isMicrophonePermissionEnabled();
            options.microphonePermissionText         = owner.getMicrophonePermissionsTextString();
            options.cameraPermissionEnabled          = owner.isCameraPermissionEnabled();
            options.cameraPermissionText             = owner.getCameraPermissionTextString();
            options.bluetoothPermissionEnabled       = owner.isBluetoothPermissionEnabled();
            options.bluetoothPermissionText          = owner.getBluetoothPermissionTextString();
            options.sendAppleEventsPermissionEnabled = owner.isSendAppleEventsPermissionEnabled();
            options.sendAppleEventsPermissionText    = owner.getSendAppleEventsPermissionTextString();
            options.shouldAddStoryboardToProject     = owner.shouldAddStoryboardToProject();
            options.iconFile                         = owner.iconFile;
            options.projectName                      = owner.projectName;
            options.marketingVersion                 = owner.project.getVersionString();
            options.currentProjectVersion            = owner.getBuildNumber();
            options.companyCopyright                 = owner.project.getCompanyCopyrightString();
            options.allPreprocessorDefs              = owner.getAllPreprocessorDefs();
            options.documentExtensions               = owner.getDocumentExtensionsString();
            options.fileSharingEnabled               = owner.isFileSharingEnabled();
            options.documentBrowserEnabled           = owner.isDocumentBrowserEnabled();
            options.statusBarHidden                  = owner.isStatusBarHidden();
            options.requiresFullScreen               = owner.requiresFullScreen();
            options.backgroundAudioEnabled           = owner.isBackgroundAudioEnabled();
            options.backgroundBleEnabled             = owner.isBackgroundBleEnabled();
            options.pushNotificationsEnabled         = owner.isPushNotificationsEnabled();
            options.enableIAA                        = owner.project.shouldEnableIAA();
            options.IAAPluginName                    = owner.project.getIAAPluginName();
            options.pluginManufacturerCode           = owner.project.getPluginManufacturerCodeString();
            options.IAATypeCode                      = owner.project.getIAATypeCode();
            options.pluginCode                       = owner.project.getPluginCodeString();
            options.iPhoneScreenOrientations         = owner.getiPhoneScreenOrientations();
            options.iPadScreenOrientations           = owner.getiPadScreenOrientations();

            options.storyboardName = [&]
            {
                const auto customLaunchStoryboard = owner.getCustomLaunchStoryboardString();

                if (customLaunchStoryboard.isEmpty())
                    return owner.getDefaultLaunchStoryboardName();

                return customLaunchStoryboard.fromLastOccurrenceOf ("/", false, false)
                                             .upToLastOccurrenceOf (".storyboard", false, false);
            }();

            options.pluginName                      = owner.project.getPluginNameString();
            options.pluginManufacturer              = owner.project.getPluginManufacturerString();
            options.pluginDescription               = owner.project.getPluginDescriptionString();
            options.pluginAUExportPrefix            = owner.project.getPluginAUExportPrefixString();
            options.auMainType                      = owner.project.getAUMainTypeString();
            options.isAuSandboxSafe                 = owner.project.isAUSandBoxSafe();
            options.isPluginSynth                   = owner.project.isPluginSynth();
            options.suppressResourceUsage           = owner.getSuppressPlistResourceUsage();
            options.isPluginARAEffect               = owner.project.shouldEnableARA();

            options.write (infoPlistFile);
        }

        //==============================================================================
        void addShellScriptBuildPhase (const String& phaseName, const String& script)
        {
            if (script.trim().isEmpty())
                return;

            auto v = addBuildPhase ("PBXShellScriptBuildPhase", {});
            v.setProperty (Ids::name, phaseName, nullptr);
            v.setProperty ("alwaysOutOfDate", 1, nullptr);
            v.setProperty ("shellPath", "/bin/sh", nullptr);
            v.setProperty ("shellScript", script.replace ("\\", "\\\\")
                                                .replace ("\"", "\\\"")
                                                .replace ("\r\n", "\\n")
                                                .replace ("\n", "\\n"), nullptr);
        }

        void addCopyFilesPhase (const String& phaseName, const StringArray& files, XcodeCopyFilesDestinationIDs dst)
        {
            auto v = addBuildPhase ("PBXCopyFilesBuildPhase", files, phaseName);
            v.setProperty ("dstPath", "", nullptr);
            v.setProperty ("dstSubfolderSpec", (int) dst, nullptr);
        }

        //==============================================================================
        void sanitiseAndEscapeSearchPaths (const BuildConfiguration& config, StringArray& paths) const
        {
            paths = getCleanedStringArray (paths);

            for (auto& path : paths)
            {
                path = owner.replacePreprocessorTokens (config, expandPath (path));

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

            constexpr auto audioPluginClient = "juce_audio_plugin_client";

            if (owner.project.getEnabledModules().isModuleEnabled (audioPluginClient))
            {
                paths.add (owner.getModuleFolderRelativeToProject (audioPluginClient)
                                .getChildFile ("AU")
                                .rebased (owner.projectFolder,
                                          owner.getTargetFolder(),
                                          build_tools::RelativePath::buildTargetFolder)
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
            xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64 -d arm64_$arm64"
                                 " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                 " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\""
                                 " -I \\\"$(DEVELOPER_DIR)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/AudioUnit.framework/Headers\\\"";

            xcodeFrameworks.addArray ({ "AudioUnit", "CoreAudioKit" });
        }

        void addExtraAudioUnitv3PlugInTargetSettings()
        {
            xcodeFrameworks.addArray ({ "AVFoundation", "CoreAudioKit" });

            if (owner.isOSX())
                xcodeFrameworks.add ("AudioUnit");
        }

        //==============================================================================
        const XcodeProjectExporter& owner;

        Target& operator= (const Target&) = delete;
    };

    mutable StringArray xcodeFrameworks;
    mutable StringArray xcodeWeakFrameworks;
    StringArray xcodeLibs;

private:
    //==============================================================================
    static String replaceHomeTildeInPath (const String& path)
    {
        return path.startsWithChar ('~') ? "$(HOME)" + path.substring (1)
                                         : path;
    }

    static String expandPath (const String& path)
    {
        if (! build_tools::isAbsolutePath (path))  return "$(SRCROOT)/" + path;

        return replaceHomeTildeInPath (path);
    }

    static String addQuotesIfRequired (const String& s)
    {
        return s.containsAnyOf (" $") ? s.quoted() : s;
    }

    File getProjectBundle() const                 { return getTargetFolder().getChildFile (project.getProjectFilenameRootString()).withFileExtension (".xcodeproj"); }

    void canCreateMessageBox (CreatorFunction f) override
    {
        if (hasInvalidPostBuildScript())
        {
            String alertWindowText = iOS ? "Your Xcode (iOS) Exporter settings use an invalid post-build script. Click 'Update' to remove it."
                                         : "Your Xcode (macOS) Exporter settings use a pre-JUCE 4.2 post-build script to move the plug-in binaries to their plug-in install folders.\n\n"
                                           "Since JUCE 4.2, this is instead done using \"AU/VST/VST2/AAX Binary Location\" in the Xcode (OS X) configuration settings.\n\n"
                                           "Click 'Update' to remove the script (otherwise your plug-in may not compile correctly).";

            auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                                   "Project settings: " + project.getDocumentTitle(),
                                                                   alertWindowText,
                                                                   "Update",
                                                                   "Cancel");

            messageBox = f (options, [this] (int result)
                            {
                                userAcknowledgedInvalidPostBuildScript = true;

                                if (result != 0)
                                    postbuildCommandValue.resetToDefault();

                                if (! needsDisplayMessageBox())
                                    messageBoxQueueListenerScope.reset();
                            });
        }
        else if (hasDefunctIOKitSetting())
        {
            String alertWindowText = "Your Xcode (macOS) Exporter settings use a defunct, boolean value for the iokit-user-client-class temporary exception entitlement.\n\n"
                                     "If you need this entitlement, add the IOUserClient subclasses to the new IOKit exception related field.\n\n"
                                     "For more information see Apple's IOKit User Client Class Temporary Exception documentation.\n\n"
                                     "Clicking 'Update' will remove the defunct setting from your project.";

            auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                                   "Project settings: " + project.getDocumentTitle(),
                                                                   alertWindowText,
                                                                   "Update",
                                                                   "Cancel");

            messageBox = f (std::move (options), [this] (int result)
                                                 {
                                                     userAcknowledgedDefunctIOKitSetting = true;

                                                     if (result != 0)
                                                     {
                                                         auto v = appSandboxOptionsValue.get();
                                                         v.getArray()->removeAllInstancesOf ("com.apple.security.temporary-exception.iokit-user-client-class");
                                                         appSandboxOptionsValue.setValue (v, nullptr);
                                                     }

                                                     if (! needsDisplayMessageBox())
                                                         messageBoxQueueListenerScope.reset();
                                                 });
        }
    }

    //==============================================================================
    void createObjects() const
    {
        prepareTargets();

        // Must be called before adding embedded frameworks, as we want to
        // embed any frameworks found in subprojects.
        addSubprojects();

        addFrameworks();

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
                    addLaunchStoryboardFileReference (build_tools::RelativePath (customLaunchStoryboard, build_tools::RelativePath::projectFolder)
                                                          .rebased (getProject().getProjectFolder(), getTargetFolder(), build_tools::RelativePath::buildTargetFolder));
            }
        }
        else
        {
            addNibFiles();
        }

        addIcons();
        addBuildConfigurations();

        addProjectConfigList (createID ("__projList"));

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
            target->addDependencies();

            if (target->type == XcodeTarget::AggregateTarget)
                continue;

            target->addMainBuildProduct();

            if (project.getEnabledModules().isModuleEnabled ("juce_audio_plugin_client"))
            {
                auto getFileOptions = [this, target] (const build_tools::RelativePath& path)
                {
                    const auto rebasedPath = rebaseFromProjectFolderToBuildTarget (path);
                    return FileOptions().withRelativePath ({ replaceHomeTildeInPath (rebasedPath.toUnixStyle()), rebasedPath.getRoot() })
                                        .withSkipPCHEnabled (true)
                                        .withCompilationEnabled (true)
                                        .withInhibitWarningsEnabled (true)
                                        .withXcodeTarget (target);
                };

                if (target->type == XcodeTarget::LV2Helper)
                    addFile (getFileOptions (getLV2HelperProgramSource()));
                else if (target->type == XcodeTarget::VST3Helper)
                    addFile (getFileOptions (getVST3HelperProgramSource()).withCompilerFlags ("-fobjc-arc"));
            }

            auto targetName = String (target->getName());
            auto fileID = createID (targetName + "__targetbuildref");
            auto fileRefID = createID ("__productFileID" + targetName);

            ValueTree v (fileID + " /* " + targetName + " */");
            v.setProperty ("isa", "PBXBuildFile", nullptr);
            v.setProperty ("fileRef", fileRefID, nullptr);

            target->mainBuildProductID = fileID;

            addObject (v);
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
                build_tools::RelativePath plistPath (target->infoPlistFile, getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
                addFileReference (plistPath.toUnixStyle());
                resourceFileRefs.add (createFileRefID (plistPath));
            }
        }
    }

    void addNibFiles() const
    {
        build_tools::writeStreamToFile (menuNibFile, [&] (MemoryOutputStream& mo)
        {
            mo.write (BinaryData::RecentFilesMenuTemplate_nib, BinaryData::RecentFilesMenuTemplate_nibSize);
        });

        build_tools::RelativePath menuNibPath (menuNibFile, getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
        addFileReference (menuNibPath.toUnixStyle());
        resourceIDs.add (addBuildFile (FileOptions().withRelativePath (menuNibPath)));
        resourceFileRefs.add (createFileRefID (menuNibPath));
    }

    void addIcons() const
    {
        if (iconFile.exists())
        {
            build_tools::RelativePath iconPath (iconFile, getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
            addFileReference (iconPath.toUnixStyle());
            resourceIDs.add (addBuildFile (FileOptions().withRelativePath (iconPath)));
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
            auto keys = configSettings.getAllKeys();
            keys.sort (false);

            for (auto& key : keys)
                settingsLines.add (key + " = " + configSettings[key]);

            addProjectConfig (config->getName(), settingsLines);
        }
    }

    void addFilesAndGroupsToProject (StringArray& topLevelGroupIDs) const
    {
        for (auto* target : targets)
            if (target->shouldAddEntitlements())
                addEntitlementsFile (*target);

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
                auto& xcodeConfig = static_cast<const XcodeBuildConfiguration&> (*config);

                auto configSettings = target->getTargetSettings (xcodeConfig);
                StringArray settingsLines;
                auto keys = configSettings.getAllKeys();
                keys.sort (false);

                for (auto& key : keys)
                    settingsLines.add (key + " = " + configSettings.getValue (key, "\"\""));

                target->addTargetConfig (config->getName(), settingsLines);
            }

            addConfigList (*target, createID (String ("__configList") + target->getName()));

            target->addShellScriptBuildPhase ("Pre-build script", getPreBuildScript());

            if (target->type != XcodeTarget::AggregateTarget)
            {
                auto skipAUv3 = (target->type == XcodeTarget::AudioUnitv3PlugIn && ! shouldDuplicateAppExResourcesFolder());

                if (! projectType.isStaticLibrary()
                    && target->type != XcodeTarget::SharedCodeTarget
                    && target->type != XcodeTarget::LV2Helper
                    && target->type != XcodeTarget::VST3Helper
                    && ! skipAUv3)
                    target->addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

                auto rezFiles = rezFileIDs;
                rezFiles.addArray (target->rezFileIDs);

                if (rezFiles.size() > 0)
                    target->addBuildPhase ("PBXRezBuildPhase", rezFiles);

                auto sourceFiles = target->sourceIDs;

                if (target->type == XcodeTarget::SharedCodeTarget
                     || (! project.isAudioPluginProject()))
                    sourceFiles.addArray (sourceIDs);

                target->addBuildPhase ("PBXSourcesBuildPhase", sourceFiles);

                if (! projectType.isStaticLibrary()
                    && target->type != XcodeTarget::SharedCodeTarget
                    && target->type != XcodeTarget::LV2Helper)
                {
                    target->addBuildPhase ("PBXFrameworksBuildPhase", target->frameworkIDs);
                }
            }

            // When building LV2 and VST3 plugins on Arm macs, we need to load and run the plugin
            // bundle during a post-build step in order to generate the plugin's supporting files.
            // Arm macs will only load shared libraries if they are signed, but Xcode runs its
            // signing step after any post-build scripts. As a workaround, we sign the plugin
            // using an adhoc certificate.
            if (target->type == XcodeTarget::VST3PlugIn || target->type == XcodeTarget::LV2PlugIn)
            {
                ScriptBuilder script;

                if (target->type == XcodeTarget::LV2PlugIn)
                {
                    // Note: LV2 has a non-standard config build dir
                    script.run ("codesign --verbose=4 --force --sign -", doubleQuoted ("${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_NAME}"))
                          .insertLine()
                          .run (doubleQuoted ("${CONFIGURATION_BUILD_DIR}/../" + Project::getLV2FileWriterName()),
                                doubleQuoted ("${CONFIGURATION_BUILD_DIR}/${EXECUTABLE_NAME}"));
                }
                else if (target->type == XcodeTarget::VST3PlugIn)
                {
                    script.run ("codesign --verbose=4 --force --sign -", doubleQuoted ("${CONFIGURATION_BUILD_DIR}/${WRAPPER_NAME}"))
                          .insertLine()
                          .run (doubleQuoted ("${CONFIGURATION_BUILD_DIR}/" + Project::getVST3FileWriterName()),
                                "-create",
                                "-version", doubleQuoted (project.getVersionString()),
                                "-path",    doubleQuoted ("${CONFIGURATION_BUILD_DIR}/${WRAPPER_NAME}"),
                                "-output",  doubleQuoted ("${CONFIGURATION_BUILD_DIR}/${WRAPPER_NAME}/Contents/Resources/moduleinfo.json"));
                }

                target->addShellScriptBuildPhase ("Update manifest", script.toStringWithDefaultShellOptions());
            }

            target->addShellScriptBuildPhase ("Post-build script", getPostBuildScript());

            if (project.isAudioPluginProject() && project.shouldBuildAUv3()
                && project.shouldBuildStandalonePlugin() && target->type == XcodeTarget::StandalonePlugIn)
                embedAppExtension();

            if (project.isAudioPluginProject() && project.shouldBuildUnityPlugin()
                && target->type == XcodeTarget::UnityPlugIn)
                embedUnityScript();

            ScriptBuilder copyPluginStepScript;

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto& xcodeConfig = static_cast<const XcodeBuildConfiguration&> (*config);
                auto installPath = target->getInstallPathForConfiguration (xcodeConfig);

                if (installPath.isEmpty() || ! target->xcodeCopyToProductInstallPathAfterBuild)
                    continue;

                if (installPath.startsWith ("~"))
                    installPath = installPath.replace ("~", "$(HOME)");

                installPath = installPath.replace ("$(HOME)", "${HOME}");

                const auto sourcePlugin = target->type == XcodeTarget::Target::LV2PlugIn
                                        ? "${TARGET_BUILD_DIR}"
                                        : "${TARGET_BUILD_DIR}/${WRAPPER_NAME}";

                const auto copyScript = ScriptBuilder{}
                        .set ("destinationPlugin", installPath + "/$(basename " + doubleQuoted (sourcePlugin) + ")")
                        .remove ("${destinationPlugin}")
                        .copy (sourcePlugin, "${destinationPlugin}");

                const auto objectToSignTail = target->type == XcodeTarget::Target::LV2PlugIn
                                            ? "/$(basename \"${TARGET_BUILD_DIR}\")/${EXECUTABLE_NAME}"
                                            : "/${WRAPPER_NAME}";

                const auto codesignScript = ScriptBuilder{}
                        .ifSet ("EXPANDED_CODE_SIGN_IDENTITY",
                                ScriptBuilder{}.ifSet ("CODE_SIGN_ENTITLEMENTS",
                                                       R"(entitlementsArg=(--entitlements "${CODE_SIGN_ENTITLEMENTS}"))")
                                               .echo ("Signing Identity: " + doubleQuoted ("${EXPANDED_CODE_SIGN_IDENTITY_NAME}") )
                                               .run ("codesign --verbose=4 --force --sign",
                                                   doubleQuoted ("${EXPANDED_CODE_SIGN_IDENTITY}"),
                                                   "${entitlementsArg[*]-}",
                                                   "${OTHER_CODE_SIGN_FLAGS-}",
                                                   doubleQuoted (installPath + objectToSignTail))
                                               .toString());

                copyPluginStepScript.ifEqual (doubleQuoted ("${CONFIGURATION}"), doubleQuoted (config->getName()),
                                              ScriptBuilder{}.insertScript (copyScript.toString())
                                                             .insertLine()
                                                             .insertScript (codesignScript.toString())
                                                             .toString());
            }

            if (! copyPluginStepScript.isEmpty())
                target->addShellScriptBuildPhase ("Plugin Copy Step", copyPluginStepScript.toStringWithDefaultShellOptions());

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
            build_tools::RelativePath scriptPath (getProject().getGeneratedCodeFolder().getChildFile (getProject().getUnityScriptName()),
                                                  getTargetFolder(),
                                                  build_tools::RelativePath::buildTargetFolder);

            auto path = scriptPath.toUnixStyle();
            auto refID = addFileReference (path);
            auto fileID = addBuildFile (FileOptions().withPath (path)
                                                     .withFileRefID (refID));

            resourceIDs.add (fileID);
            resourceFileRefs.add (refID);

            unityTarget->addCopyFilesPhase ("Embed Unity Script", fileID, kResourcesFolder);
        }
    }

    //==============================================================================
    XcodeTarget* getTargetOfType (build_tools::ProjectType::Target::Type type) const
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
        ValueTree v (targetID);
        v.setProperty ("isa", target.type == XcodeTarget::AggregateTarget ? "PBXAggregateTarget" : "PBXNativeTarget", nullptr);
        v.setProperty ("buildConfigurationList", createID (String ("__configList") + targetName), nullptr);

        v.setProperty ("buildPhases", indentParenthesisedList (target.buildPhaseIDs), nullptr);

        if (target.type != XcodeTarget::AggregateTarget)
            v.setProperty ("buildRules", indentParenthesisedList ({}), nullptr);

        StringArray allDependencyIDs { subprojectDependencyIDs };
        allDependencyIDs.addArray (target.dependencyIDs);
        v.setProperty ("dependencies", indentParenthesisedList (allDependencyIDs), nullptr);

        v.setProperty (Ids::name, target.getXcodeSchemeName(), nullptr);
        v.setProperty ("productName", projectName, nullptr);

        if (target.type != XcodeTarget::AggregateTarget)
        {
            v.setProperty ("productReference", createID (String ("__productFileID") + targetName), nullptr);

            jassert (target.xcodeProductType.isNotEmpty());
            v.setProperty ("productType", target.xcodeProductType, nullptr);
        }

        targetIDs.add (targetID);
        addObject (v);
    }

    void createIconFile() const
    {
        const auto icons = getIcons();

        if (! build_tools::asArray (icons).isEmpty())
        {
            iconFile = getTargetFolder().getChildFile ("Icon.icns");
            build_tools::writeMacIcon (icons, iconFile);
        }
    }

    void writeWorkspaceSettings() const
    {
        const auto settingsFile = getProjectBundle().getChildFile ("project.xcworkspace")
                                                    .getChildFile ("xcshareddata")
                                                    .getChildFile ("WorkspaceSettings.xcsettings");

        if (shouldUseLegacyBuildSystem())
        {
            build_tools::writeStreamToFile (settingsFile, [this] (MemoryOutputStream& mo)
            {
                mo.setNewLineString (getNewLineString());

                mo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                 << newLine
                   << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << newLine
                   << "<plist version=\"1.0\">"                                    << newLine
                   << "<dict>"                                                     << newLine
                   << "\t" << "<key>BuildSystemType</key>"                         << newLine
                   << "\t" << "<string>Original</string>"                          << newLine
                   << "\t" << "<key>DisableBuildSystemDeprecationWarning</key>"    << newLine
                   << "\t" << "<true/>"                                            << newLine
                   << "\t" << "<key>DisableBuildSystemDeprecationDiagnostic</key>" << newLine
                   << "\t" << "<true/>"                                            << newLine
                   << "</dict>"                                                    << newLine
                   << "</plist>"                                                   << newLine;
            });
        }
        else
        {
            settingsFile.deleteFile();
        }
    }

    void writeInfoPlistFiles() const
    {
        for (auto& target : targets)
           target->writeInfoPlistFile();
    }

    // Delete .rsrc files in folder but don't follow sym-links
    void deleteRsrcFiles (const File& folder) const
    {
        for (const auto& di : RangedDirectoryIterator (folder, false, "*", File::findFilesAndDirectories))
        {
            const auto& entry = di.getFile();

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

    String getSearchPathForStaticLibrary (const build_tools::RelativePath& library) const
    {
        auto searchPath = library.toUnixStyle().upToLastOccurrenceOf ("/", false, false);

        if (! library.isAbsolute())
        {
            auto srcRoot = rebaseFromProjectFolderToBuildTarget (build_tools::RelativePath (".", build_tools::RelativePath::projectFolder)).toUnixStyle();

            if (srcRoot.endsWith ("/."))      srcRoot = srcRoot.dropLastCharacters (2);
            if (! srcRoot.endsWithChar ('/')) srcRoot << '/';

            searchPath = srcRoot + searchPath;
        }

        return expandPath (searchPath);
    }

    bool isUsingDefaultSigningIdentity (const XcodeBuildConfiguration& config) const
    {
        return config.getCodeSignIdentityString().isEmpty() && getDevelopmentTeamIDString().isNotEmpty();
    }

    String getCodeSigningIdentity (const XcodeBuildConfiguration& config) const
    {
        if (isUsingDefaultSigningIdentity (config))
            return iOS ? "iPhone Developer" : "Mac Developer";

        return config.getCodeSignIdentityString();
    }

    void addCodeSigningIdentity (const XcodeBuildConfiguration& config, StringPairArray& result) const
    {
        if (const auto codeSigningIdentity = getCodeSigningIdentity (config); codeSigningIdentity.isNotEmpty())
            result.set (iOS ? "\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\"" : "CODE_SIGN_IDENTITY",
                        codeSigningIdentity.quoted());
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
        s.set ("GCC_INLINES_ARE_PRIVATE_EXTERN", projectType.isStaticLibrary() ? "NO" : "YES");

        // GCC_SYMBOLS_PRIVATE_EXTERN only takes effect if ENABLE_TESTABILITY is off
        s.set ("ENABLE_TESTABILITY", "NO");
        s.set ("GCC_SYMBOLS_PRIVATE_EXTERN", "YES");

        if (config.isDebug())
        {
            if (config.getMacOSArchitectureString() == macOSArch_Default)
                s.set ("ONLY_ACTIVE_ARCH", "YES");
        }

        addCodeSigningIdentity (config, s);

        if (iOS)
        {
            s.set ("SDKROOT", "iphoneos" + config.getiOSBaseSDKString());
            s.set ("TARGETED_DEVICE_FAMILY", getDeviceFamilyString().quoted());
            s.set ("IPHONEOS_DEPLOYMENT_TARGET", config.getiOSDeploymentTargetString());
        }
        else
        {
            s.set ("SDKROOT", "macosx" + config.getMacOSBaseSDKString());
            s.set ("OTHER_CODE_SIGN_FLAGS", "--timestamp");
        }

        s.set ("ZERO_LINK", "NO");

        if (xcodeCanUseDwarf)
            s.set ("DEBUG_INFORMATION_FORMAT", "dwarf");

        s.set ("PRODUCT_NAME", replacePreprocessorTokens (config, config.getTargetBinaryNameString()).quoted());

        return s;
    }

    template<typename AddFrameworkFn>
    void addFrameworkList (const String& frameworksString, AddFrameworkFn&& addFrameworkFn) const
    {
        auto frameworks = StringArray::fromTokens (frameworksString, "\n\r", "\"'");
        frameworks.trim();

        for (auto& framework : frameworks)
        {
            auto frameworkID = addFrameworkFn (framework);

            for (auto& target : targets)
            {
                target->frameworkIDs.add (frameworkID);
                target->frameworkNames.add (framework);
            }
        }
    }

    void addFrameworks() const
    {
        if (! projectType.isStaticLibrary())
        {
            if (isInAppPurchasesEnabled())
                xcodeFrameworks.addIfNotAlreadyThere ("StoreKit");

            if (iOS)
            {
                if (isPushNotificationsEnabled())
                    xcodeFrameworks.addIfNotAlreadyThere ("UserNotifications");

                if (project.getEnabledModules().isModuleEnabled ("juce_video")
                    && project.isConfigFlagEnabled ("JUCE_USE_CAMERA", false))
                {
                    xcodeFrameworks.addIfNotAlreadyThere ("ImageIO");
                }
            }

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

            // When building against the 10.15 SDK we need to make sure the
            // AudioUnit framework is linked before the AudioToolbox framework.
            auto audioUnitIndex = s.indexOf ("AudioUnit", false, 1);

            if (audioUnitIndex != -1)
            {
                s.remove (audioUnitIndex);
                s.insert (0, "AudioUnit");
            }

            for (const auto& [frameworkList, kind] : { std::tuple (&s,                   FrameworkKind::normal),
                                                       std::tuple (&xcodeWeakFrameworks, FrameworkKind::weak) })
            {
                auto cleaned = *frameworkList;
                cleaned.trim();
                cleaned.removeDuplicates (true);

                for (auto& framework : cleaned)
                {
                    auto frameworkID = addFramework (framework, kind);

                    // find all the targets that are referring to this object
                    for (auto& target : targets)
                    {
                        if (xcodeFrameworks.contains (framework)
                            || xcodeWeakFrameworks.contains (framework)
                            || target->xcodeFrameworks.contains (framework))
                        {
                            target->frameworkIDs.add (frameworkID);
                            target->frameworkNames.add (framework);
                        }
                    }
                }
            }
        }

        addFrameworkList (getExtraCustomFrameworksString(),
                          [this] (const String& framework) { return addCustomFramework (framework); });

        addFrameworkList (getEmbeddedFrameworksString(),
                          [this] (const String& framework)
                          {
                              auto frameworkId = addEmbeddedFramework (framework);
                              embeddedFrameworkIDs.add (frameworkId);

                              return frameworkId;
                          });

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
            addCustomResourceFolder (build_tools::RelativePath { crf, build_tools::RelativePath::projectFolder });
    }

    void addSubprojects() const
    {
        auto subprojectLines = StringArray::fromLines (getSubprojectsString());
        subprojectLines.removeEmptyStrings (true);

        struct SubprojectInfo
        {
            String path;
            StringArray buildProducts;
        };

        std::vector<SubprojectInfo> subprojects;

        for (auto& line : subprojectLines)
        {
            String subprojectPath (line.upToFirstOccurrenceOf (":", false, false));

            if (! subprojectPath.endsWith (".xcodeproj"))
                subprojectPath << ".xcodeproj";

            StringArray requestedBuildProducts (StringArray::fromTokens (line.fromFirstOccurrenceOf (":", false, false), ",;|", "\"'"));
            requestedBuildProducts.trim();
            subprojects.push_back ({ subprojectPath, requestedBuildProducts });
        }

        for (const auto& subprojectInfo : subprojects)
        {
            auto subprojectFile = getTargetFolder().getChildFile (subprojectInfo.path);

            if (! subprojectFile.isDirectory())
                continue;

            auto availableBuildProducts = XcodeProjectParser::parseBuildProducts (subprojectFile);

            if (! subprojectInfo.buildProducts.isEmpty())
            {
                auto newEnd = std::remove_if (availableBuildProducts.begin(), availableBuildProducts.end(),
                                              [&subprojectInfo] (const XcodeProjectParser::BuildProduct& item)
                                              {
                                                  return ! subprojectInfo.buildProducts.contains (item.name);
                                              });
                availableBuildProducts.erase (newEnd, availableBuildProducts.end());
            }

            if (availableBuildProducts.empty())
                continue;

            auto subprojectPath = build_tools::RelativePath (subprojectFile,
                                                             getTargetFolder(),
                                                             build_tools::RelativePath::buildTargetFolder).toUnixStyle();

            auto subprojectFileType = getFileType (subprojectPath);
            auto subprojectFileID = addFileOrFolderReference (subprojectPath, "<group>", subprojectFileType);
            subprojectFileIDs.add (subprojectFileID);

            StringArray productIDs;

            for (auto& buildProduct : availableBuildProducts)
            {
                auto buildProductFileType = getFileType (buildProduct.path);

                auto dependencyProxyID = addContainerItemProxy (subprojectFileID, buildProduct.name, "1");
                auto dependencyID = addTargetDependency (dependencyProxyID, buildProduct.name);
                subprojectDependencyIDs.add (dependencyID);

                auto containerItemProxyReferenceID = addContainerItemProxy (subprojectFileID, buildProduct.name, "2");
                auto proxyID = addReferenceProxy (containerItemProxyReferenceID, buildProduct.path, buildProductFileType);
                productIDs.add (proxyID);

                if (StringArray { "archive.ar", "compiled.mach-o.dylib", "wrapper.framework" }.contains (buildProductFileType))
                {
                    auto buildFileID = addBuildFile (FileOptions().withPath (buildProduct.path)
                                                                  .withFileRefID (proxyID)
                                                                  .withInhibitWarningsEnabled (true));

                    for (auto& target : targets)
                        target->frameworkIDs.add (buildFileID);

                    if (buildProductFileType == "wrapper.framework")
                    {
                        auto fileID = createID (subprojectPath + "_" + buildProduct.path + "_framework_buildref");

                        ValueTree v (fileID + " /* " + buildProduct.path + " */");
                        v.setProperty ("isa", "PBXBuildFile", nullptr);
                        v.setProperty ("fileRef", proxyID, nullptr);
                        v.setProperty ("settings", "{ATTRIBUTES = (CodeSignOnCopy, RemoveHeadersOnCopy, ); }", nullptr);

                        addObject (v);

                        embeddedFrameworkIDs.add (fileID);
                    }
                }
            }

            auto productGroupID = createFileRefID (subprojectFile.getFullPathName() + "_products");
            addGroup (productGroupID, "Products", productIDs);

            subprojectReferences.add ({ productGroupID, subprojectFileID });
        }
    }

    void addXcassets() const
    {
        if (const auto customXcassetsPath = getCustomXcassetsFolder())
            addCustomResourceFolder (*customXcassetsPath, "folder.assetcatalog");
        else
            addDefaultXcassetsFolders();
    }

    File makeFile (const build_tools::RelativePath& path) const
    {
        switch (path.getRoot())
        {
            case build_tools::RelativePath::projectFolder:
                return getProject().getProjectFolder().getChildFile (path.toUnixStyle());

            case build_tools::RelativePath::buildTargetFolder:
                return getTargetFolder().getChildFile (path.toUnixStyle());

            case build_tools::RelativePath::unknown:
                jassertfalse;
        }

        return {};
    }

    bool customXcassetsFolderContainsLaunchImage() const
    {
        if (const auto xcassetsFolder = getCustomXcassetsFolder())
            return makeFile (*xcassetsFolder).getChildFile ("LaunchImage.launchimage").exists();

        return false;
    }

    void addCustomResourceFolder (const build_tools::RelativePath& path, const String fileType = "folder") const
    {
        jassert (path.getRoot() == build_tools::RelativePath::projectFolder);

        auto folderPath = path.rebased (projectFolder, getTargetFolder(), build_tools::RelativePath::buildTargetFolder)
                              .toUnixStyle();

        auto fileRefID = createFileRefID (folderPath);

        addFileOrFolderReference (folderPath, "<group>", fileType);

        resourceIDs.add (addBuildFile (FileOptions().withPath (folderPath)
                                                    .withFileRefID (fileRefID)));

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

        StringArray objectTypes;

        for (auto it : objects)
            objectTypes.add (it.getType().toString());

        objectTypes.sort (false);

        for (const auto& objectType : objectTypes)
        {
            auto objectsWithType = objects.getChildWithName (objectType);
            auto requiresSingleLine = objectType == "PBXBuildFile" || objectType == "PBXFileReference";

            output << "\n/* Begin " << objectType << " section */\n";

            for (const auto& o : objectsWithType)
            {
                auto label = [&o]() -> String
                {
                    if (auto* objName = o.getPropertyPointer ("name"))
                        return " /* " + objName->toString() + " */";

                    return {};
                }();

                output << "\t\t" << o.getType().toString() << label << " = {";

                if (! requiresSingleLine)
                    output << "\n";

                for (int j = 0; j < o.getNumProperties(); ++j)
                {
                    auto propertyName = o.getPropertyName (j);
                    auto val = o.getProperty (propertyName).toString();

                    if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-@~\r\n\\#%^`*!")
                                            && ! (val.trimStart().startsWithChar ('(')
                                                    || val.trimStart().startsWithChar ('{'))))
                        val = "\"" + val + "\"";

                    auto content = propertyName.toString() + " = " + val + ";";

                    if (requiresSingleLine)
                        content = content + " ";
                    else
                        content = "\t\t\t" + content + "\n";

                    output << content;
                }

                if (! requiresSingleLine)
                    output << "\t\t";

                output << "};\n";
            }

            output << "/* End " << objectType << " section */\n";
        }

        output << "\t};\n\trootObject = " << createID ("__root") << " /* Project object */;\n}\n";
    }

    String addFileReference (String pathString, const String& fileType = {}) const
    {
        String sourceTree ("SOURCE_ROOT");
        build_tools::RelativePath path (pathString, build_tools::RelativePath::unknown);

        if (pathString.startsWith ("${"))
        {
            sourceTree = pathString.substring (2).upToFirstOccurrenceOf ("}", false, false);
            pathString = pathString.fromFirstOccurrenceOf ("}/", false, false);
        }
        else if (path.isAbsolute())
        {
            sourceTree = "<absolute>";
        }

        return addFileOrFolderReference (pathString, sourceTree, fileType.isEmpty() ? getFileType (pathString) : fileType);
    }

    String addFileOrFolderReference (const String& pathString, const String& sourceTree, const String& fileType) const
    {
        auto fileRefID = createFileRefID (pathString);
        auto filename = build_tools::RelativePath (pathString, build_tools::RelativePath::unknown).getFileName();

        ValueTree v (fileRefID + " /* " + filename + " */");
        v.setProperty ("isa", "PBXFileReference", nullptr);
        v.setProperty ("lastKnownFileType", fileType, nullptr);
        v.setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), nullptr);
        v.setProperty ("path", pathString, nullptr);
        v.setProperty ("sourceTree", sourceTree, nullptr);

        addObject (v);

        return fileRefID;
    }

    String addContainerItemProxy (const String& subprojectID, const String& itemName, const String& proxyType) const
    {
        auto uniqueString = subprojectID + "_" + itemName + "_" + proxyType;
        auto objectID = createFileRefID (uniqueString);

        ValueTree v (objectID + " /* PBXContainerItemProxy */");
        v.setProperty ("isa", "PBXContainerItemProxy", nullptr);
        v.setProperty ("containerPortal", subprojectID, nullptr);
        v.setProperty ("proxyType", proxyType, nullptr);
        v.setProperty ("remoteGlobalIDString", createFileRefID (uniqueString + "_global"), nullptr);
        v.setProperty ("remoteInfo", itemName, nullptr);

        addObject (v);

        return objectID;
    }

    String addTargetDependency (const String& proxyID, const String& itemName) const
    {
        auto objectID = createFileRefID (proxyID + "_" + itemName + "_PBXTargetDependency");

        ValueTree v (objectID);
        v.setProperty ("isa", "PBXTargetDependency", nullptr);
        v.setProperty ("name", itemName, nullptr);
        v.setProperty ("targetProxy", proxyID, nullptr);

        addObject (v);

        return objectID;
    }

    String addReferenceProxy (const String& remoteRef, const String& path, const String& fileType) const
    {
        auto objectID = createFileRefID (remoteRef + "_" + path);

        ValueTree v (objectID + " /* " + path + " */");
        v.setProperty ("isa", "PBXReferenceProxy", nullptr);
        v.setProperty ("fileType", fileType, nullptr);
        v.setProperty ("path", path, nullptr);
        v.setProperty ("remoteRef", remoteRef, nullptr);
        v.setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", nullptr);

        addObject (v);

        return objectID;
    }

private:
    struct FileOptions
    {
        FileOptions& withPath (const String& p)                             { path = p;                  return *this; }
        FileOptions& withRelativePath (const build_tools::RelativePath& p)  { path = p.toUnixStyle();    return *this; }
        FileOptions& withFileRefID (const String& fid)                      { fileRefID = fid;           return *this; }
        FileOptions& withCompilerFlags (const String& f)                    { compilerFlags = f;         return *this; }
        FileOptions& withCompilationEnabled (bool e)                        { compile = e;               return *this; }
        FileOptions& withAddToBinaryResourcesEnabled (bool e)               { addToBinaryResources = e;  return *this; }
        FileOptions& withAddToXcodeResourcesEnabled (bool e)                { addToXcodeResources = e;   return *this; }
        FileOptions& withInhibitWarningsEnabled (bool e)                    { inhibitWarnings = e;       return *this; }
        FileOptions& withSkipPCHEnabled (bool e)                            { skipPCH = e;               return *this; }
        FileOptions& withXcodeTarget (XcodeTarget* t)                       { xcodeTarget = t;           return *this; }
        FileOptions& withAttributeWeak (bool w)                             { weak = w;                  return *this; }

        String path;
        String fileRefID;
        String compilerFlags;
        bool compile = false;
        bool addToBinaryResources = false;
        bool addToXcodeResources = false;
        bool inhibitWarnings = false;
        bool skipPCH = false;
        bool weak = false;
        XcodeTarget* xcodeTarget = nullptr;
    };

    static String getFileType (const String& filePath)
    {
        build_tools::RelativePath file (filePath, build_tools::RelativePath::unknown);

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
        if (file.hasFileExtension ("dylib"))                return "compiled.mach-o.dylib";
        if (file.hasFileExtension ("xcassets"))             return "folder.assetcatalog";

        return "file" + file.getFileExtension();
    }

    String addFile (const FileOptions& opts) const
    {
        auto refID = addFileReference (opts.path);

        if (opts.compile || opts.addToXcodeResources)
        {
            auto fileID = addBuildFile (FileOptions (opts).withFileRefID (refID));

            if (opts.addToXcodeResources)
            {
                resourceIDs.add (fileID);
                resourceFileRefs.add (refID);
            }
        }

        return refID;
    }

    String addBuildFile (const FileOptions& opts) const
    {
        auto fileID = createID (opts.path + "buildref");
        auto filename = build_tools::RelativePath (opts.path, build_tools::RelativePath::unknown).getFileName();

        if (opts.compile)
        {
            if (opts.xcodeTarget != nullptr)
                opts.xcodeTarget->sourceIDs.add (fileID);
            else
                sourceIDs.add (fileID);
        }

        ValueTree v (fileID + " /* " + filename + " */");
        v.setProperty ("isa", "PBXBuildFile", nullptr);
        auto fileRefID = opts.fileRefID.isEmpty() ? createFileRefID (opts.path)
                                                  : opts.fileRefID;
        v.setProperty ("fileRef", fileRefID, nullptr);

        auto compilerFlags = [&opts]
        {
            return (opts.compilerFlags
                    + (opts.inhibitWarnings ? " -w" : String())
                    + (opts.skipPCH ? " -D" + BuildConfiguration::getSkipPrecompiledHeaderDefine() : String())).trim();
        }();

        const auto compilerFlagSetting = compilerFlags.isNotEmpty() ? (" COMPILER_FLAGS = \"" + compilerFlags + "\"; ") : "";
        const auto attributeSetting = opts.weak ? " ATTRIBUTES = (Weak, ); " : "";
        const auto settingsString = compilerFlagSetting + attributeSetting;

        if (settingsString.isNotEmpty())
            v.setProperty ("settings", "{" + settingsString + "}", nullptr);

        addObject (v);

        return fileID;
    }

    String addRezFile (const Project::Item& projectItem, const build_tools::RelativePath& path) const
    {
        auto refID = addFileReference (path.toUnixStyle());

        if (projectItem.isModuleCode())
        {
            if (auto* xcodeTarget = getTargetOfType (getProject().getTargetTypeFromFilePath (projectItem.getFile(), false)))
            {
                auto rezFileID = addBuildFile (FileOptions().withRelativePath (path)
                                                            .withFileRefID (refID)
                                                            .withXcodeTarget (xcodeTarget));

                xcodeTarget->rezFileIDs.add (rezFileID);

                return refID;
            }
        }

        return {};
    }

    void addEntitlementsFile (XcodeTarget& target) const
    {
        build_tools::EntitlementOptions options;

        options.type                            = target.type;
        options.isiOS                           = isiOS();
        options.isAudioPluginProject            = project.isAudioPluginProject();
        options.shouldEnableIAA                 = project.shouldEnableIAA();
        options.isAUPluginHost                  = project.isAUPluginHost();
        options.isiCloudPermissionsEnabled      = isiCloudPermissionsEnabled();
        options.isPushNotificationsEnabled      = isPushNotificationsEnabled();
        options.isAppGroupsEnabled              = isAppGroupsEnabled();
        options.isHardenedRuntimeEnabled        = isHardenedRuntimeEnabled();
        options.isAppSandboxEnabled             = isAppSandboxEnabled();
        options.isAppSandboxInhertianceEnabled  = isAppSandboxInhertianceEnabled();
        options.isNetworkingMulticastEnabled    = isNetworkingMulticastEnabled();
        options.appGroupIdString                = getAppGroupIdString();
        options.hardenedRuntimeOptions          = getHardenedRuntimeOptions();
        options.appSandboxOptions               = getAppSandboxOptions();
        options.appSandboxTemporaryPaths        = getAppSandboxTemporaryPaths();
        options.appSandboxExceptionIOKit        = getAppSandboxExceptionIOKitClasses();

        const auto entitlementsFile = getTargetFolder().getChildFile (target.getEntitlementsFilename());
        build_tools::overwriteFileIfDifferentOrThrow (entitlementsFile, options.getEntitlementsFileContent());

        build_tools::RelativePath entitlementsPath (entitlementsFile, getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
        addFile (FileOptions().withRelativePath (entitlementsPath));
    }

    String addProjectItem (const Project::Item& projectItem) const
    {
        if (modulesGroup != nullptr && projectItem.getParent() == *modulesGroup)
            return addFileReference (rebaseFromProjectFolderToBuildTarget (getModuleFolderRelativeToProject (projectItem.getName())).toUnixStyle(),
                                     "folder");

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
            build_tools::RelativePath path;

            if (itemPath.startsWith ("${") || build_tools::isAbsolutePath (itemPath))
                path = build_tools::RelativePath (itemPath, build_tools::RelativePath::unknown);
            else
                path = build_tools::RelativePath (projectItem.getFile(), getTargetFolder(), build_tools::RelativePath::buildTargetFolder);

            if (path.hasFileExtension (".r"))
                return addRezFile (projectItem, path);

            XcodeTarget* xcodeTarget = nullptr;
            if (projectItem.isModuleCode() && projectItem.shouldBeCompiled())
                xcodeTarget = getTargetOfType (project.getTargetTypeFromFilePath (projectItem.getFile(), false));

            return addFile (FileOptions().withRelativePath (path)
                                         .withCompilerFlags (getCompilerFlagsForProjectItem (projectItem))
                                         .withCompilationEnabled (projectItem.shouldBeCompiled())
                                         .withAddToBinaryResourcesEnabled (projectItem.shouldBeAddedToBinaryResources())
                                         .withAddToXcodeResourcesEnabled (projectItem.shouldBeAddedToXcodeResources())
                                         .withInhibitWarningsEnabled (projectItem.shouldInhibitWarnings())
                                         .withSkipPCHEnabled (isPCHEnabledForAnyConfigurations() && projectItem.shouldSkipPCH())
                                         .withXcodeTarget (xcodeTarget));
        }

        return {};
    }

    enum class FrameworkKind
    {
        normal,
        weak,
    };

    String addFramework (const String& frameworkName, FrameworkKind kind) const
    {
        auto path = frameworkName;
        auto isRelativePath = path.startsWith ("../");

        if (! build_tools::isAbsolutePath (path) && ! isRelativePath)
            path = "System/Library/Frameworks/" + path;

        if (! path.endsWithIgnoreCase (".framework"))
            path << ".framework";

        auto fileRefID = createFileRefID (path);

        addFileReference (((build_tools::isAbsolutePath (frameworkName) || isRelativePath) ? "" : "${SDKROOT}/") + path);
        frameworkFileIDs.add (fileRefID);

        return addBuildFile (FileOptions().withPath (path)
                                          .withFileRefID (fileRefID)
                                          .withAttributeWeak (kind == FrameworkKind::weak));
    }

    String addCustomFramework (String frameworkPath) const
    {
        if (! frameworkPath.endsWithIgnoreCase (".framework"))
            frameworkPath << ".framework";

        auto fileRefID = createFileRefID (frameworkPath);

        auto fileType = getFileType (frameworkPath);
        addFileOrFolderReference (frameworkPath, "<group>", fileType);

        frameworkFileIDs.add (fileRefID);

        return addBuildFile (FileOptions().withPath (frameworkPath)
                                          .withFileRefID (fileRefID));
    }

    String addEmbeddedFramework (const String& path) const
    {
        auto fileRefID = createFileRefID (path);
        auto filename = build_tools::RelativePath (path, build_tools::RelativePath::unknown).getFileName();

        auto fileType = getFileType (path);
        addFileOrFolderReference (path, "<group>", fileType);

        auto fileID = createID (path + "buildref");

        ValueTree v (fileID + " /* " + filename + " */");
        v.setProperty ("isa", "PBXBuildFile", nullptr);
        v.setProperty ("fileRef", fileRefID, nullptr);
        v.setProperty ("settings", "{ ATTRIBUTES = (CodeSignOnCopy, RemoveHeadersOnCopy, ); }", nullptr);

        addObject (v);

        frameworkFileIDs.add (fileRefID);

        return fileID;
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs) const
    {
        ValueTree v (groupID);
        v.setProperty ("isa", "PBXGroup", nullptr);
        v.setProperty ("children", indentParenthesisedList (childIDs), nullptr);
        v.setProperty (Ids::name, groupName, nullptr);
        v.setProperty ("sourceTree", "<group>", nullptr);

        addObject (v);
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
        ValueTree v (createID ("projectconfigid_" + configName));
        v.setProperty ("isa", "XCBuildConfiguration", nullptr);
        v.setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
        v.setProperty (Ids::name, configName, nullptr);

        addObject (v);
    }

    void addConfigList (XcodeTarget& target, const String& listID) const
    {
        ValueTree v (listID);
        v.setProperty ("isa", "XCConfigurationList", nullptr);
        v.setProperty ("buildConfigurations", indentParenthesisedList (target.configIDs), nullptr);
        v.setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);
        v.setProperty ("defaultConfigurationName", getConfiguration (0)->getName(), nullptr);

        addObject (v);
    }

    void addProjectConfigList (const String& listID) const
    {
        auto buildConfigs = objects.getChildWithName ("XCBuildConfiguration");
        jassert (buildConfigs.isValid());

        StringArray configIDs;

        for (const auto& child : buildConfigs)
            configIDs.add (child.getType().toString());

        ValueTree v (listID);
        v.setProperty ("isa", "XCConfigurationList", nullptr);
        v.setProperty ("buildConfigurations", indentParenthesisedList (configIDs), nullptr);
        v.setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);
        v.setProperty ("defaultConfigurationName", getConfiguration (0)->getName(), nullptr);

        addObject (v);
    }

    void addProjectObject() const
    {
        ValueTree v (createID ("__root"));
        v.setProperty ("isa", "PBXProject", nullptr);
        v.setProperty ("attributes", indentBracedList (getProjectObjectAttributes()), nullptr);
        v.setProperty ("buildConfigurationList", createID ("__projList"), nullptr);
        v.setProperty ("compatibilityVersion", "Xcode 3.2", nullptr);
        v.setProperty ("hasScannedForEncodings", (int) 0, nullptr);
        v.setProperty ("knownRegions", indentParenthesisedList ({ "en", "Base" }), nullptr);
        v.setProperty ("mainGroup", createID ("__mainsourcegroup"), nullptr);
        v.setProperty ("projectDirPath", "\"\"", nullptr);

        if (! subprojectReferences.isEmpty())
        {
            StringArray projectReferences;

            for (auto& reference : subprojectReferences)
                projectReferences.add (indentBracedList ({ "ProductGroup = " + reference.productGroup, "ProjectRef = " + reference.projectRef }, 1));

            v.setProperty ("projectReferences", indentParenthesisedList (projectReferences), nullptr);
        }

        v.setProperty ("projectRoot", "\"\"", nullptr);

        v.setProperty ("targets", indentParenthesisedList (targetIDs), nullptr);

        addObject (v);
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
        for (auto* schemesKey : dictXML.getChildWithTagNameIterator ("key"))
        {
            if (schemesKey->getAllSubText().trim().equalsIgnoreCase ("SchemeUserState"))
            {
                if (auto* dict = schemesKey->getNextElement())
                {
                    if (dict->hasTagName ("dict"))
                    {
                        StringArray names;

                        for (auto* key : dict->getChildWithTagNameIterator ("key"))
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

    StringArray getProjectObjectAttributes() const
    {
        std::map<String, String> attributes;

        attributes["LastUpgradeCheck"] = "1340";
        attributes["BuildIndependentTargetsInParallel"] = "YES";
        attributes["ORGANIZATIONNAME"] = getProject().getCompanyNameString().quoted();

        if (projectType.isGUIApplication() || projectType.isAudioPlugin())
        {
            StringArray targetAttributes;

            for (auto& target : targets)
                targetAttributes.add (target->getTargetAttributes());

            attributes["TargetAttributes"] = indentBracedList (targetAttributes, 1);
        }

        StringArray result;

        for (const auto& attrib : attributes)
            result.add (attrib.first + " = " + attrib.second);

        return result;
    }

    //==============================================================================
    void writeDefaultLaunchStoryboardFile() const
    {
        const auto storyboardFile = getTargetFolder().getChildFile (getDefaultLaunchStoryboardName() + ".storyboard");

        build_tools::writeStreamToFile (storyboardFile, [&] (MemoryOutputStream& mo)
        {
            mo << String (BinaryData::LaunchScreen_storyboard);
        });

        addLaunchStoryboardFileReference (build_tools::RelativePath (storyboardFile,
                                                                     getTargetFolder(),
                                                                     build_tools::RelativePath::buildTargetFolder));
    }

    void addLaunchStoryboardFileReference (const build_tools::RelativePath& relativePath) const
    {
        auto path = relativePath.toUnixStyle();

        auto refID  = addFileReference (path);
        auto fileID = addBuildFile (FileOptions().withPath (path)
                                                 .withFileRefID (refID));

        resourceIDs.add (fileID);
        resourceFileRefs.add (refID);
    }

    void addDefaultXcassetsFolders() const
    {
        const auto assetsPath = build_tools::createXcassetsFolderFromIcons (getIcons(),
                                                                            getTargetFolder(),
                                                                            project.getProjectFilenameRootString());
        addFileReference (assetsPath.toUnixStyle());
        resourceIDs.add (addBuildFile (FileOptions().withRelativePath (assetsPath)));
        resourceFileRefs.add (createFileRefID (assetsPath));
    }

    //==============================================================================
    static String indentBracedList        (const StringArray& list, int depth = 0) { return indentList (list, '{', '}', ";", depth, true); }
    static String indentParenthesisedList (const StringArray& list, int depth = 0) { return indentList (list, '(', ')', ",", depth, false); }

    static String indentList (StringArray list, char openBracket, char closeBracket, const String& separator, int extraTabs, bool shouldSort)
    {
        auto content = [extraTabs, shouldSort, &list, &separator]() -> String
        {
            if (list.isEmpty())
                return "";

            if (shouldSort)
                list.sort (true);

            auto tabs = String::repeatedString ("\t", extraTabs + 4);
            return tabs + list.joinIntoString (separator + "\n" + tabs) + separator + "\n";
        }();

        return openBracket + String ("\n")
            + content
            + String::repeatedString ("\t", extraTabs + 3) + closeBracket;
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        rootString += project.getProjectUIDString();

        return MD5 (rootString.toUTF8()).toHexString().substring (0, 24).toUpperCase();
    }

    String createFileRefID (const build_tools::RelativePath& path) const { return createFileRefID (path.toUnixStyle()); }
    String createFileRefID (const String& path) const                    { return createID ("__fileref_" + path); }
    String getIDForGroup (const Project::Item& item) const               { return createID (item.getID()); }

    bool shouldFileBeCompiledByDefault (const File& file) const override
    {
        return file.hasFileExtension (sourceFileExtensions);
    }

    //==============================================================================
    void updateOldOrientationSettings()
    {
        jassert (iOS);

        StringArray orientationSettingStrings { getSetting (Ids::iPhoneScreenOrientation).getValue().toString(),
                                                getSetting (Ids::iPadScreenOrientation).getValue().toString() };

        for (int i = 0; i < 2; ++i)
        {
            auto& settingsString = orientationSettingStrings[i];

            if (settingsString.isNotEmpty())
            {
                Array<var> orientations;

                if (settingsString.contains ("portrait"))   orientations.add ("UIInterfaceOrientationPortrait");
                if (settingsString.contains ("landscape"))  orientations.addArray ({ "UIInterfaceOrientationLandscapeLeft",
                                                                                     "UIInterfaceOrientationLandscapeRight" });

                if (! orientations.isEmpty())
                {
                    if (i == 0)
                        iPhoneScreenOrientationValue = orientations;
                    else
                        iPadScreenOrientationValue = orientations;
                }
            }
        }
    }

    void addObject (ValueTree data) const
    {
        if (auto* type = data.getPropertyPointer ("isa"))
        {
            auto objs = objects.getOrCreateChildWithName (type->toString(), nullptr);
            auto objectID = data.getType();
            auto numChildren = objs.getNumChildren();

            for (int i = 0; i < numChildren; ++i)
            {
                auto obj = objs.getChild (i);
                auto childID = obj.getType();

                if (objectID < childID)
                {
                    objs.addChild (data, i, nullptr);
                    return;
                }

                if (objectID == childID)
                {
                    jassert (obj.isEquivalentTo (data));
                    return;
                }
            }

            objs.appendChild (data, nullptr);
            return;
        }

        jassertfalse;
    }

    //==============================================================================
    bool xcodeCanUseDwarf;
    OwnedArray<XcodeTarget> targets;

    mutable ValueTree objects { "objects" };

    mutable StringArray resourceIDs, sourceIDs, targetIDs, frameworkFileIDs, embeddedFrameworkIDs,
                        rezFileIDs, resourceFileRefs, subprojectFileIDs, subprojectDependencyIDs;

    struct SubprojectReferenceInfo
    {
        String productGroup, projectRef;
    };

    mutable Array<SubprojectReferenceInfo> subprojectReferences;
    mutable File menuNibFile, iconFile;
    mutable StringArray buildProducts;

    const bool iOS;

    ValueTreePropertyWithDefault applicationCategoryValue,
                                 customPListValue, pListPrefixHeaderValue, pListPreprocessValue,
                                 subprojectsValue,
                                 validArchsValue,
                                 extraFrameworksValue, frameworkSearchPathsValue, extraCustomFrameworksValue, embeddedFrameworksValue,
                                 postbuildCommandValue, prebuildCommandValue,
                                 duplicateAppExResourcesFolderValue, iosDeviceFamilyValue, iPhoneScreenOrientationValue,
                                 iPadScreenOrientationValue, customXcodeResourceFoldersValue, customXcassetsFolderValue,
                                 appSandboxValue, appSandboxInheritanceValue, appSandboxOptionsValue,
                                 appSandboxHomeDirROValue, appSandboxHomeDirRWValue, appSandboxAbsDirROValue, appSandboxAbsDirRWValue,
                                 appSandboxExceptionIOKitValue,
                                 hardenedRuntimeValue, hardenedRuntimeOptionsValue,
                                 microphonePermissionNeededValue, microphonePermissionsTextValue,
                                 cameraPermissionNeededValue, cameraPermissionTextValue,
                                 bluetoothPermissionNeededValue, bluetoothPermissionTextValue,
                                 sendAppleEventsPermissionNeededValue, sendAppleEventsPermissionTextValue,
                                 uiFileSharingEnabledValue, uiSupportsDocumentBrowserValue, uiStatusBarHiddenValue, uiRequiresFullScreenValue, documentExtensionsValue, iosInAppPurchasesValue,
                                 iosContentSharingValue, iosBackgroundAudioValue, iosBackgroundBleValue, iosPushNotificationsValue, iosAppGroupsValue, iCloudPermissionsValue,
                                 networkingMulticastValue, iosDevelopmentTeamIDValue, iosAppGroupsIDValue, keepCustomXcodeSchemesValue, useHeaderMapValue, customLaunchStoryboardValue,
                                 exporterBundleIdentifierValue, suppressPlistResourceUsageValue, useLegacyBuildSystemValue, buildNumber;

    struct SandboxFileAccessProperty
    {
        const ValueTreePropertyWithDefault& property;
        const String label, key;
    };

    const std::vector<SandboxFileAccessProperty> sandboxFileAccessProperties
    {
        { appSandboxHomeDirROValue, "App sandbox temporary exception: home directory read only file access",  "home-relative-path.read-only" },
        { appSandboxHomeDirRWValue, "App sandbox temporary exception: home directory read/write file access", "home-relative-path.read-write" },
        { appSandboxAbsDirROValue,  "App sandbox temporary exception: absolute path read only file access",   "absolute-path.read-only" },
        { appSandboxAbsDirRWValue,  "App sandbox temporary exception: absolute path read/write file access",  "absolute-path.read-write" }
    };

    bool userAcknowledgedInvalidPostBuildScript = false;
    bool userAcknowledgedDefunctIOKitSetting    = false;

    ErasedScopeGuard messageBoxQueueListenerScope;
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE (XcodeProjectExporter)
};
