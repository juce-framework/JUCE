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

#include "../Application/jucer_Headers.h"
#include "jucer_Project.h"
#include "../ProjectSaving/jucer_ProjectSaver.h"
#include "../Application/jucer_Application.h"

//==============================================================================
Project::ProjectFileModificationPoller::ProjectFileModificationPoller (Project& p)
    : project (p)
{
    startTimer (250);
}

void Project::ProjectFileModificationPoller::reset()
{
    project.removeProjectMessage (ProjectMessages::Ids::jucerFileModified);
    pending = false;

    startTimer (250);
}

void Project::ProjectFileModificationPoller::timerCallback()
{
    if (project.updateCachedFileState() && ! pending)
    {
         project.addProjectMessage (ProjectMessages::Ids::jucerFileModified,
                                    { { "Save current state", [this] { resaveProject(); } },
                                      { "Re-load from disk",  [this] { reloadProjectFromDisk(); } },
                                      { "Ignore",             [this] { reset(); } } });

         stopTimer();
         pending = true;
    }
}

void Project::ProjectFileModificationPoller::reloadProjectFromDisk()
{
    auto oldTemporaryDirectory = project.getTemporaryDirectory();
    auto projectFile = project.getFile();

    MessageManager::callAsync ([oldTemporaryDirectory, projectFile]
    {
        if (auto* mw = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (projectFile))
        {
            Component::SafePointer<MainWindow> parent { mw };
            mw->closeCurrentProject (OpenDocumentManager::SaveIfNeeded::no, [parent, oldTemporaryDirectory, projectFile] (bool)
            {
                if (parent == nullptr)
                    return;

                parent->openFile (projectFile, [parent, oldTemporaryDirectory] (bool openedSuccessfully)
                {
                    if (parent == nullptr)
                        return;

                    if (openedSuccessfully && oldTemporaryDirectory != File())
                        if (auto* newProject = parent->getProject())
                            newProject->setTemporaryDirectory (oldTemporaryDirectory);
                });
            });
        }
    });
}

void Project::ProjectFileModificationPoller::resaveProject()
{
    reset();
    project.saveProject (Async::yes, nullptr, nullptr);
}

//==============================================================================
Project::Project (const File& f)
    : FileBasedDocument (projectFileExtension,
                         String ("*") + projectFileExtension,
                         "Choose a Jucer project to load",
                         "Save Jucer project")
{
    Logger::writeToLog ("Loading project: " + f.getFullPathName());

    setFile (f);

    createEnabledModulesList();
    initialiseProjectValues();
    initialiseMainGroup();
    initialiseAudioPluginValues();

    setChangedFlag (false);
    updateCachedFileState();

    auto& app = ProjucerApplication::getApp();

    app.getJUCEPathModulesList().addListener (this);
    app.getUserPathsModulesList().addListener (this);

    updateJUCEPathWarning();
    getGlobalProperties().addChangeListener (this);

    if (! app.isRunningCommandLine && app.isAutomaticVersionCheckingEnabled())
        LatestVersionCheckerAndUpdater::getInstance()->checkForNewVersion (true);
}

Project::~Project()
{
    projectRoot.removeListener (this);
    getGlobalProperties().removeChangeListener (this);

    auto& app = ProjucerApplication::getApp();

    app.openDocumentManager.closeAllDocumentsUsingProjectWithoutSaving (*this);

    app.getJUCEPathModulesList().removeListener (this);
    app.getUserPathsModulesList().removeListener (this);
}

const char* Project::projectFileExtension = ".jucer";

//==============================================================================
void Project::setTitle (const String& newTitle)
{
    projectNameValue = newTitle;

    updateTitleDependencies();
}

void Project::updateTitleDependencies()
{
    auto projectName = getProjectNameString();

    getMainGroup().getNameValue() = projectName;

    pluginNameValue.          setDefault (projectName);
    pluginDescriptionValue.   setDefault (projectName);
    bundleIdentifierValue.    setDefault (getDefaultBundleIdentifierString());
    pluginAUExportPrefixValue.setDefault (build_tools::makeValidIdentifier (projectName, false, true, false) + "AU");
    pluginAAXIdentifierValue. setDefault (getDefaultAAXIdentifierString());
    pluginLV2URIValue.        setDefault (getDefaultLV2URI());
    pluginARAFactoryIDValue.  setDefault (getDefaultARAFactoryIDString());
    pluginARAArchiveIDValue.  setDefault (getDefaultARADocumentArchiveID());
    pluginARACompatibleArchiveIDsValue.setDefault (getDefaultARACompatibleArchiveIDs());
}

String Project::getDocumentTitle()
{
    return getProjectNameString();
}

void Project::updateCompanyNameDependencies()
{
    bundleIdentifierValue.setDefault    (getDefaultBundleIdentifierString());
    companyWebsiteValue.setDefault      (getDefaultCompanyWebsiteString());
    pluginAAXIdentifierValue.setDefault (getDefaultAAXIdentifierString());
    pluginARAFactoryIDValue.setDefault  (getDefaultARAFactoryIDString());
    pluginARAArchiveIDValue.setDefault  (getDefaultARADocumentArchiveID());
    pluginManufacturerValue.setDefault  (getDefaultPluginManufacturerString());
}

void Project::updateWebsiteDependencies()
{
    pluginLV2URIValue.setDefault (getDefaultLV2URI());
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty (Ids::name, getDocumentTitle(), nullptr);
}

bool Project::setCppVersionFromOldExporterSettings()
{
    auto highestLanguageStandard = -1;

    for (ExporterIterator exporter (*this); exporter.next();)
    {
        if (exporter->isXcode()) // cpp version was per-build configuration for xcode exporters
        {
            for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            {
                auto cppLanguageStandard = config->getValue (Ids::cppLanguageStandard).getValue();

                if (cppLanguageStandard != var())
                {
                    auto versionNum = cppLanguageStandard.toString().getLastCharacters (2).getIntValue();

                    if (versionNum > highestLanguageStandard)
                        highestLanguageStandard = versionNum;
                }
            }
        }
        else
        {
            auto cppLanguageStandard = exporter->getSetting (Ids::cppLanguageStandard).getValue();

            if (cppLanguageStandard != var())
            {
                if (cppLanguageStandard.toString().containsIgnoreCase ("latest"))
                {
                    cppStandardValue = Project::getCppStandardVars().getLast();
                    return true;
                }

                auto versionNum = cppLanguageStandard.toString().getLastCharacters (2).getIntValue();

                if (versionNum > highestLanguageStandard)
                    highestLanguageStandard = versionNum;
            }
        }
    }

    if (highestLanguageStandard >= 17)
    {
        cppStandardValue = highestLanguageStandard;
        return true;
    }

    return false;
}

void Project::updateDeprecatedProjectSettings()
{
    for (const auto& version : { "11", "14" })
    {
        if (cppStandardValue.get().toString() == version)
        {
            cppStandardValue.resetToDefault();
            break;
        }
    }

    for (ExporterIterator exporter (*this); exporter.next();)
        exporter->updateDeprecatedSettings();
}

void Project::initialiseMainGroup()
{
    // Create main file group if missing
    if (! projectRoot.getChildWithName (Ids::MAINGROUP).isValid())
    {
        Item mainGroup (*this, ValueTree (Ids::MAINGROUP), false);
        projectRoot.addChild (mainGroup.state, 0, nullptr);
    }

    getMainGroup().initialiseMissingProperties();
}

void Project::initialiseProjectValues()
{
    projectNameValue.referTo         (projectRoot, Ids::name,                getUndoManager(), "JUCE Project");
    projectUIDValue.referTo          (projectRoot, Ids::ID,                  getUndoManager(), createAlphaNumericUID());

    if (projectUIDValue.isUsingDefault())
        projectUIDValue = projectUIDValue.getDefault();

    projectLineFeedValue.referTo     (projectRoot, Ids::projectLineFeed,     getUndoManager(), "\r\n");

    companyNameValue.referTo         (projectRoot, Ids::companyName,         getUndoManager());
    companyCopyrightValue.referTo    (projectRoot, Ids::companyCopyright,    getUndoManager());
    companyWebsiteValue.referTo      (projectRoot, Ids::companyWebsite,      getUndoManager(), getDefaultCompanyWebsiteString());
    companyEmailValue.referTo        (projectRoot, Ids::companyEmail,        getUndoManager());

    projectTypeValue.referTo         (projectRoot, Ids::projectType,         getUndoManager(), build_tools::ProjectType_GUIApp::getTypeName());
    versionValue.referTo             (projectRoot, Ids::version,             getUndoManager(), "1.0.0");
    bundleIdentifierValue.referTo    (projectRoot, Ids::bundleIdentifier,    getUndoManager(), getDefaultBundleIdentifierString());

    useAppConfigValue.referTo             (projectRoot, Ids::useAppConfig,                  getUndoManager(), true);
    addUsingNamespaceToJuceHeader.referTo (projectRoot, Ids::addUsingNamespaceToJuceHeader, getUndoManager(), true);

    cppStandardValue.referTo (projectRoot, Ids::cppLanguageStandard, getUndoManager(), "17");

    headerSearchPathsValue.referTo   (projectRoot, Ids::headerPath, getUndoManager());
    preprocessorDefsValue.referTo    (projectRoot, Ids::defines,    getUndoManager());
    userNotesValue.referTo           (projectRoot, Ids::userNotes,  getUndoManager());

    maxBinaryFileSizeValue.referTo   (projectRoot, Ids::maxBinaryFileSize,         getUndoManager(), 10240 * 1024);

    // this is here for backwards compatibility with old projects using the incorrect id
    if (projectRoot.hasProperty ("includeBinaryInAppConfig"))
         includeBinaryDataInJuceHeaderValue.referTo (projectRoot, "includeBinaryInAppConfig", getUndoManager(), true);
    else
        includeBinaryDataInJuceHeaderValue.referTo (projectRoot, Ids::includeBinaryInJuceHeader, getUndoManager(), true);

    binaryDataNamespaceValue.referTo (projectRoot, Ids::binaryDataNamespace, getUndoManager(), "BinaryData");

    compilerFlagSchemesValue.referTo (projectRoot, Ids::compilerFlagSchemes, getUndoManager(), Array<var>(), ",");

    postExportShellCommandPosixValue.referTo (projectRoot, Ids::postExportShellCommandPosix, getUndoManager());
    postExportShellCommandWinValue.referTo   (projectRoot, Ids::postExportShellCommandWin,   getUndoManager());
}

