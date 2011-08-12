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
#include "jucer_ProjectExporter.h"
#include "jucer_ResourceFile.h"
#include "jucer_ProjectSaver.h"
#include "../Application/jucer_OpenDocumentManager.h"


//==============================================================================
namespace Tags
{
    const Identifier projectRoot       ("JUCERPROJECT");
    const Identifier projectMainGroup  ("MAINGROUP");
    const Identifier group             ("GROUP");
    const Identifier file              ("FILE");
    const Identifier configurations    ("CONFIGURATIONS");
    const Identifier configuration     ("CONFIGURATION");
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
    getMainGroup().getName() = newTitle;
}

const String Project::getDocumentTitle()
{
    return getProjectName().toString();
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty (Ids::jucerVersion, ProjectInfo::versionString, 0);
    projectRoot.setProperty (Ids::name, getDocumentTitle(), 0);
}

void Project::setMissingDefaultValues()
{
    if (! projectRoot.hasProperty (Ids::id_))
        projectRoot.setProperty (Ids::id_, createAlphaNumericUID(), nullptr);

    // Create main file group if missing
    if (! projectRoot.getChildWithName (Tags::projectMainGroup).isValid())
    {
        Item mainGroup (*this, ValueTree (Tags::projectMainGroup));
        projectRoot.addChild (mainGroup.getNode(), 0, 0);
    }

    getMainGroup().initialiseNodeValues();

    if (getDocumentTitle().isEmpty())
        setTitle ("Juce Project");

    if (! projectRoot.hasProperty (Ids::projectType))
        getProjectTypeValue() = ProjectType::getGUIAppTypeName();

    if (! projectRoot.hasProperty (Ids::version))
        getVersion() = "1.0.0";

    // Create configs group
    if (! projectRoot.getChildWithName (Tags::configurations).isValid())
    {
        projectRoot.addChild (ValueTree (Tags::configurations), 0, 0);
        createDefaultConfigs();
    }

    if (! projectRoot.getChildWithName (Tags::exporters).isValid())
        createDefaultExporters();

    getProjectType().setMissingProjectProperties (*this);

    if (! projectRoot.hasProperty (Ids::bundleIdentifier))
        setBundleIdentifierToDefault();

    if (! projectRoot.getChildWithName (Tags::modulesGroup).isValid())
    {
        addModule ("juce_core");

        if (! isConfigFlagEnabled ("JUCE_ONLY_BUILD_CORE_LIBRARY"))
        {
            addModule ("juce_events");
            addModule ("juce_graphics");
            addModule ("juce_data_structures");
            addModule ("juce_gui_basics");
            addModule ("juce_gui_extra");
            addModule ("juce_gui_audio");
            addModule ("juce_cryptography");
            addModule ("juce_video");
            addModule ("juce_opengl");
            addModule ("juce_audio_basics");
            addModule ("juce_audio_devices");
            addModule ("juce_audio_formats");
            addModule ("juce_audio_processors");
        }
    }
}

//==============================================================================
const String Project::loadDocument (const File& file)
{
    ScopedPointer <XmlElement> xml (XmlDocument::parse (file));

    if (xml == nullptr || ! xml->hasTagName (Tags::projectRoot.toString()))
        return "Not a valid Jucer project!";

    ValueTree newTree (ValueTree::fromXml (*xml));

    if (! newTree.hasType (Tags::projectRoot))
        return "The document contains errors and couldn't be parsed!";

    StoredSettings::getInstance()->recentFiles.addFile (file);
    StoredSettings::getInstance()->flush();
    projectRoot = newTree;

    setMissingDefaultValues();

    return String::empty;
}

const String Project::saveDocument (const File& file)
{
    updateProjectSettings();
    sanitiseConfigFlags();

    {
        ScopedPointer <ProjectExporter> exp (ProjectExporter::createPlatformDefaultExporter (*this));

        if (exp != nullptr)
        {
            File f (resolveFilename (exp->getJuceFolder().toString()));

            if (FileHelpers::isJuceFolder (f))
                StoredSettings::getInstance()->setLastKnownJuceFolder (f.getFullPathName());
        }
    }

    StoredSettings::getInstance()->recentFiles.addFile (file);

    ProjectSaver saver (*this, file);
    return saver.save();
}

