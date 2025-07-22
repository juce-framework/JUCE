/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

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

        DECLARE_ID (cppStandard);
        DECLARE_ID (moduleNotFound);
        DECLARE_ID (jucePath);
        DECLARE_ID (jucerFileModified);
        DECLARE_ID (missingModuleDependencies);
        DECLARE_ID (oldProjucer);
        DECLARE_ID (newVersionAvailable);
        DECLARE_ID (pluginCodeInvalid);
        DECLARE_ID (manufacturerCodeInvalid);
        DECLARE_ID (deprecatedExporter);
        DECLARE_ID (unsupportedArm32Config);
        DECLARE_ID (arm64Warning);

        DECLARE_ID (notification);
        DECLARE_ID (warning);

        DECLARE_ID (isVisible);

       #undef DECLARE_ID
    }

    inline Identifier getTypeForMessage (const Identifier& message)
    {
        static Identifier warnings[] = { Ids::cppStandard, Ids::moduleNotFound, Ids::jucePath,
                                         Ids::jucerFileModified, Ids::missingModuleDependencies,
                                         Ids::oldProjucer, Ids::pluginCodeInvalid, Ids::manufacturerCodeInvalid,
                                         Ids::deprecatedExporter, Ids::unsupportedArm32Config, Ids::arm64Warning };

        if (std::find (std::begin (warnings), std::end (warnings), message) != std::end (warnings))
            return Ids::warning;

        static Identifier notifications[] = { Ids::newVersionAvailable };

        if (std::find (std::begin (notifications), std::end (notifications), message) != std::end (notifications))
            return Ids::notification;

        jassertfalse;
        return {};
    }

    inline String getTitleForMessage (const Identifier& message)
    {
        if (message == Ids::cppStandard)                return "C++ Standard";
        if (message == Ids::moduleNotFound)             return "Module Not Found";
        if (message == Ids::jucePath)                   return "JUCE Path";
        if (message == Ids::jucerFileModified)          return "Project File Modified";
        if (message == Ids::missingModuleDependencies)  return "Missing Module Dependencies";
        if (message == Ids::oldProjucer)                return "Projucer Out of Date";
        if (message == Ids::newVersionAvailable)        return "New Version Available";
        if (message == Ids::pluginCodeInvalid)          return "Invalid Plugin Code";
        if (message == Ids::manufacturerCodeInvalid)    return "Invalid Manufacturer Code";
        if (message == Ids::deprecatedExporter)         return "Deprecated Exporter";
        if (message == Ids::unsupportedArm32Config)     return "Unsupported Architecture";
        if (message == Ids::arm64Warning)               return "Prefer arm64ec over arm64";

        jassertfalse;
        return {};
    }

    inline String getDescriptionForMessage (const Identifier& message)
    {
        if (message == Ids::cppStandard)                return "Module(s) have a higher C++ standard requirement than the project.";
        if (message == Ids::moduleNotFound)             return "Module(s) could not be found at the specified paths.";
        if (message == Ids::jucePath)                   return "The path to your JUCE folder is incorrect.";
        if (message == Ids::jucerFileModified)          return "The .jucer file has been modified since the last save.";
        if (message == Ids::missingModuleDependencies)  return "Module(s) have missing dependencies.";
        if (message == Ids::oldProjucer)                return "The version of the Projucer you are using is out of date.";
        if (message == Ids::newVersionAvailable)        return "A new version of JUCE is available to download.";
        if (message == Ids::pluginCodeInvalid)          return "The plugin code should be exactly four characters in length.";
        if (message == Ids::manufacturerCodeInvalid)    return "The manufacturer code should be exactly four characters in length.";
        if (message == Ids::deprecatedExporter)         return "The project includes a deprecated exporter.";
        if (message == Ids::unsupportedArm32Config)     return "The project includes a Visual Studio configuration that uses the 32-bit Arm architecture, which is no longer supported. This configuration has been hidden, and will be removed on save.";
        if (message == Ids::arm64Warning)               return "For software where interoperability is a concern (such as plugins and hosts), arm64ec will provide the best compatibility with existing x64 software";

        jassertfalse;
        return {};
    }

    using MessageAction = std::pair<String, std::function<void()>>;
}

// Can be shared between multiple classes wanting to create a MessageBox. Ensures that there is one
// MessageBox active at a given time.
class MessageBoxQueue : private AsyncUpdater
{
public:
    struct Listener
    {
        using CreatorFunction = std::function<ScopedMessageBox (MessageBoxOptions, std::function<void (int)>)>;

        virtual ~Listener() = default;

        virtual void canCreateMessageBox (CreatorFunction) = 0;
    };

    void handleAsyncUpdate() override
    {
        schedule();
    }