void Project::initialiseAudioPluginValues()
{
    auto makeValid4CC = [] (const String& seed)
    {
        auto s = build_tools::makeValidIdentifier (seed, false, true, false) + "xxxx";

        return s.substring (0, 1).toUpperCase()
             + s.substring (1, 4).toLowerCase();
    };

    pluginFormatsValue.referTo               (projectRoot, Ids::pluginFormats,              getUndoManager(),
                                              Array<var> (Ids::buildVST3.toString(), Ids::buildAU.toString(), Ids::buildStandalone.toString()), ",");
    pluginCharacteristicsValue.referTo       (projectRoot, Ids::pluginCharacteristicsValue, getUndoManager(), Array<var>(), ",");

    pluginNameValue.referTo                  (projectRoot, Ids::pluginName,                 getUndoManager(), getProjectNameString());
    pluginDescriptionValue.referTo           (projectRoot, Ids::pluginDesc,                 getUndoManager(), getProjectNameString());
    pluginManufacturerValue.referTo          (projectRoot, Ids::pluginManufacturer,         getUndoManager(), getDefaultPluginManufacturerString());
    pluginManufacturerCodeValue.referTo      (projectRoot, Ids::pluginManufacturerCode,     getUndoManager(), "Manu");
    pluginCodeValue.referTo                  (projectRoot, Ids::pluginCode,                 getUndoManager(), makeValid4CC (getProjectUIDString() + getProjectUIDString()));
    pluginChannelConfigsValue.referTo        (projectRoot, Ids::pluginChannelConfigs,       getUndoManager());
    pluginAAXIdentifierValue.referTo         (projectRoot, Ids::aaxIdentifier,              getUndoManager(), getDefaultAAXIdentifierString());
    pluginARAFactoryIDValue.referTo          (projectRoot, Ids::araFactoryID,               getUndoManager(), getDefaultARAFactoryIDString());
    pluginARAArchiveIDValue.referTo          (projectRoot, Ids::araDocumentArchiveID,       getUndoManager(), getDefaultARADocumentArchiveID());
    pluginAUExportPrefixValue.referTo        (projectRoot, Ids::pluginAUExportPrefix,       getUndoManager(),
                                              build_tools::makeValidIdentifier (getProjectNameString(), false, true, false) + "AU");

    pluginAUMainTypeValue.referTo            (projectRoot, Ids::pluginAUMainType,           getUndoManager(), getDefaultAUMainTypes(),    ",");
    pluginAUSandboxSafeValue.referTo         (projectRoot, Ids::pluginAUIsSandboxSafe,      getUndoManager(), false);
    pluginVSTCategoryValue.referTo           (projectRoot, Ids::pluginVSTCategory,          getUndoManager(), getDefaultVSTCategories(),  ",");
    pluginVST3CategoryValue.referTo          (projectRoot, Ids::pluginVST3Category,         getUndoManager(), getDefaultVST3Categories(), ",");
    pluginAAXCategoryValue.referTo           (projectRoot, Ids::pluginAAXCategory,          getUndoManager(), getDefaultAAXCategories(),  ",");

    pluginEnableARA.referTo                  (projectRoot, Ids::enableARA,                  getUndoManager(),  shouldEnableARA(), ",");
    pluginARAAnalyzableContentValue.referTo  (projectRoot, Ids::pluginARAAnalyzableContent, getUndoManager(), getDefaultARAContentTypes(), ",");
    pluginARATransformFlagsValue.referTo     (projectRoot, Ids::pluginARATransformFlags,    getUndoManager(), getDefaultARATransformationFlags(), ",");
    pluginARACompatibleArchiveIDsValue.referTo (projectRoot, Ids::araCompatibleArchiveIDs,    getUndoManager(), getDefaultARACompatibleArchiveIDs());

    pluginVSTNumMidiInputsValue.referTo      (projectRoot, Ids::pluginVSTNumMidiInputs,     getUndoManager(), 16);
    pluginVSTNumMidiOutputsValue.referTo     (projectRoot, Ids::pluginVSTNumMidiOutputs,    getUndoManager(), 16);

    pluginLV2URIValue.referTo                (projectRoot, Ids::lv2Uri,                     getUndoManager(), getDefaultLV2URI());
}

