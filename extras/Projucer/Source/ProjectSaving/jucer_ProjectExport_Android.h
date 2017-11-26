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


//==============================================================================
class AndroidProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    bool isXcode() const override                { return false; }
    bool isVisualStudio() const override         { return false; }
    bool isCodeBlocks() const override           { return false; }
    bool isMakefile() const override             { return false; }
    bool isAndroidStudio() const override        { return true;  }
    bool isCLion() const override                { return false; }

    bool isAndroid() const override              { return true; }
    bool isWindows() const override              { return false; }
    bool isLinux() const override                { return false; }
    bool isOSX() const override                  { return false; }
    bool isiOS() const override                  { return false; }

    bool usesMMFiles() const override                       { return false; }
    bool canCopeWithDuplicateFiles() override               { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool supportsTargetType (ProjectType::Target::Type type) const override
    {
        switch (type)
        {
            case ProjectType::Target::GUIApp:
            case ProjectType::Target::StaticLibrary:
            case ProjectType::Target::StandalonePlugIn:
                return true;
            default:
                break;
        }

        return false;
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        // no-op.
    }

    //==============================================================================
    void createExporterProperties (PropertyListBuilder& props) override
    {
        createBaseExporterProperties (props);
        createToolchainExporterProperties (props);
        createManifestExporterProperties (props);
        createLibraryModuleExporterProperties (props);
        createCodeSigningExporterProperties (props);
        createOtherExporterProperties (props);
    }

    static const char* getName()                         { return "Android"; }
    static const char* getValueTreeTypeName()            { return "ANDROIDSTUDIO"; }

    static AndroidProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new AndroidProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        sdkPath.referTo  (Value (new DependencyPathValueSource (getSetting (Ids::androidSDKPath), Ids::androidSDKPath, TargetOS::getThisOS())));
        ndkPath.referTo  (Value (new DependencyPathValueSource (getSetting (Ids::androidNDKPath), Ids::androidNDKPath, TargetOS::getThisOS())));
    }

    //==============================================================================
    CachedValue<String> androidRepositories, androidDependencies,
                        androidScreenOrientation, androidActivityClass, androidActivitySubClassName,
                        androidManifestCustomXmlElements, androidVersionCode, androidMinimumSDK, androidTheme,
                        androidSharedLibraries, androidStaticLibraries, androidExtraAssetsFolder;

    CachedValue<bool>   androidInternetNeeded, androidMicNeeded, androidBluetoothNeeded,
                        androidExternalReadPermission, androidExternalWritePermission,
                        androidInAppBillingPermission, androidVibratePermission;
    CachedValue<String> androidOtherPermissions;

    CachedValue<bool>   androidEnableRemoteNotifications;
    CachedValue<String> androidRemoteNotificationsConfigFile;

    CachedValue<String> androidKeyStore, androidKeyStorePass, androidKeyAlias, androidKeyAliasPass;

    CachedValue<String> gradleVersion, androidPluginVersion, gradleToolchain, buildToolsVersion;

    //==============================================================================
    AndroidProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          androidRepositories (settings, Ids::androidRepositories, nullptr, ""),
          androidDependencies (settings, Ids::androidDependencies, nullptr, ""),
          androidScreenOrientation (settings, Ids::androidScreenOrientation, nullptr, "unspecified"),
          androidActivityClass (settings, Ids::androidActivityClass, nullptr, createDefaultClassName()),
          androidActivitySubClassName (settings, Ids::androidActivitySubClassName, nullptr),
          androidManifestCustomXmlElements (settings, Ids::androidManifestCustomXmlElements, nullptr, ""),
          androidVersionCode (settings, Ids::androidVersionCode, nullptr, "1"),
          androidMinimumSDK (settings, Ids::androidMinimumSDK, nullptr, "10"),
          androidTheme (settings, Ids::androidTheme, nullptr),
          androidSharedLibraries (settings, Ids::androidSharedLibraries, nullptr, ""),
          androidStaticLibraries (settings, Ids::androidStaticLibraries, nullptr, ""),
          androidExtraAssetsFolder (settings, Ids::androidExtraAssetsFolder, nullptr, ""),
          androidInternetNeeded (settings, Ids::androidInternetNeeded, nullptr, true),
          androidMicNeeded (settings, Ids::microphonePermissionNeeded, nullptr, false),
          androidBluetoothNeeded (settings, Ids::androidBluetoothNeeded, nullptr, true),
          androidExternalReadPermission  (settings, Ids::androidExternalReadNeeded, nullptr, true),
          androidExternalWritePermission (settings, Ids::androidExternalWriteNeeded, nullptr, true),
          androidInAppBillingPermission (settings, Ids::androidInAppBilling, nullptr, false),
          androidVibratePermission (settings, Ids::androidVibratePermissionNeeded, nullptr, false),
          androidOtherPermissions (settings, Ids::androidOtherPermissions, nullptr),
          androidEnableRemoteNotifications (settings, Ids::androidEnableRemoteNotifications, nullptr, false),
          androidRemoteNotificationsConfigFile (settings, Ids::androidRemoteNotificationsConfigFile, nullptr, ""),
          androidKeyStore (settings, Ids::androidKeyStore, nullptr, "${user.home}/.android/debug.keystore"),
          androidKeyStorePass (settings, Ids::androidKeyStorePass, nullptr, "android"),
          androidKeyAlias (settings, Ids::androidKeyAlias, nullptr, "androiddebugkey"),
          androidKeyAliasPass (settings, Ids::androidKeyAliasPass, nullptr, "android"),
          gradleVersion (settings, Ids::gradleVersion, nullptr, "3.3"),
          androidPluginVersion (settings, Ids::androidPluginVersion, nullptr, "2.3.3"),
          gradleToolchain (settings, Ids::gradleToolchain, nullptr, "clang"),
          buildToolsVersion (settings, Ids::buildToolsVersion, nullptr, "26.0.0"),
          AndroidExecutable (findAndroidExecutable())
    {
        name = getName();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "Android";
    }

    //==============================================================================
    void createToolchainExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextWithDefaultPropertyComponent<String> (gradleVersion, "gradle version", 32),
                   "The version of gradle that is used to build this app (3.3 is fine for JUCE)");

        props.add (new TextWithDefaultPropertyComponent<String> (androidPluginVersion, "android plug-in version", 32),
                   "The version of the android build plugin for gradle that is used to build this app");

        static const char* toolchains[] = { "clang", "gcc", nullptr };

        props.add (new ChoicePropertyComponent (gradleToolchain.getPropertyAsValue(), "NDK Toolchain", StringArray (toolchains), Array<var> (toolchains)),
                   "The toolchain that gradle should invoke for NDK compilation (variable model.android.ndk.tooclhain in app/build.gradle)");

        props.add (new TextWithDefaultPropertyComponent<String> (buildToolsVersion, "Android build tools version", 32),
                   "The Android build tools version that should use to build this app");
    }

    void createLibraryModuleExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidStaticLibraries.getPropertyAsValue(), "Import static library modules", 8192, true),
                   "Comma or whitespace delimited list of static libraries (.a) defined in NDK_MODULE_PATH.");

        props.add (new TextPropertyComponent (androidSharedLibraries.getPropertyAsValue(), "Import shared library modules", 8192, true),
                   "Comma or whitespace delimited list of shared libraries (.so) defined in NDK_MODULE_PATH.");
    }

    //==============================================================================
    bool canLaunchProject() override
    {
        return AndroidExecutable.exists();
    }

    bool launchProject() override
    {
        if (! AndroidExecutable.exists())
        {
            jassertfalse;
            return false;
        }

        const File targetFolder (getTargetFolder());

        // we have to surround the path with extra quotes, otherwise Android Studio
        // will choke if there are any space characters in the path.
        return AndroidExecutable.startAsProcess ("\"" + targetFolder.getFullPathName() + "\"");
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        const File targetFolder (getTargetFolder());
        const File appFolder (targetFolder.getChildFile (isLibrary() ? "lib" : "app"));

        removeOldFiles (targetFolder);

        if (! isLibrary())
            copyJavaFiles (modules);

        copyExtraResourceFiles();

        writeFile (targetFolder, "settings.gradle",  isLibrary() ? "include ':lib'" : "include ':app'");
        writeFile (targetFolder, "build.gradle",     getProjectBuildGradleFileContent());
        writeFile (appFolder,    "build.gradle",     getAppBuildGradleFileContent());
        writeFile (targetFolder, "local.properties", getLocalPropertiesFileContent());
        writeFile (targetFolder, "gradle/wrapper/gradle-wrapper.properties", getGradleWrapperPropertiesFileContent());

        writeBinaryFile (targetFolder, "gradle/wrapper/LICENSE-for-gradlewrapper.txt", BinaryData::LICENSE, BinaryData::LICENSESize);
        writeBinaryFile (targetFolder, "gradle/wrapper/gradle-wrapper.jar", BinaryData::gradlewrapper_jar, BinaryData::gradlewrapper_jarSize);
        writeBinaryFile (targetFolder, "gradlew",                           BinaryData::gradlew,           BinaryData::gradlewSize);
        writeBinaryFile (targetFolder, "gradlew.bat",                       BinaryData::gradlew_bat,       BinaryData::gradlew_batSize);

        targetFolder.getChildFile ("gradlew").setExecutePermission (true);

        writeAndroidManifest (appFolder);

        if (! isLibrary())
        {
            writeStringsXML      (targetFolder);
            writeAppIcons        (targetFolder);
        }

        writeCmakeFile (appFolder.getChildFile ("CMakeLists.txt"));

        const String androidExtraAssetsFolderValue = androidExtraAssetsFolder.get();
        if (androidExtraAssetsFolderValue.isNotEmpty())
        {
            File extraAssets (getProject().getFile().getParentDirectory().getChildFile(androidExtraAssetsFolderValue));
            if (extraAssets.exists() && extraAssets.isDirectory())
            {
                const File assetsFolder (appFolder.getChildFile ("src/main/assets"));
                if (assetsFolder.deleteRecursively())
                    extraAssets.copyDirectoryTo (assetsFolder);
            }
        }
    }

    void removeOldFiles (const File& targetFolder) const
    {
        targetFolder.getChildFile ("app/build").deleteRecursively();
        targetFolder.getChildFile ("app/build.gradle").deleteFile();
        targetFolder.getChildFile ("gradle").deleteRecursively();
        targetFolder.getChildFile ("local.properties").deleteFile();
        targetFolder.getChildFile ("settings.gradle").deleteFile();
    }

    void writeFile (const File& gradleProjectFolder, const String& filePath, const String& fileContent) const
    {
        MemoryOutputStream outStream;
        outStream << fileContent;
        overwriteFileIfDifferentOrThrow (gradleProjectFolder.getChildFile (filePath), outStream);
    }

    void writeBinaryFile (const File& gradleProjectFolder, const String& filePath, const char* binaryData, const int binarySize) const
    {
        MemoryOutputStream outStream;
        outStream.write (binaryData, static_cast<size_t> (binarySize));
        overwriteFileIfDifferentOrThrow (gradleProjectFolder.getChildFile (filePath), outStream);
    }

    //==============================================================================
    static File findAndroidExecutable()
    {
       #if JUCE_WINDOWS
        const File defaultInstallation ("C:\\Program Files\\Android\\Android Studio\\bin");

        if (defaultInstallation.exists())
        {
            {
                const File studio64 = defaultInstallation.getChildFile ("studio64.exe");

                if (studio64.existsAsFile())
                    return studio64;
            }

            {
                const File studio = defaultInstallation.getChildFile ("studio.exe");

                if (studio.existsAsFile())
                    return studio;
            }
        }
      #elif JUCE_MAC
       const File defaultInstallation ("/Applications/Android Studio.app");

       if (defaultInstallation.exists())
           return defaultInstallation;
      #endif

        return {};
    }

