/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_PROJECTEXPORTER_JUCEHEADER__
#define __JUCER_PROJECTEXPORTER_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Project/jucer_Project.h"
#include "../Project/jucer_ProjectType.h"


//==============================================================================
class ProjectExporter
{
public:
    ProjectExporter (Project&, const ValueTree& settings);
    virtual ~ProjectExporter();

    static int getNumExporters();
    static StringArray getExporterNames();

    static ProjectExporter* createNewExporter (Project&, const int index);
    static ProjectExporter* createNewExporter (Project&, const String& name);
    static ProjectExporter* createExporter (Project&, const ValueTree& settings);
    static ProjectExporter* createPlatformDefaultExporter (Project&);

    static StringArray getDefaultExporters();

    //=============================================================================
    // return 0 if this can't be opened in the current OS, or a higher value, where higher numbers are more preferable.
    virtual int getLaunchPreferenceOrderForCurrentOS() = 0;
    virtual bool isPossibleForCurrentProject() = 0;
    virtual bool usesMMFiles() const = 0;
    virtual void createPropertyEditors (PropertyListBuilder&);
    virtual void launchProject() = 0;
    virtual void create() = 0; // may throw a SaveError
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;
    virtual bool canCopeWithDuplicateFiles() = 0;

    virtual bool isXcode() const            { return false; }
    virtual bool isVisualStudio() const     { return false; }
    virtual bool isLinux() const            { return false; }
    virtual bool isOSX() const              { return false; }

    //==============================================================================
    String getName() const                      { return name; }
    File getTargetFolder() const;

    Project& getProject() noexcept              { return project; }
    const Project& getProject() const noexcept  { return project; }

    const ValueTree& getSettings() const                { return settings; }
    Value getSetting (const Identifier& name_) const    { return settings.getPropertyAsValue (name_, project.getUndoManagerFor (settings)); }

    Value getJuceFolder() const                 { return getSetting (Ids::juceFolder); }
    Value getTargetLocation() const             { return getSetting (Ids::targetFolder); }

    Value getExtraCompilerFlags() const         { return getSetting (Ids::extraCompilerFlags); }
    Value getExtraLinkerFlags() const           { return getSetting (Ids::extraLinkerFlags); }

    // This adds the quotes, and may return angle-brackets, eg: <foo/bar.h> or normal quotes.
    String getIncludePathForFileInJuceFolder (const String& pathFromJuceFolder, const File& targetIncludeFile) const;

    RelativePath rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const;
    void addToExtraSearchPaths (const RelativePath& pathFromProjectFolder);

    Value getBigIconImageItemID() const         { return getSetting (Ids::bigIcon); }
    Value getSmallIconImageItemID() const       { return getSetting (Ids::smallIcon); }
    Image getBigIcon();
    Image getSmallIcon();
    Image getBestIconForSize (int size, bool returnNullIfNothingBigEnough);

