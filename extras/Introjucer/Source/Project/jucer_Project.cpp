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

#include "jucer_Project.h"
#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "../Application/jucer_OpenDocumentManager.h"
#include "../Application/jucer_Application.h"


//==============================================================================
namespace Tags
{
    const Identifier projectRoot       ("JUCERPROJECT");
    const Identifier projectMainGroup  ("MAINGROUP");
    const Identifier group             ("GROUP");
    const Identifier file              ("FILE");
    const Identifier exporters         ("EXPORTFORMATS");
    const Identifier configGroup       ("JUCEOPTIONS");
    const Identifier modulesGroup      ("MODULES");
    const Identifier module            ("MODULE");
}

const char* Project::projectFileExtension = ".jucer";

//==============================================================================
Project::Project (const File& file_)
    : FileBasedDocument (projectFileExtension,
                         String ("*") + projectFileExtension,
                         "Choose a Jucer project to load",
                         "Save Jucer project"),
      projectRoot (Tags::projectRoot)
{
    setFile (file_);
    removeDefunctExporters();
    setMissingDefaultValues();

    setChangedFlag (false);

    mainProjectIcon.setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));

    projectRoot.addListener (this);
}

Project::~Project()
{
    projectRoot.removeListener (this);
    OpenDocumentManager::getInstance()->closeAllDocumentsUsingProject (*this, false);
}

//==============================================================================
void Project::setTitle (const String& newTitle)
{
    projectRoot.setProperty (Ids::name, newTitle, getUndoManagerFor (projectRoot));
    getMainGroup().getNameValue() = newTitle;
}

String Project::getTitle() const
{
    return projectRoot.getChildWithName (Tags::projectMainGroup) [Ids::name];
}

String Project::getDocumentTitle()
{
    return getTitle();
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty (Ids::jucerVersion, ProjectInfo::versionString, 0);
    projectRoot.setProperty (Ids::name, getDocumentTitle(), 0);
}

void Project::setMissingDefaultValues()
{
    if (! projectRoot.hasProperty (ComponentBuilder::idProperty))
        projectRoot.setProperty (ComponentBuilder::idProperty, createAlphaNumericUID(), nullptr);

    // Create main file group if missing
    if (! projectRoot.getChildWithName (Tags::projectMainGroup).isValid())
    {
        Item mainGroup (*this, ValueTree (Tags::projectMainGroup));
        projectRoot.addChild (mainGroup.state, 0, 0);
    }

    getMainGroup().initialiseMissingProperties();

    if (getDocumentTitle().isEmpty())
        setTitle ("JUCE Project");

    if (! projectRoot.hasProperty (Ids::projectType))
        getProjectTypeValue() = ProjectType::getGUIAppTypeName();

    if (! projectRoot.hasProperty (Ids::version))
        getVersionValue() = "1.0.0";

    updateOldStyleConfigList();
    moveOldPropertyFromProjectToAllExporters (Ids::bigIcon);
    moveOldPropertyFromProjectToAllExporters (Ids::smallIcon);

    getProjectType().setMissingProjectProperties (*this);

    if (! projectRoot.getChildWithName (Tags::modulesGroup).isValid())
        addDefaultModules (false);
}

void Project::updateOldStyleConfigList()
{
    ValueTree deprecatedConfigsList (projectRoot.getChildWithName (ProjectExporter::configurations));

    if (deprecatedConfigsList.isValid())
    {
        projectRoot.removeChild (deprecatedConfigsList, nullptr);

        for (Project::ExporterIterator exporter (*this); exporter.next();)
        {
            if (exporter->getNumConfigurations() == 0)
            {
                ValueTree newConfigs (deprecatedConfigsList.createCopy());

                if (! exporter->isXcode())
                {
                    for (int j = newConfigs.getNumChildren(); --j >= 0;)
                    {
                        ValueTree config (newConfigs.getChild(j));

                        config.removeProperty (Ids::osxSDK, nullptr);
                        config.removeProperty (Ids::osxCompatibility, nullptr);
                        config.removeProperty (Ids::osxArchitecture, nullptr);
                    }
                }

                exporter->settings.addChild (newConfigs, 0, nullptr);
            }
        }
    }
}

