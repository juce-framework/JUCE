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

#ifndef __JUCER_PROJECTTYPE_JUCEHEADER__
#define __JUCER_PROJECTTYPE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"
class ProjectExporter;
class ProjectSaver;

//==============================================================================
class LibraryModule
{
public:
    LibraryModule();
    virtual ~LibraryModule() {}

    virtual void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const = 0;
    virtual void getHeaderFiles (Project& project, StringArray& includePaths, StringArray& headerGuards) = 0;
    virtual void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const = 0;
    virtual void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const = 0;
    virtual void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const = 0;
};


//==============================================================================
class ProjectType
{
public:
    //==============================================================================
    virtual ~ProjectType();

    const String& getType() const noexcept          { return type; }
    const String& getDescription() const noexcept   { return desc; }

    //==============================================================================
    static Array<ProjectType*>& getAllTypes();
    static const ProjectType* findType (const String& typeCode);

    //==============================================================================
    virtual bool isLibrary() const              { return false; }
    virtual bool isGUIApplication() const       { return false; }
    virtual bool isCommandLineApp() const       { return false; }
    virtual bool isAudioPlugin() const          { return false; }
    virtual bool isBrowserPlugin() const        { return false; }

    static const char* getGUIAppTypeName();
    static const char* getConsoleAppTypeName();
    static const char* getAudioPluginTypeName();

    virtual void prepareExporter (ProjectExporter& exporter) const;
    virtual void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const;

protected:
    ProjectType (const String& type, const String& desc);

private:
    const String type, desc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectType);
};


#endif  // __JUCE_PROJECTTYPE_H_C1C6BC3E__
