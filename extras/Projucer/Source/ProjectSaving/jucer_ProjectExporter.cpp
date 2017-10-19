/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"

#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_MSVC.h"
#include "jucer_ProjectExport_Xcode.h"
#include "jucer_ProjectExport_Android.h"
#include "jucer_ProjectExport_CodeBlocks.h"

#include "jucer_ProjectExport_CLion.h"

//==============================================================================
static void addType (Array<ProjectExporter::ExporterTypeInfo>& list,
                     const char* name, const void* iconData, int iconDataSize)
{
    ProjectExporter::ExporterTypeInfo type = { name, iconData, iconDataSize };
    list.add (type);
}

Array<ProjectExporter::ExporterTypeInfo> ProjectExporter::getExporterTypes()
{
    Array<ProjectExporter::ExporterTypeInfo> types;

    addType (types, XcodeProjectExporter::getNameMac(),          BinaryData::export_xcode_svg,          BinaryData::export_xcode_svgSize);
    addType (types, XcodeProjectExporter::getNameiOS(),          BinaryData::export_xcode_svg,          BinaryData::export_xcode_svgSize);
    addType (types, MSVCProjectExporterVC2017::getName(),        BinaryData::export_visualStudio_svg,   BinaryData::export_visualStudio_svgSize);
    addType (types, MSVCProjectExporterVC2015::getName(),        BinaryData::export_visualStudio_svg,   BinaryData::export_visualStudio_svgSize);
    addType (types, MSVCProjectExporterVC2013::getName(),        BinaryData::export_visualStudio_svg,   BinaryData::export_visualStudio_svgSize);
    addType (types, MakefileProjectExporter::getNameLinux(),     BinaryData::export_linux_svg,          BinaryData::export_linux_svgSize);
    addType (types, AndroidProjectExporter::getName(),           BinaryData::export_android_svg,        BinaryData::export_android_svgSize);
    addType (types, CodeBlocksProjectExporter::getNameWindows(), BinaryData::export_codeBlocks_svg,     BinaryData::export_codeBlocks_svgSize);
    addType (types, CodeBlocksProjectExporter::getNameLinux(),   BinaryData::export_codeBlocks_svg,     BinaryData::export_codeBlocks_svgSize);
    addType (types, CLionProjectExporter::getName(),             BinaryData::export_clion_svg,          BinaryData::export_clion_svgSize);

    return types;
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const int index)
{
    ProjectExporter* exp = nullptr;

    switch (index)
    {
        case 0:     exp = new XcodeProjectExporter         (project, ValueTree (XcodeProjectExporter         ::getValueTreeTypeName (false)), false); break;
        case 1:     exp = new XcodeProjectExporter         (project, ValueTree (XcodeProjectExporter         ::getValueTreeTypeName (true)), true); break;
        case 2:     exp = new MSVCProjectExporterVC2017    (project, ValueTree (MSVCProjectExporterVC2017    ::getValueTreeTypeName())); break;
        case 3:     exp = new MSVCProjectExporterVC2015    (project, ValueTree (MSVCProjectExporterVC2015    ::getValueTreeTypeName())); break;
        case 4:     exp = new MSVCProjectExporterVC2013    (project, ValueTree (MSVCProjectExporterVC2013    ::getValueTreeTypeName())); break;
        case 5:     exp = new MakefileProjectExporter      (project, ValueTree (MakefileProjectExporter      ::getValueTreeTypeName())); break;
        case 6:     exp = new AndroidProjectExporter       (project, ValueTree (AndroidProjectExporter       ::getValueTreeTypeName())); break;
        case 7:     exp = new CodeBlocksProjectExporter    (project, ValueTree (CodeBlocksProjectExporter    ::getValueTreeTypeName (CodeBlocksProjectExporter::windowsTarget)), CodeBlocksProjectExporter::windowsTarget); break;
        case 8:     exp = new CodeBlocksProjectExporter    (project, ValueTree (CodeBlocksProjectExporter    ::getValueTreeTypeName (CodeBlocksProjectExporter::linuxTarget)),   CodeBlocksProjectExporter::linuxTarget); break;
        case 9:     exp = new CLionProjectExporter         (project, ValueTree (CLionProjectExporter         ::getValueTreeTypeName())); break;
    }

    exp->createDefaultConfigs();
    exp->createDefaultModulePaths();

    return exp;
}

StringArray ProjectExporter::getExporterNames()
{
    StringArray s;

    for (auto& e : getExporterTypes())
        s.add (e.name);

    return s;
}