void Project::moveOldPropertyFromProjectToAllExporters (Identifier name)
{
    if (projectRoot.hasProperty (name))
    {
        for (Project::ExporterIterator exporter (*this); exporter.next();)
            exporter->settings.setProperty (name, projectRoot [name], nullptr);

        projectRoot.removeProperty (name, nullptr);
    }
}

void Project::removeDefunctExporters()
{
    ValueTree exporters (projectRoot.getChildWithName (Tags::exporters));

    for (;;)
    {
        ValueTree oldVC6Exporter (exporters.getChildWithName ("MSVC6"));

        if (oldVC6Exporter.isValid())
            exporters.removeChild (oldVC6Exporter, nullptr);
        else
            break;
    }
}

void Project::addDefaultModules (bool shouldCopyFilesLocally)
{
    addModule ("juce_core", shouldCopyFilesLocally);

    if (! isConfigFlagEnabled ("JUCE_ONLY_BUILD_CORE_LIBRARY"))
    {
        addModule ("juce_events", shouldCopyFilesLocally);
        addModule ("juce_graphics", shouldCopyFilesLocally);
        addModule ("juce_data_structures", shouldCopyFilesLocally);
        addModule ("juce_gui_basics", shouldCopyFilesLocally);
        addModule ("juce_gui_extra", shouldCopyFilesLocally);
        addModule ("juce_gui_audio", shouldCopyFilesLocally);
        addModule ("juce_cryptography", shouldCopyFilesLocally);
        addModule ("juce_video", shouldCopyFilesLocally);
        addModule ("juce_opengl", shouldCopyFilesLocally);
        addModule ("juce_audio_basics", shouldCopyFilesLocally);
        addModule ("juce_audio_devices", shouldCopyFilesLocally);
        addModule ("juce_audio_formats", shouldCopyFilesLocally);
        addModule ("juce_audio_processors", shouldCopyFilesLocally);
    }
}

bool Project::isAudioPluginModuleMissing() const
{
    return getProjectType().isAudioPlugin()
            && ! isModuleEnabled ("juce_audio_plugin_client");
}

//==============================================================================
static void registerRecentFile (const File& file)
{
    RecentlyOpenedFilesList::registerRecentFileNatively (file);
    StoredSettings::getInstance()->recentFiles.addFile (file);
    StoredSettings::getInstance()->flush();
}

Result Project::loadDocument (const File& file)
{
    ScopedPointer <XmlElement> xml (XmlDocument::parse (file));

    if (xml == nullptr || ! xml->hasTagName (Tags::projectRoot.toString()))
        return Result::fail ("Not a valid Jucer project!");

    ValueTree newTree (ValueTree::fromXml (*xml));

    if (! newTree.hasType (Tags::projectRoot))
        return Result::fail ("The document contains errors and couldn't be parsed!");

    registerRecentFile (file);
    projectRoot = newTree;

    removeDefunctExporters();
    setMissingDefaultValues();

    return Result::ok();
}

Result Project::saveDocument (const File& file)
{
    return saveProject (file, false);
}

Result Project::saveProject (const File& file, bool isCommandLineApp)
{
    updateProjectSettings();
    sanitiseConfigFlags();

    if (! isCommandLineApp)
        registerRecentFile (file);

    ProjectSaver saver (*this, file);
    return saver.save (! isCommandLineApp);
}

Result Project::saveResourcesOnly (const File& file)
{
    ProjectSaver saver (*this, file);
    return saver.saveResourcesOnly();
}

//==============================================================================
static File lastDocumentOpened;

File Project::getLastDocumentOpened()                   { return lastDocumentOpened; }
void Project::setLastDocumentOpened (const File& file)  { lastDocumentOpened = file; }

//==============================================================================
void Project::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (property == Ids::projectType)
        setMissingDefaultValues();

    changed();
}

