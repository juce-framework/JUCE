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

class AndroidStudioProjectExporter  : public AndroidProjectExporterBase
{
public:
    //==============================================================================
    bool usesMMFiles() const override                    { return false; }
    bool canCopeWithDuplicateFiles() override            { return false; }
    bool supportsUserDefinedConfigurations() const override { return false; }

    bool isAndroidStudio() const override                { return true; }
    bool isAndroidAnt() const override                   { return false; }

    static const char* getName()                         { return "Android Studio"; }
    static const char* getValueTreeTypeName()            { return "ANDROIDSTUDIO"; }

    static AndroidStudioProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new AndroidStudioProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    CachedValue<String> gradleVersion, gradleWrapperVersion, gradleToolchain, buildToolsVersion;

    //==============================================================================
    AndroidStudioProjectExporter (Project& p, const ValueTree& t)
        : AndroidProjectExporterBase (p, t),
          gradleVersion (settings, Ids::gradleVersion, nullptr, "2.14.1"),
          gradleWrapperVersion (settings, Ids::gradleWrapperVersion, nullptr, "0.8.1"),
          gradleToolchain (settings, Ids::gradleToolchain, nullptr, "clang"),
          buildToolsVersion (settings, Ids::buildToolsVersion, nullptr, "23.0.2"),
          androidStudioExecutable (findAndroidStudioExecutable())
    {
        name = getName();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "AndroidStudio";
    }

    //==============================================================================
    void createToolchainExporterProperties (PropertyListBuilder& props) override
    {
        props.add (new TextWithDefaultPropertyComponent<String> (gradleVersion, "gradle version", 32),
                   "The version of gradle that Android Studio should use to build this app");

        props.add (new TextWithDefaultPropertyComponent<String> (gradleWrapperVersion, "gradle-experimental wrapper version", 32),
                   "The version of the gradle-experimental wrapper that Android Studio should use to build this app");

        static const char* toolchains[] = { "clang", "gcc", nullptr };

        props.add (new ChoicePropertyComponent (gradleToolchain.getPropertyAsValue(), "NDK Toolchain", StringArray (toolchains), Array<var> (toolchains)),
                   "The toolchain that gradle should invoke for NDK compilation (variable model.android.ndk.tooclhain in app/build.gradle)");

        props.add (new TextWithDefaultPropertyComponent<String> (buildToolsVersion, "Android build tools version", 32),
                   "The Android build tools version that Android Studio should use to build this app");
    }

    void createLibraryModuleExporterProperties (PropertyListBuilder&) override
    {
        // gradle cannot do native library modules as far as we know...
    }

    //==============================================================================
    bool canLaunchProject() override
    {
        return androidStudioExecutable.exists();
    }

    bool launchProject() override
    {
        if (! androidStudioExecutable.exists())
        {
            jassertfalse;
            return false;
        }

        const File targetFolder (getTargetFolder());

        // we have to surround the path with extra quotes, otherwise Android Studio
        // will choke if there are any space characters in the path.
        return androidStudioExecutable.startAsProcess ("\"" + targetFolder.getFullPathName() + "\"");
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        const File targetFolder (getTargetFolder());

        removeOldFiles (targetFolder);

        {
            const String package (getActivityClassPackage());
            const String path (package.replaceCharacter ('.', File::separator));
            const File javaTarget (targetFolder.getChildFile ("app/src/main/java").getChildFile (path));

            copyActivityJavaFiles (modules, javaTarget, package);
        }

        writeFile (targetFolder, "settings.gradle",  getSettingsGradleFileContent());
        writeFile (targetFolder, "build.gradle",     getProjectBuildGradleFileContent());
        writeFile (targetFolder, "app/build.gradle", getAppBuildGradleFileContent());
        writeFile (targetFolder, "local.properties", getLocalPropertiesFileContent());
        writeFile (targetFolder, "gradle/wrapper/gradle-wrapper.properties", getGradleWrapperPropertiesFileContent());

        writeBinaryFile (targetFolder, "gradle/wrapper/LICENSE-for-gradlewrapper.txt", BinaryData::LICENSE, BinaryData::LICENSESize);
        writeBinaryFile (targetFolder, "gradle/wrapper/gradle-wrapper.jar", BinaryData::gradlewrapper_jar, BinaryData::gradlewrapper_jarSize);
        writeBinaryFile (targetFolder, "gradlew",                           BinaryData::gradlew,           BinaryData::gradlewSize);
        writeBinaryFile (targetFolder, "gradlew.bat",                       BinaryData::gradlew_bat,       BinaryData::gradlew_batSize);

        targetFolder.getChildFile ("gradlew").setExecutePermission (true);

        writeAndroidManifest (targetFolder);
        writeStringsXML      (targetFolder);
        writeAppIcons        (targetFolder);
        createSourceSymlinks (targetFolder);
    }