void Project::updateOldStyleConfigList()
{
    auto deprecatedConfigsList = projectRoot.getChildWithName (Ids::CONFIGURATIONS);

    if (deprecatedConfigsList.isValid())
    {
        projectRoot.removeChild (deprecatedConfigsList, nullptr);

        for (ExporterIterator exporter (*this); exporter.next();)
        {
            if (exporter->getNumConfigurations() == 0)
            {
                auto newConfigs = deprecatedConfigsList.createCopy();

                if (! exporter->isXcode())
                {
                    for (auto j = newConfigs.getNumChildren(); --j >= 0;)
                    {
                        auto config = newConfigs.getChild (j);

                        config.removeProperty (Ids::osxSDK,           nullptr);
                        config.removeProperty (Ids::osxCompatibility, nullptr);
                        config.removeProperty (Ids::osxArchitecture,  nullptr);
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
        for (ExporterIterator exporter (*this); exporter.next();)
            exporter->settings.setProperty (name, projectRoot [name], nullptr);

        projectRoot.removeProperty (name, nullptr);
    }
}

void Project::removeDefunctExporters()
{
    auto exporters = projectRoot.getChildWithName (Ids::EXPORTFORMATS);

    StringPairArray oldExporters;
    oldExporters.set ("ANDROID", "Android Ant Exporter");
    oldExporters.set ("MSVC6",   "MSVC6");
    oldExporters.set ("VS2010",  "Visual Studio 2010");
    oldExporters.set ("VS2012",  "Visual Studio 2012");
    oldExporters.set ("VS2013",  "Visual Studio 2013");
    oldExporters.set ("VS2015",  "Visual Studio 2015");
    oldExporters.set ("CLION",   "CLion");

    std::vector<String> removedExporterKeys;

    for (auto& key : oldExporters.getAllKeys())
    {
        auto oldExporter = exporters.getChildWithName (key);

        if (oldExporter.isValid())
        {
            removedExporterKeys.push_back (key);
            exporters.removeChild (oldExporter, nullptr);
        }
    }

    if (! removedExporterKeys.empty())
    {
        if (ProjucerApplication::getApp().isRunningCommandLine)
        {
            for (const auto& key : removedExporterKeys)
                std::cout <<  "WARNING! The " + oldExporters[key]
                              + " Exporter is deprecated. The exporter will be removed from this project."
                          << std::endl;
        }
        else
        {
            const String warningTitle { TRANS ("Unsupported exporters") };

            String warningMessage;
            warningMessage << TRANS ("The following exporters are no longer supported") << "\n\n";

            for (const auto& key : removedExporterKeys)
                warningMessage << "    - " + oldExporters[key] + "\n";

            warningMessage << "\n"
                           << TRANS ("These exporters have been removed from the project. If you save the project they will be also erased from the .jucer file.");

            exporterRemovalMessageBoxOptions = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon, warningTitle, warningMessage);
            messageBoxQueueListenerScope = messageBoxQueue.addListener (*this);
        }
    }
}

void Project::updateOldModulePaths()
{
    for (ExporterIterator exporter (*this); exporter.next();)
        exporter->updateOldModulePaths();
}

Array<Identifier> Project::getLegacyPluginFormatIdentifiers() noexcept
{
    static Array<Identifier> legacyPluginFormatIdentifiers { Ids::buildVST, Ids::buildVST3, Ids::buildAU, Ids::buildAUv3,
                                                             Ids::buildAAX, Ids::buildStandalone, Ids::enableIAA };

    return legacyPluginFormatIdentifiers;
}

Array<Identifier> Project::getLegacyPluginCharacteristicsIdentifiers() noexcept
{
    static Array<Identifier> legacyPluginCharacteristicsIdentifiers { Ids::pluginIsSynth, Ids::pluginWantsMidiIn, Ids::pluginProducesMidiOut,
                                                                      Ids::pluginIsMidiEffectPlugin, Ids::pluginEditorRequiresKeys,
                                                                      Ids::pluginAAXDisableBypass, Ids::pluginAAXDisableMultiMono };

    return legacyPluginCharacteristicsIdentifiers;
}

void Project::coalescePluginFormatValues()
{
    Array<var> formatsToBuild;

    for (auto& formatIdentifier : getLegacyPluginFormatIdentifiers())
    {
        if (projectRoot.getProperty (formatIdentifier, false))
            formatsToBuild.add (formatIdentifier.toString());
    }

    if (formatsToBuild.size() > 0)
    {
        if (pluginFormatsValue.isUsingDefault())
        {
            pluginFormatsValue = formatsToBuild;
        }
        else
        {
            auto formatVar = pluginFormatsValue.get();

            if (auto* arr = formatVar.getArray())
                arr->addArray (formatsToBuild);
        }

        shouldWriteLegacyPluginFormatSettings = true;
    }
}

void Project::coalescePluginCharacteristicsValues()
{
    Array<var> pluginCharacteristics;

    for (auto& characteristicIdentifier : getLegacyPluginCharacteristicsIdentifiers())
    {
        if (projectRoot.getProperty (characteristicIdentifier, false))
            pluginCharacteristics.add (characteristicIdentifier.toString());
    }

    if (pluginCharacteristics.size() > 0)
    {
        pluginCharacteristicsValue = pluginCharacteristics;
        shouldWriteLegacyPluginCharacteristicsSettings = true;
    }
}

void Project::updatePluginCategories()
{
    {
        auto aaxCategory = projectRoot.getProperty (Ids::pluginAAXCategory, {}).toString();

        if (getAllAAXCategoryVars().contains (aaxCategory))
            pluginAAXCategoryValue = aaxCategory;
        else if (getAllAAXCategoryStrings().contains (aaxCategory))
            pluginAAXCategoryValue = Array<var> (getAllAAXCategoryVars()[getAllAAXCategoryStrings().indexOf (aaxCategory)]);
    }

    {
        auto vstCategory = projectRoot.getProperty (Ids::pluginVSTCategory, {}).toString();

        if (vstCategory.isNotEmpty() && getAllVSTCategoryStrings().contains (vstCategory))
            pluginVSTCategoryValue = Array<var> (vstCategory);
        else
            pluginVSTCategoryValue.resetToDefault();
    }

    {
        auto auMainType = projectRoot.getProperty (Ids::pluginAUMainType, {}).toString();

        if (auMainType.isNotEmpty())
        {
            if (getAllAUMainTypeVars().contains (auMainType))
                pluginAUMainTypeValue = Array<var> (auMainType);
            else if (getAllAUMainTypeVars().contains (auMainType.quoted ('\'')))
                pluginAUMainTypeValue = Array<var> (auMainType.quoted ('\''));
            else if (getAllAUMainTypeStrings().contains (auMainType))
                pluginAUMainTypeValue = Array<var> (getAllAUMainTypeVars()[getAllAUMainTypeStrings().indexOf (auMainType)]);
        }
        else
        {
            pluginAUMainTypeValue.resetToDefault();
        }
    }
}

void Project::writeLegacyPluginFormatSettings()
{
    if (pluginFormatsValue.isUsingDefault())
    {
        for (auto& formatIdentifier : getLegacyPluginFormatIdentifiers())
            projectRoot.removeProperty (formatIdentifier, nullptr);
    }
    else
    {
        auto formatVar = pluginFormatsValue.get();

        if (auto* arr = formatVar.getArray())
        {
            for (auto& formatIdentifier : getLegacyPluginFormatIdentifiers())
                projectRoot.setProperty (formatIdentifier, arr->contains (formatIdentifier.toString()), nullptr);
        }
    }
}

void Project::writeLegacyPluginCharacteristicsSettings()
{
    if (pluginFormatsValue.isUsingDefault())
    {
        for (auto& characteristicIdentifier : getLegacyPluginCharacteristicsIdentifiers())
            projectRoot.removeProperty (characteristicIdentifier, nullptr);
    }
    else
    {
        auto characteristicsVar = pluginCharacteristicsValue.get();

        if (auto* arr = characteristicsVar.getArray())
        {
            for (auto& characteristicIdentifier : getLegacyPluginCharacteristicsIdentifiers())
                projectRoot.setProperty (characteristicIdentifier, arr->contains (characteristicIdentifier.toString()), nullptr);
        }
    }
}

//==============================================================================
static int getVersionElement (StringRef v, int index)
{
    StringArray parts = StringArray::fromTokens (v, "., ", {});

    return parts [parts.size() - index - 1].getIntValue();
}

static int getJuceVersion (const String& v)
{
    return getVersionElement (v, 2) * 100000
         + getVersionElement (v, 1) * 1000
         + getVersionElement (v, 0);
}

static constexpr int getBuiltJuceVersion()
{
    return JUCE_MAJOR_VERSION * 100000
         + JUCE_MINOR_VERSION * 1000
         + JUCE_BUILDNUMBER;
}

//==============================================================================
static File& lastDocumentOpenedSingleton()
{
    static File lastDocumentOpened;
    return lastDocumentOpened;
}

File Project::getLastDocumentOpened()                   { return lastDocumentOpenedSingleton(); }
void Project::setLastDocumentOpened (const File& file)  { lastDocumentOpenedSingleton() = file; }

static void registerRecentFile (const File& file)
{
    RecentlyOpenedFilesList::registerRecentFileNatively (file);
    getAppSettings().recentFiles.addFile (file);
    getAppSettings().flush();
}

static void forgetRecentFile (const File& file)
{
    RecentlyOpenedFilesList::forgetRecentFileNatively (file);
    getAppSettings().recentFiles.removeFile (file);
    getAppSettings().flush();
}

//==============================================================================
Result Project::loadDocument (const File& file)
{
    auto xml = parseXMLIfTagMatches (file, Ids::JUCERPROJECT.toString());

    if (xml == nullptr)
        return Result::fail ("Not a valid Jucer project!");

    auto newTree = ValueTree::fromXml (*xml);

    if (! newTree.hasType (Ids::JUCERPROJECT))
        return Result::fail ("The document contains errors and couldn't be parsed!");

    registerRecentFile (file);

    enabledModulesList.reset();

    projectRoot = newTree;
    projectRoot.addListener (this);

    createEnabledModulesList();
    initialiseProjectValues();
    initialiseMainGroup();
    initialiseAudioPluginValues();

    coalescePluginFormatValues();
    coalescePluginCharacteristicsValues();
    updatePluginCategories();

    parsedPreprocessorDefs = parsePreprocessorDefs (preprocessorDefsValue.get());

    removeDefunctExporters();
    updateOldModulePaths();
    updateOldStyleConfigList();
    moveOldPropertyFromProjectToAllExporters (Ids::bigIcon);
    moveOldPropertyFromProjectToAllExporters (Ids::smallIcon);
    getEnabledModules().sortAlphabetically();

    rescanExporterPathModules (! ProjucerApplication::getApp().isRunningCommandLine);
    exporterPathsModulesList.addListener (this);

    if (cppStandardValue.isUsingDefault())
        setCppVersionFromOldExporterSettings();

    updateDeprecatedProjectSettings();

    setChangedFlag (false);

    return Result::ok();
}

Result Project::saveDocument ([[maybe_unused]] const File& file)
{
    jassert (file == getFile());

    auto sharedResult = Result::ok();

    saveProject (Async::no, nullptr, [&sharedResult] (Result actualResult)
    {
        sharedResult = actualResult;
    });

    return sharedResult;
}

void Project::saveDocumentAsync ([[maybe_unused]] const File& file,
                                 std::function<void (Result)> afterSave)
{
    jassert (file == getFile());

    saveProject (Async::yes, nullptr, std::move (afterSave));
}

void Project::saveProject (Async async,
                           ProjectExporter* exporterToSave,
                           std::function<void (Result)> onCompletion)
{
    if (isSaveAndExportDisabled())
    {
        onCompletion (Result::fail ("Save and export is disabled."));
        return;
    }

    if (isTemporaryProject())
    {
        // Don't try to save a temporary project directly. Instead, check whether the
        // project is temporary before saving it, and call saveAndMoveTemporaryProject
        // in that case.
        onCompletion (Result::fail ("Cannot save temporary project."));
        return;
    }

    if (saver != nullptr)
    {
        onCompletion (Result::ok());
        return;
    }

    updateProjectSettings();

    if (! ProjucerApplication::getApp().isRunningCommandLine)
    {
        ProjucerApplication::getApp().openDocumentManager.saveAllSyncWithoutAsking();

        if (! isTemporaryProject())
            registerRecentFile (getFile());
    }

    saver = std::make_unique<ProjectSaver> (*this);

    saver->save (async, exporterToSave, [ref = WeakReference<Project> { this }, onCompletion] (Result result)
    {
        if (ref == nullptr)
            return;

        ref->saver = nullptr;
        NullCheckedInvocation::invoke (onCompletion, result);
    });
}

void Project::openProjectInIDE (ProjectExporter& exporterToOpen)
{
    for (ExporterIterator exporter (*this); exporter.next();)
    {
        if (exporter->canLaunchProject() && exporter->getUniqueName() == exporterToOpen.getUniqueName())
        {
            if (isTemporaryProject())
            {
                saveAndMoveTemporaryProject (true);
                return;
            }

            exporter->launchProject();
            return;
        }
    }
}

Result Project::saveResourcesOnly()
{
    saver = std::make_unique<ProjectSaver> (*this);
    return saver->saveResourcesOnly();
}

bool Project::isFileModificationCheckPending() const
{
    return fileModificationPoller.isCheckPending();
}

bool Project::isSaveAndExportDisabled() const
{
    return ! ProjucerApplication::getApp().isRunningCommandLine
           && isFileModificationCheckPending();
}

void Project::updateJUCEPathWarning()
{
    if (ProjucerApplication::getApp().shouldPromptUserAboutIncorrectJUCEPath()
        && ProjucerApplication::getApp().settings->isJUCEPathIncorrect())
    {
        auto dontAskAgain = [this]
        {
            ProjucerApplication::getApp().setShouldPromptUserAboutIncorrectJUCEPath (false);
            removeProjectMessage (ProjectMessages::Ids::jucePath);
        };

        addProjectMessage (ProjectMessages::Ids::jucePath,
                           { { "Set path", [] { ProjucerApplication::getApp().showPathsWindow (true); } },
                             { "Ignore", [this] { removeProjectMessage (ProjectMessages::Ids::jucePath); } },
                             { "Don't ask again", std::move (dontAskAgain) } });
    }
    else
    {
        removeProjectMessage (ProjectMessages::Ids::jucePath);
    }
}

void Project::updateCodeWarning (Identifier identifier, String value)
{
    if (value.length() != 4 || value.toStdString().size() != 4)
        addProjectMessage (identifier, {});
    else
        removeProjectMessage (identifier);
}

void Project::updateModuleWarnings()
{
    auto& modules = getEnabledModules();

    bool cppStandard = false, missingDependencies = false, oldProjucer = false, moduleNotFound = false;

    for (auto moduleID : modules.getAllModules())
    {
        if (! cppStandard && modules.doesModuleHaveHigherCppStandardThanProject (moduleID))
            cppStandard = true;

        if (! missingDependencies && ! modules.getExtraDependenciesNeeded (moduleID).isEmpty())
            missingDependencies = true;

        auto info = modules.getModuleInfo (moduleID);

        if (! oldProjucer && (isJUCEModule (moduleID) && getJuceVersion (info.getVersion()) > getBuiltJuceVersion()))
            oldProjucer = true;

        if (! moduleNotFound && ! info.isValid())
            moduleNotFound = true;
    }

    updateCppStandardWarning (cppStandard);
    updateMissingModuleDependenciesWarning (missingDependencies);
    updateOldProjucerWarning (oldProjucer);
    updateModuleNotFoundWarning (moduleNotFound);
}

void Project::updateExporterWarnings()
{
    const Identifier deprecatedExporters[] = { "CODEBLOCKS_WINDOWS", "CODEBLOCKS_LINUX" };

    for (const auto exporter : getExporters())
    {
        for (const auto& name : deprecatedExporters)
        {
            if (exporter.getType() == name)
            {
                addProjectMessage (ProjectMessages::Ids::deprecatedExporter, {});
                return;
            }
        }
    }

    removeProjectMessage (ProjectMessages::Ids::deprecatedExporter);
}

void Project::updateCppStandardWarning (bool showWarning)
{
    if (showWarning)
    {
        auto removeModules = [this]
        {
            auto& modules = getEnabledModules();

            for (auto& module : modules.getModulesWithHigherCppStandardThanProject())
                modules.removeModule (module);
        };

        auto updateCppStandard = [this]
        {
            cppStandardValue = getEnabledModules().getHighestModuleCppStandard();
        };

        addProjectMessage (ProjectMessages::Ids::cppStandard,
                           { { "Update project C++ standard" , std::move (updateCppStandard) },
                             { "Remove module(s)", std::move (removeModules) } });
    }
    else
    {
        removeProjectMessage (ProjectMessages::Ids::cppStandard);
    }
}

void Project::updateMissingModuleDependenciesWarning (bool showWarning)
{
    if (showWarning)
    {
        auto removeModules = [this]
        {
            auto& modules = getEnabledModules();

            for (auto& mod : modules.getModulesWithMissingDependencies())
                modules.removeModule (mod);
        };

        auto addMissingDependencies = [this]
        {
            auto& modules = getEnabledModules();

            for (auto& mod : modules.getModulesWithMissingDependencies())
                modules.tryToFixMissingDependencies (mod);
        };

        addProjectMessage (ProjectMessages::Ids::missingModuleDependencies,
                           { { "Add missing dependencies", std::move (addMissingDependencies) },
                             { "Remove module(s)", std::move (removeModules) } });
    }
    else
    {
        removeProjectMessage (ProjectMessages::Ids::missingModuleDependencies);
    }
}

void Project::updateOldProjucerWarning (bool showWarning)
{
    if (showWarning)
        addProjectMessage (ProjectMessages::Ids::oldProjucer, {});
    else
        removeProjectMessage (ProjectMessages::Ids::oldProjucer);
}

void Project::updateModuleNotFoundWarning (bool showWarning)
{
    if (showWarning)
        addProjectMessage (ProjectMessages::Ids::moduleNotFound, {});
    else
        removeProjectMessage (ProjectMessages::Ids::moduleNotFound);
}

void Project::changeListenerCallback (ChangeBroadcaster*)
{
    updateJUCEPathWarning();
}

void Project::availableModulesChanged (AvailableModulesList* listThatHasChanged)
{
    if (listThatHasChanged == &ProjucerApplication::getApp().getJUCEPathModulesList())
        updateJUCEPathWarning();

    updateModuleWarnings();
}

void Project::addProjectMessage (const Identifier& messageToAdd,
                                 std::vector<ProjectMessages::MessageAction>&& actions)
{
    removeProjectMessage (messageToAdd);

    messageActions[messageToAdd] = std::move (actions);

    ValueTree child (messageToAdd);
    child.setProperty (ProjectMessages::Ids::isVisible, true, nullptr);

    projectMessages.getChildWithName (ProjectMessages::getTypeForMessage (messageToAdd)).addChild (child, -1, nullptr);
}

void Project::removeProjectMessage (const Identifier& messageToRemove)
{
    auto subTree = projectMessages.getChildWithName (ProjectMessages::getTypeForMessage (messageToRemove));
    auto child = subTree.getChildWithName (messageToRemove);

    if (child.isValid())
        subTree.removeChild (child, nullptr);

    messageActions.erase (messageToRemove);
}

std::vector<ProjectMessages::MessageAction> Project::getMessageActions (const Identifier& message)
{
    auto iter = messageActions.find (message);

    if (iter != messageActions.end())
        return iter->second;

    jassertfalse;
    return {};
}

//==============================================================================
void Project::setTemporaryDirectory (const File& dir) noexcept
{
    tempDirectory = dir;

     // remove this file from the recent documents list as it is a temporary project
    forgetRecentFile (getFile());
}

void Project::saveAndMoveTemporaryProject (bool openInIDE)
{
    chooser = std::make_unique<FileChooser> ("Save Project");
    auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (flags, [this, openInIDE] (const FileChooser& fc)
    {
        auto newParentDirectory = fc.getResult();

        if (! newParentDirectory.exists())
            return;

        auto newDirectory = newParentDirectory.getChildFile (tempDirectory.getFileName());
        auto oldJucerFileName = getFile().getFileName();

        saver = std::make_unique<ProjectSaver> (*this);
        saver->save (Async::yes, nullptr, [this, newDirectory, oldJucerFileName, openInIDE] (Result)
        {
            tempDirectory.copyDirectoryTo (newDirectory);
            tempDirectory.deleteRecursively();
            tempDirectory = File();

            // reload project from new location
            if (auto* window = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (getFile()))
            {
                MessageManager::callAsync ([newDirectory, oldJucerFileName, openInIDE,
                                            safeWindow = Component::SafePointer<MainWindow> { window }]() mutable
                                           {
                                               if (safeWindow != nullptr)
                                                   safeWindow->moveProject (newDirectory.getChildFile (oldJucerFileName),
                                                                            openInIDE ? MainWindow::OpenInIDE::yes
                                                                                      : MainWindow::OpenInIDE::no);
                                           });
            }
        });
    });
}

//==============================================================================
void Project::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (tree.getRoot() == tree)
    {
        if (property == Ids::name)
        {
            updateTitleDependencies();
        }
        else if (property == Ids::companyName)
        {
            updateCompanyNameDependencies();
        }
        else if (property == Ids::companyWebsite)
        {
            updateWebsiteDependencies();
        }
        else if (property == Ids::defines)
        {
            parsedPreprocessorDefs = parsePreprocessorDefs (preprocessorDefsValue.get());
        }
        else if (property == Ids::pluginFormats)
        {
            if (shouldWriteLegacyPluginFormatSettings)
                writeLegacyPluginFormatSettings();
        }
        else if (property == Ids::pluginCharacteristicsValue)
        {
            pluginAUMainTypeValue.setDefault   (getDefaultAUMainTypes());
            pluginVSTCategoryValue.setDefault  (getDefaultVSTCategories());
            pluginVST3CategoryValue.setDefault (getDefaultVST3Categories());
            pluginAAXCategoryValue.setDefault  (getDefaultAAXCategories());
            pluginEnableARA.setDefault         (getDefaultEnableARA());
            pluginARAAnalyzableContentValue.setDefault (getDefaultARAContentTypes());
            pluginARATransformFlagsValue.setDefault    (getDefaultARATransformationFlags());

            if (shouldWriteLegacyPluginCharacteristicsSettings)
                writeLegacyPluginCharacteristicsSettings();
        }
        else if (property == Ids::cppLanguageStandard)
        {
            updateModuleWarnings();
        }
        else if (property == Ids::pluginCode)
        {
            updateCodeWarning (ProjectMessages::Ids::pluginCodeInvalid, pluginCodeValue.get());
        }
        else if (property == Ids::pluginManufacturerCode)
        {
            updateCodeWarning (ProjectMessages::Ids::manufacturerCodeInvalid, pluginManufacturerCodeValue.get());
        }
    }

    changed();
}

void Project::valueTreeChildAddedOrRemoved (ValueTree& parent, ValueTree& child)
{
    if (child.getType() == Ids::MODULE)
        updateModuleWarnings();
    else if (parent.getType() == Ids::EXPORTFORMATS)
        updateExporterWarnings();

    changed();
}

void Project::canCreateMessageBox (CreatorFunction f)
{
    messageBox = f (*exporterRemovalMessageBoxOptions, [this] (auto)
                                                       {
                                                           messageBoxQueueListenerScope.reset();
                                                       });
}

void Project::valueTreeChildAdded (ValueTree& parent, ValueTree& child)
{
    valueTreeChildAddedOrRemoved (parent, child);
}

void Project::valueTreeChildRemoved (ValueTree& parent, ValueTree& child, int)
{
    valueTreeChildAddedOrRemoved (parent, child);
}

void Project::valueTreeChildOrderChanged (ValueTree&, int, int)
{
    changed();
}

//==============================================================================
String Project::serialiseProjectXml (std::unique_ptr<XmlElement> xml) const
{
    if (xml == nullptr)
        return {};

    XmlElement::TextFormat format;
    format.newLineChars = getProjectLineFeed().toRawUTF8();
    return xml->toString (format);
}

bool Project::updateCachedFileState()
{
    auto lastModificationTime = getFile().getLastModificationTime();

    if (lastModificationTime <= cachedFileState.first)
        return false;

    cachedFileState.first = lastModificationTime;

    auto serialisedFileContent = serialiseProjectXml (XmlDocument (getFile()).getDocumentElement());

    if (serialisedFileContent == cachedFileState.second)
        return false;

    cachedFileState.second = serialisedFileContent;
    return true;
}

//==============================================================================
File Project::resolveFilename (String filename) const
{
    if (filename.isEmpty())
        return {};

    filename = build_tools::replacePreprocessorDefs (getPreprocessorDefs(), filename);

   #if ! JUCE_WINDOWS
    if (filename.startsWith ("~"))
        return File::getSpecialLocation (File::userHomeDirectory).getChildFile (filename.trimCharactersAtStart ("~/"));
   #endif

    if (build_tools::isAbsolutePath (filename))
        return File::createFileWithoutCheckingPath (build_tools::currentOSStylePath (filename)); // (avoid assertions for windows-style paths)

    return getFile().getSiblingFile (build_tools::currentOSStylePath (filename));
}

String Project::getRelativePathForFile (const File& file) const
{
    auto filename = file.getFullPathName();

    auto relativePathBase = getFile().getParentDirectory();

    auto p1 = relativePathBase.getFullPathName();
    auto p2 = file.getFullPathName();

    while (p1.startsWithChar (File::getSeparatorChar()))
        p1 = p1.substring (1);

    while (p2.startsWithChar (File::getSeparatorChar()))
        p2 = p2.substring (1);

    if (p1.upToFirstOccurrenceOf (File::getSeparatorString(), true, false)
          .equalsIgnoreCase (p2.upToFirstOccurrenceOf (File::getSeparatorString(), true, false)))
    {
        filename = build_tools::getRelativePathFrom (file, relativePathBase);
    }

    return filename;
}

//==============================================================================
const build_tools::ProjectType& Project::getProjectType() const
{
    if (auto* type = build_tools::ProjectType::findType (getProjectTypeString()))
        return *type;

    auto* guiType = build_tools::ProjectType::findType (build_tools::ProjectType_GUIApp::getTypeName());
    jassert (guiType != nullptr);
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.UndefReturn)
    return *guiType;
}

bool Project::shouldBuildTargetType (build_tools::ProjectType::Target::Type targetType) const noexcept
{
    auto& projectType = getProjectType();

    if (! projectType.supportsTargetType (targetType))
        return false;

    using Target = build_tools::ProjectType::Target;

    switch (targetType)
    {
        case Target::VSTPlugIn:
            return shouldBuildVST();
        case Target::VST3PlugIn:
            return shouldBuildVST3();
        case Target::VST3Helper:
            return shouldBuildVST3();
        case Target::AAXPlugIn:
            return shouldBuildAAX();
        case Target::AudioUnitPlugIn:
            return shouldBuildAU();
        case Target::AudioUnitv3PlugIn:
            return shouldBuildAUv3();
        case Target::StandalonePlugIn:
            return shouldBuildStandalonePlugin();
        case Target::UnityPlugIn:
            return shouldBuildUnityPlugin();
        case Target::LV2PlugIn:
        case Target::LV2Helper:
            return shouldBuildLV2();
        case Target::AggregateTarget:
        case Target::SharedCodeTarget:
            return projectType.isAudioPlugin();
        case Target::unspecified:
            return false;
        case Target::GUIApp:
        case Target::ConsoleApp:
        case Target::StaticLibrary:
        case Target::DynamicLibrary:
        default:
            break;
    }

    return true;
}

build_tools::ProjectType::Target::Type Project::getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffix)
{
    auto path = file.getFullPathName();
    String pluginClientModuleName = "juce_audio_plugin_client";

    auto isInPluginClientSubdir = [&path, &pluginClientModuleName] (StringRef subDir)
    {
        return path.contains (pluginClientModuleName
                             + File::getSeparatorString()
                             + subDir
                             + File::getSeparatorString());
    };

    auto isPluginClientSource = [&path, &pluginClientModuleName] (StringRef suffix)
    {
        auto prefix = pluginClientModuleName + "_" + suffix;
        return path.contains (prefix + ".") || path.contains (prefix + "_");
    };

    using Target = build_tools::ProjectType::Target::Type;

    struct FormatInfo
    {
        const char* source;
        const char* subdir;
        Target target;
    };

    const FormatInfo formatInfo[] { { "AU",         "AU",         Target::AudioUnitPlugIn },
                                    { "AUv3",       "AU",         Target::AudioUnitv3PlugIn },
                                    { "AAX",        "AAX",        Target::AAXPlugIn },
                                    { "VST2",       "VST",        Target::VSTPlugIn },
                                    { "VST3",       "VST3",       Target::VST3PlugIn },
                                    { "Standalone", "Standalone", Target::StandalonePlugIn },
                                    { "Unity",      "Unity",      Target::UnityPlugIn },
                                    { "LV2",        "LV2",        Target::LV2PlugIn } };

    for (const auto& info : formatInfo)
        if (isPluginClientSource (info.source) || isInPluginClientSubdir (info.subdir))
            return info.target;

    return (returnSharedTargetIfNoValidSuffix ? build_tools::ProjectType::Target::SharedCodeTarget
                                              : build_tools::ProjectType::Target::unspecified);
}

