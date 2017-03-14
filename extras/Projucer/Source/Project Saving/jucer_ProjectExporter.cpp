/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"

#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_MSVC.h"
#include "jucer_ProjectExport_XCode.h"
#include "jucer_ProjectExport_AndroidBase.h"
#include "jucer_ProjectExport_AndroidStudio.h"
#include "jucer_ProjectExport_AndroidAnt.h"
#include "jucer_ProjectExport_CodeBlocks.h"

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

    addType (types, XCodeProjectExporter::getNameMac(),          BinaryData::projectIconXcode_png,          BinaryData::projectIconXcode_pngSize);
    addType (types, XCodeProjectExporter::getNameiOS(),          BinaryData::projectIconXcodeIOS_png,       BinaryData::projectIconXcodeIOS_pngSize);
    addType (types, MSVCProjectExporterVC2015::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MSVCProjectExporterVC2013::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MSVCProjectExporterVC2012::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MSVCProjectExporterVC2010::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MSVCProjectExporterVC2008::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MSVCProjectExporterVC2005::getName(),        BinaryData::projectIconVisualStudio_png,   BinaryData::projectIconVisualStudio_pngSize);
    addType (types, MakefileProjectExporter::getNameLinux(),     BinaryData::projectIconLinuxMakefile_png,  BinaryData::projectIconLinuxMakefile_pngSize);
    addType (types, AndroidStudioProjectExporter::getName(),     BinaryData::projectIconAndroid_png,        BinaryData::projectIconAndroid_pngSize);
    addType (types, AndroidAntProjectExporter::getName(),        BinaryData::projectIconAndroid_png,        BinaryData::projectIconAndroid_pngSize);
    addType (types, CodeBlocksProjectExporter::getNameWindows(), BinaryData::projectIconCodeblocks_png,     BinaryData::projectIconCodeblocks_pngSize);
    addType (types, CodeBlocksProjectExporter::getNameLinux(),   BinaryData::projectIconCodeblocks_png,     BinaryData::projectIconCodeblocks_pngSize);

    return types;
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const int index)
{
    ProjectExporter* exp = nullptr;

    switch (index)
    {
        case 0:     exp = new XCodeProjectExporter         (project, ValueTree (XCodeProjectExporter         ::getValueTreeTypeName (false)), false); break;
        case 1:     exp = new XCodeProjectExporter         (project, ValueTree (XCodeProjectExporter         ::getValueTreeTypeName (true)), true); break;
        case 2:     exp = new MSVCProjectExporterVC2015    (project, ValueTree (MSVCProjectExporterVC2015    ::getValueTreeTypeName())); break;
        case 3:     exp = new MSVCProjectExporterVC2013    (project, ValueTree (MSVCProjectExporterVC2013    ::getValueTreeTypeName())); break;
        case 4:     exp = new MSVCProjectExporterVC2012    (project, ValueTree (MSVCProjectExporterVC2012    ::getValueTreeTypeName())); break;
        case 5:     exp = new MSVCProjectExporterVC2010    (project, ValueTree (MSVCProjectExporterVC2010    ::getValueTreeTypeName())); break;
        case 6:     exp = new MSVCProjectExporterVC2008    (project, ValueTree (MSVCProjectExporterVC2008    ::getValueTreeTypeName())); break;
        case 7:     exp = new MSVCProjectExporterVC2005    (project, ValueTree (MSVCProjectExporterVC2005    ::getValueTreeTypeName())); break;
        case 8:     exp = new MakefileProjectExporter      (project, ValueTree (MakefileProjectExporter      ::getValueTreeTypeName())); break;
        case 9:     exp = new AndroidStudioProjectExporter (project, ValueTree (AndroidStudioProjectExporter ::getValueTreeTypeName())); break;
        case 10:    exp = new AndroidAntProjectExporter    (project, ValueTree (AndroidAntProjectExporter    ::getValueTreeTypeName())); break;
        case 11:    exp = new CodeBlocksProjectExporter    (project, ValueTree (CodeBlocksProjectExporter    ::getValueTreeTypeName (CodeBlocksProjectExporter::windowsTarget)), CodeBlocksProjectExporter::windowsTarget); break;
        case 12:    exp = new CodeBlocksProjectExporter    (project, ValueTree (CodeBlocksProjectExporter    ::getValueTreeTypeName (CodeBlocksProjectExporter::linuxTarget)),   CodeBlocksProjectExporter::linuxTarget); break;
        default:    jassertfalse; return 0;
    }

    exp->createDefaultConfigs();
    exp->createDefaultModulePaths();

    return exp;
}