    String getExporterIdentifierMacro() const
    {
        return "JUCER_" + settings.getType().toString() + "_"
                + String::toHexString (settings [Ids::targetFolder].toString().hashCode()).toUpperCase();
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

    RelativePath getJucePathFromTargetFolder() const;
    RelativePath getJucePathFromProjectFolder() const;

    //==============================================================================
    Array<Project::Item> groups;
    Project::Item& getModulesGroup();

    //==============================================================================
    String xcodePackageType, xcodeBundleSignature, xcodeBundleExtension;
    String xcodeProductType, xcodeProductInstallPath, xcodeFileType;
    String xcodeShellScript, xcodeShellScriptTitle, xcodeOtherRezFlags;
    bool xcodeIsBundle, xcodeCreatePList, xcodeCanUseDwarf;
    StringArray xcodeFrameworks;
    Array<RelativePath> xcodeExtraLibrariesDebug, xcodeExtraLibrariesRelease;
    Array<XmlElement> xcodeExtraPListEntries;

    //==============================================================================
    String makefileTargetSuffix;
    bool makefileIsDLL;

    //==============================================================================
    String msvcTargetSuffix;
    StringPairArray msvcExtraPreprocessorDefs;
    bool msvcIsDLL, msvcIsWindowsSubsystem, msvcNeedsDLLRuntimeLib;
    String msvcDelayLoadedDLLs;

    //==============================================================================
    StringArray extraSearchPaths;

    //==============================================================================
    class BuildConfiguration  : public ReferenceCountedObject
    {
    public:
        BuildConfiguration (Project& project, const ValueTree& configNode);
        ~BuildConfiguration();

        typedef ReferenceCountedObjectPtr<BuildConfiguration> Ptr;

        //==============================================================================
        virtual void createPropertyEditors (PropertyListBuilder&) = 0;

        //==============================================================================
        Value getName() const                               { return getValue (Ids::name); }
        Value isDebug() const                               { return getValue (Ids::isDebug); }
        Value getTargetBinaryName() const                   { return getValue (Ids::targetName); }
        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath() const           { return getValue (Ids::binaryPath); }
        Value getOptimisationLevel() const                  { return getValue (Ids::optimisation); }
        String getGCCOptimisationFlag() const;
        Value getBuildConfigPreprocessorDefs() const        { return getValue (Ids::defines); }
        StringPairArray getAllPreprocessorDefs() const; // includes inherited definitions
        Value getHeaderSearchPath() const                   { return getValue (Ids::headerPath); }
        StringArray getHeaderSearchPaths() const;
        Value getLibrarySearchPath() const                  { return getValue (Ids::libraryPath); }
        StringArray getLibrarySearchPaths() const;
        String getGCCLibraryPathFlags() const;

        //==============================================================================
        ValueTree config;

        //==============================================================================
        String msvcExtraLinkerOptions, msvcModuleDefinitionFile;
        String msvcPreBuildCommand, msvcPostBuildCommand;

    protected:
        Project& project;

        Value getValue (const Identifier& name) const       { return config.getPropertyAsValue (name, getUndoManager()); }
        UndoManager* getUndoManager() const                 { return project.getUndoManagerFor (config); }

        void createBasicPropertyEditors (PropertyListBuilder&);

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuildConfiguration);
    };

    void addNewConfiguration (const BuildConfiguration* configToCopy);
    void deleteConfiguration (int index);
    bool hasConfigurationNamed (const String& name) const;
    String getUniqueConfigName (String name) const;

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
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigIterator);
    };

    int getNumConfigurations() const;
    BuildConfiguration::Ptr getConfiguration (int index) const;

    ValueTree getConfigurations() const;
    void createDefaultConfigs();

    static const Identifier configurations, configuration;

    //==============================================================================
    Value getExporterPreprocessorDefs() const   { return getSetting (Ids::extraDefs); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const BuildConfiguration& config) const;
    // includes exporter + project defs..
    StringPairArray getAllPreprocessorDefs() const;

    String replacePreprocessorTokens (const BuildConfiguration&, const String& sourceString) const;

    ValueTree settings;

protected:
    //==============================================================================
    String name;
    Project& project;
    const ProjectType& projectType;
    const String projectName;
    const File projectFolder;
    Project::Item* modulesGroup;

    virtual BuildConfiguration::Ptr createBuildConfig (const ValueTree&) const = 0;

    static String getDefaultBuildsRootFolder()            { return "Builds/"; }

    static String getLibbedFilename (String name)
    {
        if (! name.startsWith ("lib"))
            name = "lib" + name;
        if (! name.endsWithIgnoreCase (".a"))
            name = name + ".a";
        return name;
    }

    //==============================================================================
    static void overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData)
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
        xml.writeToStream (mo, String::empty, false, true, encoding, maxCharsPerLine);

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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter);
};


#endif   // __JUCER_PROJECTEXPORTER_JUCEHEADER__
