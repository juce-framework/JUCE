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
#include "jucer_ProjectSaver.h"


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

    static int countMaxPluginChannels (const String& configString, bool isInput)
    {
        StringArray configs;
        configs.addTokens (configString, ", {}", String::empty);
        configs.trim();
        configs.removeEmptyStrings();
        jassert ((configs.size() & 1) == 0);  // looks like a syntax error in the configs?

        int maxVal = 0;
        for (int i = (isInput ? 0 : 1); i < configs.size(); i += 2)
            maxVal = jmax (maxVal, configs[i].getIntValue());

        return maxVal;
    }

    static void writePluginCharacteristics (Project& project, OutputStream& out)
    {
        String headerGuard ("__PLUGINCHARACTERISTICS_" + project.getProjectUID().toUpperCase() + "__");

        ProjectSaver::writeAutoGenWarningComment (out);

        out << "    This header file contains configuration options for the plug-in. If you need to change any of" << newLine
            << "    these, it'd be wise to do so using the Jucer, rather than editing this file directly..." << newLine
            << newLine
            << "*/" << newLine
            << newLine
            << "#ifndef " << headerGuard << newLine
            << "#define " << headerGuard << newLine
            << newLine
            << "#define JucePlugin_Build_VST    " << ((bool) project.shouldBuildVST().getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << "#define JucePlugin_Build_AU     " << ((bool) project.shouldBuildAU().getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << "#define JucePlugin_Build_RTAS   " << ((bool) project.shouldBuildRTAS().getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << newLine
            << "#define JucePlugin_Name                 " << project.getPluginName().toString().quoted() << newLine
            << "#define JucePlugin_Desc                 " << project.getPluginDesc().toString().quoted() << newLine
            << "#define JucePlugin_Manufacturer         " << project.getPluginManufacturer().toString().quoted() << newLine
            << "#define JucePlugin_ManufacturerCode     '" << project.getPluginManufacturerCode().toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_PluginCode           '" << project.getPluginCode().toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_MaxNumInputChannels  " << countMaxPluginChannels (project.getPluginChannelConfigs().toString(), true) << newLine
            << "#define JucePlugin_MaxNumOutputChannels " << countMaxPluginChannels (project.getPluginChannelConfigs().toString(), false) << newLine
            << "#define JucePlugin_PreferredChannelConfigurations   " << project.getPluginChannelConfigs().toString() << newLine
            << "#define JucePlugin_IsSynth              " << ((bool) project.getPluginIsSynth().getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_WantsMidiInput       " << ((bool) project.getPluginWantsMidiInput().getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_ProducesMidiOutput   " << ((bool) project.getPluginProducesMidiOut().getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_SilenceInProducesSilenceOut  " << ((bool) project.getPluginSilenceInProducesSilenceOut().getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_TailLengthSeconds    " << (double) project.getPluginTailLengthSeconds().getValue() << newLine
            << "#define JucePlugin_EditorRequiresKeyboardFocus  " << ((bool) project.getPluginEditorNeedsKeyFocus().getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_VersionCode          " << project.getVersionAsHex() << newLine
            << "#define JucePlugin_VersionString        " << project.getVersion().toString().quoted() << newLine
            << "#define JucePlugin_VSTUniqueID          JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_VSTCategory          " << ((bool) project.getPluginIsSynth().getValue() ? "kPlugCategSynth" : "kPlugCategEffect") << newLine
            << "#define JucePlugin_AUMainType           " << ((bool) project.getPluginIsSynth().getValue() ? "kAudioUnitType_MusicDevice" : "kAudioUnitType_Effect") << newLine
            << "#define JucePlugin_AUSubType            JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_AUExportPrefix       " << project.getPluginAUExportPrefix().toString() << newLine
            << "#define JucePlugin_AUExportPrefixQuoted " << project.getPluginAUExportPrefix().toString().quoted() << newLine
            << "#define JucePlugin_AUManufacturerCode   JucePlugin_ManufacturerCode" << newLine
            << "#define JucePlugin_CFBundleIdentifier   " << project.getBundleIdentifier().toString() << newLine
            << "#define JucePlugin_AUCocoaViewClassName " << project.getPluginAUCocoaViewClassName().toString() << newLine
            << "#define JucePlugin_RTASCategory         " << ((bool) project.getPluginIsSynth().getValue() ? "ePlugInCategory_SWGenerators" : "ePlugInCategory_None") << newLine
            << "#define JucePlugin_RTASManufacturerCode JucePlugin_ManufacturerCode" << newLine
            << "#define JucePlugin_RTASProductId        JucePlugin_PluginCode" << newLine;

        out << "#define JUCE_USE_VSTSDK_2_4             1" << newLine
            << newLine
            << "#endif   // " << headerGuard << newLine;
    }

    void writePluginCharacteristicsFile (ProjectSaver& projectSaver)
    {
        MemoryOutputStream mem;
        writePluginCharacteristics (projectSaver.getProject(), mem);
        projectSaver.saveGeneratedFile (projectSaver.getProject().getPluginCharacteristicsFilename(), mem);
    }
}


//==============================================================================
class JuceLibraryModule  : public LibraryModule
{
public:
    JuceLibraryModule() {}

    void getHeaderFiles (Project& project, StringArray& includePaths, StringArray& headerGuards)
    {
        if (project.getJuceLinkageMode() != Project::notLinkedToJuce)
        {
            if (project.isUsingSingleTemplateFile()
                 || project.isUsingMultipleTemplateFiles()
                 || project.isUsingFullyAmalgamatedFile())
                createMultipleIncludes (project, "juce_amalgamated.h", includePaths, headerGuards);
            else
                createMultipleIncludes (project, "juce.h", includePaths, headerGuards);
        }
    }

    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        const Project& project = exporter.getProject();

        const String linkageMode (project.getJuceLinkageMode());
        int numJuceSourceFiles = 0;

        if (linkageMode == Project::useAmalgamatedJuce
              || linkageMode == Project::useAmalgamatedJuceViaSingleTemplate)
        {
            numJuceSourceFiles = 1;
        }
        else if (linkageMode == Project::useAmalgamatedJuceViaMultipleTemplates)
        {
            numJuceSourceFiles = project.getNumSeparateAmalgamatedFiles();
        }
        else
        {
            jassert (linkageMode == Project::notLinkedToJuce
                      || linkageMode == Project::useLinkedJuce);
        }

        for (int i = 0; i <= project.getNumSeparateAmalgamatedFiles(); ++i)
        {
            String sourceWrapper (project.getJuceSourceFilenameRoot());

            if (i != 0)
                sourceWrapper << i;

            sourceWrapper << (exporter.usesMMFiles() ? ".mm" : ".cpp");

            if (numJuceSourceFiles > 0
                 && ((i == 0 && numJuceSourceFiles == 1) || (i != 0 && numJuceSourceFiles > 1)))
            {
                MemoryOutputStream mem;
                writeSourceWrapper (mem, const_cast<Project&> (project), i);
                projectSaver.saveGeneratedFile (sourceWrapper, mem);
            }
            else
            {
                project.getGeneratedCodeFolder().getChildFile (sourceWrapper).deleteFile();
            }
        }

        if (project.isConfigFlagEnabled ("JUCE_PLUGINHOST_AU"))
            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);
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

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
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

private:
    static void writeSourceWrapper (OutputStream& out, Project& project, int fileNumber)
    {
        const String appConfigFileName (project.getAppConfigFilename());

        ProjectSaver::writeAutoGenWarningComment (out);

        out << "    This file pulls in all the Juce source code, and builds it using the settings" << newLine
            << "    defined in " << appConfigFileName << "." << newLine
            << newLine
            << "    If you want to change the method by which Juce is linked into your app, use the" << newLine
            << "    Jucer to change it, rather than trying to edit this file directly." << newLine
            << newLine
            << "*/"
            << newLine
            << newLine
            << CodeHelpers::createIncludeStatement (appConfigFileName) << newLine;

        if (fileNumber == 0)
            writeInclude (project, out, project.isUsingFullyAmalgamatedFile() ? "juce_amalgamated.cpp"
                                                                              : "amalgamation/juce_amalgamated_template.cpp");
        else
            writeInclude (project, out, "amalgamation/juce_amalgamated" + String (fileNumber) + ".cpp");
    }

    static void createMultipleIncludes (Project& project, const String& pathFromLibraryFolder,
                                        StringArray& paths, StringArray& guards)
    {
        for (int i = project.getNumExporters(); --i >= 0;)
        {
            ScopedPointer <ProjectExporter> exporter (project.createExporter (i));

            if (exporter != nullptr)
            {
                paths.add (exporter->getIncludePathForFileInJuceFolder (pathFromLibraryFolder, project.getAppIncludeFile()));
                guards.add ("defined (" + exporter->getExporterIdentifierMacro() + ")");
            }
        }
    }

    static void writeInclude (Project& project, OutputStream& out, const String& pathFromJuceFolder)
    {
        StringArray paths, guards;
        createMultipleIncludes (project, pathFromJuceFolder, paths, guards);

        StringArray uniquePaths (paths);
        uniquePaths.removeDuplicates (false);

        if (uniquePaths.size() == 1)
        {
            out << "#include " << paths[0] << newLine;
        }
        else
        {
            int i = paths.size();
            for (; --i >= 0;)
            {
                for (int j = i; --j >= 0;)
                {
                    if (paths[i] == paths[j] && guards[i] == guards[j])
                    {
                        paths.remove (i);
                        guards.remove (i);
                    }
                }
            }

            for (i = 0; i < paths.size(); ++i)
            {
                out << (i == 0 ? "#if " : "#elif ") << guards[i] << newLine
                    << " #include " << paths[i] << newLine;
            }

            out << "#endif" << newLine;
        }
    }
};

//==============================================================================
#define JUCE_PLUGINS_ROOT       "src/audio/plugin_client/"
#define JUCE_PLUGINS_PATH_VST   JUCE_PLUGINS_ROOT "VST/"
#define JUCE_PLUGINS_PATH_RTAS  JUCE_PLUGINS_ROOT "RTAS/"
#define JUCE_PLUGINS_PATH_AU    JUCE_PLUGINS_ROOT "AU/"

//==============================================================================
class VSTLibraryModule  : public LibraryModule
{
public:
    VSTLibraryModule() {}

    void getHeaderFiles (Project& project, StringArray& includePaths, StringArray& headerGuards)
    {
    }

    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        writePluginCharacteristicsFile (projectSaver);

        exporter.makefileTargetSuffix = ".so";

        Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(), "Juce VST Wrapper"));
        group.setID ("__jucevstfiles");

        const char* osxFiles[] = { JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.cpp",
                                   JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.mm", 0 };

        const char* winFiles[] = { JUCE_PLUGINS_PATH_VST "juce_VST_Wrapper.cpp", 0};

        for (const char** f = (exporter.isXcode() ? osxFiles : winFiles); *f != 0; ++f)
            group.addRelativeFile (exporter.getJucePathFromProjectFolder().getChildFile (*f), -1, true);

        exporter.groups.add (group);
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

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
    {
    }
};

//==============================================================================
class RTASLibraryModule  : public LibraryModule
{
public:
    RTASLibraryModule() {}

    void getHeaderFiles (Project& project, StringArray& includePaths, StringArray& headerGuards)
    {
    }

    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        exporter.xcodeCanUseDwarf = false;

        exporter.msvcTargetSuffix = ".dpm";
        exporter.msvcNeedsDLLRuntimeLib = true;

        RelativePath rtasFolder (exporter.getRTASFolder().toString(), RelativePath::projectFolder);
        exporter.msvcExtraPreprocessorDefs.set ("JucePlugin_WinBag_path", CodeHelpers::addEscapeChars (rtasFolder.getChildFile ("WinBag")
                                                                                .toWindowsStyle().quoted()));

        exporter.msvcExtraLinkerOptions = "/FORCE:multiple";
        exporter.msvcDelayLoadedDLLs = "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; DSPManager.dll";
        exporter.msvcModuleDefinitionFile = exporter.getJucePathFromTargetFolder()
                                                    .getChildFile (JUCE_PLUGINS_PATH_RTAS "juce_RTAS_WinExports.def")
                                                    .toWindowsStyle();

        exporter.msvcPostBuildOutputs = "\"$(TargetPath)\".rsr";
        exporter.msvcPostBuildCommand = "copy /Y \"" + exporter.getJucePathFromTargetFolder()
                                                               .getChildFile (JUCE_PLUGINS_PATH_RTAS "juce_RTAS_WinResources.rsr")
                                                               .toWindowsStyle()
                                           + "\" \"$(TargetPath)\".rsr";


        exporter.xcodeExtraLibrariesDebug.add   (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
        exporter.xcodeExtraLibrariesRelease.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));

        writePluginCharacteristicsFile (projectSaver);

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
                group.getChild (group.getNumChildren() - 1).getShouldInhibitWarningsValue() = true;
            }

            exporter.groups.add (group);
        }
    }

    void addExtraSearchPaths (const ProjectExporter& exporter, StringArray& paths) const
    {
        RelativePath rtasFolder (exporter.getRTASFolder().toString(), RelativePath::projectFolder);

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
                paths.add (exporter.rebaseFromProjectFolderToBuildTarget (rtasFolder.getChildFile (p[i])).toWindowsStyle());
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
                paths.add (exporter.rebaseFromProjectFolderToBuildTarget (rtasFolder.getChildFile (p[i])).toUnixStyle());
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

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
    {
    }
};