StringArray ProjectExporter::getExporterNames()
{
    StringArray s;
    Array<ExporterTypeInfo> types (getExporterTypes());

    for (int i = 0; i < types.size(); ++i)
        s.add (types.getReference(i).name);

    return s;
}

String ProjectExporter::getCurrentPlatformExporterName()
{
   #if JUCE_MAC
    return XCodeProjectExporter::getNameMac();
   #elif JUCE_WINDOWS
    return MSVCProjectExporterVC2015::getName();
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
    ProjectExporter*       exp = MSVCProjectExporterVC2005    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2008    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2010    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2012    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2013    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2015    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = XCodeProjectExporter         ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MakefileProjectExporter      ::createForSettings (project, settings);
    if (exp == nullptr)    exp = AndroidStudioProjectExporter ::createForSettings (project, settings);
    if (exp == nullptr)    exp = AndroidAntProjectExporter    ::createForSettings (project, settings);
    if (exp == nullptr)    exp = CodeBlocksProjectExporter    ::createForSettings (project, settings);

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
            XCodeProjectExporter::getValueTreeTypeName (false),
            XCodeProjectExporter::getValueTreeTypeName (true),
           #elif JUCE_WINDOWS
            MSVCProjectExporterVC2005::getValueTreeTypeName(),
            MSVCProjectExporterVC2008::getValueTreeTypeName(),
            MSVCProjectExporterVC2010::getValueTreeTypeName(),
            MSVCProjectExporterVC2012::getValueTreeTypeName(),
            MSVCProjectExporterVC2013::getValueTreeTypeName(),
            MSVCProjectExporterVC2015::getValueTreeTypeName(),
           #elif JUCE_LINUX
            // (this doesn't currently launch.. not really sure what it would do on linux)
            //MakefileProjectExporter::getValueTreeTypeName(),
           #endif
            AndroidStudioProjectExporter::getValueTreeTypeName(),

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
    : makefileIsDLL (false),
      msvcIsDLL (false),
      msvcIsWindowsSubsystem (true),
      settings (state),
      project (p),
      projectType (p.getProjectType()),
      projectName (p.getTitle()),
      projectFolder (p.getProjectFolder()),
      modulesGroup (nullptr)
{
}

ProjectExporter::~ProjectExporter()
{
}

void ProjectExporter::updateDeprecatedProjectSettingsInteractively() {}

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

    createIconProperties (props);

    createExporterProperties (props);

    props.add (new TextPropertyComponent (getUserNotes(), "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

void ProjectExporter::createDependencyPathProperties (PropertyListBuilder& props)
{
    if (supportsVST3() && (project.shouldBuildVST3().getValue() || project.isVST3PluginHost()))
    {
        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), getVST3PathValue(), "VST3 SDK Folder"),
                   "If you're building a VST3 plugin or host, this must be the folder containing the VST3 SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (supportsAAX() && project.shouldBuildAAX().getValue())
    {
        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), getAAXPathValue(), "AAX SDK Folder"),
                   "If you're building an AAX plugin, this must be the folder containing the AAX SDK. This can be an absolute path, or a path relative to the Projucer project file.");
    }

    if (supportsRTAS() && project.shouldBuildRTAS().getValue())
    {
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
    if (supportsVST() && project.shouldBuildVST().getValue())
        makefileTargetSuffix = ".so";

    if (supportsVST3())
    {
        if (project.shouldBuildVST3().getValue())
            makefileTargetSuffix = ".so";

        if (project.shouldBuildVST3().getValue() || project.isVST3PluginHost())
            addVST3FolderToPath();
    }
}