    auto addListener (Listener& l)
    {
        triggerAsyncUpdate();
        return listeners.addScoped (l);
    }

private:
    ScopedMessageBox create (MessageBoxOptions options, std::function<void (int)> callback)
    {
        hasActiveMessageBox = true;

        return AlertWindow::showScopedAsync (options, [this, cb = std::move (callback)] (int result)
                                             {
                                                 cb (result);
                                                 hasActiveMessageBox = false;
                                                 triggerAsyncUpdate();
                                             });
    }

    void schedule()
    {
        if (hasActiveMessageBox)
            return;

        auto& currentListeners = listeners.getListeners();

        if (! currentListeners.isEmpty())
        {
            currentListeners[0]->canCreateMessageBox ([this] (auto o, auto c)
                                                              {
                                                                  return create (o, c);
                                                              });
        }
    }

    ListenerList<Listener> listeners;
    bool hasActiveMessageBox = false;
};

enum class Async { no, yes };

//==============================================================================
class Project final : public FileBasedDocument,
                      private ValueTree::Listener,
                      private ChangeListener,
                      private AvailableModulesList::Listener,
                      private MessageBoxQueue::Listener
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
    static String getJuceLV2DefinesFilename()                   { return "JuceLV2Defines.h"; }
    static String getLV2FileWriterName()                        { return "juce_lv2_helper"; }
    static String getVST3FileWriterName()                       { return "juce_vst3_helper"; }

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
    String getDefaultCompanyWebsiteString() const;
    String getDefaultAAXIdentifierString() const         { return getDefaultBundleIdentifierString(); }
    String getDefaultPluginManufacturerString() const;
    String getDefaultLV2URI() const                      { return getCompanyWebsiteString() + "/plugins/" + build_tools::makeValidIdentifier (getProjectNameString(), false, true, false); }
    String getDefaultARAFactoryIDString() const;
    String getDefaultARADocumentArchiveID() const;
    String getDefaultARACompatibleArchiveIDs() const;

    String getCompanyNameString() const                  { return companyNameValue.get(); }
    String getCompanyCopyrightString() const             { return companyCopyrightValue.get(); }
    String getCompanyWebsiteString() const               { return companyWebsiteValue.get(); }
    String getCompanyEmailString() const                 { return companyEmailValue.get(); }

    String getHeaderSearchPathsString() const            { return headerSearchPathsValue.get(); }

    StringPairArray getPreprocessorDefs() const          { return parsedPreprocessorDefs; }

    int getMaxBinaryFileSize() const                     { return maxBinaryFileSizeValue.get(); }
    bool shouldIncludeBinaryInJuceHeader() const         { return includeBinaryDataInJuceHeaderValue.get(); }
    String getBinaryDataNamespaceString() const          { return binaryDataNamespaceValue.get(); }

    static StringArray getCppStandardStrings()           { return { "C++17", "C++20", "Use Latest" }; }
    static Array<var> getCppStandardVars()               { return { "17",    "20",    "latest" }; }

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
    String getARAFactoryIDString() const              { return pluginARAFactoryIDValue.get(); }
    String getARADocumentArchiveIDString() const      { return pluginARAArchiveIDValue.get(); }
    String getARACompatibleArchiveIDStrings() const   { return pluginARACompatibleArchiveIDsValue.get(); }
    String getPluginAUExportPrefixString() const      { return pluginAUExportPrefixValue.get(); }
    String getPluginAUMainTypeString() const          { return pluginAUMainTypeValue.get(); }
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
    bool shouldBuildAAX() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildAAX); }
    bool shouldBuildStandalonePlugin() const          { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildStandalone); }
    bool shouldBuildUnityPlugin() const               { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildUnity); }
    bool shouldBuildLV2() const                       { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::buildLV2); }
    bool shouldEnableIAA() const                      { return isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::enableIAA); }
    bool shouldEnableARA() const                      { return (isAudioPluginProject() && checkMultiChoiceVar (pluginFormatsValue, Ids::enableARA)) || getProjectType().isARAAudioPlugin(); }

    bool isPluginSynth() const                        { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsSynth); }
    bool pluginWantsMidiInput() const                 { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginWantsMidiIn); }
    bool pluginProducesMidiOutput() const             { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginProducesMidiOut); }
    bool isPluginMidiEffect() const                   { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginIsMidiEffectPlugin); }
    bool pluginEditorNeedsKeyFocus() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginEditorRequiresKeys); }
    bool isPluginAAXBypassDisabled() const            { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableBypass); }
    bool isPluginAAXMultiMonoDisabled() const         { return checkMultiChoiceVar (pluginCharacteristicsValue, Ids::pluginAAXDisableMultiMono); }

    void disableStandaloneForARAPlugIn();

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

    bool getDefaultEnableARA() const noexcept;
    static StringArray getAllARAContentTypeStrings() noexcept;
    static Array<var> getAllARAContentTypeVars() noexcept;
    Array<var> getDefaultARAContentTypes() const noexcept;

    static StringArray getAllARATransformationFlagStrings() noexcept;
    static Array<var> getAllARATransformationFlagVars() noexcept;
    Array<var> getDefaultARATransformationFlags() const noexcept;

    String getAUMainTypeString() const noexcept;
    bool isAUSandBoxSafe() const noexcept;
    String getVSTCategoryString() const noexcept;
    String getVST3CategoryString() const noexcept;
    int getAAXCategory() const noexcept;
    int getARAContentTypes() const noexcept;
    int getARATransformationFlags() const noexcept;

    String getIAATypeCode() const;
    String getIAAPluginName() const;

    String getUnityScriptName() const    { return addUnityPluginPrefixIfNecessary (getProjectNameString()) + "_UnityScript.cs"; }
    static String addUnityPluginPrefixIfNecessary (const String& name)
    {
        if (! name.startsWithIgnoreCase ("audioplugin"))
            return "audioplugin_" + name;

        return name;
    }

    String getLV2URI() const        { return pluginLV2URIValue.get(); }

    //==============================================================================
    bool isAUPluginHost()   const;
    bool isVSTPluginHost()  const;
    bool isVST3PluginHost() const;
    bool isLV2PluginHost()  const;
    bool isARAPluginHost()  const;

    //==============================================================================
    bool shouldBuildTargetType (build_tools::ProjectType::Target::Type targetType) const noexcept;
    static build_tools::ProjectType::Target::Type getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffix);

    //==============================================================================
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
    void createEnabledModulesList();

          EnabledModulesList& getEnabledModules();
    const EnabledModulesList& getEnabledModules() const;

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
    bool isFileModificationCheckPending() const;
    bool isSaveAndExportDisabled() const;

    MessageBoxQueue messageBoxQueue;