//==============================================================================
class AULibraryModule  : public LibraryModule
{
public:
    AULibraryModule() {}

    void getHeaderFiles (Project& project, StringArray& includePaths, StringArray& headerGuards)
    {
    }

    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        writePluginCharacteristicsFile (projectSaver);

        if (exporter.isXcode())
        {
            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

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

            exporter.groups.add (group);
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

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
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

void ProjectType::prepareExporter (ProjectExporter& exporter) const
{
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

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodePackageType = "APPL";
        exporter.xcodeBundleSignature = "????";
        exporter.xcodeCreatePList = true;
        exporter.xcodeFileType = "wrapper.application";
        exporter.xcodeBundleExtension = ".app";
        exporter.xcodeProductType = "com.apple.product-type.application";
        exporter.xcodeProductInstallPath = "$(HOME)/Applications";

        exporter.msvcIsWindowsSubsystem = true;
        exporter.msvcTargetSuffix = ".exe";
    }

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

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "compiled.mach-o.executable";
        exporter.xcodeBundleExtension = String::empty;
        exporter.xcodeProductType = "com.apple.product-type.tool";
        exporter.xcodeProductInstallPath = "/usr/bin";

        exporter.msvcIsWindowsSubsystem = false;
        exporter.msvcTargetSuffix = ".exe";
        exporter.msvcExtraPreprocessorDefs.set ("_CONSOLE", "");
    }

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

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "archive.ar";
        exporter.xcodeProductType = "com.apple.product-type.library.static";
        exporter.xcodeProductInstallPath = String::empty;

