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

    bool isAndroid() const override              { return true; }
    bool isWindows() const override              { return false; }
    bool isLinux() const override                { return false; }
    bool isOSX() const override                  { return false; }
    bool isiOS() const override                  { return false; }

    bool usesMMFiles() const override                       { return false; }
    bool canCopeWithDuplicateFiles() override               { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    String getNewLineString() const override     { return "\n"; }

    bool supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        return type == build_tools::ProjectType::Target::GUIApp || type == build_tools::ProjectType::Target::StaticLibrary
              || type == build_tools::ProjectType::Target::DynamicLibrary || type == build_tools::ProjectType::Target::StandalonePlugIn;
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) override
    {
        // no-op.
    }

    //==============================================================================
    void createExporterProperties (PropertyListBuilder& props) override
    {
        createBaseExporterProperties (props);
        createToolchainExporterProperties (props);
        createManifestExporterProperties (props);
        createCodeSigningExporterProperties (props);
        createOtherExporterProperties (props);
    }

    static String getDisplayName()        { return "Android"; }
    static String getValueTreeTypeName()  { return "ANDROIDSTUDIO"; }
    static String getTargetFolderName()   { return "Android"; }

    Identifier getExporterIdentifier() const override { return getValueTreeTypeName(); }

    static const char* getDefaultActivityClass()     { return "com.rmsl.juce.JuceActivity"; }
    static const char* getDefaultApplicationClass()  { return "com.rmsl.juce.JuceApp"; }

    static AndroidProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new AndroidProjectExporter (projectToUse, settingsToUse);

        return {};
    }

    //==============================================================================
    ValueTreePropertyWithDefault androidJavaLibs, androidAdditionalJavaFolders, androidAdditionalResourceFolders, androidProjectRepositories,
                                 androidRepositories, androidDependencies, androidCustomAppBuildGradleContent, androidScreenOrientation,
                                 androidCustomActivityClass, androidCustomApplicationClass, androidManifestCustomXmlElements,
                                 androidGradleSettingsContent, androidVersionCode, androidMinimumSDK, androidTargetSDK, androidTheme,
                                 androidExtraAssetsFolder, androidOboeRepositoryPath, androidInternetNeeded, androidMicNeeded, androidCameraNeeded,
                                 androidBluetoothNeeded, androidExternalReadPermission, androidExternalWritePermission,
                                 androidInAppBillingPermission, androidVibratePermission, androidOtherPermissions, androidPushNotifications,
                                 androidEnableRemoteNotifications, androidRemoteNotificationsConfigFile, androidEnableContentSharing, androidKeyStore,
                                 androidKeyStorePass, androidKeyAlias, androidKeyAliasPass, gradleVersion, gradleToolchain, androidPluginVersion;

    //==============================================================================
    AndroidProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          androidJavaLibs                      (settings, Ids::androidJavaLibs,                      getUndoManager()),
          androidAdditionalJavaFolders         (settings, Ids::androidAdditionalJavaFolders,         getUndoManager()),
          androidAdditionalResourceFolders     (settings, Ids::androidAdditionalResourceFolders,     getUndoManager()),
          androidProjectRepositories           (settings, Ids::androidProjectRepositories,           getUndoManager(), "google()\nmavenCentral()"),
          androidRepositories                  (settings, Ids::androidRepositories,                  getUndoManager()),
          androidDependencies                  (settings, Ids::androidDependencies,                  getUndoManager()),
          androidCustomAppBuildGradleContent   (settings, Ids::androidCustomAppBuildGradleContent,   getUndoManager()),
          androidScreenOrientation             (settings, Ids::androidScreenOrientation,             getUndoManager(), "unspecified"),
          androidCustomActivityClass           (settings, Ids::androidCustomActivityClass,           getUndoManager()),
          androidCustomApplicationClass        (settings, Ids::androidCustomApplicationClass,        getUndoManager(), getDefaultApplicationClass()),
          androidManifestCustomXmlElements     (settings, Ids::androidManifestCustomXmlElements,     getUndoManager()),
          androidGradleSettingsContent         (settings, Ids::androidGradleSettingsContent,         getUndoManager()),
          androidVersionCode                   (settings, Ids::androidVersionCode,                   getUndoManager(), "1"),
          androidMinimumSDK                    (settings, Ids::androidMinimumSDK,                    getUndoManager(), "16"),
          androidTargetSDK                     (settings, Ids::androidTargetSDK,                     getUndoManager(), "30"),
          androidTheme                         (settings, Ids::androidTheme,                         getUndoManager()),
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
          androidPushNotifications             (settings, Ids::androidPushNotifications,             getUndoManager(), ! isLibrary()),
          androidEnableRemoteNotifications     (settings, Ids::androidEnableRemoteNotifications,     getUndoManager(), false),
          androidRemoteNotificationsConfigFile (settings, Ids::androidRemoteNotificationsConfigFile, getUndoManager()),
          androidEnableContentSharing          (settings, Ids::androidEnableContentSharing,          getUndoManager(), false),
          androidKeyStore                      (settings, Ids::androidKeyStore,                      getUndoManager(), "${user.home}/.android/debug.keystore"),
          androidKeyStorePass                  (settings, Ids::androidKeyStorePass,                  getUndoManager(), "android"),
          androidKeyAlias                      (settings, Ids::androidKeyAlias,                      getUndoManager(), "androiddebugkey"),
          androidKeyAliasPass                  (settings, Ids::androidKeyAliasPass,                  getUndoManager(), "android"),
          gradleVersion                        (settings, Ids::gradleVersion,                        getUndoManager(), "7.0.2"),
          gradleToolchain                      (settings, Ids::gradleToolchain,                      getUndoManager(), "clang"),
          androidPluginVersion                 (settings, Ids::androidPluginVersion,                 getUndoManager(), "7.0.0"),
          AndroidExecutable                    (getAppSettings().getStoredPath (Ids::androidStudioExePath, TargetOS::getThisOS()).get().toString())
    {
        name = getDisplayName();
        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderName());
    }

    //==============================================================================
    void createToolchainExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (gradleVersion, "Gradle Version", 32, false),
                   "The version of gradle that is used to build this app (4.10 is fine for JUCE)");

        props.add (new TextPropertyComponent (androidPluginVersion, "Android Plug-in Version", 32, false),
                   "The version of the android build plugin for gradle that is used to build this app");

        props.add (new ChoicePropertyComponent (gradleToolchain, "NDK Toolchain",
                                                { "clang", "gcc" },
                                                { "clang", "gcc" }),
                   "The toolchain that gradle should invoke for NDK compilation (variable model.android.ndk.tooclhain in app/build.gradle)");
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
        copyExtraResourceFiles();

        writeFile (targetFolder, "settings.gradle",                          getGradleSettingsFileContent());
        writeFile (targetFolder, "build.gradle",                             getProjectBuildGradleFileContent());
        writeFile (appFolder,    "build.gradle",                             getAppBuildGradleFileContent (modules));
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
            copyAdditionalJavaLibs (appFolder);
            writeStringsXML        (targetFolder);
            writeAppIcons          (targetFolder);
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
        build_tools::writeStreamToFile (gradleProjectFolder.getChildFile (filePath), [&] (MemoryOutputStream& mo)
        {
            mo.setNewLineString (getNewLineString());
            mo << fileContent;
        });
    }

    void writeBinaryFile (const File& gradleProjectFolder, const String& filePath, const char* binaryData, const int binarySize) const
    {
        build_tools::writeStreamToFile (gradleProjectFolder.getChildFile (filePath), [&] (MemoryOutputStream& mo)
        {
            mo.setNewLineString (getNewLineString());
            mo.write (binaryData, static_cast<size_t> (binarySize));
        });
    }

