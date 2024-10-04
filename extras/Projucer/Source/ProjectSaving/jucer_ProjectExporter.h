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

#include "../Project/jucer_Project.h"
#include "../Utility/UI/PropertyComponents/jucer_PropertyComponentsWithEnablement.h"
#include "../Utility/Helpers/jucer_ValueTreePropertyWithDefaultWrapper.h"
#include "../Project/Modules/jucer_Modules.h"

class ProjectSaver;

class LinuxSubprocessHelperProperties
{
public:
    explicit LinuxSubprocessHelperProperties (ProjectExporter& projectExporter);

    bool shouldUseLinuxSubprocessHelper() const;

    void deployLinuxSubprocessHelperSourceFilesIfNecessary() const;

    build_tools::RelativePath getLinuxSubprocessHelperSource() const;

    void setCompileDefinitionIfNecessary (StringPairArray& defs) const;

    build_tools::RelativePath getSimpleBinaryBuilderSource() const;

    build_tools::RelativePath getLinuxSubprocessHelperBinaryDataSource() const;

    void addToExtraSearchPathsIfNecessary() const;

    static std::optional<String> getParentDirectoryRelativeToBuildTargetFolder (build_tools::RelativePath rp);

    static String makeSnakeCase (const String& s);

    static String getBinaryNameFromSource (const build_tools::RelativePath& rp);

    static constexpr const char* useLinuxSubprocessHelperCompileDefinition = "JUCE_USE_EXTERNAL_TEMPORARY_SUBPROCESS";

private:
    ProjectExporter& owner;
};

//==============================================================================
struct PackageDependency
{
    explicit PackageDependency (StringRef dependencyIn)
        : dependency { dependencyIn }
    {
    }

    PackageDependency (StringRef dependencyIn, StringRef fallbackIn)
        : dependency { dependencyIn },
          fallback { fallbackIn }
    {
    }

    String dependency;
    std::optional<String> fallback;
};

std::vector<PackageDependency> makePackageDependencies (const StringArray& dependencies);

//==============================================================================
class ProjectExporter : private Value::Listener
{
public:
    ProjectExporter (Project&, const ValueTree& settings);

    //==============================================================================
    struct ExporterTypeInfo
    {
        Identifier identifier;
        String displayName;
        String targetFolder;

        Image icon;
    };

    static std::vector<ExporterTypeInfo> getExporterTypeInfos();
    static ExporterTypeInfo getTypeInfoForExporter (const Identifier& exporterIdentifier);
    static ExporterTypeInfo getCurrentPlatformExporterTypeInfo();

    static std::unique_ptr<ProjectExporter> createNewExporter (Project&, const Identifier& exporterIdentifier);
    static std::unique_ptr<ProjectExporter> createExporterFromSettings (Project&, const ValueTree& settings);

    static bool canProjectBeLaunched (Project*);

    virtual Identifier getExporterIdentifier() const = 0;

    //==============================================================================
    // capabilities of exporter
    virtual bool usesMMFiles() const = 0;
    virtual void createExporterProperties (PropertyListBuilder&) = 0;
    virtual bool canLaunchProject() = 0;
    virtual bool launchProject() = 0;
    virtual void create (const OwnedArray<LibraryModule>&) const = 0; // may throw a SaveError
    virtual bool shouldFileBeCompiledByDefault (const File& path) const;
    virtual bool canCopeWithDuplicateFiles() = 0;
    virtual bool supportsUserDefinedConfigurations() const = 0; // false if exporter only supports two configs Debug and Release
    virtual void updateDeprecatedSettings()               {}
    virtual void initialiseDependencyPathValues()         {}

    // IDE targeted by exporter
    virtual bool isXcode() const         = 0;
    virtual bool isVisualStudio() const  = 0;
    virtual bool isMakefile() const      = 0;
    virtual bool isAndroidStudio() const = 0;

    // operating system targeted by exporter
    virtual bool isAndroid() const = 0;
    virtual bool isWindows() const = 0;
    virtual bool isLinux() const   = 0;
    virtual bool isOSX() const     = 0;
    virtual bool isiOS() const     = 0;

    virtual String getNewLineString() const = 0;
    virtual String getDescription()  { return {}; }

