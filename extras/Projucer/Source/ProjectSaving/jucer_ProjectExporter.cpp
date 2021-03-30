/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

#include "../Utility/UI/PropertyComponents/jucer_FilePathPropertyComponent.h"

//==============================================================================
std::vector<ProjectExporter::ExporterTypeInfo> ProjectExporter::getExporterTypeInfos()
{
    auto createIcon = [] (const void* iconData, size_t iconDataSize)
    {
        Image image (Image::ARGB, 200, 200, true);
        Graphics g (image);

        std::unique_ptr<Drawable> svgDrawable (Drawable::createFromImageData (iconData, iconDataSize));

        svgDrawable->drawWithin (g, image.getBounds().toFloat(), RectanglePlacement::fillDestination, 1.0f);

        return image;
    };

    using namespace BinaryData;

    static std::vector<ProjectExporter::ExporterTypeInfo> infos
    {
        { XcodeProjectExporter::getValueTreeTypeNameMac(),
          XcodeProjectExporter::getDisplayNameMac(),
          XcodeProjectExporter::getTargetFolderNameMac(),
          createIcon (export_xcode_svg, (size_t) export_xcode_svgSize) },
        { XcodeProjectExporter::getValueTreeTypeNameiOS(),
          XcodeProjectExporter::getDisplayNameiOS(),
          XcodeProjectExporter::getTargetFolderNameiOS(),
          createIcon (export_xcode_svg, (size_t) export_xcode_svgSize) },

        { MSVCProjectExporterVC2019::getValueTreeTypeName(),
          MSVCProjectExporterVC2019::getDisplayName(),
          MSVCProjectExporterVC2019::getTargetFolderName(),
          createIcon (export_visualStudio_svg, export_visualStudio_svgSize) },
        { MSVCProjectExporterVC2017::getValueTreeTypeName(),
          MSVCProjectExporterVC2017::getDisplayName(),
          MSVCProjectExporterVC2017::getTargetFolderName(),
          createIcon (export_visualStudio_svg, export_visualStudio_svgSize) },
        { MSVCProjectExporterVC2015::getValueTreeTypeName(),
          MSVCProjectExporterVC2015::getDisplayName(),
          MSVCProjectExporterVC2015::getTargetFolderName(),
          createIcon (export_visualStudio_svg, export_visualStudio_svgSize) },

        { MakefileProjectExporter::getValueTreeTypeName(),
          MakefileProjectExporter::getDisplayName(),
          MakefileProjectExporter::getTargetFolderName(),
          createIcon (export_linux_svg, export_linux_svgSize) },

        { AndroidProjectExporter::getValueTreeTypeName(),
          AndroidProjectExporter::getDisplayName(),
          AndroidProjectExporter::getTargetFolderName(),
          createIcon (export_android_svg, export_android_svgSize) },

        { CodeBlocksProjectExporter::getValueTreeTypeNameWindows(),
          CodeBlocksProjectExporter::getDisplayNameWindows(),
          CodeBlocksProjectExporter::getTargetFolderNameWindows(),
          createIcon (export_codeBlocks_svg, export_codeBlocks_svgSize) },
        { CodeBlocksProjectExporter::getValueTreeTypeNameLinux(),
          CodeBlocksProjectExporter::getDisplayNameLinux(),
          CodeBlocksProjectExporter::getTargetFolderNameLinux(),
          createIcon (export_codeBlocks_svg, export_codeBlocks_svgSize) },

        { CLionProjectExporter::getValueTreeTypeName(),
          CLionProjectExporter::getDisplayName(),
          CLionProjectExporter::getTargetFolderName(),
          createIcon (export_clion_svg, export_clion_svgSize) }
    };

    return infos;
}

ProjectExporter::ExporterTypeInfo ProjectExporter::getTypeInfoForExporter (const Identifier& exporterIdentifier)
{
    auto typeInfos = getExporterTypeInfos();

    auto predicate = [exporterIdentifier] (const ProjectExporter::ExporterTypeInfo& info)
    {
        return info.identifier == exporterIdentifier;
    };

    auto iter = std::find_if (typeInfos.begin(), typeInfos.end(),
                              std::move (predicate));

    if (iter != typeInfos.end())
        return *iter;

    jassertfalse;
    return {};
}

ProjectExporter::ExporterTypeInfo ProjectExporter::getCurrentPlatformExporterTypeInfo()
{
    #if JUCE_MAC
     return ProjectExporter::getTypeInfoForExporter (XcodeProjectExporter::getValueTreeTypeNameMac());
    #elif JUCE_WINDOWS
     return ProjectExporter::getTypeInfoForExporter (MSVCProjectExporterVC2019::getValueTreeTypeName());
    #elif JUCE_LINUX || JUCE_BSD
     return ProjectExporter::getTypeInfoForExporter (MakefileProjectExporter::getValueTreeTypeName());
    #else
     #error "unknown platform!"
    #endif
}

