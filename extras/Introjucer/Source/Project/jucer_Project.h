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

#ifndef __JUCER_PROJECT_JUCEHEADER__
#define __JUCER_PROJECT_JUCEHEADER__

#include "../jucer_Headers.h"
class ProjectExporter;
class ProjectType;
class ModuleList;
class LibraryModule;

//==============================================================================
class Project  : public FileBasedDocument,
                 public ValueTree::Listener
{
public:
    //==============================================================================
    Project (const File& file);
    ~Project();

    //==============================================================================
    // FileBasedDocument stuff..
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);

    void setTitle (const String& newTitle);

    //==============================================================================
    ValueTree getProjectRoot() const                    { return projectRoot; }
    Value getProjectName()                              { return getMainGroup().getName(); }
    String getProjectFilenameRoot()                     { return File::createLegalFileName (getDocumentTitle()); }
    String getProjectUID() const                        { return projectRoot [ComponentBuilder::idProperty]; }

    //==============================================================================
    template <class FileType>
    bool shouldBeAddedToBinaryResourcesByDefault (const FileType& file)
    {
        return ! file.hasFileExtension (sourceOrHeaderFileExtensions);
    }

    File resolveFilename (String filename) const;
    String getRelativePathForFile (const File& file) const;

    //==============================================================================
    // Creates editors for the project settings
    void createPropertyEditors (Array <PropertyComponent*>& properties);

    //==============================================================================
    // project types
    const ProjectType& getProjectType() const;
    Value getProjectTypeValue() const                   { return getProjectValue ("projectType"); }

    Value getVersion() const                            { return getProjectValue ("version"); }
    String getVersionAsHex() const;
    Value getBundleIdentifier() const                   { return getProjectValue (Ids::bundleIdentifier); }
    void setBundleIdentifierToDefault()                 { getBundleIdentifier() = "com.yourcompany." + CodeHelpers::makeValidIdentifier (getProjectName().toString(), false, true, false); }

    //==============================================================================
    Value getProjectValue (const Identifier& name) const  { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }

    Value getProjectPreprocessorDefs() const            { return getProjectValue (Ids::defines); }
    StringPairArray getPreprocessorDefs() const;

    Value getBigIconImageItemID() const                 { return getProjectValue ("bigIcon"); }
    Value getSmallIconImageItemID() const               { return getProjectValue ("smallIcon"); }
    Image getBigIcon();
    Image getSmallIcon();

    //==============================================================================
    File getAppIncludeFile() const                      { return getGeneratedCodeFolder().getChildFile (getJuceSourceHFilename()); }
    File getGeneratedCodeFolder() const                 { return getFile().getSiblingFile ("JuceLibraryCode"); }

    //==============================================================================
    String getAmalgamatedHeaderFileName() const         { return "juce_amalgamated.h"; }
    String getAmalgamatedMMFileName() const             { return "juce_amalgamated.mm"; }
    String getAmalgamatedCppFileName() const            { return "juce_amalgamated.cpp"; }

    String getAppConfigFilename() const                 { return "AppConfig.h"; }
    String getJuceSourceFilenameRoot() const            { return "JuceLibraryCode"; }
    int getNumSeparateAmalgamatedFiles() const          { return 4; }
    String getJuceSourceHFilename() const               { return "JuceHeader.h"; }

    //==============================================================================
    class Item
    {
    public:
        //==============================================================================
        Item (Project& project, const ValueTree& itemNode);
        Item (const Item& other);

        static Item createGroup (Project& project, const String& name, const String& uid);
        void initialiseMissingProperties();

        //==============================================================================
        bool isValid() const                            { return state.isValid(); }
        bool operator== (const Item& other) const       { return state == other.state && &project == &other.project; }
        bool operator!= (const Item& other) const       { return ! operator== (other); }

        //==============================================================================
        bool isFile() const;
        bool isGroup() const;
        bool isMainGroup() const;
        bool isImageFile() const;

        String getID() const;
        void setID (const String& newID);
        Item findItemWithID (const String& targetId) const; // (recursive search)

        String getImageFileID() const;
        Image loadAsImageFile() const;

        //==============================================================================
        Value getName() const;
        File getFile() const;
        String getFilePath() const;
        void setFile (const File& file);
        void setFile (const RelativePath& file);
        File determineGroupFolder() const;
        bool renameFile (const File& newFile);

        bool shouldBeAddedToTargetProject() const;
        bool shouldBeCompiled() const;
        Value getShouldCompileValue() const;
        bool shouldBeAddedToBinaryResources() const;
        Value getShouldAddToResourceValue() const;
        Value getShouldInhibitWarningsValue() const;
        Value getShouldUseStdCallValue() const;

        //==============================================================================
        bool canContain (const Item& child) const;
        int getNumChildren() const                      { return state.getNumChildren(); }
        Item getChild (int index) const                 { return Item (project, state.getChild (index)); }

        Item addNewSubGroup (const String& name, int insertIndex);
        Item getOrCreateSubGroup (const String& name);
        void addChild (const Item& newChild, int insertIndex);
        bool addFile (const File& file, int insertIndex, bool shouldCompile);
        void addFileUnchecked (const File& file, int insertIndex, bool shouldCompile);
        bool addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile);
        void removeItemFromProject();
        void sortAlphabetically (bool keepGroupsAtStart);
        Item findItemForFile (const File& file) const;
        bool containsChildForFile (const RelativePath& file) const;

        Item getParent() const;
        Item createCopy();

        UndoManager* getUndoManager() const              { return project.getUndoManagerFor (state); }

        const Drawable* getIcon() const;

        Project& project;
        ValueTree state;

    private:
        Item& operator= (const Item&);
    };

    Item getMainGroup();

    void findAllImageItems (OwnedArray<Item>& items);

    //==============================================================================
    class BuildConfiguration
    {
    public:
        BuildConfiguration (const BuildConfiguration&);
        const BuildConfiguration& operator= (const BuildConfiguration&);
        ~BuildConfiguration();

        //==============================================================================
        Project& getProject() const                         { return *project; }

        void createPropertyEditors (Array <PropertyComponent*>& properties);

        //==============================================================================
        Value getName() const                               { return getValue (Ids::name); }
        Value isDebug() const                               { return getValue (Ids::isDebug); }
        Value getTargetBinaryName() const                   { return getValue (Ids::targetName); }
        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath() const           { return getValue (Ids::binaryPath); }
        Value getOptimisationLevel() const                  { return getValue (Ids::optimisation); }
        String getGCCOptimisationFlag() const;
        Value getBuildConfigPreprocessorDefs() const        { return getValue (Ids::defines); }
        StringPairArray getAllPreprocessorDefs() const; // includes inherited definitions
        Value getHeaderSearchPath() const                   { return getValue (Ids::headerPath); }
        StringArray getHeaderSearchPaths() const;

        static const char* const osxVersionDefault;
        static const char* const osxVersion10_4;
        static const char* const osxVersion10_5;
        static const char* const osxVersion10_6;
        Value getMacSDKVersion() const                      { return getValue (Ids::osxSDK); }
        Value getMacCompatibilityVersion() const            { return getValue (Ids::osxCompatibility); }

        static const char* const osxArch_Default;
        static const char* const osxArch_Native;
        static const char* const osxArch_32BitUniversal;
        static const char* const osxArch_64BitUniversal;
        static const char* const osxArch_64Bit;
        Value getMacArchitecture() const                    { return getValue (Ids::osxArchitecture); }

        //==============================================================================
    private:
        friend class Project;
        Project* project;
        ValueTree config;

        Value getValue (const Identifier& name) const       { return config.getPropertyAsValue (name, getUndoManager()); }
        UndoManager* getUndoManager() const                 { return project->getUndoManagerFor (config); }

        BuildConfiguration (Project* project, const ValueTree& configNode);
    };

    int getNumConfigurations() const;
    BuildConfiguration getConfiguration (int index);
    void addNewConfiguration (BuildConfiguration* configToCopy);
    void deleteConfiguration (int index);
    bool hasConfigurationNamed (const String& name) const;
    String getUniqueConfigName (String name) const;

    //==============================================================================
    ValueTree getExporters();
    int getNumExporters();
    ProjectExporter* createExporter (int index);
    void addNewExporter (const String& exporterName);
    void deleteExporter (int index);
    void createDefaultExporters();

    //==============================================================================
    struct ConfigFlag
    {
        String symbol, description, sourceModuleID;
        Value value;   // 1 = true, 2 = false, anything else = use default
    };

    static const char* const configFlagDefault;
    static const char* const configFlagEnabled;
    static const char* const configFlagDisabled;
    Value getConfigFlag (const String& name);
    bool isConfigFlagEnabled (const String& name) const;

    //==============================================================================
    bool isModuleEnabled (const String& moduleID) const;
    Value shouldShowAllModuleFilesInProject (const String& moduleID);
    Value shouldCopyModuleFilesLocally (const String& moduleID);

    void addModule (const String& moduleID, bool shouldCopyFilesLocally);
    void removeModule (const String& moduleID);
    int getNumModules() const;
    String getModuleID (int index) const;
    void addDefaultModules (bool shouldCopyFilesLocally);

    void createRequiredModules (const ModuleList& availableModules, OwnedArray<LibraryModule>& modules) const;

    //==============================================================================
    String getFileTemplate (const String& templateName);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property);
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded);
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved);
    void valueTreeChildOrderChanged (ValueTree& parentTree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree&) const             { return nullptr; }

    //==============================================================================
    static const char* projectFileExtension;

private:
    friend class Item;
    ValueTree projectRoot;
    static File lastDocumentOpened;
    DrawableImage mainProjectIcon;

    void updateProjectSettings();
    void sanitiseConfigFlags();
    void setMissingDefaultValues();
    ValueTree getConfigurations() const;
    void createDefaultConfigs();
    ValueTree getConfigNode();
    ValueTree getModulesNode();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project);
};


#endif   // __JUCER_PROJECT_JUCEHEADER__