void Project::valueTreeChildAdded (ValueTree&, ValueTree&)      { changed(); }
void Project::valueTreeChildRemoved (ValueTree&, ValueTree&)    { changed(); }
void Project::valueTreeChildOrderChanged (ValueTree&)           { changed(); }
void Project::valueTreeParentChanged (ValueTree&)               {}

//==============================================================================
File Project::resolveFilename (String filename) const
{
    if (filename.isEmpty())
        return File::nonexistent;

    filename = replacePreprocessorDefs (getPreprocessorDefs(), filename)
                .replaceCharacter ('\\', '/');

    if (FileHelpers::isAbsolutePath (filename))
        return File::createFileWithoutCheckingPath (filename); // (avoid assertions for windows-style paths)

    return getFile().getSiblingFile (filename);
}

String Project::getRelativePathForFile (const File& file) const
{
    String filename (file.getFullPathName());

    File relativePathBase (getFile().getParentDirectory());

    String p1 (relativePathBase.getFullPathName());
    String p2 (file.getFullPathName());

    while (p1.startsWithChar (File::separator))
        p1 = p1.substring (1);

    while (p2.startsWithChar (File::separator))
        p2 = p2.substring (1);

    if (p1.upToFirstOccurrenceOf (File::separatorString, true, false)
          .equalsIgnoreCase (p2.upToFirstOccurrenceOf (File::separatorString, true, false)))
    {
        filename = FileHelpers::getRelativePathFrom (file, relativePathBase);
    }

    return filename;
}

//==============================================================================
const ProjectType& Project::getProjectType() const
{
    const ProjectType* type = ProjectType::findType (getProjectTypeString());
    jassert (type != nullptr);

    if (type == nullptr)
    {
        type = ProjectType::findType (ProjectType::getGUIAppTypeName());
        jassert (type != nullptr);
    }

    return *type;
}

//==============================================================================
void Project::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (getProjectNameValue(), "Project Name", 256, false),
               "The name of the project.");

    props.add (new TextPropertyComponent (getVersionValue(), "Project Version", 16, false),
               "The project's version number, This should be in the format major.minor.point");

    props.add (new TextPropertyComponent (getCompanyName(), "Company Name", 256, false),
               "Your company name, which will be added to the properties of the binary where possible");

    {
        StringArray projectTypeNames;
        Array<var> projectTypeCodes;

        const Array<ProjectType*>& types = ProjectType::getAllTypes();

        for (int i = 0; i < types.size(); ++i)
        {
            projectTypeNames.add (types.getUnchecked(i)->getDescription());
            projectTypeCodes.add (types.getUnchecked(i)->getType());
        }

        props.add (new ChoicePropertyComponent (getProjectTypeValue(), "Project Type", projectTypeNames, projectTypeCodes));
    }

    props.add (new TextPropertyComponent (getBundleIdentifier(), "Bundle Identifier", 256, false),
               "A unique identifier for this product, mainly for use in Mac builds. It should be something like 'com.yourcompanyname.yourproductname'");

    getProjectType().createPropertyEditors (*this, props);

    props.add (new TextPropertyComponent (getProjectPreprocessorDefs(), "Preprocessor definitions", 32768, false),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.setPreferredHeight (22);
}

String Project::getVersionAsHex() const
{
    StringArray configs;
    configs.addTokens (getVersionString(), ",.", String::empty);
    configs.trim();
    configs.removeEmptyStrings();

    int value = (configs[0].getIntValue() << 16) + (configs[1].getIntValue() << 8) + configs[2].getIntValue();

    if (configs.size() >= 4)
        value = (value << 8) + configs[3].getIntValue();

    return "0x" + String::toHexString (value);
}

StringPairArray Project::getPreprocessorDefs() const
{
    return parsePreprocessorDefs (projectRoot [Ids::defines]);
}

//==============================================================================
Project::Item Project::getMainGroup()
{
    return Item (*this, projectRoot.getChildWithName (Tags::projectMainGroup));
}