//==============================================================================
File Project::lastDocumentOpened;

const File Project::getLastDocumentOpened()
{
    return lastDocumentOpened;
}

void Project::setLastDocumentOpened (const File& file)
{
    lastDocumentOpened = file;
}

//==============================================================================
void Project::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (property == Ids::projectType)
        setMissingDefaultValues();

    changed();
}

void Project::valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
{
    changed();
}

void Project::valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved)
{
    changed();
}

void Project::valueTreeChildOrderChanged (ValueTree& parentTree)
{
    changed();
}

void Project::valueTreeParentChanged (ValueTree& tree)
{
}

//==============================================================================
File Project::resolveFilename (String filename) const
{
    if (filename.isEmpty())
        return File::nonexistent;

    filename = replacePreprocessorDefs (getPreprocessorDefs(), filename)
                .replaceCharacter ('\\', '/');

    if (File::isAbsolutePath (filename))
        return File (filename);

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
        filename = file.getRelativePathFrom (relativePathBase);
    }

    return filename;
}

//==============================================================================
const ProjectType& Project::getProjectType() const
{
    const ProjectType* type = ProjectType::findType (getProjectTypeValue().toString());
    jassert (type != nullptr);

    if (type == nullptr)
    {
        type = ProjectType::findType (ProjectType::getGUIAppTypeName());
        jassert (type != nullptr);
    }

    return *type;
}

//==============================================================================
void Project::createPropertyEditors (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getProjectName(), "Project Name", 256, false));
    props.getLast()->setTooltip ("The name of the project.");

    props.add (new TextPropertyComponent (getVersion(), "Project Version", 16, false));
    props.getLast()->setTooltip ("The project's version number, This should be in the format major.minor.point");

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

    props.add (new TextPropertyComponent (getBundleIdentifier(), "Bundle Identifier", 256, false));
    props.getLast()->setTooltip ("A unique identifier for this product, mainly for use in Mac builds. It should be something like 'com.yourcompanyname.yourproductname'");

    {
        OwnedArray<Project::Item> images;
        findAllImageItems (images);

        StringArray choices;
        Array<var> ids;

        choices.add ("<None>");
        ids.add (var::null);
        choices.add (String::empty);
        ids.add (var::null);

        for (int i = 0; i < images.size(); ++i)
        {
            choices.add (images.getUnchecked(i)->getName().toString());
            ids.add (images.getUnchecked(i)->getID());
        }

        props.add (new ChoicePropertyComponent (getSmallIconImageItemID(), "Icon (small)", choices, ids));
        props.getLast()->setTooltip ("Sets an icon to use for the executable.");

        props.add (new ChoicePropertyComponent (getBigIconImageItemID(), "Icon (large)", choices, ids));
        props.getLast()->setTooltip ("Sets an icon to use for the executable.");
    }

    getProjectType().createPropertyEditors(*this, props);

    props.add (new TextPropertyComponent (getProjectPreprocessorDefs(), "Preprocessor definitions", 32768, false));
    props.getLast()->setTooltip ("Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    for (int i = props.size(); --i >= 0;)
        props.getUnchecked(i)->setPreferredHeight (22);
}

String Project::getVersionAsHex() const
{
    StringArray configs;
    configs.addTokens (getVersion().toString(), ",.", String::empty);
    configs.trim();
    configs.removeEmptyStrings();

    int value = (configs[0].getIntValue() << 16) + (configs[1].getIntValue() << 8) + configs[2].getIntValue();

    if (configs.size() >= 4)
        value = (value << 8) + configs[3].getIntValue();

    return "0x" + String::toHexString (value);
}

Image Project::getBigIcon()
{
    Item icon (getMainGroup().findItemWithID (getBigIconImageItemID().toString()));

    if (icon.isValid())
        return ImageCache::getFromFile (icon.getFile());

    return Image::null;
}

Image Project::getSmallIcon()
{
    Item icon (getMainGroup().findItemWithID (getSmallIconImageItemID().toString()));

    if (icon.isValid())
        return ImageCache::getFromFile (icon.getFile());

    return Image::null;
}

