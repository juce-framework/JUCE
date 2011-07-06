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
LibraryModule::LibraryModule()
{
}

//==============================================================================
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


//==============================================================================
class JuceLibraryModule  : public LibraryModule
{
public:
    JuceLibraryModule() {}

    void addExtraCodeGroups (const ProjectExporter& exporter, Array<Project::Item>& groups) const
    {
    }

    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
    {
        if (exporter.getProject().isConfigFlagEnabled ("JUCE_PLUGINHOST_VST"))
            addVSTFolderToPath (exporter, paths);
    }

    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
    {
        if (exporter.getProject().isConfigFlagEnabled ("JUCE_PLUGINHOST_VST"))
            createVSTPathEditor (exporter, props);
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags)
    {
        if (project.getJuceLinkageMode() == Project::notLinkedToJuce)
            return;

        StringArray lines;
        project.getLocalJuceFolder().getChildFile ("juce_Config.h").readLines (lines);

        for (int i = 0; i < lines.size(); ++i)
        {
            String line (lines[i].trim());

            if (line.startsWith ("/** ") && line.containsChar (':'))
            {
                ScopedPointer <Project::ConfigFlag> config (new Project::ConfigFlag());
                config->symbol = line.substring (4).upToFirstOccurrenceOf (":", false, false).trim();

                if (config->symbol.length() > 4)
                {
                    config->description = line.fromFirstOccurrenceOf (":", false, false).trimStart();
                    ++i;
                    while (! (lines[i].contains ("*/") || lines[i].contains ("@see")))
                    {
                        if (lines[i].trim().isNotEmpty())
                            config->description = config->description.trim() + " " + lines[i].trim();

                        ++i;
                    }

                    config->description = config->description.upToFirstOccurrenceOf ("*/", false, false);
                    flags.add (config.release());
                }
            }
        }
    }
};

//==============================================================================
class VSTLibraryModule  : public LibraryModule
{
public:
    VSTLibraryModule() {}

    void addExtraCodeGroups (const ProjectExporter& exporter, Array<Project::Item>& groups) const
    {
        Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(), "Juce VST Wrapper"));
        group.setID ("__jucevstfiles");

        const char* osxFiles[] = { JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.cpp",
                                   JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.mm", 0 };

        const char* winFiles[] = { JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.cpp", 0};

        for (const char** f = (exporter.isXcode() ? osxFiles : winFiles); *f != 0; ++f)
            group.addRelativeFile (exporter.getJucePathFromProjectFolder().getChildFile (*f), -1, true);

        groups.add (group);
    }

    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
    {
        RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                        exporter.getTargetFolder(), RelativePath::buildTargetFolder);

        addVSTFolderToPath (exporter, paths);

        if (exporter.isVisualStudio())
            paths.add (juceWrapperFolder.toWindowsStyle());
        else if (exporter.isLinux())
            paths.add (juceWrapperFolder.toUnixStyle());
    }

    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
    {
        createVSTPathEditor (exporter, props);
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags)
    {
    }
};

//==============================================================================
class RTASLibraryModule  : public LibraryModule
{
public:
    RTASLibraryModule() {}

    void addExtraCodeGroups (const ProjectExporter& exporter, Array<Project::Item>& groups) const
    {
        if (exporter.isXcode() || exporter.isVisualStudio())
        {
            Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(), "Juce RTAS Wrapper"));
            group.setID ("__jucertasfiles");

            const char* osxFiles[] = { JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode1.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode2.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode3.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode_Header.h",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_MacResources.r",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_MacUtilities.mm",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_Wrapper.cpp", 0 };

            const char* winFiles[] = { JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode1.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode2.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode3.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_DigiCode_Header.h",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_WinUtilities.cpp",
                                       JUCE_PLUGINS_PATH_RTAS "juce_RTAS_Wrapper.cpp" , 0};

            for (const char** f = (exporter.isXcode() ? osxFiles : winFiles); *f != 0; ++f)
            {
                const RelativePath file (exporter.getJucePathFromProjectFolder().getChildFile (*f));
                group.addRelativeFile (file, -1, file.hasFileExtension ("cpp;mm;r"));
            }