std::unique_ptr<ProjectExporter> ProjectExporter::createNewExporter (Project& project, const Identifier& exporterIdentifier)
{
    auto exporter = createExporterFromSettings (project, ValueTree (exporterIdentifier));
    jassert (exporter != nullptr);

    exporter->createDefaultConfigs();
    exporter->createDefaultModulePaths();

    return exporter;
}

std::unique_ptr<ProjectExporter> ProjectExporter::createExporterFromSettings (Project& project, const ValueTree& settings)
{
    std::unique_ptr<ProjectExporter> exporter;

    exporter.reset (XcodeProjectExporter::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (MSVCProjectExporterVC2019::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (MSVCProjectExporterVC2017::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (MSVCProjectExporterVC2015::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (MakefileProjectExporter::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (AndroidProjectExporter::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (CodeBlocksProjectExporter::createForSettings (project, settings));
    if (exporter == nullptr) exporter.reset (CLionProjectExporter::createForSettings (project, settings));

    jassert (exporter != nullptr);
    return exporter;
}

bool ProjectExporter::canProjectBeLaunched (Project* project)
{
    if (project != nullptr)
    {
        static Identifier types[]
        {
            #if JUCE_MAC
             XcodeProjectExporter::getValueTreeTypeNameMac(),
             XcodeProjectExporter::getValueTreeTypeNameiOS(),
            #elif JUCE_WINDOWS
             MSVCProjectExporterVC2019::getValueTreeTypeName(),
             MSVCProjectExporterVC2017::getValueTreeTypeName(),
             MSVCProjectExporterVC2015::getValueTreeTypeName(),
            #endif
             AndroidProjectExporter::getValueTreeTypeName()
        };

        for (auto& exporterIdentifier : types)
            if (project->getExporters().getChildWithName (exporterIdentifier).isValid())
                return true;
    }

    return false;
}

//==============================================================================
ProjectExporter::ProjectExporter (Project& p, const ValueTree& state)
    : settings (state),
      project (p),
      projectType (p.getProjectType()),
      projectName (p.getProjectNameString()),
      projectFolder (p.getProjectFolder()),
      targetLocationValue     (settings, Ids::targetFolder,        getUndoManager()),
      extraCompilerFlagsValue (settings, Ids::extraCompilerFlags,  getUndoManager()),
      extraLinkerFlagsValue   (settings, Ids::extraLinkerFlags,    getUndoManager()),
      externalLibrariesValue  (settings, Ids::externalLibraries,   getUndoManager()),
      userNotesValue          (settings, Ids::userNotes,           getUndoManager()),
      gnuExtensionsValue      (settings, Ids::enableGNUExtensions, getUndoManager()),
      bigIconValue            (settings, Ids::bigIcon,             getUndoManager()),
      smallIconValue          (settings, Ids::smallIcon,           getUndoManager()),
      extraPPDefsValue        (settings, Ids::extraDefs,           getUndoManager())
{
    projectCompilerFlagSchemesValue = project.getProjectValue (Ids::compilerFlagSchemes);
    projectCompilerFlagSchemesValue.addListener (this);
    updateCompilerFlagValues();
}

String ProjectExporter::getUniqueName() const
{
    auto targetLocationString = getTargetLocationString();
    auto defaultBuildsRootFolder = getDefaultBuildsRootFolder();

    auto typeInfos = getExporterTypeInfos();

    auto predicate = [targetLocationString, defaultBuildsRootFolder] (const ProjectExporter::ExporterTypeInfo& info)
    {
        return defaultBuildsRootFolder + info.targetFolder == targetLocationString;
    };

    auto iter = std::find_if (typeInfos.begin(), typeInfos.end(),
                              std::move (predicate));

    if (iter == typeInfos.end())
        return name + " - " + targetLocationString;

    return name;
}

File ProjectExporter::getTargetFolder() const
{
    return project.resolveFilename (getTargetLocationString());
}

build_tools::RelativePath ProjectExporter::rebaseFromProjectFolderToBuildTarget (const build_tools::RelativePath& path) const
{
    return path.rebased (project.getProjectFolder(), getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
}

bool ProjectExporter::shouldFileBeCompiledByDefault (const File& file) const
{
    return file.hasFileExtension (cOrCppFileExtensions)
        || file.hasFileExtension (asmFileExtensions);
}

void ProjectExporter::updateCompilerFlagValues()
{
    compilerFlagSchemesMap.clear();

    for (auto& scheme : project.getCompilerFlagSchemes())
        compilerFlagSchemesMap.set (scheme, { settings, scheme, getUndoManager() });
}

//==============================================================================
void ProjectExporter::createPropertyEditors (PropertyListBuilder& props)
{
    if (! isCLion())
    {
        props.add (new TextPropertyComponent (targetLocationValue, "Target Project Folder", 2048, false),
                   "The location of the folder in which the " + name + " project will be created. "
                   "This path can be absolute, but it's much more sensible to make it relative to the jucer project directory.");

        if ((shouldBuildTargetType (build_tools::ProjectType::Target::VSTPlugIn) && project.shouldBuildVST()) || (project.isVSTPluginHost() && supportsTargetType (build_tools::ProjectType::Target::VSTPlugIn)))
        {
            props.add (new FilePathPropertyComponent (vstLegacyPathValueWrapper.getWrappedValueWithDefault(), "VST (Legacy) SDK Folder", true,
                                                      getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                       "If you're building a VST plug-in or host, you can use this field to override the global VST (Legacy) SDK path with a project-specific path. "
                       "This can be an absolute path, or a path relative to the Projucer project file.");
        }

        if (shouldBuildTargetType (build_tools::ProjectType::Target::AAXPlugIn) && project.shouldBuildAAX())
        {
            props.add (new FilePathPropertyComponent (aaxPathValueWrapper.getWrappedValueWithDefault(), "AAX SDK Folder", true,
                                                      getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                       "If you're building an AAX plug-in, this must be the folder containing the AAX SDK. This can be an absolute path, or a path relative to the Projucer project file.");
        }

        if (shouldBuildTargetType (build_tools::ProjectType::Target::RTASPlugIn) && project.shouldBuildRTAS())
        {
            props.add (new FilePathPropertyComponent (rtasPathValueWrapper.getWrappedValueWithDefault(), "RTAS SDK Folder", true,
                                                      getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                       "If you're building an RTAS plug-in, this must be the folder containing the RTAS SDK. This can be an absolute path, or a path relative to the Projucer project file.");
        }

        props.add (new TextPropertyComponent (extraPPDefsValue, "Extra Preprocessor Definitions", 32768, true),
                   "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, "
                   "or new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

        props.add (new TextPropertyComponent (extraCompilerFlagsValue, "Extra Compiler Flags", 8192, true),
                   "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor definitions in the "
                   "form ${NAME_OF_DEFINITION}, which will be replaced with their values.");

        for (HashMap<String, ValueWithDefault>::Iterator i (compilerFlagSchemesMap); i.next();)
            props.add (new TextPropertyComponent (compilerFlagSchemesMap.getReference (i.getKey()), "Compiler Flags for " + i.getKey().quoted(), 8192, false),
                       "The exporter-specific compiler flags that will be added to files using this scheme.");

        props.add (new TextPropertyComponent (extraLinkerFlagsValue, "Extra Linker Flags", 8192, true),
                   "Extra command-line flags to be passed to the linker. You might want to use this for adding additional libraries. "
                   "This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");

        props.add (new TextPropertyComponent (externalLibrariesValue, "External Libraries to Link", 8192, true),
                   "Additional libraries to link (one per line). You should not add any platform specific decoration to these names. "
                   "This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");

        if (! isVisualStudio())
            props.add (new ChoicePropertyComponent (gnuExtensionsValue, "GNU Compiler Extensions"),
                       "Enabling this will use the GNU C++ language standard variant for compilation.");

        createIconProperties (props);
    }

    createExporterProperties (props);

    props.add (new TextPropertyComponent (userNotesValue, "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

void ProjectExporter::createIconProperties (PropertyListBuilder& props)
{
    OwnedArray<Project::Item> images;
    project.findAllImageItems (images);

    StringArray choices;
    Array<var> ids;

    choices.add ("<None>");
    ids.add (var());

    for (int i = 0; i < images.size(); ++i)
    {
        choices.add (images.getUnchecked(i)->getName());
        ids.add (images.getUnchecked(i)->getID());
    }

    props.add (new ChoicePropertyComponent (smallIconValue, "Icon (Small)", choices, ids),
               "Sets an icon to use for the executable.");

    props.add (new ChoicePropertyComponent (bigIconValue, "Icon (Large)", choices, ids),
               "Sets an icon to use for the executable.");
}

//==============================================================================
void ProjectExporter::addSettingsForProjectType (const build_tools::ProjectType& type)
{
    addVSTPathsIfPluginOrHost();

    if (type.isAudioPlugin())
        addCommonAudioPluginSettings();

    addPlatformSpecificSettingsForProjectType (type);
}

void ProjectExporter::addVSTPathsIfPluginOrHost()
{
    if (((shouldBuildTargetType (build_tools::ProjectType::Target::VSTPlugIn) && project.shouldBuildVST()) || project.isVSTPluginHost())
         || ((shouldBuildTargetType (build_tools::ProjectType::Target::VST3PlugIn) && project.shouldBuildVST3()) || project.isVST3PluginHost()))
    {
        addLegacyVSTFolderToPathIfSpecified();

        if (! project.isConfigFlagEnabled ("JUCE_CUSTOM_VST3_SDK"))
            addToExtraSearchPaths (getInternalVST3SDKPath(), 0);
    }
}

void ProjectExporter::addCommonAudioPluginSettings()
{
    if (shouldBuildTargetType (build_tools::ProjectType::Target::AAXPlugIn))
        addAAXFoldersToPath();

    // Note: RTAS paths are platform-dependent, impl -> addPlatformSpecificSettingsForProjectType
 }

void ProjectExporter::addLegacyVSTFolderToPathIfSpecified()
{
    auto vstFolder = getVSTLegacyPathString();

    if (vstFolder.isNotEmpty())
        addToExtraSearchPaths (build_tools::RelativePath (vstFolder, build_tools::RelativePath::projectFolder), 0);
}

build_tools::RelativePath ProjectExporter::getInternalVST3SDKPath()
{
    return getModuleFolderRelativeToProject ("juce_audio_processors")
                           .getChildFile ("format_types")
                           .getChildFile ("VST3_SDK");
}

void ProjectExporter::addAAXFoldersToPath()
{
    auto aaxFolder = getAAXPathString();

    if (aaxFolder.isNotEmpty())
    {
        build_tools::RelativePath aaxFolderPath (aaxFolder, build_tools::RelativePath::projectFolder);

        addToExtraSearchPaths (aaxFolderPath);
        addToExtraSearchPaths (aaxFolderPath.getChildFile ("Interfaces"));
        addToExtraSearchPaths (aaxFolderPath.getChildFile ("Interfaces").getChildFile ("ACF"));
    }
}

//==============================================================================
StringPairArray ProjectExporter::getAllPreprocessorDefs (const BuildConfiguration& config, const build_tools::ProjectType::Target::Type targetType) const
{
    auto defs = mergePreprocessorDefs (config.getAllPreprocessorDefs(),
                                       parsePreprocessorDefs (getExporterPreprocessorDefsString()));

    addDefaultPreprocessorDefs (defs);
    addTargetSpecificPreprocessorDefs (defs, targetType);

    if (! project.shouldUseAppConfig())
        defs = mergePreprocessorDefs (project.getAppConfigDefs(), defs);

    return defs;
}

StringPairArray ProjectExporter::getAllPreprocessorDefs() const
{
    auto defs = mergePreprocessorDefs (project.getPreprocessorDefs(),
                                       parsePreprocessorDefs (getExporterPreprocessorDefsString()));
    addDefaultPreprocessorDefs (defs);

    return defs;
}

void ProjectExporter::addTargetSpecificPreprocessorDefs (StringPairArray& defs, const build_tools::ProjectType::Target::Type targetType) const
{
    std::pair<String, build_tools::ProjectType::Target::Type> targetFlags[] = {
        {"JucePlugin_Build_VST",        build_tools::ProjectType::Target::VSTPlugIn},
        {"JucePlugin_Build_VST3",       build_tools::ProjectType::Target::VST3PlugIn},
        {"JucePlugin_Build_AU",         build_tools::ProjectType::Target::AudioUnitPlugIn},
        {"JucePlugin_Build_AUv3",       build_tools::ProjectType::Target::AudioUnitv3PlugIn},
        {"JucePlugin_Build_RTAS",       build_tools::ProjectType::Target::RTASPlugIn},
        {"JucePlugin_Build_AAX",        build_tools::ProjectType::Target::AAXPlugIn},
        {"JucePlugin_Build_Standalone", build_tools::ProjectType::Target::StandalonePlugIn},
        {"JucePlugin_Build_Unity",      build_tools::ProjectType::Target::UnityPlugIn}
    };

    if (targetType == build_tools::ProjectType::Target::SharedCodeTarget)
    {
        for (auto& flag : targetFlags)
            defs.set (flag.first, (shouldBuildTargetType (flag.second) ? "1" : "0"));

        defs.set ("JUCE_SHARED_CODE", "1");
    }
    else if (targetType != build_tools::ProjectType::Target::unspecified)
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
    return build_tools::replacePreprocessorDefs (getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified),
                                                 sourceString);
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
        itemGroups.add (Project::Item::createGroup (project, "JUCE Modules", "__modulesgroup__", true));
        modulesGroup = &(itemGroups.getReference (itemGroups.size() - 1));
    }

    return *modulesGroup;
}

//==============================================================================
static bool isWebBrowserComponentEnabled (Project& project)
{
    static String guiExtrasModule ("juce_gui_extra");

    return (project.getEnabledModules().isModuleEnabled (guiExtrasModule)
            && project.isConfigFlagEnabled ("JUCE_WEB_BROWSER", true));
}

static bool isCurlEnabled (Project& project)
{
    static String juceCoreModule ("juce_core");

    return (project.getEnabledModules().isModuleEnabled (juceCoreModule)
            && project.isConfigFlagEnabled ("JUCE_USE_CURL", true));
}

static bool isLoadCurlSymbolsLazilyEnabled (Project& project)
{
    static String juceCoreModule ("juce_core");

    return (project.getEnabledModules().isModuleEnabled (juceCoreModule)
            && project.isConfigFlagEnabled ("JUCE_LOAD_CURL_SYMBOLS_LAZILY", false));
}

StringArray ProjectExporter::getLinuxPackages (PackageDependencyType type) const
{
    auto packages = linuxPackages;

    // don't add libcurl if curl symbols are loaded at runtime
    if (isCurlEnabled (project) && ! isLoadCurlSymbolsLazilyEnabled (project))
        packages.add ("libcurl");

    if (isWebBrowserComponentEnabled (project) && type == PackageDependencyType::compile)
    {
        packages.add ("webkit2gtk-4.0");
        packages.add ("gtk+-x11-3.0");
    }

    packages.removeEmptyStrings();
    packages.removeDuplicates (false);

    return packages;
}

void ProjectExporter::addProjectPathToBuildPathList (StringArray& pathList,
                                                     const build_tools::RelativePath& pathFromProjectFolder,
                                                     int index) const
{
    auto localPath = build_tools::RelativePath (rebaseFromProjectFolderToBuildTarget (pathFromProjectFolder));

    auto path = isVisualStudio() ? localPath.toWindowsStyle() : localPath.toUnixStyle();

    if (! pathList.contains (path))
        pathList.insert (index, path);
}

void ProjectExporter::addToModuleLibPaths (const build_tools::RelativePath& pathFromProjectFolder)
{
    addProjectPathToBuildPathList (moduleLibSearchPaths, pathFromProjectFolder);
}

void ProjectExporter::addToExtraSearchPaths (const build_tools::RelativePath& pathFromProjectFolder, int index)
{
    addProjectPathToBuildPathList (extraSearchPaths, pathFromProjectFolder, index);
}

static var getStoredPathForModule (const String& id, const ProjectExporter& exp)
{
    return getAppSettings().getStoredPath (isJUCEModule (id) ? Ids::defaultJuceModulePath : Ids::defaultUserModulePath,
                                           exp.getTargetOSForExporter()).get();
}

ValueWithDefault ProjectExporter::getPathForModuleValue (const String& moduleID)
{
    auto* um = getUndoManager();

    auto paths = settings.getOrCreateChildWithName (Ids::MODULEPATHS, um);
    auto m = paths.getChildWithProperty (Ids::ID, moduleID);

    if (! m.isValid())
    {
        m = ValueTree (Ids::MODULEPATH);
        m.setProperty (Ids::ID, moduleID, um);
        paths.appendChild (m, um);
    }

    return { m, Ids::path, um, getStoredPathForModule (moduleID, *this) };
}

String ProjectExporter::getPathForModuleString (const String& moduleID) const
{
    auto exporterPath = settings.getChildWithName (Ids::MODULEPATHS)
                                .getChildWithProperty (Ids::ID, moduleID) [Ids::path].toString();

    if (exporterPath.isEmpty() || project.getEnabledModules().shouldUseGlobalPath (moduleID))
        return getStoredPathForModule (moduleID, *this);

    return exporterPath;
}

void ProjectExporter::removePathForModule (const String& moduleID)
{
    auto paths = settings.getChildWithName (Ids::MODULEPATHS);
    auto m = paths.getChildWithProperty (Ids::ID, moduleID);

    paths.removeChild (m, project.getUndoManagerFor (settings));
}

TargetOS::OS ProjectExporter::getTargetOSForExporter() const
{
    auto targetOS = TargetOS::unknown;

    if      (isWindows())                 targetOS = TargetOS::windows;
    else if (isOSX() || isiOS())          targetOS = TargetOS::osx;
    else if (isLinux())                   targetOS = TargetOS::linux;
    else if (isAndroid() || isCLion())    targetOS = TargetOS::getThisOS();

    return targetOS;
}

build_tools::RelativePath ProjectExporter::getModuleFolderRelativeToProject (const String& moduleID) const
{
    if (project.getEnabledModules().shouldCopyModuleFilesLocally (moduleID))
        return build_tools::RelativePath (project.getRelativePathForFile (project.getLocalModuleFolder (moduleID)),
                             build_tools::RelativePath::projectFolder);

    auto path = getPathForModuleString (moduleID);

    if (path.isEmpty())
        return getLegacyModulePath (moduleID).getChildFile (moduleID);

    return build_tools::RelativePath (path, build_tools::RelativePath::projectFolder).getChildFile (moduleID);
}

String ProjectExporter::getLegacyModulePath() const
{
    return getSettingString ("juceFolder");
}

build_tools::RelativePath ProjectExporter::getLegacyModulePath (const String& moduleID) const
{
    if (project.getEnabledModules().shouldCopyModuleFilesLocally (moduleID))
        return build_tools::RelativePath (project.getRelativePathForFile (project.getGeneratedCodeFolder()
                                                                           .getChildFile ("modules")
                                                                           .getChildFile (moduleID)), build_tools::RelativePath::projectFolder);

    auto oldJucePath = getLegacyModulePath();

    if (oldJucePath.isEmpty())
        return build_tools::RelativePath();

    build_tools::RelativePath p (oldJucePath, build_tools::RelativePath::projectFolder);
    if (p.getFileName() != "modules")
        p = p.getChildFile ("modules");

    return p.getChildFile (moduleID);
}

void ProjectExporter::updateOldModulePaths()
{
    auto oldPath = getLegacyModulePath();

    if (oldPath.isNotEmpty())
    {
        for (int i = project.getEnabledModules().getNumModules(); --i >= 0;)
        {
            auto modID = project.getEnabledModules().getModuleID (i);
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
            for (int i = project.getEnabledModules().getNumModules(); --i >= 0;)
            {
                auto modID = project.getEnabledModules().getModuleID (i);
                getPathForModuleValue (modID) = exporter->getPathForModuleValue (modID);
            }

            return;
        }
    }

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (exporter->canLaunchProject())
        {
            for (int i = project.getEnabledModules().getNumModules(); --i >= 0;)
            {
                auto modID = project.getEnabledModules().getModuleID (i);
                getPathForModuleValue (modID) = exporter->getPathForModuleValue (modID);
            }

            return;
        }
    }

    for (int i = project.getEnabledModules().getNumModules(); --i >= 0;)
    {
        auto modID = project.getEnabledModules().getModuleID (i);
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
    auto configs = getConfigurations();
    for (int i = configs.getNumChildren(); --i >= 0;)
        if (configs.getChild(i) [Ids::name].toString() == nameToFind)
            return true;

    return false;
}

String ProjectExporter::getUniqueConfigName (String nm) const
{
    auto nameRoot = nm;
    while (CharacterFunctions::isDigit (nameRoot.getLastCharacter()))
        nameRoot = nameRoot.dropLastCharacters (1);

    nameRoot = nameRoot.trim();

    int suffix = 2;
    while (hasConfigurationNamed (name))
        nm = nameRoot + " " + String (suffix++);

    return nm;
}

void ProjectExporter::addNewConfigurationFromExisting (const BuildConfiguration& configToCopy)
{
    auto configs = getConfigurations();

    if (! configs.isValid())
    {
        settings.addChild (ValueTree (Ids::CONFIGURATIONS), 0, project.getUndoManagerFor (settings));
        configs = getConfigurations();
    }

    ValueTree newConfig (Ids::CONFIGURATION);
    newConfig = configToCopy.config.createCopy();

    newConfig.setProperty (Ids::name, configToCopy.getName(), nullptr);

    configs.appendChild (newConfig, project.getUndoManagerFor (configs));
}

void ProjectExporter::addNewConfiguration (bool isDebugConfig)
{
    auto configs = getConfigurations();

    if (! configs.isValid())
    {
        settings.addChild (ValueTree (Ids::CONFIGURATIONS), 0, project.getUndoManagerFor (settings));
        configs = getConfigurations();
    }

    ValueTree newConfig (Ids::CONFIGURATION);
    newConfig.setProperty (Ids::isDebug, isDebugConfig, project.getUndoManagerFor (settings));

    configs.appendChild (newConfig, project.getUndoManagerFor (settings));
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
        auto isDebug = i == 0;

        addNewConfiguration (isDebug);
        BuildConfiguration::Ptr config (getConfiguration (i));
        config->getValue (Ids::name) = (isDebug ? "Debug" : "Release");
    }
}

std::unique_ptr<Drawable> ProjectExporter::getBigIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::bigIcon]).loadAsImageFile();
}

std::unique_ptr<Drawable> ProjectExporter::getSmallIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::smallIcon]).loadAsImageFile();
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
   : config (configNode), project (p), exporter (e),
     isDebugValue                  (config, Ids::isDebug,                  getUndoManager(), getValue (Ids::isDebug)),
     configNameValue               (config, Ids::name,                     getUndoManager(), "Build Configuration"),
     targetNameValue               (config, Ids::targetName,               getUndoManager(), project.getProjectFilenameRootString()),
     targetBinaryPathValue         (config, Ids::binaryPath,               getUndoManager()),
     recommendedWarningsValue      (config, Ids::recommendedWarnings,      getUndoManager()),
     optimisationLevelValue        (config, Ids::optimisation,             getUndoManager()),
     linkTimeOptimisationValue     (config, Ids::linkTimeOptimisation,     getUndoManager(), ! isDebug()),
     ppDefinesValue                (config, Ids::defines,                  getUndoManager()),
     headerSearchPathValue         (config, Ids::headerPath,               getUndoManager()),
     librarySearchPathValue        (config, Ids::libraryPath,              getUndoManager()),
     userNotesValue                (config, Ids::userNotes,                getUndoManager()),
     usePrecompiledHeaderFileValue (config, Ids::usePrecompiledHeaderFile, getUndoManager(), false),
     precompiledHeaderFileValue    (config, Ids::precompiledHeaderFile,    getUndoManager())
{
    recommendedCompilerWarningFlags["LLVM"] = { "-Wall", "-Wshadow-all", "-Wshorten-64-to-32", "-Wstrict-aliasing", "-Wuninitialized", "-Wunused-parameter",
        "-Wconversion", "-Wsign-compare", "-Wint-conversion", "-Wconditional-uninitialized", "-Woverloaded-virtual",
        "-Wreorder", "-Wconstant-conversion", "-Wsign-conversion", "-Wunused-private-field", "-Wbool-conversion",
        "-Wextra-semi", "-Wunreachable-code", "-Wzero-as-null-pointer-constant", "-Wcast-align",
        "-Winconsistent-missing-destructor-override", "-Wshift-sign-overflow", "-Wnullable-to-nonnull-conversion",
        "-Wno-missing-field-initializers", "-Wno-ignored-qualifiers",
        "-Wswitch-enum"
    };
    recommendedCompilerWarningFlags["GCC"] = { "-Wall", "-Wextra", "-Wstrict-aliasing", "-Wuninitialized", "-Wunused-parameter", "-Wsign-compare",
        "-Woverloaded-virtual", "-Wreorder", "-Wsign-conversion", "-Wunreachable-code",
        "-Wzero-as-null-pointer-constant", "-Wcast-align", "-Wno-implicit-fallthrough",
        "-Wno-maybe-uninitialized", "-Wno-missing-field-initializers", "-Wno-ignored-qualifiers",
        "-Wswitch-enum", "-Wredundant-decls"
    };
    recommendedCompilerWarningFlags["GCC-7"] = recommendedCompilerWarningFlags["GCC"];
    recommendedCompilerWarningFlags["GCC-7"].add ("-Wno-strict-overflow");
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
    props.add (new ChoicePropertyComponent (optimisationLevelValue, "Optimisation",
                                            { "-O0 (no optimisation)", "-Os (minimise code size)", "-O1 (fast)", "-O2 (faster)",
                                              "-O3 (fastest with safe optimisations)", "-Ofast (uses aggressive optimisations)" },
                                            { gccO0, gccOs, gccO1, gccO2, gccO3, gccOfast }),
               "The optimisation level for this configuration");
}

void ProjectExporter::BuildConfiguration::addRecommendedLinuxCompilerWarningsProperty (PropertyListBuilder& props)
{
    props.add (new ChoicePropertyComponent (recommendedWarningsValue, "Add Recommended Compiler Warning Flags",
                                            { "GCC", "GCC 7 and below", "LLVM", "Disabled" },
                                            { "GCC", "GCC-7", "LLVM", "" }),
               "Enable this to add a set of recommended compiler warning flags.");
    recommendedWarningsValue.setDefault ("");
}

void ProjectExporter::BuildConfiguration::addRecommendedLLVMCompilerWarningsProperty (PropertyListBuilder& props)
{
    props.add (new ChoicePropertyComponent (recommendedWarningsValue, "Add Recommended Compiler Warning Flags",
                                            { "Enabled", "Disabled" },
                                            { "LLVM", "" }),
               "Enable this to add a set of recommended compiler warning flags.");
    recommendedWarningsValue.setDefault ("");
}

StringArray ProjectExporter::BuildConfiguration::getRecommendedCompilerWarningFlags() const
{
    auto label = recommendedWarningsValue.get().toString();
    auto it = recommendedCompilerWarningFlags.find (label);

    if (it != recommendedCompilerWarningFlags.end())
        return it->second;

    return {};
}

void ProjectExporter::BuildConfiguration::createPropertyEditors (PropertyListBuilder& props)
{
    if (exporter.supportsUserDefinedConfigurations())
        props.add (new TextPropertyComponent (configNameValue, "Name", 96, false),
                   "The name of this configuration.");

    props.add (new ChoicePropertyComponent (isDebugValue, "Debug Mode"),
               "If enabled, this means that the configuration should be built with debug symbols.");

    props.add (new TextPropertyComponent (targetNameValue, "Binary Name", 256, false),
               "The filename to use for the destination binary executable file. If you don't add a suffix to this name, "
               "a suitable platform-specific suffix will be added automatically.");

    props.add (new TextPropertyComponent (targetBinaryPathValue, "Binary Location", 1024, false),
               "The folder in which the finished binary should be placed. Leave this blank to cause the binary to be placed "
               "in its default location in the build folder.");

    props.addSearchPathProperty (headerSearchPathValue, "Header Search Paths", "Extra header search paths.");
    props.addSearchPathProperty (librarySearchPathValue, "Extra Library Search Paths", "Extra library search paths.");

    props.add (new TextPropertyComponent (ppDefinesValue, "Preprocessor Definitions", 32768, true),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, or "
               "new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new ChoicePropertyComponent (linkTimeOptimisationValue, "Link-Time Optimisation"),
               "Enable this to perform link-time code optimisation. This is recommended for release builds.");

    if (exporter.supportsPrecompiledHeaders())
    {
        props.add (new ChoicePropertyComponent (usePrecompiledHeaderFileValue, "Use Precompiled Header"),
                   "Enable this to turn on precompiled header support for this configuration. Use the setting "
                   "below to specify the header file to use.");

        auto quotedHeaderFileName = (getPrecompiledHeaderFilename() + ".h").quoted();

        props.add (new FilePathPropertyComponentWithEnablement (precompiledHeaderFileValue, usePrecompiledHeaderFileValue,
                                                                "Precompiled Header File", false, true, "*", project.getProjectFolder()),
                   "Specify an input header file that will be used to generate a file named " + quotedHeaderFileName + " which is used to generate the "
                   "PCH file artefact for this exporter configuration. This file can be an absolute path, or relative to the jucer project folder. "
                   "The " + quotedHeaderFileName + " file will be force included to all source files unless the \"Skip PCH\" setting has been enabled. "
                   "The generated header will be written on project save and placed in the target folder for this exporter.");
    }

    createConfigProperties (props);

    props.add (new TextPropertyComponent (userNotesValue, "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

StringPairArray ProjectExporter::BuildConfiguration::getAllPreprocessorDefs() const
{
    return mergePreprocessorDefs (project.getPreprocessorDefs(),
                                  parsePreprocessorDefs (getBuildConfigPreprocessorDefsString()));
}

StringPairArray ProjectExporter::BuildConfiguration::getUniquePreprocessorDefs() const
{
    auto perConfigurationDefs = parsePreprocessorDefs (getBuildConfigPreprocessorDefsString());
    auto globalDefs = project.getPreprocessorDefs();

    for (int i = 0; i < globalDefs.size(); ++i)
    {
        auto globalKey = globalDefs.getAllKeys()[i];

        int idx = perConfigurationDefs.getAllKeys().indexOf (globalKey);
        if (idx >= 0)
        {
            auto globalValue = globalDefs.getAllValues()[i];

            if (globalValue == perConfigurationDefs.getAllValues()[idx])
                perConfigurationDefs.remove (idx);
        }
    }

    return perConfigurationDefs;
}

StringArray ProjectExporter::BuildConfiguration::getHeaderSearchPaths() const
{
    return getSearchPathsFromString (getHeaderSearchPathString() + ';' + project.getHeaderSearchPathsString());
}

StringArray ProjectExporter::BuildConfiguration::getLibrarySearchPaths() const
{
    auto separator = exporter.isVisualStudio() ? "\\" : "/";
    auto s = getSearchPathsFromString (getLibrarySearchPathString());

    for (auto path : exporter.moduleLibSearchPaths)
        s.add (path + separator + getModuleLibraryArchName());

    return s;
}

String ProjectExporter::BuildConfiguration::getPrecompiledHeaderFileContent() const
{
    if (shouldUsePrecompiledHeaderFile())
    {
        auto f = project.getProjectFolder().getChildFile (precompiledHeaderFileValue.get().toString());

        if (f.existsAsFile() && f.hasFileExtension (headerFileExtensions))
        {
            MemoryOutputStream content;
            content.setNewLineString (exporter.getNewLineString());

            writeAutoGenWarningComment (content);

            content << "*/" << newLine << newLine
                    << "#ifndef " << getSkipPrecompiledHeaderDefine() << newLine << newLine
                    << f.loadFileAsString() << newLine
                    << "#endif" << newLine;

            return content.toString();
        }
    }

    return {};
}

String ProjectExporter::getExternalLibraryFlags (const BuildConfiguration& config) const
{
    auto libraries = StringArray::fromTokens (getExternalLibrariesString(), ";\n", "\"'");
    libraries.removeEmptyStrings (true);

    if (libraries.size() != 0)
        return replacePreprocessorTokens (config, "-l" + libraries.joinIntoString (" -l")).trim();

    return {};
}