StringPairArray Project::getPreprocessorDefs() const
{
    return parsePreprocessorDefs (getProjectPreprocessorDefs().toString());
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
Project::Item::Item (Project& project_, const ValueTree& node_)
    : project (&project_), node (node_)
{
}

Project::Item::Item (const Item& other)
    : project (other.project), node (other.node)
{
}

Project::Item& Project::Item::operator= (const Project::Item& other)
{
    project = other.project;
    node = other.node;
    return *this;
}

Project::Item::~Item()
{
}

Project::Item Project::Item::createCopy()         { Item i (*this); i.node = i.node.createCopy(); return i; }

String Project::Item::getID() const               { return node [Ids::id_]; }
void Project::Item::setID (const String& newID)   { node.setProperty (Ids::id_, newID, nullptr); }

String Project::Item::getImageFileID() const      { return "id:" + getID(); }

Project::Item Project::Item::createGroup (Project& project, const String& name, const String& uid)
{
    Item group (project, ValueTree (Tags::group));
    group.setID (uid);
    group.initialiseNodeValues();
    group.getName() = name;
    return group;
}

bool Project::Item::isFile() const          { return node.hasType (Tags::file); }
bool Project::Item::isGroup() const         { return node.hasType (Tags::group) || isMainGroup(); }
bool Project::Item::isMainGroup() const     { return node.hasType (Tags::projectMainGroup); }
bool Project::Item::isImageFile() const     { return isFile() && getFile().hasFileExtension ("png;jpg;jpeg;gif;drawable"); }

Project::Item Project::Item::findItemWithID (const String& targetId) const
{
    if (node [Ids::id_] == targetId)
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

    return Item (*project, ValueTree::invalid);
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

bool Project::Item::shouldBeAddedToTargetProject() const
{
    return isFile();
}

bool Project::Item::shouldBeCompiled() const        { return getShouldCompileValue().getValue(); }
Value Project::Item::getShouldCompileValue() const  { return node.getPropertyAsValue (Ids::compile, getUndoManager()); }

bool Project::Item::shouldBeAddedToBinaryResources() const  { return getShouldAddToResourceValue().getValue(); }
Value Project::Item::getShouldAddToResourceValue() const    { return node.getPropertyAsValue (Ids::resource, getUndoManager()); }

Value Project::Item::getShouldInhibitWarningsValue() const  { return node.getPropertyAsValue (Ids::noWarnings, getUndoManager()); }
Value Project::Item::getShouldUseStdCallValue() const       { return node.getPropertyAsValue (Ids::useStdCall, nullptr); }


String Project::Item::getFilePath() const
{
    if (isFile())
        return node [Ids::file].toString();
    else
        return String::empty;
}

File Project::Item::getFile() const
{
    if (isFile())
        return getProject().resolveFilename (node [Ids::file].toString());
    else
        return File::nonexistent;
}

void Project::Item::setFile (const File& file)
{
    setFile (RelativePath (getProject().getRelativePathForFile (file), RelativePath::projectFolder));
    jassert (getFile() == file);
}

void Project::Item::setFile (const RelativePath& file)
{
    jassert (file.getRoot() == RelativePath::projectFolder);
    jassert (isFile());
    node.setProperty (Ids::file, file.toUnixStyle(), getUndoManager());
    node.setProperty (Ids::name, file.getFileName(), getUndoManager());
}

bool Project::Item::renameFile (const File& newFile)
{
    const File oldFile (getFile());

    if (oldFile.moveFileTo (newFile))
    {
        setFile (newFile);
        OpenDocumentManager::getInstance()->fileHasBeenRenamed (oldFile, newFile);
        return true;
    }

    return false;
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

    return Item (getProject(), ValueTree::invalid);
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

        if (f.getChildFile (getName().toString()).isDirectory())
            f = f.getChildFile (getName().toString());
    }
    else
    {
        f = getProject().getFile().getParentDirectory();

        if (f.getChildFile ("Source").isDirectory())
            f = f.getChildFile ("Source");
    }

    return f;
}

void Project::Item::initialiseNodeValues()
{
    if (! node.hasProperty (Ids::id_))
        setID (createAlphaNumericUID());

    if (isFile())
    {
        node.setProperty (Ids::name, getFile().getFileName(), 0);
    }
    else if (isGroup())
    {
        for (int i = getNumChildren(); --i >= 0;)
            getChild(i).initialiseNodeValues();
    }
}

Value Project::Item::getName() const
{
    return node.getPropertyAsValue (Ids::name, getUndoManager());
}

void Project::Item::addChild (const Item& newChild, int insertIndex)
{
    node.addChild (newChild.getNode(), insertIndex, getUndoManager());
}

void Project::Item::removeItemFromProject()
{
    node.getParent().removeChild (node, getUndoManager());
}

Project::Item Project::Item::getParent() const
{
    if (isMainGroup() || ! isGroup())
        return *this;

    return Item (getProject(), node.getParent());
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
        node.sort (sorter, getUndoManager(), true);
    }
    else
    {
        ItemSorter sorter;
        node.sort (sorter, getUndoManager(), true);
    }
}

