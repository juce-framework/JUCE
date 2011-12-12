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
protected:
    //==============================================================================
    ProjectExporter (Project& project, const ValueTree& settings);

public:
    virtual ~ProjectExporter();

    static int getNumExporters();
    static StringArray getExporterNames();
    static ProjectExporter* createNewExporter (Project& project, const int index);
    static ProjectExporter* createNewExporter (Project& project, const String& name);

    static ProjectExporter* createExporter (Project& project, const ValueTree& settings);
    static ProjectExporter* createPlatformDefaultExporter (Project& project);
    static StringArray getDefaultExporters();

    //=============================================================================
    // return 0 if this can't be opened in the current OS, or a higher value, where higher numbers are more preferable.
    virtual int getLaunchPreferenceOrderForCurrentOS() = 0;
    virtual bool isPossibleForCurrentProject() = 0;
    virtual bool usesMMFiles() const = 0;
    virtual void createPropertyEditors (Array <PropertyComponent*>& props);
    virtual void launchProject() = 0;
    virtual void create() = 0; // may throw a SaveError
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;
    virtual bool canCopeWithDuplicateFiles() = 0;

    virtual bool isXcode() const            { return false; }
    virtual bool isVisualStudio() const     { return false; }
    virtual bool isLinux() const            { return false; }
    virtual bool isOSX() const              { return false; }

    //==============================================================================
    String getName() const                  { return name; }
    File getTargetFolder() const;

    Project& getProject() noexcept              { return project; }
    const Project& getProject() const noexcept  { return project; }

    const ValueTree& getSettings() const                { return settings; }
    Value getSetting (const Identifier& name_) const    { return settings.getPropertyAsValue (name_, project.getUndoManagerFor (settings)); }

    Value getJuceFolder() const             { return getSetting (Ids::juceFolder); }
    Value getTargetLocation() const         { return getSetting (Ids::targetFolder); }

    Value getExtraCompilerFlags() const     { return getSetting (Ids::extraCompilerFlags); }
    Value getExtraLinkerFlags() const       { return getSetting (Ids::extraLinkerFlags); }

    Value getExporterPreprocessorDefs() const   { return getSetting (Ids::extraDefs); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const Project::BuildConfiguration& config) const;
    // includes exporter + project defs..
    StringPairArray getAllPreprocessorDefs() const;

    String replacePreprocessorTokens (const Project::BuildConfiguration& config,
                                      const String& sourceString) const;

    // This adds the quotes, and may return angle-brackets, eg: <foo/bar.h> or normal quotes.
    String getIncludePathForFileInJuceFolder (const String& pathFromJuceFolder, const File& targetIncludeFile) const;

    RelativePath rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const;
    void addToExtraSearchPaths (const RelativePath& pathFromProjectFolder);

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
    String msvcExtraLinkerOptions, msvcDelayLoadedDLLs, msvcModuleDefinitionFile;
    String msvcPostBuildCommand, msvcPostBuildOutputs;
    String msvcPreBuildCommand;

    //==============================================================================
    StringArray androidDynamicLibs;

    //==============================================================================
    StringArray extraSearchPaths;

protected:
    //==============================================================================
    String name;
    Project& project;
    const ProjectType& projectType;
    const String projectName;
    const File projectFolder;
    Array<Project::BuildConfiguration> configs;
    ValueTree settings;
    Project::Item* modulesGroup;

    static String getDefaultBuildsRootFolder()            { return "Builds/"; }

    static String getLibbedFilename (String name)
    {
        if (! name.startsWith ("lib"))
            name = "lib" + name;
        if (! name.endsWithIgnoreCase (".a"))
            name = name + ".a";
        return name;
    }

    Image getBestIconForSize (int size, bool returnNullIfNothingBigEnough);

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
