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
class AvailableModuleList;
class ProjectContentComponent;
class CompileEngineSettings;

//==============================================================================
class Project  : public FileBasedDocument,
                 public ValueTree::Listener
{
public:
    //==============================================================================
    Project (const File&);
    ~Project() override;

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
    File getProjectFolder() const                               { return getFile().getParentDirectory(); }
    File getGeneratedCodeFolder() const                         { return getFile().getSiblingFile ("JuceLibraryCode"); }
    File getSourceFilesFolder() const                           { return getProjectFolder().getChildFile ("Source"); }
    File getLocalModulesFolder() const                          { return getGeneratedCodeFolder().getChildFile ("modules"); }
    File getLocalModuleFolder (const String& moduleID) const    { return getLocalModulesFolder().getChildFile (moduleID); }
    File getAppIncludeFile() const                              { return getGeneratedCodeFolder().getChildFile (getJuceSourceHFilename()); }

    File getBinaryDataCppFile (int index) const;
    File getBinaryDataHeaderFile() const                        { return getBinaryDataCppFile (0).withFileExtension (".h"); }

    String getAppConfigFilename() const                         { return "AppConfig.h"; }
    String getJuceSourceFilenameRoot() const                    { return "JuceLibraryCode"; }
    String getJuceSourceHFilename() const                       { return "JuceHeader.h"; }

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
    ValueTree getProjectRoot() const                     { return projectRoot; }
    Value getProjectValue (const Identifier& name)       { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }
    var   getProjectVar   (const Identifier& name) const { return projectRoot.getProperty        (name); }

    const ProjectType& getProjectType() const;
    String getProjectTypeString() const                  { return projectTypeValue.get(); }
    void setProjectType (const String& newProjectType)   { projectTypeValue = newProjectType; }

    String getProjectNameString() const                  { return projectNameValue.get(); }
    String getProjectFilenameRootString()                { return File::createLegalFileName (getDocumentTitle()); }
    String getProjectUIDString() const                   { return projectUIDValue.get(); }

    String getProjectLineFeed() const                    { return projectLineFeedValue.get(); }

    String getVersionString() const                      { return versionValue.get(); }
    String getVersionAsHex() const;
    int getVersionAsHexInteger() const;
    void setProjectVersion (const String& newVersion)    { versionValue = newVersion; }

    String getBundleIdentifierString() const             { return bundleIdentifierValue.get(); }
    String getDefaultBundleIdentifierString() const;
    String getDefaultAAXIdentifierString() const         { return getDefaultBundleIdentifierString(); }
    String getDefaultPluginManufacturerString() const;

    String getCompanyNameString() const                  { return companyNameValue.get(); }
    String getCompanyCopyrightString() const             { return companyCopyrightValue.get(); }
    String getCompanyWebsiteString() const               { return companyWebsiteValue.get(); }
    String getCompanyEmailString() const                 { return companyEmailValue.get(); }

    String getHeaderSearchPathsString() const            { return headerSearchPathsValue.get(); }

    StringPairArray getPreprocessorDefs() const          { return parsedPreprocessorDefs; }

    int getMaxBinaryFileSize() const                     { return maxBinaryFileSizeValue.get(); }
    bool shouldIncludeBinaryInJuceHeader() const         { return includeBinaryDataInJuceHeaderValue.get(); }
    String getBinaryDataNamespaceString() const          { return binaryDataNamespaceValue.get(); }

    bool shouldDisplaySplashScreen() const               { return displaySplashScreenValue.get(); }
    bool shouldReportAppUsage() const                    { return reportAppUsageValue.get(); }
    String getSplashScreenColourString() const           { return splashScreenColourValue.get(); }

    String getCppStandardString() const                  { return cppStandardValue.get(); }

    //==============================================================================
    String getPluginNameString() const                { return pluginNameValue.get(); }
    String getPluginDescriptionString() const         { return pluginDescriptionValue.get();}
    String getPluginManufacturerString() const        { return pluginManufacturerValue.get(); }
    String getPluginManufacturerCodeString() const    { return pluginManufacturerCodeValue.get(); }
    String getPluginCodeString() const                { return pluginCodeValue.get(); }
    String getPluginChannelConfigsString() const      { return pluginChannelConfigsValue.get(); }
    String getAAXIdentifierString() const             { return pluginAAXIdentifierValue.get(); }
    String getPluginAUExportPrefixString() const      { return pluginAUExportPrefixValue.get(); }
    String getPluginAUMainTypeString() const          { return pluginAUMainTypeValue.get(); }

    //==============================================================================
    static bool checkMultiChoiceVar (const ValueWithDefault& valueToCheck, Identifier idToCheck) noexcept
    {
        if (! valueToCheck.get().isArray())
            return false;

        auto v = valueToCheck.get();

        if (auto* varArray = v.getArray())
            return varArray->contains (idToCheck.toString());

        return false;
    }

    //==============================================================================
    bool shouldBuildVST() const                       { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST); }
    bool shouldBuildVST3() const                      { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST3); }
    bool shouldBuildAU() const                        { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildAU); }
    bool shouldBuildAUv3() const                      { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildAUv3); }
    bool shouldBuildRTAS() const                      { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildRTAS); }
    bool shouldBuildAAX() const                       { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildAAX); }
    bool shouldBuildStandalonePlugin() const          { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildStandalone); }
    bool shouldBuildUnityPlugin() const               { return checkMultiChoiceVar (pluginFormatsValue, Ids::buildUnity); }
    bool shouldEnableIAA() const                      { return checkMultiChoiceVar (pluginFormatsValue, Ids::enableIAA); }

    //==============================================================================
    bool isPluginSynth() const                        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsSynth); }
    bool pluginWantsMidiInput() const                 { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginWantsMidiIn); }
    bool pluginProducesMidiOutput() const             { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginProducesMidiOut); }
    bool isPluginMidiEffect() const                   { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsMidiEffectPlugin); }
    bool pluginEditorNeedsKeyFocus() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginEditorRequiresKeys); }
    bool isPluginRTASBypassDisabled() const           { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginRTASDisableBypass); }
    bool isPluginRTASMultiMonoDisabled() const        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginRTASDisableMultiMono); }
    bool isPluginAAXBypassDisabled() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableBypass); }
    bool isPluginAAXMultiMonoDisabled() const         { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableMultiMono); }

    //==============================================================================
    static StringArray getAllAUMainTypeStrings() noexcept;
    static Array<var> getAllAUMainTypeVars() noexcept;
    Array<var> getDefaultAUMainTypes() const noexcept;

    static StringArray getAllVSTCategoryStrings() noexcept;
    Array<var> getDefaultVSTCategories() const noexcept;

    static StringArray getAllVST3CategoryStrings() noexcept;
    Array<var> getDefaultVST3Categories() const noexcept;

    static StringArray getAllAAXCategoryStrings() noexcept;
    static Array<var> getAllAAXCategoryVars() noexcept;
    Array<var> getDefaultAAXCategories() const noexcept;

    static StringArray getAllRTASCategoryStrings() noexcept;
    static Array<var> getAllRTASCategoryVars() noexcept;
    Array<var> getDefaultRTASCategories() const noexcept;

    String getAUMainTypeString() const noexcept;
    bool isAUSandBoxSafe() const noexcept;
    String getVSTCategoryString() const noexcept;
    String getVST3CategoryString() const noexcept;
    int getAAXCategory() const noexcept;
    int getRTASCategory() const noexcept;

    String getIAATypeCode();
    String getIAAPluginName();

    String getUnityScriptName() const    { return addUnityPluginPrefixIfNecessary (getProjectNameString()) + "_UnityScript.cs"; }
    static String addUnityPluginPrefixIfNecessary (const String& name)
    {
        if (! name.startsWithIgnoreCase ("audioplugin"))
            return "audioplugin_" + name;

        return name;
    }

    //==============================================================================
    bool isAUPluginHost();
    bool isVSTPluginHost();
    bool isVST3PluginHost();

    //==============================================================================
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

        bool needsSaving() const noexcept;

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
        ProjectExporter* operator->() const      { return exporter.get(); }

        std::unique_ptr<ProjectExporter> exporter;
        int index;

    private:
        Project& project;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterIterator)
    };

    //==============================================================================
    struct ConfigFlag
    {
        String symbol, description, sourceModuleID;
        ValueWithDefault value;
    };

    ValueWithDefault getConfigFlag (const String& name);
    bool isConfigFlagEnabled (const String& name, bool defaultIsEnabled = false) const;

    //==============================================================================
    EnabledModuleList& getEnabledModules();

    AvailableModuleList& getExporterPathsModuleList();
    void rescanExporterPathModules (bool async = false);

    std::pair<String, File> getModuleWithID (const String&);

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
    UndoManager* getUndoManager() const                                 { return nullptr; }

    //==============================================================================
    static const char* projectFileExtension;

    //==============================================================================
    bool hasProjectBeenModified();
    void updateModificationTime() { modificationTime = getFile().getLastModificationTime(); }

    //==============================================================================
    String getUniqueTargetFolderSuffixForExporter (const String& exporterName, const String& baseTargetFolder);

    //==============================================================================
    bool isCurrentlySaving() const noexcept        { return isSaving; }
    bool shouldWaitAfterSaving = false;
    String specifiedExporterToSave = {};

    //==============================================================================
    bool isTemporaryProject() const noexcept             { return tempDirectory != File(); }
    File getTemporaryDirectory() const noexcept          { return tempDirectory; }
    void setTemporaryDirectory (const File&) noexcept;

    void setOpenInIDEAfterSaving (bool open) noexcept    { openInIDEAfterSaving = open; }
    bool shouldOpenInIDEAfterSaving() const noexcept     { return openInIDEAfterSaving; }

    //==============================================================================
    bool shouldSendGUIBuilderAnalyticsEvent() noexcept;

    //==============================================================================
    CompileEngineSettings& getCompileEngineSettings()    { return *compileEngineSettings; }