protected:
    //==============================================================================
    class AndroidBuildConfiguration  : public BuildConfiguration
    {
    public:
        AndroidBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              androidArchitectures                     (config, Ids::androidArchitectures,                     getUndoManager(), isDebug() ? "armeabi-v7a x86 arm64-v8a x86_64" : ""),
              androidBuildConfigRemoteNotifsConfigFile (config, Ids::androidBuildConfigRemoteNotifsConfigFile, getUndoManager()),
              androidAdditionalXmlValueResources       (config, Ids::androidAdditionalXmlValueResources,       getUndoManager()),
              androidAdditionalDrawableResources       (config, Ids::androidAdditionalDrawableResources,       getUndoManager()),
              androidAdditionalRawValueResources       (config, Ids::androidAdditionalRawValueResources,       getUndoManager()),
              androidCustomStringXmlElements           (config, Ids::androidCustomStringXmlElements,           getUndoManager())
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
            addRecommendedLLVMCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            props.add (new TextPropertyComponent (androidArchitectures, "Architectures", 256, false),
                       "A list of the architectures to build (for a fat binary). Leave empty to build for all possible android architectures.");

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

            props.add (new TextPropertyComponent (androidCustomStringXmlElements, "Custom String Resources", 8192, true),
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

        ValueTreePropertyWithDefault androidArchitectures, androidBuildConfigRemoteNotifsConfigFile,
                                     androidAdditionalXmlValueResources, androidAdditionalDrawableResources,
                                     androidAdditionalRawValueResources, androidCustomStringXmlElements;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return *new AndroidBuildConfiguration (project, v, *this);
    }

private:
    void writeCmakeFile (const File& file) const
    {
        build_tools::writeStreamToFile (file, [&] (MemoryOutputStream& mo)
        {
            mo.setNewLineString (getNewLineString());

            mo << "# Automatically generated makefile, created by the Projucer" << newLine
               << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
               << newLine;

            mo << "cmake_minimum_required(VERSION 3.4.1)" << newLine << newLine;

            if (! isLibrary())
                mo << "set(BINARY_NAME \"juce_jni\")" << newLine << newLine;

            auto useOboe = project.getEnabledModules().isModuleEnabled ("juce_audio_devices")
                          && project.isConfigFlagEnabled ("JUCE_USE_ANDROID_OBOE", true);

            if (useOboe)
            {
                auto oboePath = [&]
                {
                    auto oboeDir = androidOboeRepositoryPath.get().toString().trim();

                    if (oboeDir.isEmpty())
                        oboeDir = getModuleFolderRelativeToProject ("juce_audio_devices").getChildFile ("native")
                                                                                         .getChildFile ("oboe")
                                                                                         .rebased (getProject().getProjectFolder(), getTargetFolder(),
                                                                                                   build_tools::RelativePath::buildTargetFolder)
                                                                                         .toUnixStyle();

                    // CMakeLists.txt is in the "app" subfolder
                    if (! build_tools::isAbsolutePath (oboeDir))
                        oboeDir = "../" + oboeDir;

                    return expandHomeFolderToken (oboeDir);
                }();

                mo << "set(OBOE_DIR " << oboePath.quoted() << ")" << newLine << newLine;
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

                mo << ")" << newLine << newLine;
            }

            auto cfgExtraLinkerFlags = getExtraLinkerFlagsString();
            if (cfgExtraLinkerFlags.isNotEmpty())
            {
                mo << "set( JUCE_LDFLAGS \"" << cfgExtraLinkerFlags.replace ("\"", "\\\"") << "\")" << newLine;
                mo << "set( CMAKE_SHARED_LINKER_FLAGS  \"${CMAKE_EXE_LINKER_FLAGS} ${JUCE_LDFLAGS}\")" << newLine << newLine;
            }

            mo << "enable_language(ASM)" << newLine << newLine;

            const auto userLibraries = getUserLibraries();

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

                    mo << (first ? "if" : "elseif") << "(JUCE_BUILD_CONFIGURATION MATCHES \"" << cfg.getProductFlavourCMakeIdentifier() <<"\")" << newLine;

                    if (isLibrary())
                    {
                        mo << "    set(BINARY_NAME \"" << getNativeModuleBinaryName (cfg) << "\")" << newLine;

                        auto binaryLocation = cfg.getTargetBinaryRelativePathString();

                        if (binaryLocation.isNotEmpty())
                        {
                            auto locationRelativeToCmake = build_tools::RelativePath (binaryLocation, build_tools::RelativePath::projectFolder)
                                .rebased (getProject().getFile().getParentDirectory(),
                                          file.getParentDirectory(), build_tools::RelativePath::buildTargetFolder);

                            mo << "    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \"" << "../../../../" << locationRelativeToCmake.toUnixStyle() << "\")" << newLine;
                        }
                    }

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
                            mo << "        set(" << configVariable << " \"${" << configVariable << "} -flto\")" << newLine;
                        }

                        mo << "    endif()" << newLine;
                    }

                    first = false;
                }

                if (! first)
                {
                    ProjectExporter::BuildConfiguration::Ptr config (getConfiguration(0));

                    if (config)
                    {
                        if (dynamic_cast<const AndroidBuildConfiguration*> (config.get()) != nullptr)
                        {
                            mo << "else()" << newLine;
                            mo << "    message( FATAL_ERROR \"No matching build-configuration found.\" )" << newLine;
                            mo << "endif()" << newLine << newLine;
                        }
                    }
                }
            }

            Array<build_tools::RelativePath> excludeFromBuild;
            Array<std::pair<build_tools::RelativePath, String>> extraCompilerFlags;

            mo << "add_library( ${BINARY_NAME}" << newLine;
            mo << newLine;
            mo << "    " << (getProject().getProjectType().isStaticLibrary() ? "STATIC" : "SHARED") << newLine;
            mo << newLine;
            addCompileUnits (mo, excludeFromBuild, extraCompilerFlags);
            mo << ")" << newLine << newLine;

            if (excludeFromBuild.size() > 0)
            {
                mo << "set_source_files_properties(" << newLine;

                for (auto& exclude : excludeFromBuild)
                    mo << "    \"" << exclude.toUnixStyle() << '"' << newLine;

                mo << "    PROPERTIES HEADER_FILE_ONLY TRUE)" << newLine;
                mo << newLine;
            }

            if (! extraCompilerFlags.isEmpty())
            {
                for (auto& extra : extraCompilerFlags)
                    mo << "set_source_files_properties(\"" << extra.first.toUnixStyle() << "\" PROPERTIES COMPILE_FLAGS " << extra.second << " )" << newLine;

                mo << newLine;
            }

            auto flags = getProjectCompilerFlags();

            if (flags.size() > 0)
                mo << "target_compile_options( ${BINARY_NAME} PRIVATE " << flags.joinIntoString (" ") << " )" << newLine << newLine;

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

                mo << "if( JUCE_BUILD_CONFIGURATION MATCHES \"" << cfg.getProductFlavourCMakeIdentifier() << "\" )" << newLine;
                mo << "    target_compile_options( ${BINARY_NAME} PRIVATE";

                auto recommendedFlags = cfg.getRecommendedCompilerWarningFlags();

                for (auto& recommendedFlagsType : { recommendedFlags.common, recommendedFlags.cpp })
                    for (auto& flag : recommendedFlagsType)
                        mo << " " << flag;

                mo << ")" << newLine;
                mo << "endif()" << newLine << newLine;
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

            if (useOboe)
                mo << "    \"oboe\"" << newLine;

            mo << ")" << newLine;
        });
    }

    //==============================================================================
    String getGradleSettingsFileContent() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "rootProject.name = " << "\'" << escapeQuotes (projectName) << "\'" << newLine;
        mo << (isLibrary() ? "include ':lib'" : "include ':app'");

        auto extraContent = androidGradleSettingsContent.get().toString();

        if (extraContent.isNotEmpty())
            mo << newLine << extraContent << newLine;

        return mo.toString();
    }

    String getProjectBuildGradleFileContent() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "buildscript {"                                                                              << newLine;
        mo << "   repositories {"                                                                          << newLine;
        mo << "       google()"                                                                            << newLine;
        mo << "       mavenCentral()"                                                                      << newLine;
        mo << "   }"                                                                                       << newLine;
        mo << "   dependencies {"                                                                          << newLine;
        mo << "       classpath 'com.android.tools.build:gradle:" << androidPluginVersion.get().toString() << "'" << newLine;

        if (areRemoteNotificationsEnabled())
            mo << "       classpath 'com.google.gms:google-services:4.0.1'" << newLine;

        mo << "   }"                                                                                   << newLine;
        mo << "}"                                                                                      << newLine;
        mo << ""                                                                                       << newLine;
        mo << "allprojects {"                                                                          << newLine;
        mo << getAndroidProjectRepositories();
        mo << "}"                                                                                      << newLine;

        return mo.toString();
    }

    //==============================================================================
    String getAppBuildGradleFileContent (const OwnedArray<LibraryModule>& modules) const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "apply plugin: 'com.android." << (isLibrary() ? "library" : "application") << "'" << newLine << newLine;

        mo << "android {"                                                                    << newLine;
        mo << "    compileSdkVersion " << static_cast<int> (androidTargetSDK.get())          << newLine;
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

        mo << getAndroidJavaSourceSets (modules)                                             << newLine;
        mo << getAndroidRepositories()                                                       << newLine;
        mo << getAndroidDependencies()                                                       << newLine;
        mo << androidCustomAppBuildGradleContent.get().toString()                            << newLine;
        mo << getApplyPlugins()                                                              << newLine;

        mo << "}"                                                                            << newLine << newLine;

        return mo.toString();
    }

    String getAndroidProductFlavours() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

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

            mo << "                    arguments "
               << "\"-DJUCE_BUILD_CONFIGURATION=" << cfg.getProductFlavourCMakeIdentifier() << "\"";

            mo << ", \"-DCMAKE_CXX_FLAGS_" << (cfg.isDebug() ? "DEBUG" : "RELEASE")
               << "=-O" << cfg.getGCCOptimisationFlag();

            mo << "\""
               << ", \"-DCMAKE_C_FLAGS_"   << (cfg.isDebug() ? "DEBUG" : "RELEASE")
               << "=-O" << cfg.getGCCOptimisationFlag()
               << "\"" << newLine;

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
        mo.setNewLineString (getNewLineString());

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
        auto minSdkVersion     = static_cast<int> (androidMinimumSDK.get());
        auto targetSdkVersion  = static_cast<int> (androidTargetSDK.get());

        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "    defaultConfig {"                                               << newLine;

        if (! isLibrary())
            mo << "        applicationId \"" << bundleIdentifier << "\""          << newLine;

        mo << "        minSdkVersion    " << minSdkVersion                        << newLine;
        mo << "        targetSdkVersion " << targetSdkVersion                     << newLine;

        mo << "        externalNativeBuild {"                                     << newLine;
        mo << "            cmake {"                                               << newLine;

        mo << "                arguments " << cmakeDefs.joinIntoString (", ")     << newLine;

        mo << "            }"                                                     << newLine;
        mo << "        }"                                                         << newLine;
        mo << "    }"                                                             << newLine;

        return mo.toString();
    }

    String getAndroidBuildTypes() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

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
        mo.setNewLineString (getNewLineString());

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

    String getAndroidProjectRepositories() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        auto repositories = StringArray::fromLines (androidProjectRepositories.get().toString());

        if (areRemoteNotificationsEnabled())
            repositories.add ("maven { url \"https://maven.google.com\" }");

        mo << "   repositories {"                << newLine;

        for (auto& r : repositories)
            mo << "       " << r << newLine;

        mo << "   }"                             << newLine;

        return mo.toString();
    }

    String getAndroidRepositories() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        auto repositories = StringArray::fromLines (androidRepositories.get().toString());

        mo << "    repositories {"                << newLine;

        for (auto& r : repositories)
            mo << "        " << r << newLine;

        mo << "    }"                             << newLine;

        return mo.toString();
    }

    String getAndroidDependencies() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "    dependencies {" << newLine;

        for (auto& d : StringArray::fromLines (androidDependencies.get().toString()))
            mo << "        " << d << newLine;

        for (auto& d : StringArray::fromLines (androidJavaLibs.get().toString()))
            mo << "        implementation files('libs/" << File (d).getFileName() << "')" << newLine;

        if (isInAppBillingEnabled())
            mo << "        implementation 'com.android.billingclient:billing:5.0.0'" << newLine;

        if (areRemoteNotificationsEnabled())
        {
            mo << "        implementation 'com.google.firebase:firebase-core:16.0.1'" << newLine;
            mo << "        implementation 'com.google.firebase:firebase-messaging:17.6.0'" << newLine;
        }

        mo << "    }" << newLine;

        return mo.toString();
    }

    String getApplyPlugins() const
    {
        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        if (areRemoteNotificationsEnabled())
            mo << "apply plugin: 'com.google.gms.google-services'" << newLine;

        return mo.toString();
    }

    void addModuleJavaFolderToSourceSet(StringArray& javaSourceSets, const File& source) const
    {
        if (source.isDirectory())
        {
            auto appFolder = getTargetFolder().getChildFile ("app");

            build_tools::RelativePath relativePath (source, appFolder, build_tools::RelativePath::buildTargetFolder);
            javaSourceSets.add (relativePath.toUnixStyle());
        }
    }

    void addOptJavaFolderToSourceSetsForModule (StringArray& javaSourceSets,
                                                const OwnedArray<LibraryModule>& modules,
                                                const String& moduleID) const
    {
        for (auto& m : modules)
        {
            if (m->getID() == moduleID)
            {
                auto javaFolder = m->getFolder().getChildFile ("native").getChildFile ("javaopt");
                addModuleJavaFolderToSourceSet (javaSourceSets, javaFolder.getChildFile ("app"));
                return;
            }
        }
    }

    String getAndroidJavaSourceSets (const OwnedArray<LibraryModule>& modules) const
    {
        auto javaSourceSets = getSourceSetArrayFor (androidAdditionalJavaFolders.get().toString());
        auto resourceSets   = getSourceSetArrayFor (androidAdditionalResourceFolders.get().toString());

        for (auto* module : modules)
        {
            auto javaFolder = module->getFolder().getChildFile ("native").getChildFile ("javacore");

            addModuleJavaFolderToSourceSet (javaSourceSets, javaFolder.getChildFile ("init"));

            if (! isLibrary())
                addModuleJavaFolderToSourceSet (javaSourceSets, javaFolder.getChildFile ("app"));
        }

        if (isUsingDefaultActivityClass() || isContentSharingEnabled())
            addOptJavaFolderToSourceSetsForModule (javaSourceSets, modules, "juce_gui_basics");

        if (areRemoteNotificationsEnabled())
            addOptJavaFolderToSourceSetsForModule (javaSourceSets, modules, "juce_gui_extra");

        if (isInAppBillingEnabled())
            addOptJavaFolderToSourceSetsForModule (javaSourceSets, modules, "juce_product_unlocking");

        MemoryOutputStream mo;
        mo.setNewLineString (getNewLineString());

        mo << "    sourceSets {" << newLine;
        mo << getSourceSetStringFor ("main.java.srcDirs", javaSourceSets, getNewLineString());
        mo << newLine;
        mo << getSourceSetStringFor ("main.res.srcDirs", resourceSets, getNewLineString());
        mo << "    }" << newLine;

        return mo.toString();
    }

    StringArray getSourceSetArrayFor (const String& srcDirs) const
    {
        StringArray sourceSets;

        for (auto folder : StringArray::fromLines (srcDirs))
        {
            if (File::isAbsolutePath (folder))
            {
                sourceSets.add (folder);
            }
            else
            {
                auto appFolder = getTargetFolder().getChildFile ("app");

                auto relativePath = build_tools::RelativePath (folder, build_tools::RelativePath::projectFolder)
                                        .rebased (getProject().getProjectFolder(), appFolder,
                                                  build_tools::RelativePath::buildTargetFolder);

                sourceSets.add (relativePath.toUnixStyle());
            }
        }

        return sourceSets;
    }

    static String getSourceSetStringFor (const String& type, const StringArray& srcDirs, const String& newLineString)
    {
        String s;

        s << "        " << type << " +=" << newLine;
        s << "            [";

        bool isFirst = true;

        for (auto sourceSet : srcDirs)
        {
            if (! isFirst)
                s << "," << newLine << "             ";

            isFirst = false;
            s << "\"" << sourceSet << "\"";
        }

        s << "]"     << newLine;

        return replaceLineFeeds (s, newLineString);
    }

    //==============================================================================
    String getLocalPropertiesFileContent() const
    {
        String props;

        props << "sdk.dir=" << sanitisePath (getAppSettings().getStoredPath (Ids::androidSDKPath, TargetOS::getThisOS()).get().toString()) << newLine;

        return replaceLineFeeds (props, getNewLineString());
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
        props.add (new TextPropertyComponent (androidAdditionalJavaFolders, "Java Source code folders", 32768, true),
                   "Folders inside which additional java source files can be found (one per line). For example, if you "
                   "are using your own Activity you should place the java files for this into a folder and add the folder "
                   "path to this field.");

        props.add (new TextPropertyComponent (androidAdditionalResourceFolders, "Resource folders", 32768, true),
                   "Folders inside which additional resource files can be found (one per line). For example, if you "
                   "want to add your own layout xml files then you should place a layout xml file inside a folder and add "
                   "the folder path to this field.");

        props.add (new TextPropertyComponent (androidJavaLibs, "Java libraries to include", 32768, true),
                   "Java libs (JAR files) (one per line). These will be copied to app/libs folder and \"implementation files\" "
                   "dependency will be automatically added to module \"dependencies\" section for each library, so do "
                   "not add the dependency yourself.");

        props.add (new TextPropertyComponent (androidProjectRepositories, "Project Repositories", 32768, true),
                   "Custom project repositories (one per line). These will be used in project-level gradle file "
                   "\"allprojects { repositories {\" section instead of default ones.");

        props.add (new TextPropertyComponent (androidRepositories, "Module Repositories", 32768, true),
                   "Module repositories (one per line). These will be added to module-level gradle file repositories section. ");

        props.add (new TextPropertyComponent (androidDependencies, "Module Dependencies", 32768, true),
                   "Module dependencies (one per line). These will be added to module-level gradle file \"dependencies\" section. "
                   "If adding any java libs in \"Java libraries to include\" setting, do not add them here as "
                   "they will be added automatically.");

        props.add (new TextPropertyComponent (androidCustomAppBuildGradleContent, "Extra module's build.gradle content", 32768, true),
                   "Additional content to be appended to module's build.gradle inside android { section. ");

        props.add (new TextPropertyComponent (androidGradleSettingsContent, "Custom gradle.settings content", 32768, true),
                   "You can customize the content of settings.gradle here");

        props.add (new ChoicePropertyComponent (androidScreenOrientation, "Screen Orientation",
                                                { "Portrait and Landscape", "Portrait", "Landscape" },
                                                { "unspecified",            "portrait", "landscape" }),
                   "The screen orientations that this app should support");

        props.add (new TextPropertyComponent (androidCustomActivityClass, "Custom Android Activity", 256, false),
                   "If not empty, specifies the Android Activity class name stored in the app's manifest which "
                   "should be used instead of Android's default Activity. If you specify a custom Activity "
                   "then you should implement onNewIntent() function like the one in com.rmsl.juce.JuceActivity, if "
                   "you wish to be able to handle push notification events.");

        props.add (new TextPropertyComponent (androidCustomApplicationClass, "Custom Android Application", 256, false),
                   "If not empty, specifies the Android Application class name stored in the app's manifest which "
                   "should be used instead of JUCE's default JuceApp class. If you specify a custom App then you must "
                   "call com.rmsl.juce.Java.initialiseJUCE somewhere in your code before calling any JUCE functions.");

        props.add (new TextPropertyComponent (androidVersionCode, "Android Version Code", 32, false),
                   "An integer value that represents the version of the application code, relative to other versions.");

        props.add (new TextPropertyComponent (androidMinimumSDK, "Minimum SDK Version", 32, false),
                   "The number of the minimum version of the Android SDK that the app requires (must be 16 or higher).");

        props.add (new TextPropertyComponent (androidTargetSDK, "Target SDK Version", 32, false),
                   "The number of the version of the Android SDK that the app is targeting.");

        props.add (new TextPropertyComponent (androidExtraAssetsFolder, "Extra Android Assets", 256, false),
                   "A path to a folder (relative to the project folder) which contains extra android assets.");
    }

    //==============================================================================
    void createManifestExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidOboeRepositoryPath, "Custom Oboe Repository", 2048, false),
                   "Path to the root of Oboe repository. This path can be absolute, or relative to the build directory. "
                   "Make sure to point Oboe repository to commit with SHA c5c3cc17f78974bf005bf33a2de1a093ac55cc07 before building. "
                   "Leave blank to use the version of Oboe distributed with JUCE.");

        props.add (new ChoicePropertyComponent (androidInternetNeeded, "Internet Access"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidMicNeeded, "Audio Input Required"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidCameraNeeded, "Camera Required"),
                   "If enabled, this will set the android.permission.CAMERA flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidBluetoothNeeded, "Bluetooth Permissions Required"),
                   "If enabled, this will set the android.permission.BLUETOOTH and  android.permission.BLUETOOTH_ADMIN flag in the manifest. This is required for Bluetooth MIDI on Android.");

        props.add (new ChoicePropertyComponent (androidExternalReadPermission, "Read From External Storage"),
                   "If enabled, this will set the android.permission.READ_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidExternalWritePermission, "Write to External Storage"),
                   "If enabled, this will set the android.permission.WRITE_EXTERNAL_STORAGE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidInAppBillingPermission, "In-App Billing"),
                   "If enabled, this will set the com.android.vending.BILLING flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidVibratePermission, "Vibrate"),
                   "If enabled, this will set the android.permission.VIBRATE flag in the manifest.");

        props.add (new ChoicePropertyComponent (androidEnableContentSharing, "Content Sharing"),
                   "If enabled, your app will be able to share content with other apps.");

        props.add (new TextPropertyComponent (androidOtherPermissions, "Custom Permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");

        props.add (new ChoicePropertyComponent (androidPushNotifications, "Push Notifications Capability"),
                   "Enable this to grant your app the capability to receive push notifications.");

        props.add (new ChoicePropertyComponentWithEnablement (androidEnableRemoteNotifications, androidPushNotifications, "Remote Notifications"),
                   "Enable to be able to send remote notifications to devices running your app (min API level 14). Enable the \"Push Notifications Capability\" "
                   "setting, provide Remote Notifications Config File, configure your app in Firebase Console and ensure you have the latest Google Repository "
                   "in Android Studio's SDK Manager.");

        props.add (new TextPropertyComponent (androidRemoteNotificationsConfigFile.getPropertyAsValue(), "Remote Notifications Config File", 2048, false),
                   "Path to google-services.json file. This will be the file provided by Firebase when creating a new app in Firebase console.");

        props.add (new TextPropertyComponent (androidManifestCustomXmlElements, "Custom Manifest XML Content", 8192, true),
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
    void copyAdditionalJavaLibs (const File& targetFolder) const
    {
        auto libFolder = targetFolder.getChildFile ("libs");
        libFolder.createDirectory();

        auto libPaths = StringArray::fromLines (androidJavaLibs.get().toString());

        for (auto& p : libPaths)
        {
            auto f = getTargetFolder().getChildFile (p);

            // Is the path to the java lib correct?
            jassert (f.existsAsFile());

            f.copyFileTo (libFolder.getChildFile (f.getFileName()));
        }
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

            if (areRemoteNotificationsEnabled())
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

    //==============================================================================
    String getActivityClassString() const
    {
        auto customActivityClass = androidCustomActivityClass.get().toString();

        if (customActivityClass.isNotEmpty())
            return customActivityClass;

        return arePushNotificationsEnabled() ? getDefaultActivityClass() : "android.app.Activity";
    }

    String getApplicationClassString() const    { return androidCustomApplicationClass.get(); }
    String getJNIActivityClassName() const      { return getActivityClassString().replaceCharacter ('.', '/'); }

    bool isUsingDefaultActivityClass() const    { return getActivityClassString() == getDefaultActivityClass(); }

    //==============================================================================
    bool arePushNotificationsEnabled() const
    {
        return project.getEnabledModules().isModuleEnabled ("juce_gui_extra")
              && androidPushNotifications.get();
    }

    bool areRemoteNotificationsEnabled() const
    {
        return arePushNotificationsEnabled()
              && androidEnableRemoteNotifications.get();
    }

    bool isInAppBillingEnabled() const
    {
        return project.getEnabledModules().isModuleEnabled ("juce_product_unlocking")
              && androidInAppBillingPermission.get();
    }

    bool isContentSharingEnabled() const
    {
        return project.getEnabledModules().isModuleEnabled ("juce_gui_basics")
              && androidEnableContentSharing.get();
    }

    //==============================================================================
    String getNativeModuleBinaryName (const AndroidBuildConfiguration& config) const
    {
        return (isLibrary() ? File::createLegalFileName (config.getTargetBinaryNameString().trim()) : "juce_jni");
    }

    String getAppPlatform() const
    {
        return "android-" + androidMinimumSDK.get().toString();
    }

    static String escapeQuotes (const String& str)
    {
        return str.replace ("'", "\\'").replace ("\"", "\\\"");
    }

    //==============================================================================
    void writeStringsXML (const File& folder) const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            auto& cfg = dynamic_cast<const AndroidBuildConfiguration&> (*config);

            String customStringsXmlContent ("<resources>\n");
            customStringsXmlContent << "<string name=\"app_name\">" << escapeQuotes (projectName) << "</string>\n";
            customStringsXmlContent << cfg.getCustomStringsXml();
            customStringsXmlContent << "\n</resources>";

            if (auto strings = parseXML (customStringsXmlContent))
            {
                String dir     = cfg.isDebug() ? "debug" : "release";
                String subPath = "app/src/" + dir + "/res/values/string.xml";

                writeXmlOrThrow (*strings, folder.getChildFile (subPath), "utf-8", 100, true);
            }
            else
            {
                jassertfalse; // needs handling?
            }
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

            build_tools::writeStreamToFile (file, [&] (MemoryOutputStream& mo)
            {
                mo.setNewLineString (getNewLineString());

                PNGImageFormat png;

                if (! png.writeImageToStream (im, mo))
                    throw build_tools::SaveError ("Can't generate Android icon file");
            });
        }
    }

    void writeIcons (const File& folder) const
    {
        const auto icons = getIcons();

        if (icons.big != nullptr && icons.small != nullptr)
        {
            auto step = jmax (icons.big->getWidth(), icons.big->getHeight()) / 8;
            writeIcon (folder.getChildFile ("drawable-xhdpi/icon.png"), build_tools::getBestIconForSize (icons, step * 8, false));
            writeIcon (folder.getChildFile ("drawable-hdpi/icon.png"),  build_tools::getBestIconForSize (icons, step * 6, false));
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"),  build_tools::getBestIconForSize (icons, step * 4, false));
            writeIcon (folder.getChildFile ("drawable-ldpi/icon.png"),  build_tools::getBestIconForSize (icons, step * 3, false));
        }
        else if (auto* icon = (icons.big != nullptr ? icons.big.get() : icons.small.get()))
        {
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"), build_tools::rescaleImageForIcon (*icon, icon->getWidth()));
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
    void addCompileUnits (const Project::Item& projectItem, MemoryOutputStream& mo,
                          Array<build_tools::RelativePath>& excludeFromBuild, Array<std::pair<build_tools::RelativePath, String>>& extraCompilerFlags) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addCompileUnits (projectItem.getChild (i), mo, excludeFromBuild, extraCompilerFlags);
        }
        else if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (*this))
        {
            auto f = projectItem.getFile();
            build_tools::RelativePath file (f, getTargetFolder().getChildFile ("app"), build_tools::RelativePath::buildTargetFolder);

            auto targetType = getProject().getTargetTypeFromFilePath (f, true);

            mo << "    \"" << file.toUnixStyle() << "\"" << newLine;

            if ((! projectItem.shouldBeCompiled()) || (! shouldFileBeCompiledByDefault (f))
                || (getProject().isAudioPluginProject()
                    && targetType != build_tools::ProjectType::Target::SharedCodeTarget
                    && targetType != build_tools::ProjectType::Target::StandalonePlugIn))
            {
                excludeFromBuild.add (file);
            }
            else
            {
                auto extraFlags = compilerFlagSchemesMap[projectItem.getCompilerFlagSchemeString()].get().toString();

                if (extraFlags.isNotEmpty())
                    extraCompilerFlags.add ({ file, extraFlags });
            }
        }
    }

    void addCompileUnits (MemoryOutputStream& mo, Array<build_tools::RelativePath>& excludeFromBuild,
                          Array<std::pair<build_tools::RelativePath, String>>& extraCompilerFlags) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
            addCompileUnits (getAllGroups().getReference(i), mo, excludeFromBuild, extraCompilerFlags);
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

        auto cppStandard = [this]
        {
            auto projectStandard = project.getCppStandardString();

            if (projectStandard == "latest")
                return project.getLatestNumberedCppStandardString();

            return projectStandard;
        }();

        cmakeArgs.add ("\"-DCMAKE_CXX_STANDARD=" + cppStandard + "\"");
        cmakeArgs.add ("\"-DCMAKE_CXX_EXTENSIONS=" + String (shouldUseGNUExtensions() ? "ON" : "OFF") + "\"");

        return cmakeArgs;
    }

    //==============================================================================
    StringArray getAndroidCompilerFlags() const
    {
        StringArray cFlags;
        cFlags.add ("\"-fsigned-char\"");

        return cFlags;
    }

    StringArray getProjectCompilerFlags() const
    {
        auto cFlags = getAndroidCompilerFlags();
        cFlags.addArray (getEscapedFlags (StringArray::fromTokens (getExtraCompilerFlagsString(), true)));
        return cFlags;
    }

    //==============================================================================
    StringPairArray getAndroidPreprocessorDefs() const
    {
        StringPairArray defines;

        defines.set ("JUCE_ANDROID", "1");
        defines.set ("JUCE_ANDROID_API_VERSION", androidMinimumSDK.get());

        if (arePushNotificationsEnabled())
        {
            defines.set ("JUCE_PUSH_NOTIFICATIONS", "1");
            defines.set ("JUCE_PUSH_NOTIFICATIONS_ACTIVITY", getJNIActivityClassName().quoted());
        }

        if (isInAppBillingEnabled())
            defines.set ("JUCE_IN_APP_PURCHASES", "1");

        if (isContentSharingEnabled())
            defines.set ("JUCE_CONTENT_SHARING", "1");

        if (supportsGLv3())
            defines.set ("JUCE_ANDROID_GL_ES_VERSION_3_0", "1");

        if (areRemoteNotificationsEnabled())
        {
            defines.set ("JUCE_FIREBASE_INSTANCE_ID_SERVICE_CLASSNAME", "com_rmsl_juce_JuceFirebaseInstanceIdService");
            defines.set ("JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME", "com_rmsl_juce_JuceFirebaseMessagingService");
        }

        return defines;
    }

    StringPairArray getProjectPreprocessorDefs() const
    {
        auto defines = getAndroidPreprocessorDefs();

        return mergePreprocessorDefs (defines, getAllPreprocessorDefs());
    }

    StringPairArray getConfigPreprocessorDefs (const BuildConfiguration& config) const
    {
        auto cfgDefines = getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified);

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
    StringArray getUserLibraries() const
    {
        auto userLibraries = StringArray::fromTokens (getExternalLibrariesString(), ";", "");
        userLibraries = getCleanedStringArray (userLibraries);

        const auto ppDefs = getAllPreprocessorDefs();

        for (auto& lib : userLibraries)
            lib = build_tools::replacePreprocessorDefs (ppDefs, lib);

        userLibraries.addArray (androidLibs);
        return userLibraries;
    }

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
            build_tools::RelativePath (path, build_tools::RelativePath::buildTargetFolder)
                .rebased (getTargetFolder(), getTargetFolder().getChildFile ("app"), build_tools::RelativePath::buildTargetFolder);

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
            auto escaped = "[[-D" + defs.getAllKeys()[i];
            auto value = defs.getAllValues()[i];

            if (value.isNotEmpty())
                escaped += ("=" + value);

            escapedDefs.add (escaped + "]]");
        }

        return escapedDefs;
    }

    static StringArray getEscapedFlags (const StringArray& flags)
    {
        StringArray escaped;

        for (auto& flag : flags)
            escaped.add ("[[" + flag + "]]");

        return escaped;
    }

    //==============================================================================
    std::unique_ptr<XmlElement> createManifestXML() const
    {
        auto manifest = createManifestElement();

        createSupportsScreensElement (*manifest);
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

    std::unique_ptr<XmlElement> createManifestElement() const
    {
        auto manifest = parseXML (androidManifestCustomXmlElements.get());

        if (manifest == nullptr)
            manifest = std::make_unique<XmlElement> ("manifest");

        setAttributeIfNotPresent (*manifest, "xmlns:android", "http://schemas.android.com/apk/res/android");
        setAttributeIfNotPresent (*manifest, "android:versionCode", androidVersionCode.get());
        setAttributeIfNotPresent (*manifest, "android:versionName",  project.getVersionString());
        setAttributeIfNotPresent (*manifest, "package", project.getBundleIdentifierString().toLowerCase());

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
                screens->setAttribute ("android:xlargeScreens", "true");
            }
        }
    }

    void createPermissionElements (XmlElement& manifest) const
    {
        auto permissions = getPermissionsRequired();

        for (auto* child : manifest.getChildWithTagNameIterator ("uses-permission"))
        {
            permissions.removeString (child->getStringAttribute ("android:name"), false);
        }

        for (int i = permissions.size(); --i >= 0;)
        {
            const auto permission = permissions[i];
            auto* usesPermission = manifest.createNewChildElement ("uses-permission");
            usesPermission->setAttribute ("android:name", permission);

            // This permission only has an effect on SDK version 28 and lower
            if (permission == "android.permission.WRITE_EXTERNAL_STORAGE")
                usesPermission->setAttribute ("android:maxSdkVersion", "28");
        }
    }

    void createOpenGlFeatureElement (XmlElement& manifest) const
    {
        if (project.getEnabledModules().isModuleEnabled ("juce_opengl"))
        {
            XmlElement* glVersion = nullptr;

            for (auto* child : manifest.getChildWithTagNameIterator ("uses-feature"))
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
        setAttributeIfNotPresent (*app, "android:name", getApplicationClassString());

        if (androidTheme.get().toString().isNotEmpty())
            setAttributeIfNotPresent (*app, "android:theme", androidTheme.get());

        if (! app->hasAttribute ("android:icon"))
        {
            std::unique_ptr<Drawable> bigIcon (getBigIcon()), smallIcon (getSmallIcon());

            if (bigIcon != nullptr || smallIcon != nullptr)
                app->setAttribute ("android:icon", "@drawable/icon");
        }

        if (! app->hasAttribute ("android:hardwareAccelerated"))
            app->setAttribute ("android:hardwareAccelerated", "false"); // (using the 2D acceleration slows down openGL)

        return app;
    }

    XmlElement* createActivityElement (XmlElement& application) const
    {
        auto* act = getOrCreateChildWithName (application, "activity");

        setAttributeIfNotPresent (*act, "android:name", getActivityClassString());
        setAttributeIfNotPresent (*act, "android:label", "@string/app_name");

        if (! act->hasAttribute ("android:configChanges"))
            act->setAttribute ("android:configChanges", "keyboardHidden|orientation|screenSize");

        if (androidScreenOrientation.get() == "landscape")
        {
            setAttributeIfNotPresent (*act, "android:screenOrientation",
                                      static_cast<int> (androidMinimumSDK.get()) < 18 ? "sensorLandscape" : "userLandscape");
        }
        else
        {
            setAttributeIfNotPresent (*act, "android:screenOrientation", androidScreenOrientation.get());
        }

        setAttributeIfNotPresent (*act, "android:launchMode", "singleTask");

        // Using the 2D acceleration slows down OpenGL. We *do* enable it here for the activity though, and we disable it
        // in each ComponentPeerView instead. This way any embedded native views, which are not children of ComponentPeerView,
        // can still use hardware acceleration if needed (e.g. web view).
        if (! act->hasAttribute ("android:hardwareAccelerated"))
            act->setAttribute ("android:hardwareAccelerated", "true"); // (using the 2D acceleration slows down openGL)

        act->setAttribute ("android:exported", "true");

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
        if (areRemoteNotificationsEnabled())
        {
            auto* service = application.createNewChildElement ("service");
            service->setAttribute ("android:name", "com.rmsl.juce.JuceFirebaseMessagingService");
            auto* intentFilter = service->createNewChildElement ("intent-filter");
            intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.MESSAGING_EVENT");

            service = application.createNewChildElement ("service");
            service->setAttribute ("android:name", "com.rmsl.juce.JuceFirebaseInstanceIdService");
            intentFilter = service->createNewChildElement ("intent-filter");
            intentFilter->createNewChildElement ("action")->setAttribute ("android:name", "com.google.firebase.INSTANCE_ID_EVENT");

            auto* metaData = application.createNewChildElement ("meta-data");
            metaData->setAttribute ("android:name", "firebase_analytics_collection_deactivated");
            metaData->setAttribute ("android:value", "true");
        }
    }

    void createProviderElement (XmlElement& application) const
    {
        if (isContentSharingEnabled())
        {
            auto* provider = application.createNewChildElement ("provider");

            provider->setAttribute ("android:name", "com.rmsl.juce.JuceSharingContentProvider");
            provider->setAttribute ("android:authorities", project.getBundleIdentifierString().toLowerCase() + ".sharingcontentprovider");
            provider->setAttribute ("android:grantUriPermissions", "true");
            provider->setAttribute ("android:exported", "true");
        }
    }

    static XmlElement* getOrCreateChildWithName (XmlElement& element, const String& childName)
    {
        auto* child = element.getChildByName (childName);

        if (child == nullptr)
            child = element.createNewChildElement (childName);

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
        {
            s.add ("android.permission.INTERNET");
            s.add ("android.permission.CHANGE_WIFI_MULTICAST_STATE");
        }

        if (androidMicNeeded.get())
            s.add ("android.permission.RECORD_AUDIO");

        if (androidCameraNeeded.get())
            s.add ("android.permission.CAMERA");

        if (androidBluetoothNeeded.get())
        {
            s.add ("android.permission.BLUETOOTH");
            s.add ("android.permission.BLUETOOTH_ADMIN");
            s.add ("android.permission.ACCESS_FINE_LOCATION");
            s.add ("android.permission.ACCESS_COARSE_LOCATION");
        }

        if (androidExternalReadPermission.get())
            s.add ("android.permission.READ_EXTERNAL_STORAGE");

        if (androidExternalWritePermission.get())
            s.add ("android.permission.WRITE_EXTERNAL_STORAGE");

        if (isInAppBillingEnabled())
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
    const File AndroidExecutable;

    JUCE_DECLARE_NON_COPYABLE (AndroidProjectExporter)
};
