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

#include "jucer_Project.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ResourceFile.h"
#include "jucer_ProjectSaver.h"
#include "../../ui/jucer_OpenDocumentManager.h"


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
    projectRoot.setProperty ("name", newTitle, getUndoManagerFor (projectRoot));
    getMainGroup().getName() = newTitle;
}

const String Project::getDocumentTitle()
{
    return getProjectName().toString();
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty ("jucerVersion", ProjectInfo::versionString, 0);
    projectRoot.setProperty ("name", getDocumentTitle(), 0);
}

void Project::setMissingDefaultValues()
{
    if (! projectRoot.hasProperty ("id"))
        projectRoot.setProperty ("id", createAlphaNumericUID(), 0);

    // Create main file group if missing
    if (! projectRoot.getChildWithName (Tags::projectMainGroup).isValid())
    {
        Item mainGroup (*this, ValueTree (Tags::projectMainGroup));
        projectRoot.addChild (mainGroup.getNode(), 0, 0);
    }

    getMainGroup().initialiseNodeValues();

    if (getDocumentTitle().isEmpty())
        setTitle ("Juce Project");

    if (! projectRoot.hasProperty ("projectType"))
        getProjectType() = (int) application;

    if (! projectRoot.hasProperty ("version"))
        getVersion() = "1.0.0";

    if (! projectRoot.hasProperty ("juceLinkage"))
        getJuceLinkageModeValue() = (int) useAmalgamatedJuceViaMultipleTemplates;

    const String juceFolderPath (getRelativePathForFile (StoredSettings::getInstance()->getLastKnownJuceFolder()));

    // Create configs group
    if (! projectRoot.getChildWithName (Tags::configurations).isValid())
    {
        projectRoot.addChild (ValueTree (Tags::configurations), 0, 0);
        createDefaultConfigs();
    }

    if (! projectRoot.getChildWithName (Tags::exporters).isValid())
        createDefaultExporters();

    const String sanitisedProjectName (CodeHelpers::makeValidIdentifier (getProjectName().toString(), false, true, false));

    if (! projectRoot.hasProperty ("buildVST"))
    {
        shouldBuildVST() = true;
        shouldBuildRTAS() = false;
        shouldBuildAU() = true;

        getPluginName() = getProjectName().toString();
        getPluginDesc() = getProjectName().toString();
        getPluginManufacturer() = "yourcompany";
        getPluginManufacturerCode() = "abcd";
        getPluginCode() = "Abcd";
        getPluginChannelConfigs() = "{1, 1}, {2, 2}";
        getPluginIsSynth() = false;
        getPluginWantsMidiInput() = false;
        getPluginProducesMidiOut() = false;
        getPluginSilenceInProducesSilenceOut() = false;
        getPluginTailLengthSeconds() = 0;
        getPluginEditorNeedsKeyFocus() = false;
        getPluginAUExportPrefix() = sanitisedProjectName + "AU";
        getPluginAUCocoaViewClassName() = sanitisedProjectName + "AU_V1";
        getPluginRTASCategory() = String::empty;
    }

    if (! projectRoot.hasProperty ("bundleIdentifier"))
        setBundleIdentifierToDefault();
}

//==============================================================================
const String Project::loadDocument (const File& file)
{
    XmlDocument doc (file);
    ScopedPointer <XmlElement> xml (doc.getDocumentElement());

    if (xml == 0 || ! xml->hasTagName (Tags::projectRoot.toString()))
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

    {
        // (getting these forces the values to be sanitised)
        OwnedArray <Project::JuceConfigFlag> flags;
        getJuceConfigFlags (flags);
    }

    if (FileHelpers::isJuceFolder (getLocalJuceFolder()))
        StoredSettings::getInstance()->setLastKnownJuceFolder (getLocalJuceFolder().getFullPathName());

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
    if (isLibrary())
        getJuceLinkageModeValue() = (int) notLinkedToJuce;

    changed();
}

void Project::valueTreeChildrenChanged (ValueTree& tree)
{
    changed();
}

void Project::valueTreeParentChanged (ValueTree& tree)
{
}

//==============================================================================
const File Project::resolveFilename (const String& filename) const
{
    if (filename.isEmpty())
        return File::nonexistent;

    if (File::isAbsolutePath (filename))
        return File (filename);

    return getFile().getSiblingFile (filename);
}

const String Project::getRelativePathForFile (const File& file) const
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
bool Project::shouldBeAddedToBinaryResourcesByDefault (const File& file)
{
    return ! file.hasFileExtension (sourceFileExtensions);
}