    virtual bool supportsPrecompiledHeaders() const  { return false; }

    //==============================================================================
    // cross-platform audio plug-ins supported by exporter
    virtual bool supportsTargetType (build_tools::ProjectType::Target::Type type) const = 0;

    inline bool shouldBuildTargetType (build_tools::ProjectType::Target::Type type) const
    {
        return project.shouldBuildTargetType (type) && supportsTargetType (type);
    }

    inline void callForAllSupportedTargets (std::function<void (build_tools::ProjectType::Target::Type)> callback)
    {
        for (int i = 0; i < build_tools::ProjectType::Target::unspecified; ++i)
            if (shouldBuildTargetType (static_cast<build_tools::ProjectType::Target::Type> (i)))
                callback (static_cast<build_tools::ProjectType::Target::Type> (i));
    }

    //==============================================================================
    bool mayCompileOnCurrentOS() const
    {
       #if JUCE_MAC
        return isOSX() || isAndroid() || isiOS();
       #elif JUCE_WINDOWS
        return isWindows() || isAndroid();
       #elif JUCE_LINUX
        return isLinux() || isAndroid();
       #elif JUCE_BSD
        return isLinux();
       #else
        #error
       #endif
    }

    //==============================================================================
    String getUniqueName() const;
    File getTargetFolder() const;

    Project& getProject() noexcept                        { return project; }
    const Project& getProject() const noexcept            { return project; }

    UndoManager* getUndoManager() const                   { return project.getUndoManagerFor (settings); }

    Value getSetting (const Identifier& nm)               { return settings.getPropertyAsValue (nm, project.getUndoManagerFor (settings)); }
    String getSettingString (const Identifier& nm) const  { return settings [nm]; }

    Value getTargetLocationValue()                        { return targetLocationValue.getPropertyAsValue(); }
    String getTargetLocationString() const                { return targetLocationValue.get(); }

    StringArray getExternalLibrariesStringArray() const   { return getSearchPathsFromString (externalLibrariesValue.get().toString()); }
    String getExternalLibrariesString() const             { return getExternalLibrariesStringArray().joinIntoString (";"); }

    bool shouldUseGNUExtensions() const                   { return gnuExtensionsValue.get(); }

    String getVSTLegacyPathString() const                 { return vstLegacyPathValueWrapper.getCurrentValue(); }

    auto getAAXPathRelative() const
    {
        const String userAaxFolder = aaxPathValueWrapper.getCurrentValue();
        return userAaxFolder.isNotEmpty()
             ? build_tools::RelativePath (userAaxFolder, build_tools::RelativePath::projectFolder)
             : getModuleFolderRelativeToProject ("juce_audio_plugin_client").getChildFile ("AAX")
                                                                            .getChildFile ("SDK");
    }

    String getARAPathString() const                       { return araPathValueWrapper.getCurrentValue(); }

    // NB: this is the path to the parent "modules" folder that contains the named module, not the
    // module folder itself.
    ValueTreePropertyWithDefault getPathForModuleValue (const String& moduleID);
    String getPathForModuleString (const String& moduleID) const;
    void removePathForModule (const String& moduleID);

    TargetOS::OS getTargetOSForExporter() const;

    build_tools::RelativePath getLegacyModulePath (const String& moduleID) const;
    String getLegacyModulePath() const;

    // Returns a path to the actual module folder itself
    build_tools::RelativePath getModuleFolderRelativeToProject (const String& moduleID) const;
    void updateOldModulePaths();

    build_tools::RelativePath rebaseFromProjectFolderToBuildTarget (const build_tools::RelativePath& path) const;
    build_tools::RelativePath rebaseFromBuildTargetToProjectFolder (const build_tools::RelativePath& path) const;
    File resolveRelativePath (const build_tools::RelativePath&) const;
    void addToExtraSearchPaths (const build_tools::RelativePath& pathFromProjectFolder, int index = -1);
    void addToModuleLibPaths   (const build_tools::RelativePath& pathFromProjectFolder);

    void addProjectPathToBuildPathList (StringArray&, const build_tools::RelativePath&, int index = -1) const;