void ProjectExporter::addCommonAudioPluginSettings()
{
    if (isLinux() && (getProject().shouldBuildVST().getValue() || getProject().shouldBuildVST3().getValue()))
        makefileExtraLinkerFlags.add ("-Wl,--no-undefined");

    if (supportsAAX() && getProject().shouldBuildAAX().getValue())
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
StringPairArray ProjectExporter::getAllPreprocessorDefs (const ProjectExporter::BuildConfiguration& config) const
{
    StringPairArray defs (mergePreprocessorDefs (config.getAllPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    addDefaultPreprocessorDefs (defs);
    return defs;
}

StringPairArray ProjectExporter::getAllPreprocessorDefs() const
{
    StringPairArray defs (mergePreprocessorDefs (project.getPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    addDefaultPreprocessorDefs (defs);
    return defs;
}

void ProjectExporter::addDefaultPreprocessorDefs (StringPairArray& defs) const
{
    defs.set (getExporterIdentifierMacro(), "1");
    defs.set ("JUCE_APP_VERSION", project.getVersionString());
    defs.set ("JUCE_APP_VERSION_HEX", project.getVersionAsHex());
}

String ProjectExporter::replacePreprocessorTokens (const ProjectExporter::BuildConfiguration& config, const String& sourceString) const
{
    return replacePreprocessorDefs (getAllPreprocessorDefs (config), sourceString);
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

void ProjectExporter::addToExtraSearchPaths (const RelativePath& pathFromProjectFolder, int index)
{
    RelativePath localPath (rebaseFromProjectFolderToBuildTarget (pathFromProjectFolder));

    const String path (isVisualStudio() ? localPath.toWindowsStyle() : localPath.toUnixStyle());

    if (! extraSearchPaths.contains (path))
        extraSearchPaths.insert (index, path);
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
    return settings.getChildWithName (Ids::MODULEPATHS)
                .getChildWithProperty (Ids::ID, moduleID) [Ids::path].toString();
}

void ProjectExporter::removePathForModule (const String& moduleID)
{
    ValueTree paths (settings.getChildWithName (Ids::MODULEPATHS));
    ValueTree m (paths.getChildWithProperty (Ids::ID, moduleID));
    paths.removeChild (m, project.getUndoManagerFor (settings));
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
        || (p1.isAndroid() && p2.isAndroid())
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

    createConfigProperties (props);

    props.add (new TextPropertyComponent (getUserNotes(), "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

StringPairArray ProjectExporter::BuildConfiguration::getAllPreprocessorDefs() const
{
    return mergePreprocessorDefs (project.getPreprocessorDefs(),
                                  parsePreprocessorDefs (getBuildConfigPreprocessorDefsString()));
}

StringArray ProjectExporter::BuildConfiguration::getHeaderSearchPaths() const
{
    return getSearchPathsFromString (getHeaderSearchPathString());
}

StringArray ProjectExporter::BuildConfiguration::getLibrarySearchPaths() const
{
    return getSearchPathsFromString (getLibrarySearchPathString());
}

String ProjectExporter::BuildConfiguration::getGCCLibraryPathFlags() const
{
    String s;
    const StringArray libraryPaths (getLibrarySearchPaths());

    for (int i = 0; i < libraryPaths.size(); ++i)
        s << " -L" << escapeSpaces (libraryPaths[i]).replace ("~", "$(HOME)");

    return s;
}

String ProjectExporter::getExternalLibraryFlags (const BuildConfiguration& config) const
{
    StringArray libraries;
    libraries.addTokens (getExternalLibrariesString(), ";\n", "\"'");
    libraries.removeEmptyStrings (true);

    if (libraries.size() != 0)
        return replacePreprocessorTokens (config, "-l" + libraries.joinIntoString (" -l")).trim();

    return String();
}
