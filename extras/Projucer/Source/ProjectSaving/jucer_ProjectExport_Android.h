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
    ValueWithDefault androidJavaLibs, androidRepositories, androidDependencies, androidScreenOrientation, androidActivityClass,
                     androidActivitySubClassName, androidActivityBaseClassName, androidManifestCustomXmlElements, androidVersionCode,
                     androidMinimumSDK, androidTheme, androidSharedLibraries, androidStaticLibraries, androidExtraAssetsFolder,
                     androidOboeRepositoryPath, androidInternetNeeded, androidMicNeeded, androidCameraNeeded, androidBluetoothNeeded, androidExternalReadPermission,
                     androidExternalWritePermission, androidInAppBillingPermission, androidVibratePermission,androidOtherPermissions,
                     androidEnableRemoteNotifications, androidRemoteNotificationsConfigFile, androidEnableContentSharing, androidKeyStore,
                     androidKeyStorePass, androidKeyAlias, androidKeyAliasPass, gradleVersion, gradleToolchain, androidPluginVersion, buildToolsVersion;

    //==============================================================================
    AndroidProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          androidJavaLibs                      (settings, Ids::androidJavaLibs,                      getUndoManager()),
          androidRepositories                  (settings, Ids::androidRepositories,                  getUndoManager()),
          androidDependencies                  (settings, Ids::androidDependencies,                  getUndoManager()),
          androidScreenOrientation             (settings, Ids::androidScreenOrientation,             getUndoManager(), "unspecified"),
          androidActivityClass                 (settings, Ids::androidActivityClass,                 getUndoManager(), createDefaultClassName()),
          androidActivitySubClassName          (settings, Ids::androidActivitySubClassName,          getUndoManager()),
          androidActivityBaseClassName         (settings, Ids::androidActivityBaseClassName,         getUndoManager(), "Activity"),
          androidManifestCustomXmlElements     (settings, Ids::androidManifestCustomXmlElements,     getUndoManager()),
          androidVersionCode                   (settings, Ids::androidVersionCode,                   getUndoManager(), "1"),
          androidMinimumSDK                    (settings, Ids::androidMinimumSDK,                    getUndoManager(), "10"),
          androidTheme                         (settings, Ids::androidTheme,                         getUndoManager()),
          androidSharedLibraries               (settings, Ids::androidSharedLibraries,               getUndoManager()),
          androidStaticLibraries               (settings, Ids::androidStaticLibraries,               getUndoManager()),
          androidExtraAssetsFolder             (settings, Ids::androidExtraAssetsFolder,             getUndoManager()),
          androidOboeRepositoryPath            (settings, Ids::androidOboeRepositoryPath,            getUndoManager()),
          androidInternetNeeded                (settings, Ids::androidInternetNeeded,                getUndoManager(), true),
          androidMicNeeded                     (settings, Ids::microphonePermissionNeeded,           getUndoManager(), false),
          androidCameraNeeded                  (settings, Ids::cameraPermissionNeeded,               getUndoManager(), false),
          androidBluetoothNeeded               (settings, Ids::androidBluetoothNeeded,               getUndoManager(), true),
          androidExternalReadPermission        (settings, Ids::androidExternalReadNeeded,            getUndoManager(), true),
          androidExternalWritePermission       (settings, Ids::androidExternalWriteNeeded,           getUndoManager(), true),
          androidInAppBillingPermission        (settings, Ids::androidInAppBilling,                  getUndoManager(), false),
          androidVibratePermission             (settings, Ids::androidVibratePermissionNeeded,       getUndoManager(), false),
          androidOtherPermissions              (settings, Ids::androidOtherPermissions,              getUndoManager()),
          androidEnableRemoteNotifications     (settings, Ids::androidEnableRemoteNotifications,     getUndoManager(), false),
          androidRemoteNotificationsConfigFile (settings, Ids::androidRemoteNotificationsConfigFile, getUndoManager()),
          androidEnableContentSharing          (settings, Ids::androidEnableContentSharing,          getUndoManager(), false),
          androidKeyStore                      (settings, Ids::androidKeyStore,                      getUndoManager(), "${user.home}/.android/debug.keystore"),
          androidKeyStorePass                  (settings, Ids::androidKeyStorePass,                  getUndoManager(), "android"),
          androidKeyAlias                      (settings, Ids::androidKeyAlias,                      getUndoManager(), "androiddebugkey"),
          androidKeyAliasPass                  (settings, Ids::androidKeyAliasPass,                  getUndoManager(), "android"),
          gradleVersion                        (settings, Ids::gradleVersion,                        getUndoManager(), "4.4"),
          gradleToolchain                      (settings, Ids::gradleToolchain,                      getUndoManager(), "clang"),
          androidPluginVersion                 (settings, Ids::androidPluginVersion,                 getUndoManager(), "3.1.3"),
          buildToolsVersion                    (settings, Ids::buildToolsVersion,                    getUndoManager(), "28.0.0"),
          AndroidExecutable (findAndroidExecutable())
    {
        name = getName();

        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderForExporter (getValueTreeTypeName()));
    }

    //==============================================================================
    void createToolchainExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (gradleVersion, "gradle version", 32, false),
                   "The version of gradle that is used to build this app (4.4 is fine for JUCE)");

        props.add (new TextPropertyComponent (androidPluginVersion, "android plug-in version", 32, false),
                   "The version of the android build plugin for gradle that is used to build this app");

        props.add (new ChoicePropertyComponent (gradleToolchain, "NDK Toolchain",
                                                { "clang", "gcc" },
                                                { "clang", "gcc" }),
                   "The toolchain that gradle should invoke for NDK compilation (variable model.android.ndk.tooclhain in app/build.gradle)");

        props.add (new TextPropertyComponent (buildToolsVersion, "Android build tools version", 32, false),
                   "The Android build tools version that should use to build this app");
    }

    void createLibraryModuleExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidStaticLibraries, "Import static library modules", 8192, true),
                   "Comma or whitespace delimited list of static libraries (.a) defined in NDK_MODULE_PATH.");

        props.add (new TextPropertyComponent (androidSharedLibraries, "Import shared library modules", 8192, true),
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

        auto targetFolder = getTargetFolder();

        // we have to surround the path with extra quotes, otherwise Android Studio
        // will choke if there are any space characters in the path.
        return AndroidExecutable.startAsProcess ("\"" + targetFolder.getFullPathName() + "\"");
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        auto targetFolder = getTargetFolder();
        auto appFolder = targetFolder.getChildFile (isLibrary() ? "lib" : "app");

        removeOldFiles (targetFolder);

        if (! isLibrary())
            copyJavaFiles (modules);

        copyExtraResourceFiles();

        writeFile (targetFolder, "settings.gradle",                          isLibrary() ? "include ':lib'" : "include ':app'");
        writeFile (targetFolder, "build.gradle",                             getProjectBuildGradleFileContent());
        writeFile (appFolder,    "build.gradle",                             getAppBuildGradleFileContent());
        writeFile (targetFolder, "local.properties",                         getLocalPropertiesFileContent());
        writeFile (targetFolder, "gradle/wrapper/gradle-wrapper.properties", getGradleWrapperPropertiesFileContent());

        writeBinaryFile (targetFolder, "gradle/wrapper/LICENSE-for-gradlewrapper.txt", BinaryData::LICENSE,           BinaryData::LICENSESize);
        writeBinaryFile (targetFolder, "gradle/wrapper/gradle-wrapper.jar",            BinaryData::gradlewrapper_jar, BinaryData::gradlewrapper_jarSize);
        writeBinaryFile (targetFolder, "gradlew",                                      BinaryData::gradlew,           BinaryData::gradlewSize);
        writeBinaryFile (targetFolder, "gradlew.bat",                                  BinaryData::gradlew_bat,       BinaryData::gradlew_batSize);

        targetFolder.getChildFile ("gradlew").setExecutePermission (true);

        writeAndroidManifest (appFolder);

        if (! isLibrary())
        {
            writeStringsXML      (targetFolder);
            writeAppIcons        (targetFolder);
        }

        writeCmakeFile (appFolder.getChildFile ("CMakeLists.txt"));

        auto androidExtraAssetsFolderValue = androidExtraAssetsFolder.get().toString();

        if (androidExtraAssetsFolderValue.isNotEmpty())
        {
            auto extraAssets = getProject().getFile().getParentDirectory().getChildFile (androidExtraAssetsFolderValue);

            if (extraAssets.exists() && extraAssets.isDirectory())
            {
                auto assetsFolder = appFolder.getChildFile ("src/main/assets");

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
        File defaultInstallation ("C:\\Program Files\\Android\\Android Studio\\bin");

        if (defaultInstallation.exists())
        {
            {
                auto studio64 = defaultInstallation.getChildFile ("studio64.exe");

                if (studio64.existsAsFile())
                    return studio64;
            }

            {
                auto studio = defaultInstallation.getChildFile ("studio.exe");

                if (studio.existsAsFile())
                    return studio;
            }
        }
      #elif JUCE_MAC
       File defaultInstallation ("/Applications/Android Studio.app");

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
            : BuildConfiguration (p, settings, e),
              androidArchitectures               (config, Ids::androidArchitectures,               getUndoManager(), isDebug() ? "armeabi-v7a x86" : ""),
              androidBuildConfigRemoteNotifsConfigFile (config, Ids::androidBuildConfigRemoteNotifsConfigFile, getUndoManager()),
              androidAdditionalXmlValueResources (config, Ids::androidAdditionalXmlValueResources, getUndoManager()),
              androidAdditionalDrawableResources (config, Ids::androidAdditionalDrawableResources, getUndoManager()),
              androidAdditionalRawValueResources (config, Ids::androidAdditionalRawValueResources, getUndoManager()),
              androidCustomStringXmlElements     (config, Ids::androidCustomStringXmlElements,     getUndoManager())
        {
            linkTimeOptimisationValue.setDefault (false);
            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        String getArchitectures() const               { return androidArchitectures.get().toString(); }
        String getRemoteNotifsConfigFile() const      { return androidBuildConfigRemoteNotifsConfigFile.get().toString(); }
        String getAdditionalXmlResources() const      { return androidAdditionalXmlValueResources.get().toString(); }
        String getAdditionalDrawableResources() const { return androidAdditionalDrawableResources.get().toString(); }
        String getAdditionalRawResources() const      { return androidAdditionalRawValueResources.get().toString();}
        String getCustomStringsXml() const            { return androidCustomStringXmlElements.get().toString(); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            props.add (new TextPropertyComponent (androidArchitectures, "Architectures", 256, false),
                       "A list of the ARM architectures to build (for a fat binary). Leave empty to build for all possible android archiftectures.");

            props.add (new TextPropertyComponent (androidBuildConfigRemoteNotifsConfigFile.getPropertyAsValue(), "Remote Notifications Config File", 2048, false),
                       "Path to google-services.json file. This will be the file provided by Firebase when creating a new app in Firebase console. "
                       "This will override the setting from the main Android exporter node.");

            props.add (new TextPropertyComponent (androidAdditionalXmlValueResources, "Extra Android XML Value Resources", 8192, true),
                       "Paths to additional \"value resource\" files in XML format that should be included in the app (one per line). "
                       "If you have additional XML resources that should be treated as value resources, add them here.");

            props.add (new TextPropertyComponent (androidAdditionalDrawableResources, "Extra Android Drawable Resources", 8192, true),
                       "Paths to additional \"drawable resource\" directories that should be included in the app (one per line). "
                       "They will be added to \"res\" directory of Android project. "
                       "Each path should point to a directory named \"drawable\" or \"drawable-<size>\" where <size> should be "
                       "something like \"hdpi\", \"ldpi\", \"xxxhdpi\" etc, for instance \"drawable-xhdpi\". "
                       "Refer to Android Studio documentation for available sizes.");

            props.add (new TextPropertyComponent (androidAdditionalRawValueResources, "Extra Android Raw Resources", 8192, true),
                       "Paths to additional \"raw resource\" files that should be included in the app (one per line). "
                       "Resource file names must contain only lowercase a-z, 0-9 or underscore.");

            props.add (new TextPropertyComponent (androidCustomStringXmlElements, "Custom string resources", 8192, true),
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

        ValueWithDefault androidArchitectures, androidBuildConfigRemoteNotifsConfigFile,
                         androidAdditionalXmlValueResources, androidAdditionalDrawableResources,
                         androidAdditionalRawValueResources, androidCustomStringXmlElements;
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

        if (project.getConfigFlag ("JUCE_USE_ANDROID_OBOE").get())
        {
            String oboePath (androidOboeRepositoryPath.get().toString().quoted());

            mo << "SET(OBOE_DIR " << oboePath << ")" << newLine << newLine;
            mo << "add_subdirectory (${OBOE_DIR} ./oboe)" << newLine << newLine;
        }

        String cpufeaturesPath ("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c");
        mo << "add_library(\"cpufeatures\" STATIC \"" << cpufeaturesPath << "\")" << newLine
           << "set_source_files_properties(\"" << cpufeaturesPath << "\" PROPERTIES COMPILE_FLAGS \"-Wno-sign-conversion -Wno-gnu-statement-expression\")" << newLine << newLine;

        {
            auto projectDefines = getEscapedPreprocessorDefs (getProjectPreprocessorDefs());
            if (projectDefines.size() > 0)
                mo << "add_definitions(" << projectDefines.joinIntoString (" ") << ")" << newLine << newLine;
        }

        {
            mo << "include_directories( AFTER" << newLine;

            for (auto& path : extraSearchPaths)
                mo << "    \"" << escapeDirectoryForCmake (path) << "\"" << newLine;

            mo << "    \"${ANDROID_NDK}/sources/android/cpufeatures\"" << newLine;

            if (project.getConfigFlag ("JUCE_USE_ANDROID_OBOE").get())
                mo << "    \"${OBOE_DIR}/include\"" << newLine;

            mo << ")" << newLine << newLine;
        }

        auto cfgExtraLinkerFlags = getExtraLinkerFlagsString();
        if (cfgExtraLinkerFlags.isNotEmpty())
        {
            mo << "SET( JUCE_LDFLAGS \"" << cfgExtraLinkerFlags.replace ("\"", "\\\"") << "\")" << newLine;
            mo << "SET( CMAKE_SHARED_LINKER_FLAGS  \"${CMAKE_EXE_LINKER_FLAGS} ${JUCE_LDFLAGS}\")" << newLine << newLine;
        }

        mo << "enable_language(ASM)" << newLine << newLine;

        auto userLibraries = StringArray::fromTokens (getExternalLibrariesString(), ";", "");
        userLibraries.addArray (androidLibs);

        if (getNumConfigurations() > 0)
        {
            bool first = true;

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto& cfg            = dynamic_cast<const AndroidBuildConfiguration&> (*config);
                auto libSearchPaths  = cfg.getLibrarySearchPaths();
                auto cfgDefines      = getConfigPreprocessorDefs (cfg);
                auto cfgHeaderPaths  = cfg.getHeaderSearchPaths();
                auto cfgLibraryPaths = cfg.getLibrarySearchPaths();

                if (! isLibrary() && libSearchPaths.size() == 0 && cfgDefines.size() == 0
                       && cfgHeaderPaths.size() == 0 && cfgLibraryPaths.size() == 0)
                    continue;

                mo << (first ? "IF" : "ELSEIF") << "(JUCE_BUILD_CONFIGURATION MATCHES \"" << cfg.getProductFlavourCMakeIdentifier() <<"\")" << newLine;

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

                if (cfg.isLinkTimeOptimisationEnabled())
                {
                    // There's no MIPS support for LTO
                    String mipsCondition ("NOT (ANDROID_ABI STREQUAL \"mips\" OR ANDROID_ABI STREQUAL \"mips64\")");
                    mo << "    if(" << mipsCondition << ")" << newLine;
                    StringArray cmakeVariables ("CMAKE_C_FLAGS", "CMAKE_CXX_FLAGS", "CMAKE_EXE_LINKER_FLAGS");

                    for (auto& variable : cmakeVariables)
                    {
                        auto configVariable = variable + "_" + cfg.getProductFlavourCMakeIdentifier();
                        mo << "        SET(" << configVariable << " \"${" << configVariable << "} -flto\")" << newLine;
                    }

                    mo << "    ENDIF(" << mipsCondition << ")" << newLine;
                }

                first = false;
            }

            if (! first)
            {
                ProjectExporter::BuildConfiguration::Ptr config (getConfiguration(0));

                if (config)
                {
                    if (auto* cfg = dynamic_cast<const AndroidBuildConfiguration*> (config.get()))
                    {
                        mo << "ELSE(JUCE_BUILD_CONFIGURATION MATCHES \"" << cfg->getProductFlavourCMakeIdentifier() <<"\")" << newLine;
                        mo << "    MESSAGE( FATAL_ERROR \"No matching build-configuration found.\" )" << newLine;
                        mo << "ENDIF(JUCE_BUILD_CONFIGURATION MATCHES \"" << cfg->getProductFlavourCMakeIdentifier() <<"\")" << newLine << newLine;
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

        auto libraries = getAndroidLibraries();
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

        if (project.getConfigFlag ("JUCE_USE_ANDROID_OBOE").get())
            mo << "    \"oboe\"" << newLine;

        mo << ")" << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    //==============================================================================
    String getProjectBuildGradleFileContent() const
    {
        MemoryOutputStream mo;

        mo << "buildscript {"                                                                              << newLine;
        mo << "   repositories {"                                                                          << newLine;
        mo << "       google()"                                                                            << newLine;
        mo << "       jcenter()"                                                                           << newLine;
        mo << "   }"                                                                                       << newLine;
        mo << "   dependencies {"                                                                          << newLine;
        mo << "       classpath 'com.android.tools.build:gradle:" << androidPluginVersion.get().toString() << "'" << newLine;

        if (androidEnableRemoteNotifications.get())
            mo << "       classpath 'com.google.gms:google-services:3.1.0'" << newLine;

        mo << "   }"                                                                                   << newLine;
        mo << "}"                                                                                      << newLine;
        mo << ""                                                                                       << newLine;
        mo << "allprojects {"                                                                          << newLine;
        mo << "   repositories {"                                                                      << newLine;
        mo << "       google()"                                                                        << newLine;
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

        mo << "android {"                                                                    << newLine;
        mo << "    compileSdkVersion " << static_cast<int> (androidMinimumSDK.get())         << newLine;
        mo << "    buildToolsVersion \"" << buildToolsVersion.get().toString() << "\""       << newLine;
        mo << "    externalNativeBuild {"                                                    << newLine;
        mo << "        cmake {"                                                              << newLine;
        mo << "            path \"CMakeLists.txt\""                                          << newLine;
        mo << "        }"                                                                    << newLine;
        mo << "    }"                                                                        << newLine;

        mo << getAndroidSigningConfig()                                                      << newLine;
        mo << getAndroidDefaultConfig()                                                      << newLine;
        mo << getAndroidBuildTypes()                                                         << newLine;
        mo << getAndroidProductFlavours()                                                    << newLine;
        mo << getAndroidVariantFilter()                                                      << newLine;

        mo << getAndroidRepositories()                                                       << newLine;
        mo << getAndroidDependencies()                                                       << newLine;
        mo << getApplyPlugins()                                                              << newLine;

        mo << "}"                                                                            << newLine << newLine;

        return mo.toString();
    }

    String getAndroidProductFlavours() const
    {
        MemoryOutputStream mo;

        mo << "    flavorDimensions \"default\"" << newLine;
        mo << "    productFlavors {" << newLine;

        for (ConstConfigIterator config (*this); config.next();)
        {
            auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

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

            mo << "                    arguments \"-DJUCE_BUILD_CONFIGURATION=" << cfg.getProductFlavourCMakeIdentifier() << "\""
                                           << ", \"-DCMAKE_CXX_FLAGS_" << (cfg.isDebug() ? "DEBUG" : "RELEASE")
                                           << "=-O" << cfg.getGCCOptimisationFlag() << "\""
                                           << ", \"-DCMAKE_C_FLAGS_"   << (cfg.isDebug() ? "DEBUG" : "RELEASE")
                                           << "=-O" << cfg.getGCCOptimisationFlag() << "\"" << newLine;

            mo << "                }"                   << newLine;
            mo << "            }"                       << newLine << newLine;
            mo << "            dimension \"default\""   << newLine;
            mo << "       }"                            << newLine;
        }

        mo << "    }" << newLine;

        return mo.toString();
    }

    String getAndroidSigningConfig() const
    {
        MemoryOutputStream mo;

        auto keyStoreFilePath = androidKeyStore.get().toString().replace ("${user.home}", "${System.properties['user.home']}")
                                                                .replace ("/", "${File.separator}");

        mo << "    signingConfigs {"                                                         << newLine;
        mo << "        juceSigning {"                                                        << newLine;
        mo << "            storeFile     file(\"" << keyStoreFilePath << "\")"               << newLine;
        mo << "            storePassword \"" << androidKeyStorePass.get().toString() << "\"" << newLine;
        mo << "            keyAlias      \"" << androidKeyAlias.get().toString() << "\""     << newLine;
        mo << "            keyPassword   \"" << androidKeyAliasPass.get().toString() << "\"" << newLine;
        mo << "            storeType     \"jks\""                                            << newLine;
        mo << "        }"                                                                    << newLine;
        mo << "    }"                                                                        << newLine;

        return mo.toString();
    }

    String getAndroidDefaultConfig() const
    {
        auto bundleIdentifier  = project.getBundleIdentifierString().toLowerCase();
        auto cmakeDefs         = getCmakeDefinitions();
        auto cFlags            = getProjectCompilerFlags();
        auto cxxFlags          = getProjectCxxCompilerFlags();
        auto minSdkVersion     = static_cast<int> (androidMinimumSDK.get());

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
        auto numConfigs = getNumConfigurations();
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
            auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

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

        auto repositories = StringArray::fromLines (androidRepositories.get().toString());

        mo << "repositories {"                << newLine;

        for (auto& r : repositories)
            mo << "    " << r << newLine;

        mo << "}"                             << newLine;

        return mo.toString();
    }

    String getAndroidDependencies() const
    {
        MemoryOutputStream mo;
        mo << "dependencies {" << newLine;

        for (auto& d : StringArray::fromLines (androidDependencies.get().toString()))
            mo << "    " << d << newLine;

        for (auto& d : StringArray::fromLines (androidJavaLibs.get().toString()))
            mo << "    implementation files('libs/" << File (d).getFileName() << "')" << newLine;

        if (androidEnableRemoteNotifications.get())
        {
            mo << "    'com.google.firebase:firebase-core:11.4.0'" << newLine;
            mo << "    compile 'com.google.firebase:firebase-messaging:11.4.0'" << newLine;
        }

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
              << gradleVersion.get().toString() << "-all.zip";

        return props;
    }

    //==============================================================================
    void createBaseExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidJavaLibs, "Java libraries to include", 32768, true),
                   "Java libs (JAR files) (one per line). These will be copied to app/libs folder and \"implementation files\" "
                   "dependency will be automatically added to module \"dependencies\" section for each library, so do "
                   "not add the dependency yourself.");

        props.add (new TextPropertyComponent (androidRepositories, "Module repositories", 32768, true),
                   "Module repositories (one per line). These will be added to module-level gradle file repositories section. ");

        props.add (new TextPropertyComponent (androidDependencies, "Module dependencies", 32768, true),
                   "Module dependencies (one per line). These will be added to module-level gradle file \"dependencies\" section. "
                   "If adding any java libs in \"Java libraries to include\" setting, do not add them here as "
                   "they will be added automatically.");

        props.add (new ChoicePropertyComponent (androidScreenOrientation, "Screen orientation",
                                                { "Portrait and Landscape", "Portrait", "Landscape" },
                                                { "unspecified",            "portrait", "landscape" }),
                   "The screen orientations that this app should support");

        props.add (new TextPropertyComponent (androidActivityClass, "Android Activity class name", 256, false),
                   "The full java class name to use for the app's Activity class.");

        props.add (new TextPropertyComponent (androidActivitySubClassName, "Android Activity sub-class name", 256, false),
                   "If not empty, specifies the Android Activity class name stored in the app's manifest. "
                   "Use this if you would like to use your own Android Activity sub-class.");

        props.add (new TextPropertyComponent (androidActivityBaseClassName, "Android Activity base class", 256, false),
                   "If not empty, specifies the base class to use for your activity. If custom base class is "
                   "specified, that base class should be a sub-class of android.app.Activity. When empty, Activity "
                   "(android.app.Activity) will be used as the base class. "
                   "Use this if you would like to use your own Android Activity base class.");

        props.add (new TextPropertyComponent (androidVersionCode, "Android Version Code", 32, false),
                   "An integer value that represents the version of the application code, relative to other versions.");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), sdkPath, "Android SDK Path"),
                   "The path to the Android SDK folder on the target build machine");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), ndkPath, "Android NDK Path"),
                   "The path to the Android NDK folder on the target build machine");

        props.add (new TextPropertyComponent (androidMinimumSDK, "Minimum SDK version", 32, false),
                   "The number of the minimum version of the Android SDK that the app requires");

        props.add (new TextPropertyComponent (androidExtraAssetsFolder, "Extra Android Assets", 256, false),
                   "A path to a folder (relative to the project folder) which contains extra android assets.");
    }

    //==============================================================================
    void createManifestExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidOboeRepositoryPath, "Oboe repository path", 2048, false),
                   "Path to the root of Oboe repository. Make sure to point Oboe repository to "
                   "commit with SHA 44c6b6ea9c8fa9b5b74cbd60f355068b57b50b37 before building.");

        props.add (new ChoicePropertyComponent (androidInternetNeeded, "Internet Access"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidMicNeeded, "Audio Input Required"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidCameraNeeded, "Camera Required"),
                   "If enabled, this will set the android.permission.CAMERA flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidBluetoothNeeded, "Bluetooth permissions Required"),
                   "If enabled, this will set the android.permission.BLUETOOTH and  android.permission.BLUETOOTH_ADMIN flag in the manifest. This is required for Bluetooth MIDI on Android.");

        props.add (new ChoicePropertyComponent (androidExternalReadPermission, "Read from external storage"),
                   "If enabled, this will set the android.permission.READ_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidExternalWritePermission, "Write to external storage"),
                   "If enabled, this will set the android.permission.WRITE_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidInAppBillingPermission, "In-App Billing"),
                   "If enabled, this will set the com.android.vending.BILLING flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidVibratePermission, "Vibrate"),
                   "If enabled, this will set the android.permission.VIBRATE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidEnableContentSharing, "Content Sharing"),
                   "If enabled, your app will be able to share content with other apps.");

        props.add (new TextPropertyComponent (androidOtherPermissions, "Custom permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");

        props.add (new ChoicePropertyComponent (androidEnableRemoteNotifications, "Remote Notifications"),
                   "Enable to be able to send remote notifications to devices running your app (min API level 14). Provide Remote Notifications Config File, "
                   "configure your app in Firebase Console and ensure you have the latest Google Repository in Android Studio's SDK Manager.");

        props.add (new TextPropertyComponent (androidRemoteNotificationsConfigFile.getPropertyAsValue(), "Remote Notifications Config File", 2048, false),
                   "Path to google-services.json file. This will be the file provided by Firebase when creating a new app in Firebase console.");

        props.add (new TextPropertyComponent (androidManifestCustomXmlElements, "Custom manifest XML content", 8192, true),
                   "You can specify custom AndroidManifest.xml content overriding the default one generated by Projucer. "
                   "Projucer will automatically create any missing and required XML elements and attributes "
                   "and merge them into your custom content.");
    }

    //==============================================================================
    void createCodeSigningExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidKeyStore, "Key Signing: key.store", 2048, false),
                   "The key.store value, used when signing the release package.");

        props.add (new TextPropertyComponent (androidKeyStorePass, "Key Signing: key.store.password", 2048, false),
                   "The key.store password, used when signing the release package.");

        props.add (new TextPropertyComponent (androidKeyAlias, "Key Signing: key.alias", 2048, false),
                   "The key.alias value, used when signing the release package.");

        props.add (new TextPropertyComponent (androidKeyAliasPass, "Key Signing: key.alias.password", 2048, false),
                   "The key.alias password, used when signing the release package.");
    }

    //==============================================================================
    void createOtherExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidTheme, "Android Theme", 256, false),
                   "E.g. @android:style/Theme.NoTitleBar or leave blank for default");
    }

    //==============================================================================
    String createDefaultClassName() const
    {
        auto s = project.getBundleIdentifierString().toLowerCase();

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

        return s + CodeHelpers::makeValidIdentifier (project.getProjectFilenameRootString(), false, true, false);
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
            auto libTarget = targetFolder.getChildFile ("app/libs");
            libTarget.createDirectory();

            copyActivityJavaFiles (javaSourceFolder, javaTarget, package);
            copyServicesJavaFiles (javaSourceFolder, javaTarget, package);
            copyProviderJavaFile  (javaSourceFolder, javaTarget, package);
            copyAdditionalJavaFiles (javaSourceFolder, javaInAppBillingTarget);
            copyAdditionalJavaLibs (libTarget);
        }
    }

    void copyActivityJavaFiles (const File& javaSourceFolder, const File& targetFolder, const String& package) const
    {
        if (androidActivityClass.get().toString().contains ("_"))
            throw SaveError ("Your Android activity class name or path may not contain any underscores! Try a project name without underscores.");

        auto className = getActivityName();

        if (className.isEmpty())
            throw SaveError ("Invalid Android Activity class name: " + androidActivityClass.get().toString());

        createDirectoryOrThrow (targetFolder);

        auto activityCode = getActivityCode (javaSourceFolder, className, package);

        auto javaDestFile = targetFolder.getChildFile (className + ".java");
        overwriteFileIfDifferentOrThrow (javaDestFile, activityCode);
    }

    String getActivityCode (const File& javaSourceFolder, const String& className, const String& package) const
    {
        auto runtimePermissionsCode = getRuntimePermissionsCode (javaSourceFolder, className);
        auto midiCode = getMidiCode (javaSourceFolder, className);
        auto webViewCode = getWebViewCode (javaSourceFolder);
        auto cameraCode = getCameraCode (javaSourceFolder);
        auto videoCode = getVideoCode (javaSourceFolder);

        auto javaSourceFile = javaSourceFolder.getChildFile ("JuceAppActivity.java");
        auto javaSourceLines = StringArray::fromLines (javaSourceFile.loadFileAsString());

        {
            MemoryOutputStream newFile;

            for (auto& line : javaSourceLines)
            {
                if (line.contains ("$$JuceAndroidMidiImports$$"))
                    newFile << midiCode.imports;
                else if (line.contains ("$$JuceAndroidMidiCode$$"))
                    newFile << midiCode.main;
                else if (line.contains ("$$JuceAndroidRuntimePermissionsCode$$"))
                    newFile << runtimePermissionsCode;
                else if (line.contains ("$$JuceAndroidWebViewImports$$"))
                    newFile << webViewCode.imports;
                else if (line.contains ("$$JuceAndroidWebViewNativeCode$$"))
                    newFile << webViewCode.native;
                else if (line.contains ("$$JuceAndroidWebViewCode$$"))
                    newFile << webViewCode.main;
                else if (line.contains ("$$JuceAndroidCameraImports$$"))
                    newFile << cameraCode.imports;
                else if (line.contains ("$$JuceAndroidCameraCode$$"))
                    newFile << cameraCode.main;
                else if (line.contains ("$$JuceAndroidVideoImports$$"))
                    newFile << videoCode.imports;
                else if (line.contains ("$$JuceAndroidVideoCode$$"))
                    newFile << videoCode.main;
                else
                    newFile << line.replace ("$$JuceAppActivityBaseClass$$", androidActivityBaseClassName.get().toString())
                                   .replace ("JuceAppActivity", className)
                                   .replace ("package com.juce;", "package " + package + ";") << newLine;
            }

            javaSourceLines = StringArray::fromLines (newFile.toString());
        }

        while (javaSourceLines.size() > 2
                && javaSourceLines[javaSourceLines.size() - 1].trim().isEmpty()
                && javaSourceLines[javaSourceLines.size() - 2].trim().isEmpty())
            javaSourceLines.remove (javaSourceLines.size() - 1);

        return javaSourceLines.joinIntoString (newLine);
    }

    String getRuntimePermissionsCode (const File& javaSourceFolder, const String& className) const
    {
        if (static_cast<int> (androidMinimumSDK.get()) >= 23)
        {
            auto javaRuntimePermissions = javaSourceFolder.getChildFile ("AndroidRuntimePermissions.java");
            return javaRuntimePermissions.loadFileAsString().replace ("JuceAppActivity", className);
        }

        return {};
    }

    struct MidiCode
    {
        String imports;
        String main;
    };

    MidiCode getMidiCode (const File& javaSourceFolder, const String& className) const
    {
        String juceMidiCode, juceMidiImports;

        juceMidiImports << newLine;

        if (static_cast<int> (androidMinimumSDK.get()) >= 23)
        {
            auto javaAndroidMidi = javaSourceFolder.getChildFile ("AndroidMidi.java");

            juceMidiImports << "import android.media.midi.*;" << newLine
                            << "import android.bluetooth.*;" << newLine
                            << "import android.bluetooth.le.*;" << newLine;

            juceMidiCode = javaAndroidMidi.loadFileAsString().replace ("JuceAppActivity", className);
        }
        else
        {
            juceMidiCode = javaSourceFolder.getChildFile ("AndroidMidiFallback.java")
                                           .loadFileAsString()
                                           .replace ("JuceAppActivity", className);
        }

        return { juceMidiImports, juceMidiCode };
    }

    struct WebViewCode
    {
        String imports;
        String native;
        String main;
    };

    WebViewCode getWebViewCode (const File& javaSourceFolder) const
    {
        String juceWebViewImports, juceWebViewCodeNative, juceWebViewCode;

        if (static_cast<int> (androidMinimumSDK.get()) >= 23)
            juceWebViewImports << "import android.webkit.WebResourceError;" << newLine;

        if (static_cast<int> (androidMinimumSDK.get()) >= 21)
            juceWebViewImports << "import android.webkit.WebResourceRequest;" << newLine;

        if (static_cast<int> (androidMinimumSDK.get()) >= 11)
            juceWebViewImports << "import android.webkit.WebResourceResponse;" << newLine;

        auto javaWebViewFile = javaSourceFolder.getChildFile ("AndroidWebView.java");
        auto juceWebViewCodeAll = javaWebViewFile.loadFileAsString();

        if (static_cast<int> (androidMinimumSDK.get()) <= 10)
        {
            juceWebViewCode << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewApi1_10", false, false)
                                                 .upToFirstOccurrenceOf ("WebViewApi1_10$$", false, false);
        }
        else
        {
            if (static_cast<int> (androidMinimumSDK.get()) >= 23)
            {
                juceWebViewCodeNative << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewNativeApi23", false, false)
                                                           .upToFirstOccurrenceOf ("WebViewNativeApi23$$", false, false);

                juceWebViewCode << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewApi23", false, false)
                                                     .upToFirstOccurrenceOf ("WebViewApi23$$", false, false);
            }

            if (static_cast<int> (androidMinimumSDK.get()) >= 21)
            {
                juceWebViewCodeNative << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewNativeApi21", false, false)
                                                           .upToFirstOccurrenceOf ("WebViewNativeApi21$$", false, false);

                juceWebViewCode << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewApi21", false, false)
                                                     .upToFirstOccurrenceOf ("WebViewApi21$$", false, false);
            }
            else
            {
                juceWebViewCode << juceWebViewCodeAll.fromFirstOccurrenceOf ("$$WebViewApi11_20", false, false)
                                                     .upToFirstOccurrenceOf ("WebViewApi11_20$$", false, false);
            }
        }

        return { juceWebViewImports, juceWebViewCodeNative, juceWebViewCode };
    }

    struct CameraCode
    {
        String imports;
        String main;
    };

    CameraCode getCameraCode (const File& javaSourceFolder) const
    {
        String juceCameraImports, juceCameraCode;

        if (static_cast<int> (androidMinimumSDK.get()) >= 21)
        {
            juceCameraImports << "import android.hardware.camera2.*;" << newLine;

            auto javaCameraFile = javaSourceFolder.getChildFile ("AndroidCamera.java");
            auto juceCameraCodeAll = javaCameraFile.loadFileAsString();

            juceCameraCode << juceCameraCodeAll.fromFirstOccurrenceOf ("$$CameraApi21", false, false)
                                               .upToFirstOccurrenceOf ("CameraApi21$$", false, false);
        }

        return { juceCameraImports, juceCameraCode };
    }

    struct VideoCode
    {
        String imports;
        String main;
    };

    VideoCode getVideoCode (const File& javaSourceFolder) const
    {
        String juceVideoImports, juceVideoCode;

        if (static_cast<int> (androidMinimumSDK.get()) >= 21)
        {
            juceVideoImports << "import android.database.ContentObserver;" << newLine;
            juceVideoImports << "import android.media.session.*;" << newLine;
            juceVideoImports << "import android.media.MediaMetadata;" << newLine;

            auto javaVideoFile = javaSourceFolder.getChildFile ("AndroidVideo.java");
            auto juceVideoCodeAll = javaVideoFile.loadFileAsString();

            juceVideoCode << juceVideoCodeAll.fromFirstOccurrenceOf ("$$VideoApi21", false, false)
                                             .upToFirstOccurrenceOf ("VideoApi21$$", false, false);
        }

        return { juceVideoImports, juceVideoCode };
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

    void copyAdditionalJavaLibs (const File& targetFolder) const
    {
        auto libPaths = StringArray::fromLines (androidJavaLibs.get().toString());

        for (auto& p : libPaths)
        {
            File f = getTargetFolder().getChildFile (p);

            // Is the path to the java lib correct?
            jassert (f.existsAsFile());

            f.copyFileTo (targetFolder.getChildFile (f.getFileName()));
        }
    }

    void copyServicesJavaFiles (const File& javaSourceFolder, const File& targetFolder, const String& package) const
    {
        if (androidEnableRemoteNotifications.get())
        {
            String instanceIdFileName ("JuceFirebaseInstanceIdService.java");
            String messagingFileName  ("JuceFirebaseMessagingService.java");

            File instanceIdFile (javaSourceFolder.getChildFile (instanceIdFileName));
            File messagingFile  (javaSourceFolder.getChildFile (messagingFileName));

            jassert (instanceIdFile.existsAsFile());
            jassert (messagingFile .existsAsFile());

            Array<File> files;
            files.add (instanceIdFile);
            files.add (messagingFile);

            for (auto& file : files)
            {
                auto newContent = file.loadFileAsString()
                                      .replace ("package com.juce;", "package " + package + ";");
                auto targetFile = targetFolder.getChildFile (file.getFileName());
                overwriteFileIfDifferentOrThrow (targetFile, newContent);
            }
        }
    }

    void copyProviderJavaFile (const File& javaSourceFolder, const File& targetFolder, const String& package) const
    {
        auto providerFile = javaSourceFolder.getChildFile ("AndroidSharingContentProvider.java");

        jassert (providerFile.existsAsFile());

        auto targetFile = targetFolder.getChildFile ("SharingContentProvider.java");

        auto fileContent = providerFile.loadFileAsString()
                                       .replace ("package com.juce;", "package " + package + ";");

        auto commonStart = fileContent.upToFirstOccurrenceOf ("$$ContentProviderApi11", false, false);
        auto commonEnd   = fileContent.fromFirstOccurrenceOf ("ContentProviderApi11$$", false, false);

        auto middleContent = static_cast<int> (androidMinimumSDK.get()) >= 11
                                ? fileContent.fromFirstOccurrenceOf ("$$ContentProviderApi11", false, false)
                                             .upToFirstOccurrenceOf ("ContentProviderApi11$$", false, false)
                                : String();

        auto newContent = commonStart;
        newContent << middleContent << commonEnd;

        overwriteFileIfDifferentOrThrow (targetFile, newContent);
    }

    void copyExtraResourceFiles() const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);
            String cfgPath = cfg.isDebug() ? "app/src/debug" : "app/src/release";
            String xmlValuesPath = cfg.isDebug() ? "app/src/debug/res/values" : "app/src/release/res/values";
            String drawablesPath = cfg.isDebug() ? "app/src/debug/res" : "app/src/release/res";
            String rawPath = cfg.isDebug() ? "app/src/debug/res/raw" : "app/src/release/res/raw";

            copyExtraResourceFiles (cfg.getAdditionalXmlResources(), xmlValuesPath);
            copyExtraResourceFiles (cfg.getAdditionalDrawableResources(), drawablesPath);
            copyExtraResourceFiles (cfg.getAdditionalRawResources(), rawPath);

            if (androidEnableRemoteNotifications.get())
            {
                auto remoteNotifsConfigFilePath = cfg.getRemoteNotifsConfigFile();

                if (remoteNotifsConfigFilePath.isEmpty())
                    remoteNotifsConfigFilePath = androidRemoteNotificationsConfigFile.get().toString();

                File file (getProject().getFile().getChildFile (remoteNotifsConfigFilePath));
                // Settings file must be present for remote notifications to work and it must be called google-services.json.
                jassert (file.existsAsFile() && file.getFileName() == "google-services.json");

                copyExtraResourceFiles (remoteNotifsConfigFilePath, cfgPath);
            }
        }
    }

    void copyExtraResourceFiles (const String& resources, const String& dstRelativePath) const
    {
        auto resourcePaths = StringArray::fromTokens (resources, true);

        auto parentFolder = getTargetFolder().getChildFile (dstRelativePath);

        parentFolder.createDirectory();

        for (auto& path : resourcePaths)
        {
            auto file = getProject().getFile().getChildFile (path);

            jassert (file.exists());

            if (file.exists())
                file.copyFileTo (parentFolder.getChildFile (file.getFileName()));
        }
    }

    String getActivityName() const
    {
        return androidActivityClass.get().toString().fromLastOccurrenceOf (".", false, false);
    }

    String getActivitySubClassName() const
    {
        auto activityPath = androidActivitySubClassName.get().toString();

        return (activityPath.isEmpty()) ? getActivityName() : activityPath.fromLastOccurrenceOf (".", false, false);
    }

    String getActivityClassPackage() const
    {
        return androidActivityClass.get().toString().upToLastOccurrenceOf (".", false, false);
    }

    String getJNIActivityClassName() const
    {
        return androidActivityClass.get().toString().replaceCharacter ('.', '/');
    }

    static LibraryModule* getCoreModule (const OwnedArray<LibraryModule>& modules)
    {
        for (int i = modules.size(); --i >= 0;)
            if (modules.getUnchecked (i)->getID() == "juce_core")
                return modules.getUnchecked (i);

        return nullptr;
    }

    //==============================================================================
    String getNativeModuleBinaryName (const AndroidBuildConfiguration& config) const
    {
        return (isLibrary() ? File::createLegalFileName (config.getTargetBinaryNameString().trim()) : "juce_jni");
    }

    String getAppPlatform() const
    {
        auto ndkVersion = static_cast<int> (androidMinimumSDK.get());
        if (ndkVersion == 9)
            ndkVersion = 10; // (doesn't seem to be a version '9')

        return "android-" + String (ndkVersion);
    }

    //==============================================================================
    void writeStringsXML (const File& folder) const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

            String customStringsXmlContent ("<resources>\n");
            customStringsXmlContent << "<string name=\"app_name\">" << projectName << "</string>\n";
            customStringsXmlContent << cfg.getCustomStringsXml();
            customStringsXmlContent << "\n</resources>";

            std::unique_ptr<XmlElement> strings (XmlDocument::parse (customStringsXmlContent));

            String dir     = cfg.isDebug() ? "debug" : "release";
            String subPath = "app/src/" + dir + "/res/values/string.xml";

            writeXmlOrThrow (*strings, folder.getChildFile (subPath), "utf-8", 100, true);
        }
    }

    void writeAndroidManifest (const File& folder) const
    {
        std::unique_ptr<XmlElement> manifest (createManifestXML());

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
        std::unique_ptr<Drawable> bigIcon (getBigIcon());
        std::unique_ptr<Drawable> smallIcon (getSmallIcon());

        if (bigIcon != nullptr && smallIcon != nullptr)
        {
            auto step = jmax (bigIcon->getWidth(), bigIcon->getHeight()) / 8;
            writeIcon (folder.getChildFile ("drawable-xhdpi/icon.png"), getBestIconForSize (step * 8, false));
            writeIcon (folder.getChildFile ("drawable-hdpi/icon.png"),  getBestIconForSize (step * 6, false));
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"),  getBestIconForSize (step * 4, false));
            writeIcon (folder.getChildFile ("drawable-ldpi/icon.png"),  getBestIconForSize (step * 3, false));
        }
        else if (auto* icon = (bigIcon != nullptr ? bigIcon.get() : smallIcon.get()))
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
        auto homeFolder = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

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
            RelativePath file (projectItem.getFile(), getTargetFolder().getChildFile ("app"), RelativePath::buildTargetFolder);
            auto targetType = getProject().getTargetTypeFromFilePath (projectItem.getFile(), true);

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
        auto toolchain = gradleToolchain.get().toString();
        bool isClang = (toolchain == "clang");

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

        auto cppStandard = project.getCppStandardString();

        if (cppStandard == "latest" || cppStandard == "17") // C++17 flag isn't supported yet so use 1z for now
            cppStandard = "1z";

        cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

        cxxFlags.add (cppStandard.quoted());

        return cxxFlags;
    }

    StringArray getProjectCompilerFlags() const
    {
        auto cFlags = getAndroidCompilerFlags();

        cFlags.addArray (getEscapedFlags (StringArray::fromTokens (getExtraCompilerFlagsString(), true)));
        return cFlags;
    }

    StringArray getProjectCxxCompilerFlags() const
    {
        auto cxxFlags = getAndroidCxxCompilerFlags();
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
        defines.set ("JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSNAME", getSharingContentProviderClassName().replaceCharacter('.', '_'));
        defines.set ("JUCE_ANDROID_SHARING_CONTENT_PROVIDER_CLASSPATH", "\"" + getSharingContentProviderClassName().replaceCharacter('.', '/') + "\"");
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

    String getSharingContentProviderClassName() const
    {
        return getActivityClassPackage() + ".SharingContentProvider";
    }

    StringPairArray getProjectPreprocessorDefs() const
    {
        auto defines = getAndroidPreprocessorDefs();

        return mergePreprocessorDefs (defines, getAllPreprocessorDefs());
    }

    StringPairArray getConfigPreprocessorDefs (const BuildConfiguration& config) const
    {
        auto cfgDefines = config.getUniquePreprocessorDefs();

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
        auto paths = extraSearchPaths;
        paths.addArray (config.getHeaderSearchPaths());
        paths = getCleanedStringArray (paths);
        return paths;
    }

    //==============================================================================
    String escapeDirectoryForCmake (const String& path) const
    {
        auto relative =
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
            auto escaped = "\"-D" + defs.getAllKeys()[i];
            auto value = defs.getAllValues()[i];

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
        auto* manifest = createManifestElement();

        createSupportsScreensElement (*manifest);
        createUsesSdkElement         (*manifest);
        createPermissionElements     (*manifest);
        createOpenGlFeatureElement   (*manifest);

        if (! isLibrary())
        {
            auto* app = createApplicationElement (*manifest);

            auto* act = createActivityElement (*app);
            createIntentElement (*act);

            createServiceElements (*app);
            createProviderElement (*app);
        }

        return manifest;
    }

    XmlElement* createManifestElement() const
    {
        auto* manifest = XmlDocument::parse (androidManifestCustomXmlElements.get());

        if (manifest == nullptr)
            manifest = new XmlElement ("manifest");

        setAttributeIfNotPresent (*manifest, "xmlns:android", "http://schemas.android.com/apk/res/android");
        setAttributeIfNotPresent (*manifest, "android:versionCode", androidVersionCode.get());
        setAttributeIfNotPresent (*manifest, "android:versionName",  project.getVersionString());
        setAttributeIfNotPresent (*manifest, "package", getActivityClassPackage());

        return manifest;
    }

    void createSupportsScreensElement (XmlElement& manifest) const
    {
        if (! isLibrary())
        {
            if (manifest.getChildByName ("supports-screens") == nullptr)
            {
                auto* screens = manifest.createNewChildElement ("supports-screens");
                screens->setAttribute ("android:smallScreens", "true");
                screens->setAttribute ("android:normalScreens", "true");
                screens->setAttribute ("android:largeScreens", "true");
                screens->setAttribute ("android:anyDensity", "true");
            }
        }
    }

    void createUsesSdkElement (XmlElement& manifest) const
    {
        auto* sdk = getOrCreateChildWithName (manifest, "uses-sdk");
        setAttributeIfNotPresent (*sdk, "android:minSdkVersion", androidMinimumSDK.get());
        setAttributeIfNotPresent (*sdk, "android:targetSdkVersion", androidMinimumSDK.get());
    }

    void createPermissionElements (XmlElement& manifest) const
    {
        auto permissions = getPermissionsRequired();

        forEachXmlChildElementWithTagName (manifest, child, "uses-permission")
        {
            permissions.removeString (child->getStringAttribute ("android:name"), false);
        }

        for (int i = permissions.size(); --i >= 0;)
            manifest.createNewChildElement ("uses-permission")->setAttribute ("android:name", permissions[i]);
    }

    void createOpenGlFeatureElement (XmlElement& manifest) const
    {
        if (project.getModules().isModuleEnabled ("juce_opengl"))
        {
            XmlElement* glVersion = nullptr;

            forEachXmlChildElementWithTagName (manifest, child, "uses-feature")
            {
                if (child->getStringAttribute ("android:glEsVersion").isNotEmpty())
                {
                    glVersion = child;
                    break;
                }
            }

            if (glVersion == nullptr)
                glVersion = manifest.createNewChildElement ("uses-feature");

            setAttributeIfNotPresent (*glVersion, "android:glEsVersion", (static_cast<int> (androidMinimumSDK.get()) >= 18 ? "0x00030000" : "0x00020000"));
            setAttributeIfNotPresent (*glVersion, "android:required", "true");
        }
    }

    XmlElement* createApplicationElement (XmlElement& manifest) const
    {
        auto* app = getOrCreateChildWithName (manifest, "application");
        setAttributeIfNotPresent (*app, "android:label", "@string/app_name");

        if (androidTheme.get().toString().isNotEmpty())
            setAttributeIfNotPresent (*app, "android:theme", androidTheme.get());

        if (! app->hasAttribute ("android:icon"))
        {
            std::unique_ptr<Drawable> bigIcon (getBigIcon()), smallIcon (getSmallIcon());

            if (bigIcon != nullptr || smallIcon != nullptr)
                app->setAttribute ("android:icon", "@drawable/icon");
        }

        if (static_cast<int> (androidMinimumSDK.get()) >= 11)
        {
            if (! app->hasAttribute ("android:hardwareAccelerated"))
                app->setAttribute ("android:hardwareAccelerated", "false"); // (using the 2D acceleration slows down openGL)
        }
        else
        {
            app->removeAttribute ("android:hardwareAccelerated");
        }

        return app;
    }

    XmlElement* createActivityElement (XmlElement& application) const
    {
        auto* act = getOrCreateChildWithName (application, "activity");

        setAttributeIfNotPresent (*act, "android:name", getActivitySubClassName());
        setAttributeIfNotPresent (*act, "android:label", "@string/app_name");

        if (! act->hasAttribute ("android:configChanges"))
        {
            String configChanges ("keyboardHidden|orientation");
            if (static_cast<int> (androidMinimumSDK.get()) >= 13)
                configChanges += "|screenSize";

            act->setAttribute ("android:configChanges", configChanges);
        }
        else
        {
            auto configChanges = act->getStringAttribute ("android:configChanges");

            if (static_cast<int> (androidMinimumSDK.get()) < 13 && configChanges.contains ("screenSize"))
            {
                configChanges = configChanges.replace ("|screenSize", "")
                                             .replace ("screenSize|", "")
                                             .replace ("screenSize", "");

                act->setAttribute ("android:configChanges", configChanges);
            }
        }

        if (androidScreenOrientation.get() == "landscape")
        {
            String landscapeString = static_cast<int> (androidMinimumSDK.get()) < 9
                                   ? "landscape"
                                   : (static_cast<int> (androidMinimumSDK.get()) < 18 ? "sensorLandscape" : "userLandscape");

            setAttributeIfNotPresent (*act, "android:screenOrientation", landscapeString);
        }
        else
        {
            setAttributeIfNotPresent (*act, "android:screenOrientation", androidScreenOrientation.get());
        }

        setAttributeIfNotPresent (*act, "android:launchMode", "singleTask");

        // Using the 2D acceleration slows down OpenGL. We *do* enable it here for the activity though, and we disable it
        // in each ComponentPeerView instead. This way any embedded native views, which are not children of ComponentPeerView,
        // can still use hardware acceleration if needed (e.g. web view).
        if (static_cast<int> (androidMinimumSDK.get()) >= 11)
        {
            if (! act->hasAttribute ("android:hardwareAccelerated"))
                act->setAttribute ("android:hardwareAccelerated", "true"); // (using the 2D acceleration slows down openGL)
        }
        else
        {
            act->removeAttribute ("android:hardwareAccelerated");
        }

        return act;
    }

    void createIntentElement (XmlElement& application) const
    {
        auto* intent = getOrCreateChildWithName (application, "intent-filter");

        auto* action = getOrCreateChildWithName (*intent, "action");
        setAttributeIfNotPresent (*action, "android:name", "android.intent.action.MAIN");

        auto* category = getOrCreateChildWithName (*intent, "category");
        setAttributeIfNotPresent (*category, "android:name", "android.intent.category.LAUNCHER");
    }

    void createServiceElements (XmlElement& application) const
    {
        if (androidEnableRemoteNotifications.get())
        {
            auto* service = application.createNewChildElement ("service");
            service->setAttribute ("android:name", ".JuceFirebaseMessagingService");
            auto* intentFilter = service->createNewChildElement ("intent-filter");
            intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.MESSAGING_EVENT");

            service = application.createNewChildElement ("service");
            service->setAttribute ("android:name", ".JuceFirebaseInstanceIdService");
            intentFilter = service->createNewChildElement ("intent-filter");
            intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.INSTANCE_ID_EVENT");

            auto* metaData = application.createNewChildElement ("meta-data");
            metaData->setAttribute ("android:name", "firebase_analytics_collection_deactivated");
            metaData->setAttribute ("android:value", "true");
        }
    }

    void createProviderElement (XmlElement& application) const
    {
        if (androidEnableContentSharing.get())
        {
            auto* provider = application.createNewChildElement ("provider");
            provider->setAttribute ("android:name", getSharingContentProviderClassName());
            provider->setAttribute ("android:authorities", getSharingContentProviderClassName().toLowerCase());
            provider->setAttribute ("android:grantUriPermissions", "true");
            provider->setAttribute ("android:exported", "false");
        }
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
        StringArray s = StringArray::fromTokens (androidOtherPermissions.get().toString(), ", ", {});

        if (androidInternetNeeded.get())
            s.add ("android.permission.INTERNET");

        if (androidMicNeeded.get())
            s.add ("android.permission.RECORD_AUDIO");

        if (androidCameraNeeded.get())
            s.add ("android.permission.CAMERA");

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
        return (static_cast<int> (androidMinimumSDK.get()) >= 18);
    }

    //==============================================================================
    Value sdkPath, ndkPath;
    const File AndroidExecutable;

    JUCE_DECLARE_NON_COPYABLE (AndroidProjectExporter)
};

inline ProjectExporter* createAndroidExporter (Project& p, const ValueTree& t)
{
    return new AndroidProjectExporter (p, t);
}

inline ProjectExporter* createAndroidExporterForSetting (Project& p, const ValueTree& t)
{
    return AndroidProjectExporter::createForSettings (p, t);
}