Project::Item Project::Item::getOrCreateSubGroup (const String& name)
{
    for (int i = node.getNumChildren(); --i >= 0;)
    {
        const ValueTree child (node.getChild (i));
        if (child.getProperty (Ids::name) == name && child.hasType (Tags::group))
            return Item (getProject(), child);
    }

    return addNewSubGroup (name, -1);
}

Project::Item Project::Item::addNewSubGroup (const String& name, int insertIndex)
{
    Item group (createGroup (getProject(), name, createGUID (getID() + name + String (getNumChildren()))));

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
            if (! getProject().getMainGroup().findItemForFile (iter.getFile()).isValid())
                group.addFile (iter.getFile(), -1, shouldCompile);
        }

        group.sortAlphabetically (false);
    }
    else if (file.existsAsFile())
    {
        if (! getProject().getMainGroup().findItemForFile (file).isValid())
        {
            Item item (getProject(), ValueTree (Tags::file));
            item.initialiseNodeValues();
            item.getName() = file.getFileName();
            item.getShouldCompileValue() = shouldCompile && file.hasFileExtension ("cpp;mm;c;m;cc;cxx;r");
            item.getShouldAddToResourceValue() = getProject().shouldBeAddedToBinaryResourcesByDefault (file);

            if (canContain (item))
            {
                item.setFile (file);
                addChild (item, insertIndex);
            }
        }
    }
    else
    {
        jassertfalse;
    }

    return true;
}

bool Project::Item::addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile)
{
    Item item (getProject(), ValueTree (Tags::file));
    item.initialiseNodeValues();
    item.getName() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile;
    item.getShouldAddToResourceValue() = getProject().shouldBeAddedToBinaryResourcesByDefault (file);

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
        return &(getProject().mainProjectIcon);
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
    const ValueTree configNode (getConfigNode());
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
        if (modules.getChild(i) [Ids::id_] == moduleID)
            return true;

    return false;
}

Value Project::shouldShowAllModuleFilesInProject (const String& moduleID)
{
    return getModulesNode().getChildWithProperty (Ids::id_, moduleID)
                           .getPropertyAsValue (Ids::showAllCode, getUndoManagerFor (getModulesNode()));
}

void Project::addModule (const String& moduleID)
{
    if (! isModuleEnabled (moduleID))
    {
        ValueTree module (Tags::module);
        module.setProperty (Ids::id_, moduleID, nullptr);

        ValueTree modules (getModulesNode());
        modules.addChild (module, -1, getUndoManagerFor (modules));

        shouldShowAllModuleFilesInProject (moduleID) = true;
    }
}

void Project::removeModule (const String& moduleID)
{
    ValueTree modules (getModulesNode());

    for (int i = 0; i < modules.getNumChildren(); ++i)
        if (modules.getChild(i) [Ids::id_] == moduleID)
            modules.removeChild (i, getUndoManagerFor (modules));
}

//==============================================================================
ValueTree Project::getConfigurations() const
{
    return projectRoot.getChildWithName (Tags::configurations);
}

int Project::getNumConfigurations() const
{
    return getConfigurations().getNumChildren();
}

Project::BuildConfiguration Project::getConfiguration (int index)
{
    jassert (index < getConfigurations().getNumChildren());
    return BuildConfiguration (this, getConfigurations().getChild (index));
}

bool Project::hasConfigurationNamed (const String& name) const
{
    const ValueTree configs (getConfigurations());
    for (int i = configs.getNumChildren(); --i >= 0;)
        if (configs.getChild(i) [Ids::name].toString() == name)
            return true;

    return false;
}

