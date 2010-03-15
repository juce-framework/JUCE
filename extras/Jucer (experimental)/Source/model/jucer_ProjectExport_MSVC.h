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

#ifndef __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__

#include "jucer_ProjectExporter.h"


//==============================================================================
class MSVCProjectExporter   : public ProjectExporter
{
public:
    //==============================================================================
    enum VisualStudioVersion
    {
        visualStudio2005,
        visualStudio2008
    };

    static const char* getName2005()            { return "Visual Studio 2005"; }
    static const char* getName2008()            { return "Visual Studio 2008"; }

    static const char* getValueTreeTypeName (VisualStudioVersion  version)
    {
        switch (version)
        {
            case visualStudio2005:  return "VS2005"; break;
            case visualStudio2008:  return "VS2008"; break;
            default:                jassertfalse; break;
        }

        return 0;
    }

    //==============================================================================
    static MSVCProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (visualStudio2005)))
            return new MSVCProjectExporter (project, settings, visualStudio2005);
        else if (settings.hasType (getValueTreeTypeName (visualStudio2008)))
            return new MSVCProjectExporter (project, settings, visualStudio2008);

        return 0;
    }

    //==============================================================================
    MSVCProjectExporter (Project& project_, const ValueTree& settings_, const VisualStudioVersion version_)
        : ProjectExporter (project_, settings_), version (version_)
    {
        String subFolderName (getDefaultBuildsRootFolder());

        switch (version)
        {
            case visualStudio2005:  name = "Visual Studio 2005"; subFolderName += "VisualStudio2005"; break;
            case visualStudio2008:  name = "Visual Studio 2008"; subFolderName += "VisualStudio2008"; break;
            default:                jassertfalse; break;
        }

        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = subFolderName;

        if (getVSTFolder().toString().isEmpty())
            getVSTFolder() = "c:\\SDKs\\vstsdk2.4";

        if (getRTASFolder().toString().isEmpty())
            getRTASFolder() = "c:\\SDKs\\PT_80_SDK";
    }

    ~MSVCProjectExporter()  {}

    //==============================================================================
    bool isDefaultFormatForCurrentOS()
    {
      #if JUCE_WINDOWS
        return true;
      #else
        return false;
      #endif
    }

    bool isPossibleForCurrentProject()          { return true; }
    bool usesMMFiles() const                    { return false; }
    const String getOSTestMacro()               { return "(defined (_WIN32) || defined (_WIN64))"; }

    void launchProject()
    {
        getSLNFile().startAsProcess();
    }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);
    }

    //==============================================================================
    const String create()
    {
        projectGUID = createGUID (project.getProjectUID());

        XmlElement masterXml ("VisualStudioProject");
        fillInMasterXml (masterXml);

        {
            MemoryOutputStream mo;
            masterXml.writeToStream (mo, String::empty, false, true, "UTF-8", 10);

            if (! overwriteFileWithNewDataIfDifferent (getVCProjFile(), mo))
                return "Can't write to the VC project file: " + getVCProjFile().getFullPathName();
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo);

            if (! overwriteFileWithNewDataIfDifferent (getSLNFile(), mo))
                return "Can't write to the VC solution file: " + getSLNFile().getFullPathName();
        }

        return String::empty;
    }

