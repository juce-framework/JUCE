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

#ifndef JUCER_PROJECTTYPE_H_INCLUDED
#define JUCER_PROJECTTYPE_H_INCLUDED

#include "../jucer_Headers.h"
class Project;
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
    virtual bool isStaticLibrary() const        { return false; }
    virtual bool isDynamicLibrary() const       { return false; }
    virtual bool isGUIApplication() const       { return false; }
    virtual bool isCommandLineApp() const       { return false; }
    virtual bool isAudioPlugin() const          { return false; }
    virtual bool isBrowserPlugin() const        { return false; }

    static const char* getGUIAppTypeName();
    static const char* getConsoleAppTypeName();
    static const char* getStaticLibTypeName();
    static const char* getDynamicLibTypeName();
    static const char* getAudioPluginTypeName();


    virtual void setMissingProjectProperties (Project&) const = 0;
    virtual void createPropertyEditors (Project&, PropertyListBuilder&) const = 0;
    virtual void prepareExporter (ProjectExporter&) const = 0;

protected:
    ProjectType (const String& type, const String& desc);

private:
    const String type, desc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectType)
};


#endif   // JUCER_PROJECTTYPE_H_INCLUDED
