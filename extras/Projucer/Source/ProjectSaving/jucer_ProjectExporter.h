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

#include "../Project/jucer_Project.h"
#include "../Utility/UI/PropertyComponents/jucer_DependencyPathPropertyComponent.h"
#include "../Utility/UI/PropertyComponents/jucer_TextWithDefaultPropertyComponent.h"

class ProjectSaver;

//==============================================================================
class ProjectExporter
{
public:
    ProjectExporter (Project&, const ValueTree& settings);
    virtual ~ProjectExporter();

    struct ExporterTypeInfo
    {
        String name;
        const void* iconData;
        int iconDataSize;

        Image getIcon() const
        {
            Image image (Image::ARGB, 200, 200, true);
            Graphics g (image);

            ScopedPointer<Drawable> svgDrawable = Drawable::createFromImageData (iconData, (size_t) iconDataSize);

            svgDrawable->drawWithin (g, image.getBounds().toFloat(), RectanglePlacement::fillDestination, 1.0f);

            return image;
        }
    };

    static StringArray getExporterNames();
    static Array<ExporterTypeInfo> getExporterTypes();
    static String getValueTreeNameForExporter (const String& exporterName);
    static StringArray getAllDefaultBuildsFolders();

    static ProjectExporter* createNewExporter (Project&, const int index);
    static ProjectExporter* createNewExporter (Project&, const String& name);
    static ProjectExporter* createExporter (Project&, const ValueTree& settings);
    static bool canProjectBeLaunched (Project*);

    static String getCurrentPlatformExporterName();

    //==============================================================================
    // capabilities of exporter
    virtual bool usesMMFiles() const = 0;
    virtual void createExporterProperties (PropertyListBuilder&) = 0;
    virtual bool canLaunchProject() = 0;
    virtual bool launchProject() = 0;
    virtual void create (const OwnedArray<LibraryModule>&) const = 0; // may throw a SaveError
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;
    virtual bool canCopeWithDuplicateFiles() = 0;
    virtual bool supportsUserDefinedConfigurations() const = 0; // false if exporter only supports two configs Debug and Release
    virtual void updateDeprecatedProjectSettingsInteractively();
    virtual void initialiseDependencyPathValues() {}

    // IDE targeted by exporter
    virtual bool isXcode() const         = 0;
    virtual bool isVisualStudio() const  = 0;
    virtual bool isCodeBlocks() const    = 0;
    virtual bool isMakefile() const      = 0;
    virtual bool isAndroidStudio() const = 0;
    virtual bool isCLion() const         = 0;

    // operating system targeted by exporter
    virtual bool isAndroid() const = 0;
    virtual bool isWindows() const = 0;
    virtual bool isLinux() const   = 0;
    virtual bool isOSX() const     = 0;
    virtual bool isiOS() const     = 0;

    virtual String getDescription()   { return {}; }

    //==============================================================================
    // cross-platform audio plug-ins supported by exporter
    virtual bool supportsTargetType (ProjectType::Target::Type type) const = 0;

    inline bool shouldBuildTargetType (ProjectType::Target::Type type) const
    {
        return project.shouldBuildTargetType (type) && supportsTargetType (type);
    }

    inline void callForAllSupportedTargets (std::function<void (ProjectType::Target::Type)> callback)
    {
        for (int i = 0; i < ProjectType::Target::unspecified; ++i)
            if (shouldBuildTargetType (static_cast<ProjectType::Target::Type> (i)))
                callback (static_cast<ProjectType::Target::Type> (i));
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
       #else
        #error
       #endif
    }

    //==============================================================================
    String getName() const;
    File getTargetFolder() const;

    Project& getProject() noexcept              { return project; }
    const Project& getProject() const noexcept  { return project; }

    Value getSetting (const Identifier& nm)     { return settings.getPropertyAsValue (nm, project.getUndoManagerFor (settings)); }
    String getSettingString (const Identifier& nm) const  { return settings [nm]; }

    Value getTargetLocationValue()              { return getSetting (Ids::targetFolder); }
    String getTargetLocationString() const      { return getSettingString (Ids::targetFolder); }

    Value getExtraCompilerFlags()               { return getSetting (Ids::extraCompilerFlags); }
    String getExtraCompilerFlagsString() const  { return getSettingString (Ids::extraCompilerFlags).replaceCharacters ("\r\n", "  "); }

    Value getExtraLinkerFlags()                 { return getSetting (Ids::extraLinkerFlags); }
    String getExtraLinkerFlagsString() const    { return getSettingString (Ids::extraLinkerFlags).replaceCharacters ("\r\n", "  "); }

    Value getExternalLibraries()                { return getSetting (Ids::externalLibraries); }
    String getExternalLibrariesString() const   { return getSearchPathsFromString (getSettingString (Ids::externalLibraries)).joinIntoString (";"); }

