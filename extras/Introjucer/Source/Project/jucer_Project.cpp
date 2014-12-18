/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "jucer_Project.h"
#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "../Application/jucer_OpenDocumentManager.h"
#include "../Application/jucer_Application.h"

//==============================================================================
Project::Project (const File& f)
    : FileBasedDocument (projectFileExtension,
                         String ("*") + projectFileExtension,
                         "Choose a Jucer project to load",
                         "Save Jucer project"),
      projectRoot (Ids::JUCERPROJECT)
{
    Logger::writeToLog ("Loading project: " + f.getFullPathName());
    setFile (f);
    removeDefunctExporters();
    updateOldModulePaths();
    setMissingDefaultValues();

    setChangedFlag (false);

    projectRoot.addListener (this);
}

Project::~Project()
{
    projectRoot.removeListener (this);
    IntrojucerApp::getApp().openDocumentManager.closeAllDocumentsUsingProject (*this, false);
}

const char* Project::projectFileExtension = ".jucer";

//==============================================================================
void Project::setTitle (const String& newTitle)
{
    projectRoot.setProperty (Ids::name, newTitle, getUndoManagerFor (projectRoot));
    getMainGroup().getNameValue() = newTitle;
}

String Project::getTitle() const
{
    return projectRoot.getChildWithName (Ids::MAINGROUP) [Ids::name];
}

String Project::getDocumentTitle()
{
    return getTitle();
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty (Ids::jucerVersion, ProjectInfo::versionString, nullptr);
    projectRoot.setProperty (Ids::name, getDocumentTitle(), nullptr);
}

void Project::setMissingDefaultValues()
{
    if (! projectRoot.hasProperty (Ids::ID))
        projectRoot.setProperty (Ids::ID, createAlphaNumericUID(), nullptr);

    // Create main file group if missing
    if (! projectRoot.getChildWithName (Ids::MAINGROUP).isValid())
    {
        Item mainGroup (*this, ValueTree (Ids::MAINGROUP));
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

    getModules().sortAlphabetically();

    if (getBundleIdentifier().toString().isEmpty())
        getBundleIdentifier() = getDefaultBundleIdentifier();

    if (shouldIncludeBinaryInAppConfig() == var::null)
        shouldIncludeBinaryInAppConfig() = true;

    IntrojucerApp::getApp().updateNewlyOpenedProject (*this);
}

void Project::updateOldStyleConfigList()
{
    ValueTree deprecatedConfigsList (projectRoot.getChildWithName (Ids::CONFIGURATIONS));

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
    ValueTree exporters (projectRoot.getChildWithName (Ids::EXPORTFORMATS));

    for (;;)
    {
        ValueTree oldVC6Exporter (exporters.getChildWithName ("MSVC6"));

        if (oldVC6Exporter.isValid())
            exporters.removeChild (oldVC6Exporter, nullptr);
        else
            break;
    }
}

void Project::updateOldModulePaths()
{
    for (Project::ExporterIterator exporter (*this); exporter.next();)
        exporter->updateOldModulePaths();
}

//==============================================================================
static int getVersionElement (StringRef v, int index)
{
    StringArray parts;
    parts.addTokens (v, "., ", StringRef());

    return parts [parts.size() - index - 1].getIntValue();
}

static int getJuceVersion (const String& v)
{
    return getVersionElement (v, 2) * 100000
         + getVersionElement (v, 1) * 1000
         + getVersionElement (v, 0);
}

static int getBuiltJuceVersion()
{
    return JUCE_MAJOR_VERSION * 100000
         + JUCE_MINOR_VERSION * 1000
         + JUCE_BUILDNUMBER;
}

static bool isAnyModuleNewerThanIntrojucer (const OwnedArray<ModuleDescription>& modules)
{
    for (int i = modules.size(); --i >= 0;)
    {
        const ModuleDescription* m = modules.getUnchecked(i);

        if (m->getID().startsWith ("juce_")
              && getJuceVersion (m->getVersion()) > getBuiltJuceVersion())
            return true;
    }

    return false;
}

void Project::warnAboutOldIntrojucerVersion()
{
    ModuleList available;
    available.scanAllKnownFolders (*this);

    if (isAnyModuleNewerThanIntrojucer (available.modules))
    {
        if (IntrojucerApp::getApp().isRunningCommandLine)
            std::cout <<  "WARNING! This version of the introjucer is out-of-date!" << std::endl;
        else
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Introjucer",
                                              "This version of the introjucer is out-of-date!"
                                              "\n\n"
                                              "Always make sure that you're running the very latest version, "
                                              "preferably compiled directly from the JUCE repository that you're working with!");
    }
}

