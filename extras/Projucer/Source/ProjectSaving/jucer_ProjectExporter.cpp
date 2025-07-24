/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"

#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_MSVC.h"
#include "jucer_ProjectExport_Xcode.h"
#include "jucer_ProjectExport_Android.h"

#include "../Utility/UI/PropertyComponents/jucer_FilePathPropertyComponent.h"

//==============================================================================
static auto createIcon (const void* iconData, size_t iconDataSize)
{
    Image image (Image::ARGB, 200, 200, true);
    Graphics g (image);

    std::unique_ptr<Drawable> svgDrawable (Drawable::createFromImageData (iconData, iconDataSize));
    svgDrawable->drawWithin (g, image.getBounds().toFloat(), RectanglePlacement::fillDestination, 1.0f);

    return image;
}

std::vector<PackageDependency> makePackageDependencies (const StringArray& dependencies)
{
    std::vector<PackageDependency> result;
    result.reserve ((size_t) dependencies.size());
    std::transform (dependencies.begin(),
                    dependencies.end(),
                    std::back_inserter (result),
                    [] (auto& d) { return PackageDependency { d }; });
    return result;
}

template <typename Exporter>
static ProjectExporter::ExporterTypeInfo createExporterTypeInfo (const void* iconData, size_t iconDataSize)
{
    return { Exporter::getValueTreeTypeName(),
             Exporter::getDisplayName(),
             Exporter::getTargetFolderName(),
             createIcon (iconData, iconDataSize) };
}

std::vector<ProjectExporter::ExporterTypeInfo> ProjectExporter::getExporterTypeInfos()
{
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

        createExporterTypeInfo<MSVCProjectExporterVC2022> (export_visualStudio_svg, export_visualStudio_svgSize),
        createExporterTypeInfo<MSVCProjectExporterVC2019> (export_visualStudio_svg, export_visualStudio_svgSize),

        createExporterTypeInfo<MakefileProjectExporter> (export_linux_svg, export_linux_svgSize),

        createExporterTypeInfo<AndroidProjectExporter> (export_android_svg, export_android_svgSize),
    };

    return infos;
}

