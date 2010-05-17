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
        visualStudio6,
        visualStudio2005,
        visualStudio2008
    };

    static const char* getNameVC6()             { return "Visual C++ 6.0"; }
    static const char* getName2005()            { return "Visual Studio 2005"; }
    static const char* getName2008()            { return "Visual Studio 2008"; }

    static const char* getValueTreeTypeName (VisualStudioVersion  version)
    {
        switch (version)
        {
            case visualStudio6:     return "MSVC6"; break;
            case visualStudio2005:  return "VS2005"; break;
            case visualStudio2008:  return "VS2008"; break;
            default:                jassertfalse; break;
        }

        return 0;
    }

    //==============================================================================
    static MSVCProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (visualStudio6)))
            return new MSVCProjectExporter (project, settings, visualStudio6);
        else if (settings.hasType (getValueTreeTypeName (visualStudio2005)))
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
            case visualStudio6:     name = "Visual C++ 6.0";     subFolderName += "MSVC6"; break;
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

        if ((int) getLibraryType().getValue() <= 0)
            getLibraryType() = 1;
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

    void launchProject()
    {
        getSLNFile().startAsProcess();
    }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);

        if (project.isLibrary())
        {
            const char* const libTypes[] = { "Static Library (.lib)", "Dynamic Library (.dll)", 0 };
            const int libTypeValues[] = { 1, 2, 0 };
            props.add (new ChoicePropertyComponent (getLibraryType(), "Library Type", StringArray (libTypes), Array<var> (libTypeValues)));

            props.add (new TextPropertyComponent (getSetting ("libraryName_Debug"), "Library Name (Debug)", 128, false));
            props.getLast()->setTooltip ("If set, this name will override the binary name specified in the configuration settings, for a debug build. You must include the .lib or .dll suffix on this filename.");

            props.add (new TextPropertyComponent (getSetting ("libraryName_Release"), "Library Name (Release)", 128, false));
            props.getLast()->setTooltip ("If set, this name will override the binary name specified in the configuration settings, for a release build. You must include the .lib or .dll suffix on this filename.");
        }
    }

    //==============================================================================
    const String create()
    {
        if (version == visualStudio6)
        {
            {
                MemoryOutputStream mo;
                writeVC6Project (mo);

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getDSPFile(), mo))
                    return "Can't write to the VC project file: " + getDSPFile().getFullPathName();
            }

            {
                MemoryOutputStream mo;
                writeDSWFile (mo);

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getDSWFile(), mo))
                    return "Can't write to the VC solution file: " + getDSWFile().getFullPathName();
            }
        }
        else
        {
            projectGUID = createGUID (project.getProjectUID());

            XmlElement masterXml ("VisualStudioProject");
            fillInMasterXml (masterXml);

            {
                MemoryOutputStream mo;
                masterXml.writeToStream (mo, String::empty, false, true, "UTF-8", 10);

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getVCProjFile(), mo))
                    return "Can't write to the VC project file: " + getVCProjFile().getFullPathName();
            }

            {
                MemoryOutputStream mo;
                writeSolutionFile (mo);

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (getSLNFile(), mo))
                    return "Can't write to the VC solution file: " + getSLNFile().getFullPathName();
            }
        }

        return String::empty;
    }

