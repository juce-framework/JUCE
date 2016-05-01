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

class AndroidAntProjectExporter  : public AndroidProjectExporterBase
{
public:
    //==============================================================================
    bool canLaunchProject() override             { return false; }
    bool launchProject() override                { return false; }
    bool isAndroid() const override              { return true; }
    bool usesMMFiles() const override            { return false; }
    bool canCopeWithDuplicateFiles() override    { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isAndroidStudio() const override        { return false; }
    bool isAndroidAnt() const override           { return true; }

    bool supportsVST() const override            { return false; }
    bool supportsVST3() const override           { return false; }
    bool supportsAAX() const override            { return false; }
    bool supportsRTAS() const override           { return false; }
    bool supportsStandalone() const override     { return false;  }

    //==============================================================================
    static const char* getName()                 { return "Android Ant Project"; }
    static const char* getValueTreeTypeName()    { return "ANDROID"; }

    //==============================================================================
    Value  getNDKToolchainVersionValue()            { return getSetting (Ids::toolset); }
    String getNDKToolchainVersionString() const     { return settings [Ids::toolset]; }
    Value  getStaticLibrariesValue()                { return getSetting (Ids::androidStaticLibraries); }
    String getStaticLibrariesString() const         { return settings [Ids::androidStaticLibraries]; }
    Value  getSharedLibrariesValue()                { return getSetting (Ids::androidSharedLibraries); }
    String getSharedLibrariesString() const         { return settings [Ids::androidSharedLibraries]; }

    //==============================================================================
    static AndroidAntProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new AndroidAntProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    AndroidAntProjectExporter (Project& p, const ValueTree& t)
    : AndroidProjectExporterBase (p, t)
    {
        name = getName();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "Android";
    }

    //==============================================================================
    void createToolchainExporterProperties (PropertyListBuilder& props) override
    {
        props.add (new TextPropertyComponent (getNDKToolchainVersionValue(), "NDK Toolchain version", 32, false),
                   "The variable NDK_TOOLCHAIN_VERSION in Application.mk - leave blank for a default value");
    }

    void createLibraryModuleExporterProperties (PropertyListBuilder& props) override
    {
        props.add (new TextPropertyComponent (getStaticLibrariesValue(), "Import static library modules", 8192, true),
                   "Comma or whitespace delimited list of static libraries (.a) defined in NDK_MODULE_PATH.");

        props.add (new TextPropertyComponent (getSharedLibrariesValue(), "Import shared library modules", 8192, true),
                   "Comma or whitespace delimited list of shared libraries (.so) defined in NDK_MODULE_PATH.");
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        AndroidProjectExporterBase::create (modules);

        const File target (getTargetFolder());
        const File jniFolder (target.getChildFile ("jni"));

        createDirectoryOrThrow (jniFolder);
        createDirectoryOrThrow (target.getChildFile ("res").getChildFile ("values"));
        createDirectoryOrThrow (target.getChildFile ("libs"));
        createDirectoryOrThrow (target.getChildFile ("bin"));

        {
            ScopedPointer<XmlElement> manifest (createManifestXML());
            writeXmlOrThrow (*manifest, target.getChildFile ("AndroidManifest.xml"), "utf-8", 100, true);
        }

        writeApplicationMk (jniFolder.getChildFile ("Application.mk"));
        writeAndroidMk (jniFolder.getChildFile ("Android.mk"));

        {
            ScopedPointer<XmlElement> antBuildXml (createAntBuildXML());
            writeXmlOrThrow (*antBuildXml, target.getChildFile ("build.xml"), "UTF-8", 100);
        }

        writeProjectPropertiesFile (target.getChildFile ("project.properties"));
        writeLocalPropertiesFile (target.getChildFile ("local.properties"));
        writeStringsFile (target.getChildFile ("res/values/strings.xml"));
        writeIcons (target.getChildFile ("res"));
    }

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
        return new AndroidBuildConfiguration (project, v, *this);
    }

private:
    //==============================================================================
    String getToolchainVersion() const
    {
        String v (getNDKToolchainVersionString());
        return v.isNotEmpty() ? v : "4.9";
    }


    //==============================================================================
    String getCppFlags() const
    {
        String flags ("-fsigned-char -fexceptions -frtti");

        if (! getNDKToolchainVersionString().startsWithIgnoreCase ("clang"))
            flags << " -Wno-psabi";

        return flags;
    }

    String getAppPlatform() const
    {
        int ndkVersion = androidMinimumSDK.get().getIntValue();
        if (ndkVersion == 9)
            ndkVersion = 10; // (doesn't seem to be a version '9')

        return "android-" + String (ndkVersion);
    }