String ProjectExporter::getValueTreeNameForExporter (const String& exporterName)
{
    if (exporterName == XcodeProjectExporter::getNameMac())
        return XcodeProjectExporter::getValueTreeTypeName (false);

    if (exporterName == XcodeProjectExporter::getNameiOS())
        return XcodeProjectExporter::getValueTreeTypeName (true);

    if (exporterName == MSVCProjectExporterVC2013::getName())
        return MSVCProjectExporterVC2013::getValueTreeTypeName();

    if (exporterName == MSVCProjectExporterVC2015::getName())
        return MSVCProjectExporterVC2015::getValueTreeTypeName();

    if (exporterName == MSVCProjectExporterVC2017::getName())
        return MSVCProjectExporterVC2017::getValueTreeTypeName();

    if (exporterName == MakefileProjectExporter::getNameLinux())
        return MakefileProjectExporter::getValueTreeTypeName();

    if (exporterName == AndroidProjectExporter::getName())
        return AndroidProjectExporter::getValueTreeTypeName();

    if (exporterName == CodeBlocksProjectExporter::getNameLinux())
        return CodeBlocksProjectExporter::getValueTreeTypeName (CodeBlocksProjectExporter::CodeBlocksOS::linuxTarget);

    if (exporterName == CodeBlocksProjectExporter::getNameWindows())
        return CodeBlocksProjectExporter::getValueTreeTypeName (CodeBlocksProjectExporter::CodeBlocksOS::windowsTarget);

    if (exporterName == CLionProjectExporter::getName())
        return CLionProjectExporter::getValueTreeTypeName();

    return {};
}

StringArray ProjectExporter::getAllDefaultBuildsFolders()
{
    StringArray folders;

    folders.add (getDefaultBuildsRootFolder() + "iOS");
    folders.add (getDefaultBuildsRootFolder() + "MacOSX");
    folders.add (getDefaultBuildsRootFolder() + "VisualStudio2013");
    folders.add (getDefaultBuildsRootFolder() + "VisualStudio2015");
    folders.add (getDefaultBuildsRootFolder() + "VisualStudio2017");
    folders.add (getDefaultBuildsRootFolder() + "LinuxMakefile");
    folders.add (getDefaultBuildsRootFolder() + "CodeBlocksWindows");
    folders.add (getDefaultBuildsRootFolder() + "CodeBlocksLinux");
    folders.add (getDefaultBuildsRootFolder() + "Android");
    folders.add (getDefaultBuildsRootFolder() + "CLion");

    return folders;
}

String ProjectExporter::getCurrentPlatformExporterName()
{
   #if JUCE_MAC
    return XcodeProjectExporter::getNameMac();
   #elif JUCE_WINDOWS
    return MSVCProjectExporterVC2017::getName();
   #elif JUCE_LINUX
    return MakefileProjectExporter::getNameLinux();
   #else
    #error // huh?
   #endif
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const String& name)
{
    return createNewExporter (project, getExporterNames().indexOf (name));
}

ProjectExporter* ProjectExporter::createExporter (Project& project, const ValueTree& settings)
{
    ProjectExporter*       exp = MSVCProjectExporterVC2013    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2015    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2017    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = XcodeProjectExporter         ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MakefileProjectExporter      ::createForSettings (project, settings);
    if (exp == nullptr)    exp = AndroidProjectExporter       ::createForSettings (project, settings);
    if (exp == nullptr)    exp = CodeBlocksProjectExporter    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = CLionProjectExporter         ::createForSettings (project, settings);

    jassert (exp != nullptr);
    return exp;
}

bool ProjectExporter::canProjectBeLaunched (Project* project)
{
    if (project != nullptr)
    {
        const char* types[] =
        {
           #if JUCE_MAC
            XcodeProjectExporter::getValueTreeTypeName (false),
            XcodeProjectExporter::getValueTreeTypeName (true),
           #elif JUCE_WINDOWS
            MSVCProjectExporterVC2013::getValueTreeTypeName(),
            MSVCProjectExporterVC2015::getValueTreeTypeName(),
            MSVCProjectExporterVC2017::getValueTreeTypeName(),
           #elif JUCE_LINUX
            // (this doesn't currently launch.. not really sure what it would do on linux)
            //MakefileProjectExporter::getValueTreeTypeName(),
           #endif
            AndroidProjectExporter::getValueTreeTypeName(),

            nullptr
        };

        for (const char** type = types; *type != nullptr; ++type)
            if (project->getExporters().getChildWithName (*type).isValid())
                return true;
    }

    return false;
}

