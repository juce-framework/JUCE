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

#include "../Application/jucer_Application.h"
#include "jucer_TextWithDefaultPropertyComponent.h"

namespace
{
    const char* const osxVersionDefault         = "default";
    const int oldestSDKVersion  = 5;
    const int currentSDKVersion = 12;
    const int minimumAUv3SDKVersion = 11;

    const char* const osxArch_Default           = "default";
    const char* const osxArch_Native            = "Native";
    const char* const osxArch_32BitUniversal    = "32BitUniversal";
    const char* const osxArch_64BitUniversal    = "64BitUniversal";
    const char* const osxArch_64Bit             = "64BitIntel";
}

//==============================================================================
class XCodeProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameMac()                         { return "Xcode (MacOSX)"; }
    static const char* getNameiOS()                         { return "Xcode (iOS)"; }
    static const char* getValueTreeTypeName (bool iOS)      { return iOS ? "XCODE_IPHONE" : "XCODE_MAC"; }

    //==============================================================================
    XCodeProjectExporter (Project& p, const ValueTree& t, const bool isIOS)
        : ProjectExporter (p, t),
          xcodeCanUseDwarf (true),
          iOS (isIOS)
    {
        name = iOS ? getNameiOS() : getNameMac();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + (iOS ? "iOS" : "MacOSX");

        initialiseDependencyPathValues();

        if (iOS)
        {
            if (getScreenOrientationValue().toString().isEmpty())
                getScreenOrientationValue() = "portraitlandscape";
        }
    }

    static XCodeProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (false)))  return new XCodeProjectExporter (project, settings, false);
        if (settings.hasType (getValueTreeTypeName (true)))   return new XCodeProjectExporter (project, settings, true);

        return nullptr;
    }

    //==============================================================================
    Value getPListToMergeValue()              { return getSetting ("customPList"); }
    String getPListToMergeString() const      { return settings   ["customPList"]; }

    Value getPListPrefixHeaderValue()         { return getSetting ("PListPrefixHeader"); }
    String getPListPrefixHeaderString() const { return settings   ["PListPrefixHeader"]; }

    Value getPListPreprocessValue()           { return getSetting ("PListPreprocess"); }
    bool  isPListPreprocessEnabled() const    { return settings   ["PListPreprocess"]; }

    Value getExtraFrameworksValue()           { return getSetting (Ids::extraFrameworks); }
    String getExtraFrameworksString() const   { return settings   [Ids::extraFrameworks]; }

    Value  getPostBuildScriptValue()          { return getSetting (Ids::postbuildCommand); }
    String getPostBuildScript() const         { return settings   [Ids::postbuildCommand]; }

    Value  getPreBuildScriptValue()           { return getSetting (Ids::prebuildCommand); }
    String getPreBuildScript() const          { return settings   [Ids::prebuildCommand]; }

    Value getDuplicateResourcesFolderForAppExtensionValue()     { return getSetting (Ids::iosAppExtensionDuplicateResourcesFolder); }
    bool  shouldDuplicateResourcesFolderForAppExtension() const { return settings   [Ids::iosAppExtensionDuplicateResourcesFolder]; }

    Value  getScreenOrientationValue()               { return getSetting (Ids::iosScreenOrientation); }
    String getScreenOrientationString() const        { return settings   [Ids::iosScreenOrientation]; }

    Value getCustomResourceFoldersValue()            { return getSetting (Ids::customXcodeResourceFolders); }
    String getCustomResourceFoldersString() const    { return getSettingString (Ids::customXcodeResourceFolders).replaceCharacters ("\r\n", "::"); }

    Value  getCustomXcassetsFolderValue()            { return getSetting (Ids::customXcassetsFolder); }
    String getCustomXcassetsFolderString() const     { return settings   [Ids::customXcassetsFolder]; }

    Value  getMicrophonePermissionValue()            { return getSetting (Ids::microphonePermissionNeeded); }
    bool   isMicrophonePermissionEnabled() const     { return settings   [Ids::microphonePermissionNeeded]; }
    Value  getInAppPurchasesValue()                  { return getSetting (Ids::iosInAppPurchases); }
    bool   isInAppPurchasesEnabled() const           { return settings   [Ids::iosInAppPurchases]; }
    Value  getBackgroundAudioValue()                 { return getSetting (Ids::iosBackgroundAudio); }
    bool   isBackgroundAudioEnabled() const          { return settings   [Ids::iosBackgroundAudio]; }
    Value  getBackgroundBleValue()                   { return getSetting (Ids::iosBackgroundBle); }
    bool   isBackgroundBleEnabled() const            { return settings   [Ids::iosBackgroundBle]; }
    Value  getPushNotificationsValue()               { return getSetting (Ids::iosPushNotifications); }
    bool   isPushNotificationsEnabled() const        { return settings   [Ids::iosPushNotifications]; }
    Value  getAppGroupsEnabledValue()                { return getSetting (Ids::iosAppGroups); }
    bool   isAppGroupsEnabled() const                { return settings   [Ids::iosAppGroups]; }

    Value  getIosDevelopmentTeamIDValue()            { return getSetting (Ids::iosDevelopmentTeamID); }
    String getIosDevelopmentTeamIDString() const     { return settings   [Ids::iosDevelopmentTeamID]; }
    Value  getAppGroupIdValue()                      { return getSetting (Ids::iosAppGroupsId); }
    String getAppGroupIdString() const               { return settings   [Ids::iosAppGroupsId]; }

    bool usesMMFiles() const override                { return true; }
    bool canCopeWithDuplicateFiles() override        { return true; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                    { return true; }
    bool isVisualStudio() const override             { return false; }
    bool isCodeBlocks() const override               { return false; }
    bool isMakefile() const override                 { return false; }
    bool isAndroidStudio() const override            { return false; }

    bool isAndroid() const override                  { return false; }
    bool isWindows() const override                  { return false; }
    bool isLinux() const override                    { return false; }
    bool isOSX() const override                      { return ! iOS; }
    bool isiOS() const override                      { return iOS; }

    bool supportsTargetType (ProjectType::Target::Type type) const override
    {
        switch (type)
        {
            case ProjectType::Target::AudioUnitv3PlugIn:
            case ProjectType::Target::StandalonePlugIn:
            case ProjectType::Target::GUIApp:
            case ProjectType::Target::StaticLibrary:
            case ProjectType::Target::SharedCodeTarget:
            case ProjectType::Target::AggregateTarget:
                return true;
            case ProjectType::Target::ConsoleApp:
            case ProjectType::Target::VSTPlugIn:
            case ProjectType::Target::VST3PlugIn:
            case ProjectType::Target::AAXPlugIn:
            case ProjectType::Target::RTASPlugIn:
            case ProjectType::Target::AudioUnitPlugIn:
            case ProjectType::Target::DynamicLibrary:
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
            props.add (new TextPropertyComponent (getCustomXcassetsFolderValue(), "Custom Xcassets folder", 128, false),
                       "If this field is not empty, your Xcode project will use the custom xcassets folder specified here "
                       "for the app icons and launchimages, and will ignore the Icon files specified above.");
        }

        props.add (new TextPropertyComponent (getCustomResourceFoldersValue(), "Custom Xcode Resource folders", 8192, true),
                   "You can specify a list of custom resource folders here (separated by newlines or whitespace). "
                   "References to these folders will then be added to the Xcode resources. "
                   "This way you can specify them for OS X and iOS separately, and modify the content of the resource folders "
                   "without re-saving the Projucer project.");

        if (iOS)
        {
            if (getProject().getProjectType().isAudioPlugin())
                props.add (new BooleanPropertyComponent (getDuplicateResourcesFolderForAppExtensionValue(),
                                                         "Don't add resources folder to app extension", "Enabled"),
                           "Enable this to prevent the Projucer from creating a resources folder for AUv3 app extensions.");

            static const char* orientations[] = { "Portrait and Landscape", "Portrait", "Landscape", nullptr };
            static const char* orientationValues[] = { "portraitlandscape", "portrait", "landscape", nullptr };

            props.add (new ChoicePropertyComponent (getScreenOrientationValue(), "Screen orientation",StringArray (orientations), Array<var> (orientationValues)),
                       "The screen orientations that this app should support");

            props.add (new BooleanPropertyComponent (getSetting ("UIFileSharingEnabled"), "File Sharing Enabled", "Enabled"),
                       "Enable this to expose your app's files to iTunes.");

            props.add (new BooleanPropertyComponent (getSetting ("UIStatusBarHidden"), "Status Bar Hidden", "Enabled"),
                       "Enable this to disable the status bar in your app.");

            props.add (new BooleanPropertyComponent (getMicrophonePermissionValue(), "Microphone access", "Enabled"),
                       "Enable this to allow your app to use the microphone. "
                       "The user of your app will be prompted to grant microphone access permissions.");

            props.add (new BooleanPropertyComponent (getInAppPurchasesValue(), "In-App purchases capability", "Enabled"),
                       "Enable this to grant your app the capability for in-app purchases. "
                       "This option requires that you specify a valid Development Team ID.");

            props.add (new BooleanPropertyComponent (getBackgroundAudioValue(), "Audio background capability", "Enabled"),
                       "Enable this to grant your app the capability to access audio when in background mode.");

            props.add (new BooleanPropertyComponent (getBackgroundBleValue(), "Bluetooth MIDI background capability", "Enabled"),
                       "Enable this to grant your app the capability to connect to Bluetooth LE devices when in background mode.");

            props.add (new BooleanPropertyComponent (getPushNotificationsValue(), "Push Notifications capability", "Enabled"),
                       "Enable this to grant your app the capability to receive push notifications.");

            props.add (new BooleanPropertyComponent (getAppGroupsEnabledValue(), "App groups capability", "Enabled"),
                       "Enable this to grant your app the capability to share resources between apps using the same app group ID.");
        }
        else if (projectType.isGUIApplication())
        {
            props.add (new TextPropertyComponent (getSetting ("documentExtensions"), "Document file extensions", 128, false),
                       "A comma-separated list of file extensions for documents that your app can open. "
                       "Using a leading '.' is optional, and the extensions are not case-sensitive.");
        }

        props.add (new TextPropertyComponent (getPListToMergeValue(), "Custom PList", 8192, true),
                   "You can paste the contents of an XML PList file in here, and the settings that it contains will override any "
                   "settings that the Projucer creates. BEWARE! When doing this, be careful to remove from the XML any "
                   "values that you DO want the Projucer to change!");

        props.add (new BooleanPropertyComponent (getPListPreprocessValue(), "PList Preprocess", "Enabled"),
                   "Enable this to preprocess PList file. This will allow you to set values to preprocessor defines,"
                   " for instance if you define: #define MY_FLAG 1 in a prefix header file (see PList prefix header), you can have"
                   " a key with MY_FLAG value and it will be replaced with 1.");

        props.add (new TextPropertyComponent (getPListPrefixHeaderValue(), "PList Prefix Header", 512, false),
                   "Header file containing definitions used in plist file (see PList Preprocess).");

        props.add (new TextPropertyComponent (getExtraFrameworksValue(), "Extra Frameworks", 2048, false),
                   "A comma-separated list of extra frameworks that should be added to the build. "
                   "(Don't include the .framework extension in the name)");

        props.add (new TextPropertyComponent (getPreBuildScriptValue(), "Pre-build shell script", 32768, true),
                   "Some shell-script that will be run before a build starts.");

        props.add (new TextPropertyComponent (getPostBuildScriptValue(), "Post-build shell script", 32768, true),
                   "Some shell-script that will be run after a build completes.");

        props.add (new TextPropertyComponent (getIosDevelopmentTeamIDValue(), "Development Team ID", 10, false),
                   "The Development Team ID to be used for setting up code-signing your iOS app. This is a ten-character "
                   "string (for example, \"S7B6T5XJ2Q\") that describes the distribution certificate Apple issued to you. "
                   "You can find this string in the OS X app Keychain Access under \"Certificates\".");

        if (iOS)
            props.add (new TextPropertyComponentWithEnablement (getAppGroupIdValue(), getAppGroupsEnabledValue(), "App Group ID", 256, false),
                       "The App Group ID to be used for allowing multiple apps to access a shared resource folder. Multiple IDs can be "
                       "added seperated by a semicolon.");

        props.add (new BooleanPropertyComponent (getSetting ("keepCustomXcodeSchemes"), "Keep custom Xcode schemes", "Enabled"),
                   "Enable this to keep any Xcode schemes you have created for debugging or running, e.g. to launch a plug-in in"
                   "various hosts. If disabled, all schemes are replaced by a default set.");

        props.add (new BooleanPropertyComponent (getSetting ("useHeaderMap"), "USE_HEADERMAP", "Enabled"),
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

        File projectBundle (getProjectBundle());
        createDirectoryOrThrow (projectBundle);

        createObjects();

        File projectFile (projectBundle.getChildFile ("project.pbxproj"));

        {
            MemoryOutputStream mo;
            writeProjectFile (mo);
            overwriteFileIfDifferentOrThrow (projectFile, mo);
        }

        writeInfoPlistFiles();

        // Deleting the .rsrc files can be needed to force Xcode to update the version number.
        deleteRsrcFiles (getTargetFolder().getChildFile ("build"));
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        callForAllSupportedTargets ([this] (ProjectType::Target::Type targetType)
                                    {
                                        if (XCodeTarget* target = new XCodeTarget (targetType, *this))
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
                getPostBuildScriptValue() = var();
        }
    }

    bool hasInvalidPostBuildScript() const
    {
        // check whether the script is identical to the old one that the Introjucer used to auto-generate
        return (MD5 (getPostBuildScript().toUTF8()).toHexString() == "265ac212a7e734c5bbd6150e1eae18a1");
    }

protected:
    //==============================================================================
    class XcodeBuildConfiguration  : public BuildConfiguration
    {
    public:
        XcodeBuildConfiguration (Project& p, const ValueTree& t, const bool isIOS, const ProjectExporter& e)
            : BuildConfiguration (p, t, e),
              iOS (isIOS),
              osxSDKVersion                (config, Ids::osxSDK,                       nullptr, "default"),
              osxDeploymentTarget          (config, Ids::osxCompatibility,             nullptr, "default"),
              iosDeploymentTarget          (config, Ids::iosCompatibility,             nullptr, "default"),
              osxArchitecture              (config, Ids::osxArchitecture,              nullptr, "default"),
              customXcodeFlags             (config, Ids::customXcodeFlags,             nullptr),
              plistPreprocessorDefinitions (config, Ids::plistPreprocessorDefinitions, nullptr),
              cppStandardLibrary           (config, Ids::cppLibType,                   nullptr),
              codeSignIdentity             (config, Ids::codeSigningIdentity,          nullptr, iOS ? "iPhone Developer" : "Mac Developer"),
              fastMathEnabled              (config, Ids::fastMath,                     nullptr),
              linkTimeOptimisationEnabled  (config, Ids::linkTimeOptimisation,         nullptr),
              stripLocalSymbolsEnabled     (config, Ids::stripLocalSymbols,            nullptr),
              vstBinaryLocation            (config, Ids::xcodeVstBinaryLocation,       nullptr, "$(HOME)/Library/Audio/Plug-Ins/VST/"),
              vst3BinaryLocation           (config, Ids::xcodeVst3BinaryLocation,      nullptr, "$(HOME)/Library/Audio/Plug-Ins/VST3/"),
              auBinaryLocation             (config, Ids::xcodeAudioUnitBinaryLocation, nullptr, "$(HOME)/Library/Audio/Plug-Ins/Components/"),
              rtasBinaryLocation           (config, Ids::xcodeRtasBinaryLocation,      nullptr, "/Library/Application Support/Digidesign/Plug-Ins/"),
              aaxBinaryLocation            (config, Ids::xcodeAaxBinaryLocation,       nullptr, "/Library/Application Support/Avid/Audio/Plug-Ins/")
        {
        }

        //==========================================================================
        bool iOS;

        CachedValue<String> osxSDKVersion, osxDeploymentTarget, iosDeploymentTarget, osxArchitecture,
                            customXcodeFlags, plistPreprocessorDefinitions, cppStandardLibrary, codeSignIdentity;
        CachedValue<bool>   fastMathEnabled, linkTimeOptimisationEnabled, stripLocalSymbolsEnabled;
        CachedValue<String> vstBinaryLocation, vst3BinaryLocation, auBinaryLocation, rtasBinaryLocation, aaxBinaryLocation;

        //==========================================================================
        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addXcodePluginInstallPathProperties (props);
            addGCCOptimisationProperty (props);

            if (iOS)
            {
                const char* iosVersions[]      = { "Use Default",     "7.0", "7.1", "8.0", "8.1", "8.2", "8.3", "8.4", "9.0", "9.1", "9.2", "9.3", "10.0", 0 };
                const char* iosVersionValues[] = { osxVersionDefault, "7.0", "7.1", "8.0", "8.1", "8.2", "8.3", "8.4", "9.0", "9.1", "9.2", "9.3", "10.0", 0 };

                props.add (new ChoicePropertyComponent (iosDeploymentTarget.getPropertyAsValue(), "iOS Deployment Target",
                                                        StringArray (iosVersions), Array<var> (iosVersionValues)),
                           "The minimum version of iOS that the target binary will run on.");
            }
            else
            {
                StringArray sdkVersionNames, osxVersionNames;
                Array<var> versionValues;

                sdkVersionNames.add ("Use Default");
                osxVersionNames.add ("Use Default");
                versionValues.add (osxVersionDefault);

                for (int ver = oldestSDKVersion; ver <= currentSDKVersion; ++ver)
                {
                    sdkVersionNames.add (getSDKName (ver));
                    osxVersionNames.add (getOSXVersionName (ver));
                    versionValues.add (getSDKName (ver));
                }

                props.add (new ChoicePropertyComponent (osxSDKVersion.getPropertyAsValue(), "OSX Base SDK Version", sdkVersionNames, versionValues),
                           "The version of OSX to link against in the XCode build.");

                props.add (new ChoicePropertyComponent (osxDeploymentTarget.getPropertyAsValue(), "OSX Deployment Target", osxVersionNames, versionValues),
                           "The minimum version of OSX that the target binary will be compatible with.");

                const char* osxArch[] = { "Use Default", "Native architecture of build machine",
                                          "Universal Binary (32-bit)", "Universal Binary (32/64-bit)", "64-bit Intel", 0 };
                const char* osxArchValues[] = { osxArch_Default, osxArch_Native, osxArch_32BitUniversal,
                                                osxArch_64BitUniversal, osxArch_64Bit, 0 };

                props.add (new ChoicePropertyComponent (osxArchitecture.getPropertyAsValue(), "OSX Architecture",
                                                        StringArray (osxArch), Array<var> (osxArchValues)),
                           "The type of OSX binary that will be produced.");
            }

            props.add (new TextPropertyComponent (customXcodeFlags.getPropertyAsValue(), "Custom Xcode flags", 8192, false),
                       "A comma-separated list of custom Xcode setting flags which will be appended to the list of generated flags, "
                       "e.g. MACOSX_DEPLOYMENT_TARGET_i386 = 10.5, VALID_ARCHS = \"ppc i386 x86_64\"");

            props.add (new TextPropertyComponent (plistPreprocessorDefinitions.getPropertyAsValue(), "PList Preprocessor Definitions", 2048, true),
                       "Preprocessor definitions used during PList preprocessing (see PList Preprocess).");

            {
                static const char* cppLibNames[] = { "Use Default", "LLVM libc++", "GNU libstdc++", nullptr };
                static const var cppLibValues[] =  { var(),         "libc++",      "libstdc++" };

                props.add (new ChoicePropertyComponent (cppStandardLibrary.getPropertyAsValue(), "C++ Library",
                                                        StringArray (cppLibNames),
                                                        Array<var> (cppLibValues, numElementsInArray (cppLibValues))),
                           "The type of C++ std lib that will be linked.");
            }

            props.add (new TextWithDefaultPropertyComponent<String> (codeSignIdentity, "Code-signing Identity", 1024),
                       "The name of a code-signing identity for Xcode to apply.");

            props.add (new BooleanPropertyComponent (fastMathEnabled.getPropertyAsValue(), "Relax IEEE compliance", "Enabled"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new BooleanPropertyComponent (linkTimeOptimisationEnabled.getPropertyAsValue(), "Link-Time Optimisation", "Enabled"),
                       "Enable this to perform link-time code generation. This is recommended for release builds.");

            props.add (new BooleanPropertyComponent (stripLocalSymbolsEnabled.getPropertyAsValue(), "Strip local symbols", "Enabled"),
                       "Enable this to strip any locally defined symbols resulting in a smaller binary size. Enabling this "
                       "will also remove any function names from crash logs. Must be disabled for static library projects.");
        }

        String getModuleLibraryArchName() const override
        {
            return "${CURRENT_ARCH}";
        }

    private:
        //==========================================================================
        void addXcodePluginInstallPathProperties (PropertyListBuilder& props)
        {
            if (project.shouldBuildVST())
                props.add (new TextWithDefaultPropertyComponent<String> (vstBinaryLocation, "VST Binary location", 1024),
                           "The folder in which the compiled VST binary should be placed.");

            if (project.shouldBuildVST3())
                props.add (new TextWithDefaultPropertyComponent<String> (vst3BinaryLocation, "VST3 Binary location", 1024),
                           "The folder in which the compiled VST3 binary should be placed.");

            if (project.shouldBuildAU())
                props.add (new TextWithDefaultPropertyComponent<String> (auBinaryLocation, "AU Binary location", 1024),
                           "The folder in which the compiled AU binary should be placed.");

            if (project.shouldBuildRTAS())
                props.add (new TextWithDefaultPropertyComponent<String> (rtasBinaryLocation, "RTAS Binary location", 1024),
                           "The folder in which the compiled RTAS binary should be placed.");

            if (project.shouldBuildAAX())
                props.add (new TextWithDefaultPropertyComponent<String> (aaxBinaryLocation, "AAX Binary location", 1024),
                           "The folder in which the compiled AAX binary should be placed.");
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new XcodeBuildConfiguration (project, v, iOS, *this);
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
    struct XCodeTarget : ProjectType::Target
    {
        //==============================================================================
        XCodeTarget (ProjectType::Target::Type targetType, const XCodeProjectExporter& exporter)
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

                case SharedCodeTarget:
                    xcodeFileType = "archive.ar";
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

        String getXCodeSchemeName() const
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
        String xcodeOtherRezFlags, xcodeExcludedFiles64Bit, xcodeBundleIDSubPath;
        bool xcodeCopyToProductInstallPathAfterBuild;
        StringArray xcodeFrameworks, xcodeLibs;
        Array<XmlElement> xcodeExtraPListEntries;
        Array<RelativePath> xcodeExtraLibrariesDebug, xcodeExtraLibrariesRelease;

        StringArray frameworkIDs, buildPhaseIDs, configIDs, sourceIDs, rezFileIDs;
        String dependencyID, mainBuildProductID;
        File infoPlistFile;

        //==============================================================================
        void addMainBuildProduct() const
        {
            jassert (xcodeFileType.isNotEmpty());
            jassert (xcodeBundleExtension.isEmpty() || xcodeBundleExtension.startsWithChar ('.'));

            if (ProjectExporter::BuildConfiguration::Ptr config = owner.getConfiguration(0))
            {
                String productName (owner.replacePreprocessorTokens (*config, config->getTargetBinaryNameString()));

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
            ValueTree* v = new ValueTree (owner.createID (String ("__productFileID") + getName()));
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
            ValueTree* const v = new ValueTree (dependencyID);

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
            String configID = owner.createID (String ("targetconfigid_") + getName() + String ("_") + configName);

            ValueTree* v = new ValueTree (configID);
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

            auto developmentTeamID = owner.getIosDevelopmentTeamIDString();
            if (developmentTeamID.isNotEmpty())
                attributes << "DevelopmentTeam = " << developmentTeamID << "; ";

            auto appGroupsEnabled      = (owner.iOS && owner.isAppGroupsEnabled() ? 1 : 0);
            auto inAppPurchasesEnabled = (owner.iOS && owner.isInAppPurchasesEnabled()) ? 1 : 0;
            auto interAppAudioEnabled  = (owner.iOS
                                          && type == Target::StandalonePlugIn
                                          && owner.getProject().shouldEnableIAA()) ? 1 : 0;

            auto pushNotificationsEnabled = (owner.iOS && owner.isPushNotificationsEnabled()) ? 1 : 0;
            auto sandboxEnabled = (type == Target::AudioUnitv3PlugIn ? 1 : 0);

            attributes << "SystemCapabilities = {";
            attributes << "com.apple.ApplicationGroups.iOS = { enabled = " << appGroupsEnabled << "; }; ";
            attributes << "com.apple.InAppPurchase = { enabled = " << inAppPurchasesEnabled << "; }; ";
            attributes << "com.apple.InterAppAudio = { enabled = " << interAppAudioEnabled << "; }; ";
            attributes << "com.apple.Push = { enabled = " << pushNotificationsEnabled << "; }; ";
            attributes << "com.apple.Sandbox = { enabled = " << sandboxEnabled << "; }; ";
            attributes << "}; };";

            return attributes;
        }

        //==============================================================================
        ValueTree& addBuildPhase (const String& buildPhaseType, const StringArray& fileIds, const StringRef humanReadableName = StringRef())
        {
            String buildPhaseName = buildPhaseType + String ("_") + getName() + String ("_") + (humanReadableName.isNotEmpty() ? String (humanReadableName) : String ("resbuildphase"));
            String buildPhaseId (owner.createID (buildPhaseName));

            int n = 0;
            while (buildPhaseIDs.contains (buildPhaseId))
                buildPhaseId = owner.createID (buildPhaseName + String (++n));

            buildPhaseIDs.add (buildPhaseId);

            ValueTree* v = new ValueTree (buildPhaseId);
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
            const ProjectType::Target::TargetFileType fileType = getTargetFileType();
            return (fileType == executable && type != ConsoleApp) || fileType == pluginBundle || fileType == macOSAppex;
        }

        //==============================================================================
        StringArray getTargetSettings (const XcodeBuildConfiguration& config) const
        {
            if (type == AggregateTarget)
                // the aggregate target should not specify any settings at all!
                // it just defines dependencies on the other targets.
                return {};

            StringArray s;

            String bundleIdentifier = owner.project.getBundleIdentifier().toString();
            if (xcodeBundleIDSubPath.isNotEmpty())
            {
                StringArray bundleIdSegments = StringArray::fromTokens (bundleIdentifier, ".", StringRef());

                jassert (bundleIdSegments.size() > 0);
                bundleIdentifier += String (".") + bundleIdSegments[bundleIdSegments.size() - 1] + xcodeBundleIDSubPath;
            }

            s.add ("PRODUCT_BUNDLE_IDENTIFIER = " + bundleIdentifier);

            const String arch ((! owner.isiOS() && type == Target::AudioUnitv3PlugIn) ? osxArch_64Bit : config.osxArchitecture.get());
            if (arch == osxArch_Native)                s.add ("ARCHS = \"$(NATIVE_ARCH_ACTUAL)\"");
            else if (arch == osxArch_32BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_BIT)\"");
            else if (arch == osxArch_64BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\"");
            else if (arch == osxArch_64Bit)            s.add ("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\"");

            s.add ("HEADER_SEARCH_PATHS = " + getHeaderSearchPaths (config));
            s.add ("USE_HEADERMAP = " + String (static_cast<bool> (config.exporter.settings.getProperty ("useHeaderMap")) ? "YES" : "NO"));

            s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());

            if (shouldCreatePList())
            {
                s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

                if (owner.getPListPrefixHeaderString().isNotEmpty())
                    s.add ("INFOPLIST_PREFIX_HEADER = " + owner.getPListPrefixHeaderString());

                s.add ("INFOPLIST_PREPROCESS = " + (owner.isPListPreprocessEnabled() ? String ("YES") : String ("NO")));

                auto plistDefs = parsePreprocessorDefs (config.plistPreprocessorDefinitions.get());
                StringArray defsList;

                for (int i = 0; i < plistDefs.size(); ++i)
                {
                    String def (plistDefs.getAllKeys()[i]);
                    const String value (plistDefs.getAllValues()[i]);

                    if (value.isNotEmpty())
                        def << "=" << value.replace ("\"", "\\\\\\\"");

                    defsList.add ("\"" + def + "\"");
                }

                if (defsList.size() > 0)
                    s.add ("INFOPLIST_PREPROCESSOR_DEFINITIONS = " + indentParenthesisedList (defsList));
            }

            if (config.linkTimeOptimisationEnabled.get())
                s.add ("LLVM_LTO = YES");

            if (config.fastMathEnabled.get())
                s.add ("GCC_FAST_MATH = YES");

            const String extraFlags (owner.replacePreprocessorTokens (config, owner.getExtraCompilerFlagsString()).trim());
            if (extraFlags.isNotEmpty())
                s.add ("OTHER_CPLUSPLUSFLAGS = \"" + extraFlags + "\"");

            String installPath = getInstallPathForConfiguration (config);

            if (installPath.isNotEmpty())
            {
                s.add ("INSTALL_PATH = \"" + installPath + "\"");

                if (xcodeCopyToProductInstallPathAfterBuild)
                {
                    s.add ("DEPLOYMENT_LOCATION = YES");
                    s.add ("DSTROOT = /");
                }
            }

            if (getTargetFileType() == pluginBundle)
            {
                s.add ("LIBRARY_STYLE = Bundle");
                s.add ("WRAPPER_EXTENSION = " + xcodeBundleExtension.substring (1));
                s.add ("GENERATE_PKGINFO_FILE = YES");
            }

            if (xcodeOtherRezFlags.isNotEmpty())
                s.add ("OTHER_REZFLAGS = \"" + xcodeOtherRezFlags + "\"");

            String configurationBuildDir = "$(PROJECT_DIR)/build/$(CONFIGURATION)";

            if (config.getTargetBinaryRelativePathString().isNotEmpty())
            {
                // a target's position can either be defined via installPath + xcodeCopyToProductInstallPathAfterBuild
                // (= for audio plug-ins) or using a custom binary path (for everything else), but not both (= conflict!)
                jassert (! xcodeCopyToProductInstallPathAfterBuild);

                RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
                configurationBuildDir = sanitisePath (binaryPath.rebased (owner.projectFolder, owner.getTargetFolder(), RelativePath::buildTargetFolder)
                                                                .toUnixStyle());
            }

            s.add ("CONFIGURATION_BUILD_DIR = " + addQuotesIfRequired (configurationBuildDir));

            String gccVersion ("com.apple.compilers.llvm.clang.1_0");

            if (owner.iOS)
            {
                s.add ("ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon");
                s.add ("ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME = LaunchImage");
            }
            else
            {
                String sdkRoot;
                s.add ("MACOSX_DEPLOYMENT_TARGET = " + getOSXDeploymentTarget(config, &sdkRoot));

                if (sdkRoot.isNotEmpty())
                    s.add ("SDKROOT = " + sdkRoot);

                s.add ("MACOSX_DEPLOYMENT_TARGET_ppc = 10.4");
                s.add ("SDKROOT_ppc = macosx10.5");

                if (xcodeExcludedFiles64Bit.isNotEmpty())
                {
                    s.add ("EXCLUDED_SOURCE_FILE_NAMES = \"$(EXCLUDED_SOURCE_FILE_NAMES_$(CURRENT_ARCH))\"");
                    s.add ("EXCLUDED_SOURCE_FILE_NAMES_x86_64 = " + xcodeExcludedFiles64Bit);
                }
            }

            s.add ("GCC_VERSION = " + gccVersion);
            s.add ("CLANG_LINK_OBJC_RUNTIME = NO");

            if (! config.codeSignIdentity.isUsingDefault())
                s.add ("CODE_SIGN_IDENTITY = " + config.codeSignIdentity.get().quoted());

            if (owner.isPushNotificationsEnabled())
                s.add ("CODE_SIGN_ENTITLEMENTS = " + owner.getProject().getTitle() + ".entitlements");

            {
                auto cppStandard = owner.project.getCppStandardValue().toString();

                if (cppStandard == "latest")
                    cppStandard = "1z";

                s.add ("CLANG_CXX_LANGUAGE_STANDARD = " + (String (owner.shouldUseGNUExtensions() ? "gnu++"
                                                                                                  : "c++") + cppStandard).quoted());
            }

            if (config.cppStandardLibrary.get().isNotEmpty())
                s.add ("CLANG_CXX_LIBRARY = " + config.cppStandardLibrary.get().quoted());

            s.add ("COMBINE_HIDPI_IMAGES = YES");

            {
                StringArray linkerFlags, librarySearchPaths;
                getLinkerSettings (config, linkerFlags, librarySearchPaths);

                if (linkerFlags.size() > 0)
                    s.add ("OTHER_LDFLAGS = \"" + linkerFlags.joinIntoString (" ") + "\"");

                librarySearchPaths.addArray (config.getLibrarySearchPaths());
                librarySearchPaths = getCleanedStringArray (librarySearchPaths);

                if (librarySearchPaths.size() > 0)
                {
                    String libPaths ("LIBRARY_SEARCH_PATHS = (\"$(inherited)\"");

                    for (auto& p : librarySearchPaths)
                        libPaths += ", \"\\\"" + p + "\\\"\"";

                    s.add (libPaths + ")");
                }
            }

            StringPairArray defines;

            if (config.isDebug())
            {
                defines.set ("_DEBUG", "1");
                defines.set ("DEBUG", "1");
                s.add ("COPY_PHASE_STRIP = NO");
                s.add ("GCC_DYNAMIC_NO_PIC = NO");
            }
            else
            {
                defines.set ("_NDEBUG", "1");
                defines.set ("NDEBUG", "1");
                s.add ("GCC_GENERATE_DEBUGGING_SYMBOLS = NO");
                s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = YES");
                s.add ("DEAD_CODE_STRIPPING = YES");
            }

            if (type != Target::SharedCodeTarget && type != Target::StaticLibrary && type != Target::DynamicLibrary
                  && config.stripLocalSymbolsEnabled.get())
            {
                s.add ("STRIPFLAGS = \"-x\"");
                s.add ("DEPLOYMENT_POSTPROCESSING = YES");
                s.add ("SEPARATE_STRIP = YES");
            }

            if (owner.project.getProjectType().isAudioPlugin()
                && (   (owner.isOSX() && type == Target::AudioUnitv3PlugIn)
                    || (owner.isiOS() && type == Target::StandalonePlugIn && owner.getProject().shouldEnableIAA())))
                s.add (String ("CODE_SIGN_ENTITLEMENTS = \"") + owner.getEntitlementsFileName() + String ("\""));

            defines = mergePreprocessorDefs (defines, owner.getAllPreprocessorDefs (config, type));

            StringArray defsList;

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines.getAllKeys()[i]);
                const String value (defines.getAllValues()[i]);
                if (value.isNotEmpty())
                    def << "=" << value.replace ("\"", "\\\\\\\"");

                defsList.add ("\"" + def + "\"");
            }

            s.add ("GCC_PREPROCESSOR_DEFINITIONS = " + indentParenthesisedList (defsList));

            s.addTokens (config.customXcodeFlags.get(), ",", "\"'");

            return getCleanedStringArray (s);
        }

        String getInstallPathForConfiguration (const XcodeBuildConfiguration& config) const
        {
            switch (type)
            {
                case GUIApp:            return "$(HOME)/Applications";
                case ConsoleApp:        return "/usr/bin";
                case VSTPlugIn:         return config.vstBinaryLocation.get();
                case VST3PlugIn:        return config.vst3BinaryLocation.get();
                case AudioUnitPlugIn:   return config.auBinaryLocation.get();
                case RTASPlugIn:        return config.rtasBinaryLocation.get();
                case AAXPlugIn:         return config.aaxBinaryLocation.get();
                case SharedCodeTarget:  return owner.isiOS() ? "@executable_path/Frameworks" : "@executable_path/../Frameworks";
                default:                return {};
            }
        }

        //==============================================================================
        void getLinkerSettings (const BuildConfiguration& config, StringArray& flags, StringArray& librarySearchPaths) const
        {
            if (getTargetFileType() == pluginBundle)
                flags.add (owner.isiOS() ? "-bitcode_bundle" : "-bundle");

            Array<RelativePath> extraLibs (config.isDebug() ? xcodeExtraLibrariesDebug
                                                            : xcodeExtraLibrariesRelease);

            addExtraLibsForTargetType (config, extraLibs);

            for (auto& lib : extraLibs)
            {
                flags.add (getLinkerFlagForLib (lib.getFileNameWithoutExtension()));
                librarySearchPaths.add (owner.getSearchPathForStaticLibrary (lib));
            }

            if (owner.project.getProjectType().isAudioPlugin() && type != Target::SharedCodeTarget)
            {
                if (owner.getTargetOfType (Target::SharedCodeTarget) != nullptr)
                {
                    String productName (getStaticLibbedFilename (owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString())));

                    RelativePath sharedCodelib (productName, RelativePath::buildTargetFolder);
                    flags.add (getLinkerFlagForLib (sharedCodelib.getFileNameWithoutExtension()));
                }
            }

            flags.add (owner.replacePreprocessorTokens (config, owner.getExtraLinkerFlagsString()));
            flags.add (owner.getExternalLibraryFlags (config));

            StringArray libs (owner.xcodeLibs);
            libs.addArray (xcodeLibs);

            for (auto& l : libs)
                flags.add (getLinkerFlagForLib (l));

            flags = getCleanedStringArray (flags);
        }

        //========================================================================== c
        void writeInfoPlistFile() const
        {
            if (! shouldCreatePList())
                return;

            ScopedPointer<XmlElement> plist (XmlDocument::parse (owner.getPListToMergeString()));

            if (plist == nullptr || ! plist->hasTagName ("plist"))
                plist = new XmlElement ("plist");

            XmlElement* dict = plist->getChildByName ("dict");

            if (dict == nullptr)
                dict = plist->createNewChildElement ("dict");

            if (owner.iOS)
            {
                addPlistDictionaryKeyBool (dict, "LSRequiresIPhoneOS", true);
                if (owner.isMicrophonePermissionEnabled())
                    addPlistDictionaryKey (dict, "NSMicrophoneUsageDescription", "This app requires microphone input.");

                if (type != AudioUnitv3PlugIn)
                    addPlistDictionaryKeyBool (dict, "UIViewControllerBasedStatusBarAppearance", false);
            }

            addPlistDictionaryKey (dict, "CFBundleExecutable",          "${EXECUTABLE_NAME}");

            if (! owner.iOS) // (NB: on iOS this causes error ITMS-90032 during publishing)
                addPlistDictionaryKey (dict, "CFBundleIconFile", owner.iconFile.exists() ? owner.iconFile.getFileName() : String());

            addPlistDictionaryKey (dict, "CFBundleIdentifier",          "$(PRODUCT_BUNDLE_IDENTIFIER)");
            addPlistDictionaryKey (dict, "CFBundleName",                owner.projectName);

            // needed by NSExtension on iOS
            addPlistDictionaryKey (dict, "CFBundleDisplayName",         owner.projectName);
            addPlistDictionaryKey (dict, "CFBundlePackageType",         xcodePackageType);
            addPlistDictionaryKey (dict, "CFBundleSignature",           xcodeBundleSignature);
            addPlistDictionaryKey (dict, "CFBundleShortVersionString",  owner.project.getVersionString());
            addPlistDictionaryKey (dict, "CFBundleVersion",             owner.project.getVersionString());
            addPlistDictionaryKey (dict, "NSHumanReadableCopyright",    owner.project.getCompanyName().toString());
            addPlistDictionaryKeyBool (dict, "NSHighResolutionCapable", true);

            StringArray documentExtensions;
            documentExtensions.addTokens (replacePreprocessorDefs (owner.getAllPreprocessorDefs(), owner.settings ["documentExtensions"]),
                                          ",", StringRef());
            documentExtensions.trim();
            documentExtensions.removeEmptyStrings (true);

            if (documentExtensions.size() > 0 && type != AudioUnitv3PlugIn)
            {
                dict->createNewChildElement ("key")->addTextElement ("CFBundleDocumentTypes");
                XmlElement* dict2 = dict->createNewChildElement ("array")->createNewChildElement ("dict");
                XmlElement* arrayTag = nullptr;

                for (String ex : documentExtensions)
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

            if (owner.settings ["UIFileSharingEnabled"] && type != AudioUnitv3PlugIn)
                addPlistDictionaryKeyBool (dict, "UIFileSharingEnabled", true);

            if (owner.settings ["UIStatusBarHidden"] && type != AudioUnitv3PlugIn)
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
                    XmlElement* audioComponentsDict = audioComponentsPlistEntry.createNewChildElement ("dict");

                    addPlistDictionaryKey    (audioComponentsDict, "name",         owner.project.getIAAPluginName());
                    addPlistDictionaryKey    (audioComponentsDict, "manufacturer", owner.project.getPluginManufacturerCode().toString().trim().substring (0, 4));
                    addPlistDictionaryKey    (audioComponentsDict, "type",         owner.project.getIAATypeCode());
                    addPlistDictionaryKey    (audioComponentsDict, "subtype",      owner.project.getPluginCode().toString().trim().substring (0, 4));
                    addPlistDictionaryKeyInt (audioComponentsDict, "version",      owner.project.getVersionAsHexInteger());

                    dict->addChildElement (new XmlElement (audioComponentsPlistEntry));
                }
            }

            for (auto& e : xcodeExtraPListEntries)
                dict->addChildElement (new XmlElement (e));

            MemoryOutputStream mo;
            plist->writeToStream (mo, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");

            overwriteFileIfDifferentOrThrow (infoPlistFile, mo);
        }

        //==============================================================================
        void addIosScreenOrientations (XmlElement* dict) const
        {
            String screenOrientation = owner.getScreenOrientationString();
            StringArray iOSOrientations;

            if (screenOrientation.contains ("portrait"))   { iOSOrientations.add ("UIInterfaceOrientationPortrait"); }
            if (screenOrientation.contains ("landscape"))  { iOSOrientations.add ("UIInterfaceOrientationLandscapeLeft");  iOSOrientations.add ("UIInterfaceOrientationLandscapeRight"); }

            addArrayToPlist (dict, "UISupportedInterfaceOrientations", iOSOrientations);

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
            XmlElement* plistStringArray = dict->createNewChildElement ("array");

            for (auto& e : arrayElements)
                plistStringArray->createNewChildElement ("string")->addTextElement (e);
        }

        //==============================================================================
        void addShellScriptBuildPhase (const String& phaseName, const String& script)
        {
            if (script.trim().isNotEmpty())
            {
                ValueTree& v = addBuildPhase ("PBXShellScriptBuildPhase", StringArray());
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
            ValueTree& v = addBuildPhase ("PBXCopyFilesBuildPhase", files, phaseName);
            v.setProperty ("dstPath", "", nullptr);
            v.setProperty ("dstSubfolderSpec", (int) dst, nullptr);
        }

        //==============================================================================
        String getHeaderSearchPaths (const BuildConfiguration& config) const
        {
            StringArray paths (owner.extraSearchPaths);
            paths.addArray (config.getHeaderSearchPaths());
            paths.addArray (getTargetExtraHeaderSearchPaths());

            if (owner.project.getModules().isModuleEnabled ("juce_audio_plugin_client"))
            {
                // Needed to compile .r files
                paths.add (owner.getModuleFolderRelativeToProject ("juce_audio_plugin_client")
                                .rebased (owner.projectFolder, owner.getTargetFolder(), RelativePath::buildTargetFolder)
                                .toUnixStyle());
            }

            paths.add ("$(inherited)");

            paths = getCleanedStringArray (paths);

            for (auto& s : paths)
            {
                s = owner.replacePreprocessorTokens (config, s);

                if (s.containsChar (' '))
                    s = "\"\\\"" + s + "\\\"\""; // crazy double quotes required when there are spaces..
                else
                    s = "\"" + s + "\"";
            }

            return "(" + paths.joinIntoString (", ") + ")";
        }

    private:
        //==============================================================================
        void addExtraAudioUnitTargetSettings()
        {
            xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                                 " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                 " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"";

            xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

            XmlElement plistKey ("key");
            plistKey.addTextElement ("AudioComponents");

            XmlElement plistEntry ("array");
            XmlElement* dict = plistEntry.createNewChildElement ("dict");

            const String pluginManufacturerCode = owner.project.getPluginManufacturerCode().toString().trim().substring (0, 4);
            const String pluginSubType          = owner.project.getPluginCode()            .toString().trim().substring (0, 4);

            if (pluginManufacturerCode.toLowerCase() == pluginManufacturerCode)
            {
                throw SaveError ("AudioUnit plugin code identifiers invalid!\n\n"
                                 "You have used only lower case letters in your AU plugin manufacturer identifier. "
                                 "You must have at least one uppercase letter in your AU plugin manufacturer "
                                 "identifier code.");
            }

            addPlistDictionaryKey (dict, "name", owner.project.getPluginManufacturer().toString()
                                                   + ": " + owner.project.getPluginName().toString());
            addPlistDictionaryKey (dict, "description", owner.project.getPluginDesc().toString());
            addPlistDictionaryKey (dict, "factoryFunction", owner.project.getPluginAUExportPrefix().toString() + "Factory");
            addPlistDictionaryKey (dict, "manufacturer", pluginManufacturerCode);
            addPlistDictionaryKey (dict, "type", owner.project.getAUMainTypeCode());
            addPlistDictionaryKey (dict, "subtype", pluginSubType);
            addPlistDictionaryKeyInt (dict, "version", owner.project.getVersionAsHexInteger());

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

            addPlistDictionaryKey (&plistEntry, "NSExtensionPrincipalClass", owner.project.getPluginAUExportPrefix().toString() + "FactoryAUv3");
            addPlistDictionaryKey (&plistEntry, "NSExtensionPointIdentifier", "com.apple.AudioUnit-UI");
            plistEntry.createNewChildElement ("key")->addTextElement ("NSExtensionAttributes");

            XmlElement* dict = plistEntry.createNewChildElement ("dict");
            dict->createNewChildElement ("key")->addTextElement ("AudioComponents");
            XmlElement* componentArray = dict->createNewChildElement ("array");

            XmlElement* componentDict = componentArray->createNewChildElement ("dict");

            addPlistDictionaryKey (componentDict, "name", owner.project.getPluginManufacturer().toString()
                                                            + ": " + owner.project.getPluginName().toString());
            addPlistDictionaryKey (componentDict, "description", owner.project.getPluginDesc().toString());
            addPlistDictionaryKey (componentDict, "factoryFunction",owner.project. getPluginAUExportPrefix().toString() + "FactoryAUv3");
            addPlistDictionaryKey (componentDict, "manufacturer", owner.project.getPluginManufacturerCode().toString().trim().substring (0, 4));
            addPlistDictionaryKey (componentDict, "type", owner.project.getAUMainTypeCode());
            addPlistDictionaryKey (componentDict, "subtype", owner.project.getPluginCode().toString().trim().substring (0, 4));
            addPlistDictionaryKeyInt (componentDict, "version", owner.project.getVersionAsHexInteger());
            addPlistDictionaryKeyBool (componentDict, "sandboxSafe", true);

            componentDict->createNewChildElement ("key")->addTextElement ("tags");
            XmlElement* tagsArray = componentDict->createNewChildElement ("array");

            tagsArray->createNewChildElement ("string")
                ->addTextElement (static_cast<bool> (owner.project.getPluginIsSynth().getValue()) ? "Synth" : "Effects");

            xcodeExtraPListEntries.add (plistKey);
            xcodeExtraPListEntries.add (plistEntry);
        }

        void addExtraLibsForTargetType  (const BuildConfiguration& config, Array<RelativePath>& extraLibs) const
        {
            if (type == AAXPlugIn)
            {
                 auto aaxLibsFolder
                    = RelativePath (owner.getAAXPathValue().toString(), RelativePath::projectFolder)
                        .getChildFile ("Libs");

                String libraryPath (config.isDebug() ? "Debug/libAAXLibrary" : "Release/libAAXLibrary");
                libraryPath += (isUsingClangCppLibrary (config) ? "_libcpp.a" : ".a");

                extraLibs.add   (aaxLibsFolder.getChildFile (libraryPath));
            }
            else if (type == RTASPlugIn)
            {
                RelativePath rtasFolder (owner.getRTASPathValue().toString(), RelativePath::projectFolder);

                extraLibs.add (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
                extraLibs.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));
            }
        }

        StringArray getTargetExtraHeaderSearchPaths() const
        {
            StringArray targetExtraSearchPaths;

            if (type == RTASPlugIn)
            {
                RelativePath rtasFolder (owner.getRTASPathValue().toString(), RelativePath::projectFolder);

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

        bool isUsingClangCppLibrary (const BuildConfiguration& config) const
        {
            if (auto xcodeConfig = dynamic_cast<const XcodeBuildConfiguration*> (&config))
            {
                const auto& configValue = xcodeConfig->cppStandardLibrary.get();

                if (configValue.isNotEmpty())
                    return (configValue == "libc++");

                auto minorOSXDeploymentTarget = getOSXDeploymentTarget (*xcodeConfig)
                                               .fromLastOccurrenceOf (".", false, false)
                                               .getIntValue();

                return (minorOSXDeploymentTarget > 8);
            }

            return false;
        }

        String getOSXDeploymentTarget (const XcodeBuildConfiguration& config, String* sdkRoot = nullptr) const
        {
            const String sdk (config.osxSDKVersion.get());
            const String sdkCompat (config.osxDeploymentTarget.get());

            // The AUv3 target always needs to be at least 10.11
            int oldestAllowedDeploymentTarget = (type == Target::AudioUnitv3PlugIn ? minimumAUv3SDKVersion
                                                 : oldestSDKVersion);

            // if the user doesn't set it, then use the last known version that works well with JUCE
            String deploymentTarget = "10.11";

            for (int ver = oldestAllowedDeploymentTarget; ver <= currentSDKVersion; ++ver)
            {
                if (sdk == getSDKName (ver) && sdkRoot != nullptr) *sdkRoot = String ("macosx10." + String (ver));
                if (sdkCompat == getSDKName (ver))   deploymentTarget = "10." + String (ver);
            }

            return deploymentTarget;
        }

        //==============================================================================
        const XCodeProjectExporter& owner;

        Target& operator= (const Target&) JUCE_DELETED_FUNCTION;
    };

    mutable StringArray xcodeFrameworks;
    StringArray xcodeLibs;

private:
    //==============================================================================
    bool xcodeCanUseDwarf;
    OwnedArray<XCodeTarget> targets;

    mutable OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, pbxGroups, misc, projectConfigs, targetConfigs;
    mutable StringArray resourceIDs, sourceIDs, targetIDs;
    mutable StringArray frameworkFileIDs, rezFileIDs, resourceFileRefs;
    mutable File menuNibFile, iconFile;
    mutable StringArray buildProducts;

    const bool iOS;

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

    File getProjectBundle() const                 { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (".xcodeproj"); }

    //==============================================================================
    void createObjects() const
    {
        prepareTargets();

        addFrameworks();
        addCustomResourceFolders();
        addPlistFileReferences();

        if (iOS && ! projectType.isStaticLibrary())
            addXcassets();
        else
            addNibFiles();

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
            if (target->type == XCodeTarget::AggregateTarget)
                continue;

            target->addMainBuildProduct();

            String targetName = target->getName();
            String fileID (createID (targetName + String ("__targetbuildref")));
            String fileRefID (createID (String ("__productFileID") + targetName));

            ValueTree* v = new ValueTree (fileID);
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
            if (target->type == XCodeTarget::AggregateTarget)
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
        // add build configurations
        for (ConstConfigIterator config (*this); config.next();)
        {
            const XcodeBuildConfiguration& xcodeConfig = dynamic_cast<const XcodeBuildConfiguration&> (*config);
            addProjectConfig (config->getName(), getProjectSettings (xcodeConfig));
        }
    }

    void addFilesAndGroupsToProject (StringArray& topLevelGroupIDs) const
    {
        StringPairArray entitlements = getEntitlements();
        if (entitlements.size() > 0)
            topLevelGroupIDs.add (addEntitlementsFile (entitlements));

        for (auto& group : getAllGroups())
            if (group.getNumChildren() > 0)
                topLevelGroupIDs.add (addProjectItem (group));
    }

    void addExtraGroupsToProject (StringArray& topLevelGroupIDs) const
    {
        { // Add 'resources' group
            String resourcesGroupID (createID ("__resources"));
            addGroup (resourcesGroupID, "Resources", resourceFileRefs);
            topLevelGroupIDs.add (resourcesGroupID);
        }

        { // Add 'frameworks' group
            String frameworksGroupID (createID ("__frameworks"));
            addGroup (frameworksGroupID, "Frameworks", frameworkFileIDs);
            topLevelGroupIDs.add (frameworksGroupID);
        }

        { // Add 'products' group
            String productsGroupID (createID ("__products"));
            addGroup (productsGroupID, "Products", buildProducts);
            topLevelGroupIDs.add (productsGroupID);
        }
    }

    void addBuildPhases() const
    {
        // add build phases
        for (auto* target : targets)
        {
            if (target->type != XCodeTarget::AggregateTarget)
                buildProducts.add (createID (String ("__productFileID") + String (target->getName())));

            for (ConstConfigIterator config (*this); config.next();)
            {
                const XcodeBuildConfiguration& xcodeConfig = dynamic_cast<const XcodeBuildConfiguration&> (*config);
                target->addTargetConfig (config->getName(), target->getTargetSettings (xcodeConfig));
            }

            addConfigList (*target, targetConfigs, createID (String ("__configList") + target->getName()));

            target->addShellScriptBuildPhase ("Pre-build script", getPreBuildScript());

            if (target->type != XCodeTarget::AggregateTarget)
            {
                auto skipAUv3 = (target->type == XCodeTarget::AudioUnitv3PlugIn
                                 && ! shouldDuplicateResourcesFolderForAppExtension());

                if (! projectType.isStaticLibrary() && target->type != XCodeTarget::SharedCodeTarget && ! skipAUv3)
                    target->addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

                StringArray rezFiles (rezFileIDs);
                rezFiles.addArray (target->rezFileIDs);

                if (rezFiles.size() > 0)
                    target->addBuildPhase ("PBXRezBuildPhase", rezFiles);

                StringArray sourceFiles (target->sourceIDs);

                if (target->type == XCodeTarget::SharedCodeTarget
                     || (! project.getProjectType().isAudioPlugin()))
                    sourceFiles.addArray (sourceIDs);

                target->addBuildPhase ("PBXSourcesBuildPhase", sourceFiles);

                if (! projectType.isStaticLibrary() && target->type != XCodeTarget::SharedCodeTarget)
                    target->addBuildPhase ("PBXFrameworksBuildPhase", target->frameworkIDs);
            }

            target->addShellScriptBuildPhase ("Post-build script", getPostBuildScript());

            if (project.getProjectType().isAudioPlugin() && project.shouldBuildAUv3()
                && project.shouldBuildStandalonePlugin() && target->type == XCodeTarget::StandalonePlugIn)
                embedAppExtension();

            addTargetObject (*target);
        }
    }

    void embedAppExtension() const
    {
        if (auto* standaloneTarget = getTargetOfType (XCodeTarget::StandalonePlugIn))
        {
            if (auto* auv3Target   = getTargetOfType (XCodeTarget::AudioUnitv3PlugIn))
            {
                StringArray files;
                files.add (auv3Target->mainBuildProductID);
                standaloneTarget->addCopyFilesPhase ("Embed App Extensions", files, kPluginsFolder);
            }
        }
    }

    static Image fixMacIconImageSize (Drawable& image)
    {
        const int validSizes[] = { 16, 32, 48, 128, 256, 512, 1024 };

        const int w = image.getWidth();
        const int h = image.getHeight();

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
    XCodeTarget* getTargetOfType (ProjectType::Target::Type type) const
    {
        for (auto& target : targets)
            if (target->type == type)
                return target;

        return nullptr;
    }

    void addTargetObject (XCodeTarget& target) const
    {
        String targetName = target.getName();

        String targetID = target.getID();
        ValueTree* const v = new ValueTree (targetID);
        v->setProperty ("isa", target.type == XCodeTarget::AggregateTarget ? "PBXAggregateTarget" : "PBXNativeTarget", nullptr);
        v->setProperty ("buildConfigurationList", createID (String ("__configList") + targetName), nullptr);

        v->setProperty ("buildPhases", indentParenthesisedList (target.buildPhaseIDs), nullptr);
        v->setProperty ("buildRules", "( )", nullptr);

        v->setProperty ("dependencies", indentParenthesisedList (getTargetDependencies (target)), nullptr);
        v->setProperty (Ids::name, target.getXCodeSchemeName(), nullptr);
        v->setProperty ("productName", projectName, nullptr);

        if (target.type != XCodeTarget::AggregateTarget)
        {
            v->setProperty ("productReference", createID (String ("__productFileID") + targetName), nullptr);

            jassert (target.xcodeProductType.isNotEmpty());
            v->setProperty ("productType", target.xcodeProductType, nullptr);
        }

        targetIDs.add (targetID);
        misc.add (v);
    }

    StringArray getTargetDependencies (const XCodeTarget& target) const
    {
        StringArray dependencies;

        if (project.getProjectType().isAudioPlugin())
        {
            if (target.type == XCodeTarget::StandalonePlugIn) // depends on AUv3 and shared code
            {
                if (XCodeTarget* auv3Target = getTargetOfType (XCodeTarget::AudioUnitv3PlugIn))
                    dependencies.add (auv3Target->getDependencyID());

                if (XCodeTarget* sharedCodeTarget = getTargetOfType (XCodeTarget::SharedCodeTarget))
                    dependencies.add (sharedCodeTarget->getDependencyID());
            }
            else if (target.type == XCodeTarget::AggregateTarget) // depends on all other targets
            {
                for (int i = 1; i < targets.size(); ++i)
                    dependencies.add (targets[i]->getDependencyID());
            }
            else if (target.type != XCodeTarget::SharedCodeTarget) // shared code doesn't depend on anything; all other targets depend only on the shared code
            {
                if (XCodeTarget* sharedCodeTarget = getTargetOfType (XCodeTarget::SharedCodeTarget))
                    dependencies.add (sharedCodeTarget->getDependencyID());
            }
        }

        return dependencies;
    }

    static void writeOldIconFormat (MemoryOutputStream& out, const Image& image, const char* type, const char* maskType)
    {
        const int w = image.getWidth();
        const int h = image.getHeight();

        out.write (type, 4);
        out.writeIntBigEndian (8 + 4 * w * h);

        const Image::BitmapData bitmap (image, Image::BitmapData::readOnly);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));
                out.writeByte ((char) pixel.getAlpha());
                out.writeByte ((char) pixel.getRed());
                out.writeByte ((char) pixel.getGreen());
                out.writeByte ((char) pixel.getBlue());
            }
        }

        out.write (maskType, 4);
        out.writeIntBigEndian (8 + w * h);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));
                out.writeByte ((char) pixel.getAlpha());
            }
        }
    }

    static void writeNewIconFormat (MemoryOutputStream& out, const Image& image, const char* type)
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
        int smallest = 0x7fffffff;
        Drawable* smallestImage = nullptr;

        for (int i = 0; i < images.size(); ++i)
        {
            const Image image (fixMacIconImageSize (*images.getUnchecked(i)));
            jassert (image.getWidth() == image.getHeight());

            if (image.getWidth() < smallest)
            {
                smallest = image.getWidth();
                smallestImage = images.getUnchecked(i);
            }

            switch (image.getWidth())
            {
                case 16:   writeOldIconFormat (data, image, "is32", "s8mk"); break;
                case 32:   writeOldIconFormat (data, image, "il32", "l8mk"); break;
                case 48:   writeOldIconFormat (data, image, "ih32", "h8mk"); break;
                case 128:  writeOldIconFormat (data, image, "it32", "t8mk"); break;
                case 256:  writeNewIconFormat (data, image, "ic08"); break;
                case 512:  writeNewIconFormat (data, image, "ic09"); break;
                case 1024: writeNewIconFormat (data, image, "ic10"); break;
                default:   break;
            }
        }

        jassert (data.getDataSize() > 0); // no suitable sized images?

        // If you only supply a 1024 image, the file doesn't work on 10.8, so we need
        // to force a smaller one in there too..
        if (smallest > 512 && smallestImage != nullptr)
            writeNewIconFormat (data, rescaleImageForIcon (*smallestImage, 512), "ic09");

        out.write ("icns", 4);
        out.writeIntBigEndian ((int) data.getDataSize() + 8);
        out << data;
    }

    void getIconImages (OwnedArray<Drawable>& images) const
    {
        ScopedPointer<Drawable> bigIcon (getBigIcon());
        if (bigIcon != nullptr)
            images.add (bigIcon.release());

        ScopedPointer<Drawable> smallIcon (getSmallIcon());
        if (smallIcon != nullptr)
            images.add (smallIcon.release());
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
            const File& entry = di.getFile();

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

        return "-l" + library.replace (" ", "\\\\ ").upToLastOccurrenceOf (".", false, false);
    }

    String getSearchPathForStaticLibrary (const RelativePath& library) const
    {
        String searchPath (library.toUnixStyle().upToLastOccurrenceOf ("/", false, false));

        if (! library.isAbsolute())
        {
            String srcRoot (rebaseFromProjectFolderToBuildTarget (RelativePath (".", RelativePath::projectFolder)).toUnixStyle());

            if (srcRoot.endsWith ("/."))      srcRoot = srcRoot.dropLastCharacters (2);
            if (! srcRoot.endsWithChar ('/')) srcRoot << '/';

            searchPath = srcRoot + searchPath;
        }

        return sanitisePath (searchPath);
    }

    StringArray getProjectSettings (const XcodeBuildConfiguration& config) const
    {
        StringArray s;
        s.add ("ALWAYS_SEARCH_USER_PATHS = NO");
        s.add ("ENABLE_STRICT_OBJC_MSGSEND = YES");
        s.add ("GCC_C_LANGUAGE_STANDARD = c11");
        s.add ("GCC_NO_COMMON_BLOCKS = YES");
        s.add ("GCC_MODEL_TUNING = G5");
        s.add ("GCC_WARN_ABOUT_RETURN_TYPE = YES");
        s.add ("GCC_WARN_CHECK_SWITCH_STATEMENTS = YES");
        s.add ("GCC_WARN_UNUSED_VARIABLE = YES");
        s.add ("GCC_WARN_MISSING_PARENTHESES = YES");
        s.add ("GCC_WARN_NON_VIRTUAL_DESTRUCTOR = YES");
        s.add ("GCC_WARN_TYPECHECK_CALLS_TO_PRINTF = YES");
        s.add ("GCC_WARN_64_TO_32_BIT_CONVERSION = YES");
        s.add ("GCC_WARN_UNDECLARED_SELECTOR = YES");
        s.add ("GCC_WARN_UNINITIALIZED_AUTOS = YES");
        s.add ("GCC_WARN_UNUSED_FUNCTION = YES");
        s.add ("CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = YES");
        s.add ("CLANG_WARN_BOOL_CONVERSION = YES");
        s.add ("CLANG_WARN_COMMA = YES");
        s.add ("CLANG_WARN_CONSTANT_CONVERSION = YES");
        s.add ("CLANG_WARN_EMPTY_BODY = YES");
        s.add ("CLANG_WARN_ENUM_CONVERSION = YES");
        s.add ("CLANG_WARN_INFINITE_RECURSION = YES");
        s.add ("CLANG_WARN_INT_CONVERSION = YES");
        s.add ("CLANG_WARN_NON_LITERAL_NULL_CONVERSION = YES");
        s.add ("CLANG_WARN_OBJC_LITERAL_CONVERSION = YES");
        s.add ("CLANG_WARN_RANGE_LOOP_ANALYSIS = YES");
        s.add ("CLANG_WARN_STRICT_PROTOTYPES = YES");
        s.add ("CLANG_WARN_SUSPICIOUS_MOVE = YES");
        s.add ("CLANG_WARN_UNREACHABLE_CODE = YES");
        s.add ("CLANG_WARN__DUPLICATE_METHOD_MATCH = YES");
        s.add ("WARNING_CFLAGS = -Wreorder");

        if (projectType.isStaticLibrary())
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = NO");
        }
        else
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = YES");
        }

        if (config.isDebug())
        {
            s.add ("ENABLE_TESTABILITY = YES");

            if (config.osxArchitecture.get() == osxArch_Default || config.osxArchitecture.get().isEmpty())
                s.add ("ONLY_ACTIVE_ARCH = YES");
        }

        if (iOS)
        {
            s.add ("\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = " + config.codeSignIdentity.get().quoted());
            s.add ("SDKROOT = iphoneos");
            s.add ("TARGETED_DEVICE_FAMILY = \"1,2\"");

            const String iosVersion (config.iosDeploymentTarget.get());
            if (iosVersion.isNotEmpty() && iosVersion != osxVersionDefault)
                s.add ("IPHONEOS_DEPLOYMENT_TARGET = " + iosVersion);
            else
                s.add ("IPHONEOS_DEPLOYMENT_TARGET = 9.3");
        }
        else
        {
            if (! config.codeSignIdentity.isUsingDefault() || getIosDevelopmentTeamIDString().isNotEmpty())
                s.add ("\"CODE_SIGN_IDENTITY\" = " + config.codeSignIdentity.get().quoted());
        }

        s.add ("ZERO_LINK = NO");

        if (xcodeCanUseDwarf)
            s.add ("DEBUG_INFORMATION_FORMAT = \"dwarf\"");

        s.add ("PRODUCT_NAME = \"" + replacePreprocessorTokens (config, config.getTargetBinaryNameString()) + "\"");
        return s;
    }

    void addFrameworks() const
    {
        if (! projectType.isStaticLibrary())
        {
            if (iOS && isInAppPurchasesEnabled())
                xcodeFrameworks.addIfNotAlreadyThere ("StoreKit");

            xcodeFrameworks.addTokens (getExtraFrameworksString(), ",;", "\"'");
            xcodeFrameworks.trim();

            StringArray s (xcodeFrameworks);

            for (auto& target : targets)
                s.addArray (target->xcodeFrameworks);

            if (project.getConfigFlag ("JUCE_QUICKTIME") == Project::configFlagDisabled)
                s.removeString ("QuickTime");

            s.trim();
            s.removeDuplicates (true);
            s.sort (true);

            for (auto& framework : s)
            {
                String frameworkID = addFramework (framework);

                // find all the targets that are referring to this object
                for (auto& target : targets)
                    if (xcodeFrameworks.contains (framework) || target->xcodeFrameworks.contains (framework))
                        target->frameworkIDs.add (frameworkID);
            }
        }
    }

    void addCustomResourceFolders() const
    {
        StringArray folders;

        folders.addTokens (getCustomResourceFoldersString(), ":", "");
        folders.trim();

        for (auto& crf : folders)
            addCustomResourceFolder (crf);
    }

    void addXcassets() const
    {
        String customXcassetsPath = getCustomXcassetsFolderString();

        if (customXcassetsPath.isEmpty())
            createXcassetsFolderFromIcons();
        else
            addCustomResourceFolder (customXcassetsPath, "folder.assetcatalog");
    }

    void addCustomResourceFolder (String folderPathRelativeToProjectFolder, const String fileType = "folder") const
    {
        String folderPath = RelativePath (folderPathRelativeToProjectFolder, RelativePath::projectFolder)
                                        .rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder)
                                        .toUnixStyle();

        const String fileRefID (createFileRefID (folderPath));

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
                  "\tobjects = {\n\n";

        Array<ValueTree*> objects;
        objects.addArray (pbxBuildFiles);
        objects.addArray (pbxFileReferences);
        objects.addArray (pbxGroups);
        objects.addArray (targetConfigs);
        objects.addArray (projectConfigs);
        objects.addArray (misc);

        for (auto* o : objects)
        {
            output << "\t\t" << o->getType().toString() << " = {";

            for (int j = 0; j < o->getNumProperties(); ++j)
            {
                const Identifier propertyName (o->getPropertyName(j));
                String val (o->getProperty (propertyName).toString());

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-_@~\r\n\\#%^`*")
                                        && ! (val.trimStart().startsWithChar ('(')
                                                || val.trimStart().startsWithChar ('{'))))
                    val = "\"" + val + "\"";

                output << propertyName.toString() << " = " << val << "; ";
            }

            output << "};\n";
        }

        output << "\t};\n\trootObject = " << createID ("__root") << ";\n}\n";
    }

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings, XCodeTarget* xcodeTarget = nullptr) const
    {
        String fileID (createID (path + "buildref"));

        if (addToSourceBuildPhase)
        {
            if (xcodeTarget != nullptr) xcodeTarget->sourceIDs.add (fileID);
            else sourceIDs.add (fileID);
        }

        ValueTree* v = new ValueTree (fileID);
        v->setProperty ("isa", "PBXBuildFile", nullptr);
        v->setProperty ("fileRef", fileRefID, nullptr);

        if (inhibitWarnings)
            v->setProperty ("settings", "{COMPILER_FLAGS = \"-w\"; }", nullptr);

        pbxBuildFiles.add (v);
        return fileID;
    }

    String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings, XCodeTarget* xcodeTarget = nullptr) const
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

        String fileType = getFileType (path);

        return addFileOrFolderReference (pathString, sourceTree, fileType);
    }

    String addFileOrFolderReference (String pathString, String sourceTree, String fileType) const
    {
        const String fileRefID (createFileRefID (pathString));

        ScopedPointer<ValueTree> v (new ValueTree (fileRefID));
        v->setProperty ("isa", "PBXFileReference", nullptr);
        v->setProperty ("lastKnownFileType", fileType, nullptr);
        v->setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), nullptr);
        v->setProperty ("path", pathString, nullptr);
        v->setProperty ("sourceTree", sourceTree, nullptr);

        const int existing = pbxFileReferences.indexOfSorted (*this, v);

        if (existing >= 0)
        {
            // If this fails, there's either a string hash collision, or the same file is being added twice (incorrectly)
            jassert (pbxFileReferences.getUnchecked (existing)->isEquivalentTo (*v));
        }
        else
        {
            pbxFileReferences.addSorted (*this, v.release());
        }

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
                    bool shouldBeAddedToXcodeResources, bool inhibitWarnings, XCodeTarget* xcodeTarget) const
    {
        const String pathAsString (path.toUnixStyle());
        const String refID (addFileReference (path.toUnixStyle()));

        if (shouldBeCompiled)
        {
            addBuildFile (pathAsString, refID, true, inhibitWarnings, xcodeTarget);
        }
        else if (! shouldBeAddedToBinaryResources || shouldBeAddedToXcodeResources)
        {
            const String fileType (getFileType (path));

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
        const String pathAsString (path.toUnixStyle());
        const String refID (addFileReference (path.toUnixStyle()));

        if (projectItem.isModuleCode())
        {
            if (XCodeTarget* xcodeTarget = getTargetOfType (getProject().getTargetTypeFromFilePath (projectItem.getFile(), false)))
            {
                String rezFileID = addBuildFile (pathAsString, refID, false, false, xcodeTarget);
                xcodeTarget->rezFileIDs.add (rezFileID);

                return refID;
            }
        }

        return {};
    }

    String getEntitlementsFileName() const
    {
        return project.getProjectFilenameRoot() + String (".entitlements");
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
            if (isiOS() && isPushNotificationsEnabled())
                entitlements.set ("aps-environment", "<string>development</string>");
        }

        if (isAppGroupsEnabled())
        {
            auto appGroups = StringArray::fromTokens (getAppGroupIdString(), ";", { });
            auto groups = String ("<array>");

            for (auto group : appGroups)
                groups += "\n\t\t<string>" + group.trim() + "</string>";

            groups += "\n\t</array>";

            entitlements.set ("com.apple.security.application-groups", groups);
        }

        return entitlements;
    }

    String addEntitlementsFile (StringPairArray entitlements) const
    {
        String content =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n";

        const auto keys = entitlements.getAllKeys();

        for (auto& key : keys)
        {
            content += "\t<key>" + key + "</key>\n"
                       "\t" + entitlements[key] + "\n";
        }
        content += "</dict>\n"
                   "</plist>\n";

        File entitlementsFile = getTargetFolder().getChildFile (getEntitlementsFileName());
        overwriteFileIfDifferentOrThrow (entitlementsFile, content);

        RelativePath plistPath (entitlementsFile, getTargetFolder(), RelativePath::buildTargetFolder);
        return addFile (plistPath, false, false, false, false, nullptr);
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
                const String childID (addProjectItem (projectItem.getChild(i)));

                if (childID.isNotEmpty())
                    childIDs.add (childID);
            }

            return addGroup (projectItem, childIDs);
        }

        if (projectItem.shouldBeAddedToTargetProject())
        {
            const String itemPath (projectItem.getFilePath());
            RelativePath path;

            if (itemPath.startsWith ("${"))
                path = RelativePath (itemPath, RelativePath::unknown);
            else
                path = RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            if (path.hasFileExtension (".r"))
                return addRezFile (projectItem, path);

            XCodeTarget* xcodeTarget = nullptr;
            if (projectItem.isModuleCode() && projectItem.shouldBeCompiled())
                xcodeTarget = getTargetOfType (project.getTargetTypeFromFilePath (projectItem.getFile(), false));

            return addFile (path, projectItem.shouldBeCompiled(),
                            projectItem.shouldBeAddedToBinaryResources(),
                            projectItem.shouldBeAddedToXcodeResources(),
                            projectItem.shouldInhibitWarnings(),
                            xcodeTarget);
        }

        return {};
    }

    String addFramework (const String& frameworkName) const
    {
        String path (frameworkName);
        if (! File::isAbsolutePath (path))
            path = "System/Library/Frameworks/" + path;

        if (! path.endsWithIgnoreCase (".framework"))
            path << ".framework";

        const String fileRefID (createFileRefID (path));

        addFileReference ((File::isAbsolutePath (frameworkName) ? "" : "${SDKROOT}/") + path);
        frameworkFileIDs.add (fileRefID);

        return addBuildFile (path, fileRefID, false, false);
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs) const
    {
        ValueTree* v = new ValueTree (groupID);
        v->setProperty ("isa", "PBXGroup", nullptr);
        v->setProperty ("children", indentParenthesisedList (childIDs), nullptr);
        v->setProperty (Ids::name, groupName, nullptr);
        v->setProperty ("sourceTree", "<group>", nullptr);
        pbxGroups.add (v);
    }

    String addGroup (const Project::Item& item, StringArray& childIDs) const
    {
        const String groupName (item.getName());
        const String groupID (getIDForGroup (item));
        addGroup (groupID, groupName, childIDs);
        return groupID;
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", nullptr);
        v->setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
        v->setProperty (Ids::name, configName, nullptr);
        projectConfigs.add (v);
    }

    void addConfigList (XCodeTarget& target, const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        ValueTree* v = new ValueTree (listID);
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

        ValueTree* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", indentParenthesisedList (configIDs), nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (auto* first = configsToUse.getFirst())
            v->setProperty ("defaultConfigurationName", first->getProperty (Ids::name), nullptr);

        misc.add (v);
    }

    void addProjectObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__root"));
        v->setProperty ("isa", "PBXProject", nullptr);
        v->setProperty ("buildConfigurationList", createID ("__projList"), nullptr);
        v->setProperty ("attributes", getProjectObjectAttributes(), nullptr);
        v->setProperty ("compatibilityVersion", "Xcode 3.2", nullptr);
        v->setProperty ("hasScannedForEncodings", (int) 0, nullptr);
        v->setProperty ("mainGroup", createID ("__mainsourcegroup"), nullptr);
        v->setProperty ("projectDirPath", "\"\"", nullptr);
        v->setProperty ("projectRoot", "\"\"", nullptr);

        String targetString = "(" + targetIDs.joinIntoString (", ") + ")";
        v->setProperty ("targets", targetString, nullptr);
        misc.add (v);
    }

    //==============================================================================
    void removeMismatchedXcuserdata() const
    {
        if (settings ["keepCustomXcodeSchemes"])
            return;

        File xcuserdata = getProjectBundle().getChildFile ("xcuserdata");

        if (! xcuserdata.exists())
            return;

        if (! xcuserdataMatchesTargets (xcuserdata))
        {
            xcuserdata.deleteRecursively();
            getProjectBundle().getChildFile ("project.xcworkspace").deleteRecursively();
        }
    }

    bool xcuserdataMatchesTargets (const File& xcuserdata) const
    {
        Array<File> xcschemeManagementPlists;
        xcuserdata.findChildFiles (xcschemeManagementPlists, File::findFiles, true, "xcschememanagement.plist");

        for (auto& plist : xcschemeManagementPlists)
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
            names.add (target->getXCodeSchemeName());

        names.sort (false);
        return names;
    }

    bool xcschemeManagementPlistMatchesTargets (const File& plist) const
    {
        ScopedPointer<XmlElement> xml (XmlDocument::parse (plist));

        if (xml != nullptr)
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
            { "iphone", "29x29",     "Icon-29.png",                "1x", 29  },
            { "iphone", "29x29",     "Icon-29@2x.png",             "2x", 58  },
            { "iphone", "29x29",     "Icon-29@3x.png",             "3x", 87  },
            { "iphone", "40x40",     "Icon-Spotlight-40@2x.png",   "2x", 80  },
            { "iphone", "40x40",     "Icon-Spotlight-40@3x.png",   "3x", 120 },
            { "iphone", "57x57",     "Icon.png",                   "1x", 57  },
            { "iphone", "57x57",     "Icon@2x.png",                "2x", 114 },
            { "iphone", "60x60",     "Icon-60@2x.png",             "2x", 120 },
            { "iphone", "60x60",     "Icon-@3x.png",               "3x", 180 },
            { "ipad",   "29x29",     "Icon-Small-1.png",           "1x", 29  },
            { "ipad",   "29x29",     "Icon-Small@2x-1.png",        "2x", 58  },
            { "ipad",   "40x40",     "Icon-Spotlight-40.png",      "1x", 40  },
            { "ipad",   "40x40",     "Icon-Spotlight-40@2x-1.png", "2x", 80  },
            { "ipad",   "50x50",     "Icon-Small-50.png",          "1x", 50  },
            { "ipad",   "50x50",     "Icon-Small-50@2x.png",       "2x", 100 },
            { "ipad",   "72x72",     "Icon-72.png",                "1x", 72  },
            { "ipad",   "72x72",     "Icon-72@2x.png",             "2x", 144 },
            { "ipad",   "76x76",     "Icon-76.png",                "1x", 76  },
            { "ipad",   "76x76",     "Icon-76@2x.png",             "2x", 152 },
            { "ipad",   "83.5x83.5", "Icon-83.5@2x.png",           "2x", 167 }
        };

        return Array<AppIconType> (types, numElementsInArray (types));
    }

    static String getiOSAppIconContents()
    {
        var images;

        for (auto& type : getiOSAppIconTypes())
        {
            DynamicObject::Ptr d = new DynamicObject();
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

        attributes << "{ LastUpgradeCheck = 0830; ";

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
            DynamicObject::Ptr d = new DynamicObject();
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

    void createXcassetsFolderFromIcons() const
    {
        const File assets (getTargetFolder().getChildFile (project.getProjectFilenameRoot())
                                            .getChildFile ("Images.xcassets"));
        const File iconSet (assets.getChildFile ("AppIcon.appiconset"));
        const File launchImage (assets.getChildFile ("LaunchImage.launchimage"));

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
    static String indentBracedList (const StringArray& list)        { return "{" + indentList (list, ";", 0, true) + " }"; }
    static String indentParenthesisedList (const StringArray& list) { return "(" + indentList (list, ",", 1, false) + " )"; }

    static String indentList (const StringArray& list, const String& separator, int extraTabs, bool shouldSort)
    {
        if (list.size() == 0)
            return " ";

        const String tabs ("\n" + String::repeatedString ("\t", extraTabs + 4));

        if (shouldSort)
        {
            StringArray sorted (list);
            sorted.sort (true);

            return tabs + sorted.joinIntoString (separator + tabs) + separator;
        }

        return tabs + list.joinIntoString (separator + tabs) + separator;
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        rootString += project.getProjectUID();

        return MD5 (rootString.toUTF8()).toHexString().substring (0, 24).toUpperCase();
    }

    String createFileRefID (const RelativePath& path) const     { return createFileRefID (path.toUnixStyle()); }
    String createFileRefID (const String& path) const           { return createID ("__fileref_" + path); }
    String getIDForGroup (const Project::Item& item) const      { return createID (item.getID()); }

    bool shouldFileBeCompiledByDefault (const RelativePath& file) const override
    {
        return file.hasFileExtension (sourceFileExtensions);
    }

    static String getOSXVersionName (int version)
    {
        jassert (version >= 4);
        return "10." + String (version);
    }

    static String getSDKName (int version)
    {
        return getOSXVersionName (version) + " SDK";
    }

    void initialiseDependencyPathValues()
    {
        vst3Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vst3Folder), Ids::vst3Path, TargetOS::osx)));
        aaxPath. referTo (Value (new DependencyPathValueSource (getSetting (Ids::aaxFolder),  Ids::aaxPath,  TargetOS::osx)));
        rtasPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::rtasFolder), Ids::rtasPath, TargetOS::osx)));
    }

    JUCE_DECLARE_NON_COPYABLE (XCodeProjectExporter)
};