String Project::getUniqueConfigName (String name) const
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

void Project::addNewConfiguration (BuildConfiguration* configToCopy)
{
    const String configName (getUniqueConfigName (configToCopy != nullptr ? configToCopy->config [Ids::name].toString()
                                                                          : "New Build Configuration"));

    ValueTree configs (getConfigurations());

    if (! configs.isValid())
    {
        projectRoot.addChild (ValueTree (Tags::configurations), 0, getUndoManagerFor (projectRoot));
        configs = getConfigurations();
    }

    ValueTree newConfig (Tags::configuration);
    if (configToCopy != nullptr)
        newConfig = configToCopy->config.createCopy();

    newConfig.setProperty (Ids::name, configName, 0);

    configs.addChild (newConfig, -1, getUndoManagerFor (configs));
}

void Project::deleteConfiguration (int index)
{
    ValueTree configs (getConfigurations());
    configs.removeChild (index, getUndoManagerFor (getConfigurations()));
}

void Project::createDefaultConfigs()
{
    for (int i = 0; i < 2; ++i)
    {
        addNewConfiguration (nullptr);
        BuildConfiguration config = getConfiguration (i);

        const bool debugConfig = i == 0;

        config.getName() = debugConfig ? "Debug" : "Release";
        config.isDebug() = debugConfig;
        config.getOptimisationLevel() = debugConfig ? 1 : 2;
        config.getTargetBinaryName() = getProjectFilenameRoot();
    }
}

//==============================================================================
Project::BuildConfiguration::BuildConfiguration (Project* project_, const ValueTree& configNode)
   : project (project_),
     config (configNode)
{
}

Project::BuildConfiguration::BuildConfiguration (const BuildConfiguration& other)
   : project (other.project),
     config (other.config)
{
}

const Project::BuildConfiguration& Project::BuildConfiguration::operator= (const BuildConfiguration& other)
{
    project = other.project;
    config = other.config;
    return *this;
}

Project::BuildConfiguration::~BuildConfiguration()
{
}

String Project::BuildConfiguration::getGCCOptimisationFlag() const
{
    const int level = (int) getOptimisationLevel().getValue();
    return String (level <= 1 ? "0" : (level == 2 ? "s" : "3"));
}

const char* const Project::BuildConfiguration::osxVersionDefault = "default";
const char* const Project::BuildConfiguration::osxVersion10_4    = "10.4 SDK";
const char* const Project::BuildConfiguration::osxVersion10_5    = "10.5 SDK";
const char* const Project::BuildConfiguration::osxVersion10_6    = "10.6 SDK";

const char* const Project::BuildConfiguration::osxArch_Default        = "default";
const char* const Project::BuildConfiguration::osxArch_Native         = "Native";
const char* const Project::BuildConfiguration::osxArch_32BitUniversal = "32BitUniversal";
const char* const Project::BuildConfiguration::osxArch_64BitUniversal = "64BitUniversal";
const char* const Project::BuildConfiguration::osxArch_64Bit          = "64BitIntel";

