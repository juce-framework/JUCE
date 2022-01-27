/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Application/UserAccount/jucer_LicenseController.h"
#include "Modules/jucer_AvailableModulesList.h"

class ProjectExporter;
class LibraryModule;
class EnabledModulesList;
class ProjectSaver;

namespace ProjectMessages
{
    namespace Ids
    {
       #define DECLARE_ID(name)  static const Identifier name (#name)

        DECLARE_ID (projectMessages);

        DECLARE_ID (incompatibleLicense);
        DECLARE_ID (cppStandard);
        DECLARE_ID (moduleNotFound);
        DECLARE_ID (jucePath);
        DECLARE_ID (jucerFileModified);
        DECLARE_ID (missingModuleDependencies);
        DECLARE_ID (oldProjucer);
        DECLARE_ID (cLion);
        DECLARE_ID (newVersionAvailable);
        DECLARE_ID (pluginCodeInvalid);
        DECLARE_ID (manufacturerCodeInvalid);

        DECLARE_ID (notification);
        DECLARE_ID (warning);

        DECLARE_ID (isVisible);

       #undef DECLARE_ID
    }

    inline Identifier getTypeForMessage (const Identifier& message)
    {
        static Identifier warnings[] = { Ids::incompatibleLicense, Ids::cppStandard, Ids::moduleNotFound,
                                         Ids::jucePath, Ids::jucerFileModified, Ids::missingModuleDependencies,
                                         Ids::oldProjucer, Ids::cLion, Ids::pluginCodeInvalid, Ids::manufacturerCodeInvalid };

        if (std::find (std::begin (warnings), std::end (warnings), message) != std::end (warnings))
            return Ids::warning;

        if (message == Ids::newVersionAvailable)
            return Ids::notification;

        jassertfalse;
        return {};
    }

    inline String getTitleForMessage (const Identifier& message)
    {
        if (message == Ids::incompatibleLicense)        return "Incompatible License and Splash Screen Setting";
        if (message == Ids::cppStandard)                return "C++ Standard";
        if (message == Ids::moduleNotFound)             return "Module Not Found";
        if (message == Ids::jucePath)                   return "JUCE Path";
        if (message == Ids::jucerFileModified)          return "Project File Modified";
        if (message == Ids::missingModuleDependencies)  return "Missing Module Dependencies";
        if (message == Ids::oldProjucer)                return "Projucer Out of Date";
        if (message == Ids::newVersionAvailable)        return "New Version Available";
        if (message == Ids::cLion)                      return "Deprecated Exporter";
        if (message == Ids::pluginCodeInvalid)          return "Invalid Plugin Code";
        if (message == Ids::manufacturerCodeInvalid)    return "Invalid Manufacturer Code";

        jassertfalse;
        return {};
    }

    inline String getDescriptionForMessage (const Identifier& message)
    {
        if (message == Ids::incompatibleLicense)        return "Save and export is disabled.";
        if (message == Ids::cppStandard)                return "Module(s) have a higher C++ standard requirement than the project.";
        if (message == Ids::moduleNotFound)             return "Module(s) could not be found at the specified paths.";
        if (message == Ids::jucePath)                   return "The path to your JUCE folder is incorrect.";
        if (message == Ids::jucerFileModified)          return "The .jucer file has been modified since the last save.";
        if (message == Ids::missingModuleDependencies)  return "Module(s) have missing dependencies.";
        if (message == Ids::oldProjucer)                return "The version of the Projucer you are using is out of date.";
        if (message == Ids::newVersionAvailable)        return "A new version of JUCE is available to download.";
        if (message == Ids::cLion)                      return "The CLion exporter is deprecated. Use JUCE's CMake support instead.";
        if (message == Ids::pluginCodeInvalid)          return "The plugin code should be exactly four characters in length.";
        if (message == Ids::manufacturerCodeInvalid)    return "The manufacturer code should be exactly four characters in length.";

        jassertfalse;
        return {};
    }

    using MessageAction = std::pair<String, std::function<void()>>;
}

enum class Async { no, yes };