static void findImages (const Project::Item& item, OwnedArray<Project::Item>& found)
{
    if (item.isImageFile())
    {
        found.add (new Project::Item (item));
    }
    else if (item.isGroup())
    {
        for (int i = 0; i < item.getNumChildren(); ++i)
            findImages (item.getChild (i), found);
    }
}

void Project::findAllImageItems (OwnedArray<Project::Item>& items)
{
    findImages (getMainGroup(), items);
}

//==============================================================================
Project::Item::Item (Project& project_, const ValueTree& state_)
    : project (project_), state (state_)
{
}

Project::Item::Item (const Item& other)
    : project (other.project), state (other.state)
{
}

Project::Item Project::Item::createCopy()         { Item i (*this); i.state = i.state.createCopy(); return i; }

String Project::Item::getID() const               { return state [ComponentBuilder::idProperty]; }
void Project::Item::setID (const String& newID)   { state.setProperty (ComponentBuilder::idProperty, newID, nullptr); }

String Project::Item::getImageFileID() const      { return "id:" + getID(); }

Image Project::Item::loadAsImageFile() const
{
    return isValid() ? ImageCache::getFromFile (getFile())
                     : Image::null;
}

Project::Item Project::Item::createGroup (Project& project, const String& name, const String& uid)
{
    Item group (project, ValueTree (Tags::group));
    group.setID (uid);
    group.initialiseMissingProperties();
    group.getNameValue() = name;
    return group;
}

bool Project::Item::isFile() const          { return state.hasType (Tags::file); }
bool Project::Item::isGroup() const         { return state.hasType (Tags::group) || isMainGroup(); }
bool Project::Item::isMainGroup() const     { return state.hasType (Tags::projectMainGroup); }
bool Project::Item::isImageFile() const     { return isFile() && getFile().hasFileExtension ("png;jpg;jpeg;gif;drawable"); }

Project::Item Project::Item::findItemWithID (const String& targetId) const
{
    if (state [ComponentBuilder::idProperty] == targetId)
        return *this;

    if (isGroup())
    {
        for (int i = getNumChildren(); --i >= 0;)
        {
            Item found (getChild(i).findItemWithID (targetId));
            if (found.isValid())
                return found;
        }
    }

    return Item (project, ValueTree::invalid);
}

bool Project::Item::canContain (const Item& child) const
{
    if (isFile())
        return false;

    if (isGroup())
        return child.isFile() || child.isGroup();

    jassertfalse
    return false;
}

bool Project::Item::shouldBeAddedToTargetProject() const    { return isFile(); }

Value Project::Item::getShouldCompileValue()                { return state.getPropertyAsValue (Ids::compile, getUndoManager()); }
bool Project::Item::shouldBeCompiled() const                { return state [Ids::compile]; }

Value Project::Item::getShouldAddToResourceValue()          { return state.getPropertyAsValue (Ids::resource, getUndoManager()); }
bool Project::Item::shouldBeAddedToBinaryResources() const  { return state [Ids::resource]; }

Value Project::Item::getShouldInhibitWarningsValue()        { return state.getPropertyAsValue (Ids::noWarnings, getUndoManager()); }
bool Project::Item::shouldInhibitWarnings() const           { return state [Ids::noWarnings]; }

Value Project::Item::getShouldUseStdCallValue()             { return state.getPropertyAsValue (Ids::useStdCall, nullptr); }
bool Project::Item::shouldUseStdCall() const                { return state [Ids::useStdCall]; }

String Project::Item::getFilePath() const
{
    if (isFile())
        return state [Ids::file].toString();
    else
        return String::empty;
}

File Project::Item::getFile() const
{
    if (isFile())
        return project.resolveFilename (state [Ids::file].toString());
    else
        return File::nonexistent;
}

void Project::Item::setFile (const File& file)
{
    setFile (RelativePath (project.getRelativePathForFile (file), RelativePath::projectFolder));
    jassert (getFile() == file);
}

void Project::Item::setFile (const RelativePath& file)
{
    jassert (file.getRoot() == RelativePath::projectFolder);
    jassert (isFile());
    state.setProperty (Ids::file, file.toUnixStyle(), getUndoManager());
    state.setProperty (Ids::name, file.getFileName(), getUndoManager());
}