        exporter.makefileTargetSuffix = ".so";

        exporter.msvcTargetSuffix = ".lib";
        exporter.msvcExtraPreprocessorDefs.set ("_LIB", "");
    }

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

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeIsBundle = true;
        exporter.xcodeCreatePList = true;
        exporter.xcodePackageType = "TDMw";
        exporter.xcodeBundleSignature = "PTul";
        exporter.xcodeFileType = "wrapper.cfbundle";
        exporter.xcodeBundleExtension = ".component";
        exporter.xcodeProductType = "com.apple.product-type.bundle";
        exporter.xcodeProductInstallPath = "$(HOME)/Library/Audio/Plug-Ins/Components/";

        exporter.xcodeShellScriptTitle = "Copy to the different plugin folders";
        exporter.xcodeShellScript = String::fromUTF8 (BinaryData::AudioPluginXCodeScript_txt, BinaryData::AudioPluginXCodeScript_txtSize);

        exporter.xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                                      " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                      " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"";

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }

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
class ProjectType_BrowserPlugin  : public ProjectType
{
public:
    ProjectType_BrowserPlugin()  : ProjectType (getTypeName(), "Browser Plug-in") {}

    static const char* getTypeName() noexcept   { return "browserplug"; }
    bool isBrowserPlugin() const                { return true; }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeIsBundle = true;
        exporter.xcodeCreatePList = true;
        exporter.xcodeFileType = "wrapper.cfbundle";
        exporter.xcodeBundleExtension = ".plugin";
        exporter.xcodeProductType = "com.apple.product-type.bundle";
        exporter.xcodeProductInstallPath = "$(HOME)/Library/Internet Plug-Ins//";

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }

    void createRequiredModules (Project& project, OwnedArray<LibraryModule>& modules) const
    {
        ProjectType::createRequiredModules (project, modules);
    }
};

//==============================================================================
static ProjectType_GUIApp       guiType;
static ProjectType_ConsoleApp   consoleType;
static ProjectType_Library      libraryType;
static ProjectType_AudioPlugin  audioPluginType;

//==============================================================================
const char* ProjectType::getGUIAppTypeName()        { return ProjectType_GUIApp::getTypeName(); }
const char* ProjectType::getConsoleAppTypeName()    { return ProjectType_ConsoleApp::getTypeName(); }
const char* ProjectType::getAudioPluginTypeName()   { return ProjectType_AudioPlugin::getTypeName(); }
