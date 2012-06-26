/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "jucer_ProjectExporter.h"
#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_MSVC.h"
#include "jucer_ProjectExport_XCode.h"
#include "jucer_ProjectExport_Android.h"


//==============================================================================
int ProjectExporter::getNumExporters()
{
    return 6;
}

StringArray ProjectExporter::getExporterNames()
{
    StringArray s;
    s.add (XCodeProjectExporter::getNameMac());
    s.add (XCodeProjectExporter::getNameiOS());
    s.add (MSVCProjectExporterVC2005::getName());
    s.add (MSVCProjectExporterVC2008::getName());
    s.add (MSVCProjectExporterVC2010::getName());
    s.add (MakefileProjectExporter::getNameLinux());
    s.add (AndroidProjectExporter::getNameAndroid());
    return s;
}

StringArray ProjectExporter::getDefaultExporters()
{
    StringArray s;
    s.add (XCodeProjectExporter::getNameMac());
    s.add (MSVCProjectExporterVC2010::getName());
    s.add (MakefileProjectExporter::getNameLinux());
    return s;
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const int index)
{
    ProjectExporter* exp = nullptr;

    switch (index)
    {
        case 0:     exp = new XCodeProjectExporter      (project, ValueTree (XCodeProjectExporter     ::getValueTreeTypeName (false)), false); break;
        case 1:     exp = new XCodeProjectExporter      (project, ValueTree (XCodeProjectExporter     ::getValueTreeTypeName (true)), true); break;
        case 2:     exp = new MSVCProjectExporterVC2005 (project, ValueTree (MSVCProjectExporterVC2005::getValueTreeTypeName())); break;
        case 3:     exp = new MSVCProjectExporterVC2008 (project, ValueTree (MSVCProjectExporterVC2008::getValueTreeTypeName())); break;
        case 4:     exp = new MSVCProjectExporterVC2010 (project, ValueTree (MSVCProjectExporterVC2010::getValueTreeTypeName())); break;
        case 5:     exp = new MakefileProjectExporter   (project, ValueTree (MakefileProjectExporter  ::getValueTreeTypeName())); break;
        case 6:     exp = new AndroidProjectExporter    (project, ValueTree (AndroidProjectExporter   ::getValueTreeTypeName())); break;
        default:    jassertfalse; return 0;
    }

    File juceFolder (ModuleList::getLocalModulesFolder (&project));
    File target (exp->getTargetFolder());

    if (FileHelpers::shouldPathsBeRelative (juceFolder.getFullPathName(), project.getFile().getFullPathName()))
        exp->getJuceFolderValue() = FileHelpers::getRelativePathFrom (juceFolder, project.getFile().getParentDirectory());
    else
        exp->getJuceFolderValue() = juceFolder.getFullPathName();

    exp->createDefaultConfigs();

    return exp;
}

ProjectExporter* ProjectExporter::createNewExporter (Project& project, const String& name)
{
    return createNewExporter (project, getExporterNames().indexOf (name));
}

ProjectExporter* ProjectExporter::createExporter (Project& project, const ValueTree& settings)
{
    ProjectExporter*       exp = MSVCProjectExporterVC2005::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2008::createForSettings (project, settings);
    if (exp == nullptr)    exp = MSVCProjectExporterVC2010::createForSettings (project, settings);
    if (exp == nullptr)    exp = XCodeProjectExporter     ::createForSettings (project, settings);
    if (exp == nullptr)    exp = MakefileProjectExporter  ::createForSettings (project, settings);
    if (exp == nullptr)    exp = AndroidProjectExporter   ::createForSettings (project, settings);

    jassert (exp != nullptr);
    return exp;
}

ProjectExporter* ProjectExporter::createPlatformDefaultExporter (Project& project)
{
    ScopedPointer <ProjectExporter> best;
    int bestPref = 0;

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        const int pref = exporter->getLaunchPreferenceOrderForCurrentOS();

        if (pref > bestPref)
        {
            bestPref = pref;
            best = exporter.exporter;
        }
    }

    return best.release();
}