    void writeApplicationMk (const File& file) const
    {
        MemoryOutputStream mo;

        mo << "# Automatically generated makefile, created by the Projucer" << newLine
        << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
        << newLine
        << "APP_STL := gnustl_static" << newLine
        << "APP_CPPFLAGS += " << getCppFlags() << newLine
        << "APP_PLATFORM := " << getAppPlatform() << newLine
        << "NDK_TOOLCHAIN_VERSION := " << getToolchainVersion() << newLine
        << newLine
        << "ifeq ($(NDK_DEBUG),1)" << newLine
        << "    APP_ABI := " << getABIs<AndroidBuildConfiguration> (true) << newLine
        << "else" << newLine
        << "    APP_ABI := " << getABIs<AndroidBuildConfiguration> (false) << newLine
        << "endif" << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    struct ShouldFileBeCompiledPredicate
    {
        bool operator() (const Project::Item& projectItem) const  { return projectItem.shouldBeCompiled(); }
    };

    void writeAndroidMk (const File& file) const
    {
        Array<RelativePath> files;

        for (int i = 0; i < getAllGroups().size(); ++i)
            findAllProjectItemsWithPredicate (getAllGroups().getReference(i), files, ShouldFileBeCompiledPredicate());

        MemoryOutputStream mo;
        writeAndroidMk (mo, files);

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeAndroidMkVariableList (OutputStream& out, const String& variableName, const String& settingsValue) const
    {
        const StringArray separatedItems (getCommaOrWhitespaceSeparatedItems (settingsValue));

        if (separatedItems.size() > 0)
            out << newLine << variableName << " := " << separatedItems.joinIntoString (" ") << newLine;
    }

    void writeAndroidMk (OutputStream& out, const Array<RelativePath>& files) const
    {
        out << "# Automatically generated makefile, created by the Projucer" << newLine
        << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
        << newLine
        << "LOCAL_PATH := $(call my-dir)" << newLine
        << newLine
        << "include $(CLEAR_VARS)" << newLine
        << newLine
        << "ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)" << newLine
        << "    LOCAL_ARM_MODE := arm" << newLine
        << "endif" << newLine
        << newLine
        << "LOCAL_MODULE := juce_jni" << newLine
        << "LOCAL_SRC_FILES := \\" << newLine;

        for (int i = 0; i < files.size(); ++i)
            out << "  " << (files.getReference(i).isAbsolute() ? "" : "../")
            << escapeSpaces (files.getReference(i).toUnixStyle()) << "\\" << newLine;

        writeAndroidMkVariableList (out, "LOCAL_STATIC_LIBRARIES", getStaticLibrariesString());
        writeAndroidMkVariableList (out, "LOCAL_SHARED_LIBRARIES", getSharedLibrariesString());

        out << newLine
        << "ifeq ($(NDK_DEBUG),1)" << newLine;
        writeConfigSettings (out, true);
        out << "else" << newLine;
        writeConfigSettings (out, false);
        out << "endif" << newLine
        << newLine
        << "include $(BUILD_SHARED_LIBRARY)" << newLine;

        StringArray importModules (getCommaOrWhitespaceSeparatedItems (getStaticLibrariesString()));
        importModules.addArray (getCommaOrWhitespaceSeparatedItems (getSharedLibrariesString()));

        for (int i = 0; i < importModules.size(); ++i)
            out << "$(call import-module," << importModules[i] << ")" << newLine;
    }

    void writeConfigSettings (OutputStream& out, bool forDebug) const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            if (config->isDebug() == forDebug)
            {
                const AndroidBuildConfiguration& androidConfig = dynamic_cast<const AndroidBuildConfiguration&> (*config);

                String cppFlags;
                cppFlags << createCPPFlags (androidConfig)
                << (" " + replacePreprocessorTokens (androidConfig, getExtraCompilerFlagsString()).trim()).trimEnd()
                << newLine
                << getLDLIBS (androidConfig).trimEnd()
                << newLine;

                out << "  LOCAL_CPPFLAGS += " << cppFlags;
                out << "  LOCAL_CFLAGS += " << cppFlags;
                break;
            }
        }
    }

    String getLDLIBS (const AndroidBuildConfiguration& config) const
    {
        return "  LOCAL_LDLIBS :=" + config.getGCCLibraryPathFlags()
        + " -llog -lGLESv2 -landroid -lEGL" + getExternalLibraryFlags (config)
        + " " + replacePreprocessorTokens (config, getExtraLinkerFlagsString());
    }

    String createIncludePathFlags (const BuildConfiguration& config) const
    {
        String flags;
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());

        searchPaths = getCleanedStringArray (searchPaths);