    Value getUserNotes()                        { return getSetting (Ids::userNotes); }

    Value getVST3PathValue() const              { return vst3Path; }
    Value getRTASPathValue() const              { return rtasPath; }
    Value getAAXPathValue() const               { return aaxPath; }

    Value getShouldUseGNUExtensionsValue()      { return getSetting (Ids::enableGNUExtensions); }
    bool shouldUseGNUExtensions() const         { return (getSettingString (Ids::enableGNUExtensions) == "1");}

    // NB: this is the path to the parent "modules" folder that contains the named module, not the
    // module folder itself.
    Value getPathForModuleValue (const String& moduleID);
    String getPathForModuleString (const String& moduleID) const;
    void removePathForModule (const String& moduleID);

    TargetOS::OS getTargetOSForExporter() const;

    RelativePath getLegacyModulePath (const String& moduleID) const;
    String getLegacyModulePath() const;

    // Returns a path to the actual module folder itself
    RelativePath getModuleFolderRelativeToProject (const String& moduleID) const;
    void updateOldModulePaths();

    RelativePath rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const;
    void addToExtraSearchPaths (const RelativePath& pathFromProjectFolder, int index = -1);
    void addToModuleLibPaths   (const RelativePath& pathFromProjectFolder);

    void addProjectPathToBuildPathList (StringArray&, const RelativePath&, int index = -1) const;

    Value getBigIconImageItemID()               { return getSetting (Ids::bigIcon); }
    Value getSmallIconImageItemID()             { return getSetting (Ids::smallIcon); }
    Drawable* getBigIcon() const;
    Drawable* getSmallIcon() const;
    Image getBestIconForSize (int size, bool returnNullIfNothingBigEnough) const;

    String getExporterIdentifierMacro() const
    {
        return "JUCER_" + settings.getType().toString() + "_"
                + String::toHexString (getSettingString (Ids::targetFolder).hashCode()).toUpperCase();
    }

    // An exception that can be thrown by the create() method.
    class SaveError
    {
    public:
        SaveError (const String& error) : message (error)
        {}

        SaveError (const File& fileThatFailedToWrite)
            : message ("Can't write to the file: " + fileThatFailedToWrite.getFullPathName())
        {}

        String message;
    };

    void createPropertyEditors (PropertyListBuilder&);
    void addSettingsForProjectType (const ProjectType&);

    //==============================================================================
    void copyMainGroupFromProject();
    Array<Project::Item>& getAllGroups() noexcept               { jassert (itemGroups.size() > 0); return itemGroups; }
    const Array<Project::Item>& getAllGroups() const noexcept   { jassert (itemGroups.size() > 0); return itemGroups; }
    Project::Item& getModulesGroup();

    //==============================================================================
    StringArray linuxLibs, linuxPackages, makefileExtraLinkerFlags;

    //==============================================================================
    StringPairArray msvcExtraPreprocessorDefs;
    String msvcDelayLoadedDLLs;
    StringArray mingwLibs, windowsLibs;

    //==============================================================================
    StringArray androidLibs;

    //==============================================================================
    StringArray extraSearchPaths;
    StringArray moduleLibSearchPaths;

    //==============================================================================
    class BuildConfiguration  : public ReferenceCountedObject
    {
    public:
        BuildConfiguration (Project& project, const ValueTree& configNode, const ProjectExporter&);
        ~BuildConfiguration();

        typedef ReferenceCountedObjectPtr<BuildConfiguration> Ptr;

        //==============================================================================
        virtual void createConfigProperties (PropertyListBuilder&) = 0;
        virtual var getDefaultOptimisationLevel() const = 0;
        virtual String getModuleLibraryArchName() const = 0;

        //==============================================================================
        Value getNameValue()                                { return getValue (Ids::name); }
        String getName() const                              { return config [Ids::name]; }

        Value isDebugValue()                                { return getValue (Ids::isDebug); }
        bool isDebug() const                                { return config [Ids::isDebug]; }

        Value getTargetBinaryName()                         { return getValue (Ids::targetName); }
        String getTargetBinaryNameString() const            { return config [Ids::targetName]; }

        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath()                 { return getValue (Ids::binaryPath); }
        String getTargetBinaryRelativePathString() const    { return config [Ids::binaryPath]; }

        Value getOptimisationLevel()                        { return getValue (Ids::optimisation); }
        int getOptimisationLevelInt() const                 { return config [Ids::optimisation]; }
        String getGCCOptimisationFlag() const;

        Value getLinkTimeOptimisationEnabledValue()         { return getValue (Ids::linkTimeOptimisation); }
        bool isLinkTimeOptimisationEnabled() const          { return config [Ids::linkTimeOptimisation]; }

