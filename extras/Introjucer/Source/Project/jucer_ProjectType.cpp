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
#include "jucer_ProjectExporter.h"

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
Result ProjectType::createRequiredFiles (Project::Item& projectRoot,
                                         Array<File>& filesCreated) const
{
    return Result::ok();
}

namespace
{
    void addVSTFolderToPath (const ProjectExporter& exporter, StringArray& searchPaths)
    {
        const String vstFolder (exporter.getVSTFolder().toString());

        if (vstFolder.isNotEmpty())
        {
            RelativePath path (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (vstFolder, RelativePath::projectFolder)));

            if (exporter.isVisualStudio())
                searchPaths.add (path.toWindowsStyle());
            else if (exporter.isLinux() || exporter.isXcode())
                searchPaths.insert (0, path.toUnixStyle());
        }
    }

    void createVSTPathEditor (const ProjectExporter& exporter, Array <PropertyComponent*>& props)
    {
        props.add (new TextPropertyComponent (exporter.getVSTFolder(), "VST Folder", 1024, false));
        props.getLast()->setTooltip ("If you're building a VST, this must be the folder containing the VST SDK. This should be an absolute path.");
    }
}

void ProjectType::addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
{
    if (exporter.getProject().isJuceConfigFlagEnabled ("JUCE_PLUGINHOST_VST"))
        addVSTFolderToPath (exporter, paths);
}

void ProjectType::createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
{
    if (exporter.getProject().isJuceConfigFlagEnabled ("JUCE_PLUGINHOST_VST"))
        createVSTPathEditor (exporter, props);
}

//==============================================================================
Result ProjectType_GUIApp::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return ProjectType::createRequiredFiles (projectRoot, filesCreated);
}

//==============================================================================
Result ProjectType_ConsoleApp::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return ProjectType::createRequiredFiles (projectRoot, filesCreated);
}

//==============================================================================
Result ProjectType_StaticLibrary::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return ProjectType::createRequiredFiles (projectRoot, filesCreated);
}

//==============================================================================
Result ProjectType_AudioPlugin::createRequiredFiles (Project::Item& projectRoot, Array<File>& filesCreated) const
{
    return ProjectType::createRequiredFiles (projectRoot, filesCreated);
}

void ProjectType_AudioPlugin::createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
{
    if (exporter.isVST())
        createVSTPathEditor (exporter, props);

    if (exporter.isRTAS())
    {
        props.add (new TextPropertyComponent (exporter.getRTASFolder(), "RTAS Folder", 1024, false));
        props.getLast()->setTooltip ("If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
    }
}

void ProjectType_AudioPlugin::addExtraSearchPaths (const ProjectExporter& exporter, StringArray& searchPaths) const
{
    const Project& project = exporter.getProject();
    RelativePath juceWrapperFolder (project.getWrapperFolder(), exporter.getTargetFolder(), RelativePath::buildTargetFolder);

    if (exporter.isVST())
        addVSTFolderToPath (exporter, searchPaths);

    if (exporter.isVisualStudio())
        searchPaths.add (juceWrapperFolder.toWindowsStyle());
    else if (exporter.isLinux() && exporter.isVST())
        searchPaths.insert (0, juceWrapperFolder.toUnixStyle());

    if (exporter.isAU() && exporter.isXcode())
    {
        searchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/PublicUtility");
        searchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/Utility");
    }

    if (exporter.isRTAS())
    {
        RelativePath sdkFolder (exporter.getRTASFolder().toString(), RelativePath::projectFolder);

        if (exporter.isVisualStudio())
        {
            const char* paths[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses/Interfaces",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/Utilities",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/RTASP_Adapt",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/CoreClasses",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/Controls",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/Meters",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/ViewClasses",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/DSPClasses",
                                    "AlturaPorts/TDMPlugins/PluginLibrary/Interfaces",
                                    "AlturaPorts/TDMPlugins/common",
                                    "AlturaPorts/TDMPlugins/common/Platform",
                                    "AlturaPorts/TDMPlugins/SignalProcessing/Public",
                                    "AlturaPorts/TDMPlugIns/DSPManager/Interfaces",
                                    "AlturaPorts/SADriver/Interfaces",
                                    "AlturaPorts/DigiPublic/Interfaces",
                                    "AlturaPorts/Fic/Interfaces/DAEClient",
                                    "AlturaPorts/NewFileLibs/Cmn",
                                    "AlturaPorts/NewFileLibs/DOA",
                                    "AlturaPorts/AlturaSource/PPC_H",
                                    "AlturaPorts/AlturaSource/AppSupport",
                                    "AvidCode/AVX2sdk/AVX/avx2/avx2sdk/inc",
                                    "xplat/AVX/avx2/avx2sdk/inc" };

            for (int i = 0; i < numElementsInArray (paths); ++i)
                searchPaths.add (exporter.rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (paths[i])).toWindowsStyle());
        }
        else if (exporter.isXcode())
        {
            searchPaths.add ("/Developer/Headers/FlatCarbon");

            const char* paths[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/CoreClasses",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/DSPClasses",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/MacBuild",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/Meters",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses/Interfaces",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/RTASP_Adapt",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/Utilities",
                                    "AlturaPorts/TDMPlugIns/PlugInLibrary/ViewClasses",
                                    "AlturaPorts/TDMPlugIns/DSPManager/**",
                                    "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/Encryption",
                                    "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/GraphicsExtensions",
                                    "AlturaPorts/TDMPlugIns/common",
                                    "AlturaPorts/TDMPlugIns/common/PI_LibInterface",
                                    "AlturaPorts/TDMPlugIns/PACEProtection/**",
                                    "AlturaPorts/TDMPlugIns/SignalProcessing/**",
                                    "AlturaPorts/OMS/Headers",
                                    "AlturaPorts/Fic/Interfaces/**",
                                    "AlturaPorts/Fic/Source/SignalNets",
                                    "AlturaPorts/DSIPublicInterface/PublicHeaders",
                                    "DAEWin/Include",
                                    "AlturaPorts/DigiPublic/Interfaces",
                                    "AlturaPorts/DigiPublic",
                                    "AlturaPorts/NewFileLibs/DOA",
                                    "AlturaPorts/NewFileLibs/Cmn",
                                    "xplat/AVX/avx2/avx2sdk/inc",
                                    "xplat/AVX/avx2/avx2sdk/utils" };

            for (int i = 0; i < numElementsInArray (paths); ++i)
                searchPaths.add (exporter.rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (paths[i])).toUnixStyle());
        }
    }
}
