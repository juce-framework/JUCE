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
    static const char* getName()                { return "Android Studio"; }
    static const char* getValueTreeTypeName()   { return "ANDROIDSTUDIO"; }

    static AndroidStudioProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new AndroidStudioProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    AndroidStudioProjectExporter (Project& p, const ValueTree& t)
        : AndroidProjectExporterBase (p, t),
          androidStudioExecutable (findAndroidStudioExecutable())
    {
        name = getName();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "AndroidStudio";
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

        return androidStudioExecutable.startAsProcess (targetFolder.getFullPathName());
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        AndroidProjectExporterBase::createExporterProperties (props);
    }

    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        const File targetFolder (getTargetFolder());
        targetFolder.deleteRecursively();

        {
            const String package (getActivityClassPackage());
            const String path (package.replaceCharacter ('.', File::separator));
            const File javaTarget (targetFolder.getChildFile ("app/src/main/java").getChildFile (path));

            copyActivityJavaFiles (modules, javaTarget, package);
        }

        writeSettingsDotGradle       (targetFolder);
        writeLocalDotProperties      (targetFolder);
        writeBuildDotGradleRoot      (targetFolder);
        writeBuildDotGradleApp       (targetFolder);
        writeGradleWrapperProperties (targetFolder);
        writeAndroidManifest         (targetFolder);
        writeStringsXML              (targetFolder);
        writeAppIcons                (targetFolder);

        createSourceSymlinks         (targetFolder);
    }

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

        return File::nonexistent;
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
    static void createSymboicLinkAndCreateParentFolders (const File& originalFile, const File& linkFile)
    {
        {
            const File linkFileParentDirectory (linkFile.getParentDirectory());

            // this will recursively creative the parent directories for the file
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

                createSymboicLinkAndCreateParentFolders (originalFile, targetFile);
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

    void writeSettingsDotGradle (const File& folder) const
    {
        MemoryOutputStream memoryOutputStream;

        memoryOutputStream << "include ':app'";

        overwriteFileIfDifferentOrThrow (folder.getChildFile ("settings.gradle"), memoryOutputStream);
    }

    static String sanitisePath (String path)
    {
        if (path.startsWith ("~"))
        {
            const String homeFolder (File::getSpecialLocation (File::SpecialLocationType::userHomeDirectory).getFullPathName());

            path = path.replaceSection (0, 1, homeFolder);
        }

        return path.replace ("\\", "\\\\");
    }

    void writeLocalDotProperties (const File& folder) const
    {
        MemoryOutputStream memoryOutputStream;

        memoryOutputStream << "ndk.dir=" << sanitisePath (getNDKPathString()) << newLine
                           << "sdk.dir=" << sanitisePath (getSDKPathString());

        overwriteFileIfDifferentOrThrow (folder.getChildFile ("local.properties"), memoryOutputStream);
    }

    void writeGradleWrapperProperties (const File& folder) const
    {
        MemoryOutputStream memoryOutputStream;

        memoryOutputStream << "distributionUrl=https\\://services.gradle.org/distributions/gradle-2.6-all.zip";

        overwriteFileIfDifferentOrThrow (folder.getChildFile ("gradle/wrapper/gradle-wrapper.properties"), memoryOutputStream);
    }

    void writeBuildDotGradleRoot (const File& folder) const
    {
        MemoryOutputStream memoryOutputStream;

        const String indent = getIndentationString();

        // this is needed to make sure the correct version of
        // the gradle build tools is available
        // otherwise, the user will get an error about
        // com.android.tools.something not being available
        memoryOutputStream << "buildscript {" << newLine
                           << indent << "repositories {" << newLine
                           << indent << indent << "jcenter()" << newLine
                           << indent << "}" << newLine
                           << indent << "dependencies {" << newLine
                           << indent << indent << "classpath 'com.android.tools.build:gradle-experimental:0.3.0-alpha7'" << newLine
                           << indent << "}" << newLine
                           << "}" << newLine
                           << newLine
                           << "allprojects {" << newLine
                           << indent << "repositories {" << newLine
                           << indent << indent << "jcenter()" << newLine
                           << indent << "}" << newLine
                           << "}";

        overwriteFileIfDifferentOrThrow (folder.getChildFile ("build.gradle"), memoryOutputStream);
    }

    void writeStringsXML (const File& folder) const
    {
        XmlElement strings ("resources");
        XmlElement* resourceName = strings.createNewChildElement ("string");

        resourceName->setAttribute ("name", "app_name");
        resourceName->addTextElement (projectName);

        writeXmlOrThrow (strings, folder.getChildFile ("app/src/main/res/values/string.xml"), "utf-8", 100, true);
    }

    void writeAndroidManifest (const File& folder) const
    {
        ScopedPointer<XmlElement> manifest (createManifestXML());

        writeXmlOrThrow (*manifest, folder.getChildFile ("app/src/main/AndroidManifest.xml"), "utf-8", 100, true);
    }

    String createModelDotAndroid (const String& indent, const String& minimumSDKVersion, const String& bundleIdentifier) const
    {
        String result;

        result << "android {" << newLine
               << indent << "compileSdkVersion = " << minimumSDKVersion << newLine
               << indent << "buildToolsVersion = \"" << "23.0.1" << "\"" << newLine
               << indent << "defaultConfig.with {" << newLine
               << indent << indent << "applicationId = \"" << bundleIdentifier.toLowerCase() << "\"" << newLine
               << indent << indent << "minSdkVersion.apiLevel = 11" << newLine
               << indent << indent << "targetSdkVersion.apiLevel = " << minimumSDKVersion << newLine
               << indent << "}" << newLine
               << "}" << newLine;

        return result;
    }

    String createModelDotCompileOptions (const String& indent) const
    {
        String result;

        result << "compileOptions.with {" << newLine
               << indent << "sourceCompatibility = JavaVersion.VERSION_1_7" << newLine
               << indent << indent << "targetCompatibility = JavaVersion.VERSION_1_7" << newLine
               << "}" << newLine;

        return result;
    }

    String createModelDotAndroidSources (const String& indent) const
    {
        String result;

        result << "android.sources {" << newLine
               << indent << "main {" << newLine
               << indent << indent << "jni {" << newLine
               << indent << indent << indent << "source {" << newLine
               << indent << indent << indent << indent << "exclude \"**/JuceModules/\"" << newLine
               << indent << indent << indent << "}" << newLine
               << indent << indent << "}" << newLine
               << indent << "}" << newLine
               << "}" << newLine;

        return result;
    }

    StringArray getCPPFlags() const
    {
        StringArray result;

        result.add ("\"-fsigned-char\"");
        result.add ("\"-fexceptions\"");
        result.add ("\"-frtti\"");

        if (isCPP11Enabled())
            result.add ("\"-std=gnu++11\"");

        // preprocessor definitions

        {
            StringPairArray preprocessorDefinitions = getAllPreprocessorDefs();
            preprocessorDefinitions.set ("JUCE_ANDROID", "1");
            preprocessorDefinitions.set ("JUCE_ANDROID_ACTIVITY_CLASSNAME", getJNIActivityClassName().replaceCharacter ('/', '_'));
            preprocessorDefinitions.set ("JUCE_ANDROID_ACTIVITY_CLASSPATH", "\\\"" + getActivityClassPath().replaceCharacter('.', '/') + "\\\"");

            const StringArray& keys = preprocessorDefinitions.getAllKeys();

            for (int i = 0; i < keys.size(); ++i)
                result.add (String ("\"-D") + keys[i] + String ("=") + preprocessorDefinitions[keys[i]] + "\"");
        }

        // include paths

        result.add ("\"-I${project.rootDir}/app\".toString()");
        result.add ("\"-I${ext.juceRootDir}\".toString()");
        result.add ("\"-I${ext.juceModuleDir}\".toString()");

        {
            Array<RelativePath> cppFiles;
            const Array<Project::Item>& groups = getAllGroups();

            struct Predicate
            {
                bool operator() (const Project::Item& projectItem) const { return projectItem.shouldBeAddedToTargetProject(); }
            };

            for (int i = 0; i < groups.size(); ++i)
                findAllProjectItemsWithPredicate (groups.getReference (i), cppFiles, Predicate());

            for (int i = 0; i < cppFiles.size(); ++i)
            {
                const RelativePath absoluteSourceFile (cppFiles.getReference (i).rebased (getTargetFolder(),
                                                                                          project.getProjectFolder(),
                                                                                          RelativePath::projectFolder));

                const String absoluteIncludeFolder (sanitisePath (project.getProjectFolder().getFullPathName() + "/"
                                                                   + absoluteSourceFile.toUnixStyle().upToLastOccurrenceOf ("/", false, false)));

                result.addIfNotAlreadyThere ("\"-I" + absoluteIncludeFolder + "\".toString()");
            }
        }

        return result;
    }

    StringArray getLDLibs() const
    {
        StringArray result;

        result.add ("android");
        result.add ("EGL");
        result.add ("GLESv2");
        result.add ("log");

        result.addArray (StringArray::fromTokens(getExternalLibrariesString(), ";", ""));

        return result;
    }

    String createModelDotAndroidNDK (const String& indent) const
    {
        String result;

        result << "android.ndk {" << newLine
               << indent << "moduleName = \"juce_jni\"" << newLine
               << indent << "stl = \"gnustl_static\"" << newLine
               << indent << "toolchainVersion = 4.9" << newLine
               << indent << "ext {" << newLine
               << indent << indent << "juceRootDir = \"" << "${project.rootDir}/../../../../" << "\".toString()" << newLine
               << indent << indent << "juceModuleDir = \"" << "${juceRootDir}/modules" << "\".toString()" << newLine
               << indent << "}" << newLine;

        // CPP flags

        {
            StringArray cppFlags (getCPPFlags());

            for (int i = 0; i < cppFlags.size(); ++i)
                result << indent << "cppFlags += " << cppFlags[i] << newLine;
        }

        // libraries

        {
            StringArray libraries (getLDLibs());

            result << indent << "ldLibs += [";

            for (int i = 0; i < libraries.size(); ++i)
            {
                result << "\"" << libraries[i] << "\"";

                if (i + 1 != libraries.size())
                    result << ", ";
            }

            result << "]" << newLine;
        }

        result << "}" << newLine;

        return result;
    }

    String getGradleCPPFlags (const String& indent, const ConstConfigIterator& config) const
    {
        String result;
        StringArray rootFlags;
        StringArray ndkFlags;

        if (config->isDebug())
        {
            ndkFlags.add ("debuggable = true");
            ndkFlags.add ("cppFlags += \"-g\"");
            ndkFlags.add ("cppFlags += \"-DDEBUG=1\"");
            ndkFlags.add ("cppFlags += \"-D_DEBUG=1\"");
        }
        else
        {
            rootFlags.add ("minifyEnabled = true");
            rootFlags.add ("proguardFiles += 'proguard-android-optimize.txt'");

            ndkFlags.add ("cppFlags += \"-DNDEBUG=1\"");
        }

        {
            StringArray extraFlags (StringArray::fromTokens (getExtraCompilerFlagsString(), " ", ""));

            for (int i = 0; extraFlags.size(); ++i)
                ndkFlags.add (String ("cppFlags += \"") + extraFlags[i] + "\"");
        }

        // there appears to be an issue with build types that have a name other than
        // "debug" or "release". Apparently this is hard coded in Android Studio ...
        {
            const String configName (config->getName());

            if (configName != "Debug" && configName != "Release")
                throw SaveError ("Build configurations other than Debug and Release are not yet support for Android Studio");

            result << configName.toLowerCase() << " {" << newLine;
        }

        for (int i = 0; i < rootFlags.size(); ++i)
            result << indent << rootFlags[i] << newLine;

        result << indent << "ndk.with {" << newLine;

        for (int i = 0; i < ndkFlags.size(); ++i)
            result << indent << indent << ndkFlags[i] << newLine;

        result << indent << "}" << newLine
               << "}" << newLine;

        return result;
    }

    String createModelDotAndroidDotBuildTypes (const String& indent) const
    {
        String result;

        result << "android.buildTypes {" << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            result << CodeHelpers::indent (getGradleCPPFlags (indent, config), indent.length(), true);

        result << "}";

        return result;
    }

    String createModelDotAndroidDotProductFlavors (const String& indent) const
    {
        String result;

        result << "android.productFlavors {" << newLine;

        // TODO! - this needs to be changed so that it generates seperate flags for debug and release ...
        // at present, it just generates all ABIs for all build types

        StringArray architectures (StringArray::fromTokens (getABIs<AndroidStudioBuildConfiguration> (true),  " ", ""));
        architectures.mergeArray  (StringArray::fromTokens (getABIs<AndroidStudioBuildConfiguration> (false), " ", ""));

        if (architectures.size() == 0)
            throw SaveError ("Can't build for no architectures!");

        for (int i = 0; i < architectures.size(); ++i)
        {
            String architecture (architectures[i].trim());

            if (architecture.isEmpty())
                continue;

            result << indent << "create(\"" << architecture << "\") {" << newLine
                   << indent << indent << "ndk.abiFilters += \"" << architecture << "\"" << newLine
                   << indent << "}" << newLine;
        }

        result << "}" << newLine;

        return result;
    }

    void writeBuildDotGradleApp (const File& folder) const
    {
        MemoryOutputStream memoryOutputStream;

        const String indent            = getIndentationString();
        const String minimumSDKVersion = getMinimumSDKVersionString();
        const String bundleIdentifier  = project.getBundleIdentifier().toString();

        memoryOutputStream << "apply plugin: 'com.android.model.application'" << newLine
                           << newLine
                           << "model {" << newLine
                           << CodeHelpers::indent (createModelDotAndroid (indent, minimumSDKVersion, bundleIdentifier), indent.length(), true)
                           << newLine
                           << CodeHelpers::indent (createModelDotCompileOptions (indent), indent.length(), true)
                           << newLine
                           << CodeHelpers::indent (createModelDotAndroidSources (indent), indent.length(), true)
                           << newLine
                           << CodeHelpers::indent (createModelDotAndroidNDK (indent), indent.length(), true)
                           << newLine
                           << CodeHelpers::indent (createModelDotAndroidDotBuildTypes (indent), indent.length(), true)
                           << newLine
                           << CodeHelpers::indent (createModelDotAndroidDotProductFlavors (indent), indent.length(), true)
                           << "}";

        overwriteFileIfDifferentOrThrow (folder.getChildFile ("app/build.gradle"), memoryOutputStream);
    }

    static const char* getIndentationString() noexcept
    {
        return "    ";
    }

    const File androidStudioExecutable;

    JUCE_DECLARE_NON_COPYABLE (AndroidStudioProjectExporter)
};