            groups.add (group);
        }
    }

    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
    {
        RelativePath sdkFolder (exporter.getRTASFolder().toString(), RelativePath::projectFolder);

        if (exporter.isVisualStudio())
        {
            RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                            exporter.getTargetFolder(), RelativePath::buildTargetFolder);

            paths.add (juceWrapperFolder.toWindowsStyle());

            const char* p[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
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

            for (int i = 0; i < numElementsInArray (p); ++i)
                paths.add (exporter.rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (p[i])).toWindowsStyle());
        }
        else if (exporter.isXcode())
        {
            paths.add ("/Developer/Headers/FlatCarbon");

            const char* p[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
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

            for (int i = 0; i < numElementsInArray (p); ++i)
                paths.add (exporter.rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (p[i])).toUnixStyle());
        }
    }

    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
    {
        if (exporter.isXcode() || exporter.isVisualStudio())
        {
            props.add (new TextPropertyComponent (exporter.getRTASFolder(), "RTAS Folder", 1024, false));
            props.getLast()->setTooltip ("If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags)
    {
    }
};

//==============================================================================
class AULibraryModule  : public LibraryModule
{
public:
    AULibraryModule() {}

    void addExtraCodeGroups (const ProjectExporter& exporter, Array<Project::Item>& groups) const
    {
        if (exporter.isXcode())
        {
            Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(), "Juce AU Wrapper"));
            group.setID ("__juceaufiles");

            {
                const char* files[] = { JUCE_PLUGINS_PATH_AU "juce_AU_Resources.r",
                                        JUCE_PLUGINS_PATH_AU "juce_AU_Wrapper.mm", 0 };

                for (const char** f = files; *f != 0; ++f)
                    group.addRelativeFile (exporter.getJucePathFromProjectFolder().getChildFile (*f), -1, true);
            }

            Project::Item subGroup (group.addNewSubGroup ("Apple AU Files", -1));
            subGroup.setID ("__juceappleaufiles");

            {
                #define JUCE_AU_PUBLICUTILITY   "${DEVELOPER_DIR}/Extras/CoreAudio/PublicUtility/"
                #define JUCE_AU_PUBLIC          "${DEVELOPER_DIR}/Extras/CoreAudio/AudioUnits/AUPublic/"

                const char* appleAUFiles[] = {  JUCE_AU_PUBLICUTILITY "CADebugMacros.h",
                                                JUCE_AU_PUBLICUTILITY "CAAUParameter.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAAUParameter.h",
                                                JUCE_AU_PUBLICUTILITY "CAAudioChannelLayout.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAAudioChannelLayout.h",
                                                JUCE_AU_PUBLICUTILITY "CAMutex.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAMutex.h",
                                                JUCE_AU_PUBLICUTILITY "CAStreamBasicDescription.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAStreamBasicDescription.h",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnitTypes.h",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnit.cpp",
                                                JUCE_AU_PUBLICUTILITY "CAVectorUnit.h",
                                                JUCE_AU_PUBLIC "AUViewBase/AUViewLocalizedStringKeys.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewDispatch.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewControl.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewControl.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/CarbonEventHandler.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/CarbonEventHandler.h",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewBase.cpp",
                                                JUCE_AU_PUBLIC "AUCarbonViewBase/AUCarbonViewBase.h",
                                                JUCE_AU_PUBLIC "AUBase/AUBase.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUBase.h",
                                                JUCE_AU_PUBLIC "AUBase/AUDispatch.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUDispatch.h",
                                                JUCE_AU_PUBLIC "AUBase/AUInputElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUInputElement.h",
                                                JUCE_AU_PUBLIC "AUBase/AUOutputElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUOutputElement.h",
                                                JUCE_AU_PUBLIC "AUBase/AUResources.r",
                                                JUCE_AU_PUBLIC "AUBase/AUScopeElement.cpp",
                                                JUCE_AU_PUBLIC "AUBase/AUScopeElement.h",
                                                JUCE_AU_PUBLIC "AUBase/ComponentBase.cpp",
                                                JUCE_AU_PUBLIC "AUBase/ComponentBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIEffectBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUMIDIEffectBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUOutputBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUOutputBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/MusicDeviceBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/MusicDeviceBase.h",
                                                JUCE_AU_PUBLIC "OtherBases/AUEffectBase.cpp",
                                                JUCE_AU_PUBLIC "OtherBases/AUEffectBase.h",
                                                JUCE_AU_PUBLIC "Utility/AUBuffer.cpp",
                                                JUCE_AU_PUBLIC "Utility/AUBuffer.h",
                                                JUCE_AU_PUBLIC "Utility/AUDebugDispatcher.cpp",
                                                JUCE_AU_PUBLIC "Utility/AUDebugDispatcher.h",
                                                JUCE_AU_PUBLIC "Utility/AUInputFormatConverter.h",
                                                JUCE_AU_PUBLIC "Utility/AUSilentTimeout.h",
                                                JUCE_AU_PUBLIC "Utility/AUTimestampGenerator.h", 0 };

                for (const char** f = appleAUFiles; *f != 0; ++f)
                {
                    const RelativePath file (*f, RelativePath::projectFolder);
                    subGroup.addRelativeFile (file, -1, file.hasFileExtension ("cpp;mm"));
                    subGroup.getChild (subGroup.getNumChildren() - 1).getShouldInhibitWarningsValue() = true;
                }
            }

            groups.add (group);
        }
    }

    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
    {
        if (exporter.isXcode())
        {
            paths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/PublicUtility");
            paths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/Utility");
        }
    }

    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
    {
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags)
    {
    }
};


