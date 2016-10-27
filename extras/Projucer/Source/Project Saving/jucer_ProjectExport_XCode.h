/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../Application/jucer_Application.h"
#include "jucer_TextWithDefaultPropertyComponent.h"

namespace
{
    const char* const osxVersionDefault         = "default";
    const int oldestSDKVersion  = 5;
    const int currentSDKVersion = 12;

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
    Value getPListToMergeValue()            { return getSetting ("customPList"); }
    String getPListToMergeString() const    { return settings   ["customPList"]; }

    Value getExtraFrameworksValue()         { return getSetting (Ids::extraFrameworks); }
    String getExtraFrameworksString() const { return settings   [Ids::extraFrameworks]; }

    Value  getPostBuildScriptValue()        { return getSetting (Ids::postbuildCommand); }
    String getPostBuildScript() const       { return settings   [Ids::postbuildCommand]; }

    Value  getPreBuildScriptValue()         { return getSetting (Ids::prebuildCommand); }
    String getPreBuildScript() const        { return settings   [Ids::prebuildCommand]; }

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

    Value  getIosDevelopmentTeamIDValue()            { return getSetting (Ids::iosDevelopmentTeamID); }
    String getIosDevelopmentTeamIDString() const     { return settings   [Ids::iosDevelopmentTeamID]; }