private:
    ValueTree projectRoot  { Ids::JUCERPROJECT };

    ValueWithDefault projectNameValue, projectUIDValue, projectLineFeedValue, projectTypeValue, versionValue, bundleIdentifierValue, companyNameValue,
                     companyCopyrightValue, companyWebsiteValue, companyEmailValue, displaySplashScreenValue, reportAppUsageValue, splashScreenColourValue, cppStandardValue,
                     headerSearchPathsValue, preprocessorDefsValue, userNotesValue, maxBinaryFileSizeValue, includeBinaryDataInJuceHeaderValue, binaryDataNamespaceValue;

    ValueWithDefault pluginFormatsValue, pluginNameValue, pluginDescriptionValue, pluginManufacturerValue, pluginManufacturerCodeValue,
                     pluginCodeValue, pluginChannelConfigsValue, pluginCharacteristicsValue, pluginAUExportPrefixValue, pluginAAXIdentifierValue,
                     pluginAUMainTypeValue, pluginAUSandboxSafeValue, pluginRTASCategoryValue, pluginVSTCategoryValue, pluginVST3CategoryValue, pluginAAXCategoryValue;

    //==============================================================================
    std::unique_ptr<CompileEngineSettings> compileEngineSettings;
    std::unique_ptr<EnabledModuleList> enabledModuleList;
    std::unique_ptr<AvailableModuleList> exporterPathsModuleList;

    //==============================================================================
    bool shouldWriteLegacyPluginFormatSettings = false;
    bool shouldWriteLegacyPluginCharacteristicsSettings = false;

    static Array<Identifier> getLegacyPluginFormatIdentifiers() noexcept;
    static Array<Identifier> getLegacyPluginCharacteristicsIdentifiers() noexcept;

    void writeLegacyPluginFormatSettings();
    void writeLegacyPluginCharacteristicsSettings();

    void coalescePluginFormatValues();
    void coalescePluginCharacteristicsValues();
    void updatePluginCategories();

    //==============================================================================
    File tempDirectory = {};
    bool openInIDEAfterSaving = false;

    void askUserWhereToSaveProject();
    void moveTemporaryDirectory (const File&);
    bool saveProjectRootToFile();

    //==============================================================================
    bool hasSentGUIBuilderAnalyticsEvent = false;

    //==============================================================================
    friend class Item;
    bool isSaving = false;
    Time modificationTime;
    StringPairArray parsedPreprocessorDefs;

    //==============================================================================
    void initialiseProjectValues();
    void initialiseMainGroup();
    void initialiseAudioPluginValues();

    bool setCppVersionFromOldExporterSettings();

    void createAudioPluginPropertyEditors (PropertyListBuilder& props);

    //==============================================================================
    void updateTitleDependencies();
    void updateCompanyNameDependencies();
    void updateProjectSettings();
    ValueTree getConfigurations() const;
    ValueTree getConfigNode();

    void updateOldStyleConfigList();
    void moveOldPropertyFromProjectToAllExporters (Identifier name);
    void removeDefunctExporters();
    void updateOldModulePaths();
    void warnAboutOldProjucerVersion();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
};