//==============================================================================
static File lastDocumentOpened;

File Project::getLastDocumentOpened()                   { return lastDocumentOpened; }
void Project::setLastDocumentOpened (const File& file)  { lastDocumentOpened = file; }

static void registerRecentFile (const File& file)
{
    RecentlyOpenedFilesList::registerRecentFileNatively (file);
    getAppSettings().recentFiles.addFile (file);
    getAppSettings().flush();
}

//==============================================================================
Result Project::loadDocument (const File& file)
{
    ScopedPointer <XmlElement> xml (XmlDocument::parse (file));

    if (xml == nullptr || ! xml->hasTagName (Ids::JUCERPROJECT.toString()))
        return Result::fail ("Not a valid Jucer project!");

    ValueTree newTree (ValueTree::fromXml (*xml));

    if (! newTree.hasType (Ids::JUCERPROJECT))
        return Result::fail ("The document contains errors and couldn't be parsed!");

    registerRecentFile (file);
    enabledModulesList = nullptr;
    projectRoot = newTree;

    removeDefunctExporters();
    setMissingDefaultValues();
    updateOldModulePaths();
    setChangedFlag (false);
    warnAboutOldIntrojucerVersion();

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
void Project::valueTreePropertyChanged (ValueTree&, const Identifier& property)
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

    filename = replacePreprocessorDefs (getPreprocessorDefs(), filename);

    if (FileHelpers::isAbsolutePath (filename))
        return File::createFileWithoutCheckingPath (FileHelpers::currentOSStylePath (filename)); // (avoid assertions for windows-style paths)

    return getFile().getSiblingFile (FileHelpers::currentOSStylePath (filename));
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
    if (const ProjectType* type = ProjectType::findType (getProjectTypeString()))
        return *type;

    const ProjectType* guiType = ProjectType::findType (ProjectType::getGUIAppTypeName());
    jassert (guiType != nullptr);
    return *guiType;
}

