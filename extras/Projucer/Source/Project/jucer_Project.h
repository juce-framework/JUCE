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

    String getVersionString() const                      { return versionValue.get(); }
    String getVersionAsHex() const;
    int getVersionAsHexInteger() const;
    void setProjectVersion (const String& newVersion)    { versionValue = newVersion; }

    String getBundleIdentifierString() const             { return bundleIdentifierValue.get(); }
    String getDefaultBundleIdentifierString()            { return "com.yourcompany." + CodeHelpers::makeValidIdentifier (getProjectNameString(), false, true, false); }
    String getDefaultAAXIdentifierString()               { return getDefaultBundleIdentifierString(); }

    String getCompanyNameString() const                  { return companyNameValue.get(); }
    String getCompanyCopyrightString() const             { return companyCopyrightValue.get(); }
    String getCompanyWebsiteString() const               { return companyWebsiteValue.get(); }
    String getCompanyEmailString() const                 { return companyEmailValue.get(); }

    String getHeaderSearchPathsString() const            { return headerSearchPathsValue.get(); }

    StringPairArray getPreprocessorDefs() const          { return parsedPreprocessorDefs; }

    int getMaxBinaryFileSize() const                     { return maxBinaryFileSizeValue.get(); }
    bool shouldIncludeBinaryInAppConfig() const          { return includeBinaryDataInAppConfigValue.get(); }
    String getBinaryDataNamespaceString() const          { return binaryDataNamespaceValue.get(); }

    bool shouldDisplaySplashScreen() const               { return displaySplashScreenValue.get(); }
    bool shouldReportAppUsage() const                    { return reportAppUsageValue.get(); }
    String getSplashScreenColourString() const           { return splashScreenColourValue.get(); }

    String getCppStandardString() const                  { return cppStandardValue.get(); }

    //==============================================================================
    bool shouldBuildVST() const                 { return buildVSTValue.get(); }
    bool shouldBuildVST3() const                { return buildVST3Value.get(); }
    bool shouldBuildAU() const                  { return buildAUValue.get(); }
    bool shouldBuildAUv3() const                { return buildAUv3Value.get(); }
    bool shouldBuildRTAS() const                { return buildRTASValue.get(); }
    bool shouldBuildAAX() const                 { return buildAAXValue.get(); }
    bool shouldBuildStandalonePlugin() const    { return buildStandaloneValue.get(); }
    bool shouldEnableIAA() const                { return enableIAAValue.get(); }

    //==============================================================================
    String getPluginNameString() const                { return pluginNameValue.get(); }
    String getPluginDescriptionString() const         { return pluginDescriptionValue.get();}
    String getPluginManufacturerString() const        { return pluginManufacturerValue.get(); }
    String getPluginManufacturerCodeString() const    { return pluginManufacturerCodeValue.get(); }
    String getPluginCodeString() const                { return pluginCodeValue.get(); }
    String getPluginChannelConfigsString() const      { return pluginChannelConfigsValue.get(); }
    String getPluginAUExportPrefixString() const      { return pluginAUExportPrefixValue.get(); }
    String getPluginAUMainTypeString() const          { return pluginAUMainTypeValue.get(); }
    String getPluginRTASCategoryString() const        { return pluginRTASCategoryValue.get(); }
    String getAAXIdentifierString() const             { return pluginAAXIdentifierValue.get(); }
    String getPluginAAXCategoryString() const         { return pluginAAXCategoryValue.get(); }

    bool isPluginSynth() const                        { return pluginIsSynthValue.get(); }
    bool pluginWantsMidiInput() const                 { return pluginWantsMidiInputValue.get(); }
    bool pluginProducesMidiOutput() const             { return pluginProducesMidiOutValue.get(); }
    bool isPluginMidiEffect() const                   { return pluginIsMidiEffectPluginValue.get(); }
    bool pluginEditorNeedsKeyFocus() const            { return pluginEditorNeedsKeyFocusValue.get(); }
    bool isPluginRTASBypassDisabled() const           { return pluginRTASBypassDisabledValue.get(); }
    bool isPluginRTASMultiMonoDisabled() const        { return pluginRTASMultiMonoDisabledValue.get(); }
    bool isPluginAAXBypassDisabled() const            { return pluginAAXBypassDisabledValue.get(); }
    bool isPluginAAXMultiMonoDisabled() const         { return pluginAAXMultiMonoDisabledValue.get(); }

    String getPluginRTASCategoryCode();
    String getAUMainTypeString();
    String getAUMainTypeCode();
    String getIAATypeCode();
    String getIAAPluginName();
    String getPluginVSTCategoryString();

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

        ScopedPointer<ProjectExporter> exporter;
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
    UndoManager* getUndoManager() const                                 { return nullptr; }

    //==============================================================================
    static const char* projectFileExtension;

    //==============================================================================
    bool hasProjectBeenModified();
    void updateModificationTime() { modificationTime = getFile().getLastModificationTime(); }

    //==============================================================================
    String getUniqueTargetFolderSuffixForExporter (const String& exporterName, const String& baseTargetFolder);

    //==============================================================================
    bool isCurrentlySaving() const noexcept     { return isSaving; }
    bool shouldWaitAfterSaving = false;
    String specifiedExporterToSave = {};

private:
    ValueTree projectRoot  { Ids::JUCERPROJECT };

    ValueWithDefault projectNameValue, projectUIDValue, projectTypeValue, versionValue, bundleIdentifierValue, companyNameValue, companyCopyrightValue,
                     companyWebsiteValue, companyEmailValue, displaySplashScreenValue, reportAppUsageValue, splashScreenColourValue, cppStandardValue,
                     headerSearchPathsValue, preprocessorDefsValue, userNotesValue, maxBinaryFileSizeValue, includeBinaryDataInAppConfigValue, binaryDataNamespaceValue;

    ValueWithDefault buildVSTValue, buildVST3Value, buildAUValue, buildAUv3Value, buildRTASValue, buildAAXValue, buildStandaloneValue,
                     enableIAAValue, pluginNameValue, pluginDescriptionValue, pluginManufacturerValue, pluginManufacturerCodeValue,
                     pluginCodeValue, pluginChannelConfigsValue, pluginIsSynthValue, pluginWantsMidiInputValue, pluginProducesMidiOutValue,
                     pluginIsMidiEffectPluginValue, pluginEditorNeedsKeyFocusValue, pluginVSTCategoryValue, pluginAUExportPrefixValue,
                     pluginAUMainTypeValue, pluginRTASCategoryValue, pluginRTASBypassDisabledValue, pluginRTASMultiMonoDisabledValue,
                     pluginAAXIdentifierValue, pluginAAXCategoryValue, pluginAAXBypassDisabledValue, pluginAAXMultiMonoDisabledValue;

    //==============================================================================
    friend class Item;
    ScopedPointer<EnabledModuleList> enabledModulesList;
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