//==============================================================================
ProjectExporter::ProjectExporter (Project& project_, const ValueTree& settings_)
    : xcodeIsBundle (false),
      xcodeCreatePList (false),
      xcodeCanUseDwarf (true),
      makefileIsDLL (false),
      msvcIsDLL (false),
      msvcIsWindowsSubsystem (true),
      msvcNeedsDLLRuntimeLib (false),
      settings (settings_),
      project (project_),
      projectType (project_.getProjectType()),
      projectName (project_.getTitle()),
      projectFolder (project_.getFile().getParentDirectory()),
      modulesGroup (nullptr)
{
    groups.add (project.getMainGroup().createCopy());
}

ProjectExporter::~ProjectExporter()
{
}

File ProjectExporter::getTargetFolder() const
{
    return project.resolveFilename (getTargetLocationString());
}

String ProjectExporter::getIncludePathForFileInJuceFolder (const String& pathFromJuceFolder, const File& targetIncludeFile) const
{
    String juceFolderPath (getJuceFolderString());

    if (juceFolderPath.startsWithChar ('<'))
    {
        juceFolderPath = FileHelpers::unixStylePath (File::addTrailingSeparator (juceFolderPath.substring (1).dropLastCharacters(1)));
        if (juceFolderPath == "/")
            juceFolderPath = String::empty;

        return "<" + juceFolderPath + pathFromJuceFolder + ">";
    }
    else
    {
        const RelativePath juceFromProject (juceFolderPath, RelativePath::projectFolder);
        const RelativePath fileFromProject (juceFromProject.getChildFile (pathFromJuceFolder));
        const RelativePath fileFromHere (fileFromProject.rebased (project.getFile().getParentDirectory(),
                                                                  targetIncludeFile.getParentDirectory(), RelativePath::unknown));
        return fileFromHere.toUnixStyle().quoted();
    }
}

RelativePath ProjectExporter::getJucePathFromProjectFolder() const
{
    return RelativePath (getJuceFolderString(), RelativePath::projectFolder);
}

RelativePath ProjectExporter::getJucePathFromTargetFolder() const
{
    return rebaseFromProjectFolderToBuildTarget (getJucePathFromProjectFolder());
}

RelativePath ProjectExporter::rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const
{
    return path.rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder);
}

bool ProjectExporter::shouldFileBeCompiledByDefault (const RelativePath& file) const
{
    return file.hasFileExtension ("cpp;cc;c;cxx");
}

void ProjectExporter::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (getTargetLocationValue(), "Target Project Folder", 1024, false),
               "The location of the folder in which the " + name + " project will be created. This path can be absolute, but it's much more sensible to make it relative to the jucer project directory.");

    props.add (new TextPropertyComponent (getJuceFolderValue(), "Local JUCE folder", 1024, false),
               "The location of the Juce library folder that the " + name + " project will use to when compiling. This can be an absolute path, or relative to the jucer project folder, but it must be valid on the filesystem of the machine you use to actually do the compiling.");

    OwnedArray<LibraryModule> modules;
    ModuleList moduleList;
    moduleList.rescan (ModuleList::getDefaultModulesFolder (&project));
    project.createRequiredModules (moduleList, modules);
    for (int i = 0; i < modules.size(); ++i)
        modules.getUnchecked(i)->createPropertyEditors (*this, props);

    props.add (new TextPropertyComponent (getExporterPreprocessorDefs(), "Extra Preprocessor Definitions", 32768, false),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new TextPropertyComponent (getExtraCompilerFlags(), "Extra compiler flags", 2048, false),
               "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor definitions in the form ${NAME_OF_DEFINITION}, which will be replaced with their values.");
    props.add (new TextPropertyComponent (getExtraLinkerFlags(), "Extra linker flags", 2048, false),
               "Extra command-line flags to be passed to the linker. You might want to use this for adding additional libraries. This string can contain references to preprocessor definitions in the form ${NAME_OF_VALUE}, which will be replaced with their values.");

    {
        OwnedArray<Project::Item> images;
        project.findAllImageItems (images);

        StringArray choices;
        Array<var> ids;

        choices.add ("<None>");
        ids.add (var::null);
        choices.add (String::empty);
        ids.add (var::null);

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
}