        Value getBuildConfigPreprocessorDefs()              { return getValue (Ids::defines); }
        String getBuildConfigPreprocessorDefsString() const { return config [Ids::defines]; }
        StringPairArray getAllPreprocessorDefs() const;    // includes inherited definitions
        StringPairArray getUniquePreprocessorDefs() const; // returns pre-processor definitions that are not already in the project pre-processor defs

        Value getHeaderSearchPathValue()                    { return getValue (Ids::headerPath); }
        String getHeaderSearchPathString() const            { return config [Ids::headerPath]; }
        StringArray getHeaderSearchPaths() const;

        Value getLibrarySearchPathValue()                   { return getValue (Ids::libraryPath); }
        String getLibrarySearchPathString() const           { return config [Ids::libraryPath]; }
        StringArray getLibrarySearchPaths() const;
        String getGCCLibraryPathFlags() const;

        Value getUserNotes()                                { return getValue (Ids::userNotes); }

        Value getValue (const Identifier& nm)               { return config.getPropertyAsValue (nm, getUndoManager()); }
        UndoManager* getUndoManager() const                 { return project.getUndoManagerFor (config); }

        void createPropertyEditors (PropertyListBuilder&);
        void addGCCOptimisationProperty (PropertyListBuilder&);
        void removeFromExporter();

        //==============================================================================
        ValueTree config;
        Project& project;
        const ProjectExporter& exporter;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuildConfiguration)
    };

    void addNewConfiguration (const BuildConfiguration* configToCopy);
    bool hasConfigurationNamed (const String& name) const;
    String getUniqueConfigName (String name) const;

    String getExternalLibraryFlags (const BuildConfiguration& config) const;

    //==============================================================================
    struct ConfigIterator
    {
        ConfigIterator (ProjectExporter& exporter);

        bool next();

        BuildConfiguration& operator*() const       { return *config; }
        BuildConfiguration* operator->() const      { return config; }

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
        const BuildConfiguration* operator->() const      { return config; }

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
    Value getExporterPreprocessorDefs()                 { return getSetting (Ids::extraDefs); }
    String getExporterPreprocessorDefsString() const    { return getSettingString (Ids::extraDefs); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const BuildConfiguration& config, const ProjectType::Target::Type targetType) const;
    // includes exporter + project defs..
    StringPairArray getAllPreprocessorDefs() const;

    void addTargetSpecificPreprocessorDefs (StringPairArray& defs, const ProjectType::Target::Type targetType) const;

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

protected:
    //==============================================================================
    String name;
    Project& project;
    const ProjectType& projectType;
    const String projectName;
    const File projectFolder;
    Value vst3Path, rtasPath, aaxPath; // these must be initialised in the specific exporter c'tors!

    mutable Array<Project::Item> itemGroups;
    void initItemGroups() const;
    Project::Item* modulesGroup = nullptr;

    virtual BuildConfiguration::Ptr createBuildConfig (const ValueTree&) const = 0;

    void addDefaultPreprocessorDefs (StringPairArray&) const;

    static String getDefaultBuildsRootFolder()            { return "Builds/"; }

    static String getStaticLibbedFilename (String name)
    {
        return addSuffix (addLibPrefix (name), ".a");
    }

    static String getDynamicLibbedFilename (String name)
    {
        return addSuffix (addLibPrefix (name), ".so");
    }

    virtual void addPlatformSpecificSettingsForProjectType (const ProjectType&) = 0;

    //==============================================================================
    static void overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData)
    {
        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (file, newData))
            throw SaveError (file);
    }

    static void overwriteFileIfDifferentOrThrow (const File& file, const String& newData)
    {
        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (file, newData))
            throw SaveError (file);
    }

    static void createDirectoryOrThrow (const File& dirToCreate)
    {
        if (! dirToCreate.createDirectory())
            throw SaveError ("Can't create folder: " + dirToCreate.getFullPathName());
    }

    static void writeXmlOrThrow (const XmlElement& xml, const File& file, const String& encoding, int maxCharsPerLine, bool useUnixNewLines = false)
    {
        MemoryOutputStream mo;
        xml.writeToStream (mo, String(), false, true, encoding, maxCharsPerLine);

        if (useUnixNewLines)
        {
            MemoryOutputStream mo2;
            mo2 << mo.toString().replace ("\r\n", "\n");
            overwriteFileIfDifferentOrThrow (file, mo2);
        }
        else
        {
            overwriteFileIfDifferentOrThrow (file, mo);
        }
    }

    static Image rescaleImageForIcon (Drawable&, int iconSize);

private:
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

    void createDependencyPathProperties (PropertyListBuilder&);
    void createIconProperties (PropertyListBuilder&);
    void addVSTPathsIfPluginOrHost();
    void addCommonAudioPluginSettings();
    void addVST3FolderToPath();
    void addAAXFoldersToPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter)
};