bool Project::Item::renameFile (const File& newFile)
{
    const File oldFile (getFile());

    if (oldFile.moveFileTo (newFile)
         || (newFile.exists() && ! oldFile.exists()))
    {
        setFile (newFile);
        OpenDocumentManager::getInstance()->fileHasBeenRenamed (oldFile, newFile);
        return true;
    }

    return false;
}

bool Project::Item::containsChildForFile (const RelativePath& file) const
{
    return state.getChildWithProperty (Ids::file, file.toUnixStyle()).isValid();
}

Project::Item Project::Item::findItemForFile (const File& file) const
{
    if (getFile() == file)
        return *this;

    if (isGroup())
    {
        for (int i = getNumChildren(); --i >= 0;)
        {
            Item found (getChild(i).findItemForFile (file));

            if (found.isValid())
                return found;
        }
    }

    return Item (project, ValueTree::invalid);
}

File Project::Item::determineGroupFolder() const
{
    jassert (isGroup());
    File f;

    for (int i = 0; i < getNumChildren(); ++i)
    {
        f = getChild(i).getFile();

        if (f.exists())
            return f.getParentDirectory();
    }

    Item parent (getParent());
    if (parent != *this)
    {
        f = parent.determineGroupFolder();

        if (f.getChildFile (getName()).isDirectory())
            f = f.getChildFile (getName());
    }
    else
    {
        f = project.getFile().getParentDirectory();

        if (f.getChildFile ("Source").isDirectory())
            f = f.getChildFile ("Source");
    }

    return f;
}

void Project::Item::initialiseMissingProperties()
{
    if (! state.hasProperty (ComponentBuilder::idProperty))
        setID (createAlphaNumericUID());

    if (isFile())
    {
        state.setProperty (Ids::name, getFile().getFileName(), 0);
    }
    else if (isGroup())
    {
        for (int i = getNumChildren(); --i >= 0;)
            getChild(i).initialiseMissingProperties();
    }
}

Value Project::Item::getNameValue()
{
    return state.getPropertyAsValue (Ids::name, getUndoManager());
}

String Project::Item::getName() const
{
    return state [Ids::name];
}

void Project::Item::addChild (const Item& newChild, int insertIndex)
{
    state.addChild (newChild.state, insertIndex, getUndoManager());
}

void Project::Item::removeItemFromProject()
{
    state.getParent().removeChild (state, getUndoManager());
}

Project::Item Project::Item::getParent() const
{
    if (isMainGroup() || ! isGroup())
        return *this;

    return Item (project, state.getParent());
}

struct ItemSorter
{
    static int compareElements (const ValueTree& first, const ValueTree& second)
    {
        return first [Ids::name].toString().compareIgnoreCase (second [Ids::name].toString());
    }
};

struct ItemSorterWithGroupsAtStart
{
    static int compareElements (const ValueTree& first, const ValueTree& second)
    {
        const bool firstIsGroup = first.hasType (Tags::group);
        const bool secondIsGroup = second.hasType (Tags::group);

        if (firstIsGroup == secondIsGroup)
            return first [Ids::name].toString().compareIgnoreCase (second [Ids::name].toString());
        else
            return firstIsGroup ? -1 : 1;
    }
};

void Project::Item::sortAlphabetically (bool keepGroupsAtStart)
{
    if (keepGroupsAtStart)
    {
        ItemSorterWithGroupsAtStart sorter;
        state.sort (sorter, getUndoManager(), true);
    }
    else
    {
        ItemSorter sorter;
        state.sort (sorter, getUndoManager(), true);
    }
}

Project::Item Project::Item::getOrCreateSubGroup (const String& name)
{
    for (int i = state.getNumChildren(); --i >= 0;)
    {
        const ValueTree child (state.getChild (i));
        if (child.getProperty (Ids::name) == name && child.hasType (Tags::group))
            return Item (project, child);
    }

    return addNewSubGroup (name, -1);
}