StringPairArray ProjectExporter::getAllPreprocessorDefs (const ProjectExporter::BuildConfiguration& config) const
{
    StringPairArray defs (mergePreprocessorDefs (config.getAllPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    defs.set (getExporterIdentifierMacro(), "1");
    return defs;
}

StringPairArray ProjectExporter::getAllPreprocessorDefs() const
{
    StringPairArray defs (mergePreprocessorDefs (project.getPreprocessorDefs(),
                                                 parsePreprocessorDefs (getExporterPreprocessorDefsString())));
    defs.set (getExporterIdentifierMacro(), "1");
    return defs;
}

String ProjectExporter::replacePreprocessorTokens (const ProjectExporter::BuildConfiguration& config, const String& sourceString) const
{
    return replacePreprocessorDefs (getAllPreprocessorDefs (config), sourceString);
}

Project::Item& ProjectExporter::getModulesGroup()
{
    if (modulesGroup == nullptr)
    {
        groups.add (Project::Item::createGroup (project, "Juce Modules", "__modulesgroup__"));
        modulesGroup = &(groups.getReference (groups.size() - 1));
    }

    return *modulesGroup;
}

void ProjectExporter::addToExtraSearchPaths (const RelativePath& pathFromProjectFolder)
{
    RelativePath localPath (rebaseFromProjectFolderToBuildTarget (pathFromProjectFolder));

    const String path (isVisualStudio() ? localPath.toWindowsStyle() : localPath.toUnixStyle());
    extraSearchPaths.addIfNotAlreadyThere (path, false);
}


//==============================================================================
const Identifier ProjectExporter::configurations ("CONFIGURATIONS");
const Identifier ProjectExporter::configuration  ("CONFIGURATION");

ValueTree ProjectExporter::getConfigurations() const
{
    return settings.getChildWithName (configurations);
}

int ProjectExporter::getNumConfigurations() const
{
    return getConfigurations().getNumChildren();
}

ProjectExporter::BuildConfiguration::Ptr ProjectExporter::getConfiguration (int index) const
{
    return createBuildConfig (getConfigurations().getChild (index));
}

bool ProjectExporter::hasConfigurationNamed (const String& name) const
{
    const ValueTree configs (getConfigurations());
    for (int i = configs.getNumChildren(); --i >= 0;)
        if (configs.getChild(i) [Ids::name].toString() == name)
            return true;

    return false;
}

String ProjectExporter::getUniqueConfigName (String name) const
{
    String nameRoot (name);
    while (CharacterFunctions::isDigit (nameRoot.getLastCharacter()))
        nameRoot = nameRoot.dropLastCharacters (1);

    nameRoot = nameRoot.trim();

    int suffix = 2;
    while (hasConfigurationNamed (name))
        name = nameRoot + " " + String (suffix++);

    return name;
}

void ProjectExporter::addNewConfiguration (const BuildConfiguration* configToCopy)
{
    const String configName (getUniqueConfigName (configToCopy != nullptr ? configToCopy->config [Ids::name].toString()
                                                                          : "New Build Configuration"));

    ValueTree configs (getConfigurations());

    if (! configs.isValid())
    {
        settings.addChild (ValueTree (configurations), 0, project.getUndoManagerFor (settings));
        configs = getConfigurations();
    }

    ValueTree newConfig (configuration);
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
    settings.getOrCreateChildWithName (configurations, nullptr);

    for (int i = 0; i < 2; ++i)
    {
        addNewConfiguration (nullptr);
        BuildConfiguration::Ptr config (getConfiguration (i));

        const bool debugConfig = i == 0;

        config->getNameValue() = debugConfig ? "Debug" : "Release";
        config->isDebugValue() = debugConfig;
        config->getOptimisationLevel() = debugConfig ? 1 : 2;
        config->getTargetBinaryName() = project.getProjectFilenameRoot();
    }
}

Image ProjectExporter::getBigIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::bigIcon]).loadAsImageFile();
}

