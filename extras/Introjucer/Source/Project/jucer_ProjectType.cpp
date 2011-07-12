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
class JuceModule
{
public:
    JuceModule (const File& file)
        : moduleInfo (JSON::parse (file)),
          moduleFolder (file.getParentDirectory())
    {
    }

    static StringArray findAllModuleIDs()
    {
        StringArray ids;
        DirectoryIterator iter (getModulesFolder(), false, "*", File::findDirectories);

        while (iter.next())
        {
            const File moduleDef (iter.getFile().getChildFile ("juce_module.txt"));

            if (moduleDef.exists())
            {
                ScopedPointer<JuceModule> m (new JuceModule (moduleDef));
                jassert (m->isValid());

                if (m->isValid())
                    ids.add (m->getID());
            }
        }

        return ids;
    }

    static JuceModule* createModuleForID (const String& moduleID)
    {
        DirectoryIterator iter (getModulesFolder(), false, "*", File::findDirectories);

        while (iter.next())
        {
            const File moduleDef (iter.getFile().getChildFile ("juce_module.txt"));

            if (moduleDef.exists())
            {
                ScopedPointer<JuceModule> m (new JuceModule (moduleDef));

                if (m->getID() == moduleID)
                    return m.release();
            }
        }

        return nullptr;
    }

    String getID() const        { return moduleInfo ["id"]; }
    bool isValid() const        { return getID().isNotEmpty(); }

    void getDependencies (OwnedArray<JuceModule>& dependencies) const
    {
        const Array<var>* const deps = moduleInfo ["dependencies"].getArray();

        if (deps != nullptr)
        {
            for (int i = 0; i < deps->size(); ++i)
            {
                const String requiredID (deps->getUnchecked(i) ["id"].toString());

                if (requiredID.isNotEmpty()
                      && ! containsModule (dependencies, requiredID))
                {
                    JuceModule* const m = createModuleForID (requiredID);

                    if (m != nullptr)
                        dependencies.add (m);
                }
            }
        }
    }

    var moduleInfo;
    File moduleFolder;

private:
    static bool containsModule (const OwnedArray<JuceModule>& modules, const String& requiredID)
    {
        for (int i = modules.size(); --i >= 0;)
            if (modules.getUnchecked(i)->getID() == requiredID)
                return true;

        return false;
    }

    static File getModulesFolder()
    {
        return StoredSettings::getInstance()->getLastKnownJuceFolder().getChildFile ("modules");
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceModule);
};


//==============================================================================
LibraryModule::LibraryModule()
{
}

//==============================================================================
namespace
{
    Value getVSTFolder (const ProjectExporter& exporter)         { return exporter.getSetting (Ids::vstFolder); }