        for (int i = 0; i < searchPaths.size(); ++i)
            flags << " -I " << FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i])).quoted();

        return flags;
    }

    String createCPPFlags (const BuildConfiguration& config) const
    {
        StringPairArray defines;
        defines.set ("JUCE_ANDROID", "1");
        defines.set ("JUCE_ANDROID_API_VERSION", androidMinimumSDK.get());
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSNAME", getJNIActivityClassName().replaceCharacter ('/', '_'));
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSPATH", "\\\"" + getJNIActivityClassName() + "\\\"");

        String flags ("-fsigned-char -fexceptions -frtti");

        if (config.isDebug())
        {
            flags << " -g";
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        flags << createIncludePathFlags (config)
        << " -O" << config.getGCCOptimisationFlag();

        flags << " -std=gnu++11";

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));
        return flags + createGCCPreprocessorFlags (defines);
    }

    //==============================================================================
    XmlElement* createAntBuildXML() const
    {
        XmlElement* proj = new XmlElement ("project");
        proj->setAttribute ("name", projectName);
        proj->setAttribute ("default", "debug");

        proj->createNewChildElement ("loadproperties")->setAttribute ("srcFile", "local.properties");
        proj->createNewChildElement ("loadproperties")->setAttribute ("srcFile", "project.properties");

        {
            XmlElement* target = proj->createNewChildElement ("target");
            target->setAttribute ("name", "clean");
            target->setAttribute ("depends", "android_rules.clean");

            target->createNewChildElement ("delete")->setAttribute ("dir", "libs");
            target->createNewChildElement ("delete")->setAttribute ("dir", "obj");

            XmlElement* executable = target->createNewChildElement ("exec");
            executable->setAttribute ("executable", "${ndk.dir}/ndk-build");
            executable->setAttribute ("dir", "${basedir}");
            executable->setAttribute ("failonerror", "true");

            executable->createNewChildElement ("arg")->setAttribute ("value", "clean");
        }

        {
            XmlElement* target = proj->createNewChildElement ("target");
            target->setAttribute ("name", "-pre-build");

            addDebugConditionClause (target, "makefileConfig", "Debug", "Release");
            addDebugConditionClause (target, "ndkDebugValue", "NDK_DEBUG=1", "NDK_DEBUG=0");

            String debugABIs, releaseABIs;

            for (ConstConfigIterator config (*this); config.next();)
            {
                const AndroidBuildConfiguration& androidConfig = dynamic_cast<const AndroidBuildConfiguration&> (*config);

                if (config->isDebug())
                    debugABIs = androidConfig.getArchitectures();
                else
                    releaseABIs = androidConfig.getArchitectures();
            }

            addDebugConditionClause (target, "app_abis", debugABIs, releaseABIs);

            XmlElement* executable = target->createNewChildElement ("exec");
            executable->setAttribute ("executable", "${ndk.dir}/ndk-build");
            executable->setAttribute ("dir", "${basedir}");
            executable->setAttribute ("failonerror", "true");

            executable->createNewChildElement ("arg")->setAttribute ("value", "--jobs=4");
            executable->createNewChildElement ("arg")->setAttribute ("value", "CONFIG=${makefileConfig}");
            executable->createNewChildElement ("arg")->setAttribute ("value", "${ndkDebugValue}");
            executable->createNewChildElement ("arg")->setAttribute ("value", "APP_ABI=${app_abis}");

            target->createNewChildElement ("delete")->setAttribute ("file", "${out.final.file}");
            target->createNewChildElement ("delete")->setAttribute ("file", "${out.packaged.file}");
        }

        proj->createNewChildElement ("import")->setAttribute ("file", "${sdk.dir}/tools/ant/build.xml");

        return proj;
    }

    void addDebugConditionClause (XmlElement* target, const String& property,
                                  const String& debugValue, const String& releaseValue) const
    {
        XmlElement* condition = target->createNewChildElement ("condition");
        condition->setAttribute ("property", property);
        condition->setAttribute ("value", debugValue);
        condition->setAttribute ("else", releaseValue);

        XmlElement* equals = condition->createNewChildElement ("equals");
        equals->setAttribute ("arg1", "${ant.project.invoked-targets}");
        equals->setAttribute ("arg2", "debug");
    }

    void writeProjectPropertiesFile (const File& file) const
    {
        MemoryOutputStream mo;
        mo << "# This file is used to override default values used by the Ant build system." << newLine
        << "# It is automatically generated - DO NOT EDIT IT or your changes will be lost!." << newLine
        << newLine
        << "target=" << getAppPlatform() << newLine
        << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeLocalPropertiesFile (const File& file) const
    {
        MemoryOutputStream mo;
        mo << "# This file is used to override default values used by the Ant build system." << newLine
        << "# It is automatically generated by the Projucer - DO NOT EDIT IT or your changes will be lost!." << newLine
        << newLine
        << "sdk.dir=" << escapeSpaces (replacePreprocessorDefs (getAllPreprocessorDefs(), sdkPath.toString())) << newLine
        << "ndk.dir=" << escapeSpaces (replacePreprocessorDefs (getAllPreprocessorDefs(), ndkPath.toString())) << newLine
        << "key.store=" << androidKeyStore.get() << newLine
        << "key.alias=" << androidKeyAlias.get() << newLine
        << "key.store.password=" << androidKeyStorePass.get() << newLine
        << "key.alias.password=" << androidKeyAliasPass.get() << newLine
        << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeStringsFile (const File& file) const
    {
        XmlElement strings ("resources");
        XmlElement* resourceName = strings.createNewChildElement ("string");
        resourceName->setAttribute ("name", "app_name");
        resourceName->addTextElement (projectName);

        writeXmlOrThrow (strings, file, "utf-8", 100);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (AndroidAntProjectExporter)
};