Image ProjectExporter::getSmallIcon() const
{
    return project.getMainGroup().findItemWithID (settings [Ids::smallIcon]).loadAsImageFile();
}

Image ProjectExporter::getBestIconForSize (int size, bool returnNullIfNothingBigEnough) const
{
    Image im;

    const Image im1 (getSmallIcon());
    const Image im2 (getBigIcon());

    if (im1.isValid() && im2.isValid())
    {
        if (im1.getWidth() >= size && im2.getWidth() >= size)
            im = im1.getWidth() < im2.getWidth() ? im1 : im2;
        else if (im1.getWidth() >= size)
            im = im1;
        else if (im2.getWidth() >= size)
            im = im2;
        else
            return Image::null;
    }
    else
    {
        im = im1.isValid() ? im1 : im2;
    }

    if (returnNullIfNothingBigEnough && im.getWidth() < size && im.getHeight() < size)
        return Image::null;

    return rescaleImageForIcon (im, size);
}

Image ProjectExporter::rescaleImageForIcon (Image im, const int size)
{
    im = SoftwareImageType().convert (im);

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


//==============================================================================
ProjectExporter::ConfigIterator::ConfigIterator (ProjectExporter& exporter_)
    : index (-1), exporter (exporter_)
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
ProjectExporter::BuildConfiguration::BuildConfiguration (Project& project_, const ValueTree& configNode)
   : config (configNode), project (project_)
{
}

ProjectExporter::BuildConfiguration::~BuildConfiguration()
{
}

String ProjectExporter::BuildConfiguration::getGCCOptimisationFlag() const
{
    const int level = getOptimisationLevelInt();
    return String (level <= 1 ? "0" : (level == 2 ? "s" : "3"));
}

void ProjectExporter::BuildConfiguration::createBasicPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (getNameValue(), "Name", 96, false),
               "The name of this configuration.");

    props.add (new BooleanPropertyComponent (isDebugValue(), "Debug mode", "Debugging enabled"),
               "If enabled, this means that the configuration should be built with debug synbols.");

    const char* optimisationLevels[] = { "No optimisation", "Optimise for size and speed", "Optimise for maximum speed", 0 };
    const int optimisationLevelValues[] = { 1, 2, 3, 0 };
    props.add (new ChoicePropertyComponent (getOptimisationLevel(), "Optimisation",
                                            StringArray (optimisationLevels), Array<var> (optimisationLevelValues)),
               "The optimisation level for this configuration");

    props.add (new TextPropertyComponent (getTargetBinaryName(), "Binary name", 256, false),
               "The filename to use for the destination binary executable file. If you don't add a suffix to this name, "
               "a suitable platform-specific suffix will be added automatically.");

    props.add (new TextPropertyComponent (getTargetBinaryRelativePath(), "Binary location", 1024, false),
               "The folder in which the finished binary should be placed. Leave this blank to cause the binary to be placed "
               "in its default location in the build folder.");

    props.add (new TextPropertyComponent (getHeaderSearchPathValue(), "Header search paths", 16384, false),
               "Extra header search paths. Use semi-colons to separate multiple paths.");

    props.add (new TextPropertyComponent (getLibrarySearchPathValue(), "Extra library search paths", 16384, false),
               "Extra library search paths. Use semi-colons to separate multiple paths.");

    props.add (new TextPropertyComponent (getBuildConfigPreprocessorDefs(), "Preprocessor definitions", 32768, false),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate "
               "the items - to include a space or comma in a definition, precede it with a backslash.");

    props.setPreferredHeight (22);
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
        s << " -L" << addQuotesIfContainsSpaces (libraryPaths[i]);

    return s;
}