//==============================================================================
ProjectExporter::ProjectExporter (Project& p, const ValueTree& state)
    : settings (state),
      project (p),
      projectType (p.getProjectType()),
      projectName (p.getTitle()),
      projectFolder (p.getProjectFolder())
{
}

ProjectExporter::~ProjectExporter()
{
}

void ProjectExporter::updateDeprecatedProjectSettingsInteractively() {}

String ProjectExporter::getName() const
{
    if (! getAllDefaultBuildsFolders().contains (getTargetLocationString()))
        return name + " - " + getTargetLocationString();

    return name;
}

File ProjectExporter::getTargetFolder() const
{
    return project.resolveFilename (getTargetLocationString());
}

RelativePath ProjectExporter::rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const
{
    return path.rebased (project.getProjectFolder(), getTargetFolder(), RelativePath::buildTargetFolder);
}

bool ProjectExporter::shouldFileBeCompiledByDefault (const RelativePath& file) const
{
    return file.hasFileExtension (cOrCppFileExtensions)
        || file.hasFileExtension (asmFileExtensions);
}

//==============================================================================
void ProjectExporter::createPropertyEditors (PropertyListBuilder& props)
{
    if (! isCLion())
    {
        props.add (new TextPropertyComponent (getTargetLocationValue(), "Target Project Folder", 2048, false),
                   "The location of the folder in which the " + name + " project will be created. "
                   "This path can be absolute, but it's much more sensible to make it relative to the jucer project directory.");

        createDependencyPathProperties (props);

        props.add (new TextPropertyComponent (getExporterPreprocessorDefs(), "Extra Preprocessor Definitions", 32768, true),
                   "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, "
                   "or new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

        props.add (new TextPropertyComponent (getExtraCompilerFlags(), "Extra compiler flags", 8192, true),
                   "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor definitions in the "
                   "form ${NAME_OF_DEFINITION}, which will be replaced with their values.");

        props.add (new TextPropertyComponent (getExtraLinkerFlags(), "Extra linker flags", 8192, true),
                   "Extra command-line flags to be passed to the linker. You might want to use this for adding additional libraries. "
                   "This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");

        props.add (new TextPropertyComponent (getExternalLibraries(), "External libraries to link", 8192, true),
                   "Additional libraries to link (one per line). You should not add any platform specific decoration to these names. "
                   "This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");

        if (! isVisualStudio())
            props.add (new BooleanPropertyComponent (getShouldUseGNUExtensionsValue(), "GNU Compiler Extensions", "Enabled"),
                       "Enabling this will use the GNU C++ language standard variant for compilation.");

        createIconProperties (props);
    }

    createExporterProperties (props);

    props.add (new TextPropertyComponent (getUserNotes(), "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

void ProjectExporter::createDependencyPathProperties (PropertyListBuilder& props)
{
    if (shouldBuildTargetType (ProjectType::Target::VST3PlugIn) || project.isVST3PluginHost())
    {
        if (dynamic_cast<DependencyPathValueSource*> (&getVST3PathValue().getValueSource()) != nullptr)
            props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), getVST3PathValue(), "VST3 SDK Folder"),
                       "If you're building a VST3 plugin or host, this must be the folder containing the VST3 SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (shouldBuildTargetType (ProjectType::Target::AAXPlugIn) && project.shouldBuildAAX())
    {
        if (dynamic_cast<DependencyPathValueSource*> (&getAAXPathValue().getValueSource()) != nullptr)
            props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), getAAXPathValue(), "AAX SDK Folder"),
                       "If you're building an AAX plugin, this must be the folder containing the AAX SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (shouldBuildTargetType (ProjectType::Target::RTASPlugIn) && project.shouldBuildRTAS())
    {
        if (dynamic_cast<DependencyPathValueSource*> (&getRTASPathValue().getValueSource()) != nullptr)
            props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), getRTASPathValue(), "RTAS SDK Folder"),
                       "If you're building an RTAS, this must be the folder containing the RTAS SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }
}

void ProjectExporter::createIconProperties (PropertyListBuilder& props)
{
    OwnedArray<Project::Item> images;
    project.findAllImageItems (images);

    StringArray choices;
    Array<var> ids;

    choices.add ("<None>");
    ids.add (var());
    choices.add (String());
    ids.add (var());

    for (int i = 0; i < images.size(); ++i)
    {
        choices.add (images.getUnchecked(i)->getName());
        ids.add (images.getUnchecked(i)->getID());
    }

    props.add (new ChoicePropertyComponent (getSmallIconImageItemID(), "Icon (small)", choices, ids),
               "Sets an icon to use for the executable.");

    props.add (new ChoicePropertyComponent (getBigIconImageItemID(), "Icon (large)", choices, ids),
               "Sets an icon to use for the executable.");
}

//==============================================================================
void ProjectExporter::addSettingsForProjectType (const ProjectType& type)
{
    addVSTPathsIfPluginOrHost();

    if (type.isAudioPlugin())
        addCommonAudioPluginSettings();

    addPlatformSpecificSettingsForProjectType (type);
}

void ProjectExporter::addVSTPathsIfPluginOrHost()
{
    if (shouldBuildTargetType (ProjectType::Target::VST3PlugIn) || project.isVST3PluginHost())
        addVST3FolderToPath();
}

void ProjectExporter::addCommonAudioPluginSettings()
{
    if (shouldBuildTargetType (ProjectType::Target::AAXPlugIn))
        addAAXFoldersToPath();

    // Note: RTAS paths are platform-dependent, impl -> addPlatformSpecificSettingsForProjectType
 }

void ProjectExporter::addVST3FolderToPath()
{
    const String vst3Folder (getVST3PathValue().toString());

    if (vst3Folder.isNotEmpty())
        addToExtraSearchPaths (RelativePath (vst3Folder, RelativePath::projectFolder), 0);
}

void ProjectExporter::addAAXFoldersToPath()
{
    const String aaxFolder = getAAXPathValue().toString();

    if (aaxFolder.isNotEmpty())
    {
        const RelativePath aaxFolderPath (getAAXPathValue().toString(), RelativePath::projectFolder);

        addToExtraSearchPaths (aaxFolderPath);
        addToExtraSearchPaths (aaxFolderPath.getChildFile ("Interfaces"));
        addToExtraSearchPaths (aaxFolderPath.getChildFile ("Interfaces").getChildFile ("ACF"));
    }
}

//==============================================================================
StringPairArray ProjectExporter::getAllPreprocessorDefs (const BuildConfiguration& config, const ProjectType::Target::Type targetType) const
{
    StringPairArray defs (mergePreprocessorDefs (config.getAllPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    addDefaultPreprocessorDefs (defs);
    addTargetSpecificPreprocessorDefs (defs, targetType);

    return defs;
}

StringPairArray ProjectExporter::getAllPreprocessorDefs() const
{
    StringPairArray defs (mergePreprocessorDefs (project.getPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    addDefaultPreprocessorDefs (defs);
    return defs;
}

void ProjectExporter::addTargetSpecificPreprocessorDefs (StringPairArray& defs, const ProjectType::Target::Type targetType) const
{
    std::pair<String, ProjectType::Target::Type> targetFlags[] = {
        {"JucePlugin_Build_VST",        ProjectType::Target::VSTPlugIn},
        {"JucePlugin_Build_VST3",       ProjectType::Target::VST3PlugIn},
        {"JucePlugin_Build_AU",         ProjectType::Target::AudioUnitPlugIn},
        {"JucePlugin_Build_AUv3",       ProjectType::Target::AudioUnitv3PlugIn},
        {"JucePlugin_Build_RTAS",       ProjectType::Target::RTASPlugIn},
        {"JucePlugin_Build_AAX",        ProjectType::Target::AAXPlugIn},
        {"JucePlugin_Build_Standalone", ProjectType::Target::StandalonePlugIn}
    };

    if (targetType == ProjectType::Target::SharedCodeTarget)
    {
        for (auto& flag : targetFlags)
            defs.set (flag.first, (shouldBuildTargetType (flag.second) ? "1" : "0"));

        defs.set ("JUCE_SHARED_CODE", "1");
    }
    else if (targetType != ProjectType::Target::unspecified)
    {
        for (auto& flag : targetFlags)
            defs.set (flag.first, (targetType == flag.second ? "1" : "0"));
    }
}

void ProjectExporter::addDefaultPreprocessorDefs (StringPairArray& defs) const
{
    defs.set (getExporterIdentifierMacro(), "1");
    defs.set ("JUCE_APP_VERSION", project.getVersionString());
    defs.set ("JUCE_APP_VERSION_HEX", project.getVersionAsHex());
}

String ProjectExporter::replacePreprocessorTokens (const ProjectExporter::BuildConfiguration& config,
                                                   const String& sourceString) const
{
    return replacePreprocessorDefs (getAllPreprocessorDefs (config, ProjectType::Target::unspecified), sourceString);
}

void ProjectExporter::copyMainGroupFromProject()
{
    jassert (itemGroups.size() == 0);
    itemGroups.add (project.getMainGroup().createCopy());
}

Project::Item& ProjectExporter::getModulesGroup()
{
    if (modulesGroup == nullptr)
    {
        jassert (itemGroups.size() > 0); // must call copyMainGroupFromProject before this.
        itemGroups.add (Project::Item::createGroup (project, "Juce Modules", "__modulesgroup__", true));
        modulesGroup = &(itemGroups.getReference (itemGroups.size() - 1));
    }

    return *modulesGroup;
}

void ProjectExporter::addProjectPathToBuildPathList (StringArray& pathList, const RelativePath& pathFromProjectFolder, int index) const
{
    const auto localPath = RelativePath (rebaseFromProjectFolderToBuildTarget (pathFromProjectFolder));

    const auto path = isVisualStudio() ? localPath.toWindowsStyle() : localPath.toUnixStyle();

    if (! pathList.contains (path))
        pathList.insert (index, path);
}

void ProjectExporter::addToModuleLibPaths (const RelativePath& pathFromProjectFolder)
{
    addProjectPathToBuildPathList (moduleLibSearchPaths, pathFromProjectFolder);
}

void ProjectExporter::addToExtraSearchPaths (const RelativePath& pathFromProjectFolder, int index)
{
    addProjectPathToBuildPathList (extraSearchPaths, pathFromProjectFolder, index);
}

Value ProjectExporter::getPathForModuleValue (const String& moduleID)
{
    UndoManager* um = project.getUndoManagerFor (settings);

    ValueTree paths (settings.getOrCreateChildWithName (Ids::MODULEPATHS, um));
    ValueTree m (paths.getChildWithProperty (Ids::ID, moduleID));

    if (! m.isValid())
    {
        m = ValueTree (Ids::MODULEPATH);
        m.setProperty (Ids::ID, moduleID, um);
        paths.addChild (m, -1, um);
    }

    return m.getPropertyAsValue (Ids::path, um);
}

String ProjectExporter::getPathForModuleString (const String& moduleID) const
{
    auto exporterPath = settings.getChildWithName (Ids::MODULEPATHS)
                                .getChildWithProperty (Ids::ID, moduleID) [Ids::path].toString();

    if (exporterPath.isEmpty() || project.getModules().shouldUseGlobalPath (moduleID))
    {
        auto id = EnabledModuleList::isJuceModule (moduleID) ? Ids::defaultJuceModulePath
                                                             : Ids::defaultUserModulePath;

        if (TargetOS::getThisOS() != getTargetOSForExporter())
            return getAppSettings().getFallbackPathForOS (id, getTargetOSForExporter()).toString();

        if (id == Ids::defaultJuceModulePath)
            return getAppSettings().getStoredPath (Ids::defaultJuceModulePath).toString();

        return getAppSettings().getStoredPath (Ids::defaultUserModulePath).toString();
    }

    return exporterPath;
}

void ProjectExporter::removePathForModule (const String& moduleID)
{
    ValueTree paths (settings.getChildWithName (Ids::MODULEPATHS));
    ValueTree m (paths.getChildWithProperty (Ids::ID, moduleID));
    paths.removeChild (m, project.getUndoManagerFor (settings));
}

TargetOS::OS ProjectExporter::getTargetOSForExporter() const
{
    auto targetOS = TargetOS::unknown;

    if      (isWindows())            targetOS = TargetOS::windows;
    else if (isOSX() || isiOS())     targetOS = TargetOS::osx;
    else if (isLinux())              targetOS = TargetOS::linux;
    else if (isAndroid())            targetOS = TargetOS::getThisOS();

    return targetOS;
}

RelativePath ProjectExporter::getModuleFolderRelativeToProject (const String& moduleID) const
{
    if (project.getModules().shouldCopyModuleFilesLocally (moduleID).getValue())
        return RelativePath (project.getRelativePathForFile (project.getLocalModuleFolder (moduleID)),
                             RelativePath::projectFolder);

    String path (getPathForModuleString (moduleID));

    if (path.isEmpty())
        return getLegacyModulePath (moduleID).getChildFile (moduleID);

    return RelativePath (path, RelativePath::projectFolder).getChildFile (moduleID);
}

String ProjectExporter::getLegacyModulePath() const
{
    return getSettingString ("juceFolder");
}

RelativePath ProjectExporter::getLegacyModulePath (const String& moduleID) const
{
    if (project.getModules().state.getChildWithProperty (Ids::ID, moduleID) ["useLocalCopy"])
        return RelativePath (project.getRelativePathForFile (project.getGeneratedCodeFolder()
                                                                .getChildFile ("modules")
                                                                .getChildFile (moduleID)), RelativePath::projectFolder);

    String oldJucePath (getLegacyModulePath());

    if (oldJucePath.isEmpty())
        return RelativePath();

    RelativePath p (oldJucePath, RelativePath::projectFolder);
    if (p.getFileName() != "modules")
        p = p.getChildFile ("modules");

    return p.getChildFile (moduleID);
}

void ProjectExporter::updateOldModulePaths()
{
    String oldPath (getLegacyModulePath());

    if (oldPath.isNotEmpty())
    {
        for (int i = project.getModules().getNumModules(); --i >= 0;)
        {
            String modID (project.getModules().getModuleID(i));
            getPathForModuleValue (modID) = getLegacyModulePath (modID).getParentDirectory().toUnixStyle();
        }

        settings.removeProperty ("juceFolder", nullptr);
    }
}

static bool areCompatibleExporters (const ProjectExporter& p1, const ProjectExporter& p2)
{
    return (p1.isVisualStudio() && p2.isVisualStudio())
        || (p1.isXcode() && p2.isXcode())
        || (p1.isMakefile() && p2.isMakefile())
        || (p1.isAndroidStudio() && p2.isAndroidStudio())
        || (p1.isCodeBlocks() && p2.isCodeBlocks() && p1.isWindows() != p2.isLinux());
}

void ProjectExporter::createDefaultModulePaths()
{
    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (areCompatibleExporters (*this, *exporter))
        {
            for (int i = project.getModules().getNumModules(); --i >= 0;)
            {
                String modID (project.getModules().getModuleID(i));
                getPathForModuleValue (modID) = exporter->getPathForModuleValue (modID).getValue();
            }

            return;
        }
    }

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (exporter->canLaunchProject())
        {
            for (int i = project.getModules().getNumModules(); --i >= 0;)
            {
                String modID (project.getModules().getModuleID(i));
                getPathForModuleValue (modID) = exporter->getPathForModuleValue (modID).getValue();
            }

            return;
        }
    }

    for (int i = project.getModules().getNumModules(); --i >= 0;)
    {
        String modID (project.getModules().getModuleID(i));
        getPathForModuleValue (modID) = "../../juce";
    }
}

//==============================================================================
ValueTree ProjectExporter::getConfigurations() const
{
    return settings.getChildWithName (Ids::CONFIGURATIONS);
}

int ProjectExporter::getNumConfigurations() const
{
    return getConfigurations().getNumChildren();
}

ProjectExporter::BuildConfiguration::Ptr ProjectExporter::getConfiguration (int index) const
{
    return createBuildConfig (getConfigurations().getChild (index));
}

bool ProjectExporter::hasConfigurationNamed (const String& nameToFind) const
{
    const ValueTree configs (getConfigurations());
    for (int i = configs.getNumChildren(); --i >= 0;)
        if (configs.getChild(i) [Ids::name].toString() == nameToFind)
            return true;

    return false;
}

String ProjectExporter::getUniqueConfigName (String nm) const
{
    String nameRoot (nm);
    while (CharacterFunctions::isDigit (nameRoot.getLastCharacter()))
        nameRoot = nameRoot.dropLastCharacters (1);

    nameRoot = nameRoot.trim();

    int suffix = 2;
    while (hasConfigurationNamed (name))
        nm = nameRoot + " " + String (suffix++);

    return nm;
}

void ProjectExporter::addNewConfiguration (const BuildConfiguration* configToCopy)
{
    const String configName (getUniqueConfigName (configToCopy != nullptr ? configToCopy->config [Ids::name].toString()
                                                                          : "New Build Configuration"));

    ValueTree configs (getConfigurations());

    if (! configs.isValid())
    {
        settings.addChild (ValueTree (Ids::CONFIGURATIONS), 0, project.getUndoManagerFor (settings));
        configs = getConfigurations();
    }

    ValueTree newConfig (Ids::CONFIGURATION);
    if (configToCopy != nullptr)
        newConfig = configToCopy->config.createCopy();

    newConfig.setProperty (Ids::name, configName, 0);

    configs.addChild (newConfig, -1, project.getUndoManagerFor (configs));
}

void ProjectExporter::BuildConfiguration::removeFromExporter()
{
    ValueTree configs (config.getParent());
    configs.removeChild (config, project.getUndoManagerFor (configs));
}

void ProjectExporter::createDefaultConfigs()
{
    settings.getOrCreateChildWithName (Ids::CONFIGURATIONS, nullptr);

    for (int i = 0; i < 2; ++i)
    {
        addNewConfiguration (nullptr);
        BuildConfiguration::Ptr config (getConfiguration (i));

        const bool debugConfig = i == 0;

        config->getNameValue() = debugConfig ? "Debug" : "Release";
        config->isDebugValue() = debugConfig;
        config->getOptimisationLevel() = config->getDefaultOptimisationLevel();
        config->getLinkTimeOptimisationEnabledValue() = ! debugConfig;
        config->getTargetBinaryName() = project.getProjectFilenameRoot();
    }
}

Drawable* ProjectExporter::getBigIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::bigIcon]).loadAsImageFile();
}