Value Project::getProjectType() const
{
    static const char* mappings[] = { "guiapp", "1", "consoleapp", "2", "audioplug", "3", "library", "4", "browserplug", "5", 0 };
    return Value (new ValueRemapperSource (projectRoot.getPropertyAsValue ("projectType", getUndoManagerFor (projectRoot)), mappings));
}

const StringArray Project::getProjectTypes() const
{
    const char* types[] = { "Application (GUI)",
                            "Application (Non-GUI)",
                            "Audio Plug-in",
                            //"Browser Plug-in",
                            "Static Library",
                            0 };

    return StringArray (types);
}

Value Project::getJuceLinkageModeValue() const
{
    static const char* mappings[] = { "none", "1", "static", "2", "amalg_big", "3", "amalg_template", "4", "amalg_multi", "5", 0 };
    return Value (new ValueRemapperSource (projectRoot.getPropertyAsValue ("juceLinkage", getUndoManagerFor (projectRoot)), mappings));
}

const StringArray Project::getJuceLinkageModes() const
{
    const char* types[] = { "Not linked to Juce",
                            "Linked to Juce Static Library",
                            "Include Juce Amalgamated Files",
                            "Include Juce Source Code Directly (In a single file)",
                            "Include Juce Source Code Directly (Split across several files)",
                            0 };

    return StringArray (types);
}

const File Project::getLocalJuceFolder()
{
    ScopedPointer <ProjectExporter> exp (ProjectExporter::createPlatformDefaultExporter (*this));

    if (exp != 0)
    {
        File f (resolveFilename (exp->getJuceFolder().toString()));

        if (FileHelpers::isJuceFolder (f))
            return f;
    }

    return StoredSettings::getInstance()->getLastKnownJuceFolder();
}