Project::Item Project::Item::addNewSubGroup (const String& name, int insertIndex)
{
    String newID (createGUID (getID() + name + String (getNumChildren())));

    int n = 0;
    while (findItemWithID (newID).isValid())
        newID = createGUID (newID + String (++n));

    Item group (createGroup (project, name, newID));

    jassert (canContain (group));
    addChild (group, insertIndex);
    return group;
}

bool Project::Item::addFile (const File& file, int insertIndex, const bool shouldCompile)
{
    if (file == File::nonexistent || file.isHidden() || file.getFileName().startsWithChar ('.'))
        return false;

    if (file.isDirectory())
    {
        Item group (addNewSubGroup (file.getFileNameWithoutExtension(), insertIndex));

        DirectoryIterator iter (file, false, "*", File::findFilesAndDirectories);
        while (iter.next())
        {
            if (! project.getMainGroup().findItemForFile (iter.getFile()).isValid())
                group.addFile (iter.getFile(), -1, shouldCompile);
        }

        group.sortAlphabetically (false);
    }
    else if (file.existsAsFile())
    {
        if (! project.getMainGroup().findItemForFile (file).isValid())
            addFileUnchecked (file, insertIndex, shouldCompile);
    }
    else
    {
        jassertfalse;
    }

    return true;
}

void Project::Item::addFileUnchecked (const File& file, int insertIndex, const bool shouldCompile)
{
    Item item (project, ValueTree (Tags::file));
    item.initialiseMissingProperties();
    item.getNameValue() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile && file.hasFileExtension ("cpp;mm;c;m;cc;cxx;r");
    item.getShouldAddToResourceValue() = project.shouldBeAddedToBinaryResourcesByDefault (file);

    if (canContain (item))
    {
        item.setFile (file);
        addChild (item, insertIndex);
    }
}

bool Project::Item::addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile)
{
    Item item (project, ValueTree (Tags::file));
    item.initialiseMissingProperties();
    item.getNameValue() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile;
    item.getShouldAddToResourceValue() = project.shouldBeAddedToBinaryResourcesByDefault (file);

    if (canContain (item))
    {
        item.setFile (file);
        addChild (item, insertIndex);
        return true;
    }

    return false;
}

const Drawable* Project::Item::getIcon() const
{
    if (isFile())
    {
        if (isImageFile())
            return StoredSettings::getInstance()->getImageFileIcon();

        return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage();
    }
    else if (isMainGroup())
    {
        return &(project.mainProjectIcon);
    }

    return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage();
}

//==============================================================================
ValueTree Project::getConfigNode()
{
    return projectRoot.getOrCreateChildWithName (Tags::configGroup, nullptr);
}

const char* const Project::configFlagDefault = "default";
const char* const Project::configFlagEnabled = "enabled";
const char* const Project::configFlagDisabled = "disabled";

Value Project::getConfigFlag (const String& name)
{
    ValueTree configNode (getConfigNode());
    Value v (configNode.getPropertyAsValue (name, getUndoManagerFor (configNode)));

    if (v.getValue().toString().isEmpty())
        v = configFlagDefault;

    return v;
}

bool Project::isConfigFlagEnabled (const String& name) const
{
    return projectRoot.getChildWithName (Tags::configGroup).getProperty (name) == configFlagEnabled;
}

void Project::sanitiseConfigFlags()
{
    ValueTree configNode (getConfigNode());

    for (int i = configNode.getNumProperties(); --i >= 0;)
    {
        const var value (configNode [configNode.getPropertyName(i)]);

        if (value != configFlagEnabled && value != configFlagDisabled)
            configNode.removeProperty (configNode.getPropertyName(i), getUndoManagerFor (configNode));
    }
}

//==============================================================================
ValueTree Project::getModulesNode()
{
    return projectRoot.getOrCreateChildWithName (Tags::modulesGroup, nullptr);
}

bool Project::isModuleEnabled (const String& moduleID) const
{
    ValueTree modules (projectRoot.getChildWithName (Tags::modulesGroup));

    for (int i = 0; i < modules.getNumChildren(); ++i)
        if (modules.getChild(i) [ComponentBuilder::idProperty] == moduleID)
            return true;

    return false;
}

