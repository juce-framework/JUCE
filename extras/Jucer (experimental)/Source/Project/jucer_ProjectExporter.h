/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "jucer_Project.h"


//==============================================================================
class ProjectExporter
{
protected:
    //==============================================================================
    ProjectExporter (Project& project, const ValueTree& settings);

public:
    virtual ~ProjectExporter();

    static int getNumExporters();
    static const StringArray getExporterNames();
    static ProjectExporter* createNewExporter (Project& project, const int index);

    static ProjectExporter* createExporter (Project& project, const ValueTree& settings);
    static ProjectExporter* createPlatformDefaultExporter (Project& project);

    //=============================================================================
    virtual bool isDefaultFormatForCurrentOS() = 0;
    virtual bool isPossibleForCurrentProject() = 0;
    virtual bool usesMMFiles() const = 0;
    virtual void createPropertyEditors (Array <PropertyComponent*>& props);
    virtual void launchProject() = 0;
    virtual const String create() = 0;
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;

    //==============================================================================
    const String getName() const            { return name; }
    const File getTargetFolder() const;

    const ValueTree& getSettings() const                { return settings; }
    Value getSetting (const Identifier& name_) const    { return settings.getPropertyAsValue (name_, project.getUndoManagerFor (settings)); }

    Value getJuceFolder() const             { return getSetting (Ids::juceFolder); }
    Value getTargetLocation() const         { return getSetting (Ids::targetFolder); }

    Value getVSTFolder() const              { return getSetting (Ids::vstFolder); }
    Value getRTASFolder() const             { return getSetting (Ids::rtasFolder); }
    Value getAUFolder() const               { return getSetting (Ids::auFolder); }

    bool isVST() const                      { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildVST().getValue(); }
    bool isRTAS() const                     { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildRTAS().getValue(); }
    bool isAU() const                       { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildAU().getValue(); }

    Value getExtraCompilerFlags() const     { return getSetting (Ids::extraCompilerFlags); }
    Value getExtraLinkerFlags() const       { return getSetting (Ids::extraLinkerFlags); }

    Value getExporterPreprocessorDefs() const   { return getSetting (Ids::extraDefs); }
    const StringPairArray getAllPreprocessorDefs (const Project::BuildConfiguration& config) const;  // includes inherited ones..

    const String replacePreprocessorTokens (const Project::BuildConfiguration& config,
                                            const String& sourceString) const;

    // This adds the quotes, and may return angle-brackets, eg: <foo/bar.h> or normal quotes.
    const String getIncludePathForFileInJuceFolder (const String& pathFromJuceFolder, const File& targetIncludeFile) const;

    const String getExporterIdentifierMacro() const
    {
        return "JUCER_" + settings.getType() + "_"
                + String::toHexString (settings [Ids::targetFolder].toString().hashCode()).toUpperCase();
    }

    Array<RelativePath> juceWrapperFiles;
    RelativePath juceWrapperFolder;

protected:
    //==============================================================================
    Project& project;
    ValueTree settings;
    String name;

    const RelativePath getJucePathFromTargetFolder() const;

    const String getDefaultBuildsRootFolder() const         { return "Builds/"; }

    const Array<RelativePath> getVSTFilesRequired() const;

    static const String getLibbedFilename (String name)
    {
        if (! name.startsWith ("lib"))
            name = "lib" + name;
        if (! name.endsWithIgnoreCase (".a"))
            name = name + ".a";
        return name;
    }

    const RelativePath rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter);
};


#endif   // __JUCER_PROJECTEXPORTER_JUCEHEADER__