    std::unique_ptr<Drawable> getBigIcon() const;
    std::unique_ptr<Drawable> getSmallIcon() const;
    build_tools::Icons getIcons() const { return { getSmallIcon(), getBigIcon() }; }

    String getExporterIdentifierMacro() const
    {
        return "JUCER_" + settings.getType().toString() + "_"
                + String::toHexString (getTargetLocationString().hashCode()).toUpperCase();
    }

    // An exception that can be thrown by the create() method.
    void createPropertyEditors (PropertyListBuilder&);
    void addSettingsForProjectType (const build_tools::ProjectType&);

    build_tools::RelativePath getLV2HelperProgramSource() const
    {
        return getModuleFolderRelativeToProject ("juce_audio_plugin_client")
               .getChildFile ("LV2")
               .getChildFile ("juce_LV2ManifestHelper.cpp");
    }

    build_tools::RelativePath getVST3HelperProgramSource() const
    {
        const auto suffix = isOSX() ? "mm" : "cpp";
        return getModuleFolderRelativeToProject ("juce_audio_plugin_client")
               .getChildFile ("VST3")
               .getChildFile (String ("juce_VST3ManifestHelper.") + suffix);
    }

    //==============================================================================
    void copyMainGroupFromProject();
    Array<Project::Item>& getAllGroups() noexcept               { jassert (itemGroups.size() > 0); return itemGroups; }
    const Array<Project::Item>& getAllGroups() const noexcept   { jassert (itemGroups.size() > 0); return itemGroups; }
    Project::Item& getModulesGroup();

    //==============================================================================
    StringArray linuxLibs, linuxPackages, makefileExtraLinkerFlags;

    enum class PackageDependencyType
    {
        compile,
        link
    };

    std::vector<PackageDependency> getLinuxPackages (PackageDependencyType type) const;

    //==============================================================================
    StringPairArray msvcExtraPreprocessorDefs;
    String msvcDelayLoadedDLLs;
    StringArray windowsLibs;

    //==============================================================================
    StringArray androidLibs;

    //==============================================================================
    StringArray extraSearchPaths;
    StringArray moduleLibSearchPaths;

    //==============================================================================
    const LinuxSubprocessHelperProperties linuxSubprocessHelperProperties { *this };

    //==============================================================================
    class BuildConfiguration : public ReferenceCountedObject
    {
    public:
        BuildConfiguration (Project& project, const ValueTree& configNode, const ProjectExporter&);

        using Ptr = ReferenceCountedObjectPtr<BuildConfiguration>;

        //==============================================================================
        virtual void createConfigProperties (PropertyListBuilder&) = 0;
        virtual String getModuleLibraryArchName() const = 0;

        //==============================================================================
        String getName() const                                 { return configNameValue.get(); }
        bool isDebug() const                                   { return isDebugValue.get(); }

        String getTargetBinaryRelativePathString() const       { return targetBinaryPathValue.get(); }
        String getTargetBinaryNameString (bool isUnityPlugin = false) const
        {
            return (isUnityPlugin ? Project::addUnityPluginPrefixIfNecessary (targetNameValue.get().toString())
                                  : targetNameValue.get().toString());
        }

        int getOptimisationLevelInt() const                    { return optimisationLevelValue.get(); }
        String getGCCOptimisationFlag() const;
        bool isLinkTimeOptimisationEnabled() const             { return linkTimeOptimisationValue.get(); }

        String getBuildConfigPreprocessorDefsString() const    { return ppDefinesValue.get(); }
        StringPairArray getAllPreprocessorDefs() const;        // includes inherited definitions

        String getHeaderSearchPathString() const               { return headerSearchPathValue.get(); }
        StringArray getHeaderSearchPaths() const;

        String getLibrarySearchPathString() const              { return librarySearchPathValue.get(); }
        StringArray getLibrarySearchPaths() const;

        String getPrecompiledHeaderFilename() const            { return "JucePrecompiledHeader_" + getName(); }
        static String getSkipPrecompiledHeaderDefine()         { return "JUCE_SKIP_PRECOMPILED_HEADER"; }

        bool shouldUsePrecompiledHeaderFile() const            { return usePrecompiledHeaderFileValue.get(); }
        String getPrecompiledHeaderFileContent() const;