    void removeOldFiles (const File& targetFolder) const
    {
        targetFolder.getChildFile ("app/src").deleteRecursively();
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
    static File findAndroidStudioExecutable()
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

        return File();
    }

protected:
    //==============================================================================
    class AndroidStudioBuildConfiguration  : public BuildConfiguration
    {
    public:
        AndroidStudioBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
            if (getArchitectures().isEmpty())
            {
                if (isDebug())
                    getArchitecturesValue() = "armeabi x86";
                else
                    getArchitecturesValue() = "armeabi armeabi-v7a x86";
            }
        }

        Value getArchitecturesValue()           { return getValue (Ids::androidArchitectures); }
        String getArchitectures() const         { return config [Ids::androidArchitectures]; }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            props.add (new TextPropertyComponent (getArchitecturesValue(), "Architectures", 256, false),
                       "A list of the ARM architectures to build (for a fat binary).");
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new AndroidStudioBuildConfiguration (project, v, *this);
    }

private:
    static void createSymlinkAndParentFolders (const File& originalFile, const File& linkFile)
    {
        {
            const File linkFileParentDirectory (linkFile.getParentDirectory());

            // this will recursively creative the parent directories for the file.
            // without this, the symlink would fail because it doesn't automatically create
            // the folders if they don't exist
            if (! linkFileParentDirectory.createDirectory())
                throw SaveError (String ("Could not create directory ") + linkFileParentDirectory.getFullPathName());
        }

        if (! originalFile.createSymbolicLink (linkFile, true))
            throw SaveError (String ("Failed to create symlink from ")
                              + linkFile.getFullPathName() + " to "
                              + originalFile.getFullPathName() + "!");
    }

    void makeSymlinksForGroup (const Project::Item& group, const File& targetFolder) const
    {
        if (! group.isGroup())
        {
            throw SaveError ("makeSymlinksForGroup was called with something other than a group!");
        }

        for (int i = 0; i < group.getNumChildren(); ++i)
        {
            const Project::Item& projectItem = group.getChild (i);

            if (projectItem.isGroup())
            {
                makeSymlinksForGroup (projectItem, targetFolder.getChildFile (projectItem.getName()));
            }
            else if (projectItem.shouldBeAddedToTargetProject()) // must be a file then
            {
                const File originalFile (projectItem.getFile());
                const File targetFile (targetFolder.getChildFile (originalFile.getFileName()));

                createSymlinkAndParentFolders (originalFile, targetFile);
            }
        }
    }

    void createSourceSymlinks (const File& folder) const
    {
        const File targetFolder (folder.getChildFile ("app/src/main/jni"));

        // here we make symlinks to only to files included in the groups inside the project
        // this is because Android Studio does not have a concept of groups and just uses
        // the file system layout to determine what's to be compiled
        {
            const Array<Project::Item>& groups = getAllGroups();

            for (int i = 0; i < groups.size(); ++i)
            {
                const Project::Item projectItem (groups.getReference (i));
                const String projectItemName (projectItem.getName());

                if (projectItem.isGroup())
                    makeSymlinksForGroup (projectItem, projectItemName == "Juce Modules" ? targetFolder.getChildFile ("JuceModules") : targetFolder);
            }
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
    struct GradleElement
    {
        virtual ~GradleElement() {}
        String toString() const    { return toStringIndented (0); }
        virtual String toStringIndented (int indentLevel) const = 0;

        static String indent (int indentLevel)    { return String::repeatedString ("    ", indentLevel); }
    };

    //==============================================================================
    struct GradleStatement : public GradleElement
    {
        GradleStatement (const String& s)  : statement (s) {}
        String toStringIndented (int indentLevel) const override    { return indent (indentLevel) + statement; }

        String statement;
    };

    //==============================================================================
    struct GradleCppFlag  : public GradleStatement
    {
        GradleCppFlag (const String& flag)
            : GradleStatement ("cppFlags.add(" + flag.quoted() + ")") {}
    };

    struct GradlePreprocessorDefine  : public GradleStatement
    {
        GradlePreprocessorDefine (const String& define, const String& value)
            : GradleStatement ("cppFlags.add(\"-D" + define + "=" + value + "\")") {}
    };

    struct GradleHeaderIncludePath  : public GradleStatement
    {
        GradleHeaderIncludePath (const String& path)
            : GradleStatement ("cppFlags.add(\"-I${project.rootDir}/" + sanitisePath (path) + "\".toString())") {}
    };

    struct GradleLibrarySearchPath  : public GradleStatement
    {
        GradleLibrarySearchPath (const String& path)
            : GradleStatement ("cppFlags.add(\"-L" + sanitisePath (path) + "\".toString())") {}
    };

    struct GradleLinkerFlag  : public GradleStatement
    {
        GradleLinkerFlag (const String& flag)
            : GradleStatement ("ldFlags.add(" + flag.quoted() + "\")") {}
    };

    struct GradleLinkLibrary  : public GradleStatement
    {
        GradleLinkLibrary (const String& lib)
            : GradleStatement ("ldLibs.add(" + lib.quoted() + ")") {}
    };

    //==============================================================================
    struct GradleValue : public GradleElement
    {
        template <typename ValueType>
        GradleValue (const String& k, const ValueType& v)
            : key (k), value (v) {}

        GradleValue (const String& k, bool boolValue)
            : key (k), value (boolValue ? "true" : "false") {}

        String toStringIndented (int indentLevel) const override
        {
            return indent (indentLevel) + key + " = " + value;
        }

    protected:
        String key, value;
    };

    struct GradleString : public GradleValue
    {
        GradleString (const String& k, const String& str)
            : GradleValue (k, str.quoted())
        {
            if (str.containsAnyOf ("${\"\'"))
                value += ".toString()";
        }
    };

    struct GradleFilePath : public GradleValue
    {
        GradleFilePath (const String& k, const String& path)
            : GradleValue (k, "new File(\"" + sanitisePath (path) + "\")") {}
    };

    //==============================================================================
    struct GradleObject  : public GradleElement
    {
        GradleObject (const String& nm) : name (nm) {}

       #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES
        template <typename GradleType, typename... Args>
        void add (Args... args)
        {
            children.add (new GradleType (args...));
            // Note: can't use std::forward because it doesn't compile for OS X 10.8
        }
       #else // Remove this workaround once we drop VS2012 support!
        template <typename GradleType, typename Arg1>
        void add (Arg1 arg1)
        {
            children.add (new GradleType (arg1));
        }

        template <typename GradleType, typename Arg1, typename Arg2>
        void add (Arg1 arg1, Arg2 arg2)
        {
            children.add (new GradleType (arg1, arg2));
        }
       #endif

        void addChildObject (GradleObject* objectToAdd) noexcept
        {
            children.add (objectToAdd);
        }

        String toStringIndented (int indentLevel) const override
        {
            String result;
            result << indent (indentLevel) << name << " {" << newLine;

            for (const auto& child : children)
                result << child->toStringIndented (indentLevel + 1) << newLine;

            result << indent (indentLevel) << "}";

            if (indentLevel == 0)
                result << newLine;

            return result;
        }

    private:
        String name;
        OwnedArray<GradleElement> children;
    };

    //==============================================================================
    String getSettingsGradleFileContent() const
    {
        return "include ':app'";
    }

    String getProjectBuildGradleFileContent() const
    {
        String projectBuildGradle;
        projectBuildGradle << getGradleBuildScript();
        projectBuildGradle << getGradleAllProjects();

        return projectBuildGradle;
    }

    //==============================================================================
    String getGradleBuildScript() const
    {
        GradleObject buildScript ("buildscript");

        buildScript.addChildObject (getGradleRepositories());
        buildScript.addChildObject (getGradleDependencies());

        return buildScript.toString();
    }

    GradleObject* getGradleRepositories() const
    {
        auto repositories = new GradleObject ("repositories");
        repositories->add<GradleStatement> ("jcenter()");
        return repositories;
    }

    GradleObject* getGradleDependencies() const
    {
        auto dependencies = new GradleObject ("dependencies");

        dependencies->add<GradleStatement> ("classpath 'com.android.tools.build:gradle-experimental:"
                                            + gradleWrapperVersion.get() + "'");
        return dependencies;
    }

    String getGradleAllProjects() const
    {
        GradleObject allProjects ("allprojects");
        allProjects.addChildObject (getGradleRepositories());
        return allProjects.toString();
    }

    //==============================================================================
    String getAppBuildGradleFileContent() const
    {
        String appBuildGradle;

        appBuildGradle << "apply plugin: 'com.android.model.application'" << newLine;
        appBuildGradle << getAndroidModel();
        appBuildGradle << getAppDependencies();

        return appBuildGradle;
    }

    String getAndroidModel() const
    {
        GradleObject model ("model");

        model.addChildObject (getAndroidObject());
        model.addChildObject (getAndroidNdkSettings());
        model.addChildObject (getAndroidSources());
        model.addChildObject (getAndroidBuildConfigs());
        model.addChildObject (getAndroidSigningConfigs());
        model.addChildObject (getAndroidProductFlavours());

        return model.toString();
    }

    String getAppDependencies() const
    {
        GradleObject dependencies ("dependencies");
        dependencies.add<GradleStatement> ("compile \"com.android.support:support-v4:+\"");
        return dependencies.toString();
    }

    //==============================================================================
    GradleObject* getAndroidObject() const
    {
        auto android = new GradleObject ("android");

        android->add<GradleValue> ("compileSdkVersion", androidMinimumSDK.get().getIntValue());
        android->add<GradleString> ("buildToolsVersion", buildToolsVersion.get());
        android->addChildObject (getAndroidDefaultConfig());

        return android;
    }

    GradleObject* getAndroidDefaultConfig() const
    {
        const String bundleIdentifier  = project.getBundleIdentifier().toString().toLowerCase();
        const int minSdkVersion = androidMinimumSDK.get().getIntValue();

        auto defaultConfig = new GradleObject ("defaultConfig.with");

        defaultConfig->add<GradleString> ("applicationId",             bundleIdentifier);
        defaultConfig->add<GradleValue>  ("minSdkVersion.apiLevel",      minSdkVersion);
        defaultConfig->add<GradleValue>  ("targetSdkVersion.apiLevel", minSdkVersion);

        return defaultConfig;
    }

    GradleObject* getAndroidNdkSettings() const
    {
        const String toolchain = gradleToolchain.get();
        const bool isClang = (toolchain == "clang");

        auto ndkSettings = new GradleObject ("android.ndk");

        ndkSettings->add<GradleString> ("moduleName",       "juce_jni");
        ndkSettings->add<GradleString> ("toolchain",        toolchain);
        ndkSettings->add<GradleString> ("stl", isClang ? "c++_static" :  "gnustl_static");

        addAllNdkCompilerSettings (ndkSettings);

        return ndkSettings;
    }

    void addAllNdkCompilerSettings (GradleObject* ndk) const
    {
        addNdkCppFlags (ndk);
        addNdkPreprocessorDefines (ndk);
        addNdkHeaderIncludePaths (ndk);
        addNdkLinkerFlags (ndk);
        addNdkLibraries (ndk);
    }

    void addNdkCppFlags (GradleObject* ndk) const
    {
        const char* alwaysUsedFlags[] = { "-fsigned-char", "-fexceptions", "-frtti", "-std=c++11", nullptr };
        StringArray cppFlags (alwaysUsedFlags);

        cppFlags.mergeArray (StringArray::fromTokens (getExtraCompilerFlagsString(), " ", ""));

        for (int i = 0; i < cppFlags.size(); ++i)
            ndk->add<GradleCppFlag> (cppFlags[i]);
    }

    void addNdkPreprocessorDefines (GradleObject* ndk) const
    {
        const auto& defines = getAllPreprocessorDefs();

        for (int i = 0; i < defines.size(); ++i)
            ndk->add<GradlePreprocessorDefine> ( defines.getAllKeys()[i], defines.getAllValues()[i]);
    }

    void addNdkHeaderIncludePaths (GradleObject* ndk) const
    {
        StringArray includePaths;

        for (const auto& cppFile : getAllCppFilesToBeIncludedWithPath())
            includePaths.addIfNotAlreadyThere (cppFile.getParentDirectory().toUnixStyle());

        for (const auto& path : includePaths)
            ndk->add<GradleHeaderIncludePath> (path);
    }

    Array<RelativePath> getAllCppFilesToBeIncludedWithPath() const
    {
        Array<RelativePath> cppFiles;

        struct NeedsToBeIncludedWithPathPredicate
        {
            bool operator() (const Project::Item& projectItem) const
            {
                return projectItem.shouldBeAddedToTargetProject() && ! projectItem.isModuleCode();
            }
        };

        for (const auto& group : getAllGroups())
            findAllProjectItemsWithPredicate (group, cppFiles, NeedsToBeIncludedWithPathPredicate());

        return cppFiles;
    }

    void addNdkLinkerFlags (GradleObject* ndk) const
    {
        const auto linkerFlags = StringArray::fromTokens (getExtraLinkerFlagsString(), " ", "");

        for (const auto& flag : linkerFlags)
            ndk->add<GradleLinkerFlag> (flag);

    }
    void addNdkLibraries (GradleObject* ndk) const
    {
        const char* requiredAndroidLibs[] = { "android", "EGL", "GLESv2", "log", nullptr };
        StringArray libs (requiredAndroidLibs);

        libs.addArray (StringArray::fromTokens(getExternalLibrariesString(), ";", ""));

        for (const auto& lib : libs)
            ndk->add<GradleLinkLibrary> (lib);
    }

    GradleObject* getAndroidSources() const
    {
        auto source = new GradleObject ("source");   // app source folder
        source->add<GradleStatement> ("exclude \"**/JuceModules/\"");

        auto jni = new GradleObject ("jni");         // all C++ sources for app
        jni->addChildObject (source);

        auto main = new GradleObject ("main");       // all sources for app
        main->addChildObject (jni);

        auto sources = new GradleObject ("android.sources"); // all sources
        sources->addChildObject (main);
        return sources;
    }

    GradleObject* getAndroidBuildConfigs() const
    {
        auto buildConfigs = new GradleObject ("android.buildTypes");

        for (ConstConfigIterator config (*this); config.next();)
            buildConfigs->addChildObject (getBuildConfig (*config));

        return buildConfigs;
    }

    GradleObject* getBuildConfig (const BuildConfiguration& config) const
    {
        const String configName (config.getName());

        // Note: at the moment, Android Studio only supports a "debug" and a "release"
        // build config, but no custom build configs like Projucer's other exporters do.
        if (configName != "Debug" && configName != "Release")
            throw SaveError ("Build configurations other than Debug and Release are not yet support for Android Studio");

        auto gradleConfig = new GradleObject (configName.toLowerCase());

        if (! config.isDebug())
            gradleConfig->add<GradleValue> ("signingConfig", "$(\"android.signingConfigs.releaseConfig\")");

        addConfigNdkSettings (gradleConfig, config);

        return gradleConfig;
    }

    void addConfigNdkSettings (GradleObject* buildConfig, const BuildConfiguration& config) const
    {
        auto ndkSettings = new GradleObject ("ndk.with");

        if (config.isDebug())
        {
            ndkSettings->add<GradleValue> ("debuggable", true);
            ndkSettings->add<GradleCppFlag> ("-g");
            ndkSettings->add<GradlePreprocessorDefine> ("DEBUG", "1");
            ndkSettings->add<GradlePreprocessorDefine> ("_DEBUG", "1");
        }
        else
        {
            ndkSettings->add<GradlePreprocessorDefine> ("NDEBUG", "1");
        }

        ndkSettings->add<GradleCppFlag> ("-O" + config.getGCCOptimisationFlag());

        for (const auto& path : getHeaderSearchPaths (config))
            ndkSettings->add<GradleHeaderIncludePath> (path);

        for (const auto& path : config.getLibrarySearchPaths())
            ndkSettings->add<GradleLibrarySearchPath> (path);

        ndkSettings->add<GradlePreprocessorDefine> ("JUCE_ANDROID", "1");
        ndkSettings->add<GradlePreprocessorDefine> ("JUCE_ANDROID_API_VERSION", androidMinimumSDK.get());
        ndkSettings->add<GradlePreprocessorDefine> ("JUCE_ANDROID_ACTIVITY_CLASSNAME", getJNIActivityClassName().replaceCharacter ('/', '_'));
        ndkSettings->add<GradlePreprocessorDefine> ("JUCE_ANDROID_ACTIVITY_CLASSPATH","\\\"" + androidActivityClass.get().replaceCharacter('.', '/') + "\\\"");

        const auto defines = config.getAllPreprocessorDefs();
        for (int i = 0; i < defines.size(); ++i)
            ndkSettings->add<GradlePreprocessorDefine> (defines.getAllKeys()[i], defines.getAllValues()[i]);

        buildConfig->addChildObject (ndkSettings);
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray paths (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());
        paths = getCleanedStringArray (paths);
        return paths;
    }

    GradleObject* getAndroidSigningConfigs() const
    {
        auto releaseConfig = new GradleObject ("create(\"releaseConfig\")");

        releaseConfig->add<GradleFilePath>  ("storeFile",     androidKeyStore.get());
        releaseConfig->add<GradleString>    ("storePassword", androidKeyStorePass.get());
        releaseConfig->add<GradleString>    ("keyAlias",      androidKeyAlias.get());
        releaseConfig->add<GradleString>    ("keyPassword",   androidKeyAliasPass.get());
        releaseConfig->add<GradleString>    ("storeType",     "jks");

        auto signingConfigs = new GradleObject ("android.signingConfigs");

        signingConfigs->addChildObject (releaseConfig);
        // Note: no need to add a debugConfig, Android Studio will use debug.keystore by default

        return signingConfigs;
    }

    GradleObject* getAndroidProductFlavours() const
    {
        auto flavours = new GradleObject ("android.productFlavors");

        StringArray architectures (StringArray::fromTokens (getABIs<AndroidStudioBuildConfiguration> (true),  " ", ""));
        architectures.mergeArray  (StringArray::fromTokens (getABIs<AndroidStudioBuildConfiguration> (false), " ", ""));

        if (architectures.size() == 0)
            throw SaveError ("Can't build for no architectures!");

        for (int i = 0; i < architectures.size(); ++i)
        {
            String arch (architectures[i].trim());

            if ((arch).isEmpty())
                continue;

            flavours->addChildObject (getGradleProductFlavourForArch (arch));
        }

        return flavours;
    }

    GradleObject* getGradleProductFlavourForArch (const String& arch) const
    {
        auto flavour = new GradleObject ("create(\"" + arch + "\")");
        flavour->add<GradleStatement> ("ndk.abiFilters.add(\"" + arch + "\")");
        return flavour;
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
    void writeStringsXML (const File& folder) const
    {
        XmlElement strings ("resources");
        XmlElement* resourceName = strings.createNewChildElement ("string");

        resourceName->setAttribute ("name", "app_name");
        resourceName->addTextElement (projectName);

        writeXmlOrThrow (strings, folder.getChildFile ("app/src/main/res/values/string.xml"), "utf-8", 100, true);
    }

    //==============================================================================
    void writeAndroidManifest (const File& folder) const
    {
        ScopedPointer<XmlElement> manifest (createManifestXML());

        writeXmlOrThrow (*manifest, folder.getChildFile ("app/src/main/AndroidManifest.xml"), "utf-8", 100, true);
    }

    //==============================================================================
    const File androidStudioExecutable;

    JUCE_DECLARE_NON_COPYABLE (AndroidStudioProjectExporter)
};