//==============================================================================
class Project  : public FileBasedDocument,
                 private ValueTree::Listener,
                 private LicenseController::LicenseStateListener,
                 private ChangeListener,
                 private AvailableModulesList::Listener
{
public:
    //==============================================================================
    Project (const File&);
    ~Project() override;

    //==============================================================================
    String getDocumentTitle() override;
    Result loadDocument (const File& file) override;
    Result saveDocument (const File& file) override;
    void saveDocumentAsync (const File& file, std::function<void (Result)> callback) override;

    void saveProject (Async, ProjectExporter* exporterToSave, std::function<void (Result)> onCompletion);
    void saveAndMoveTemporaryProject (bool openInIDE);
    Result saveResourcesOnly();
    void openProjectInIDE (ProjectExporter& exporterToOpen);

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

    static String getAppConfigFilename()                        { return "AppConfig.h"; }
    static String getPluginDefinesFilename()                    { return "JucePluginDefines.h"; }
    static String getJuceSourceHFilename()                      { return "JuceHeader.h"; }

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

    const build_tools::ProjectType& getProjectType() const;
    String getProjectTypeString() const                  { return projectTypeValue.get(); }
    void setProjectType (const String& newProjectType)   { projectTypeValue = newProjectType; }

    String getProjectNameString() const                  { return projectNameValue.get(); }
    String getProjectFilenameRootString()                { return File::createLegalFileName (getDocumentTitle()); }
    String getProjectUIDString() const                   { return projectUIDValue.get(); }

    String getProjectLineFeed() const                    { return projectLineFeedValue.get(); }

    String getVersionString() const                      { return versionValue.get(); }
    String getVersionAsHex() const                       { return build_tools::getVersionAsHex (getVersionString()); }
    int getVersionAsHexInteger() const                   { return build_tools::getVersionAsHexInteger (getVersionString()); }
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
    String getSplashScreenColourString() const           { return splashScreenColourValue.get(); }

    static StringArray getCppStandardStrings()           { return { "C++14", "C++17", "C++20", "Use Latest" }; }
    static Array<var> getCppStandardVars()               { return { "14",    "17",    "20",    "latest" }; }

    static String getLatestNumberedCppStandardString()
    {
        auto cppStandardVars = getCppStandardVars();
        return cppStandardVars[cppStandardVars.size() - 2];
    }

    String getCppStandardString() const                  { return cppStandardValue.get(); }

    StringArray getCompilerFlagSchemes() const;
    void addCompilerFlagScheme (const String&);
    void removeCompilerFlagScheme (const String&);

    String getPostExportShellCommandPosixString() const  { return postExportShellCommandPosixValue.get(); }
    String getPostExportShellCommandWinString() const    { return postExportShellCommandWinValue.get(); }

    bool shouldUseAppConfig() const                      { return useAppConfigValue.get(); }
    bool shouldAddUsingNamespaceToJuceHeader() const     { return addUsingNamespaceToJuceHeader.get(); }

    //==============================================================================
    String getPluginNameString() const                { return pluginNameValue.get(); }
    String getPluginDescriptionString() const         { return pluginDescriptionValue.get();}
    String getPluginManufacturerString() const        { return pluginManufacturerValue.get(); }
    String getPluginManufacturerCodeString() const    { return pluginManufacturerCodeValue.get(); }
    String getPluginCodeString() const                { return pluginCodeValue.get(); }
    String getPluginChannelConfigsString() const      { return pluginChannelConfigsValue.get(); }
    String getAAXIdentifierString() const             { return pluginAAXIdentifierValue.get(); }
    String getPluginAUExportPrefixString() const      { return pluginAUExportPrefixValue.get(); }
    String getVSTNumMIDIInputsString() const          { return pluginVSTNumMidiInputsValue.get(); }
    String getVSTNumMIDIOutputsString() const         { return pluginVSTNumMidiOutputsValue.get(); }

    static bool checkMultiChoiceVar (const ValueTreePropertyWithDefault& valueToCheck, Identifier idToCheck) noexcept
    {
        if (! valueToCheck.get().isArray())
            return false;

        auto v = valueToCheck.get();

        if (auto* varArray = v.getArray())
            return varArray->contains (idToCheck.toString());

        return false;
    }

    bool isAudioPluginProject() const                 { return getProjectType().isAudioPlugin(); }

    bool shouldBuildVST() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST); }
    bool shouldBuildVST3() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildVST3); }
    bool shouldBuildAU() const                        { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAU); }
    bool shouldBuildAUv3() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAUv3); }
    bool shouldBuildRTAS() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildRTAS); }
    bool shouldBuildAAX() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAAX); }
    bool shouldBuildStandalonePlugin() const          { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildStandalone); }
    bool shouldBuildUnityPlugin() const               { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildUnity); }
    bool shouldEnableIAA() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::enableIAA); }

    bool isPluginSynth() const                        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsSynth); }
    bool pluginWantsMidiInput() const                 { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginWantsMidiIn); }
    bool pluginProducesMidiOutput() const             { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginProducesMidiOut); }
    bool isPluginMidiEffect() const                   { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsMidiEffectPlugin); }
    bool pluginEditorNeedsKeyFocus() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginEditorRequiresKeys); }
    bool isPluginRTASBypassDisabled() const           { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginRTASDisableBypass); }
    bool isPluginRTASMultiMonoDisabled() const        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginRTASDisableMultiMono); }
    bool isPluginAAXBypassDisabled() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableBypass); }
    bool isPluginAAXMultiMonoDisabled() const         { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableMultiMono); }

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

    String getIAATypeCode() const;
    String getIAAPluginName() const;

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
    bool shouldBuildTargetType (build_tools::ProjectType::Target::Type targetType) const noexcept;
    static build_tools::ProjectType::Target::Type getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffix);

    //==============================================================================
    void updateDeprecatedProjectSettingsInteractively();

    StringPairArray getAppConfigDefs();
    StringPairArray getAudioPluginFlags() const;

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
        bool isSourceFile() const;

        String getID() const;
        void setID (const String& newID);
        Item findItemWithID (const String& targetId) const; // (recursive search)

        String getImageFileID() const;
        std::unique_ptr<Drawable> loadAsImageFile() const;

        //==============================================================================
        Value getNameValue();
        String getName() const;
        File getFile() const;
        String getFilePath() const;
        void setFile (const File& file);
        void setFile (const build_tools::RelativePath& file);
        File determineGroupFolder() const;
        bool renameFile (const File& newFile);

        bool shouldBeAddedToTargetProject() const;
        bool shouldBeAddedToTargetExporter (const ProjectExporter&) const;
        bool shouldBeCompiled() const;
        Value getShouldCompileValue();

        bool shouldBeAddedToBinaryResources() const;
        Value getShouldAddToBinaryResourcesValue();

        bool shouldBeAddedToXcodeResources() const;
        Value getShouldAddToXcodeResourcesValue();

        Value getShouldInhibitWarningsValue();
        bool shouldInhibitWarnings() const;

        bool isModuleCode() const;

        Value getShouldSkipPCHValue();
        bool shouldSkipPCH() const;

        Value getCompilerFlagSchemeValue();
        String getCompilerFlagSchemeString() const;

        void setCompilerFlagScheme (const String&);
        void clearCurrentCompilerFlagScheme();

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
        bool addRelativeFile (const build_tools::RelativePath& file, int insertIndex, bool shouldCompile);
        void removeItemFromProject();
        void sortAlphabetically (bool keepGroupsAtStart, bool recursive);
        Item findItemForFile (const File& file) const;
        bool containsChildForFile (const build_tools::RelativePath& file) const;

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
    std::unique_ptr<ProjectExporter> createExporter (int index);
    void addNewExporter (const Identifier& exporterIdentifier);
    void createExporterForCurrentPlatform();

    struct ExporterIterator
    {
        ExporterIterator (Project& project);

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
        ValueTreePropertyWithDefault value;
    };

    ValueTreePropertyWithDefault getConfigFlag (const String& name);
    bool isConfigFlagEnabled (const String& name, bool defaultIsEnabled = false) const;

    //==============================================================================
    EnabledModulesList& getEnabledModules();

    AvailableModulesList& getExporterPathsModulesList()  { return exporterPathsModulesList; }
    void rescanExporterPathModules (bool async = false);

    std::pair<String, File> getModuleWithID (const String&);

    //==============================================================================
    PropertiesFile& getStoredProperties() const;

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree&) const             { return nullptr; }
    UndoManager* getUndoManager() const                                 { return nullptr; }

    //==============================================================================
    static const char* projectFileExtension;

    //==============================================================================
    bool updateCachedFileState();
    String getCachedFileStateContent() const noexcept  { return cachedFileState.second; }

    String serialiseProjectXml (std::unique_ptr<XmlElement>) const;

    //==============================================================================
    String getUniqueTargetFolderSuffixForExporter (const Identifier& exporterIdentifier, const String& baseTargetFolder);

    //==============================================================================
    bool isCurrentlySaving() const noexcept              { return saver != nullptr; }

    bool isTemporaryProject() const noexcept             { return tempDirectory != File(); }
    File getTemporaryDirectory() const noexcept          { return tempDirectory; }
    void setTemporaryDirectory (const File&) noexcept;

    //==============================================================================
    ValueTree getProjectMessages() const  { return projectMessages; }

    void addProjectMessage (const Identifier& messageToAdd, std::vector<ProjectMessages::MessageAction>&& messageActions);
    void removeProjectMessage (const Identifier& messageToRemove);

    std::vector<ProjectMessages::MessageAction> getMessageActions (const Identifier& message);

    //==============================================================================
    bool hasIncompatibleLicenseTypeAndSplashScreenSetting() const;
    bool isFileModificationCheckPending() const;
    bool isSaveAndExportDisabled() const;