        String getAllCompilerFlagsString() const               { return (exporter.extraCompilerFlagsValue.get().toString() + "  " + configCompilerFlagsValue.get().toString()).replaceCharacters ("\r\n", "  ").trim(); }
        String getAllLinkerFlagsString() const                 { return (exporter.extraLinkerFlagsValue  .get().toString() + "  " + configLinkerFlagsValue  .get().toString()).replaceCharacters ("\r\n", "  ").trim(); }

        //==============================================================================
        Value getValue (const Identifier& nm)                  { return config.getPropertyAsValue (nm, getUndoManager()); }
        UndoManager* getUndoManager() const                    { return project.getUndoManagerFor (config); }

        //==============================================================================
        void createPropertyEditors (PropertyListBuilder&);
        void addRecommendedLinuxCompilerWarningsProperty (PropertyListBuilder&);
        void addRecommendedLLVMCompilerWarningsProperty (PropertyListBuilder&);

        struct CompilerNames
        {
            static constexpr const char* gcc = "GCC";
            static constexpr const char* llvm = "LLVM";
        };

        struct CompilerWarningFlags
        {
            static CompilerWarningFlags getRecommendedForGCCAndLLVM()
            {
                CompilerWarningFlags result;
                result.common = {
                    "-Wall",
                    "-Wcast-align",
                    "-Wfloat-equal",
                    "-Wno-ignored-qualifiers",
                    "-Wsign-compare",
                    "-Wsign-conversion",
                    "-Wstrict-aliasing",
                    "-Wswitch-enum",
                    "-Wuninitialized",
                    "-Wunreachable-code",
                    "-Wunused-parameter",
                    "-Wmissing-field-initializers"
                };

                result.cpp = {
                    "-Woverloaded-virtual",
                    "-Wreorder",
                    "-Wzero-as-null-pointer-constant"
                };

                return result;
            }

            StringArray common, cpp, objc;
        };

        CompilerWarningFlags getRecommendedCompilerWarningFlags() const;

        void addGCCOptimisationProperty (PropertyListBuilder&);
        void removeFromExporter();

        //==============================================================================
        ValueTree config;
        Project& project;
        const ProjectExporter& exporter;

    protected:
        ValueTreePropertyWithDefault isDebugValue, configNameValue, targetNameValue, targetBinaryPathValue, recommendedWarningsValue, optimisationLevelValue,
                                     linkTimeOptimisationValue, ppDefinesValue, headerSearchPathValue, librarySearchPathValue, userNotesValue,
                                     usePrecompiledHeaderFileValue, precompiledHeaderFileValue, configCompilerFlagsValue, configLinkerFlagsValue;

    private:
        std::map<String, CompilerWarningFlags> recommendedCompilerWarningFlags;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuildConfiguration)
    };

    void addNewConfigurationFromExisting (const BuildConfiguration& configToCopy);
    void addNewConfiguration (bool isDebugConfig);

    String getExternalLibraryFlags (const BuildConfiguration& config) const;

    //==============================================================================
    struct ConfigIterator
    {
        ConfigIterator (ProjectExporter& exporter);

        bool next();

        BuildConfiguration& operator*() const       { return *config; }
        BuildConfiguration* operator->() const      { return config.get(); }

        BuildConfiguration::Ptr config;
        int index;

    private:
        ProjectExporter& exporter;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigIterator)
    };

    struct ConstConfigIterator
    {
        ConstConfigIterator (const ProjectExporter& exporter);

        bool next();

        const BuildConfiguration& operator*() const       { return *config; }
        const BuildConfiguration* operator->() const      { return config.get(); }

        BuildConfiguration::Ptr config;
        int index;

    private:
        const ProjectExporter& exporter;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConstConfigIterator)
    };

    int getNumConfigurations() const;
    BuildConfiguration::Ptr getConfiguration (int index) const;

    ValueTree getConfigurations() const;
    virtual void createDefaultConfigs();
    void createDefaultModulePaths();

    //==============================================================================
    Value getExporterPreprocessorDefsValue()            { return extraPPDefsValue.getPropertyAsValue(); }
    String getExporterPreprocessorDefsString() const    { return extraPPDefsValue.get(); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const BuildConfiguration& config, const build_tools::ProjectType::Target::Type targetType) const;
    // includes exporter + project defs
    StringPairArray getAllPreprocessorDefs() const;

    void addTargetSpecificPreprocessorDefs (StringPairArray& defs, const build_tools::ProjectType::Target::Type targetType) const;

    String replacePreprocessorTokens (const BuildConfiguration&, const String& sourceString) const;

    ValueTree settings;

    enum GCCOptimisationLevel
    {
        gccO0     = 1,
        gccO1     = 4,
        gccO2     = 5,
        gccO3     = 3,
        gccOs     = 2,
        gccOfast  = 6
    };

    bool isPCHEnabledForAnyConfigurations() const
    {
        if (supportsPrecompiledHeaders())
            for (ConstConfigIterator config (*this); config.next();)
                if (config->shouldUsePrecompiledHeaderFile())
                    return true;

        return false;
    }

    String getCompilerFlagsForFileCompilerFlagScheme (StringRef) const;
    String getCompilerFlagsForProjectItem (const Project::Item&) const;