protected:
    //==============================================================================
    class AndroidBuildConfiguration  : public BuildConfiguration
    {
    public:
        AndroidBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
            if (getArchitectures().isEmpty())
            {
                if (isDebug())
                    getArchitecturesValue() = "armeabi x86";
                else
                    getArchitecturesValue() = "";
            }
        }

        Value getArchitecturesValue()           { return getValue (Ids::androidArchitectures); }
        String getArchitectures() const         { return config [Ids::androidArchitectures]; }

        Value getAdditionalXmlResourcesValue()    { return getValue (Ids::androidAdditionalXmlValueResources); }
        String getAdditionalXmlResources() const  { return config [Ids::androidAdditionalXmlValueResources]; }

        Value getAdditionalRawResourcesValue()    { return getValue (Ids::androidAdditionalRawValueResources); }
        String getAdditionalRawResources() const  { return config [Ids::androidAdditionalRawValueResources]; }

        Value getCustomStringsXmlValue()        { return getValue (Ids::androidCustomStringXmlElements); }
        String getCustomStringsXml() const      { return config [Ids::androidCustomStringXmlElements]; }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            props.add (new TextPropertyComponent (getArchitecturesValue(), "Architectures", 256, false),
                       "A list of the ARM architectures to build (for a fat binary). Leave empty to build for all possible android archiftectures.");

            props.add (new TextPropertyComponent (getAdditionalXmlResourcesValue(), "Extra Android XML Value Resources", 2048, true),
                       "Paths to additional \"value resource\" files in XML format that should be included in the app (one per line). "
                       "If you have additional XML resources that should be treated as value resources, add them here.");

            props.add (new TextPropertyComponent (getAdditionalRawResourcesValue(), "Extra Android Raw Resources", 2048, true),
                       "Paths to additional \"raw resource\" files that should be included in the app (one per line). "
                       "Resource file names must contain only lowercase a-z, 0-9 or underscore.");

            props.add (new TextPropertyComponent (getCustomStringsXmlValue(), "Custom string resources", 8192, true),
                       "Custom XML resources that will be added to string.xml as children of <resources> element. "
                       "Example: \n<string name=\"value\">text</string>\n"
                       "<string name2=\"value2\">text2</string>\n");
        }

        String getProductFlavourNameIdentifier() const
        {
            return getName().toLowerCase().replaceCharacter (L' ', L'_') + String ("_");
        }

        String getProductFlavourCMakeIdentifier() const
        {
            return getName().toUpperCase().replaceCharacter (L' ', L'_');
        }

        String getModuleLibraryArchName() const override
        {
            return "${ANDROID_ABI}";
        }

        String getLinkerFlagsString() const
        {
            return String ("\"-DCMAKE_EXE_LINKER_FLAGS_") + (isDebug() ? "DEBUG" : "RELEASE") + "=-flto\"";
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new AndroidBuildConfiguration (project, v, *this);
    }