    bool usesMMFiles() const override                { return true; }
    bool canCopeWithDuplicateFiles() override        { return true; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                    { return true; }
    bool isVisualStudio() const override             { return false; }
    bool isCodeBlocks() const override               { return false; }
    bool isMakefile() const override                 { return false; }
    bool isAndroidStudio() const override            { return false; }
    bool isAndroidAnt() const override               { return false; }

    bool isAndroid() const override                  { return false; }
    bool isWindows() const override                  { return false; }
    bool isLinux() const override                    { return false; }
    bool isOSX() const override                      { return ! iOS; }
    bool isiOS() const override                      { return iOS; }

    bool supportsVST() const override                { return ! iOS; }
    bool supportsVST3() const override               { return ! iOS; }
    bool supportsAAX() const override                { return ! iOS; }
    bool supportsRTAS() const override               { return ! iOS; }
    bool supportsAU()   const override               { return ! iOS; }
    bool supportsAUv3() const override               { return true;  }
    bool supportsStandalone() const override         { return true;  }

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
            if (target->xcodeCreatePList)
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

        if (! ProjucerApplication::getApp().isRunningCommandLine)
        {
            // Workaround for a bug where Xcode thinks the project is invalid if opened immedietely
            // after writing
            Thread::sleep (2000);
        }
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType& type) override
    {
        if (type.isGUIApplication())
            targets.add (new Target (Target::GUIApp, *this));

        else if (type.isCommandLineApp())
            targets.add (new Target (Target::ConsoleApp, *this));

        else if (type.isStaticLibrary())
            targets.add (new Target (Target::StaticLibrary, *this));

        else if (type.isDynamicLibrary())
            targets.add (new Target (Target::DynamicLibrary, *this));

        else if (type.isAudioPlugin())
        {
            if (project.shouldBuildVST().getValue() && supportsVST())
                targets.add (new Target (Target::VSTPlugIn, *this));

            if (project.shouldBuildVST3().getValue() && supportsVST3())
                targets.add (new Target (Target::VST3PlugIn, *this));

            if (project.shouldBuildAU().getValue() && supportsAU())
                targets.add (new Target (Target::AudioUnitPlugIn, *this));

            if (project.shouldBuildAUv3().getValue())
                targets.add (new Target (Target::AudioUnitv3PlugIn, *this));

            if (project.shouldBuildAAX().getValue() && supportsAAX())
                targets.add (new Target (Target::AAXPlugIn, *this));

            if (project.shouldBuildStandalone().getValue())
                targets.add (new Target (Target::StandalonePlugIn, *this));

            if (project.shouldBuildRTAS().getValue() && supportsRTAS())
            {
                targets.add (new Target (Target::RTASPlugIn, *this));
                addRTASPluginSettings();
            }

            if (targets.size() > 0)
                targets.add (new Target (Target::SharedCodeTarget, *this));
        }

        if (targets.size() > 1)
            targets.insert (0, new Target (Target::AggregateTarget, *this));

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
              osxSDKVersion               (config, Ids::osxSDK,               nullptr, "default"),
              osxDeploymentTarget         (config, Ids::osxCompatibility,     nullptr, "default"),
              iosDeploymentTarget         (config, Ids::iosCompatibility,     nullptr, "default"),
              osxArchitecture             (config, Ids::osxArchitecture,      nullptr, "default"),
              customXcodeFlags            (config, Ids::customXcodeFlags,     nullptr),
              cppLanguageStandard         (config, Ids::cppLanguageStandard,  nullptr),
              cppStandardLibrary          (config, Ids::cppLibType,           nullptr),
              codeSignIdentity            (config, Ids::codeSigningIdentity,  nullptr, iOS ? "iPhone Developer" : "Mac Developer"),
              fastMathEnabled             (config, Ids::fastMath,             nullptr),
              linkTimeOptimisationEnabled (config, Ids::linkTimeOptimisation, nullptr),
              stripLocalSymbolsEnabled    (config, Ids::stripLocalSymbols,    nullptr),
              vstBinaryLocation           (config, Ids::xcodeVstBinaryLocation,       nullptr, "$(HOME)/Library/Audio/Plug-Ins/VST/"),
              vst3BinaryLocation          (config, Ids::xcodeVst3BinaryLocation,      nullptr, "$(HOME)/Library/Audio/Plug-Ins/VST3/"),
              auBinaryLocation            (config, Ids::xcodeAudioUnitBinaryLocation, nullptr, "$(HOME)/Library/Audio/Plug-Ins/Components/"),
              rtasBinaryLocation          (config, Ids::xcodeRtasBinaryLocation,      nullptr, "/Library/Application Support/Digidesign/Plug-Ins/"),
              aaxBinaryLocation           (config, Ids::xcodeAaxBinaryLocation,       nullptr, "/Library/Application Support/Avid/Audio/Plug-Ins/")
        {
        }

        //==========================================================================
        bool iOS;

        CachedValue<String> osxSDKVersion, osxDeploymentTarget, iosDeploymentTarget, osxArchitecture,
                            customXcodeFlags, cppLanguageStandard, cppStandardLibrary, codeSignIdentity;
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

            const char* cppLanguageStandardNames[] = { "Use Default", "C++98", "GNU++98", "C++11", "GNU++11", "C++14", "GNU++14", nullptr };
            Array<var> cppLanguageStandardValues;
            cppLanguageStandardValues.add (var());
            cppLanguageStandardValues.add ("c++98");
            cppLanguageStandardValues.add ("gnu++98");
            cppLanguageStandardValues.add ("c++11");
            cppLanguageStandardValues.add ("gnu++11");
            cppLanguageStandardValues.add ("c++14");
            cppLanguageStandardValues.add ("gnu++14");

            props.add (new ChoicePropertyComponent (cppLanguageStandard.getPropertyAsValue(), "C++ Language Standard",
                                                    StringArray (cppLanguageStandardNames), cppLanguageStandardValues),
                       "The standard of the C++ language that will be used for compilation.");

            const char* cppLibNames[] = { "Use Default", "LLVM libc++", "GNU libstdc++", nullptr };
            Array<var> cppLibValues;
            cppLibValues.add (var());
            cppLibValues.add ("libc++");
            cppLibValues.add ("libstdc++");

            props.add (new ChoicePropertyComponent (cppStandardLibrary.getPropertyAsValue(), "C++ Library", StringArray (cppLibNames), cppLibValues),
                       "The type of C++ std lib that will be linked.");

            props.add (new TextWithDefaultPropertyComponent<String> (codeSignIdentity, "Code-signing Identity", 1024),
                       "The name of a code-signing identity for Xcode to apply.");

            props.add (new BooleanPropertyComponent (fastMathEnabled.getPropertyAsValue(), "Relax IEEE compliance", "Enabled"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new BooleanPropertyComponent (linkTimeOptimisationEnabled.getPropertyAsValue(), "Link-Time Optimisation", "Enabled"),
                       "Enable this to perform link-time code generation. This is recommended for release builds.");

            props.add (new BooleanPropertyComponent (stripLocalSymbolsEnabled.getPropertyAsValue(), "Strip local symbols", "Enabled"),
                       "Enable this to strip any locally defined symbols resulting in a smaller binary size. Enabling this will also remove any function names from crash logs. Must be disabled for static library projects.");
        }

    private:
        //==========================================================================
        void addXcodePluginInstallPathProperties (PropertyListBuilder& props)
        {
            if (project.shouldBuildVST().getValue())
                props.add (new TextWithDefaultPropertyComponent<String> (vstBinaryLocation, "VST Binary location", 1024),
                           "The folder in which the compiled VST binary should be placed.");

            if (project.shouldBuildVST3().getValue())
                props.add (new TextWithDefaultPropertyComponent<String> (vst3BinaryLocation, "VST3 Binary location", 1024),
                           "The folder in which the compiled VST3 binary should be placed.");

            if (project.shouldBuildAU().getValue())
                props.add (new TextWithDefaultPropertyComponent<String> (auBinaryLocation, "AU Binary location", 1024),
                           "The folder in which the compiled AU binary should be placed.");

            if (project.shouldBuildRTAS().getValue())
                props.add (new TextWithDefaultPropertyComponent<String> (rtasBinaryLocation, "RTAS Binary location", 1024),
                           "The folder in which the compiled RTAS binary should be placed.");

            if (project.shouldBuildAAX().getValue())
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
    struct Target
    {
        enum Type
        {
            GUIApp            = 0,
            ConsoleApp        = 1,
            StaticLibrary     = 2,
            DynamicLibrary    = 3,

            VSTPlugIn         = 10,
            VST3PlugIn        = 11,
            AAXPlugIn         = 12,
            RTASPlugIn        = 13,
            AudioUnitPlugIn   = 14,
            AudioUnitv3PlugIn = 15,
            StandalonePlugIn  = 16,

            SharedCodeTarget  = 20, // internal
            AggregateTarget   = 21,

            unspecified       = 30
        };

        //==============================================================================
        Target (Type targetType, const XCodeProjectExporter& exporter)
            : type (targetType),
              owner (exporter)
        {
            switch (type)
            {
                case GUIApp:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = true;
                    xcodeCreatePList = true;
                    xcodePackageType = "APPL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case ConsoleApp:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = true;
                    xcodeCreatePList = false;
                    xcodeFileType = "compiled.mach-o.executable";
                    xcodeBundleExtension = String();
                    xcodeProductType = "com.apple.product-type.tool";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case StaticLibrary:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = false;
                    xcodeFileType = "archive.ar";
                    xcodeProductType = "com.apple.product-type.library.static";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case DynamicLibrary:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = false;
                    xcodeFileType = "compiled.mach-o.dylib";
                    xcodeProductType = "com.apple.product-type.library.dynamic";
                    xcodeBundleExtension = ".dylib";
                    xcodeCopyToProductInstallPathAfterBuild = false;

                    break;

                case VSTPlugIn:
                    xcodeIsBundle = true;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    break;

                case VST3PlugIn:
                    xcodeIsBundle = true;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".vst3";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    break;

                case AudioUnitPlugIn:
                    xcodeIsBundle = true;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
                    xcodePackageType = "BNDL";
                    xcodeBundleSignature = "????";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".component";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    addExtraAudioUnitTargetSettings();
                    break;

                case StandalonePlugIn:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = true;
                    xcodeCreatePList = true;
                    xcodePackageType = "APPL";
                    xcodeBundleSignature = "????";
                    xcodeCreatePList = true;
                    xcodeFileType = "wrapper.application";
                    xcodeBundleExtension = ".app";
                    xcodeProductType = "com.apple.product-type.application";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case AudioUnitv3PlugIn:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
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
                    xcodeIsBundle = true;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
                    xcodePackageType = "TDMw";
                    xcodeBundleSignature = "PTul";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".aaxplugin";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    addExtraAAXTargetSettings();
                    break;

                case RTASPlugIn:
                    xcodeIsBundle = true;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = true;
                    xcodePackageType = "TDMw";
                    xcodeBundleSignature = "PTul";
                    xcodeFileType = "wrapper.cfbundle";
                    xcodeBundleExtension = ".dpm";
                    xcodeProductType = "com.apple.product-type.bundle";
                    xcodeCopyToProductInstallPathAfterBuild = true;

                    addExtraRTASTargetSettings();
                    break;

                case SharedCodeTarget:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = false;
                    xcodeFileType = "archive.ar";
                    xcodeProductType = "com.apple.product-type.library.static";
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                case AggregateTarget:
                    xcodeIsBundle = false;
                    xcodeIsExecutable = false;
                    xcodeCreatePList = false;
                    xcodeCopyToProductInstallPathAfterBuild = false;
                    break;

                default:
                    // unknown target type!
                    jassertfalse;
                    break;
            }
        }

        const char* getName() const noexcept
        {
            switch (type)
            {
                case GUIApp:            return "App";
                case ConsoleApp:        return "ConsoleApp";
                case StaticLibrary:     return "Static Library";
                case DynamicLibrary:    return "Dynamic Library";
                case VSTPlugIn:         return "VST";
                case VST3PlugIn:        return "VST3";
                case AudioUnitPlugIn:   return "AU";
                case StandalonePlugIn:  return "AUv3 Standalone";
                case AudioUnitv3PlugIn: return "AUv3 AppExtension";
                case AAXPlugIn:         return "AAX";
                case RTASPlugIn:        return "RTAS";
                case SharedCodeTarget:  return "Shared Code";
                case AggregateTarget:   return "All";
                default:                return "undefined";
            }
        }

        String getXCodeSchemeName() const
        {
            return owner.projectName + " (" + getName() + ")";
        }

        bool shouldBuildVST()  const                      { return owner.supportsVST()  && owner.project.shouldBuildVST().getValue()  && (type == SharedCodeTarget || type == VSTPlugIn); }
        bool shouldBuildVST3() const                      { return owner.supportsVST3() && owner.project.shouldBuildVST3().getValue() && (type == SharedCodeTarget || type == VST3PlugIn); }
        bool shouldBuildAAX()  const                      { return owner.supportsAAX()  && owner.project.shouldBuildAAX().getValue()  && (type == SharedCodeTarget || type == AAXPlugIn); }
        bool shouldBuildRTAS() const                      { return owner.supportsRTAS() && owner.project.shouldBuildRTAS().getValue() && (type == SharedCodeTarget || type == RTASPlugIn); }
        bool shouldBuildAU()   const                      { return owner.supportsAU()   && owner.project.shouldBuildAU().getValue()   && (type == SharedCodeTarget || type == AudioUnitPlugIn); }
        bool shouldBuildAUv3() const                      { return owner.supportsAUv3() && owner.project.shouldBuildAUv3().getValue() && (type == SharedCodeTarget || type == AudioUnitv3PlugIn); }
        bool shouldBuildStandalone() const                { return owner.project.shouldBuildStandalone().getValue()                   && (type == SharedCodeTarget || type == StandalonePlugIn); }

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
        bool xcodeIsBundle, xcodeCreatePList, xcodeIsExecutable, xcodeCopyToProductInstallPathAfterBuild;
        StringArray xcodeFrameworks, xcodeLibs;
        Type type;
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
                    productName = getLibbedFilename (productName);
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
            String attributes;

            attributes << getID() << " = { ";

            String developmentTeamID = owner.getIosDevelopmentTeamIDString();
            if (developmentTeamID.isNotEmpty())
                attributes << "DevelopmentTeam = " << developmentTeamID << "; ";

            const int inAppPurchasesEnabled = (owner.iOS && owner.isInAppPurchasesEnabled()) ? 1 : 0;
            const int sandboxEnabled = (type == Target::AudioUnitv3PlugIn ? 1 : 0);

            attributes << "SystemCapabilities = {";
            attributes << "com.apple.InAppPurchase = { enabled = " << inAppPurchasesEnabled << "; }; ";
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

        //==============================================================================
        StringArray getTargetSettings (const XcodeBuildConfiguration& config) const
        {
            if (type == AggregateTarget)
                // the aggregate target should not specify any settings at all!
                // it just defines dependencies on the other targets.
                return StringArray();

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

            s.add ("HEADER_SEARCH_PATHS = " + owner.getHeaderSearchPaths (config));
            s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());

            if (xcodeCreatePList)
                s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

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

            if (xcodeIsBundle)
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
                const String sdk (config.osxSDKVersion.get());
                const String sdkCompat (config.osxDeploymentTarget.get());

                // if the user doesn't set it, then use the last known version that works well with JUCE
                String deploymentTarget = "10.11";

                for (int ver = oldestSDKVersion; ver <= currentSDKVersion; ++ver)
                {
                    if (sdk == getSDKName (ver))         s.add ("SDKROOT = macosx10." + String (ver));
                    if (sdkCompat == getSDKName (ver))   deploymentTarget = "10." + String (ver);
                }

                s.add ("MACOSX_DEPLOYMENT_TARGET = " + deploymentTarget);

                s.add ("MACOSX_DEPLOYMENT_TARGET_ppc = 10.4");
                s.add ("SDKROOT_ppc = macosx10.5");

                if (xcodeExcludedFiles64Bit.isNotEmpty())
                {
                    s.add ("EXCLUDED_SOURCE_FILE_NAMES = \"$(EXCLUDED_SOURCE_FILE_NAMES_$(CURRENT_ARCH))\"");
                    s.add ("EXCLUDED_SOURCE_FILE_NAMES_x86_64 = " + xcodeExcludedFiles64Bit);
                }
            }

            s.add ("GCC_VERSION = " + gccVersion);
            s.add ("CLANG_CXX_LANGUAGE_STANDARD = \"c++0x\"");
            s.add ("CLANG_LINK_OBJC_RUNTIME = NO");

            if (! config.codeSignIdentity.isUsingDefault())
                s.add ("CODE_SIGN_IDENTITY = " + config.codeSignIdentity.get().quoted());

            if (config.cppLanguageStandard.get().isNotEmpty())
                s.add ("CLANG_CXX_LANGUAGE_STANDARD = " + config.cppLanguageStandard.get().quoted());

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

                    for (int i = 0; i < librarySearchPaths.size(); ++i)
                        libPaths += ", \"\\\"" + librarySearchPaths[i] + "\\\"\"";

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

            if (type == Target::SharedCodeTarget)
                defines.set ("JUCE_SHARED_CODE", "1");

            if (owner.project.getProjectType().isAudioPlugin() && type == Target::AudioUnitv3PlugIn &&  owner.isOSX())
                s.add (String ("CODE_SIGN_ENTITLEMENTS = \"") + owner.getEntitlementsFileName() + String ("\""));

            if (owner.project.getProjectType().isAudioPlugin())
            {
                defines.set ("JucePlugin_Build_VST",        (shouldBuildVST()        ? "1" : "0"));
                defines.set ("JucePlugin_Build_VST3",       (shouldBuildVST3()       ? "1" : "0"));
                defines.set ("JucePlugin_Build_AU",         (shouldBuildAU()         ? "1" : "0"));
                defines.set ("JucePlugin_Build_AUv3",       (shouldBuildAUv3()       ? "1" : "0"));
                defines.set ("JucePlugin_Build_RTAS",       (shouldBuildRTAS()       ? "1" : "0"));
                defines.set ("JucePlugin_Build_AAX",        (shouldBuildAAX()        ? "1" : "0"));
                defines.set ("JucePlugin_Build_Standalone", (shouldBuildStandalone() ? "1" : "0"));
            }

            defines = mergePreprocessorDefs (defines, owner.getAllPreprocessorDefs (config));

            StringArray defsList;

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines.getAllKeys()[i]);
                const String value (defines.getAllValues()[i]);
                if (value.isNotEmpty())
                    def << "=" << value.replace ("\"", "\\\"");

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
                default:                return String();
            }
        }

        //==============================================================================
        void getLinkerSettings (const BuildConfiguration& config, StringArray& flags, StringArray& librarySearchPaths) const
        {
            if (xcodeIsBundle)
                flags.add (owner.isiOS() ? "-bitcode_bundle" : "-bundle");

            const Array<RelativePath>& extraLibs = config.isDebug() ? xcodeExtraLibrariesDebug
                                                                    : xcodeExtraLibrariesRelease;

            for (auto& lib : extraLibs)
            {
                flags.add (getLinkerFlagForLib (lib.getFileNameWithoutExtension()));
                librarySearchPaths.add (owner.getSearchPathForStaticLibrary (lib));
            }

            if (owner.project.getProjectType().isAudioPlugin() && type != Target::SharedCodeTarget)
            {
                if (owner.getTargetOfType (Target::SharedCodeTarget) != nullptr)
                {
                    String productName (getLibbedFilename (owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString())));

                    RelativePath sharedCodelib (productName, RelativePath::buildTargetFolder);
                    flags.add (getLinkerFlagForLib (sharedCodelib.getFileNameWithoutExtension()));
                }
            }

            flags.add (owner.replacePreprocessorTokens (config, owner.getExtraLinkerFlagsString()));
            flags.add (owner.getExternalLibraryFlags (config));

            StringArray libs (owner.xcodeLibs);
            libs.addArray (xcodeLibs);

            for (int i = 0; i < libs.size(); ++i)
                flags.add (getLinkerFlagForLib (libs[i]));

            flags = getCleanedStringArray (flags);
        }

        //========================================================================== c
        void writeInfoPlistFile() const
        {
            if (! xcodeCreatePList)
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

                for (int i = 0; i < documentExtensions.size(); ++i)
                {
                    String ex (documentExtensions[i]);
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

            if (owner.iOS && type != AudioUnitv3PlugIn)
            {
                // Forcing full screen disables the split screen feature and prevents error ITMS-90475
                addPlistDictionaryKeyBool (dict, "UIRequiresFullScreen", true);
                addPlistDictionaryKeyBool (dict, "UIStatusBarHidden", true);

                addIosScreenOrientations (dict);
                addIosBackgroundModes (dict);
            }

            for (int i = 0; i < xcodeExtraPListEntries.size(); ++i)
                dict->addChildElement (new XmlElement (xcodeExtraPListEntries.getReference(i)));

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

            addArrayToPlist (dict, "UIBackgroundModes", iosBackgroundModes);
        }

        //==============================================================================
        static void addArrayToPlist (XmlElement* dict, String arrayKey, const StringArray& arrayElements)
        {
            dict->createNewChildElement ("key")->addTextElement (arrayKey);
            XmlElement* plistStringArray = dict->createNewChildElement ("array");

            for (int i = 0; i < arrayElements.size(); ++i)
                plistStringArray->createNewChildElement ("string")->addTextElement (arrayElements[i]);
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

            tagsArray->createNewChildElement ("string")->addTextElement (static_cast<bool> (owner.project.getPluginIsSynth().getValue()) ? "Synth" : "Effects");

            xcodeExtraPListEntries.add (plistKey);
            xcodeExtraPListEntries.add (plistEntry);
        }

        void addExtraAAXTargetSettings()
        {
            const RelativePath aaxLibsFolder = RelativePath (owner.getAAXPathValue().toString(), RelativePath::projectFolder).getChildFile ("Libs");

            xcodeExtraLibrariesDebug.add   (aaxLibsFolder.getChildFile ("Debug/libAAXLibrary.a"));
            xcodeExtraLibrariesRelease.add (aaxLibsFolder.getChildFile ("Release/libAAXLibrary.a"));
        }

        void addExtraRTASTargetSettings()
        {
            RelativePath rtasFolder (owner.getRTASPathValue().toString(), RelativePath::projectFolder);

            xcodeExtraLibrariesDebug.add   (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
            xcodeExtraLibrariesRelease.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));
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
    OwnedArray<Target> targets;

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
        for (int targetIdx = 0; targetIdx < targets.size(); ++targetIdx)
        {
            Target& target = *targets[targetIdx];

            if (target.type == Target::AggregateTarget)
                continue;

            target.addMainBuildProduct();

            String targetName = target.getName();
            String fileID (createID (targetName + String ("__targetbuildref")));
            String fileRefID (createID (String ("__productFileID") + targetName));

            ValueTree* v = new ValueTree (fileID);
            v->setProperty ("isa", "PBXBuildFile", nullptr);
            v->setProperty ("fileRef", fileRefID, nullptr);

            target.mainBuildProductID = fileID;

            pbxBuildFiles.add (v);
            target.addDependency();
        }
    }

    void addPlistFileReferences() const
    {
        for (int targetIdx = 0; targetIdx < targets.size(); ++targetIdx)
        {
            Target& target = *targets[targetIdx];

            if (target.type == Target::AggregateTarget)
                continue;

            if (target.xcodeCreatePList)
            {
                RelativePath plistPath (target.infoPlistFile, getTargetFolder(), RelativePath::buildTargetFolder);
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
        if (! isiOS() && project.getProjectType().isAudioPlugin())
            topLevelGroupIDs.add (addEntitlementsFile());

        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            const Project::Item& group = getAllGroups().getReference(i);

            if (group.getNumChildren() > 0)
                topLevelGroupIDs.add (addProjectItem (group));
        }
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
        for (int i = 0; i < targets.size(); ++i)
        {
            Target& target = *targets[i];

            if (target.type != Target::AggregateTarget)
                buildProducts.add (createID (String ("__productFileID") + String (target.getName())));

            for (ConstConfigIterator config (*this); config.next();)
            {
                const XcodeBuildConfiguration& xcodeConfig = dynamic_cast<const XcodeBuildConfiguration&> (*config);
                target.addTargetConfig (config->getName(), target.getTargetSettings (xcodeConfig));
            }

            addConfigList (target, targetConfigs, createID (String ("__configList") + target.getName()));

            target.addShellScriptBuildPhase ("Pre-build script", getPreBuildScript());

            if (target.type != Target::AggregateTarget)
            {
                // TODO: ideally resources wouldn't be copied into the AUv3 bundle as well.
                // However, fixing this requires supporting App groups -> TODO: add app groups
                if (! projectType.isStaticLibrary() && target.type != Target::SharedCodeTarget)
                    target.addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

                StringArray rezFiles (rezFileIDs);
                rezFiles.addArray (target.rezFileIDs);

                if (rezFiles.size() > 0)
                    target.addBuildPhase ("PBXRezBuildPhase", rezFiles);

                StringArray sourceFiles (target.sourceIDs);

                if (target.type == Target::SharedCodeTarget
                     || (! project.getProjectType().isAudioPlugin()))
                    sourceFiles.addArray (sourceIDs);

                target.addBuildPhase ("PBXSourcesBuildPhase", sourceFiles);

                if (! projectType.isStaticLibrary() && target.type != Target::SharedCodeTarget)
                    target.addBuildPhase ("PBXFrameworksBuildPhase", target.frameworkIDs);
            }

            target.addShellScriptBuildPhase ("Post-build script", getPostBuildScript());

            if (project.getProjectType().isAudioPlugin() && project.shouldBuildAUv3().getValue()
                && project.shouldBuildStandalone().getValue() && target.type == Target::StandalonePlugIn)
                embedAppExtension();

            addTargetObject (target);
        }
    }

    void embedAppExtension() const
    {
        if (Target* standaloneTarget = getTargetOfType (Target::StandalonePlugIn))
        {
            if (Target* auv3Target = getTargetOfType (Target::AudioUnitv3PlugIn))
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

        for (int i = 0; i < numElementsInArray (validSizes); ++i)
        {
            if (w == h && w == validSizes[i])
            {
                bestSize = w;
                break;
            }

            if (jmax (w, h) > validSizes[i])
                bestSize = validSizes[i];
        }

        return rescaleImageForIcon (image, bestSize);
    }

    //==============================================================================
    Target* getTargetOfType (Target::Type type) const
    {
        for (auto& target : targets)
            if (target->type == type)
                return target;

        return nullptr;
    }

    void addTargetObject (Target& target) const
    {
        String targetName = target.getName();

        String targetID = target.getID();
        ValueTree* const v = new ValueTree (targetID);
        v->setProperty ("isa", target.type == Target::AggregateTarget ? "PBXAggregateTarget" : "PBXNativeTarget", nullptr);
        v->setProperty ("buildConfigurationList", createID (String ("__configList") + targetName), nullptr);

        v->setProperty ("buildPhases", indentParenthesisedList (target.buildPhaseIDs), nullptr);
        v->setProperty ("buildRules", "( )", nullptr);

        v->setProperty ("dependencies", indentParenthesisedList (getTargetDependencies (target)), nullptr);
        v->setProperty (Ids::name, target.getXCodeSchemeName(), nullptr);
        v->setProperty ("productName", projectName, nullptr);

        if (target.type != Target::AggregateTarget)
        {
            v->setProperty ("productReference", createID (String ("__productFileID") + targetName), nullptr);

            jassert (target.xcodeProductType.isNotEmpty());
            v->setProperty ("productType", target.xcodeProductType, nullptr);
        }

        targetIDs.add (targetID);
        misc.add (v);
    }

    StringArray getTargetDependencies (const Target& target) const
    {
        StringArray dependencies;

        if (project.getProjectType().isAudioPlugin())
        {
            if (target.type == Target::StandalonePlugIn) // depends on AUv3 and shared code
            {
                if (Target* auv3Target = getTargetOfType (Target::AudioUnitv3PlugIn))
                    dependencies.add (auv3Target->getDependencyID());

                if (Target* sharedCodeTarget = getTargetOfType (Target::SharedCodeTarget))
                    dependencies.add (sharedCodeTarget->getDependencyID());
            }
            else if (target.type == Target::AggregateTarget) // depends on all other targets
            {
                for (int i = 1; i < targets.size(); ++i)
                    dependencies.add (targets[i]->getDependencyID());
            }
            else if (target.type != Target::SharedCodeTarget) // shared code doesn't depend on anything; all other targets depend only on the shared code
            {
                if (Target* sharedCodeTarget = getTargetOfType (Target::SharedCodeTarget))
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
        const Array<AppIconType> types (getiOSAppIconTypes());

        OwnedArray<Drawable> images;
        getIconImages (images);

        if (images.size() > 0)
        {
            for (int i = 0; i < types.size(); ++i)
            {
                const AppIconType type = types.getUnchecked(i);
                const Image image (rescaleImageForIcon (*images.getFirst(), type.size));

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

    String getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray paths (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());
        paths.add ("$(inherited)");

        paths = getCleanedStringArray (paths);

        for (int i = 0; i < paths.size(); ++i)
        {
            String& s = paths.getReference(i);

            s = replacePreprocessorTokens (config, s);

            if (s.containsChar (' '))
                s = "\"\\\"" + s + "\\\"\""; // crazy double quotes required when there are spaces..
            else
                s = "\"" + s + "\"";
        }

        return "(" + paths.joinIntoString (", ") + ")";
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
        s.add ("GCC_C_LANGUAGE_STANDARD = c99");
        s.add ("GCC_WARN_ABOUT_RETURN_TYPE = YES");
        s.add ("GCC_WARN_CHECK_SWITCH_STATEMENTS = YES");
        s.add ("GCC_WARN_UNUSED_VARIABLE = YES");
        s.add ("GCC_WARN_MISSING_PARENTHESES = YES");
        s.add ("GCC_WARN_NON_VIRTUAL_DESTRUCTOR = YES");
        s.add ("GCC_WARN_TYPECHECK_CALLS_TO_PRINTF = YES");
        s.add ("WARNING_CFLAGS = -Wreorder");
        s.add ("GCC_MODEL_TUNING = G5");

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

            for (int i = 0; i < s.size(); ++i)
            {
                String frameworkID = addFramework (s[i]);

                // find all the targets that are referring to this object
                for (auto& target : targets)
                    if (xcodeFrameworks.contains (s[i]) || target->xcodeFrameworks.contains (s[i]))
                        target->frameworkIDs.add (frameworkID);
            }
        }
    }

    void addCustomResourceFolders() const
    {
        StringArray crf;

        crf.addTokens (getCustomResourceFoldersString(), ":", "");
        crf.trim();

        for (int i = 0; i < crf.size(); ++i)
            addCustomResourceFolder (crf[i]);
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

        for (int i = 0; i < objects.size(); ++i)
        {
            ValueTree& o = *objects.getUnchecked(i);
            output << "\t\t" << o.getType().toString() << " = {";

            for (int j = 0; j < o.getNumProperties(); ++j)
            {
                const Identifier propertyName (o.getPropertyName(j));
                String val (o.getProperty (propertyName).toString());

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

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings, Target* xcodeTarget = nullptr) const
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

    String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings, Target* xcodeTarget = nullptr) const
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
        v->setProperty ("path", sanitisePath (pathString), nullptr);
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
                    bool shouldBeAddedToXcodeResources, bool inhibitWarnings, Target* xcodeTarget) const
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
            if (Target* xcodeTarget = getTargetOfType (getTargetTypeFromFilePath (projectItem.getFile(), false)))
            {
                String rezFileID = addBuildFile (pathAsString, refID, false, false, xcodeTarget);
                xcodeTarget->rezFileIDs.add (rezFileID);

                return refID;
            }
        }

        return String();
    }

    String getEntitlementsFileName() const
    {
        return project.getProjectFilenameRoot() + String (".entitlements");
    }

    String addEntitlementsFile() const
    {
        const char* sandboxEntitlement =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
            "<plist version=\"1.0\">"
            "<dict>"
            " <key>com.apple.security.app-sandbox</key>"
            "  <true/>"
            "</dict>"
            "</plist>";

        File entitlementsFile = getTargetFolder().getChildFile (getEntitlementsFileName());
        overwriteFileIfDifferentOrThrow (entitlementsFile, sandboxEntitlement);

        RelativePath plistPath (entitlementsFile, getTargetFolder(), RelativePath::buildTargetFolder);
        return addFile (plistPath, false, false, false, false, nullptr);
    }

    String addProjectItem (const Project::Item& projectItem) const
    {
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

            Target* xcodeTarget = nullptr;
            if (projectItem.isModuleCode() && projectItem.shouldBeCompiled())
                xcodeTarget = getTargetOfType (getTargetTypeFromFilePath (projectItem.getFile(), false));

            return addFile (path, projectItem.shouldBeCompiled(),
                            projectItem.shouldBeAddedToBinaryResources(),
                            projectItem.shouldBeAddedToXcodeResources(),
                            projectItem.shouldInhibitWarnings(),
                            xcodeTarget);
        }

        return String();
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

    void addConfigList (Target& target, const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        ValueTree* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", indentParenthesisedList (target.configIDs), nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (configsToUse[0] != nullptr)
            v->setProperty ("defaultConfigurationName", configsToUse[0]->getProperty (Ids::name), nullptr);

        misc.add (v);
    }

    void addProjectConfigList (const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        StringArray configIDs;

        for (int i = 0; i < configsToUse.size(); ++i)
            configIDs.add (configsToUse[i]->getType().toString());

        ValueTree* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", indentParenthesisedList (configIDs), nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (configsToUse[0] != nullptr)
            v->setProperty ("defaultConfigurationName", configsToUse[0]->getProperty (Ids::name), nullptr);

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

    static Target::Type getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffic)
    {
        if      (LibraryModule::CompileUnit::hasSuffix (file, "_AU"))         return Target::AudioUnitPlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_AUv3"))       return Target::AudioUnitv3PlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_AAX"))        return Target::AAXPlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_RTAS"))       return Target::RTASPlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_VST2"))       return Target::VSTPlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_VST3"))       return Target::VST3PlugIn;
        else if (LibraryModule::CompileUnit::hasSuffix (file, "_Standalone")) return Target::StandalonePlugIn;

        return (returnSharedTargetIfNoValidSuffic ? Target::SharedCodeTarget : Target::unspecified);
    }

    //==============================================================================
    void removeMismatchedXcuserdata() const
    {
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

        return StringArray();
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
        const Array<AppIconType> types (getiOSAppIconTypes());
        var images;

        for (int i = 0; i < types.size(); ++i)
        {
            AppIconType type = types.getUnchecked(i);

            DynamicObject::Ptr d = new DynamicObject();
            d->setProperty ("idiom",    type.idiom);
            d->setProperty ("size",     type.sizeString);
            d->setProperty ("filename", type.filename);
            d->setProperty ("scale",    type.scale);
            images.append (var (d));
        }

        return getiOSAssetContents (images);
    }

    String getProjectObjectAttributes() const
    {
        String attributes;

        attributes << "{ LastUpgradeCheck = 0440; ";

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
        const Array<ImageType> types (getiOSLaunchImageTypes());
        var images;

        for (int i = 0; i < types.size(); ++i)
        {
            const ImageType& type = types.getReference(i);

            DynamicObject::Ptr d = new DynamicObject();
            d->setProperty ("orientation", type.orientation);
            d->setProperty ("idiom", type.idiom);
            d->setProperty ("extent",  type.extent);
            d->setProperty ("minimum-system-version", "7.0");
            d->setProperty ("scale", type.scale);
            d->setProperty ("filename", type.filename);

            if (type.subtype != nullptr)
                d->setProperty ("subtype", type.subtype);

            images.append (var (d));
        }

        return getiOSAssetContents (images);
    }

    static void createiOSLaunchImageFiles (const File& launchImageSet)
    {
        const Array<ImageType> types (getiOSLaunchImageTypes());

        for (int i = 0; i < types.size(); ++i)
        {
            const ImageType& type = types.getReference(i);

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

        return JSON::toString (var (v));
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

    void addRTASPluginSettings()
    {
        xcodeCanUseDwarf = false;

        extraSearchPaths.add ("$(DEVELOPER_DIR)/Headers/FlatCarbon");
        extraSearchPaths.add ("$(SDKROOT)/Developer/Headers/FlatCarbon");

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

        RelativePath rtasFolder (getRTASPathValue().toString(), RelativePath::projectFolder);

        for (int i = 0; i < numElementsInArray (p); ++i)
            addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
    }

    JUCE_DECLARE_NON_COPYABLE (XCodeProjectExporter)
};