//==============================================================================
void Project::createPropertyEditors (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getProjectName(), "Project Name", 256, false));
    props.getLast()->setTooltip ("The name of the project.");

    props.add (new TextPropertyComponent (getVersion(), "Project Version", 16, false));
    props.getLast()->setTooltip ("The project's version number, This should be in the format major.minor.point");

    props.add (new ChoicePropertyComponent (getProjectType(), "Project Type", getProjectTypes()));

    props.add (new ChoicePropertyComponent (getJuceLinkageModeValue(), "Juce Linkage Method", getJuceLinkageModes()));
    props.getLast()->setTooltip ("The method by which your project will be linked to Juce.");

    props.add (new TextPropertyComponent (getBundleIdentifier(), "Bundle Identifier", 256, false));
    props.getLast()->setTooltip ("A unique identifier for this product, mainly for use in Mac builds. It should be something like 'com.yourcompanyname.yourproductname'");

    if (isAudioPlugin())
    {
        props.add (new BooleanPropertyComponent (shouldBuildVST(), "Build VST", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce a VST plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildAU(), "Build AudioUnit", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an AudioUnit plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildRTAS(), "Build RTAS", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an RTAS plugin.");
    }

    if (isAudioPlugin())
    {
        props.add (new TextPropertyComponent (getPluginName(), "Plugin Name", 128, false));
        props.getLast()->setTooltip ("The name of your plugin (keep it short!)");
        props.add (new TextPropertyComponent (getPluginDesc(), "Plugin Description", 256, false));
        props.getLast()->setTooltip ("A short description of your plugin.");

        props.add (new TextPropertyComponent (getPluginManufacturer(), "Plugin Manufacturer", 256, false));
        props.getLast()->setTooltip ("The name of your company (cannot be blank).");
        props.add (new TextPropertyComponent (getPluginManufacturerCode(), "Plugin Manufacturer Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your company.");
        props.add (new TextPropertyComponent (getPluginCode(), "Plugin Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your plugin. Note that for AU compatibility, this must contain at least one upper-case letter!");

        props.add (new TextPropertyComponent (getPluginChannelConfigs(), "Plugin Channel Configurations", 256, false));
        props.getLast()->setTooltip ("This is the set of input/output channel configurations that your plugin can handle.  The list is a comma-separated set of pairs of values in the form { numInputs, numOutputs }, and each "
                                     "pair indicates a valid configuration that the plugin can handle. So for example, {1, 1}, {2, 2} means that the plugin can be used in just two configurations: either with 1 input "
                                     "and 1 output, or with 2 inputs and 2 outputs.");

        props.add (new BooleanPropertyComponent (getPluginIsSynth(), "Plugin is a Synth", "Is a Synth"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to be treated as a synth or generator. It doesn't make much difference to the plugin itself, but some hosts treat synths differently to other plugins.");

        props.add (new BooleanPropertyComponent (getPluginWantsMidiInput(), "Plugin Midi Input", "Plugin wants midi input"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to accept midi messages.");

        props.add (new BooleanPropertyComponent (getPluginProducesMidiOut(), "Plugin Midi Output", "Plugin produces midi output"));
        props.getLast()->setTooltip ("Enable this if your plugin is going to produce midi messages.");

        props.add (new BooleanPropertyComponent (getPluginSilenceInProducesSilenceOut(), "Silence", "Silence in produces silence out"));
        props.getLast()->setTooltip ("Enable this if your plugin has no tail - i.e. if passing a silent buffer to it will always result in a silent buffer being produced.");

        props.add (new TextPropertyComponent (getPluginTailLengthSeconds(), "Tail Length (in seconds)", 12, false));
        props.getLast()->setTooltip ("This indicates the length, in seconds, of the plugin's tail. This information may or may not be used by the host.");

        props.add (new BooleanPropertyComponent (getPluginEditorNeedsKeyFocus(), "Key Focus", "Plugin editor requires keyboard focus"));
        props.getLast()->setTooltip ("Enable this if your plugin needs keyboard input - some hosts can be a bit funny about keyboard focus..");

        props.add (new TextPropertyComponent (getPluginAUExportPrefix(), "Plugin AU Export Prefix", 64, false));
        props.getLast()->setTooltip ("A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");

        props.add (new TextPropertyComponent (getPluginAUCocoaViewClassName(), "Plugin AU Cocoa View Name", 64, false));
        props.getLast()->setTooltip ("In an AU, this is the name of Cocoa class that creates the UI. Some hosts bizarrely display the class-name, so you might want to make it reflect your plugin. But the name must be "
                                     "UNIQUE to this exact version of your plugin, to avoid objective-C linkage mix-ups that happen when different plugins containing the same class-name are loaded simultaneously.");

        props.add (new TextPropertyComponent (getPluginRTASCategory(), "Plugin RTAS Category", 64, false));
        props.getLast()->setTooltip ("(Leave this blank if your plugin is a synth). This is one of the RTAS categories from FicPluginEnums.h, such as: ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics, "
                                     "ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay, "
                                     "ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction, "
                                     "ePlugInCategory_Dither, ePlugInCategory_SoundField");
    }

    for (int i = props.size(); --i >= 0;)
        props.getUnchecked(i)->setPreferredHeight (22);
}

//==============================================================================
Project::Item Project::getMainGroup()
{
    return Item (*this, projectRoot.getChildWithName (Tags::projectMainGroup));
}

Project::Item Project::createNewGroup()
{
    Item item (*this, ValueTree (Tags::group));
    item.initialiseNodeValues();
    item.getName() = "New Group";
    return item;
}

Project::Item Project::createNewItem (const File& file)
{
    Item item (*this, ValueTree (Tags::file));
    item.initialiseNodeValues();
    item.getName() = file.getFileName();
    item.getShouldCompileValue() = file.hasFileExtension ("cpp;mm;c;m");
    item.getShouldAddToResourceValue() = shouldBeAddedToBinaryResourcesByDefault (file);
    return item;
}

//==============================================================================
Project::Item::Item (Project& project_, const ValueTree& node_)
    : project (project_), node (node_)
{
}

Project::Item::Item (const Item& other)
    : project (other.project), node (other.node)
{
}

Project::Item::~Item()
{
}

const String Project::Item::getID() const  { return node ["id"]; }

bool Project::Item::isFile() const         { return node.hasType (Tags::file); }
bool Project::Item::isGroup() const        { return node.hasType (Tags::group) || isMainGroup(); }
bool Project::Item::isMainGroup() const    { return node.hasType (Tags::projectMainGroup); }

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

bool Project::Item::shouldBeCompiled() const
{
    return getShouldCompileValue().getValue();
}

Value Project::Item::getShouldCompileValue() const
{
    return node.getPropertyAsValue ("compile", getUndoManager());
}

bool Project::Item::shouldBeAddedToBinaryResources() const
{
    return getShouldAddToResourceValue().getValue();
}

Value Project::Item::getShouldAddToResourceValue() const
{
    return node.getPropertyAsValue ("resource", getUndoManager());
}

const File Project::Item::getFile() const
{
    if (isFile())
        return project.resolveFilename (node ["file"].toString());
    else
        return File::nonexistent;
}

void Project::Item::setFile (const File& file)
{
    jassert (isFile());
    node.setProperty ("file", project.getRelativePathForFile (file), getUndoManager());
    node.setProperty ("name", file.getFileName(), getUndoManager());

    jassert (getFile() == file);
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

    return Item (project, ValueTree::invalid);
}

const File Project::Item::determineGroupFolder() const
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
        f = project.getFile().getParentDirectory();

        if (f.getChildFile ("Source").isDirectory())
            f = f.getChildFile ("Source");
    }

    return f;
}

void Project::Item::initialiseNodeValues()
{
    if (! node.hasProperty ("id"))
        node.setProperty ("id", createAlphaNumericUID(), 0);

    if (isFile())
    {
        node.setProperty ("name", getFile().getFileName(), 0);
    }
    else if (isGroup())
    {
        for (int i = getNumChildren(); --i >= 0;)
            getChild(i).initialiseNodeValues();
    }
}

Value Project::Item::getName() const
{
    return node.getPropertyAsValue ("name", getUndoManager());
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

    return Item (project, node.getParent());
}

struct ItemSorter
{
    static int compareElements (const ValueTree& first, const ValueTree& second)
    {
        return first["name"].toString().compareIgnoreCase (second["name"].toString());
    }
};

void Project::Item::sortAlphabetically()
{
    ItemSorter sorter;
    node.sort (sorter);
}

bool Project::Item::addFile (const File& file, int insertIndex)
{
    if (file == File::nonexistent || file.isHidden() || file.getFileName().startsWithChar ('.'))
        return false;

    if (file.isDirectory())
    {
        Item group (project.createNewGroup());
        group.getName() = file.getFileNameWithoutExtension();

        jassert (canContain (group));

        addChild (group, insertIndex);
        //group.setFile (file);

        DirectoryIterator iter (file, false, "*", File::findFilesAndDirectories);
        while (iter.next())
            group.addFile (iter.getFile(), -1);

        group.sortAlphabetically();
    }
    else if (file.existsAsFile())
    {
        Item item (project.createNewItem (file));

        if (canContain (item))
        {
            item.setFile (file);
            addChild (item, insertIndex);
        }
    }
    else
    {
        jassertfalse;
    }

    return true;
}

Image* Project::Item::getIcon() const
{
    if (isFile())
        return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage();
    else if (isMainGroup())
        return ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize);
    else
        return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage();
}

//==============================================================================
ValueTree Project::getJuceConfigNode()
{
    ValueTree configNode = projectRoot.getChildWithName ("JUCEOPTIONS");

    if (! configNode.isValid())
    {
        configNode = ValueTree ("JUCEOPTIONS");
        projectRoot.addChild (configNode, -1, 0);
    }

    return configNode;
}

void Project::getJuceConfigFlags (OwnedArray <JuceConfigFlag>& flags)
{
    ValueTree configNode (getJuceConfigNode());

    File juceConfigH (getLocalJuceFolder().getChildFile ("juce_Config.h"));
    StringArray lines;
    lines.addLines (juceConfigH.loadFileAsString());

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i].trim());

        if (line.startsWith ("/** ") && line.containsChar (':'))
        {
            ScopedPointer <JuceConfigFlag> config (new JuceConfigFlag());
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
                config->value.referTo (getJuceConfigFlag (config->symbol));
                flags.add (config.release());
            }
        }
    }
}