private:
    String projectGUID;
    const VisualStudioVersion version;

    const File getVCProjFile() const { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (".vcproj"); }
    const File getSLNFile() const    { return getVCProjFile().withFileExtension (".sln"); }


    //==============================================================================
    void fillInMasterXml (XmlElement& masterXml)
    {
        masterXml.setAttribute ("ProjectType", "Visual C++");

        switch (version)
        {
            case visualStudio2005:  masterXml.setAttribute ("Version", "8.00"); break;
            case visualStudio2008:  masterXml.setAttribute ("Version", "9.00"); break;
            default:                jassertfalse; break;
        }

        masterXml.setAttribute ("Name", project.getProjectName().toString());
        masterXml.setAttribute ("ProjectGUID", projectGUID);
        masterXml.setAttribute ("TargetFrameworkVersion", "131072");

        {
            XmlElement* platforms = masterXml.createNewChildElement ("Platforms");
            XmlElement* platform = platforms->createNewChildElement ("Platform");
            platform->setAttribute ("Name", "Win32");
        }

        masterXml.createNewChildElement ("ToolFiles");
        createConfigs (*masterXml.createNewChildElement ("Configurations"));
        masterXml.createNewChildElement ("References");
        createFiles (*masterXml.createNewChildElement ("Files"));
        masterXml.createNewChildElement ("Globals");
    }

    //==============================================================================
    void addFile (const RelativePath& file, XmlElement& parent, const bool excludeFromBuild, const bool useStdcall)
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        XmlElement* fileXml = parent.createNewChildElement ("File");
        fileXml->setAttribute ("RelativePath", file.toWindowsStyle());

        if (excludeFromBuild || useStdcall)
        {
            for (int i = 0; i < project.getNumConfigurations(); ++i)
            {
                Project::BuildConfiguration config (project.getConfiguration (i));

                XmlElement* fileConfig = fileXml->createNewChildElement ("FileConfiguration");
                fileConfig->setAttribute ("Name", createConfigName (config));

                if (excludeFromBuild)
                    fileConfig->setAttribute ("ExcludedFromBuild", "true");

                XmlElement* tool = createToolElement (*fileConfig, "VCCLCompilerTool");

                if (useStdcall)
                    tool->setAttribute ("CallingConvention", "2");
            }
        }
    }

    XmlElement* createGroup (const String& name, XmlElement& parent)
    {
        XmlElement* filter = parent.createNewChildElement ("Filter");
        filter->setAttribute ("Name", name);
        return filter;
    }

    void addFiles (const Project::Item& projectItem, XmlElement& parent)
    {
        if (projectItem.isGroup())
        {
            XmlElement* filter = createGroup (projectItem.getName().toString(), parent);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFiles (projectItem.getChild(i), *filter);
        }
        else
        {
            if (projectItem.shouldBeAddedToTargetProject())
            {
                const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

                addFile (path, parent,
                         projectItem.shouldBeAddedToBinaryResources() || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()),
                         false);
            }
        }
    }

    void addGroup (XmlElement& parent, const String& groupName, const Array<RelativePath>& files, const bool useStdcall)
    {
        if (files.size() > 0)
        {
            XmlElement* const group = createGroup (groupName, parent);

            for (int i = 0; i < files.size(); ++i)
                if (files.getReference(i).hasFileExtension ("cpp;c;h"))
                    addFile (files.getReference(i), *group, false,
                             useStdcall && shouldFileBeCompiledByDefault (files.getReference(i)));
        }
    }

    void createFiles (XmlElement& files)
    {
        addFiles (project.getMainGroup(), files);

        addGroup (files, project.getJuceCodeGroupName(), juceWrapperFiles, false);
        addGroup (files, "Juce VST Wrapper", getVSTFilesRequired(), false);
        addGroup (files, "Juce RTAS Wrapper", getRTASFilesRequired(), true);
    }

    //==============================================================================
    const Array<RelativePath> getRTASFilesRequired() const
    {
        Array<RelativePath> s;
        if (isRTAS())
        {
            static const char* files[] = { "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode1.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode2.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode3.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode_Header.h",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_WinUtilities.cpp",
                                           "extras/audio plugins/wrapper/RTAS/juce_RTAS_Wrapper.cpp" };

            for (int i = 0; i < numElementsInArray (files); ++i)
                s.add (getJucePathFromTargetFolder().getChildFile (files[i]));
        }

        return s;
    }

    const String getIntermediatesPath (const Project::BuildConfiguration& config) const
    {
        return ".\\" + File::createLegalFileName (config.getName().toString().trim());
    }

    const String getConfigTargetPath (const Project::BuildConfiguration& config) const
    {
        const String binaryPath (config.getTargetBinaryRelativePath().toString().trim());
        if (binaryPath.isEmpty())
            return getIntermediatesPath (config);

        return ".\\" + RelativePath (binaryPath, RelativePath::projectFolder)
                             .rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder)
                             .toWindowsStyle();
    }

    const String getTargetBinarySuffix() const
    {
        if (project.isLibrary())
            return ".lib";
        else if (isRTAS())
            return ".dpm";
        else if (project.isAudioPlugin() || project.isBrowserPlugin())
            return ".dll";

        return ".exe";
    }

    const String getPreprocessorDefs (const Project::BuildConfiguration& config) const
    {
        StringArray defines;
        defines.add ("WIN32");
        defines.add ("_WINDOWS");
        defines.add (config.isDebug().getValue() ? "_DEBUG" : "NDEBUG");
        if (project.isCommandLineApp())
            defines.add ("_CONSOLE");

        if (project.isLibrary())
            defines.add ("_LIB");

        if (isRTAS())
        {
            RelativePath rtasFolder (getRTASFolder().toString(), RelativePath::unknown);
            defines.add ("JucePlugin_WinBag_path="
                            + replaceCEscapeChars (rtasFolder.getChildFile ("WinBag")
                                                      .toWindowsStyle().quoted()));
        }

        defines.addArray (config.parsePreprocessorDefs());
        return defines.joinIntoString (";");
    }

    const StringArray getHeaderSearchPaths (const Project::BuildConfiguration& config) const
    {
        StringArray searchPaths (config.getHeaderSearchPaths());

        if (project.shouldAddVSTFolderToPath() && getVSTFolder().toString().isNotEmpty())
            searchPaths.add (RelativePath (getVSTFolder().toString(), RelativePath::projectFolder)
                                .rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder)
                                .toWindowsStyle());

        if (project.isAudioPlugin())
            searchPaths.add (juceWrapperFiles[0].getParentDirectory().toWindowsStyle());

        if (isRTAS())
        {
            static const char* rtasIncludePaths[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
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

            RelativePath sdkFolder (getRTASFolder().toString(), RelativePath::projectFolder);
            sdkFolder = sdkFolder.rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder);

            for (int i = 0; i < numElementsInArray (rtasIncludePaths); ++i)
                searchPaths.add (sdkFolder.getChildFile (rtasIncludePaths[i]).toWindowsStyle());
        }

        return searchPaths;
    }

    XmlElement* createToolElement (XmlElement& parent, const String& name) const
    {
        XmlElement* const e = parent.createNewChildElement ("Tool");
        e->setAttribute ("Name", name);
        return e;
    }

    void createConfig (XmlElement& xml, const Project::BuildConfiguration& config) const
    {
        String binariesPath (getConfigTargetPath (config));
        String intermediatesPath (getIntermediatesPath (config));
        const bool isDebug = (bool) config.isDebug().getValue();
        const String binaryName (File::createLegalFileName (config.getTargetBinaryName().toString()));

        xml.setAttribute ("Name", createConfigName (config));
        xml.setAttribute ("OutputDirectory", windowsStylePath (binariesPath));
        xml.setAttribute ("IntermediateDirectory", windowsStylePath (intermediatesPath));
        xml.setAttribute ("ConfigurationType", (project.isAudioPlugin() || project.isBrowserPlugin())
                                                    ? "2" : (project.isLibrary() ? "4" : "1"));
        xml.setAttribute ("UseOfMFC", "0");
        xml.setAttribute ("ATLMinimizesCRunTimeLibraryUsage", "false");
        xml.setAttribute ("CharacterSet", "2");

        if (! isDebug)
            xml.setAttribute ("WholeProgramOptimization", "1");

        createToolElement (xml, "VCPreBuildEventTool");

        XmlElement* customBuild = createToolElement (xml, "VCCustomBuildTool");

        if (isRTAS())
        {
            RelativePath rsrFile (getJucePathFromTargetFolder().getChildFile ("extras/audio plugins/wrapper/RTAS/juce_RTAS_WinResources.rsr"));

            customBuild->setAttribute ("CommandLine", "copy /Y \"" + rsrFile.toWindowsStyle() + "\" \"$(TargetPath)\".rsr");
            customBuild->setAttribute ("Outputs", "\"$(TargetPath)\".rsr");
        }

        createToolElement (xml, "VCXMLDataGeneratorTool");
        createToolElement (xml, "VCWebServiceProxyGeneratorTool");

        if (! project.isLibrary())
        {
            XmlElement* midl = createToolElement (xml, "VCMIDLTool");
            midl->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
            midl->setAttribute ("MkTypLibCompatible", "true");
            midl->setAttribute ("SuppressStartupBanner", "true");
            midl->setAttribute ("TargetEnvironment", "1");
            midl->setAttribute ("TypeLibraryName", windowsStylePath (intermediatesPath + "/" + binaryName + ".tlb"));
            midl->setAttribute ("HeaderFileName", "");
        }

        {
            XmlElement* compiler = createToolElement (xml, "VCCLCompilerTool");

            const int optimiseLevel = (int) config.getOptimisationLevel().getValue();
            compiler->setAttribute ("Optimization", optimiseLevel <= 1 ? "0" : (optimiseLevel == 2 ? "2" : "3"));

            if (isDebug)
            {
                compiler->setAttribute ("BufferSecurityCheck", "");
                compiler->setAttribute ("DebugInformationFormat", "4");
            }
            else
            {
                compiler->setAttribute ("InlineFunctionExpansion", "1");
                compiler->setAttribute ("StringPooling", "true");
            }

            compiler->setAttribute ("AdditionalIncludeDirectories", getHeaderSearchPaths (config).joinIntoString (";"));
            compiler->setAttribute ("PreprocessorDefinitions", getPreprocessorDefs (config));
            compiler->setAttribute ("RuntimeLibrary", isRTAS() ? (isDebug ? 3 : 2) // MT DLL
                                                               : (isDebug ? 1 : 0)); // MT static
            compiler->setAttribute ("RuntimeTypeInfo", "true");
            compiler->setAttribute ("UsePrecompiledHeader", "0");
            compiler->setAttribute ("PrecompiledHeaderFile", windowsStylePath (intermediatesPath + "/" + binaryName + ".pch"));
            compiler->setAttribute ("AssemblerListingLocation", windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ObjectFile", windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ProgramDataBaseFileName", windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("WarningLevel", "3");
            compiler->setAttribute ("SuppressStartupBanner", "true");
        }

        createToolElement (xml, "VCManagedResourceCompilerTool");

        {
            XmlElement* resCompiler = createToolElement (xml, "VCResourceCompilerTool");
            resCompiler->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
        }

        createToolElement (xml, "VCPreLinkEventTool");

        if (! project.isLibrary())
        {
            XmlElement* linker = createToolElement (xml, "VCLinkerTool");

            linker->setAttribute ("OutputFile", windowsStylePath (binariesPath + "/" + config.getTargetBinaryName().toString() + getTargetBinarySuffix()));
            linker->setAttribute ("SuppressStartupBanner", "true");

            if (project.getJuceLinkageMode() == Project::useLinkedJuce)
                linker->setAttribute ("AdditionalLibraryDirectories", getJucePathFromTargetFolder().getChildFile ("bin").toWindowsStyle());

            linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            linker->setAttribute ("GenerateDebugInformation", isDebug ? "true" : "false");
            linker->setAttribute ("ProgramDatabaseFile", windowsStylePath (intermediatesPath + "/" + binaryName + ".pdb"));
            linker->setAttribute ("SubSystem", project.isCommandLineApp() ? "1" : "2");

            if (! isDebug)
            {
                linker->setAttribute ("GenerateManifest", "false");
                linker->setAttribute ("OptimizeReferences", "2");
                linker->setAttribute ("EnableCOMDATFolding", "2");
            }

            linker->setAttribute ("RandomizedBaseAddress", "1");
            linker->setAttribute ("DataExecutionPrevention", "0");
            linker->setAttribute ("TargetMachine", "1");

            if (isRTAS())
            {
                linker->setAttribute ("AdditionalOptions", "/FORCE:multiple");
                linker->setAttribute ("DelayLoadDLLs", "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; DSPManager.dll");
                linker->setAttribute ("ModuleDefinitionFile", getJucePathFromTargetFolder()
                                            .getChildFile ("extras/audio plugins/wrapper/RTAS/juce_RTAS_WinExports.def")
                                            .toWindowsStyle());
            }
        }
        else
        {
            XmlElement* librarian = createToolElement (xml, "VCLibrarianTool");

            librarian->setAttribute ("OutputFile", windowsStylePath (binariesPath + "/" + config.getTargetBinaryName().toString() + getTargetBinarySuffix()));
            librarian->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
        }

        createToolElement (xml, "VCALinkTool");
        createToolElement (xml, "VCManifestTool");
        createToolElement (xml, "VCXDCMakeTool");

        {
            XmlElement* bscMake = createToolElement (xml, "VCBscMakeTool");
            bscMake->setAttribute ("SuppressStartupBanner", "true");
            bscMake->setAttribute ("OutputFile", windowsStylePath (intermediatesPath + "/" + binaryName + ".bsc"));
        }

        createToolElement (xml, "VCFxCopTool");

        if (! project.isLibrary())
            createToolElement (xml, "VCAppVerifierTool");

        createToolElement (xml, "VCPostBuildEventTool");
    }

    void createConfigs (XmlElement& configs)
    {
        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            createConfig (*configs.createNewChildElement ("Configuration"), config);
        }
    }

    const String createConfigName (const Project::BuildConfiguration& config) const
    {
        return config.getName().toString() + "|Win32";
    }

    //==============================================================================
    void writeSolutionFile (OutputStream& out)
    {
        out << newLine << "Microsoft Visual Studio Solution File, Format Version ";

        switch (version)
        {
            case visualStudio2005:  out << "8.00" << newLine << "# Visual C++ Express 2005"; break;
            case visualStudio2008:  out << "10.00" << newLine << "# Visual C++ Express 2008"; break;
            default:                jassertfalse; break;
        }

        out << newLine << "Project(\"" << createGUID (project.getProjectName().toString() + "sln_guid") << "\") = \"" << project.getProjectName().toString() << "\", \""
            << getVCProjFile().getFileName() << "\", \"" << projectGUID << "\"" << newLine
            << "EndProject" << newLine
            << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            out << "\t\t" << createConfigName (config) << " = " << createConfigName (config) << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            out << "\t\t" << projectGUID << "." << createConfigName (config) << ".ActiveCfg = " << createConfigName (config) << newLine;
            out << "\t\t" << projectGUID << "." << createConfigName (config) << ".Build.0 = " << createConfigName (config) << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine
            << "EndGlobal" << newLine;
    }
};


#endif   // __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