Drawable* ProjectExporter::getSmallIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::smallIcon]).loadAsImageFile();
}

Image ProjectExporter::getBestIconForSize (int size, bool returnNullIfNothingBigEnough) const
{
    Drawable* im = nullptr;

    ScopedPointer<Drawable> im1 (getSmallIcon());
    ScopedPointer<Drawable> im2 (getBigIcon());

    if (im1 != nullptr && im2 != nullptr)
    {
        if (im1->getWidth() >= size && im2->getWidth() >= size)
            im = im1->getWidth() < im2->getWidth() ? im1 : im2;
        else if (im1->getWidth() >= size)
            im = im1;
        else if (im2->getWidth() >= size)
            im = im2;
    }
    else
    {
        im = im1 != nullptr ? im1 : im2;
    }

    if (im == nullptr)
        return Image();

    if (returnNullIfNothingBigEnough && im->getWidth() < size && im->getHeight() < size)
        return Image();

    return rescaleImageForIcon (*im, size);
}

Image ProjectExporter::rescaleImageForIcon (Drawable& d, const int size)
{
    if (DrawableImage* drawableImage = dynamic_cast<DrawableImage*> (&d))
    {
        Image im = SoftwareImageType().convert (drawableImage->getImage());

        if (size == im.getWidth() && size == im.getHeight())
            return im;

        // (scale it down in stages for better resampling)
        while (im.getWidth() > 2 * size && im.getHeight() > 2 * size)
            im = im.rescaled (im.getWidth() / 2,
                              im.getHeight() / 2);

        Image newIm (Image::ARGB, size, size, true, SoftwareImageType());
        Graphics g (newIm);
        g.drawImageWithin (im, 0, 0, size, size,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
        return newIm;
    }

    Image im (Image::ARGB, size, size, true, SoftwareImageType());
    Graphics g (im);
    d.drawWithin (g, im.getBounds().toFloat(), RectanglePlacement::centred, 1.0f);
    return im;
}


//==============================================================================
ProjectExporter::ConfigIterator::ConfigIterator (ProjectExporter& e)
    : index (-1), exporter (e)
{
}

bool ProjectExporter::ConfigIterator::next()
{
    if (++index >= exporter.getNumConfigurations())
        return false;

    config = exporter.getConfiguration (index);
    return true;
}

ProjectExporter::ConstConfigIterator::ConstConfigIterator (const ProjectExporter& exporter_)
    : index (-1), exporter (exporter_)
{
}

bool ProjectExporter::ConstConfigIterator::next()
{
    if (++index >= exporter.getNumConfigurations())
        return false;

    config = exporter.getConfiguration (index);
    return true;
}

//==============================================================================
ProjectExporter::BuildConfiguration::BuildConfiguration (Project& p, const ValueTree& configNode, const ProjectExporter& e)
   : config (configNode), project (p), exporter (e)
{
}

ProjectExporter::BuildConfiguration::~BuildConfiguration()
{
}

String ProjectExporter::BuildConfiguration::getGCCOptimisationFlag() const
{
    switch (getOptimisationLevelInt())
    {
        case gccO0:     return "0";
        case gccO1:     return "1";
        case gccO2:     return "2";
        case gccO3:     return "3";
        case gccOs:     return "s";
        case gccOfast:  return "fast";
        default:        break;
    }

    return "0";
}

void ProjectExporter::BuildConfiguration::addGCCOptimisationProperty (PropertyListBuilder& props)
{
    static const char* optimisationLevels[] = { "-O0 (no optimisation)",
                                                "-Os (minimise code size)",
                                                "-O1 (fast)",
                                                "-O2 (faster)",
                                                "-O3 (fastest with safe optimisations)",
                                                "-Ofast (uses aggressive optimisations)",
                                                nullptr };

    static const int optimisationLevelValues[] = { gccO0,
                                                   gccOs,
                                                   gccO1,
                                                   gccO2,
                                                   gccO3,
                                                   gccOfast,
                                                   0 };

    props.add (new ChoicePropertyComponent (getOptimisationLevel(), "Optimisation",
                                            StringArray (optimisationLevels),
                                            Array<var> (optimisationLevelValues)),
               "The optimisation level for this configuration");
}

void ProjectExporter::BuildConfiguration::createPropertyEditors (PropertyListBuilder& props)
{
    if (exporter.supportsUserDefinedConfigurations())
        props.add (new TextPropertyComponent (getNameValue(), "Name", 96, false),
                   "The name of this configuration.");

    props.add (new BooleanPropertyComponent (isDebugValue(), "Debug mode", "Debugging enabled"),
               "If enabled, this means that the configuration should be built with debug symbols.");

    props.add (new TextPropertyComponent (getTargetBinaryName(), "Binary name", 256, false),
               "The filename to use for the destination binary executable file. If you don't add a suffix to this name, "
               "a suitable platform-specific suffix will be added automatically.");

    props.add (new TextPropertyComponent (getTargetBinaryRelativePath(), "Binary location", 1024, false),
               "The folder in which the finished binary should be placed. Leave this blank to cause the binary to be placed "
               "in its default location in the build folder.");

    props.addSearchPathProperty (getHeaderSearchPathValue(), "Header search paths", "Extra header search paths.");
    props.addSearchPathProperty (getLibrarySearchPathValue(), "Extra library search paths", "Extra library search paths.");

    props.add (new TextPropertyComponent (getBuildConfigPreprocessorDefs(), "Preprocessor definitions", 32768, true),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, or "
               "new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new BooleanPropertyComponent (getLinkTimeOptimisationEnabledValue(), "Link-Time Optimisation", "Enabled"),
               "Enable this to perform link-time code optimisation. This is recommended for release builds.");

    createConfigProperties (props);

    props.add (new TextPropertyComponent (getUserNotes(), "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

StringPairArray ProjectExporter::BuildConfiguration::getAllPreprocessorDefs() const
{
    return mergePreprocessorDefs (project.getPreprocessorDefs(),
                                  parsePreprocessorDefs (getBuildConfigPreprocessorDefsString()));
}

StringPairArray ProjectExporter::BuildConfiguration::getUniquePreprocessorDefs() const
{
    StringPairArray perConfigurationDefs (parsePreprocessorDefs (getBuildConfigPreprocessorDefsString()));
    const StringPairArray globalDefs (project.getPreprocessorDefs());

    for (int i = 0; i < globalDefs.size(); ++i)
    {
        String globalKey   = globalDefs.getAllKeys()[i];

        int idx = perConfigurationDefs.getAllKeys().indexOf (globalKey);
        if (idx >= 0)
        {
            String globalValue = globalDefs.getAllValues()[i];

            if (globalValue == perConfigurationDefs.getAllValues()[idx])
                perConfigurationDefs.remove (idx);
        }
    }

    return perConfigurationDefs;
}

StringArray ProjectExporter::BuildConfiguration::getHeaderSearchPaths() const
{
    return getSearchPathsFromString (getHeaderSearchPathString() + ';' + project.getHeaderSearchPaths());
}

StringArray ProjectExporter::BuildConfiguration::getLibrarySearchPaths() const
{
    auto separator = exporter.isVisualStudio() ? "\\" : "/";
    auto s = getSearchPathsFromString (getLibrarySearchPathString());

    for (auto path : exporter.moduleLibSearchPaths)
        s.add (path + separator + getModuleLibraryArchName());

    return s;
}

String ProjectExporter::getExternalLibraryFlags (const BuildConfiguration& config) const
{
    StringArray libraries;
    libraries.addTokens (getExternalLibrariesString(), ";\n", "\"'");
    libraries.removeEmptyStrings (true);

    if (libraries.size() != 0)
        return replacePreprocessorTokens (config, "-l" + libraries.joinIntoString (" -l")).trim();

    return {};
}