Value Project::getJuceConfigFlag (const String& name)
{
    static const char* valueRemappings[] = { "enabled", "1", "disabled", "2", "default", "3", 0 };

    ValueTree configNode (getJuceConfigNode());
    Value v (new ValueRemapperSource (configNode.getPropertyAsValue (name, getUndoManagerFor (configNode)),
                                      valueRemappings));

    if ((int) v.getValue() == 0)
        v = 3;

    return v;
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
        if (configs.getChild(i) ["name"].toString() == name)
            return true;

    return false;
}

const String Project::getUniqueConfigName (String name) const
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
    const String configName (getUniqueConfigName (configToCopy != 0 ? configToCopy->config ["name"].toString()
                                                                    : "New Build Configuration"));

    ValueTree configs (getConfigurations());

    if (! configs.isValid())
    {
        projectRoot.addChild (ValueTree (Tags::configurations), 0, getUndoManagerFor (projectRoot));
        configs = getConfigurations();
    }

    ValueTree newConfig (Tags::configuration);
    if (configToCopy != 0)
        newConfig = configToCopy->config.createCopy();

    newConfig.setProperty ("name", configName, 0);

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
        addNewConfiguration (0);
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

const String Project::BuildConfiguration::getGCCOptimisationFlag() const
{
    const int level = (int) getOptimisationLevel().getValue();
    return String (level <= 1 ? "0" : (level == 2 ? "s" : "3"));
}