//==============================================================================
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

void ProjectType::createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
{
    modules.add (new JuceLibraryModule());
}

//==============================================================================
class ProjectType_GUIApp  : public ProjectType
{
public:
    ProjectType_GUIApp()  : ProjectType (getTypeName(), "Application (GUI)") {}

    static const char* getTypeName() noexcept   { return "guiapp"; }
    bool isGUIApplication() const               { return true; }

    void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
    {
        ProjectType::createRequiredModules (project, modules);
    }
};

//==============================================================================
class ProjectType_ConsoleApp  : public ProjectType
{
public:
    ProjectType_ConsoleApp()  : ProjectType (getTypeName(), "Application (Non-GUI)") {}

    static const char* getTypeName() noexcept   { return "consoleapp"; }
    bool isCommandLineApp() const               { return true; }

    void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
    {
        ProjectType::createRequiredModules (project, modules);
    }
};

//==============================================================================
class ProjectType_Library  : public ProjectType
{
public:
    ProjectType_Library()  : ProjectType (getTypeName(), "Static Library") {}

    static const char* getTypeName() noexcept   { return "library"; }
    bool isLibrary() const                      { return true; }

    void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
    {
        ProjectType::createRequiredModules (project, modules);
    }
};

//==============================================================================
class ProjectType_AudioPlugin  : public ProjectType
{
public:
    ProjectType_AudioPlugin()  : ProjectType (getTypeName(), "Audio Plug-in") {}

    static const char* getTypeName() noexcept   { return "audioplug"; }
    bool isAudioPlugin() const                  { return true; }

    void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
    {
        ProjectType::createRequiredModules (project, modules);

        if (project.shouldBuildVST().getValue())
            modules.add (new VSTLibraryModule());

        if (project.shouldBuildRTAS().getValue())
            modules.add (new RTASLibraryModule());

        if (project.shouldBuildAU().getValue())
            modules.add (new AULibraryModule());
    }
};

//==============================================================================
static ProjectType_GUIApp guiType;
static ProjectType_ConsoleApp consoleType;
static ProjectType_Library staticLibType;
static ProjectType_AudioPlugin audioPluginType;

//==============================================================================
const char* ProjectType::getGUIAppTypeName()        { return ProjectType_GUIApp::getTypeName(); }
const char* ProjectType::getConsoleAppTypeName()    { return ProjectType_ConsoleApp::getTypeName(); }
const char* ProjectType::getAudioPluginTypeName()   { return ProjectType_AudioPlugin::getTypeName(); }