private:
    void writeCmakeFile (const File& file) const
    {
        MemoryOutputStream mo;

        mo << "# Automatically generated makefile, created by the Projucer" << newLine
           << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
           << newLine;

        mo << "cmake_minimum_required(VERSION 3.4.1)" << newLine << newLine;

        if (! isLibrary())
            mo << "SET(BINARY_NAME \"juce_jni\")" << newLine << newLine;

        String cpufeaturesPath ("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c");
        mo << "add_library(\"cpufeatures\" STATIC \"" << cpufeaturesPath << "\")" << newLine
           << "set_source_files_properties(\"" << cpufeaturesPath << "\" PROPERTIES COMPILE_FLAGS \"-Wno-sign-conversion -Wno-gnu-statement-expression\")" << newLine << newLine;

        {
            StringArray projectDefines (getEscapedPreprocessorDefs (getProjectPreprocessorDefs()));
            if (projectDefines.size() > 0)
                mo << "add_definitions(" << projectDefines.joinIntoString (" ") << ")" << newLine << newLine;
        }

        {
            mo << "include_directories( AFTER" << newLine;

            for (auto& path : extraSearchPaths)
                mo << "    \"" << escapeDirectoryForCmake (path) << "\"" << newLine;

            mo << "    \"${ANDROID_NDK}/sources/android/cpufeatures\"" << newLine;
            mo << ")" << newLine << newLine;
        }

        const String& cfgExtraLinkerFlags = getExtraLinkerFlagsString();
        if (cfgExtraLinkerFlags.isNotEmpty())
        {
            mo << "SET( JUCE_LDFLAGS \"" << cfgExtraLinkerFlags.replace ("\"", "\\\"") << "\")" << newLine;
            mo << "SET( CMAKE_SHARED_LINKER_FLAGS  \"${CMAKE_EXE_LINKER_FLAGS} ${JUCE_LDFLAGS}\")" << newLine << newLine;
        }

        mo << "enable_language(ASM)" << newLine << newLine;

        StringArray userLibraries = StringArray::fromTokens(getExternalLibrariesString(), ";", "");
        userLibraries.addArray (androidLibs);

        if (getNumConfigurations() > 0)
        {
            bool first = true;

            for (ConstConfigIterator config (*this); config.next();)
            {
                const auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);
                const StringArray& libSearchPaths = cfg.getLibrarySearchPaths();
                const StringPairArray& cfgDefines = getConfigPreprocessorDefs (cfg);
                const StringArray& cfgHeaderPaths  = cfg.getHeaderSearchPaths();
                const StringArray& cfgLibraryPaths = cfg.getLibrarySearchPaths();

                if (! isLibrary() && libSearchPaths.size() == 0 && cfgDefines.size() == 0
                       && cfgHeaderPaths.size() == 0 && cfgLibraryPaths.size() == 0)
                    continue;

                mo << (first ? "IF" : "ELSEIF") << "(JUCE_BUILD_CONFIGFURATION MATCHES \"" << cfg.getProductFlavourCMakeIdentifier() <<"\")" << newLine;

                if (isLibrary())
                    mo << "    SET(BINARY_NAME \"" << getNativeModuleBinaryName (cfg) << "\")" << newLine;

                writeCmakePathLines (mo, "    ", "link_directories(", libSearchPaths);

                if (cfgDefines.size() > 0)
                    mo << "    add_definitions(" << getEscapedPreprocessorDefs (cfgDefines).joinIntoString (" ") << ")" << newLine;

                writeCmakePathLines (mo, "    ", "include_directories( AFTER", cfgHeaderPaths);

                if (userLibraries.size() > 0)
                {
                    for (auto& lib : userLibraries)
                    {
                        String findLibraryCmd;
                        findLibraryCmd << "find_library(" << lib.toLowerCase().replaceCharacter (L' ', L'_')
                            << " \"" << lib << "\" PATHS";

                        writeCmakePathLines (mo, "    ", findLibraryCmd, cfgLibraryPaths, "    NO_CMAKE_FIND_ROOT_PATH)");
                    }

                    mo << newLine;
                }

                first = false;
            }

            if (! first)
            {
                ProjectExporter::BuildConfiguration::Ptr config (getConfiguration(0));

                if (config)
                {
                    if (const auto* cfg = dynamic_cast<const AndroidBuildConfiguration*> (config.get()))
                    {
                        mo << "ELSE(JUCE_BUILD_CONFIGFURATION MATCHES \"" << cfg->getProductFlavourCMakeIdentifier() <<"\")" << newLine;
                        mo << "    MESSAGE( FATAL_ERROR \"No matching build-configuration found.\" )" << newLine;
                        mo << "ENDIF(JUCE_BUILD_CONFIGFURATION MATCHES \"" << cfg->getProductFlavourCMakeIdentifier() <<"\")" << newLine << newLine;
                    }
                }
            }
        }

        Array<RelativePath> excludeFromBuild;

        mo << "add_library( ${BINARY_NAME}" << newLine;
        mo << newLine;
        mo << "    " << (getProject().getProjectType().isStaticLibrary() ? "STATIC" : "SHARED") << newLine;
        mo << newLine;
        addCompileUnits (mo, excludeFromBuild);
        mo << ")" << newLine << newLine;

        if (excludeFromBuild.size() > 0)
        {
            for (auto& exclude : excludeFromBuild)
                mo << "set_source_files_properties(\"" << exclude.toUnixStyle() << "\" PROPERTIES HEADER_FILE_ONLY TRUE)" << newLine;

            mo << newLine;
        }

        StringArray libraries (getAndroidLibraries());
        if (libraries.size() > 0)
        {
            for (auto& lib : libraries)
                mo << "find_library(" << lib.toLowerCase().replaceCharacter (L' ', L'_') << " \"" << lib << "\")" << newLine;

            mo << newLine;
        }

        libraries.addArray (userLibraries);
        mo << "target_link_libraries( ${BINARY_NAME}";
        if (libraries.size() > 0)
        {
            mo << newLine << newLine;

            for (auto& lib : libraries)
                mo << "    ${" << lib.toLowerCase().replaceCharacter (L' ', L'_') << "}" << newLine;

            mo << "    \"cpufeatures\"" << newLine;
        }
        mo << ")" << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    //==============================================================================
    String getProjectBuildGradleFileContent() const
    {
        MemoryOutputStream mo;

        mo << "buildscript {"                                                                          << newLine;
        mo << "   repositories {"                                                                      << newLine;
        mo << "       jcenter()"                                                                       << newLine;
        mo << "   }"                                                                                   << newLine;
        mo << "   dependencies {"                                                                      << newLine;
        mo << "       classpath 'com.android.tools.build:gradle:" << androidPluginVersion.get() << "'" << newLine;

        if (androidEnableRemoteNotifications.get())
            mo << "       classpath 'com.google.gms:google-services:3.1.0'" << newLine;

        mo << "   }"                                                                                   << newLine;
        mo << "}"                                                                                      << newLine;
        mo << ""                                                                                       << newLine;
        mo << "allprojects {"                                                                          << newLine;
        mo << "   repositories {"                                                                      << newLine;
        mo << "       jcenter()"                                                                       << newLine;

        if (androidEnableRemoteNotifications.get())
        {
            mo << "       maven {"                                                                     << newLine;
            mo << "           url \"https://maven.google.com\""                                        << newLine;
            mo << "       }"                                                                           << newLine;
        }

        mo << "   }"                                                                                   << newLine;
        mo << "}"                                                                                      << newLine;

        return mo.toString();
    }

    //==============================================================================
    String getAppBuildGradleFileContent() const
    {
        MemoryOutputStream mo;
        mo << "apply plugin: 'com.android." << (isLibrary() ? "library" : "application") << "'" << newLine << newLine;

        mo << "android {"                                                         << newLine;
        mo << "    compileSdkVersion " << androidMinimumSDK.get().getIntValue()   << newLine;
        mo << "    buildToolsVersion \"" << buildToolsVersion.get() << "\""       << newLine;
        mo << "    externalNativeBuild {"                                         << newLine;
        mo << "        cmake {"                                                   << newLine;
        mo << "            path \"CMakeLists.txt\""                               << newLine;
        mo << "        }"                                                         << newLine;
        mo << "    }"                                                             << newLine;

        mo << getAndroidSigningConfig()                                           << newLine;
        mo << getAndroidDefaultConfig()                                           << newLine;
        mo << getAndroidBuildTypes()                                              << newLine;
        mo << getAndroidProductFlavours()                                         << newLine;
        mo << getAndroidVariantFilter()                                           << newLine;

        mo << getAndroidRepositories()                                            << newLine;
        mo << getAndroidDependencies()                                            << newLine;
        mo << getApplyPlugins()                                                   << newLine;

        mo << "}"                                                                 << newLine << newLine;

        return mo.toString();
    }

    String getAndroidProductFlavours() const
    {
        MemoryOutputStream mo;

        mo << "    productFlavors {" << newLine;

        for (ConstConfigIterator config (*this); config.next();)
        {
            const auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

            mo << "        " << cfg.getProductFlavourNameIdentifier() << " {" << newLine;

            if (cfg.getArchitectures().isNotEmpty())
            {
                mo << "            ndk {" << newLine;
                mo << "                abiFilters " << toGradleList (StringArray::fromTokens (cfg.getArchitectures(),  " ", "")) << newLine;
                mo << "            }" << newLine;
            }

            mo << "            externalNativeBuild {" << newLine;
            mo << "                cmake {"           << newLine;

            if (getProject().getProjectType().isStaticLibrary())
                mo << "                    targets \"" << getNativeModuleBinaryName (cfg) << "\"" << newLine;

            mo << "                    arguments \"-DJUCE_BUILD_CONFIGFURATION=" << cfg.getProductFlavourCMakeIdentifier() << "\""
                                           << ", \"-DCMAKE_CXX_FLAGS_" << (cfg.isDebug() ? "DEBUG" : "RELEASE")
                                           << "=-O" << cfg.getGCCOptimisationFlag() << "\""
                                           << ", \"-DCMAKE_C_FLAGS_"   << (cfg.isDebug() ? "DEBUG" : "RELEASE")
                                           << "=-O" << cfg.getGCCOptimisationFlag() << "\""
                                           << (cfg.isLinkTimeOptimisationEnabled() ? ", " + cfg.getLinkerFlagsString() : "")
                                           << newLine;
            mo << "                }"                   << newLine;
            mo << "            }"                       << newLine;
            mo << "       }"                            << newLine;
        }

        mo << "    }" << newLine;

        return mo.toString();
    }

    String getAndroidSigningConfig() const
    {
        MemoryOutputStream mo;

        String keyStoreFilePath = androidKeyStore.get().replace ("${user.home}", "${System.properties['user.home']}")
                                                       .replace ("/", "${File.separator}");

        mo << "    signingConfigs {"                                              << newLine;
        mo << "        juceSigning {"                                             << newLine;
        mo << "            storeFile     file(\"" << keyStoreFilePath << "\")"    << newLine;
        mo << "            storePassword \"" << androidKeyStorePass.get() << "\"" << newLine;
        mo << "            keyAlias      \"" << androidKeyAlias.get() << "\""     << newLine;
        mo << "            keyPassword   \"" << androidKeyAliasPass.get() << "\"" << newLine;
        mo << "            storeType     \"jks\""                                 << newLine;
        mo << "        }"                                                         << newLine;
        mo << "    }"                                                             << newLine;

        return mo.toString();
    }

    String getAndroidDefaultConfig() const
    {
        const String bundleIdentifier  = project.getBundleIdentifier().toString().toLowerCase();
        const StringArray& cmakeDefs = getCmakeDefinitions();
        const StringArray& cFlags   = getProjectCompilerFlags();
        const StringArray& cxxFlags = getProjectCxxCompilerFlags();
        const int minSdkVersion = androidMinimumSDK.get().getIntValue();

        MemoryOutputStream mo;

        mo << "    defaultConfig {"                                               << newLine;

        if (! isLibrary())
            mo << "        applicationId \"" << bundleIdentifier << "\""          << newLine;

        mo << "        minSdkVersion    " << minSdkVersion                        << newLine;
        mo << "        targetSdkVersion " << minSdkVersion                        << newLine;

        mo << "        externalNativeBuild {"                                     << newLine;
        mo << "            cmake {"                                               << newLine;

        mo << "                arguments " << cmakeDefs.joinIntoString (", ")     << newLine;

        if (cFlags.size() > 0)
            mo << "                cFlags "   << cFlags.joinIntoString (", ")     << newLine;

        if (cxxFlags.size() > 0)
            mo << "                cppFlags " << cxxFlags.joinIntoString (", ")   << newLine;

        mo << "            }"                                                     << newLine;
        mo << "        }"                                                         << newLine;
        mo << "    }"                                                             << newLine;

        return mo.toString();
    }

    String getAndroidBuildTypes() const
    {
        MemoryOutputStream mo;

        mo << "    buildTypes {"                                                  << newLine;

        int numDebugConfigs = 0;
        const int numConfigs = getNumConfigurations();
        for (int i = 0; i < numConfigs; ++i)
        {
            auto config = getConfiguration(i);

            if (config->isDebug()) numDebugConfigs++;

            if (numDebugConfigs > 1 || ((numConfigs - numDebugConfigs) > 1))
                continue;

            mo << "         " << (config->isDebug() ? "debug" : "release") << " {"      << newLine;
            mo << "             initWith " << (config->isDebug() ? "debug" : "release") << newLine;
            mo << "             debuggable    " << (config->isDebug() ? "true" : "false") << newLine;
            mo << "             jniDebuggable " << (config->isDebug() ? "true" : "false") << newLine;
            mo << "             signingConfig signingConfigs.juceSigning" << newLine;

            mo << "         }" << newLine;
        }
        mo << "    }" << newLine;

        return mo.toString();
    }

    String getAndroidVariantFilter() const
    {
        MemoryOutputStream mo;

        mo << "    variantFilter { variant ->"            << newLine;
        mo << "        def names = variant.flavors*.name" << newLine;

        for (ConstConfigIterator config (*this); config.next();)
        {
            const auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

            mo << "        if (names.contains (\"" << cfg.getProductFlavourNameIdentifier() << "\")"                  << newLine;
            mo << "              && variant.buildType.name != \"" << (cfg.isDebug() ? "debug" : "release") << "\") {" << newLine;
            mo << "            setIgnore(true)"                                                                       << newLine;
            mo << "        }"                                                                                         << newLine;
        }

        mo << "    }" << newLine;

        return mo.toString();
    }

    String getAndroidRepositories() const
    {
        MemoryOutputStream mo;

        juce::StringArray  repositories;
        repositories.addLines (androidRepositories.get());

        mo << "repositories {"                << newLine;

        for (const auto& r : repositories)
            mo << "    " << r << newLine;

        mo << "}"                             << newLine;

        return mo.toString();
    }

    String getAndroidDependencies() const
    {
        MemoryOutputStream mo;

        juce::StringArray  dependencies;
        dependencies.addLines (androidDependencies.get());

        mo << "dependencies {" << newLine;

        if (androidEnableRemoteNotifications.get())
        {
            mo << "    'com.google.firebase:firebase-core:11.4.0'" << newLine;
            mo << "    compile 'com.google.firebase:firebase-messaging:11.4.0'" << newLine;
        }

        for (const auto& d : dependencies)
            mo << "    " << d << newLine;

        mo << "}" << newLine;

        return mo.toString();
    }

    String getApplyPlugins() const
    {
        MemoryOutputStream mo;

        if (androidEnableRemoteNotifications.get())
            mo << "apply plugin: 'com.google.gms.google-services'" << newLine;

        return mo.toString();
    }

    //==============================================================================
    String getLocalPropertiesFileContent() const
    {
        String props;

        props << "ndk.dir=" << sanitisePath (ndkPath.toString()) << newLine
              << "sdk.dir=" << sanitisePath (sdkPath.toString()) << newLine;

        return props;
    }

    String getGradleWrapperPropertiesFileContent() const
    {
        String props;

        props << "distributionUrl=https\\://services.gradle.org/distributions/gradle-"
              << gradleVersion.get() << "-all.zip";

        return props;
    }

    //==============================================================================
    void createBaseExporterProperties (PropertyListBuilder& props)
    {
        static const char* orientations[] = { "Portrait and Landscape", "Portrait", "Landscape", nullptr };
        static const char* orientationValues[] = { "unspecified", "portrait", "landscape", nullptr };

        props.add (new TextPropertyComponent (androidRepositories.getPropertyAsValue(), "Module repositories", 32768, true),
                   "Module repositories (one per line). These will be added to module-level gradle file repositories section. ");

        props.add (new TextPropertyComponent (androidDependencies.getPropertyAsValue(), "Module dependencies", 32768, true),
                   "Module dependencies (one per line). These will be added to module-level gradle file dependencies section. ");

        props.add (new ChoicePropertyComponent (androidScreenOrientation.getPropertyAsValue(), "Screen orientation", StringArray (orientations), Array<var> (orientationValues)),
                   "The screen orientations that this app should support");

        props.add (new TextWithDefaultPropertyComponent<String> (androidActivityClass, "Android Activity class name", 256),
                   "The full java class name to use for the app's Activity class.");

        props.add (new TextPropertyComponent (androidActivitySubClassName.getPropertyAsValue(), "Android Activity sub-class name", 256, false),
                   "If not empty, specifies the Android Activity class name stored in the app's manifest. "
                   "Use this if you would like to use your own Android Activity sub-class.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidVersionCode, "Android Version Code", 32),
                   "An integer value that represents the version of the application code, relative to other versions.");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), sdkPath, "Android SDK Path"),
                   "The path to the Android SDK folder on the target build machine");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), ndkPath, "Android NDK Path"),
                   "The path to the Android NDK folder on the target build machine");

        props.add (new TextWithDefaultPropertyComponent<String> (androidMinimumSDK, "Minimum SDK version", 32),
                   "The number of the minimum version of the Android SDK that the app requires");

        props.add (new TextPropertyComponent (androidExtraAssetsFolder.getPropertyAsValue(), "Extra Android Assets", 256, false),
                   "A path to a folder (relative to the project folder) which conatins extra android assets.");
    }

    //==============================================================================
    void createManifestExporterProperties (PropertyListBuilder& props)
    {
        props.add (new BooleanPropertyComponent (androidInternetNeeded.getPropertyAsValue(), "Internet Access", "Specify internet access permission in the manifest"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidMicNeeded.getPropertyAsValue(), "Audio Input Required", "Specify audio record permission in the manifest"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidBluetoothNeeded.getPropertyAsValue(), "Bluetooth permissions Required", "Specify bluetooth permission (required for Bluetooth MIDI)"),
                   "If enabled, this will set the android.permission.BLUETOOTH and  android.permission.BLUETOOTH_ADMIN flag in the manifest. This is required for Bluetooth MIDI on Android.");

        props.add (new BooleanPropertyComponent (androidExternalReadPermission.getPropertyAsValue(), "Read from external storage", "Specify permissions to read from external storage"),
                   "If enabled, this will set the android.permission.READ_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidExternalWritePermission.getPropertyAsValue(), "Write to external storage", "Specify permissions to write to external storage"),
                   "If enabled, this will set the android.permission.WRITE_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidInAppBillingPermission.getPropertyAsValue(), "In-App Billing", "Specify In-App Billing permission in the manifest"),
                   "If enabled, this will set the com.android.vending.BILLING flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidVibratePermission.getPropertyAsValue(), "Vibrate", "Specify permissions to vibrate"),
                   "If enabled, this will set the android.permission.VIBRATE flag in the manifest.");

        props.add (new TextPropertyComponent (androidOtherPermissions.getPropertyAsValue(), "Custom permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");

        props.add (new BooleanPropertyComponent (androidEnableRemoteNotifications.getPropertyAsValue(), "Remote Notifications", "Enabled"),
                   "Enable to be able to send remote notifications to devices running your app (min API level 14). Provide Remote Notifications Config File, "
                   "configure your app in Firebase Console and ensure you have the latest Google Repository in Android Studio's SDK Manager.");

        props.add (new TextPropertyComponent (androidRemoteNotificationsConfigFile.getPropertyAsValue(), "Remote Notifications Config File", 2048, false),
                   "Path to google-services.json file. This will be the file provided by Firebase when creating a new app in Firebase console.");

        props.add (new TextPropertyComponent (androidManifestCustomXmlElements.getPropertyAsValue(), "Custom manifest XML content", 8192, true),
                   "You can specify custom AndroidManifest.xml content overriding the default one generated by Projucer. "
                   "Projucer will automatically create any missing and required XML elements and attributes "
                   "and merge them into your custom content.");
    }

    //==============================================================================
    void createCodeSigningExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyStore, "Key Signing: key.store", 2048),
                   "The key.store value, used when signing the release package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyStorePass, "Key Signing: key.store.password", 2048),
                   "The key.store password, used when signing the release package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyAlias, "Key Signing: key.alias", 2048),
                   "The key.alias value, used when signing the release package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyAliasPass, "Key Signing: key.alias.password", 2048),
                   "The key.alias password, used when signing the release package.");
    }

    //==============================================================================
    void createOtherExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidTheme.getPropertyAsValue(), "Android Theme", 256, false),
                   "E.g. @android:style/Theme.NoTitleBar or leave blank for default");
    }

    //==============================================================================
    String createDefaultClassName() const
    {
        auto s = project.getBundleIdentifier().toString().toLowerCase();

        if (s.length() > 5
            && s.containsChar ('.')
            && s.containsOnly ("abcdefghijklmnopqrstuvwxyz_.")
            && ! s.startsWithChar ('.'))
        {
            if (! s.endsWithChar ('.'))
                s << ".";
        }
        else
        {
            s = "com.yourcompany.";
        }

        return s + CodeHelpers::makeValidIdentifier (project.getProjectFilenameRoot(), false, true, false);
    }

    //==============================================================================
    void copyJavaFiles (const OwnedArray<LibraryModule>& modules) const
    {
        if (auto* coreModule = getCoreModule (modules))
        {
            auto package = getActivityClassPackage();
            auto targetFolder = getTargetFolder();

            auto inAppBillingPath = String ("com.android.vending.billing").replaceCharacter ('.', File::getSeparatorChar());
            auto javaSourceFolder = coreModule->getFolder().getChildFile ("native").getChildFile ("java");
            auto javaInAppBillingTarget = targetFolder.getChildFile ("app/src/main/java").getChildFile (inAppBillingPath);
            auto javaTarget = targetFolder.getChildFile ("app/src/main/java")
                                          .getChildFile (package.replaceCharacter ('.', File::getSeparatorChar()));

            copyActivityJavaFiles (javaSourceFolder, javaTarget, package);
            copyServicesJavaFiles (javaSourceFolder, javaTarget, package);
            copyAdditionalJavaFiles (javaSourceFolder, javaInAppBillingTarget);
        }
    }

    void copyActivityJavaFiles (const File& javaSourceFolder, const File& targetFolder, const String& package) const
    {
        if (androidActivityClass.get().contains ("_"))
            throw SaveError ("Your Android activity class name or path may not contain any underscores! Try a project name without underscores.");

        auto className = getActivityName();

        if (className.isEmpty())
            throw SaveError ("Invalid Android Activity class name: " + androidActivityClass.get());

        createDirectoryOrThrow (targetFolder);

        auto javaDestFile = targetFolder.getChildFile (className + ".java");


        String juceMidiCode, juceMidiImports, juceRuntimePermissionsCode;

        juceMidiImports << newLine;

        if (androidMinimumSDK.get().getIntValue() >= 23)
        {
            auto javaAndroidMidi = javaSourceFolder.getChildFile ("AndroidMidi.java");
            auto javaRuntimePermissions = javaSourceFolder.getChildFile ("AndroidRuntimePermissions.java");

            juceMidiImports << "import android.media.midi.*;" << newLine
                            << "import android.bluetooth.*;" << newLine
                            << "import android.bluetooth.le.*;" << newLine;

            juceMidiCode = javaAndroidMidi.loadFileAsString().replace ("JuceAppActivity", className);

            juceRuntimePermissionsCode = javaRuntimePermissions.loadFileAsString().replace ("JuceAppActivity", className);
        }
        else
        {
            juceMidiCode = javaSourceFolder.getChildFile ("AndroidMidiFallback.java")
                               .loadFileAsString()
                               .replace ("JuceAppActivity", className);
        }

        auto javaSourceFile = javaSourceFolder.getChildFile ("JuceAppActivity.java");
        auto javaSourceLines = StringArray::fromLines (javaSourceFile.loadFileAsString());

        {
            MemoryOutputStream newFile;

            for (const auto& line : javaSourceLines)
            {
                if (line.contains ("$$JuceAndroidMidiImports$$"))
                    newFile << juceMidiImports;
                else if (line.contains ("$$JuceAndroidMidiCode$$"))
                    newFile << juceMidiCode;
                else if (line.contains ("$$JuceAndroidRuntimePermissionsCode$$"))
                    newFile << juceRuntimePermissionsCode;
                else
                    newFile << line.replace ("JuceAppActivity", className)
                                   .replace ("package com.juce;", "package " + package + ";") << newLine;
            }

            javaSourceLines = StringArray::fromLines (newFile.toString());
        }

        while (javaSourceLines.size() > 2
                && javaSourceLines[javaSourceLines.size() - 1].trim().isEmpty()
                && javaSourceLines[javaSourceLines.size() - 2].trim().isEmpty())
            javaSourceLines.remove (javaSourceLines.size() - 1);

        overwriteFileIfDifferentOrThrow (javaDestFile, javaSourceLines.joinIntoString (newLine));
    }

    void copyAdditionalJavaFiles (const File& sourceFolder, const File& targetFolder) const
    {
        auto inAppBillingJavaFileName = String ("IInAppBillingService.java");

        auto inAppBillingJavaSrcFile  = sourceFolder.getChildFile (inAppBillingJavaFileName);
        auto inAppBillingJavaDestFile = targetFolder.getChildFile (inAppBillingJavaFileName);

        createDirectoryOrThrow (targetFolder);

        jassert (inAppBillingJavaSrcFile.existsAsFile());

        if (inAppBillingJavaSrcFile.existsAsFile())
            inAppBillingJavaSrcFile.copyFileTo (inAppBillingJavaDestFile);
    }

    void copyServicesJavaFiles (const File& javaSourceFolder, const File& targetFolder, const String& package) const
    {
        if (androidEnableRemoteNotifications.get())
        {
            String instanceIdFileName = "JuceFirebaseInstanceIdService.java";
            String messagingFileName  = "JuceFirebaseMessagingService.java";

            File instanceIdFile (javaSourceFolder.getChildFile (instanceIdFileName));
            File messagingFile  (javaSourceFolder.getChildFile (messagingFileName));

            jassert (instanceIdFile.existsAsFile());
            jassert (messagingFile .existsAsFile());

            Array<File> files;
            files.add (instanceIdFile);
            files.add (messagingFile);

            for (const auto& file : files)
            {
                auto newContent = file.loadFileAsString()
                                      .replace ("package com.juce;", "package " + package + ";");
                auto targetFile = targetFolder.getChildFile (file.getFileName());
                overwriteFileIfDifferentOrThrow (targetFile, newContent);
            }
        }
    }

    void copyExtraResourceFiles() const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            const auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);
            const String xmlValuesPath = cfg.isDebug() ? "app/src/debug/res/values" : "app/src/release/res/values";
            const String rawPath = cfg.isDebug() ? "app/src/debug/res/raw" : "app/src/release/res/raw";

            copyExtraResourceFiles (cfg.getAdditionalXmlResources(), xmlValuesPath);
            copyExtraResourceFiles (cfg.getAdditionalRawResources(), rawPath);
        }

        if (androidEnableRemoteNotifications.get())
        {
            File file (getProject().getFile().getChildFile (androidRemoteNotificationsConfigFile.get()));
            // Settings file must be present for remote notifications to work and it must be called google-services.json.
            jassert (file.existsAsFile() && file.getFileName() == "google-services.json");

            copyExtraResourceFiles (androidRemoteNotificationsConfigFile.get(), "app");
        }
    }

    void copyExtraResourceFiles (const String& resources, const String& dstRelativePath) const
    {
        StringArray resourcePaths;
        resourcePaths.addTokens (resources, true);

        const File parentFolder (getTargetFolder().getChildFile (dstRelativePath));

        parentFolder.createDirectory();

        for (const auto& path : resourcePaths)
        {
            File file (getProject().getFile().getChildFile (path));

            jassert (file.existsAsFile());

            if (file.existsAsFile())
                file.copyFileTo (parentFolder.getChildFile (file.getFileName()));
        }
    }

    String getActivityName() const
    {
        return androidActivityClass.get().fromLastOccurrenceOf (".", false, false);
    }

    String getActivitySubClassName() const
    {
        String activityPath = androidActivitySubClassName.get();

        return (activityPath.isEmpty()) ? getActivityName() : activityPath.fromLastOccurrenceOf (".", false, false);
    }

    String getActivityClassPackage() const
    {
        return androidActivityClass.get().upToLastOccurrenceOf (".", false, false);
    }

    String getJNIActivityClassName() const
    {
        return androidActivityClass.get().replaceCharacter ('.', '/');
    }

    static LibraryModule* getCoreModule (const OwnedArray<LibraryModule>& modules)
    {
        for (int i = modules.size(); --i >= 0;)
            if (modules.getUnchecked(i)->getID() == "juce_core")
                return modules.getUnchecked(i);

        return nullptr;
    }

    //==============================================================================
    String getNativeModuleBinaryName (const AndroidBuildConfiguration& config) const
    {
        return (isLibrary() ? File::createLegalFileName (config.getTargetBinaryNameString().trim()) : "juce_jni");
    }

    String getAppPlatform() const
    {
        int ndkVersion = androidMinimumSDK.get().getIntValue();
        if (ndkVersion == 9)
            ndkVersion = 10; // (doesn't seem to be a version '9')

        return "android-" + String (ndkVersion);
    }

    //==============================================================================
    void writeStringsXML (const File& folder) const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            const auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

            String customStringsXmlContent = "<resources>\n";
            customStringsXmlContent << "<string name=\"app_name\">" << projectName << "</string>\n";
            customStringsXmlContent << cfg.getCustomStringsXml();
            customStringsXmlContent << "\n</resources>";

            ScopedPointer<XmlElement> strings = XmlDocument::parse (customStringsXmlContent);

            const juce::String dir     = cfg.isDebug() ? "debug" : "release";
            const juce::String subPath = "app/src/" + dir + "/res/values/string.xml";

            writeXmlOrThrow (*strings, folder.getChildFile (subPath), "utf-8", 100, true);
        }
    }

    void writeAndroidManifest (const File& folder) const
    {
        ScopedPointer<XmlElement> manifest (createManifestXML());

        writeXmlOrThrow (*manifest, folder.getChildFile ("src/main/AndroidManifest.xml"), "utf-8", 100, true);
    }

    void writeIcon (const File& file, const Image& im) const
    {
        if (im.isValid())
        {
            createDirectoryOrThrow (file.getParentDirectory());

            PNGImageFormat png;
            MemoryOutputStream mo;

            if (! png.writeImageToStream (im, mo))
                throw SaveError ("Can't generate Android icon file");

            overwriteFileIfDifferentOrThrow (file, mo);
        }
    }

    void writeIcons (const File& folder) const
    {
        ScopedPointer<Drawable> bigIcon (getBigIcon());
        ScopedPointer<Drawable> smallIcon (getSmallIcon());

        if (bigIcon != nullptr && smallIcon != nullptr)
        {
            const int step = jmax (bigIcon->getWidth(), bigIcon->getHeight()) / 8;
            writeIcon (folder.getChildFile ("drawable-xhdpi/icon.png"), getBestIconForSize (step * 8, false));
            writeIcon (folder.getChildFile ("drawable-hdpi/icon.png"),  getBestIconForSize (step * 6, false));
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"),  getBestIconForSize (step * 4, false));
            writeIcon (folder.getChildFile ("drawable-ldpi/icon.png"),  getBestIconForSize (step * 3, false));
        }
        else if (Drawable* icon = bigIcon != nullptr ? bigIcon : smallIcon)
        {
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"), rescaleImageForIcon (*icon, icon->getWidth()));
        }
    }

    void writeAppIcons (const File& folder) const
    {
        writeIcons (folder.getChildFile ("app/src/main/res/"));
    }

    static String sanitisePath (String path)
    {
        return expandHomeFolderToken (path).replace ("\\", "\\\\");
    }

    static String expandHomeFolderToken (const String& path)
    {
        String homeFolder = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

        return path.replace ("${user.home}", homeFolder)
                   .replace ("~", homeFolder);
    }

    //==============================================================================
    void addCompileUnits (const Project::Item& projectItem, MemoryOutputStream& mo, Array<RelativePath>& excludeFromBuild) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addCompileUnits (projectItem.getChild(i), mo, excludeFromBuild);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath file (projectItem.getFile(), getTargetFolder().getChildFile ("app"), RelativePath::buildTargetFolder);
            const ProjectType::Target::Type targetType = getProject().getTargetTypeFromFilePath (projectItem.getFile(), true);

            mo << "    \"" << file.toUnixStyle() << "\"" << newLine;

            if ((! projectItem.shouldBeCompiled()) || (! shouldFileBeCompiledByDefault (file))
                || (getProject().getProjectType().isAudioPlugin()
                    && targetType != ProjectType::Target::SharedCodeTarget
                    && targetType != ProjectType::Target::StandalonePlugIn))
                excludeFromBuild.add (file);
        }
    }

    void addCompileUnits (MemoryOutputStream& mo, Array<RelativePath>& excludeFromBuild) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
            addCompileUnits (getAllGroups().getReference(i), mo, excludeFromBuild);
    }

    //==============================================================================
    StringArray getCmakeDefinitions() const
    {
        const String toolchain = gradleToolchain.get();
        const bool isClang = (toolchain == "clang");

        StringArray cmakeArgs;

        cmakeArgs.add ("\"-DANDROID_TOOLCHAIN=" + toolchain + "\"");
        cmakeArgs.add ("\"-DANDROID_PLATFORM=" + getAppPlatform() + "\"");
        cmakeArgs.add (String ("\"-DANDROID_STL=") + (isClang ? "c++_static" :  "gnustl_static") + "\"");
        cmakeArgs.add ("\"-DANDROID_CPP_FEATURES=exceptions rtti\"");
        cmakeArgs.add ("\"-DANDROID_ARM_MODE=arm\"");
        cmakeArgs.add ("\"-DANDROID_ARM_NEON=TRUE\"");

        return cmakeArgs;
    }

    //==============================================================================
    StringArray getAndroidCompilerFlags() const
    {
        StringArray cFlags;
        cFlags.add ("\"-fsigned-char\"");

        return cFlags;
    }

    StringArray getAndroidCxxCompilerFlags() const
    {
        auto cxxFlags = getAndroidCompilerFlags();

        auto cppStandard = project.getCppStandardValue().toString();

        if (cppStandard == "latest")
            cppStandard = "1z";

        cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

        cxxFlags.add (cppStandard.quoted());

        return cxxFlags;
    }

    StringArray getProjectCompilerFlags() const
    {
        StringArray cFlags (getAndroidCompilerFlags());

        cFlags.addArray (getEscapedFlags (StringArray::fromTokens (getExtraCompilerFlagsString(), true)));
        return cFlags;
    }

    StringArray getProjectCxxCompilerFlags() const
    {
        StringArray cxxFlags (getAndroidCxxCompilerFlags());
        cxxFlags.addArray (getEscapedFlags (StringArray::fromTokens (getExtraCompilerFlagsString(), true)));
        return cxxFlags;
    }

    //==============================================================================
    StringPairArray getAndroidPreprocessorDefs() const
    {
        StringPairArray defines;

        defines.set ("JUCE_ANDROID", "1");
        defines.set ("JUCE_ANDROID_API_VERSION", androidMinimumSDK.get());
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSNAME", getJNIActivityClassName().replaceCharacter ('/', '_'));
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSPATH", "\"" + getJNIActivityClassName() + "\"");
        defines.set ("JUCE_PUSH_NOTIFICATIONS", "1");

        if (androidInAppBillingPermission.get())
            defines.set ("JUCE_IN_APP_PURCHASES", "1");

        if (androidEnableRemoteNotifications.get())
        {
            auto instanceIdClassName = getActivityClassPackage() + ".JuceFirebaseInstanceIdService";
            auto messagingClassName  = getActivityClassPackage() + ".JuceFirebaseMessagingService";
            defines.set ("JUCE_FIREBASE_INSTANCE_ID_SERVICE_CLASSNAME", instanceIdClassName.replaceCharacter ('.', '_'));
            defines.set ("JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME", messagingClassName.replaceCharacter ('.', '_'));
        }

        if (supportsGLv3())
            defines.set ("JUCE_ANDROID_GL_ES_VERSION_3_0", "1");

        return defines;
    }

    StringPairArray getProjectPreprocessorDefs() const
    {
        StringPairArray defines (getAndroidPreprocessorDefs());

        return mergePreprocessorDefs (defines, getAllPreprocessorDefs());
    }

    StringPairArray getConfigPreprocessorDefs (const BuildConfiguration& config) const
    {
        StringPairArray cfgDefines (config.getUniquePreprocessorDefs());

        if (config.isDebug())
        {
            cfgDefines.set ("DEBUG", "1");
            cfgDefines.set ("_DEBUG", "1");
        }
        else
        {
            cfgDefines.set ("NDEBUG", "1");
        }

        return cfgDefines;
    }

    //==============================================================================
    StringArray getAndroidLibraries() const
    {
        StringArray libraries;

        libraries.add ("log");
        libraries.add ("android");
        libraries.add (supportsGLv3() ? "GLESv3" : "GLESv2");
        libraries.add ("EGL");

        return libraries;
    }

    //==============================================================================
    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray paths (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());
        paths = getCleanedStringArray (paths);
        return paths;
    }

    //==============================================================================
    String escapeDirectoryForCmake (const String& path) const
    {
        RelativePath relative =
            RelativePath (path, RelativePath::buildTargetFolder)
                .rebased (getTargetFolder(), getTargetFolder().getChildFile ("app"), RelativePath::buildTargetFolder);

        return relative.toUnixStyle();
    }

    void writeCmakePathLines (MemoryOutputStream& mo, const String& prefix, const String& firstLine, const StringArray& paths,
                              const String& suffix = ")") const
    {
        if (paths.size() > 0)
        {
            mo << prefix << firstLine << newLine;

            for (auto& path : paths)
                mo << prefix << "    \"" << escapeDirectoryForCmake (path) << "\"" << newLine;

            mo << prefix << suffix << newLine << newLine;
        }
    }

    static StringArray getEscapedPreprocessorDefs (const StringPairArray& defs)
    {
        StringArray escapedDefs;

        for (int i = 0; i < defs.size(); ++i)
        {
            String escaped ("\"-D" + defs.getAllKeys()[i]);
            String value = defs.getAllValues()[i];

            if (value.isNotEmpty())
            {
                value = value.replace ("\"", "\\\"");
                if (value.containsChar (L' '))
                    value = "\\\"" + value + "\\\"";

                escaped += ("=" + value);
            }

            escapedDefs.add (escaped + "\"");
        }

        return escapedDefs;
    }

    static StringArray getEscapedFlags (const StringArray& flags)
    {
        StringArray escaped;

        for (auto& flag : flags)
            escaped.add ("\"" + flag + "\"");

        return escaped;
    }

    //==============================================================================
    XmlElement* createManifestXML() const
    {
        XmlElement* manifest = XmlDocument::parse (androidManifestCustomXmlElements.get());

        if (manifest == nullptr)
            manifest = new XmlElement ("manifest");

        setAttributeIfNotPresent (*manifest, "xmlns:android", "http://schemas.android.com/apk/res/android");
        setAttributeIfNotPresent (*manifest, "android:versionCode", androidVersionCode.get());
        setAttributeIfNotPresent (*manifest, "android:versionName",  project.getVersionString());
        setAttributeIfNotPresent (*manifest, "package", getActivityClassPackage());

        if (! isLibrary())
        {
            if (manifest->getChildByName ("supports-screens") == nullptr)
            {
                XmlElement* screens = manifest->createNewChildElement ("supports-screens");
                screens->setAttribute ("android:smallScreens", "true");
                screens->setAttribute ("android:normalScreens", "true");
                screens->setAttribute ("android:largeScreens", "true");
                screens->setAttribute ("android:anyDensity", "true");
            }
        }

        auto* sdk = getOrCreateChildWithName (*manifest, "uses-sdk");
        setAttributeIfNotPresent (*sdk, "android:minSdkVersion", androidMinimumSDK.get());
        setAttributeIfNotPresent (*sdk, "android:targetSdkVersion", androidMinimumSDK.get());

        {
            StringArray permissions (getPermissionsRequired());

            forEachXmlChildElementWithTagName (*manifest, child, "uses-permission")
            {
                permissions.removeString (child->getStringAttribute ("android:name"), false);
            }

            for (int i = permissions.size(); --i >= 0;)
                manifest->createNewChildElement ("uses-permission")->setAttribute ("android:name", permissions[i]);
        }

        if (project.getModules().isModuleEnabled ("juce_opengl"))
        {
            XmlElement* glVersion = nullptr;

            forEachXmlChildElementWithTagName (*manifest, child, "uses-feature")
            {
                if (child->getStringAttribute ("android:glEsVersion").isNotEmpty())
                {
                    glVersion = child;
                    break;
                }
            }

            if (glVersion == nullptr)
                glVersion = manifest->createNewChildElement ("uses-feature");

            setAttributeIfNotPresent (*glVersion, "android:glEsVersion", (androidMinimumSDK.get().getIntValue() >= 18 ? "0x00030000" : "0x00020000"));
            setAttributeIfNotPresent (*glVersion, "android:required", "true");
        }

        if (! isLibrary())
        {
            auto* app = getOrCreateChildWithName (*manifest, "application");
            setAttributeIfNotPresent (*app, "android:label", "@string/app_name");

            if (androidTheme.get().isNotEmpty())
                setAttributeIfNotPresent (*app, "android:theme", androidTheme.get());

            if (! app->hasAttribute ("android:icon"))
            {
                ScopedPointer<Drawable> bigIcon (getBigIcon()), smallIcon (getSmallIcon());

                if (bigIcon != nullptr || smallIcon != nullptr)
                    app->setAttribute ("android:icon", "@drawable/icon");
            }

            if (androidMinimumSDK.get().getIntValue() >= 11)
                app->setAttribute ("android:hardwareAccelerated", "false"); // (using the 2D acceleration slows down openGL)
            else
                app->removeAttribute ("android:hardwareAccelerated");

            auto* act = getOrCreateChildWithName (*app, "activity");

            setAttributeIfNotPresent (*act, "android:name", getActivitySubClassName());
            setAttributeIfNotPresent (*act, "android:label", "@string/app_name");

            if (! act->hasAttribute ("android:configChanges"))
            {
                String configChanges ("keyboardHidden|orientation");
                if (androidMinimumSDK.get().getIntValue() >= 13)
                    configChanges += "|screenSize";

                act->setAttribute ("android:configChanges", configChanges);
            }
            else
            {
                auto configChanges = act->getStringAttribute ("android:configChanges");

                if (androidMinimumSDK.get().getIntValue() < 13 && configChanges.contains ("screenSize"))
                {
                    configChanges = configChanges.replace ("|screenSize", "")
                                                 .replace ("screenSize|", "")
                                                 .replace ("screenSize", "");

                    act->setAttribute ("android:configChanges", configChanges);
                }
            }

            setAttributeIfNotPresent (*act, "android:screenOrientation", androidScreenOrientation.get());
            setAttributeIfNotPresent (*act, "android:launchMode", "singleTask");

            auto* intent = getOrCreateChildWithName (*act, "intent-filter");

            auto* action = getOrCreateChildWithName (*intent, "action");
            setAttributeIfNotPresent (*action, "android:name", "android.intent.action.MAIN");

            auto* category = getOrCreateChildWithName (*intent, "category");
            setAttributeIfNotPresent (*category, "android:name", "android.intent.category.LAUNCHER");

            if (androidEnableRemoteNotifications.get())
            {
                auto* service = app->createNewChildElement ("service");
                service->setAttribute ("android:name", ".JuceFirebaseMessagingService");
                auto* intentFilter = service->createNewChildElement ("intent-filter");
                intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.MESSAGING_EVENT");

                service = app->createNewChildElement ("service");
                service->setAttribute ("android:name", ".JuceFirebaseInstanceIdService");
                intentFilter = service->createNewChildElement ("intent-filter");
                intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.INSTANCE_ID_EVENT");

                auto* metaData = app->createNewChildElement ("meta-data");
                metaData->setAttribute ("android:name", "firebase_analytics_collection_deactivated");
                metaData->setAttribute ("android:value", "true");
            }
        }

        return manifest;
    }

    static XmlElement* getOrCreateChildWithName (XmlElement& element, const String& name)
    {
        auto* child = element.getChildByName (name);

        if (child == nullptr)
            child = element.createNewChildElement (name);

        return child;
    }

    static void setAttributeIfNotPresent (XmlElement& element, const Identifier& attribute, const String& value)
    {
        if (! element.hasAttribute (attribute.toString()))
            element.setAttribute (attribute, value);
    }

    StringArray getPermissionsRequired() const
    {
        StringArray s;
        s.addTokens (androidOtherPermissions.get(), ", ", "");

        if (androidInternetNeeded.get())
            s.add ("android.permission.INTERNET");

        if (androidMicNeeded.get())
            s.add ("android.permission.RECORD_AUDIO");

        if (androidBluetoothNeeded.get())
        {
            s.add ("android.permission.BLUETOOTH");
            s.add ("android.permission.BLUETOOTH_ADMIN");
            s.add ("android.permission.ACCESS_COARSE_LOCATION");
        }

        if (androidExternalReadPermission.get())
            s.add ("android.permission.READ_EXTERNAL_STORAGE");

        if (androidExternalWritePermission.get())
            s.add ("android.permission.WRITE_EXTERNAL_STORAGE");

        if (androidInAppBillingPermission.get())
            s.add ("com.android.vending.BILLING");

        if (androidVibratePermission.get())
            s.add ("android.permission.VIBRATE");

        return getCleanedStringArray (s);
    }

    //==============================================================================
    bool isLibrary() const
    {
        return getProject().getProjectType().isDynamicLibrary()
            || getProject().getProjectType().isStaticLibrary();
    }

    static String toGradleList (const StringArray& array)
    {
        StringArray escapedArray;

        for (auto element : array)
            escapedArray.add ("\"" + element.replace ("\\", "\\\\").replace ("\"", "\\\"") + "\"");

        return escapedArray.joinIntoString (", ");
    }

    bool supportsGLv3() const
    {
        return (androidMinimumSDK.get().getIntValue() >= 18);
    }

    //==============================================================================
    Value sdkPath, ndkPath;
    const File AndroidExecutable;

    JUCE_DECLARE_NON_COPYABLE (AndroidProjectExporter)
};

ProjectExporter* createAndroidExporter (Project& p, const ValueTree& t)
{
    return new AndroidProjectExporter (p, t);
}

ProjectExporter* createAndroidExporterForSetting (Project& p, const ValueTree& t)
{
    return AndroidProjectExporter::createForSettings (p, t);
}