static const char* osxSDKs[]        = { "Use default", "10.4 SDK", "10.5 SDK", "10.6 SDK", 0 };
static const char* osxSDKMappings[] = { "default", "1", "10.4 SDK", "2", "10.5 SDK", "3", "10.6 SDK", "4", "10.7 SDK", "5", 0 };

void Project::BuildConfiguration::createPropertyEditors (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getName(), "Name", 96, false));
    props.getLast()->setTooltip ("The name of this configuration.");

    props.add (new BooleanPropertyComponent (isDebug(), "Debug mode", "Debugging enabled"));
    props.getLast()->setTooltip ("If enabled, this means that the configuration should be built with debug synbols.");

    const char* optimisationLevels[] = { "No optimisation", "Optimise for size and speed", "Optimise for maximum speed", 0 };
    props.add (new ChoicePropertyComponent (getOptimisationLevel(), "Optimisation", StringArray (optimisationLevels)));
    props.getLast()->setTooltip ("The optimisation level for this configuration");

    props.add (new TextPropertyComponent (getTargetBinaryName(), "Binary name", 256, false));
    props.getLast()->setTooltip ("The filename to use for the destination binary executable file. Don't add a suffix to this, because platform-specific suffixes will be added for each target platform.");

    props.add (new TextPropertyComponent (getTargetBinaryRelativePath(), "Binary location", 1024, false));
    props.getLast()->setTooltip ("The folder in which the finished binary should be placed. Leave this blank to cause the binary to be placed in its default location in the build folder.");

    props.add (new TextPropertyComponent (getHeaderSearchPath(), "Header search path", 16384, false));
    props.getLast()->setTooltip ("Extra header search paths. Use semi-colons to separate multiple paths.");

    props.add (new TextPropertyComponent (getPreprocessorDefs(), "Preprocessor definitions", 32768, false));
    props.getLast()->setTooltip ("Extra preprocessor definitions. Use whitespace or commas as a delimiter.");

    if ((int) getMacSDKVersion().getValue() == 0)
        getMacSDKVersion() = 1;

    props.add (new ChoicePropertyComponent (getMacSDKVersion(), "OSX Base SDK Version", StringArray (osxSDKs)));
    props.getLast()->setTooltip ("The version of OSX to link against in the XCode build.");

    if ((int) getMacCompatibilityVersion().getValue() == 0)
        getMacCompatibilityVersion() = 1;

    props.add (new ChoicePropertyComponent (getMacCompatibilityVersion(), "OSX Compatibility Version", StringArray (osxSDKs)));
    props.getLast()->setTooltip ("The minimum version of OSX that the target binary will be compatible with.");

    for (int i = props.size(); --i >= 0;)
        props.getUnchecked(i)->setPreferredHeight (22);
}

const StringArray Project::BuildConfiguration::parsePreprocessorDefs() const
{
    StringArray defines;
    defines.addTokens (getPreprocessorDefs().toString(), " ,;", String::empty);
    defines.removeEmptyStrings (true);
    return defines;
}

const StringArray Project::BuildConfiguration::getHeaderSearchPaths() const
{
    StringArray s;
    s.addTokens (getHeaderSearchPath().toString(), ";", String::empty);
    return s;
}

Value Project::BuildConfiguration::getMacSDKVersion() const
{
    return Value (new ValueRemapperSource (config.getPropertyAsValue ("osxSDK", getUndoManager()), osxSDKMappings));
}

Value Project::BuildConfiguration::getMacCompatibilityVersion() const
{
    return Value (new ValueRemapperSource (config.getPropertyAsValue ("osxCompatibility", getUndoManager()), osxSDKMappings));
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

void Project::addNewExporter (int exporterIndex)
{
    ScopedPointer<ProjectExporter> exp (ProjectExporter::createNewExporter (*this, exporterIndex));

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

    for (int i = 0; i < ProjectExporter::getNumExporters(); ++i)
        addNewExporter (i);
}

//==============================================================================
const String Project::getFileTemplate (const String& templateName)
{
    int dataSize;
    const char* data = BinaryData::getNamedResource (templateName.toUTF8(), dataSize);

    if (data == 0)
    {
        jassertfalse;
        return String::empty;
    }

    return String::fromUTF8 (data, dataSize);
}
