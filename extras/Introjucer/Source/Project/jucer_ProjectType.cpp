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

#include "jucer_ProjectType.h"

//==============================================================================
static ProjectType_GUIApp guiType;
static ProjectType_ConsoleApp consoleType;
static ProjectType_StaticLibrary staticLibType;
static ProjectType_AudioPlugin audioPluginType;

//==============================================================================
ProjectType::ProjectType (const String& type_, const String& desc_)
    : type (type_), desc (desc_)
{
    getAllTypes().add (this);
}

ProjectType::~ProjectType()
{
    getAllTypes().removeValue (this);
}

Array<ProjectType*>& ProjectType::getAllTypes()
{
    static Array<ProjectType*> types;
    return types;
}

const ProjectType* ProjectType::findType (const String& typeCode)
{
    const Array<ProjectType*>& types = getAllTypes();

    for (int i = types.size(); --i >= 0;)
        if (types.getUnchecked(i)->getType() == typeCode)
            return types.getUnchecked(i);

    jassertfalse;
    return nullptr;
}


//==============================================================================
Result ProjectType_GUIApp::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return Result::ok();
}

//==============================================================================
Result ProjectType_ConsoleApp::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return Result::ok();
}

//==============================================================================
Result ProjectType_StaticLibrary::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return Result::ok();
}

//==============================================================================
Result ProjectType_AudioPlugin::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return Result::ok();
}
