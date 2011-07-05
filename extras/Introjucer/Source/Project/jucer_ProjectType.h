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

    virtual Result createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const;
    virtual void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const;
    virtual void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const;

protected:
    ProjectType (const String& type, const String& desc);

private:
    const String type, desc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectType);
};

//==============================================================================
class ProjectType_GUIApp  : public ProjectType
{
public:
    static const char* getTypeName() noexcept    { return "guiapp"; }
    ProjectType_GUIApp()    : ProjectType (getTypeName(), "Application (GUI)") {}

    bool isGUIApplication() const   { return true; }
    Result createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const;
};

//==============================================================================
class ProjectType_ConsoleApp  : public ProjectType
{
public:
    static const char* getTypeName() noexcept    { return "consoleapp"; }
    ProjectType_ConsoleApp()    : ProjectType (getTypeName(), "Application (Non-GUI)") {}

    Result createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const;
    bool isCommandLineApp() const   { return true; }
};

//==============================================================================
class ProjectType_StaticLibrary  : public ProjectType
{
public:
    static const char* getTypeName() noexcept    { return "library"; }
    ProjectType_StaticLibrary()    : ProjectType (getTypeName(), "Static Library") {}

    bool isLibrary() const          { return true; }
    Result createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const;
};

//==============================================================================
class ProjectType_AudioPlugin  : public ProjectType
{
public:
    static const char* getTypeName() noexcept    { return "audioplug"; }
    ProjectType_AudioPlugin()    : ProjectType (getTypeName(), "Audio Plug-in") {}

    bool isAudioPlugin() const      { return true; }
    Result createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const;
    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const;
    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const;
};


#endif  // __JUCE_PROJECTTYPE_H_C1C6BC3E__