private:
    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree&, int, int) override;

    //==============================================================================
    struct ProjectFileModificationPoller  : private Timer
    {
        ProjectFileModificationPoller (Project& p);
        bool isCheckPending() const noexcept  { return pending; }

    private:
        void timerCallback() override;
        void reset();

        void resaveProject();
        void reloadProjectFromDisk();

        Project& project;
        bool pending = false;
    };

    //==============================================================================
    ValueTree projectRoot  { Ids::JUCERPROJECT };

    ValueTreePropertyWithDefault projectNameValue, projectUIDValue, projectLineFeedValue, projectTypeValue, versionValue, bundleIdentifierValue, companyNameValue,
                                 companyCopyrightValue, companyWebsiteValue, companyEmailValue, displaySplashScreenValue, splashScreenColourValue, cppStandardValue,
                                 headerSearchPathsValue, preprocessorDefsValue, userNotesValue, maxBinaryFileSizeValue, includeBinaryDataInJuceHeaderValue, binaryDataNamespaceValue,
                                 compilerFlagSchemesValue, postExportShellCommandPosixValue, postExportShellCommandWinValue, useAppConfigValue, addUsingNamespaceToJuceHeader;

    ValueTreePropertyWithDefault pluginFormatsValue, pluginNameValue, pluginDescriptionValue, pluginManufacturerValue, pluginManufacturerCodeValue,
                                 pluginCodeValue, pluginChannelConfigsValue, pluginCharacteristicsValue, pluginAUExportPrefixValue, pluginAAXIdentifierValue,
                                 pluginAUMainTypeValue, pluginAUSandboxSafeValue, pluginRTASCategoryValue, pluginVSTCategoryValue, pluginVST3CategoryValue, pluginAAXCategoryValue,
                                 pluginVSTNumMidiInputsValue, pluginVSTNumMidiOutputsValue;

    //==============================================================================
    std::unique_ptr<EnabledModulesList> enabledModulesList;

    AvailableModulesList exporterPathsModulesList;

    //==============================================================================
    void updateDeprecatedProjectSettings();

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
    File tempDirectory;
    std::pair<Time, String> cachedFileState;

    //==============================================================================
    friend class Item;
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

    //==============================================================================
    void licenseStateChanged() override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void availableModulesChanged (AvailableModulesList*) override;

    void updateLicenseWarning();
    void updateJUCEPathWarning();

    void updateModuleWarnings();
    void updateExporterWarnings();
    void updateCppStandardWarning (bool showWarning);
    void updateMissingModuleDependenciesWarning (bool showWarning);
    void updateOldProjucerWarning (bool showWarning);
    void updateCLionWarning (bool showWarning);
    void updateModuleNotFoundWarning (bool showWarning);
    void updateCodeWarning (Identifier identifier, String value);

    ValueTree projectMessages { ProjectMessages::Ids::projectMessages, {},
                                { { ProjectMessages::Ids::notification, {} }, { ProjectMessages::Ids::warning, {} } } };
    std::map<Identifier, std::vector<ProjectMessages::MessageAction>> messageActions;

    ProjectFileModificationPoller fileModificationPoller { *this };

    std::unique_ptr<FileChooser> chooser;
    std::unique_ptr<ProjectSaver> saver;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
    JUCE_DECLARE_WEAK_REFERENCEABLE (Project)
};
