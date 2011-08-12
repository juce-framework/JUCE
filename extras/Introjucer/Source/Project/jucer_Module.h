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

#ifndef __JUCER_MODULE_JUCEHEADER__
#define __JUCER_MODULE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"
class ProjectExporter;
class ProjectSaver;

//==============================================================================
class LibraryModule
{
public:
    LibraryModule() {}
    virtual ~LibraryModule() {}

    virtual String getID() const = 0;
    virtual void prepareExporter (ProjectExporter&, ProjectSaver&) const = 0;
    virtual void writeIncludes (Project&, OutputStream& out) = 0;
    virtual void createPropertyEditors (const ProjectExporter&, Array <PropertyComponent*>&) const = 0;
    virtual void getConfigFlags (Project&, OwnedArray<Project::ConfigFlag>&) const = 0;
};

//==============================================================================
class ModuleList
{
public:
    ModuleList();

    static ModuleList& getInstance();

    //==============================================================================
    void rescan();
    LibraryModule* loadModule (const String& uid) const;

    void getDependencies (const String& moduleID, StringArray& dependencies) const;
    void createDependencies (const String& moduleID, OwnedArray<LibraryModule>& modules) const;

    //==============================================================================
    struct Module
    {
        LibraryModule* create() const;

        String uid, name, description;
        File file;
    };

    const Module* findModuleInfo (const String& uid) const;

    OwnedArray<Module> modules;

private:
    File moduleFolder;
};


#endif   // __JUCER_MODULE_JUCEHEADER__
