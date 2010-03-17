/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    virtual const String getOSTestMacro() = 0;
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;

    //==============================================================================
    const String getName() const            { return name; }
    const File getTargetFolder() const;

    const ValueTree& getSettings() const                    { return settings; }
    Value getSetting (const var::identifier& name) const    { return settings.getPropertyAsValue (name, project.getUndoManagerFor (settings)); }

    Value getJuceFolder() const             { return getSetting ("juceFolder"); }
    Value getTargetLocation() const         { return getSetting ("targetFolder"); }

    Value getVSTFolder() const              { return getSetting ("vstFolder"); }
    Value getRTASFolder() const             { return getSetting ("rtasFolder"); }
    Value getAUFolder() const               { return getSetting ("auFolder"); }

    bool isVST() const                      { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildVST().getValue(); }
    bool isRTAS() const                     { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildRTAS().getValue(); }
    bool isAU() const                       { return (bool) project.isAudioPlugin() && (bool) project.shouldBuildAU().getValue(); }

    Array<RelativePath> juceWrapperFiles;

protected:
    //==============================================================================
    Project& project;
    ValueTree settings;
    String name;

    const RelativePath getJucePathFromTargetFolder() const;
    const String getDefaultBuildsRootFolder() const         { return "Builds/"; }

    const Array<RelativePath> getVSTFilesRequired() const;

    const String getLibbedFilename (String name) const
    {
        if (! name.startsWith (T("lib")))
            name = "lib" + name;
        if (! name.endsWithIgnoreCase (T(".a")))
            name = name + ".a";
        return name;
    }
};


#endif   // __JUCER_PROJECTEXPORTER_JUCEHEADER__