//==============================================================================
void Project::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (projectNameValue, "Project Name", 256, false),
               "The name of the project.");

    props.add (new TextPropertyComponent (versionValue, "Project Version", 16, false),
               "The project's version number. This should be in the format major.minor.point[.point] where you should omit the final "
               "(optional) [.point] if you are targeting AU and AUv3 plug-ins as they only support three number versions.");

    props.add (new ChoicePropertyComponent (projectLineFeedValue, "Project Line Feed", { "\\r\\n", "\\n", }, { "\r\n", "\n" }),
               "Use this to set the line feed which will be used when creating new source files for this project "
               "(this won't affect any existing files).");

    props.add (new TextPropertyComponent (companyNameValue, "Company Name", 256, false),
               "Your company name, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyCopyrightValue, "Company Copyright", 256, false),
               "Your company copyright, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyWebsiteValue, "Company Website", 256, false),
               "Your company website, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyEmailValue, "Company E-mail", 256, false),
               "Your company e-mail, which will be added to the properties of the binary where possible");

    props.add (new ChoicePropertyComponent (useAppConfigValue, "Use Global AppConfig Header"),
               "If enabled, the Projucer will generate module wrapper stubs which include AppConfig.h "
               "and will include AppConfig.h in the JuceHeader.h. If disabled, all the settings that would "
               "previously have been specified in the AppConfig.h will be injected via the build system instead, "
               "which may simplify the includes in the project.");

    props.add (new ChoicePropertyComponent (addUsingNamespaceToJuceHeader, "Add \"using namespace juce\" to JuceHeader.h"),
               "If enabled, the JuceHeader.h will include a \"using namespace juce\" statement. If disabled, "
               "no such statement will be included. This setting used to be enabled by default, but it "
               "is recommended to leave it disabled for new projects.");

    {
        StringArray projectTypeNames;
        Array<var> projectTypeCodes;

        auto types = build_tools::ProjectType::getAllTypes();

        for (int i = 0; i < types.size(); ++i)
        {
            projectTypeNames.add (types.getUnchecked (i)->getDescription());
            projectTypeCodes.add (types.getUnchecked (i)->getType());
        }

        props.add (new ChoicePropertyComponent (projectTypeValue, "Project Type", projectTypeNames, projectTypeCodes),
                   "The project type for which settings should be shown.");
    }

    props.add (new TextPropertyComponent (bundleIdentifierValue, "Bundle Identifier", 256, false),
               "A unique identifier for this product, mainly for use in OSX/iOS builds. It should be something like 'com.yourcompanyname.yourproductname'");

    if (isAudioPluginProject())
        createAudioPluginPropertyEditors (props);

    {
        const int maxSizes[] = { 20480, 10240, 6144, 2048, 1024, 512, 256, 128, 64 };

        StringArray maxSizeNames;
        Array<var> maxSizeCodes;

        for (int i = 0; i < numElementsInArray (maxSizes); ++i)
        {
            auto sizeInBytes = maxSizes[i] * 1024;

            maxSizeNames.add (File::descriptionOfSizeInBytes (sizeInBytes));
            maxSizeCodes.add (sizeInBytes);
        }

        props.add (new ChoicePropertyComponent (maxBinaryFileSizeValue, "BinaryData.cpp Size Limit", maxSizeNames, maxSizeCodes),
                   "When splitting binary data into multiple cpp files, the Projucer attempts to keep the file sizes below this threshold. "
                   "(Note that individual resource files which are larger than this size cannot be split across multiple cpp files).");
    }

    props.add (new ChoicePropertyComponent (includeBinaryDataInJuceHeaderValue, "Include BinaryData in JuceHeader"),
                                             "Include BinaryData.h in the JuceHeader.h file");

    props.add (new TextPropertyComponent (binaryDataNamespaceValue, "BinaryData Namespace", 256, false),
                                          "The namespace containing the binary assets.");

    props.add (new ChoicePropertyComponent (cppStandardValue, "C++ Language Standard",
                                            getCppStandardStrings(),
                                            getCppStandardVars()),
               "The standard of the C++ language that will be used for compilation.");

    props.add (new TextPropertyComponent (preprocessorDefsValue, "Preprocessor Definitions", 32768, true),
               "Global preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, or "
               "new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.addSearchPathProperty (headerSearchPathsValue, "Header Search Paths", "Global header search paths.");

    props.add (new TextPropertyComponent (postExportShellCommandPosixValue, "Post-Export Shell Command (macOS, Linux)", 1024, false),
               "A command that will be executed by the system shell after saving this project on macOS or Linux. "
               "The string \"%%1%%\" will be substituted with the absolute path to the project root folder.");

    props.add (new TextPropertyComponent (postExportShellCommandWinValue, "Post-Export Shell Command (Windows)", 1024, false),
               "A command that will be executed by the system shell after saving this project on Windows. "
               "The string \"%%1%%\" will be substituted with the absolute path to the project root folder.");

    props.add (new TextPropertyComponent (userNotesValue, "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

void Project::createAudioPluginPropertyEditors (PropertyListBuilder& props)
{
    {
        StringArray pluginFormatChoices { "VST3", "AU", "AUv3", "AAX", "Standalone", "LV2", "Unity", "Enable IAA", "VST (Legacy)" };
        Array<var> pluginFormatChoiceValues { Ids::buildVST3.toString(), Ids::buildAU.toString(), Ids::buildAUv3.toString(),
                                              Ids::buildAAX.toString(), Ids::buildStandalone.toString(),
                                              Ids::buildLV2.toString(), Ids::buildUnity.toString(), Ids::enableIAA.toString(), Ids::buildVST.toString() };
        if (! getProjectType().isARAAudioPlugin())
        {
            pluginFormatChoices.add ("Enable ARA");
            pluginFormatChoiceValues.add (Ids::enableARA.toString());
        }
        props.add (new MultiChoicePropertyComponent (pluginFormatsValue, "Plugin Formats", pluginFormatChoices, pluginFormatChoiceValues),
                   "Plugin formats to build. If you have selected \"VST (Legacy)\" then you will need to ensure that you have a VST2 SDK "
                   "in your header search paths. The VST2 SDK can be obtained from the vstsdk3610_11_06_2018_build_37 (or older) VST3 SDK "
                   "or JUCE version 5.3.2. You also need a VST2 license from Steinberg to distribute VST2 plug-ins. If you enable ARA you "
                   "will have to obtain the ARA SDK by recursively cloning https://github.com/Celemony/ARA_SDK and checking out the tag "
                   "releases/2.1.0.");
    }
    props.add (new MultiChoicePropertyComponent (pluginCharacteristicsValue, "Plugin Characteristics",
                                                 { "Plugin is a Synth", "Plugin MIDI Input", "Plugin MIDI Output", "MIDI Effect Plugin", "Plugin Editor Requires Keyboard Focus",
                                                   "Disable AAX Bypass", "Disable AAX Multi-Mono" },
                                                 { Ids::pluginIsSynth.toString(), Ids::pluginWantsMidiIn.toString(), Ids::pluginProducesMidiOut.toString(),
                                                   Ids::pluginIsMidiEffectPlugin.toString(), Ids::pluginEditorRequiresKeys.toString(),
                                                   Ids::pluginAAXDisableBypass.toString(), Ids::pluginAAXDisableMultiMono.toString() }),
              "Some characteristics of your plugin such as whether it is a synth, produces MIDI messages, accepts MIDI messages etc.");
    props.add (new TextPropertyComponent (pluginNameValue, "Plugin Name", 128, false),
               "The name of your plugin (keep it short!)");
    props.add (new TextPropertyComponent (pluginDescriptionValue, "Plugin Description", 256, false),
               "A short description of your plugin.");
    props.add (new TextPropertyComponent (pluginManufacturerValue, "Plugin Manufacturer", 256, false),
               "The name of your company (cannot be blank).");
    props.add (new TextPropertyComponent (pluginManufacturerCodeValue, "Plugin Manufacturer Code", 4, false),
               "A four-character unique ID for your company. Note that for AU compatibility, this must contain at least one upper-case letter!"
               " GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case.");
    props.add (new TextPropertyComponent (pluginCodeValue, "Plugin Code", 4, false),
               "A four-character unique ID for your plugin. Note that for AU compatibility, this must contain exactly one upper-case letter!"
               " GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case.");
    props.add (new TextPropertyComponent (pluginChannelConfigsValue, "Plugin Channel Configurations", 1024, false),
               "This list is a comma-separated set list in the form {numIns, numOuts} and each pair indicates a valid plug-in "
               "configuration. For example {1, 1}, {2, 2} means that the plugin can be used either with 1 input and 1 output, "
               "or with 2 inputs and 2 outputs. If your plug-in requires side-chains, aux output buses etc., then you must leave "
               "this field empty and override the isBusesLayoutSupported callback in your AudioProcessor.");
    props.add (new TextPropertyComponent (pluginAAXIdentifierValue, "Plugin AAX Identifier", 256, false),
               "The value to use for the JucePlugin_AAXIdentifier setting");
    props.add (new TextPropertyComponent (pluginAUExportPrefixValue, "Plugin AU Export Prefix", 128, false),
               "A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");
    props.add (new MultiChoicePropertyComponent (pluginAUMainTypeValue, "Plugin AU Main Type", getAllAUMainTypeStrings(), getAllAUMainTypeVars(), 1),
               "AU main type.");
    props.add (new ChoicePropertyComponent (pluginAUSandboxSafeValue, "Plugin AU is sandbox safe"),
               "Check this box if your plug-in is sandbox safe. A sand-box safe plug-in is loaded in a restricted path and can only access it's own bundle resources and "
               "the Music folder. Your plug-in must be able to deal with this. Newer versions of GarageBand require this to be enabled.");

    {
        Array<var> varChoices;
        StringArray stringChoices;

        for (int i = 1; i <= 16; ++i)
        {
            varChoices.add (i);
            stringChoices.add (String (i));
        }

        props.add (new ChoicePropertyComponentWithEnablement (pluginVSTNumMidiInputsValue, pluginCharacteristicsValue, Ids::pluginWantsMidiIn,
                                                              "Plugin VST Num MIDI Inputs",  stringChoices, varChoices),
                   "For VST and VST3 plug-ins that accept MIDI, this allows you to configure the number of inputs.");

        props.add (new ChoicePropertyComponentWithEnablement (pluginVSTNumMidiOutputsValue, pluginCharacteristicsValue, Ids::pluginProducesMidiOut,
                                                              "Plugin VST Num MIDI Outputs", stringChoices, varChoices),
                   "For VST and VST3 plug-ins that produce MIDI, this allows you to configure the number of outputs.");
    }

    {
        Array<var> vst3CategoryVars;

        for (auto s : getAllVST3CategoryStrings())
            vst3CategoryVars.add (s);

        props.add (new MultiChoicePropertyComponent (pluginVST3CategoryValue, "Plugin VST3 Category", getAllVST3CategoryStrings(), vst3CategoryVars),
                   "VST3 category. Most hosts require either \"Fx\" or \"Instrument\" to be selected in order for the plugin to be recognised. "
                   "If neither of these are selected, the appropriate one will be automatically added based on the \"Plugin is a synth\" option.");
    }

    props.add (new MultiChoicePropertyComponent (pluginAAXCategoryValue, "Plugin AAX Category", getAllAAXCategoryStrings(), getAllAAXCategoryVars()),
               "AAX category.");

    {
        Array<var> vstCategoryVars;
        for (auto s : getAllVSTCategoryStrings())
            vstCategoryVars.add (s);

        props.add (new MultiChoicePropertyComponent (pluginVSTCategoryValue, "Plugin VST (Legacy) Category", getAllVSTCategoryStrings(), vstCategoryVars, 1),
                   "VST category.");
    }

    props.add (new TextPropertyComponent (pluginLV2URIValue, "LV2 URI", 128, false),
               "This acts as a unique identifier for this plugin. "
               "If you make any incompatible changes to your plugin (remove parameters, reorder parameters, change preset format etc.) "
               "you MUST change this value. LV2 hosts will assume that any plugins with the same URI are interchangeable.");

    if (shouldEnableARA())
    {
        props.add (new MultiChoicePropertyComponent (pluginARAAnalyzableContentValue, "Plugin ARA Analyzeable Content Types", getAllARAContentTypeStrings(), getAllARAContentTypeVars()),
                   "ARA Analyzeable Content Types.");

        props.add (new MultiChoicePropertyComponent (pluginARATransformFlagsValue, "Plugin ARA Transformation Flags", getAllARATransformationFlagStrings(), getAllARATransformationFlagVars()),
                   "ARA Transformation Flags.");

        props.add (new TextPropertyComponent (pluginARAFactoryIDValue, "Plugin ARA Factory ID", 256, false),
                   "ARA Factory ID.");

        props.add (new TextPropertyComponent (pluginARAArchiveIDValue, "Plugin ARA Document Archive ID", 256, false),
                   "ARA Document Archive ID.");

        props.add (new TextPropertyComponent (pluginARACompatibleArchiveIDsValue, "Plugin ARA Compatible Document Archive IDs", 1024, true),
                   "List of compatible ARA Document Archive IDs - one per line");
    }
}

//==============================================================================
File Project::getBinaryDataCppFile (int index) const
{
    auto cpp = getGeneratedCodeFolder().getChildFile ("BinaryData.cpp");

    if (index > 0)
        return cpp.getSiblingFile (cpp.getFileNameWithoutExtension() + String (index + 1))
                    .withFileExtension (cpp.getFileExtension());

    return cpp;
}

Project::Item Project::getMainGroup()
{
    return { *this, projectRoot.getChildWithName (Ids::MAINGROUP), false };
}

PropertiesFile& Project::getStoredProperties() const
{
    return getAppSettings().getProjectProperties (getProjectUIDString());
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
Project::Item::Item (Project& p, const ValueTree& s, bool isModuleCode)
    : project (p), state (s), belongsToModule (isModuleCode)
{
}

Project::Item::Item (const Item& other)
    : project (other.project), state (other.state), belongsToModule (other.belongsToModule)
{
}

Project::Item Project::Item::createCopy()         { Item i (*this); i.state = i.state.createCopy(); return i; }

String Project::Item::getID() const               { return state [Ids::ID]; }
void Project::Item::setID (const String& newID)   { state.setProperty (Ids::ID, newID, nullptr); }

std::unique_ptr<Drawable> Project::Item::loadAsImageFile() const
{
    const MessageManagerLock mml (ThreadPoolJob::getCurrentThreadPoolJob());

    if (! mml.lockWasGained())
        return nullptr;

    if (isValid())
        return Drawable::createFromImageFile (getFile());

    return {};
}

Project::Item Project::Item::createGroup (Project& project, const String& name, const String& uid, bool isModuleCode)
{
    Item group (project, ValueTree (Ids::GROUP), isModuleCode);
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

bool Project::Item::isSourceFile() const
{
    return isFile() && getFile().hasFileExtension (sourceFileExtensions);
}

Project::Item Project::Item::findItemWithID (const String& targetId) const
{
    if (state [Ids::ID] == targetId)
        return *this;

    if (isGroup())
    {
        for (auto i = getNumChildren(); --i >= 0;)
        {
            auto found = getChild (i).findItemWithID (targetId);

            if (found.isValid())
                return found;
        }
    }

    return Item (project, ValueTree(), false);
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

bool Project::Item::shouldBeAddedToTargetExporter (const ProjectExporter& exporter) const
{
    if (shouldBeAddedToXcodeResources())
        return exporter.isXcode() || shouldBeCompiled();

    return true;
}

Value Project::Item::getShouldCompileValue()                { return state.getPropertyAsValue (Ids::compile, getUndoManager()); }
bool Project::Item::shouldBeCompiled() const                { return state [Ids::compile]; }

Value Project::Item::getShouldAddToBinaryResourcesValue()   { return state.getPropertyAsValue (Ids::resource, getUndoManager()); }
bool Project::Item::shouldBeAddedToBinaryResources() const  { return state [Ids::resource]; }

Value Project::Item::getShouldAddToXcodeResourcesValue()    { return state.getPropertyAsValue (Ids::xcodeResource, getUndoManager()); }
bool Project::Item::shouldBeAddedToXcodeResources() const   { return state [Ids::xcodeResource]; }

Value Project::Item::getShouldInhibitWarningsValue()        { return state.getPropertyAsValue (Ids::noWarnings, getUndoManager()); }
bool Project::Item::shouldInhibitWarnings() const           { return state [Ids::noWarnings]; }

bool Project::Item::isModuleCode() const                    { return belongsToModule; }

Value Project::Item::getShouldSkipPCHValue()                { return state.getPropertyAsValue (Ids::skipPCH, getUndoManager()); }
bool Project::Item::shouldSkipPCH() const                   { return isModuleCode() || state [Ids::skipPCH]; }

Value Project::Item::getCompilerFlagSchemeValue()           { return state.getPropertyAsValue (Ids::compilerFlagScheme, getUndoManager()); }

String Project::Item::getCompilerFlagSchemeString() const   { return state[Ids::compilerFlagScheme]; }

void Project::Item::setCompilerFlagScheme (const String& scheme)
{
    state.getPropertyAsValue (Ids::compilerFlagScheme, getUndoManager()).setValue (scheme);
}

void Project::Item::clearCurrentCompilerFlagScheme()
{
    state.removeProperty (Ids::compilerFlagScheme, getUndoManager());
}

String Project::Item::getFilePath() const
{
    if (isFile())
        return state [Ids::file].toString();

    return {};
}

File Project::Item::getFile() const
{
    if (isFile())
        return project.resolveFilename (state [Ids::file].toString());

    return {};
}

void Project::Item::setFile (const File& file)
{
    setFile (build_tools::RelativePath (project.getRelativePathForFile (file), build_tools::RelativePath::projectFolder));
    jassert (getFile() == file);
}

void Project::Item::setFile (const build_tools::RelativePath& file)
{
    jassert (isFile());
    state.setProperty (Ids::file, file.toUnixStyle(), getUndoManager());
    state.setProperty (Ids::name, file.getFileName(), getUndoManager());
}

bool Project::Item::renameFile (const File& newFile)
{
    auto oldFile = getFile();

    if (oldFile.moveFileTo (newFile)
         || (newFile.exists() && ! oldFile.exists()))
    {
        setFile (newFile);
        ProjucerApplication::getApp().openDocumentManager.fileHasBeenRenamed (oldFile, newFile);
        return true;
    }

    return false;
}

bool Project::Item::containsChildForFile (const build_tools::RelativePath& file) const
{
    return state.getChildWithProperty (Ids::file, file.toUnixStyle()).isValid();
}

Project::Item Project::Item::findItemForFile (const File& file) const
{
    if (getFile() == file)
        return *this;

    if (isGroup())
    {
        for (auto i = getNumChildren(); --i >= 0;)
        {
            auto found = getChild (i).findItemForFile (file);

            if (found.isValid())
                return found;
        }
    }

    return Item (project, ValueTree(), false);
}

File Project::Item::determineGroupFolder() const
{
    jassert (isGroup());
    File f;

    for (int i = 0; i < getNumChildren(); ++i)
    {
        f = getChild (i).getFile();

        if (f.exists())
            return f.getParentDirectory();
    }

    auto parent = getParent();
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
        for (auto i = getNumChildren(); --i >= 0;)
            getChild (i).initialiseMissingProperties();
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

    return { project, state.getParent(), belongsToModule };
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
        auto firstIsGroup = first.hasType (Ids::GROUP);
        auto secondIsGroup = second.hasType (Ids::GROUP);

        if (firstIsGroup == secondIsGroup)
            return first [Ids::name].toString().compareNatural (second [Ids::name].toString());

        return firstIsGroup ? -1 : 1;
    }
};

static void sortGroup (ValueTree& state, bool keepGroupsAtStart, UndoManager* undoManager)
{
    if (keepGroupsAtStart)
    {
        ItemSorterWithGroupsAtStart sorter;
        state.sort (sorter, undoManager, true);
    }
    else
    {
        ItemSorter sorter;
        state.sort (sorter, undoManager, true);
    }
}

static bool isGroupSorted (const ValueTree& state, bool keepGroupsAtStart)
{
    if (state.getNumChildren() == 0)
        return false;

    if (state.getNumChildren() == 1)
        return true;

    auto stateCopy = state.createCopy();
    sortGroup (stateCopy, keepGroupsAtStart, nullptr);
    return stateCopy.isEquivalentTo (state);
}

void Project::Item::sortAlphabetically (bool keepGroupsAtStart, bool recursive)
{
    sortGroup (state, keepGroupsAtStart, getUndoManager());

    if (recursive)
        for (auto i = getNumChildren(); --i >= 0;)
            getChild (i).sortAlphabetically (keepGroupsAtStart, true);
}

Project::Item Project::Item::getOrCreateSubGroup (const String& name)
{
    for (auto i = state.getNumChildren(); --i >= 0;)
    {
        auto child = state.getChild (i);
        if (child.getProperty (Ids::name) == name && child.hasType (Ids::GROUP))
            return { project, child, belongsToModule };
    }

    return addNewSubGroup (name, -1);
}

Project::Item Project::Item::addNewSubGroup (const String& name, int insertIndex)
{
    auto newID = createGUID (getID() + name + String (getNumChildren()));

    int n = 0;
    while (project.getMainGroup().findItemWithID (newID).isValid())
        newID = createGUID (newID + String (++n));

    auto group = createGroup (project, name, newID, belongsToModule);

    jassert (canContain (group));
    addChild (group, insertIndex);
    return group;
}

bool Project::Item::addFileAtIndex (const File& file, int insertIndex, const bool shouldCompile)
{
    if (file == File() || file.isHidden() || file.getFileName().startsWithChar ('.'))
        return false;

    if (file.isDirectory())
    {
        auto group = addNewSubGroup (file.getFileName(), insertIndex);

        for (const auto& iter : RangedDirectoryIterator (file, false, "*", File::findFilesAndDirectories))
            if (! project.getMainGroup().findItemForFile (iter.getFile()).isValid())
                group.addFileRetainingSortOrder (iter.getFile(), shouldCompile);
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

bool Project::Item::addFileRetainingSortOrder (const File& file, bool shouldCompile)
{
    auto wasSortedGroupsNotFirst = isGroupSorted (state, false);
    auto wasSortedGroupsFirst    = isGroupSorted (state, true);

    if (! addFileAtIndex (file, 0, shouldCompile))
        return false;

    if (wasSortedGroupsNotFirst || wasSortedGroupsFirst)
        sortAlphabetically (wasSortedGroupsFirst, false);

    return true;
}

void Project::Item::addFileUnchecked (const File& file, int insertIndex, const bool shouldCompile)
{
    Item item (project, ValueTree (Ids::FILE), belongsToModule);
    item.initialiseMissingProperties();
    item.getNameValue() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile && file.hasFileExtension (fileTypesToCompileByDefault);
    item.getShouldAddToBinaryResourcesValue() = project.shouldBeAddedToBinaryResourcesByDefault (file);

    if (canContain (item))
    {
        item.setFile (file);
        addChild (item, insertIndex);
    }
}

bool Project::Item::addRelativeFile (const build_tools::RelativePath& file, int insertIndex, bool shouldCompile)
{
    Item item (project, ValueTree (Ids::FILE), belongsToModule);
    item.initialiseMissingProperties();
    item.getNameValue() = file.getFileName();
    item.getShouldCompileValue() = shouldCompile;
    item.getShouldAddToBinaryResourcesValue() = project.shouldBeAddedToBinaryResourcesByDefault (file);

    if (canContain (item))
    {
        item.setFile (file);
        addChild (item, insertIndex);
        return true;
    }

    return false;
}

Icon Project::Item::getIcon (bool isOpen) const
{
    auto& icons = getIcons();

    if (isFile())
    {
        if (isImageFile())
            return Icon (icons.imageDoc, Colours::transparentBlack);

        return { icons.file, Colours::transparentBlack };
    }

    return { isOpen ? icons.openFolder : icons.closedFolder, Colours::transparentBlack };
}

bool Project::Item::isIconCrossedOut() const
{
    return isFile()
            && ! (shouldBeCompiled()
                   || shouldBeAddedToBinaryResources()
                   || getFile().hasFileExtension (headerFileExtensions));
}

bool Project::Item::needsSaving() const noexcept
{
    auto& odm = ProjucerApplication::getApp().openDocumentManager;

    if (odm.anyFilesNeedSaving())
    {
        for (int i = 0; i < odm.getNumOpenDocuments(); ++i)
        {
            auto* doc = odm.getOpenDocument (i);
            if (doc->needsSaving() && doc->getFile() == getFile())
                return true;
        }
    }

    return false;
}

//==============================================================================
ValueTree Project::getConfigNode()
{
    return projectRoot.getOrCreateChildWithName (Ids::JUCEOPTIONS, nullptr);
}

ValueTreePropertyWithDefault Project::getConfigFlag (const String& name)
{
    auto configNode = getConfigNode();

    return { configNode, name, getUndoManagerFor (configNode) };
}

bool Project::isConfigFlagEnabled (const String& name, bool defaultIsEnabled) const
{
    auto configValue = projectRoot.getChildWithName (Ids::JUCEOPTIONS).getProperty (name, "default");

    if (configValue == "default")
        return defaultIsEnabled;

    return configValue;
}

//==============================================================================
StringArray Project::getCompilerFlagSchemes() const
{
    if (compilerFlagSchemesValue.isUsingDefault())
        return {};

    StringArray schemes;
    auto schemesVar = compilerFlagSchemesValue.get();

    if (auto* arr = schemesVar.getArray())
        schemes.addArray (arr->begin(), arr->end());

    return schemes;
}

void Project::addCompilerFlagScheme (const String& schemeToAdd)
{
    auto schemesVar = compilerFlagSchemesValue.get();

    if (auto* arr = schemesVar.getArray())
    {
        arr->addIfNotAlreadyThere (schemeToAdd);
        compilerFlagSchemesValue.setValue ({ *arr }, getUndoManager());
    }
}

void Project::removeCompilerFlagScheme (const String& schemeToRemove)
{
    auto schemesVar = compilerFlagSchemesValue.get();

    if (auto* arr = schemesVar.getArray())
    {
        for (int i = 0; i < arr->size(); ++i)
        {
            if (arr->getUnchecked (i).toString() == schemeToRemove)
            {
                arr->remove (i);

                if (arr->isEmpty())
                    compilerFlagSchemesValue.resetToDefault();
                else
                    compilerFlagSchemesValue.setValue ({ *arr }, getUndoManager());

                return;
            }
        }
    }
}

//==============================================================================
static String getCompanyNameOrDefault (StringRef str)
{
    if (str.isEmpty())
        return "yourcompany";

    return str;
}

String Project::getDefaultBundleIdentifierString() const
{
    return "com." + build_tools::makeValidIdentifier (getCompanyNameOrDefault (getCompanyNameString()), false, true, false)
            + "." + build_tools::makeValidIdentifier (getProjectNameString(), false, true, false);
}

String Project::getDefaultCompanyWebsiteString() const
{
    return "www." + build_tools::makeValidIdentifier (getCompanyNameOrDefault (getCompanyNameString()), false, true, false) + ".com";
}

String Project::getDefaultPluginManufacturerString() const
{
    return getCompanyNameOrDefault (getCompanyNameString());
}

String Project::getDefaultARAFactoryIDString() const
{
    return getDefaultBundleIdentifierString() + ".factory";
}

String Project::getDefaultARADocumentArchiveID() const
{
    return getDefaultBundleIdentifierString() + ".aradocumentarchive." + getVersionString();
}

String Project::getDefaultARACompatibleArchiveIDs() const
{
    return String();
}

String Project::getAUMainTypeString() const noexcept
{
    auto v = pluginAUMainTypeValue.get();

    if (auto* arr = v.getArray())
        return arr->getFirst().toString();

    jassertfalse;
    return {};
}

bool Project::isAUSandBoxSafe() const noexcept
{
    return pluginAUSandboxSafeValue.get();
}

String Project::getVSTCategoryString() const noexcept
{
    auto v = pluginVSTCategoryValue.get();

    if (auto* arr = v.getArray())
        return arr->getFirst().toString();

    jassertfalse;
    return {};
}

static String getVST3CategoryStringFromSelection (Array<var> selected, const Project& p) noexcept
{
    StringArray categories;

    for (auto& category : selected)
        categories.add (category);

    // One of these needs to be selected in order for the plug-in to be recognised in Cubase
    if (! categories.contains ("Fx") && ! categories.contains ("Instrument"))
    {
        categories.insert (0, p.isPluginSynth() ? "Instrument"
                                                : "Fx");
    }
    else
    {
        // "Fx" and "Instrument" should come first and if both are present prioritise "Fx"
        if (categories.contains ("Instrument"))
            categories.move (categories.indexOf ("Instrument"), 0);

        if (categories.contains ("Fx"))
            categories.move (categories.indexOf ("Fx"), 0);
    }

    return categories.joinIntoString ("|");
}

String Project::getVST3CategoryString() const noexcept
{
    auto v = pluginVST3CategoryValue.get();

    if (auto* arr = v.getArray())
        return getVST3CategoryStringFromSelection (*arr, *this);

    jassertfalse;
    return {};
}

int Project::getAAXCategory() const noexcept
{
    int res = 0;

    auto v = pluginAAXCategoryValue.get();

    if (auto* arr = v.getArray())
    {
        for (auto c : *arr)
            res |= static_cast<int> (c);
    }

    return res;
}

String Project::getIAATypeCode() const
{
    String s;
    if (pluginWantsMidiInput())
    {
        if (isPluginSynth())
            s = "auri";
        else
            s = "aurm";
    }
    else
    {
        if (isPluginSynth())
            s = "aurg";
        else
            s = "aurx";
    }
    return s;
}

String Project::getIAAPluginName() const
{
    auto s = getPluginManufacturerString();
    s << ": ";
    s << getPluginNameString();
    return s;
}

int Project::getARAContentTypes() const noexcept
{
    int res = 0;

    if (const auto analyzableContent = pluginARAAnalyzableContentValue.get();
        auto* arr = analyzableContent.getArray())
    {
        for (auto c : *arr)
            res |= (int) c;
    }

    return res;
}

int Project::getARATransformationFlags() const noexcept
{
    int res = 0;

    if (const auto transformFlags = pluginARATransformFlagsValue.get();
        auto* arr = transformFlags.getArray())
    {
        for (auto c : *arr)
            res |= (int) c;
    }

    return res;
}

//==============================================================================
bool Project::isAUPluginHost() const
{
    return getEnabledModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_AU", false);
}

bool Project::isVSTPluginHost() const
{
    return getEnabledModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_VST", false);
}

bool Project::isVST3PluginHost() const
{
    return getEnabledModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_VST3", false);
}

bool Project::isLV2PluginHost() const
{
    return getEnabledModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_LV2", false);
}

bool Project::isARAPluginHost() const
{
    return (isVST3PluginHost() || isAUPluginHost()) && isConfigFlagEnabled ("JUCE_PLUGINHOST_ARA", false);
}

void Project::disableStandaloneForARAPlugIn()
{
    pluginFormatsValue.referTo (projectRoot, Ids::pluginFormats, getUndoManager(), Array<var> (Ids::buildVST3.toString(), Ids::buildAU.toString()), ",");
}

//==============================================================================
StringArray Project::getAllAUMainTypeStrings() noexcept
{
    static StringArray auMainTypeStrings { "kAudioUnitType_Effect", "kAudioUnitType_FormatConverter", "kAudioUnitType_Generator", "kAudioUnitType_MIDIProcessor",
                                           "kAudioUnitType_Mixer", "kAudioUnitType_MusicDevice", "kAudioUnitType_MusicEffect", "kAudioUnitType_OfflineEffect",
                                           "kAudioUnitType_Output", "kAudioUnitType_Panner" };

    return auMainTypeStrings;
}

Array<var> Project::getAllAUMainTypeVars() noexcept
{
    static Array<var> auMainTypeVars { "'aufx'", "'aufc'", "'augn'", "'aumi'",
                                       "'aumx'", "'aumu'", "'aumf'", "'auol'",
                                       "'auou'", "'aupn'" };

    return auMainTypeVars;
}

Array<var> Project::getDefaultAUMainTypes() const noexcept
{
    if (isPluginMidiEffect())      return { "'aumi'" };
    if (isPluginSynth())           return { "'aumu'" };
    if (pluginWantsMidiInput())    return { "'aumf'" };

    return { "'aufx'" };
}

StringArray Project::getAllVSTCategoryStrings() noexcept
{
    static StringArray vstCategoryStrings { "kPlugCategUnknown", "kPlugCategEffect", "kPlugCategSynth", "kPlugCategAnalysis", "kPlugCategMastering",
                                            "kPlugCategSpacializer", "kPlugCategRoomFx", "kPlugSurroundFx", "kPlugCategRestoration", "kPlugCategOfflineProcess",
                                            "kPlugCategShell", "kPlugCategGenerator" };
    return vstCategoryStrings;
}

Array<var> Project::getDefaultVSTCategories() const noexcept
{
    if (isPluginSynth())
        return  { "kPlugCategSynth" };

    return { "kPlugCategEffect" };
}

StringArray Project::getAllVST3CategoryStrings() noexcept
{
    static StringArray vst3CategoryStrings { "Fx", "Instrument", "Analyzer", "Delay", "Distortion", "Drum", "Dynamics", "EQ", "External", "Filter",
                                             "Generator", "Mastering", "Modulation", "Mono", "Network", "NoOfflineProcess", "OnlyOfflineProcess", "OnlyRT",
                                             "Pitch Shift", "Restoration", "Reverb", "Sampler", "Spatial", "Stereo", "Surround", "Synth", "Tools", "Up-Downmix" };

    return vst3CategoryStrings;
}

Array<var> Project::getDefaultVST3Categories() const noexcept
{
    if (isPluginSynth())
        return  { "Instrument", "Synth" };

    return { "Fx" };
}

StringArray Project::getAllAAXCategoryStrings() noexcept
{
    static StringArray aaxCategoryStrings { "AAX_ePlugInCategory_None", "AAX_ePlugInCategory_EQ", "AAX_ePlugInCategory_Dynamics", "AAX_ePlugInCategory_PitchShift",
                                            "AAX_ePlugInCategory_Reverb", "AAX_ePlugInCategory_Delay", "AAX_ePlugInCategory_Modulation", "AAX_ePlugInCategory_Harmonic",
                                            "AAX_ePlugInCategory_NoiseReduction", "AAX_ePlugInCategory_Dither", "AAX_ePlugInCategory_SoundField", "AAX_ePlugInCategory_HWGenerators",
                                            "AAX_ePlugInCategory_SWGenerators", "AAX_ePlugInCategory_WrappedPlugin", "AAX_EPlugInCategory_Effect" };

    return aaxCategoryStrings;
}

Array<var> Project::getAllAAXCategoryVars() noexcept
{
    static Array<var> aaxCategoryVars { 0x00000000, 0x00000001, 0x00000002, 0x00000004,
                                        0x00000008, 0x00000010, 0x00000020, 0x00000040,
                                        0x00000080, 0x00000100, 0x00000200, 0x00000400,
                                        0x00000800, 0x00001000, 0x00002000 };

    return aaxCategoryVars;
}

Array<var> Project::getDefaultAAXCategories() const noexcept
{
    if (isPluginSynth())
        return getAllAAXCategoryVars()[getAllAAXCategoryStrings().indexOf ("AAX_ePlugInCategory_SWGenerators")];

    return getAllAAXCategoryVars()[getAllAAXCategoryStrings().indexOf ("AAX_ePlugInCategory_None")];
}

bool Project::getDefaultEnableARA() const noexcept
{
    return false;
}

StringArray Project::getAllARAContentTypeStrings() noexcept
{
    static StringArray araContentTypes { "Notes",
                                         "Tempo Entries",
                                         "Bar Signatures",
                                         "Static Tuning",
                                         "Dynamic Tuning Offsets",
                                         "Key Signatures",
                                         "Sheet Chords" };
    return araContentTypes;
}

Array<var> Project::getAllARAContentTypeVars() noexcept
{
    static Array<var> araContentVars {
        /*kARAContentTypeNotes =*/                1 << 0,
        /*kARAContentTypeTempoEntries =*/         1 << 1,
        /*kARAContentTypeBarSignatures =*/        1 << 2,
        /*kARAContentTypeStaticTuning =*/         1 << 3,
        /*kARAContentTypeDynamicTuningOffsets =*/ 1 << 4,
        /*kARAContentTypeKeySignatures =*/        1 << 5,
        /*kARAContentTypeSheetChords =*/          1 << 6,
    };
    return araContentVars;
}

Array<var> Project::getDefaultARAContentTypes() const noexcept
{
    return {};
}

StringArray Project::getAllARATransformationFlagStrings() noexcept
{
    static StringArray araTransformationFlags { "Time Stretch",
                                                "Time Stretch (reflecting tempo)",
                                                "Content Based Fades At Tail",
                                                "Content Based Fades At Head" };
    return araTransformationFlags;
}

Array<var> Project::getAllARATransformationFlagVars() noexcept
{
    static Array<var> araContentVars {
        /*kARAPlaybackTransformationTimestretch =*/                1 << 0,
        /*kARAPlaybackTransformationTimestretchReflectingTempo =*/ 1 << 1,
        /*kARAPlaybackTransformationContentBasedFadesAtTail =*/    1 << 2,
        /*kARAPlaybackTransformationContentBasedFadesAtHead =*/    1 << 3
    };
    return araContentVars;
}

Array<var> Project::getDefaultARATransformationFlags() const noexcept
{
    return {};
}

//==============================================================================
template <typename This>
auto& Project::getEnabledModulesImpl (This& t)
{
    // This won't work until you've loaded a project!
    jassert (t.enabledModulesList != nullptr);

    return *t.enabledModulesList;
}

      EnabledModulesList& Project::getEnabledModules()            { return getEnabledModulesImpl (*this); }
const EnabledModulesList& Project::getEnabledModules() const      { return getEnabledModulesImpl (*this); }

void Project::createEnabledModulesList()
{
    enabledModulesList = std::make_unique<EnabledModulesList> (*this, projectRoot.getOrCreateChildWithName (Ids::MODULES, nullptr));
}

static StringArray getModulePathsFromExporters (Project& project, bool onlyThisOS)
{
    StringArray paths;

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (onlyThisOS && ! exporter->mayCompileOnCurrentOS())
            continue;

        auto& modules = project.getEnabledModules();
        auto n = modules.getNumModules();

        for (int i = 0; i < n; ++i)
        {
            auto id = modules.getModuleID (i);

            if (modules.shouldUseGlobalPath (id))
                continue;

            auto path = exporter->getPathForModuleString (id);

            if (path.isNotEmpty())
                paths.addIfNotAlreadyThere (path);
        }

        auto oldPath = exporter->getLegacyModulePath();

        if (oldPath.isNotEmpty())
            paths.addIfNotAlreadyThere (oldPath);
    }

    return paths;
}

static Array<File> getExporterModulePathsToScan (Project& project)
{
    auto exporterPaths = getModulePathsFromExporters (project, true);

    if (exporterPaths.isEmpty())
        exporterPaths = getModulePathsFromExporters (project, false);

    Array<File> files;

    for (auto& path : exporterPaths)
    {
        auto f = project.resolveFilename (path);

        if (f.isDirectory())
        {
            files.addIfNotAlreadyThere (f);

            if (f.getChildFile ("modules").isDirectory())
                files.addIfNotAlreadyThere (f.getChildFile ("modules"));
        }
    }

    return files;
}

void Project::rescanExporterPathModules (bool async)
{
    if (async)
        exporterPathsModulesList.scanPathsAsync (getExporterModulePathsToScan (*this));
    else
        exporterPathsModulesList.scanPaths (getExporterModulePathsToScan (*this));
}

AvailableModulesList::ModuleIDAndFolder Project::getModuleWithID (const String& id)
{
    if (! getEnabledModules().shouldUseGlobalPath (id))
    {
        const auto& mod = exporterPathsModulesList.getModuleWithID (id);

        if (mod.second != File())
            return mod;
    }

    const auto& list = (isJUCEModule (id) ? ProjucerApplication::getApp().getJUCEPathModulesList().getAllModules()
                                          : ProjucerApplication::getApp().getUserPathsModulesList().getAllModules());

    for (auto& m : list)
        if (m.first == id)
            return m;

    return exporterPathsModulesList.getModuleWithID (id);
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

std::unique_ptr<ProjectExporter> Project::createExporter (int index)
{
    jassert (index >= 0 && index < getNumExporters());
    return ProjectExporter::createExporterFromSettings (*this, getExporters().getChild (index));
}

void Project::addNewExporter (const Identifier& exporterIdentifier)
{
    std::unique_ptr<ProjectExporter> exp (ProjectExporter::createNewExporter (*this, exporterIdentifier));

    exp->getTargetLocationValue() = exp->getTargetLocationString()
                                       + getUniqueTargetFolderSuffixForExporter (exporterIdentifier, exp->getTargetLocationString());

    auto exportersTree = getExporters();
    exportersTree.appendChild (exp->settings, getUndoManagerFor (exportersTree));
}

void Project::createExporterForCurrentPlatform()
{
    addNewExporter (ProjectExporter::getCurrentPlatformExporterTypeInfo().identifier);
}

String Project::getUniqueTargetFolderSuffixForExporter (const Identifier& exporterIdentifier, const String& base)
{
    StringArray buildFolders;

    auto exportersTree = getExporters();

    for (int i = 0; i < exportersTree.getNumChildren(); ++i)
    {
        auto exporterNode = exportersTree.getChild (i);

        if (exporterNode.getType() == exporterIdentifier)
            buildFolders.add (exporterNode.getProperty ("targetFolder").toString());
    }

    if (buildFolders.size() == 0 || ! buildFolders.contains (base))
        return {};

    buildFolders.remove (buildFolders.indexOf (base));

    int num = 1;
    for (auto f : buildFolders)
    {
        if (! f.endsWith ("_" + String (num)))
            break;

        ++num;
    }

    return "_" + String (num);
}

//==============================================================================
StringPairArray Project::getAppConfigDefs()
{
    StringPairArray result;
    result.set ("JUCE_PROJUCER_VERSION",       "0x" + String::toHexString (ProjectInfo::versionNumber));

    OwnedArray<LibraryModule> modules;
    getEnabledModules().createRequiredModules (modules);

    for (auto& m : modules)
        result.set ("JUCE_MODULE_AVAILABLE_" + m->getID(), "1");

    result.set ("JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED", "1");

    for (auto& m : modules)
    {
        OwnedArray<Project::ConfigFlag> flags;
        m->getConfigFlags (*this, flags);

        for (auto* flag : flags)
            if (! flag->value.isUsingDefault())
                result.set (flag->symbol, flag->value.get() ? "1" : "0");
    }

    result.addArray (getAudioPluginFlags());

    const auto& type = getProjectType();
    const auto isStandaloneApplication = (! type.isAudioPlugin() && ! type.isDynamicLibrary());

    const auto standaloneValue = [&]
    {
        if (result.containsKey ("JucePlugin_Name") && result.containsKey ("JucePlugin_Build_Standalone"))
            return "JucePlugin_Build_Standalone";

        return isStandaloneApplication ? "1" : "0";
    }();

    result.set ("JUCE_STANDALONE_APPLICATION", standaloneValue);

    return result;
}

StringPairArray Project::getAudioPluginFlags() const
{
    if (! isAudioPluginProject())
        return {};

    const auto boolToString = [] (bool b) { return b ? "1" : "0"; };

    const auto toStringLiteral = [] (const String& v)
    {
        return CppTokeniserFunctions::addEscapeChars (v).quoted();
    };

    const auto countMaxPluginChannels = [] (const String& configString, bool isInput)
    {
        auto configs = StringArray::fromTokens (configString, ", {}", {});
        configs.trim();
        configs.removeEmptyStrings();
        jassert ((configs.size() & 1) == 0);  // looks like a syntax error in the configs?

        int maxVal = 0;

        for (int i = (isInput ? 0 : 1); i < configs.size(); i += 2)
            maxVal = jmax (maxVal, configs[i].getIntValue());

        return maxVal;
    };

    const auto toCharLiteral = [] (const String& v)
    {
        auto fourCharCode = v.substring (0, 4);
        uint32 hexRepresentation = 0;

        for (int i = 0; i < 4; ++i)
        {
            const auto character = (unsigned int) (i < fourCharCode.length() ? fourCharCode[i] : 0);
            hexRepresentation = (hexRepresentation << 8u) | (character & 0xffu);
        }

        return "0x" + String::toHexString (static_cast<int> (hexRepresentation));
    };

    StringPairArray flags;
    flags.set ("JucePlugin_Build_VST",                   boolToString (shouldBuildVST()));
    flags.set ("JucePlugin_Build_VST3",                  boolToString (shouldBuildVST3()));
    flags.set ("JucePlugin_Build_AU",                    boolToString (shouldBuildAU()));
    flags.set ("JucePlugin_Build_AUv3",                  boolToString (shouldBuildAUv3()));
    flags.set ("JucePlugin_Build_AAX",                   boolToString (shouldBuildAAX()));
    flags.set ("JucePlugin_Build_Standalone",            boolToString (shouldBuildStandalonePlugin()));
    flags.set ("JucePlugin_Build_Unity",                 boolToString (shouldBuildUnityPlugin()));
    flags.set ("JucePlugin_Build_LV2",                   boolToString (shouldBuildLV2()));
    flags.set ("JucePlugin_Enable_IAA",                  boolToString (shouldEnableIAA()));
    flags.set ("JucePlugin_Enable_ARA",                  boolToString (shouldEnableARA()));
    flags.set ("JucePlugin_Name",                        toStringLiteral (getPluginNameString()));
    flags.set ("JucePlugin_Desc",                        toStringLiteral (getPluginDescriptionString()));
    flags.set ("JucePlugin_Manufacturer",                toStringLiteral (getPluginManufacturerString()));
    flags.set ("JucePlugin_ManufacturerWebsite",         toStringLiteral (getCompanyWebsiteString()));
    flags.set ("JucePlugin_ManufacturerEmail",           toStringLiteral (getCompanyEmailString()));
    flags.set ("JucePlugin_ManufacturerCode",            toCharLiteral (getPluginManufacturerCodeString()));
    flags.set ("JucePlugin_PluginCode",                  toCharLiteral (getPluginCodeString()));
    flags.set ("JucePlugin_IsSynth",                     boolToString (isPluginSynth()));
    flags.set ("JucePlugin_WantsMidiInput",              boolToString (pluginWantsMidiInput()));
    flags.set ("JucePlugin_ProducesMidiOutput",          boolToString (pluginProducesMidiOutput()));
    flags.set ("JucePlugin_IsMidiEffect",                boolToString (isPluginMidiEffect()));
    flags.set ("JucePlugin_EditorRequiresKeyboardFocus", boolToString (pluginEditorNeedsKeyFocus()));
    flags.set ("JucePlugin_Version",                     getVersionString());
    flags.set ("JucePlugin_VersionCode",                 getVersionAsHex());
    flags.set ("JucePlugin_VersionString",               toStringLiteral (getVersionString()));
    flags.set ("JucePlugin_VSTUniqueID",                 "JucePlugin_PluginCode");
    flags.set ("JucePlugin_VSTCategory",                 getVSTCategoryString());
    flags.set ("JucePlugin_Vst3Category",                toStringLiteral (getVST3CategoryString()));
    flags.set ("JucePlugin_AUMainType",                  getAUMainTypeString());
    flags.set ("JucePlugin_AUSubType",                   "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AUExportPrefix",              getPluginAUExportPrefixString());
    flags.set ("JucePlugin_AUExportPrefixQuoted",        toStringLiteral (getPluginAUExportPrefixString()));
    flags.set ("JucePlugin_AUManufacturerCode",          "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_CFBundleIdentifier",          getBundleIdentifierString());
    flags.set ("JucePlugin_AAXIdentifier",               getAAXIdentifierString());
    flags.set ("JucePlugin_AAXManufacturerCode",         "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_AAXProductId",                "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AAXCategory",                 String (getAAXCategory()));
    flags.set ("JucePlugin_AAXDisableBypass",            boolToString (isPluginAAXBypassDisabled()));
    flags.set ("JucePlugin_AAXDisableMultiMono",         boolToString (isPluginAAXMultiMonoDisabled()));
    flags.set ("JucePlugin_IAAType",                     toCharLiteral (getIAATypeCode()));
    flags.set ("JucePlugin_IAASubType",                  "JucePlugin_PluginCode");
    flags.set ("JucePlugin_IAAName",                     toStringLiteral (getIAAPluginName()));
    flags.set ("JucePlugin_VSTNumMidiInputs",            getVSTNumMIDIInputsString());
    flags.set ("JucePlugin_VSTNumMidiOutputs",           getVSTNumMIDIOutputsString());
    flags.set ("JucePlugin_ARAContentTypes",             String (getARAContentTypes()));
    flags.set ("JucePlugin_ARATransformationFlags",      String (getARATransformationFlags()));
    flags.set ("JucePlugin_ARAFactoryID",                toStringLiteral (getARAFactoryIDString()));
    flags.set ("JucePlugin_ARADocumentArchiveID",        toStringLiteral (getARADocumentArchiveIDString()));
    flags.set ("JucePlugin_ARACompatibleArchiveIDs",     toStringLiteral (getARACompatibleArchiveIDStrings()));

    {
        String plugInChannelConfig = getPluginChannelConfigsString();

        if (plugInChannelConfig.isNotEmpty())
        {
            flags.set ("JucePlugin_MaxNumInputChannels",            String (countMaxPluginChannels (plugInChannelConfig, true)));
            flags.set ("JucePlugin_MaxNumOutputChannels",           String (countMaxPluginChannels (plugInChannelConfig, false)));
            flags.set ("JucePlugin_PreferredChannelConfigurations", plugInChannelConfig);
        }
    }

    return flags;
}

//==============================================================================
Project::ExporterIterator::ExporterIterator (Project& p) : index (-1), project (p) {}

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