protected:
    //==============================================================================
    String name;
    Project& project;
    const build_tools::ProjectType& projectType;
    const String projectName;
    const File projectFolder;

    //==============================================================================
    ValueTreePropertyWithDefaultWrapper vstLegacyPathValueWrapper, aaxPathValueWrapper, araPathValueWrapper;

    ValueTreePropertyWithDefault targetLocationValue, extraCompilerFlagsValue, extraLinkerFlagsValue, externalLibrariesValue,
                                 userNotesValue, gnuExtensionsValue, bigIconValue, smallIconValue, extraPPDefsValue;

    Value projectCompilerFlagSchemesValue;

    mutable Array<Project::Item> itemGroups;
    Project::Item* modulesGroup = nullptr;

    virtual BuildConfiguration::Ptr createBuildConfig (const ValueTree&) const = 0;

    void addDefaultPreprocessorDefs (StringPairArray&) const;

    static String getDefaultBuildsRootFolder()            { return "Builds/"; }

    static String getStaticLibbedFilename (String name)   { return addSuffix (addLibPrefix (name), ".a"); }
    static String getDynamicLibbedFilename (String name)  { return addSuffix (addLibPrefix (name), ".so"); }

    virtual void addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) = 0;

    //==============================================================================
    static void createDirectoryOrThrow (const File& dirToCreate)
    {
        if (! dirToCreate.createDirectory())
            throw build_tools::SaveError ("Can't create folder: " + dirToCreate.getFullPathName());
    }

    static void writeXmlOrThrow (const XmlElement& xml, const File& file, const String& encoding,
                                 int maxCharsPerLine, bool useUnixNewLines = false)
    {
        XmlElement::TextFormat format;
        format.customEncoding = encoding;
        format.lineWrapLength = maxCharsPerLine;
        format.newLineChars = useUnixNewLines ? "\n" : "\r\n";

        MemoryOutputStream mo (8192);
        xml.writeTo (mo, format);
        build_tools::overwriteFileIfDifferentOrThrow (file, mo);
    }

private:
    //==============================================================================
    std::map<String, ValueTreePropertyWithDefault> compilerFlagSchemesMap;

    //==============================================================================
    void valueChanged (Value&) override   { updateCompilerFlagValues(); }
    void updateCompilerFlagValues();

    //==============================================================================
    static String addLibPrefix (const String name)
    {
        return name.startsWith ("lib") ? name
                                       : "lib" + name;
    }

    static String addSuffix (const String name, const String suffix)
    {
        return name.endsWithIgnoreCase (suffix) ? name
                                                : name + suffix;
    }

    void createIconProperties (PropertyListBuilder&);
    void addExtraIncludePathsIfPluginOrHost();
    void addARAPathsIfPluginOrHost();
    void addCommonAudioPluginSettings();
    void addLegacyVSTFolderToPathIfSpecified();
    build_tools::RelativePath getInternalVST3SDKPath();
    void addAAXFoldersToPath();
    void addARAFoldersToPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter)
};