    void addVSTFolderToPath (const ProjectExporter& exporter, StringArray& searchPaths)
    {
        const String vstFolder (getVSTFolder (exporter).toString());

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
        props.add (new TextPropertyComponent (getVSTFolder (exporter), "VST Folder", 1024, false));
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

    Value shouldBuildVST (const Project& project)                       { return project.getProjectValue ("buildVST"); }
    Value shouldBuildRTAS (const Project& project)                      { return project.getProjectValue ("buildRTAS"); }
    Value shouldBuildAU (const Project& project)                        { return project.getProjectValue ("buildAU"); }

    Value getPluginName (const Project& project)                        { return project.getProjectValue ("pluginName"); }
    Value getPluginDesc (const Project& project)                        { return project.getProjectValue ("pluginDesc"); }
    Value getPluginManufacturer (const Project& project)                { return project.getProjectValue ("pluginManufacturer"); }
    Value getPluginManufacturerCode (const Project& project)            { return project.getProjectValue ("pluginManufacturerCode"); }
    Value getPluginCode (const Project& project)                        { return project.getProjectValue ("pluginCode"); }
    Value getPluginChannelConfigs (const Project& project)              { return project.getProjectValue ("pluginChannelConfigs"); }
    Value getPluginIsSynth (const Project& project)                     { return project.getProjectValue ("pluginIsSynth"); }
    Value getPluginWantsMidiInput (const Project& project)              { return project.getProjectValue ("pluginWantsMidiIn"); }
    Value getPluginProducesMidiOut (const Project& project)             { return project.getProjectValue ("pluginProducesMidiOut"); }
    Value getPluginSilenceInProducesSilenceOut (const Project& project) { return project.getProjectValue ("pluginSilenceInIsSilenceOut"); }
    Value getPluginTailLengthSeconds (const Project& project)           { return project.getProjectValue ("pluginTailLength"); }
    Value getPluginEditorNeedsKeyFocus (const Project& project)         { return project.getProjectValue ("pluginEditorRequiresKeys"); }
    Value getPluginAUExportPrefix (const Project& project)              { return project.getProjectValue ("pluginAUExportPrefix"); }
    Value getPluginAUCocoaViewClassName (const Project& project)        { return project.getProjectValue ("pluginAUViewClass"); }
    Value getPluginRTASCategory (const Project& project)                { return project.getProjectValue ("pluginRTASCategory"); }

    void writePluginCharacteristics (Project& project, OutputStream& out)
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
            << "#define JucePlugin_Build_VST    " << ((bool) shouldBuildVST (project).getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << "#define JucePlugin_Build_AU     " << ((bool) shouldBuildAU (project).getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << "#define JucePlugin_Build_RTAS   " << ((bool) shouldBuildRTAS (project).getValue() ? 1 : 0) << "  // (If you change this value, you'll also need to re-export the projects using the Jucer)" << newLine
            << newLine
            << "#define JucePlugin_Name                 "  << getPluginName (project).toString().quoted() << newLine
            << "#define JucePlugin_Desc                 "  << getPluginDesc (project).toString().quoted() << newLine
            << "#define JucePlugin_Manufacturer         "  << getPluginManufacturer (project).toString().quoted() << newLine
            << "#define JucePlugin_ManufacturerCode     '" << getPluginManufacturerCode (project).toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_PluginCode           '" << getPluginCode (project).toString().trim().substring (0, 4) << "'" << newLine
            << "#define JucePlugin_MaxNumInputChannels  "  << countMaxPluginChannels (getPluginChannelConfigs (project).toString(), true) << newLine
            << "#define JucePlugin_MaxNumOutputChannels "  << countMaxPluginChannels (getPluginChannelConfigs (project).toString(), false) << newLine
            << "#define JucePlugin_PreferredChannelConfigurations   " << getPluginChannelConfigs (project).toString() << newLine
            << "#define JucePlugin_IsSynth              "  << ((bool) getPluginIsSynth (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_WantsMidiInput       "  << ((bool) getPluginWantsMidiInput (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_ProducesMidiOutput   "  << ((bool) getPluginProducesMidiOut (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_SilenceInProducesSilenceOut  " << ((bool) getPluginSilenceInProducesSilenceOut (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_TailLengthSeconds    "  << (double) getPluginTailLengthSeconds (project).getValue() << newLine
            << "#define JucePlugin_EditorRequiresKeyboardFocus  " << ((bool) getPluginEditorNeedsKeyFocus (project).getValue() ? 1 : 0) << newLine
            << "#define JucePlugin_VersionCode          "  << project.getVersionAsHex() << newLine
            << "#define JucePlugin_VersionString        "  << project.getVersion().toString().quoted() << newLine
            << "#define JucePlugin_VSTUniqueID          JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_VSTCategory          "  << ((bool) getPluginIsSynth (project).getValue() ? "kPlugCategSynth" : "kPlugCategEffect") << newLine
            << "#define JucePlugin_AUMainType           "  << ((bool) getPluginIsSynth (project).getValue() ? "kAudioUnitType_MusicDevice" : "kAudioUnitType_Effect") << newLine
            << "#define JucePlugin_AUSubType            JucePlugin_PluginCode" << newLine
            << "#define JucePlugin_AUExportPrefix       "  << getPluginAUExportPrefix (project).toString() << newLine
            << "#define JucePlugin_AUExportPrefixQuoted "  << getPluginAUExportPrefix (project).toString().quoted() << newLine
            << "#define JucePlugin_AUManufacturerCode   JucePlugin_ManufacturerCode" << newLine
            << "#define JucePlugin_CFBundleIdentifier   "  << project.getBundleIdentifier().toString() << newLine
            << "#define JucePlugin_AUCocoaViewClassName "  << getPluginAUCocoaViewClassName (project).toString() << newLine
            << "#define JucePlugin_RTASCategory         "  << ((bool) getPluginIsSynth (project).getValue() ? "ePlugInCategory_SWGenerators" : "ePlugInCategory_None") << newLine
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
        fixMissingValues (exporter);
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
        fixMissingValues (exporter);
        createVSTPathEditor (exporter, props);
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
    {
    }

    static void fixMissingValues (const ProjectExporter& exporter)
    {
        if (getVSTFolder (exporter).toString().isEmpty())
        {
            if (exporter.isVisualStudio())
                getVSTFolder (exporter) = "c:\\SDKs\\vstsdk2.4";
            else
                getVSTFolder (exporter) = "~/SDKs/vstsdk2.4";
        }
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
        fixMissingValues (exporter);

        exporter.xcodeCanUseDwarf = false;

        exporter.msvcTargetSuffix = ".dpm";
        exporter.msvcNeedsDLLRuntimeLib = true;

        RelativePath rtasFolder (getRTASFolder (exporter).toString(), RelativePath::projectFolder);
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
        RelativePath rtasFolder (getRTASFolder (exporter).toString(), RelativePath::projectFolder);

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
            fixMissingValues (exporter);

            props.add (new TextPropertyComponent (getRTASFolder (exporter), "RTAS Folder", 1024, false));
            props.getLast()->setTooltip ("If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
    {
    }

    static void fixMissingValues (const ProjectExporter& exporter)
    {
        if (getRTASFolder (exporter).toString().isEmpty())
        {
            if (exporter.isVisualStudio())
                getRTASFolder (exporter) = "c:\\SDKs\\PT_80_SDK";
            else
                getRTASFolder (exporter) = "~/SDKs/PT_80_SDK";
        }
    }

    static Value getRTASFolder (const ProjectExporter& exporter)        { return exporter.getSetting (Ids::rtasFolder); }
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

void ProjectType::setMissingProjectProperties (Project&) const
{
}

void ProjectType::createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
{
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

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

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

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

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

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

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

    void setMissingProjectProperties (Project& project) const
    {
        if (! project.getProjectRoot().hasProperty (Ids::buildVST))
        {
            const String sanitisedProjectName (CodeHelpers::makeValidIdentifier (project.getProjectName().toString(), false, true, false));

            shouldBuildVST (project) = true;
            shouldBuildRTAS (project) = false;
            shouldBuildAU (project) = true;

            getPluginName (project) = project.getProjectName().toString();
            getPluginDesc (project) = project.getProjectName().toString();
            getPluginManufacturer (project) = "yourcompany";
            getPluginManufacturerCode (project) = "Manu";
            getPluginCode (project) = "Plug";
            getPluginChannelConfigs (project) = "{1, 1}, {2, 2}";
            getPluginIsSynth (project) = false;
            getPluginWantsMidiInput (project) = false;
            getPluginProducesMidiOut (project) = false;
            getPluginSilenceInProducesSilenceOut (project) = false;
            getPluginTailLengthSeconds (project) = 0;
            getPluginEditorNeedsKeyFocus (project) = false;
            getPluginAUExportPrefix (project) = sanitisedProjectName + "AU";
            getPluginAUCocoaViewClassName (project) = sanitisedProjectName + "AU_V1";
            getPluginRTASCategory (project) = String::empty;
        }
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
        props.add (new BooleanPropertyComponent (shouldBuildVST (project), "Build VST", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce a VST plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildAU (project), "Build AudioUnit", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an AudioUnit plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildRTAS (project), "Build RTAS", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an RTAS plugin.");

        props.add (new TextPropertyComponent (getPluginName (project), "Plugin Name", 128, false));
        props.getLast()->setTooltip ("The name of your plugin (keep it short!)");
        props.add (new TextPropertyComponent (getPluginDesc (project), "Plugin Description", 256, false));
        props.getLast()->setTooltip ("A short description of your plugin.");

        props.add (new TextPropertyComponent (getPluginManufacturer (project), "Plugin Manufacturer", 256, false));
        props.getLast()->setTooltip ("The name of your company (cannot be blank).");
        props.add (new TextPropertyComponent (getPluginManufacturerCode (project), "Plugin Manufacturer Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your company. Note that for AU compatibility, this must contain at least one upper-case letter!");
        props.add (new TextPropertyComponent (getPluginCode (project), "Plugin Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your plugin. Note that for AU compatibility, this must contain at least one upper-case letter!");

        props.add (new TextPropertyComponent (getPluginChannelConfigs (project), "Plugin Channel Configurations", 256, false));
        props.getLast()->setTooltip ("This is the set of input/output channel configurations that your plugin can handle.  The list is a comma-separated set of pairs of values in the form { numInputs, numOutputs }, and each "
                                     "pair indicates a valid configuration that the plugin can handle. So for example, {1, 1}, {2, 2} means that the plugin can be used in just two configurations: either with 1 input "
                                     "and 1 output, or with 2 inputs and 2 outputs.");

        props.add (new BooleanPropertyComponent (getPluginIsSynth (project), "Plugin is a Synth", "Is a Synth"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to be treated as a synth or generator. It doesn't make much difference to the plugin itself, but some hosts treat synths differently to other plugins.");

        props.add (new BooleanPropertyComponent (getPluginWantsMidiInput (project), "Plugin Midi Input", "Plugin wants midi input"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to accept midi messages.");

        props.add (new BooleanPropertyComponent (getPluginProducesMidiOut (project), "Plugin Midi Output", "Plugin produces midi output"));
        props.getLast()->setTooltip ("Enable this if your plugin is going to produce midi messages.");

        props.add (new BooleanPropertyComponent (getPluginSilenceInProducesSilenceOut (project), "Silence", "Silence in produces silence out"));
        props.getLast()->setTooltip ("Enable this if your plugin has no tail - i.e. if passing a silent buffer to it will always result in a silent buffer being produced.");

        props.add (new TextPropertyComponent (getPluginTailLengthSeconds (project), "Tail Length (in seconds)", 12, false));
        props.getLast()->setTooltip ("This indicates the length, in seconds, of the plugin's tail. This information may or may not be used by the host.");

        props.add (new BooleanPropertyComponent (getPluginEditorNeedsKeyFocus (project), "Key Focus", "Plugin editor requires keyboard focus"));
        props.getLast()->setTooltip ("Enable this if your plugin needs keyboard input - some hosts can be a bit funny about keyboard focus..");

        props.add (new TextPropertyComponent (getPluginAUExportPrefix (project), "Plugin AU Export Prefix", 64, false));
        props.getLast()->setTooltip ("A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");

        props.add (new TextPropertyComponent (getPluginAUCocoaViewClassName (project), "Plugin AU Cocoa View Name", 64, false));
        props.getLast()->setTooltip ("In an AU, this is the name of Cocoa class that creates the UI. Some hosts bizarrely display the class-name, so you might want to make it reflect your plugin. But the name must be "
                                     "UNIQUE to this exact version of your plugin, to avoid objective-C linkage mix-ups that happen when different plugins containing the same class-name are loaded simultaneously.");

        props.add (new TextPropertyComponent (getPluginRTASCategory (project), "Plugin RTAS Category", 64, false));
        props.getLast()->setTooltip ("(Leave this blank if your plugin is a synth). This is one of the RTAS categories from FicPluginEnums.h, such as: ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics, "
                                     "ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay, "
                                     "ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction, "
                                     "ePlugInCategory_Dither, ePlugInCategory_SoundField");
    }

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

        if (shouldBuildVST (project).getValue())    modules.add (new VSTLibraryModule());
        if (shouldBuildRTAS (project).getValue())   modules.add (new RTASLibraryModule());
        if (shouldBuildAU (project).getValue())     modules.add (new AULibraryModule());
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