Value Project::shouldShowAllModuleFilesInProject (const String& moduleID)
{
    return getModulesNode().getChildWithProperty (ComponentBuilder::idProperty, moduleID)
                           .getPropertyAsValue (Ids::showAllCode, getUndoManagerFor (getModulesNode()));
}

Value Project::shouldCopyModuleFilesLocally (const String& moduleID)
{
    return getModulesNode().getChildWithProperty (ComponentBuilder::idProperty, moduleID)
                           .getPropertyAsValue (Ids::useLocalCopy, getUndoManagerFor (getModulesNode()));
}

void Project::addModule (const String& moduleID, bool shouldCopyFilesLocally)
{
    if (! isModuleEnabled (moduleID))
    {
        ValueTree module (Tags::module);
        module.setProperty (ComponentBuilder::idProperty, moduleID, nullptr);

        ValueTree modules (getModulesNode());
        modules.addChild (module, -1, getUndoManagerFor (modules));

        shouldShowAllModuleFilesInProject (moduleID) = true;
    }

    if (shouldCopyFilesLocally)
        shouldCopyModuleFilesLocally (moduleID) = true;
}

void Project::removeModule (const String& moduleID)
{
    ValueTree modules (getModulesNode());

    for (int i = 0; i < modules.getNumChildren(); ++i)
        if (modules.getChild(i) [ComponentBuilder::idProperty] == moduleID)
            modules.removeChild (i, getUndoManagerFor (modules));
}

void Project::createRequiredModules (const ModuleList& availableModules, OwnedArray<LibraryModule>& modules) const
{
    for (int i = 0; i < availableModules.modules.size(); ++i)
        if (isModuleEnabled (availableModules.modules.getUnchecked(i)->uid))
            modules.add (availableModules.modules.getUnchecked(i)->create());
}

int Project::getNumModules() const
{
    return projectRoot.getChildWithName (Tags::modulesGroup).getNumChildren();
}

String Project::getModuleID (int index) const
{
    return projectRoot.getChildWithName (Tags::modulesGroup).getChild (index) [ComponentBuilder::idProperty].toString();
}

//==============================================================================
ValueTree Project::getExporters()
{
    return projectRoot.getOrCreateChildWithName (Tags::exporters, nullptr);
}

int Project::getNumExporters()
{
    return getExporters().getNumChildren();
}

ProjectExporter* Project::createExporter (int index)
{
    jassert (index >= 0 && index < getNumExporters());
    return ProjectExporter::createExporter (*this, getExporters().getChild (index));
}

void Project::addNewExporter (const String& exporterName)
{
    ScopedPointer<ProjectExporter> exp (ProjectExporter::createNewExporter (*this, exporterName));

    ValueTree exporters (getExporters());
    exporters.addChild (exp->settings, -1, getUndoManagerFor (exporters));
}

void Project::createDefaultExporters()
{
    ValueTree exporters (getExporters());
    exporters.removeAllChildren (getUndoManagerFor (exporters));

    const StringArray exporterNames (ProjectExporter::getDefaultExporters());

    for (int i = 0; i < exporterNames.size(); ++i)
        addNewExporter (exporterNames[i]);
}

//==============================================================================
String Project::getFileTemplate (const String& templateName)
{
    int dataSize;
    const char* data = BinaryData::getNamedResource (templateName.toUTF8(), dataSize);

    if (data == nullptr)
    {
        jassertfalse;
        return String::empty;
    }

    return String::fromUTF8 (data, dataSize);
}

//==============================================================================
Project::ExporterIterator::ExporterIterator (Project& project_) : index (-1), project (project_) {}
Project::ExporterIterator::~ExporterIterator() {}

bool Project::ExporterIterator::next()
{
    if (++index >= project.getNumExporters())
        return false;

    exporter = project.createExporter (index);

    if (exporter == nullptr)
    {
        jassertfalse; // corrupted project file?
        return next();
    }

    return true;
}
