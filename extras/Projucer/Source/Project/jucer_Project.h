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

#pragma once

#include "jucer_ProjectType.h"

class ProjectExporter;
class LibraryModule;
class EnabledModuleList;

//==============================================================================
class Project  : public FileBasedDocument,
                 public ValueTree::Listener
{
public:
    //==============================================================================
    Project (const File&);
    ~Project();

    //==============================================================================
    // FileBasedDocument stuff..
    String getDocumentTitle() override;
    Result loadDocument (const File& file) override;
    Result saveDocument (const File& file) override;
    Result saveProject (const File& file, bool isCommandLineApp);
    Result saveResourcesOnly (const File& file);
    File getLastDocumentOpened() override;
    void setLastDocumentOpened (const File& file) override;

    void setTitle (const String& newTitle);

    //==============================================================================
    File getProjectFolder() const                       { return getFile().getParentDirectory(); }
    ValueTree getProjectRoot() const                    { return projectRoot; }
    String getTitle() const                             { return projectRoot [Ids::name]; }
    Value getProjectNameValue()                         { return getProjectValue (Ids::name); }
    String getProjectFilenameRoot()                     { return File::createLegalFileName (getDocumentTitle()); }
    String getProjectUID() const                        { return projectRoot [Ids::ID]; }

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
    void createPropertyEditors (PropertyListBuilder&);

    //==============================================================================
    // project types
    const ProjectType& getProjectType() const;
    Value getProjectTypeValue()                         { return getProjectValue (Ids::projectType); }
    String getProjectTypeString() const                 { return projectRoot [Ids::projectType]; }

    Value getVersionValue()                             { return getProjectValue (Ids::version); }
    String getVersionString() const                     { return projectRoot [Ids::version]; }
    String getVersionAsHex() const;
    int getVersionAsHexInteger() const;

    Value getBundleIdentifier()                         { return getProjectValue (Ids::bundleIdentifier); }
    String getDefaultBundleIdentifier()                 { return "com.yourcompany." + CodeHelpers::makeValidIdentifier (getTitle(), false, true, false); }

    Value getAAXIdentifier()                            { return getProjectValue (Ids::aaxIdentifier); }
    String getDefaultAAXIdentifier()                    { return getDefaultBundleIdentifier(); }

    Value getCompanyName()                              { return getProjectValue (Ids::companyName); }
    Value getCompanyCopyright()                         { return getProjectValue (Ids::companyCopyright); }
    Value getCompanyWebsite()                           { return getProjectValue (Ids::companyWebsite); }
    Value getCompanyEmail()                             { return getProjectValue (Ids::companyEmail); }

    Value shouldDisplaySplashScreen()                   { return getProjectValue (Ids::displaySplashScreen); }
    Value shouldReportAppUsage()                        { return getProjectValue (Ids::reportAppUsage); }
    Value splashScreenColour()                          { return getProjectValue (Ids::splashScreenColour); }

    Value getCppStandardValue()                         { return getProjectValue (Ids::cppLanguageStandard); }

    //==============================================================================
    Value getProjectValue (const Identifier& name)       { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }
    var   getProjectVar   (const Identifier& name) const { return projectRoot.getProperty        (name); }

    Value getProjectHeaderSearchPaths()                  { return getProjectValue (Ids::headerPath); }
    String getHeaderSearchPaths() const                  { return projectRoot [Ids::headerPath]; }

    Value getProjectPreprocessorDefs()                   { return getProjectValue (Ids::defines); }
    StringPairArray getPreprocessorDefs() const;

    Value getProjectUserNotes()                          { return getProjectValue (Ids::userNotes); }

    //==============================================================================
    File getGeneratedCodeFolder() const                         { return getFile().getSiblingFile ("JuceLibraryCode"); }
    File getSourceFilesFolder() const                           { return getProjectFolder().getChildFile ("Source"); }
    File getLocalModulesFolder() const                          { return getGeneratedCodeFolder().getChildFile ("modules"); }
    File getLocalModuleFolder (const String& moduleID) const    { return getLocalModulesFolder().getChildFile (moduleID); }
    File getAppIncludeFile() const                              { return getGeneratedCodeFolder().getChildFile (getJuceSourceHFilename()); }

    File getBinaryDataCppFile (int index) const;
    File getBinaryDataHeaderFile() const                { return getBinaryDataCppFile (0).withFileExtension (".h"); }
    Value getMaxBinaryFileSize()                        { return getProjectValue (Ids::maxBinaryFileSize); }
    Value shouldIncludeBinaryInAppConfig()              { return getProjectValue (Ids::includeBinaryInAppConfig); }
    Value binaryDataNamespace()                         { return getProjectValue (Ids::binaryDataNamespace); }

    //==============================================================================
    String getAppConfigFilename() const                 { return "AppConfig.h"; }
    String getJuceSourceFilenameRoot() const            { return "JuceLibraryCode"; }
    String getJuceSourceHFilename() const               { return "JuceHeader.h"; }

    //==============================================================================
    // Some helper methods for audio plugin/host projects.
    Value getShouldBuildVSTAsValue()                      { return getProjectValue ("buildVST"); }
    Value getShouldBuildVST3AsValue()                     { return getProjectValue ("buildVST3"); }
    Value getShouldBuildAUAsValue()                       { return getProjectValue ("buildAU"); }
    Value getShouldBuildAUv3AsValue()                     { return getProjectValue ("buildAUv3"); }
    Value getShouldBuildRTASAsValue()                     { return getProjectValue ("buildRTAS"); }
    Value getShouldBuildAAXAsValue()                      { return getProjectValue ("buildAAX"); }
    Value getShouldBuildStandalonePluginAsValue()         { return getProjectValue ("buildStandalone");}
    Value getShouldEnableIAAAsValue()                     { return getProjectValue ("enableIAA"); }

    bool shouldBuildVST()         const                   { return getProjectVar ("buildVST"); }
    bool shouldBuildVST3()        const                   { return getProjectVar ("buildVST3"); }
    bool shouldBuildAU()          const                   { return getProjectVar ("buildAU"); }
    bool shouldBuildAUv3()        const                   { return getProjectVar ("buildAUv3"); }
    bool shouldBuildRTAS()        const                   { return getProjectVar ("buildRTAS"); }
    bool shouldBuildAAX()         const                   { return getProjectVar ("buildAAX"); }
    bool shouldBuildStandalonePlugin()  const             { return getProjectVar ("buildStandalone"); }
    bool shouldEnableIAA()        const                   { return getProjectVar ("enableIAA"); }

    //==============================================================================
    Value getPluginName()                       { return getProjectValue ("pluginName"); }
    Value getPluginDesc()                       { return getProjectValue ("pluginDesc"); }
    Value getPluginManufacturer()               { return getProjectValue ("pluginManufacturer"); }
    Value getPluginManufacturerCode()           { return getProjectValue ("pluginManufacturerCode"); }
    Value getPluginCode()                       { return getProjectValue ("pluginCode"); }
    Value getPluginChannelConfigs()             { return getProjectValue ("pluginChannelConfigs"); }
    Value getPluginIsSynth()                    { return getProjectValue ("pluginIsSynth"); }
    Value getPluginWantsMidiInput()             { return getProjectValue ("pluginWantsMidiIn"); }
    Value getPluginProducesMidiOut()            { return getProjectValue ("pluginProducesMidiOut"); }
    Value getPluginIsMidiEffectPlugin()         { return getProjectValue ("pluginIsMidiEffectPlugin"); }
    Value getPluginEditorNeedsKeyFocus()        { return getProjectValue ("pluginEditorRequiresKeys"); }
    Value getPluginVSTCategory()                { return getProjectValue ("pluginVSTCategory"); }
    Value getPluginAUExportPrefix()             { return getProjectValue ("pluginAUExportPrefix"); }
    Value getPluginAUMainType()                 { return getProjectValue ("pluginAUMainType"); }
    Value getPluginRTASCategory()               { return getProjectValue ("pluginRTASCategory"); }
    Value getPluginRTASBypassDisabled()         { return getProjectValue ("pluginRTASDisableBypass"); }
    Value getPluginRTASMultiMonoDisabled()      { return getProjectValue ("pluginRTASDisableMultiMono"); }
    Value getPluginAAXCategory()                { return getProjectValue ("pluginAAXCategory"); }
    Value getPluginAAXBypassDisabled()          { return getProjectValue ("pluginAAXDisableBypass"); }
    Value getPluginAAXMultiMonoDisabled()       { return getProjectValue ("pluginAAXDisableMultiMono"); }
    String getPluginRTASCategoryCode();
    String getAUMainTypeString();
    String getAUMainTypeCode();
    String getIAATypeCode();
    String getIAAPluginName();
    String getPluginVSTCategoryString();

    bool isAUPluginHost();
    bool isVSTPluginHost();
    bool isVST3PluginHost();

    bool shouldBuildTargetType (ProjectType::Target::Type targetType) const noexcept;
    static ProjectType::Target::Type getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffix);

    //==============================================================================
    void updateDeprecatedProjectSettingsInteractively();

    //==============================================================================
    class Item
    {
    public:
        //==============================================================================
        Item (Project& project, const ValueTree& itemNode, bool isModuleCode);
        Item (const Item& other);

        static Item createGroup (Project& project, const String& name, const String& uid, bool isModuleCode);
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
        Drawable* loadAsImageFile() const;

        //==============================================================================
        Value getNameValue();
        String getName() const;
        File getFile() const;
        String getFilePath() const;
        void setFile (const File& file);
        void setFile (const RelativePath& file);
        File determineGroupFolder() const;
        bool renameFile (const File& newFile);

        bool shouldBeAddedToTargetProject() const;
        bool shouldBeCompiled() const;
        Value getShouldCompileValue();

        bool shouldBeAddedToBinaryResources() const;
        Value getShouldAddToBinaryResourcesValue();

        bool shouldBeAddedToXcodeResources() const;
        Value getShouldAddToXcodeResourcesValue();

        Value getShouldInhibitWarningsValue();
        bool shouldInhibitWarnings() const;

        bool isModuleCode() const;

        //==============================================================================
        bool canContain (const Item& child) const;
        int getNumChildren() const                      { return state.getNumChildren(); }
        Item getChild (int index) const                 { return Item (project, state.getChild (index), belongsToModule); }

        Item addNewSubGroup (const String& name, int insertIndex);
        Item getOrCreateSubGroup (const String& name);
        void addChild (const Item& newChild, int insertIndex);
        bool addFileAtIndex (const File& file, int insertIndex, bool shouldCompile);
        bool addFileRetainingSortOrder (const File& file, bool shouldCompile);
        void addFileUnchecked (const File& file, int insertIndex, bool shouldCompile);
        bool addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile);
        void removeItemFromProject();
        void sortAlphabetically (bool keepGroupsAtStart, bool recursive);
        Item findItemForFile (const File& file) const;
        bool containsChildForFile (const RelativePath& file) const;

        Item getParent() const;
        Item createCopy();

        UndoManager* getUndoManager() const              { return project.getUndoManagerFor (state); }

        Icon getIcon (bool isOpen = false) const;
        bool isIconCrossedOut() const;

        Project& project;
        ValueTree state;

    private:
        Item& operator= (const Item&);
        bool belongsToModule;
    };

    Item getMainGroup();

    void findAllImageItems (OwnedArray<Item>& items);

    //==============================================================================
    ValueTree getExporters();
    int getNumExporters();
    ProjectExporter* createExporter (int index);
    void addNewExporter (const String& exporterName);
    void createExporterForCurrentPlatform();

    struct ExporterIterator
    {
        ExporterIterator (Project& project);
        ~ExporterIterator();

        bool next();

        ProjectExporter& operator*() const       { return *exporter; }
        ProjectExporter* operator->() const      { return exporter; }

        ScopedPointer<ProjectExporter> exporter;
        int index;

    private:
        Project& project;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterIterator)
    };

    //==============================================================================
    struct ConfigFlag
    {
        String symbol, description, sourceModuleID, defaultValue;
        Value value;   // 1 = true, 2 = false, anything else = use default
    };

    static const char* const configFlagDefault;
    static const char* const configFlagEnabled;
    static const char* const configFlagDisabled;

    Value getConfigFlag (const String& name);
    bool isConfigFlagEnabled (const String& name, bool defaultIsEnabled = false) const;

    //==============================================================================
    EnabledModuleList& getModules();

    //==============================================================================
    String getFileTemplate (const String& templateName);

    //==============================================================================
    PropertiesFile& getStoredProperties() const;

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree&, int, int) override;
    void valueTreeParentChanged (ValueTree&) override;

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree&) const             { return nullptr; }

    //==============================================================================
    static const char* projectFileExtension;

    //==============================================================================
    bool hasProjectBeenModified();
    void updateModificationTime() { modificationTime = getFile().getLastModificationTime(); }

    //==============================================================================
    String getUniqueTargetFolderSuffixForExporter (const String& exporterName, const String& baseTargetFolder);

    //==============================================================================
    bool shouldWaitAfterSaving = false;
    String specifiedExporterToSave = {};

private:
    //==============================================================================
    void setMissingAudioPluginDefaultValues();
    void createAudioPluginPropertyEditors (PropertyListBuilder& props);
    bool setCppVersionFromOldExporterSettings();

    //==============================================================================
    friend class Item;
    ValueTree projectRoot;
    ScopedPointer<EnabledModuleList> enabledModulesList;
    bool isSaving;

    void updateProjectSettings();
    void sanitiseConfigFlags();
    void setMissingDefaultValues();
    ValueTree getConfigurations() const;
    ValueTree getConfigNode();

    void updateOldStyleConfigList();
    void moveOldPropertyFromProjectToAllExporters (Identifier name);
    void removeDefunctExporters();
    void updateOldModulePaths();
    void warnAboutOldProjucerVersion();

    Time modificationTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
};