void Project::BuildConfiguration::createPropertyEditors (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getName(), "Name", 96, false));
    props.getLast()->setTooltip ("The name of this configuration.");

    props.add (new BooleanPropertyComponent (isDebug(), "Debug mode", "Debugging enabled"));
    props.getLast()->setTooltip ("If enabled, this means that the configuration should be built with debug synbols.");

    const char* optimisationLevels[] = { "No optimisation", "Optimise for size and speed", "Optimise for maximum speed", 0 };
    const int optimisationLevelValues[] = { 1, 2, 3, 0 };
    props.add (new ChoicePropertyComponent (getOptimisationLevel(), "Optimisation", StringArray (optimisationLevels), Array<var> (optimisationLevelValues)));
    props.getLast()->setTooltip ("The optimisation level for this configuration");

    props.add (new TextPropertyComponent (getTargetBinaryName(), "Binary name", 256, false));
    props.getLast()->setTooltip ("The filename to use for the destination binary executable file. Don't add a suffix to this, because platform-specific suffixes will be added for each target platform.");

    props.add (new TextPropertyComponent (getTargetBinaryRelativePath(), "Binary location", 1024, false));
    props.getLast()->setTooltip ("The folder in which the finished binary should be placed. Leave this blank to cause the binary to be placed in its default location in the build folder.");

    props.add (new TextPropertyComponent (getHeaderSearchPath(), "Header search path", 16384, false));
    props.getLast()->setTooltip ("Extra header search paths. Use semi-colons to separate multiple paths.");

    props.add (new TextPropertyComponent (getBuildConfigPreprocessorDefs(), "Preprocessor definitions", 32768, false));
    props.getLast()->setTooltip ("Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    if (getMacSDKVersion().toString().isEmpty())
        getMacSDKVersion() = osxVersionDefault;

    const char* osxVersions[] = { "Use Default", osxVersion10_4, osxVersion10_5, osxVersion10_6, 0 };
    const char* osxVersionValues[] = { osxVersionDefault, osxVersion10_4, osxVersion10_5, osxVersion10_6, 0 };

    props.add (new ChoicePropertyComponent (getMacSDKVersion(), "OSX Base SDK Version", StringArray (osxVersions), Array<var> (osxVersionValues)));
    props.getLast()->setTooltip ("The version of OSX to link against in the XCode build.");

    if (getMacCompatibilityVersion().toString().isEmpty())
        getMacCompatibilityVersion() = osxVersionDefault;

    props.add (new ChoicePropertyComponent (getMacCompatibilityVersion(), "OSX Compatibility Version", StringArray (osxVersions), Array<var> (osxVersionValues)));
    props.getLast()->setTooltip ("The minimum version of OSX that the target binary will be compatible with.");

    const char* osxArch[] = { "Use Default", "Native architecture of build machine", "Universal Binary (32-bit)", "Universal Binary (64-bit)", "64-bit Intel", 0 };
    const char* osxArchValues[] = { osxArch_Default, osxArch_Native, osxArch_32BitUniversal, osxArch_64BitUniversal, osxArch_64Bit, 0 };

    if (getMacArchitecture().toString().isEmpty())
        getMacArchitecture() = osxArch_Default;

    props.add (new ChoicePropertyComponent (getMacArchitecture(), "OSX Architecture", StringArray (osxArch), Array<var> (osxArchValues)));
    props.getLast()->setTooltip ("The type of OSX binary that will be produced.");

    for (int i = props.size(); --i >= 0;)
        props.getUnchecked(i)->setPreferredHeight (22);
}

StringPairArray Project::BuildConfiguration::getAllPreprocessorDefs() const
{
    return mergePreprocessorDefs (project->getPreprocessorDefs(),
                                  parsePreprocessorDefs (getBuildConfigPreprocessorDefs().toString()));
}

StringArray Project::BuildConfiguration::getHeaderSearchPaths() const
{
    StringArray s;
    s.addTokens (getHeaderSearchPath().toString(), ";", String::empty);
    return s;
}

//==============================================================================
ValueTree Project::getExporters()
{
    ValueTree exporters (projectRoot.getChildWithName (Tags::exporters));

    if (! exporters.isValid())
    {
        projectRoot.addChild (ValueTree (Tags::exporters), 0, getUndoManagerFor (projectRoot));
        exporters = getExporters();
    }

    return exporters;
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
    exporters.addChild (exp->getSettings(), -1, getUndoManagerFor (exporters));
}

void Project::deleteExporter (int index)
{
    ValueTree exporters (getExporters());
    exporters.removeChild (index, getUndoManagerFor (exporters));
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
void Project::resaveJucerFile (const File& file)
{
    if (! file.exists())
    {
        std::cout << "The file " << file.getFullPathName() << " doesn't exist!" << std::endl;
        return;
    }

    if (! file.hasFileExtension (Project::projectFileExtension))
    {
        std::cout << file.getFullPathName() << " isn't a valid jucer project file!" << std::endl;
        return;
    }

    Project newDoc (file);

    if (! newDoc.loadFrom (file, true))
    {
        std::cout << "Failed to load the project file: " << file.getFullPathName() << std::endl;
        return;
    }

    std::cout << "The Introjucer - Re-saving file: " << file.getFullPathName() << std::endl;
    String error (newDoc.saveDocument (file));

    if (error.isNotEmpty())
    {
        std::cout << "Error when writing project: " << error << std::endl;
        return;
    }
}