private:
    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree&, int, int) override;

    void valueTreeChildAddedOrRemoved (ValueTree&, ValueTree&);

    //==============================================================================
    void canCreateMessageBox (CreatorFunction) override;

    //==============================================================================
    template <typename This>
    static auto& getEnabledModulesImpl (This&);

    //==============================================================================
    struct ProjectFileModificationPoller final : private Timer
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
                                 companyCopyrightValue, companyWebsiteValue, companyEmailValue, cppStandardValue, headerSearchPathsValue, preprocessorDefsValue,
                                 userNotesValue, maxBinaryFileSizeValue, includeBinaryDataInJuceHeaderValue, binaryDataNamespaceValue, compilerFlagSchemesValue,
                                 postExportShellCommandPosixValue, postExportShellCommandWinValue, useAppConfigValue, addUsingNamespaceToJuceHeader;

    ValueTreePropertyWithDefault pluginFormatsValue, pluginNameValue, pluginDescriptionValue, pluginManufacturerValue, pluginManufacturerCodeValue,
                                 pluginCodeValue, pluginChannelConfigsValue, pluginCharacteristicsValue, pluginAUExportPrefixValue, pluginAAXIdentifierValue,
                                 pluginAUMainTypeValue, pluginAUSandboxSafeValue, pluginVSTCategoryValue, pluginVST3CategoryValue, pluginAAXCategoryValue,
                                 pluginEnableARA, pluginARAAnalyzableContentValue, pluginARAFactoryIDValue, pluginARAArchiveIDValue, pluginARACompatibleArchiveIDsValue, pluginARATransformFlagsValue,
                                 pluginVSTNumMidiInputsValue, pluginVSTNumMidiOutputsValue, pluginLV2URIValue;

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
    void updateWebsiteDependencies();
    ValueTree getConfigurations() const;
    ValueTree getConfigNode();

    void updateOldStyleConfigList();
    void moveOldPropertyFromProjectToAllExporters (Identifier name);
    void removeDefunctExporters();
    void updateOldModulePaths();

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster*) override;
    void availableModulesChanged (AvailableModulesList*) override;

    void updateJUCEPathWarning();

    void updateModuleWarnings();
    void updateExporterWarnings();
    void updateCppStandardWarning (bool showWarning);
    void updateMissingModuleDependenciesWarning (bool showWarning);
    void updateOldProjucerWarning (bool showWarning);
    void updateModuleNotFoundWarning (bool showWarning);
    void updateCodeWarning (Identifier identifier, String value);

    ValueTree projectMessages { ProjectMessages::Ids::projectMessages, {},
                                { { ProjectMessages::Ids::notification, {} }, { ProjectMessages::Ids::warning, {} } } };
    std::map<Identifier, std::vector<ProjectMessages::MessageAction>> messageActions;

    ProjectFileModificationPoller fileModificationPoller { *this };

    std::unique_ptr<FileChooser> chooser;
    std::unique_ptr<ProjectSaver> saver;

    std::optional<MessageBoxOptions> exporterRemovalMessageBoxOptions;
    ErasedScopeGuard messageBoxQueueListenerScope;
    ScopedMessageBox messageBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
    JUCE_DECLARE_WEAK_REFERENCEABLE (Project)
};