private:
    String projectGUID;
    const VisualStudioVersion version;

    const File getProjectFile (const String& extension) const   { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (extension); }

    const File getVCProjFile() const    { return getProjectFile (".vcproj"); }
    const File getSLNFile() const       { return getProjectFile (".sln"); }
    const File getDSPFile() const       { return getProjectFile (".dsp"); }
    const File getDSWFile() const       { return getProjectFile (".dsw"); }

    Value getLibraryType() const        { return getSetting ("libraryType"); }
    bool isLibraryDLL() const           { return project.isLibrary() && getLibraryType() == 2; }

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

    XmlElement* createGroup (const String& groupName, XmlElement& parent)
    {
        XmlElement* filter = parent.createNewChildElement ("Filter");
        filter->setAttribute ("Name", groupName);
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

    const String getPreprocessorDefs (const Project::BuildConfiguration& config, const String& joinString) const
    {
        StringArray defines;
        defines.add (getExporterIdentifierMacro());
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
                            + CodeHelpers::addEscapeChars (rtasFolder.getChildFile ("WinBag")
                                                                .toWindowsStyle().quoted()));
        }

        defines.addArray (config.parsePreprocessorDefs());
        defines.addArray (parsePreprocessorDefs());
        return defines.joinIntoString (joinString);
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

    XmlElement* createToolElement (XmlElement& parent, const String& toolName) const
    {
        XmlElement* const e = parent.createNewChildElement ("Tool");
        e->setAttribute ("Name", toolName);
        return e;
    }

    const String getBinaryFileForConfig (const Project::BuildConfiguration& config) const
    {
        const String targetBinary (getSetting (config.isDebug().getValue() ? "libraryName_Debug" : "libraryName_Release").toString().trim());
        if (targetBinary.isNotEmpty())
            return targetBinary;

        return config.getTargetBinaryName().toString() + getTargetBinarySuffix();
    }

    void createConfig (XmlElement& xml, const Project::BuildConfiguration& config) const
    {
        String binariesPath (getConfigTargetPath (config));
        String intermediatesPath (getIntermediatesPath (config));
        const bool isDebug = (bool) config.isDebug().getValue();
        const String binaryName (File::createLegalFileName (config.getTargetBinaryName().toString()));

        xml.setAttribute ("Name", createConfigName (config));
        xml.setAttribute ("OutputDirectory", FileHelpers::windowsStylePath (binariesPath));
        xml.setAttribute ("IntermediateDirectory", FileHelpers::windowsStylePath (intermediatesPath));
        xml.setAttribute ("ConfigurationType", (project.isAudioPlugin() || project.isBrowserPlugin() || isLibraryDLL())
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
            midl->setAttribute ("TypeLibraryName", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".tlb"));
            midl->setAttribute ("HeaderFileName", "");
        }

        {
            XmlElement* compiler = createToolElement (xml, "VCCLCompilerTool");

            const int optimiseLevel = (int) config.getOptimisationLevel().getValue();
            compiler->setAttribute ("Optimization", optimiseLevel <= 1 ? "0" : (optimiseLevel == 2 ? "2" : "3"));

            if (isDebug)
            {
                compiler->setAttribute ("BufferSecurityCheck", "");
                compiler->setAttribute ("DebugInformationFormat", project.isLibrary() ? "3" : "4");
            }
            else
            {
                compiler->setAttribute ("InlineFunctionExpansion", "1");
                compiler->setAttribute ("StringPooling", "true");
            }

            compiler->setAttribute ("AdditionalIncludeDirectories", getHeaderSearchPaths (config).joinIntoString (";"));
            compiler->setAttribute ("PreprocessorDefinitions", getPreprocessorDefs (config, ";"));
            compiler->setAttribute ("RuntimeLibrary", isRTAS() ? (isDebug ? 3 : 2) // MT DLL
                                                               : (isDebug ? 1 : 0)); // MT static
            compiler->setAttribute ("RuntimeTypeInfo", "true");
            compiler->setAttribute ("UsePrecompiledHeader", "0");
            compiler->setAttribute ("PrecompiledHeaderFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".pch"));
            compiler->setAttribute ("AssemblerListingLocation", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ObjectFile", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("ProgramDataBaseFileName", FileHelpers::windowsStylePath (intermediatesPath + "/"));
            compiler->setAttribute ("WarningLevel", project.isLibrary() ? "4" : "3");
            compiler->setAttribute ("SuppressStartupBanner", "true");

            if (getExtraCompilerFlags().toString().isNotEmpty())
                compiler->setAttribute ("AdditionalOptions", getExtraCompilerFlags().toString().trim());
        }

        createToolElement (xml, "VCManagedResourceCompilerTool");

        {
            XmlElement* resCompiler = createToolElement (xml, "VCResourceCompilerTool");
            resCompiler->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
        }

        createToolElement (xml, "VCPreLinkEventTool");

        const String outputFileName (getBinaryFileForConfig (config));

        if (! project.isLibrary())
        {
            XmlElement* linker = createToolElement (xml, "VCLinkerTool");

            linker->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
            linker->setAttribute ("SuppressStartupBanner", "true");

            if (project.getJuceLinkageMode() == Project::useLinkedJuce)
                linker->setAttribute ("AdditionalLibraryDirectories", getJucePathFromTargetFolder().getChildFile ("bin").toWindowsStyle());

            linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            linker->setAttribute ("GenerateDebugInformation", isDebug ? "true" : "false");
            linker->setAttribute ("ProgramDatabaseFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".pdb"));
            linker->setAttribute ("SubSystem", project.isCommandLineApp() ? "1" : "2");

            if (! isDebug)
            {
                linker->setAttribute ("GenerateManifest", "false");
                linker->setAttribute ("OptimizeReferences", "2");
                linker->setAttribute ("EnableCOMDATFolding", "2");
            }

            linker->setAttribute ("TargetMachine", "1"); // (64-bit build = 5)

            String extraLinkerOptions (getExtraLinkerFlags().toString());

            if (isRTAS())
            {
                extraLinkerOptions += " /FORCE:multiple";
                linker->setAttribute ("DelayLoadDLLs", "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; DSPManager.dll");
                linker->setAttribute ("ModuleDefinitionFile", getJucePathFromTargetFolder()
                                            .getChildFile ("extras/audio plugins/wrapper/RTAS/juce_RTAS_WinExports.def")
                                            .toWindowsStyle());
            }

            if (extraLinkerOptions.isNotEmpty())
                linker->setAttribute ("AdditionalOptions", extraLinkerOptions.trim());
        }
        else
        {
            if (isLibraryDLL())
            {
                XmlElement* linker = createToolElement (xml, "VCLinkerTool");

                String extraLinkerOptions (getExtraLinkerFlags().toString());
                extraLinkerOptions << " /IMPLIB:" << FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName.upToLastOccurrenceOf (".", false, false) + ".lib");
                linker->setAttribute ("AdditionalOptions", extraLinkerOptions.trim());

                linker->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
                linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
            else
            {
                XmlElement* librarian = createToolElement (xml, "VCLibrarianTool");

                librarian->setAttribute ("OutputFile", FileHelpers::windowsStylePath (binariesPath + "/" + outputFileName));
                librarian->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
        }

        createToolElement (xml, "VCALinkTool");
        createToolElement (xml, "VCManifestTool");
        createToolElement (xml, "VCXDCMakeTool");

        {
            XmlElement* bscMake = createToolElement (xml, "VCBscMakeTool");
            bscMake->setAttribute ("SuppressStartupBanner", "true");
            bscMake->setAttribute ("OutputFile", FileHelpers::windowsStylePath (intermediatesPath + "/" + binaryName + ".bsc"));
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
            << getVCProjFile().getFileName() << "\", \"" << projectGUID << '"' << newLine
            << "EndProject" << newLine
            << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            out << "\t\t" << createConfigName (config) << " = " << createConfigName (config) << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (i = 0; i < project.getNumConfigurations(); ++i)
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

    //==============================================================================
    const String createConfigNameVC6 (const Project::BuildConfiguration& config) const
    {
        return project.getProjectName().toString() + " - Win32 " + config.getName().toString();
    }

    void writeVC6Project (OutputStream& out)
    {
        const String defaultConfigName (createConfigNameVC6 (project.getConfiguration (0)));

        const bool isDLL = project.isAudioPlugin() || project.isBrowserPlugin();
        String targetType, targetCode;

        if (isDLL)                              { targetType = "\"Win32 (x86) Dynamic-Link Library\"";  targetCode = "0x0102"; }
        else if (project.isLibrary())           { targetType = "\"Win32 (x86) Static Library\"";        targetCode = "0x0104"; }
        else if (project.isCommandLineApp())    { targetType = "\"Win32 (x86) Console Application\"";   targetCode = "0x0103"; }
        else                                    { targetType = "\"Win32 (x86) Application\"";           targetCode = "0x0101"; }

        out << "# Microsoft Developer Studio Project File - Name=\"" << project.getProjectName()
            << "\" - Package Owner=<4>" << newLine
            << "# Microsoft Developer Studio Generated Build File, Format Version 6.00" << newLine
            << "# ** DO NOT EDIT **" << newLine
            << "# TARGTYPE " << targetType << " " << targetCode << newLine
            << "CFG=" << defaultConfigName << newLine
            << "!MESSAGE This is not a valid makefile. To build this project using NMAKE," << newLine
            << "!MESSAGE use the Export Makefile command and run" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE NMAKE /f \"" << project.getProjectName() << ".mak.\"" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE You can specify a configuration when running NMAKE" << newLine
            << "!MESSAGE by defining the macro CFG on the command line. For example:" << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE NMAKE /f \"" << project.getProjectName() << ".mak\" CFG=\"" << defaultConfigName << '"' << newLine
            << "!MESSAGE " << newLine
            << "!MESSAGE Possible choices for configuration are:" << newLine
            << "!MESSAGE " << newLine;

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
            out << "!MESSAGE \"" << createConfigNameVC6 (project.getConfiguration (i)) << "\" (based on " << targetType << ")" << newLine;

        out << "!MESSAGE " << newLine
            << "# Begin Project" << newLine
            << "# PROP AllowPerConfigDependencies 0" << newLine
            << "# PROP Scc_ProjName \"\"" << newLine
            << "# PROP Scc_LocalPath \"\"" << newLine
            << "CPP=cl.exe" << newLine
            << "MTL=midl.exe" << newLine
            << "RSC=rc.exe" << newLine;

        String targetList;

        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            const Project::BuildConfiguration config (project.getConfiguration (i));
            const String configName (createConfigNameVC6 (config));
            targetList << "# Name \"" << configName << '"' << newLine;

            const String binariesPath (getConfigTargetPath (config));
            const String targetBinary (FileHelpers::windowsStylePath (binariesPath + "/" + getBinaryFileForConfig (config)));
            const String optimisationFlag (((int) config.getOptimisationLevel().getValue() <= 1) ? "Od" : (config.getOptimisationLevel() == 2 ? "O2" : "O3"));
            const String defines (getPreprocessorDefs (config, " /D "));
            const bool isDebug = (bool) config.isDebug().getValue();
            const String extraDebugFlags (isDebug ? "/Gm /ZI /GZ" : "");

            out << (i == 0 ? "!IF" : "!ELSEIF") << "  \"$(CFG)\" == \"" << configName << '"' << newLine
                << "# PROP BASE Use_MFC 0" << newLine
                << "# PROP BASE Use_Debug_Libraries " << (isDebug ? "1" : "0") << newLine
                << "# PROP BASE Output_Dir \"" << binariesPath << '"' << newLine
                << "# PROP BASE Intermediate_Dir \"" << getIntermediatesPath (config) << '"' << newLine
                << "# PROP BASE Target_Dir \"\"" << newLine
                << "# PROP Use_MFC 0" << newLine
                << "# PROP Use_Debug_Libraries " << (isDebug ? "1" : "0") << newLine
                << "# PROP Output_Dir \"" << binariesPath << '"' << newLine
                << "# PROP Intermediate_Dir \"" << getIntermediatesPath (config) << '"' << newLine
                << "# PROP Ignore_Export_Lib 0" << newLine
                << "# PROP Target_Dir \"\"" << newLine
                << "# ADD BASE CPP /nologo /W3 /GX /" << optimisationFlag << " /D " << defines
                << " /YX /FD /c " << extraDebugFlags << " /Zm1024" << newLine
                << "# ADD CPP /nologo " << (isDebug ? "/MTd" : "/MT") << " /W3 /GR /GX /" << optimisationFlag
                << " /I " << getHeaderSearchPaths (config).joinIntoString (" /I ")
                << " /D " << defines << " /D \"_UNICODE\" /D \"UNICODE\" /FD /c /Zm1024 " << extraDebugFlags
                << " " << getExtraCompilerFlags().toString().trim() << newLine;

            if (! isDebug)
                out << "# SUBTRACT CPP /YX" << newLine;

            if (! project.isLibrary())
                out << "# ADD BASE MTL /nologo /D " << defines << " /mktyplib203 /win32" << newLine
                    << "# ADD MTL /nologo /D " << defines << " /mktyplib203 /win32" << newLine;

            out << "# ADD BASE RSC /l 0x40c /d " << defines << newLine
                << "# ADD RSC /l 0x40c /d " << defines << newLine
                << "BSC32=bscmake.exe" << newLine
                << "# ADD BASE BSC32 /nologo" << newLine
                << "# ADD BSC32 /nologo" << newLine;

            if (project.isLibrary())
            {
                out << "LIB32=link.exe -lib" << newLine
                    << "# ADD BASE LIB32 /nologo" << newLine
                    << "# ADD LIB32 /nologo /out:\"" << targetBinary << '"' << newLine;
            }
            else
            {
                out << "LINK32=link.exe" << newLine
                    << "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386" << newLine
                    << "# ADD LINK32 \"C:\\Program Files\\Microsoft Visual Studio\\VC98\\LIB\\shell32.lib\" " // This is avoid debug information corruption when mixing Platform SDK
                    << "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "
                    << (isDebug ? " /debug" : "")
                    << " /nologo /machine:I386 /out:\"" << targetBinary << "\" "
                    << (isDLL ? "/dll" : (project.isCommandLineApp() ? "/subsystem:console "
                                                                     : "/subsystem:windows "))
                    << getExtraLinkerFlags().toString().trim() << newLine;
            }
        }

        out << "!ENDIF" << newLine
            << "# Begin Target"  << newLine
            << targetList;

        writeFilesVC6 (out, project.getMainGroup());
        writeGroupVC6 (out, project.getJuceCodeGroupName(), juceWrapperFiles);
        writeGroupVC6 (out, "Juce VST Wrapper", getVSTFilesRequired());

        out << "# End Target" << newLine
            << "# End Project" << newLine;
    }

    void writeFileVC6 (OutputStream& out, const RelativePath& file, const bool excludeFromBuild)
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        out << "# Begin Source File" << newLine
            << "SOURCE=" << file.toWindowsStyle().quoted() << newLine;

        if (excludeFromBuild)
            out << "# PROP Exclude_From_Build 1" << newLine;

        out << "# End Source File" << newLine;
    }

    void writeFilesVC6 (OutputStream& out, const Project::Item& projectItem)
    {
        if (projectItem.isGroup())
        {
            out << "# Begin Group \"" << projectItem.getName() << '"' << newLine
                << "# PROP Default_Filter \"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\"" << newLine;

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                writeFilesVC6 (out, projectItem.getChild (i));

            out << "# End Group" << newLine;
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);
            writeFileVC6 (out, path, projectItem.shouldBeAddedToBinaryResources() || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()));
        }
    }

    void writeGroupVC6 (OutputStream& out, const String& groupName, const Array<RelativePath>& files)
    {
        if (files.size() > 0)
        {
            out << "# Begin Group \"" << groupName << '"' << newLine;
            for (int i = 0; i < files.size(); ++i)
                if (files.getReference(i).hasFileExtension ("cpp;c;h"))
                    writeFileVC6 (out, files.getReference(i), false);

            out << "# End Group" << newLine;
        }
    }

    void writeDSWFile (OutputStream& out)
    {
        out << "Microsoft Developer Studio Workspace File, Format Version 6.00 " << newLine;

        if (! project.isUsingWrapperFiles())
        {
            out << "Project: \"JUCE\"= ..\\JUCE.dsp - Package Owner=<4>" << newLine
                << "Package=<5>" << newLine
                << "{{{" << newLine
                << "}}}" << newLine
                << "Package=<4>" << newLine
                << "{{{" << newLine
                << "}}}" << newLine;
        }

        out << "Project: \"" << project.getProjectName() << "\" = .\\" << getDSPFile().getFileName() << " - Package Owner=<4>" << newLine
            << "Package=<5>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine
            << "Package=<4>" << newLine
            << "{{{" << newLine;

        if (! project.isUsingWrapperFiles())
        {
            out << "    Begin Project Dependency" << newLine
                << "    Project_Dep_Name JUCE" << newLine
                << "    End Project Dependency" << newLine;
        }

        out << "}}}" << newLine
            << "Global:" << newLine
            << "Package=<5>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine
            << "Package=<3>" << newLine
            << "{{{" << newLine
            << "}}}" << newLine;
    }

    MSVCProjectExporter (const MSVCProjectExporter&);
    MSVCProjectExporter& operator= (const MSVCProjectExporter&);
};


#endif   // __JUCER_PROJECTEXPORT_MSVC_JUCEHEADER__