ProjectExporter::ExporterTypeInfo ProjectExporter::getTypeInfoForExporter (const Identifier& exporterIdentifier)
{
    auto typeInfos = getExporterTypeInfos();

    auto iter = std::find_if (typeInfos.begin(), typeInfos.end(),
                              [exporterIdentifier] (const ProjectExporter::ExporterTypeInfo& info) { return info.identifier == exporterIdentifier; });

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
     return ProjectExporter::getTypeInfoForExporter (MSVCProjectExporterVC2022::getValueTreeTypeName());
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

template <typename T> struct Tag {};

static std::unique_ptr<ProjectExporter> tryCreatingExporter (Project&, const ValueTree&) { return nullptr; }

template <typename Exporter, typename... Exporters>
static std::unique_ptr<ProjectExporter> tryCreatingExporter (Project& project,
                                                             const ValueTree& settings,
                                                             Tag<Exporter>,
                                                             Tag<Exporters>... exporters)
{
    if (auto* exporter = Exporter::createForSettings (project, settings))
        return rawToUniquePtr (exporter);

    return tryCreatingExporter (project, settings, exporters...);
}

std::unique_ptr<ProjectExporter> ProjectExporter::createExporterFromSettings (Project& project, const ValueTree& settings)
{
    return tryCreatingExporter (project,
                                settings,
                                Tag<XcodeProjectExporter>{},
                                Tag<MSVCProjectExporterVC2022>{},
                                Tag<MSVCProjectExporterVC2019>{},
                                Tag<MakefileProjectExporter>{},
                                Tag<AndroidProjectExporter>{});
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
             MSVCProjectExporterVC2022::getValueTreeTypeName(),
             MSVCProjectExporterVC2019::getValueTreeTypeName(),
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

    if (std::none_of (typeInfos.begin(), typeInfos.end(), std::move (predicate)))
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

build_tools::RelativePath ProjectExporter::rebaseFromBuildTargetToProjectFolder (const build_tools::RelativePath& path) const
{
    jassert (path.getRoot() == build_tools::RelativePath::buildTargetFolder);
    return path.rebased (getTargetFolder(), project.getProjectFolder(), build_tools::RelativePath::projectFolder);
}

File ProjectExporter::resolveRelativePath (const build_tools::RelativePath& path) const
{
    if (path.isAbsolute())
        return path.toUnixStyle();

    switch (path.getRoot())
    {
        case build_tools::RelativePath::buildTargetFolder:
            return getTargetFolder().getChildFile (path.toUnixStyle());

        case build_tools::RelativePath::projectFolder:
            return project.getProjectFolder().getChildFile (path.toUnixStyle());

        case build_tools::RelativePath::unknown:
            jassertfalse;
    }

    return path.toUnixStyle();
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
    {
        compilerFlagSchemesMap.emplace (std::piecewise_construct,
                                        std::forward_as_tuple (scheme),
                                        std::forward_as_tuple (settings, scheme, getUndoManager()));
    }
}

//==============================================================================
void ProjectExporter::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (targetLocationValue, "Target Project Folder", 2048, false),
               "The location of the folder in which the " + name + " project will be created. "
               "This path can be absolute, but it's much more sensible to make it relative to the jucer project directory.");

    if ((shouldBuildTargetType (build_tools::ProjectType::Target::VSTPlugIn) && project.shouldBuildVST()) || (project.isVSTPluginHost() && supportsTargetType (build_tools::ProjectType::Target::VSTPlugIn)))
    {
        props.add (new FilePathPropertyComponent (vstLegacyPathValueWrapper.getWrappedValueTreePropertyWithDefault(), "VST (Legacy) SDK Folder", true,
                                                  getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                   "If you're building a VST plug-in or host, you can use this field to override the global VST (Legacy) SDK path with a project-specific path. "
                   "This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (shouldBuildTargetType (build_tools::ProjectType::Target::AAXPlugIn) && project.shouldBuildAAX())
    {
        props.add (new FilePathPropertyComponent (aaxPathValueWrapper.getWrappedValueTreePropertyWithDefault(), "AAX SDK Folder", true,
                                                  getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                   "If you're building an AAX plug-in, this must be the folder containing the AAX SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (project.shouldEnableARA() || project.isARAPluginHost())
    {
        props.add (new FilePathPropertyComponent (araPathValueWrapper.getWrappedValueTreePropertyWithDefault(), "ARA SDK Folder", true,
                                                  getTargetOSForExporter() == TargetOS::getThisOS(), "*", project.getProjectFolder()),
                   "If you're building an ARA enabled plug-in, this must be the folder containing the ARA SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    props.add (new TextPropertyComponent (extraPPDefsValue, "Extra Preprocessor Definitions", 32768, true),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, "
               "or new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new TextPropertyComponent (extraCompilerFlagsValue, "Extra Compiler Flags", 8192, true),
               "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor definitions in the "
               "form ${NAME_OF_DEFINITION}, which will be replaced with their values.");

    for (const auto& [key, property] : compilerFlagSchemesMap)
    {
        props.add (new TextPropertyComponent (property, "Compiler Flags for " + key.quoted(), 8192, false),
                   "The exporter-specific compiler flags that will be added to files using this scheme.");
    }

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

    for (const auto* imageItem : images)
    {
        choices.add (imageItem->getName());
        ids.add (imageItem->getID());
    }

    props.add (new ChoicePropertyComponent (smallIconValue, "Icon (Small)", choices, ids),
               "Sets an icon to use for the executable.");

    props.add (new ChoicePropertyComponent (bigIconValue, "Icon (Large)", choices, ids),
               "Sets an icon to use for the executable.");
}

//==============================================================================
void ProjectExporter::addSettingsForProjectType (const build_tools::ProjectType& type)
{
    addExtraIncludePathsIfPluginOrHost();

    addARAPathsIfPluginOrHost();

    if (type.isAudioPlugin())
        addCommonAudioPluginSettings();

    addPlatformSpecificSettingsForProjectType (type);
}

void ProjectExporter::addExtraIncludePathsIfPluginOrHost()
{
    using Target = build_tools::ProjectType::Target;

    if (((shouldBuildTargetType (Target::VSTPlugIn) && project.shouldBuildVST()) || project.isVSTPluginHost())
         || ((shouldBuildTargetType (Target::VST3PlugIn) && project.shouldBuildVST3()) || project.isVST3PluginHost()))
    {
        addLegacyVSTFolderToPathIfSpecified();

        if (! project.isConfigFlagEnabled ("JUCE_CUSTOM_VST3_SDK"))
            addToExtraSearchPaths (getInternalVST3SDKPath(), 0);
    }

    const auto lv2BasePath = getModuleFolderRelativeToProject ("juce_audio_processors").getChildFile ("format_types")
                                                                                       .getChildFile ("LV2_SDK");

    if ((shouldBuildTargetType (Target::LV2PlugIn) && project.shouldBuildLV2()) || project.isLV2PluginHost())
    {
        const std::vector<const char*> paths[] { { "" },
                                                 { "lv2" },
                                                 { "serd" },
                                                 { "sord" },
                                                 { "sord", "src" },
                                                 { "sratom" },
                                                 { "lilv" },
                                                 { "lilv", "src" } };

        for (const auto& components : paths)
        {
            const auto appendComponent = [] (const build_tools::RelativePath& f, const char* component)
            {
                return f.getChildFile (component);
            };

            const auto includePath = std::accumulate (components.begin(),
                                                      components.end(),
                                                      lv2BasePath,
                                                      appendComponent);

            addToExtraSearchPaths (includePath, 0);
        }
    }
}

void ProjectExporter::addARAPathsIfPluginOrHost()
{
    if (project.shouldEnableARA() || project.isARAPluginHost())
        addARAFoldersToPath();
}

void ProjectExporter::addCommonAudioPluginSettings()
{
    if (shouldBuildTargetType (build_tools::ProjectType::Target::AAXPlugIn))
        addAAXFoldersToPath();
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
    const auto aaxFolder = getAAXPathRelative();

    addToExtraSearchPaths (aaxFolder);
    addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces"));
    addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces").getChildFile ("ACF"));
}

void ProjectExporter::addARAFoldersToPath()
{
    const auto araFolder = getARAPathString();

    if (araFolder.isNotEmpty())
        addToExtraSearchPaths (build_tools::RelativePath (araFolder, build_tools::RelativePath::projectFolder));
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
    using Target = build_tools::ProjectType::Target::Type;
    const std::pair<const char*, Target> targetFlags[] { { "JucePlugin_Build_VST",        Target::VSTPlugIn },
                                                         { "JucePlugin_Build_VST3",       Target::VST3PlugIn },
                                                         { "JucePlugin_Build_AU",         Target::AudioUnitPlugIn },
                                                         { "JucePlugin_Build_AUv3",       Target::AudioUnitv3PlugIn },
                                                         { "JucePlugin_Build_AAX",        Target::AAXPlugIn },
                                                         { "JucePlugin_Build_Standalone", Target::StandalonePlugIn },
                                                         { "JucePlugin_Build_Unity",      Target::UnityPlugIn },
                                                         { "JucePlugin_Build_LV2",        Target::LV2PlugIn } };

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
    if (project.shouldEnableARA())
    {
        defs.set ("JucePlugin_Enable_ARA", "1");
    }

    linuxSubprocessHelperProperties.setCompileDefinitionIfNecessary (defs);
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

String ProjectExporter::getCompilerFlagsForFileCompilerFlagScheme (StringRef schemeName) const
{
    if (const auto iter = compilerFlagSchemesMap.find (schemeName); iter != compilerFlagSchemesMap.cend())
        return iter->second.get().toString();

    return {};
}

String ProjectExporter::getCompilerFlagsForProjectItem (const Project::Item& item) const
{
    return getCompilerFlagsForFileCompilerFlagScheme (item.getCompilerFlagSchemeString());
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
static bool isWebBrowserComponentEnabled (const Project& project)
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

std::vector<PackageDependency> ProjectExporter::getLinuxPackages (PackageDependencyType type) const
{
    auto packages = linuxPackages;
    std::vector<PackageDependency> dependencies;

    // don't add libcurl if curl symbols are loaded at runtime
    if (isCurlEnabled (project) && ! isLoadCurlSymbolsLazilyEnabled (project))
        packages.add ("libcurl");

    if (isWebBrowserComponentEnabled (project) && type == PackageDependencyType::compile)
    {
        packages.add ("gtk+-x11-3.0");
        dependencies.push_back (PackageDependency { "webkit2gtk-4.1", "webkit2gtk-4.0" });
    }

    packages.removeEmptyStrings();
    packages.removeDuplicates (false);

    const auto simpleDependencies = makePackageDependencies (packages);
    dependencies.insert (dependencies.end(), simpleDependencies.begin(), simpleDependencies.end());

    return dependencies;
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
    jassert (pathFromProjectFolder.getRoot() == build_tools::RelativePath::projectFolder);
    addProjectPathToBuildPathList (extraSearchPaths, pathFromProjectFolder, index);
}

static var getStoredPathForModule (const String& id, const ProjectExporter& exp)
{
    return getAppSettings().getStoredPath (isJUCEModule (id) ? Ids::defaultJuceModulePath : Ids::defaultUserModulePath,
                                           exp.getTargetOSForExporter()).get();
}

ValueTreePropertyWithDefault ProjectExporter::getPathForModuleValue (const String& moduleID)
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
    else if (isAndroid())                 targetOS = TargetOS::getThisOS();

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

static bool areSameExporters (const ProjectExporter& p1, const ProjectExporter& p2)
{
    return p1.getExporterIdentifier() == p2.getExporterIdentifier();
}

static bool areCompatibleExporters (const ProjectExporter& p1, const ProjectExporter& p2)
{
    return (p1.isVisualStudio() && p2.isVisualStudio())
        || (p1.isXcode() && p2.isXcode())
        || (p1.isMakefile() && p2.isMakefile())
        || (p1.isAndroidStudio() && p2.isAndroidStudio());
}

void ProjectExporter::createDefaultModulePaths()
{
    auto exporterToCopy = [this]() -> std::unique_ptr<ProjectExporter>
    {
        std::vector<std::unique_ptr<ProjectExporter>> exporters;

        for (Project::ExporterIterator exporter (project); exporter.next();)
            exporters.push_back (std::move (exporter.exporter));

        auto getIf = [&exporters] (auto predicate)
        {
            auto iter = std::find_if (exporters.begin(), exporters.end(), predicate);
            return iter != exporters.end() ? std::move (*iter) : nullptr;
        };

        if (auto exporter = getIf ([this] (auto& x) { return areSameExporters (*this, *x); }))
            return exporter;

        if (auto exporter = getIf ([this] (auto& x) { return areCompatibleExporters (*this, *x); }))
            return exporter;

        if (auto exporter = getIf ([] (auto& x) { return x->canLaunchProject(); }))
            return exporter;

        return {};
    }();

    for (const auto& modID : project.getEnabledModules().getAllModules())
        getPathForModuleValue (modID) = (exporterToCopy != nullptr ? exporterToCopy->getPathForModuleString (modID) : "../../juce");
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

build_tools::Icons ProjectExporter::getIcons() const
{
    const MessageManagerLock mml (ThreadPoolJob::getCurrentThreadPoolJob());

    if (! mml.lockWasGained())
        return {};

    const auto getFile = [this] (auto id)
    {
        return project.getMainGroup().findItemWithID (settings[id]).getFile();
    };

    return build_tools::Icons::fromFilesSmallAndBig (getFile (Ids::smallIcon),
                                                     getFile (Ids::bigIcon));
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
     precompiledHeaderFileValue    (config, Ids::precompiledHeaderFile,    getUndoManager()),
     configCompilerFlagsValue      (config, Ids::extraCompilerFlags,       getUndoManager()),
     configLinkerFlagsValue        (config, Ids::extraLinkerFlags,         getUndoManager())
{
    auto& llvmFlags = recommendedCompilerWarningFlags[CompilerNames::llvm] = BuildConfiguration::CompilerWarningFlags::getRecommendedForGCCAndLLVM();

    llvmFlags.common.addArray ({ "-Wshadow-all",
                                 "-Wshorten-64-to-32",
                                 "-Wconversion",
                                 "-Wint-conversion",
                                 "-Wconditional-uninitialized",
                                 "-Wconstant-conversion",
                                 "-Wbool-conversion",
                                 "-Wextra-semi",
                                 "-Wshift-sign-overflow",
                                 "-Wmissing-prototypes",
                                 "-Wnullable-to-nonnull-conversion",
                                 "-Wpedantic",
                                 "-Wdeprecated" });

    llvmFlags.cpp.addArray ({ "-Wunused-private-field",
                              "-Winconsistent-missing-destructor-override" });

    llvmFlags.objc.addArray ({ "-Wunguarded-availability",
                               "-Wunguarded-availability-new" });

    auto& gccFlags = recommendedCompilerWarningFlags[CompilerNames::gcc] = BuildConfiguration::CompilerWarningFlags::getRecommendedForGCCAndLLVM();
    gccFlags.common.addArray ({ "-Wextra",
                                "-Wsign-compare",
                                "-Wno-implicit-fallthrough",
                                "-Wno-maybe-uninitialized",
                                "-Wredundant-decls",
                                "-Wno-strict-overflow",
                                "-Wno-multichar",
                                "-Wshadow" });
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
    recommendedWarningsValue.setDefault ("");

    props.add (new ChoicePropertyComponent (recommendedWarningsValue, "Add Recommended Compiler Warning Flags",
                                            { CompilerNames::gcc, CompilerNames::llvm, "Disabled" },
                                            { CompilerNames::gcc, CompilerNames::llvm, "" }),
               "Enable this to add a set of recommended compiler warning flags.");
}

void ProjectExporter::BuildConfiguration::addRecommendedLLVMCompilerWarningsProperty (PropertyListBuilder& props)
{
    recommendedWarningsValue.setDefault ("");

    props.add (new ChoicePropertyComponent (recommendedWarningsValue, "Add Recommended Compiler Warning Flags",
                                            { "Enabled",           "Disabled" },
                                            { CompilerNames::llvm, "" }),
               "Enable this to add a set of recommended compiler warning flags.");
}

ProjectExporter::BuildConfiguration::CompilerWarningFlags ProjectExporter::BuildConfiguration::getRecommendedCompilerWarningFlags() const
{
    auto label = recommendedWarningsValue.get().toString();

    if (label == "GCC-7")
        label = CompilerNames::gcc;

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

    props.add (new TextPropertyComponent (configCompilerFlagsValue, "Configuration-specific Compiler Flags", 8192, true),
               "Compiler flags that are only to be used in this configuration.");

    props.add (new TextPropertyComponent (configLinkerFlagsValue, "Configuration-specific Linker Flags", 8192, true),
               "Linker flags that are only to be used in this configuration.");

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

StringArray ProjectExporter::BuildConfiguration::getHeaderSearchPaths() const
{
    return getSearchPathsFromString (getHeaderSearchPathString() + ';' + project.getHeaderSearchPathsString());
}

StringArray ProjectExporter::BuildConfiguration::getLibrarySearchPaths() const
{
    auto separator = exporter.isVisualStudio() ? "\\" : "/";
    auto s = getSearchPathsFromString (getLibrarySearchPathString());

    for (auto path : exporter.moduleLibSearchPaths)
    {
        if (exporter.isXcode())
            s.add (path);

        s.add (path + separator + getModuleLibraryArchName());
    }

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

//==============================================================================
LinuxSubprocessHelperProperties::LinuxSubprocessHelperProperties (ProjectExporter& projectExporter)
    : owner (projectExporter)
{}

bool LinuxSubprocessHelperProperties::shouldUseLinuxSubprocessHelper() const
{
    const auto& project = owner.getProject();
    const auto& projectType = project.getProjectType();

    return    owner.isLinux()
           && isWebBrowserComponentEnabled (project)
           && ! (projectType.isCommandLineApp())
           && ! (projectType.isGUIApplication());
}

void LinuxSubprocessHelperProperties::deployLinuxSubprocessHelperSourceFilesIfNecessary() const
{
    if (shouldUseLinuxSubprocessHelper())
    {
        const auto deployHelperSourceFile = [] (auto& sourcePath, auto& contents)
        {
            if (! sourcePath.isRoot() && ! sourcePath.getParentDirectory().exists())
            {
                sourcePath.getParentDirectory().createDirectory();
            }

            build_tools::overwriteFileIfDifferentOrThrow (sourcePath, contents);
        };

        const std::pair<File, const char*> sources[]
        {
            { owner.resolveRelativePath (getSimpleBinaryBuilderSource()),   BinaryData::juce_SimpleBinaryBuilder_cpp   },
            { owner.resolveRelativePath (getLinuxSubprocessHelperSource()), BinaryData::juce_LinuxSubprocessHelper_cpp }
        };

        for (const auto& [path, source] : sources)
        {
            deployHelperSourceFile (path, source);
        }
    }
}

build_tools::RelativePath LinuxSubprocessHelperProperties::getLinuxSubprocessHelperSource() const
{
    return build_tools::RelativePath { "make_helpers", build_tools::RelativePath::buildTargetFolder }
        .getChildFile ("juce_LinuxSubprocessHelper.cpp");
}

void LinuxSubprocessHelperProperties::setCompileDefinitionIfNecessary (StringPairArray& defs) const
{
    if (shouldUseLinuxSubprocessHelper())
        defs.set (useLinuxSubprocessHelperCompileDefinition, "1");
}

build_tools::RelativePath LinuxSubprocessHelperProperties::getSimpleBinaryBuilderSource() const
{
    return build_tools::RelativePath { "make_helpers", build_tools::RelativePath::buildTargetFolder }
                            .getChildFile ("juce_SimpleBinaryBuilder.cpp");
}

build_tools::RelativePath LinuxSubprocessHelperProperties::getLinuxSubprocessHelperBinaryDataSource() const
{
    return build_tools::RelativePath ("pre_build", juce::build_tools::RelativePath::buildTargetFolder)
                            .getChildFile ("juce_LinuxSubprocessHelperBinaryData.cpp");
}

void LinuxSubprocessHelperProperties::addToExtraSearchPathsIfNecessary() const
{
    if (shouldUseLinuxSubprocessHelper())
    {
        const auto subprocessHelperBinaryDir = getLinuxSubprocessHelperBinaryDataSource().getParentDirectory();
        owner.addToExtraSearchPaths (owner.rebaseFromBuildTargetToProjectFolder (subprocessHelperBinaryDir));
    }
}

std::optional<String> LinuxSubprocessHelperProperties::getParentDirectoryRelativeToBuildTargetFolder (build_tools::RelativePath rp)
{
    jassert (rp.getRoot() == juce::build_tools::RelativePath::buildTargetFolder);
    const auto parentDir = rp.getParentDirectory().toUnixStyle();
    return parentDir == rp.toUnixStyle() ? std::nullopt : std::make_optional (parentDir);
}

String LinuxSubprocessHelperProperties::makeSnakeCase (const String& s)
{
    String result;
    result.preallocateBytes (128);

    bool previousCharacterUnderscore = false;

    for (const auto c : s)
    {
        if (   CharacterFunctions::isUpperCase (c)
            && result.length() != 0
            && ! (previousCharacterUnderscore))
        {
            result << "_";
        }

        result << CharacterFunctions::toLowerCase (c);

        previousCharacterUnderscore = c == '_';
    }

    return result;
}

String LinuxSubprocessHelperProperties::getBinaryNameFromSource (const build_tools::RelativePath& rp)
{
    return makeSnakeCase (rp.getFileNameWithoutExtension());
}