//==============================================================================
void Project::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (getProjectNameValue(), "Project Name", 256, false),
               "The name of the project.");

    props.add (new TextPropertyComponent (getVersionValue(), "Project Version", 16, false),
               "The project's version number, This should be in the format major.minor.point[.point]");

    props.add (new TextPropertyComponent (getCompanyName(), "Company Name", 256, false),
               "Your company name, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (getCompanyWebsite(), "Company Website", 256, false),
               "Your company website, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (getCompanyEmail(), "Company E-mail", 256, false),
               "Your company e-mail, which will be added to the properties of the binary where possible");

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
               "A unique identifier for this product, mainly for use in OSX/iOS builds. It should be something like 'com.yourcompanyname.yourproductname'");

    getProjectType().createPropertyEditors (*this, props);

    {
        const int maxSizes[] = { 20480, 10240, 6144, 2048, 1024, 512, 256, 128, 64 };

        StringArray maxSizeNames;
        Array<var> maxSizeCodes;

        maxSizeNames.add (TRANS("Default"));
        maxSizeCodes.add (var::null);

        maxSizeNames.add (String::empty);
        maxSizeCodes.add (var::null);

        for (int i = 0; i < numElementsInArray (maxSizes); ++i)
        {
            const int sizeInBytes = maxSizes[i] * 1024;
            maxSizeNames.add (File::descriptionOfSizeInBytes (sizeInBytes));
            maxSizeCodes.add (sizeInBytes);
        }

        props.add (new ChoicePropertyComponent (getMaxBinaryFileSize(), "BinaryData.cpp size limit", maxSizeNames, maxSizeCodes),
                   "When splitting binary data into multiple cpp files, the Introjucer attempts to keep the file sizes below this threshold. "
                   "(Note that individual resource files which are larger than this size cannot be split across multiple cpp files).");
    }

    props.add (new BooleanPropertyComponent (shouldIncludeBinaryInAppConfig(), "Include Binary",
                                             "Include BinaryData.h in the AppConfig.h file"));

    props.add (new TextPropertyComponent (getProjectPreprocessorDefs(), "Preprocessor definitions", 32768, true),
               "Global preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, or "
               "new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new TextPropertyComponent (getProjectUserNotes(), "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

//==============================================================================
static StringArray getVersionSegments (const Project& p)
{
    StringArray segments;
    segments.addTokens (p.getVersionString(), ",.", "");
    segments.trim();
    segments.removeEmptyStrings();
    return segments;
}

int Project::getVersionAsHexInteger() const
{
    const StringArray segments (getVersionSegments (*this));

    int value = (segments[0].getIntValue() << 16)
                 + (segments[1].getIntValue() << 8)
                  + segments[2].getIntValue();

    if (segments.size() >= 4)
        value = (value << 8) + segments[3].getIntValue();

    return value;
}

String Project::getVersionAsHex() const
{
    return "0x" + String::toHexString (getVersionAsHexInteger());
}

StringPairArray Project::getPreprocessorDefs() const
{
    return parsePreprocessorDefs (projectRoot [Ids::defines]);
}

File Project::getBinaryDataCppFile (int index) const
{
    const File cpp (getGeneratedCodeFolder().getChildFile ("BinaryData.cpp"));

    if (index > 0)
        return cpp.getSiblingFile (cpp.getFileNameWithoutExtension() + String (index + 1))
                    .withFileExtension (cpp.getFileExtension());

    return cpp;
}

Project::Item Project::getMainGroup()
{
    return Item (*this, projectRoot.getChildWithName (Ids::MAINGROUP));
}

PropertiesFile& Project::getStoredProperties() const
{
    return getAppSettings().getProjectProperties (getProjectUID());
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
Project::Item::Item (Project& p, const ValueTree& s)
    : project (p), state (s)
{
}

Project::Item::Item (const Item& other)
    : project (other.project), state (other.state)
{
}

Project::Item Project::Item::createCopy()         { Item i (*this); i.state = i.state.createCopy(); return i; }

String Project::Item::getID() const               { return state [Ids::ID]; }
void Project::Item::setID (const String& newID)   { state.setProperty (Ids::ID, newID, nullptr); }

Drawable* Project::Item::loadAsImageFile() const
{
    return isValid() ? Drawable::createFromImageFile (getFile())
                     : nullptr;
}

Project::Item Project::Item::createGroup (Project& project, const String& name, const String& uid)
{
    Item group (project, ValueTree (Ids::GROUP));
    group.setID (uid);
    group.initialiseMissingProperties();
    group.getNameValue() = name;
    return group;
}

bool Project::Item::isFile() const          { return state.hasType (Ids::FILE); }
bool Project::Item::isGroup() const         { return state.hasType (Ids::GROUP) || isMainGroup(); }
bool Project::Item::isMainGroup() const     { return state.hasType (Ids::MAINGROUP); }

bool Project::Item::isImageFile() const
{
    return isFile() && (ImageFileFormat::findImageFormatForFileExtension (getFile()) != nullptr
                          || getFile().hasFileExtension ("svg"));
}

Project::Item Project::Item::findItemWithID (const String& targetId) const
{
    if (state [Ids::ID] == targetId)
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

    jassertfalse;
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

    return String::empty;
}

File Project::Item::getFile() const
{
    if (isFile())
        return project.resolveFilename (state [Ids::file].toString());

    return File::nonexistent;
}

void Project::Item::setFile (const File& file)
{
    setFile (RelativePath (project.getRelativePathForFile (file), RelativePath::projectFolder));
    jassert (getFile() == file);
}

void Project::Item::setFile (const RelativePath& file)
{
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
        IntrojucerApp::getApp().openDocumentManager.fileHasBeenRenamed (oldFile, newFile);
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
        f = project.getProjectFolder();

        if (f.getChildFile ("Source").isDirectory())
            f = f.getChildFile ("Source");
    }

    return f;
}

void Project::Item::initialiseMissingProperties()
{
    if (! state.hasProperty (Ids::ID))
        setID (createAlphaNumericUID());

    if (isFile())
    {
        state.setProperty (Ids::name, getFile().getFileName(), nullptr);
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
        return first [Ids::name].toString().compareNatural (second [Ids::name].toString());
    }
};

struct ItemSorterWithGroupsAtStart
{
    static int compareElements (const ValueTree& first, const ValueTree& second)
    {
        const bool firstIsGroup = first.hasType (Ids::GROUP);
        const bool secondIsGroup = second.hasType (Ids::GROUP);

        if (firstIsGroup == secondIsGroup)
            return first [Ids::name].toString().compareNatural (second [Ids::name].toString());

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
        if (child.getProperty (Ids::name) == name && child.hasType (Ids::GROUP))
            return Item (project, child);
    }

    return addNewSubGroup (name, -1);
}

Project::Item Project::Item::addNewSubGroup (const String& name, int insertIndex)
{
    String newID (createGUID (getID() + name + String (getNumChildren())));

    int n = 0;
    while (project.getMainGroup().findItemWithID (newID).isValid())
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
        Item group (addNewSubGroup (file.getFileName(), insertIndex));

        for (DirectoryIterator iter (file, false, "*", File::findFilesAndDirectories); iter.next();)
            if (! project.getMainGroup().findItemForFile (iter.getFile()).isValid())
                group.addFile (iter.getFile(), -1, shouldCompile);

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
    Item item (project, ValueTree (Ids::FILE));
    item.initialiseMissingProperties();
    item.getNameValue() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile && file.hasFileExtension (fileTypesToCompileByDefault);
    item.getShouldAddToResourceValue() = project.shouldBeAddedToBinaryResourcesByDefault (file);

    if (canContain (item))
    {
        item.setFile (file);
        addChild (item, insertIndex);
    }
}

bool Project::Item::addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile)
{
    Item item (project, ValueTree (Ids::FILE));
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

Icon Project::Item::getIcon() const
{
    const Icons& icons = getIcons();

    if (isFile())
    {
        if (isImageFile())
            return Icon (icons.imageDoc, Colours::blue);

        return Icon (icons.document, Colours::yellow);
    }

    if (isMainGroup())
        return Icon (icons.juceLogo, Colours::orange);

    return Icon (icons.folder, Colours::darkgrey);
}

bool Project::Item::isIconCrossedOut() const
{
    return isFile()
            && ! (shouldBeCompiled()
                   || shouldBeAddedToBinaryResources()
                   || getFile().hasFileExtension (headerFileExtensions));
}

//==============================================================================
ValueTree Project::getConfigNode()
{
    return projectRoot.getOrCreateChildWithName (Ids::JUCEOPTIONS, nullptr);
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
    return projectRoot.getChildWithName (Ids::JUCEOPTIONS).getProperty (name) == configFlagEnabled;
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
EnabledModuleList& Project::getModules()
{
    if (enabledModulesList == nullptr)
        enabledModulesList = new EnabledModuleList (*this, projectRoot.getOrCreateChildWithName (Ids::MODULES, nullptr));

    return *enabledModulesList;
}

//==============================================================================
ValueTree Project::getExporters()
{
    return projectRoot.getOrCreateChildWithName (Ids::EXPORTFORMATS, nullptr);
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

void Project::createExporterForCurrentPlatform()
{
    addNewExporter (ProjectExporter::getCurrentPlatformExporterName());
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
Project::ExporterIterator::ExporterIterator (Project& p) : index (-1), project (p) {}
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
