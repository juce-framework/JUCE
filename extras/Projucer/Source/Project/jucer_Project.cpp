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

#include "../Application/jucer_Headers.h"
#include "jucer_Project.h"
#include "../ProjectSaving/jucer_ProjectSaver.h"
#include "../Application/jucer_Application.h"

namespace
{
    String makeValid4CC (const String& seed)
    {
        auto s = CodeHelpers::makeValidIdentifier (seed, false, true, false) + "xxxx";

        return s.substring (0, 1).toUpperCase()
             + s.substring (1, 4).toLowerCase();
    }
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

    removeDefunctExporters();
    updateOldModulePaths();
    updateOldStyleConfigList();
    setCppVersionFromOldExporterSettings();
    moveOldPropertyFromProjectToAllExporters (Ids::bigIcon);
    moveOldPropertyFromProjectToAllExporters (Ids::smallIcon);

    initialiseProjectValues();
    initialiseMainGroup();
    initialiseAudioPluginValues();

    parsedPreprocessorDefs = parsePreprocessorDefs (preprocessorDefsValue.get());

    getModules().sortAlphabetically();

    projectRoot.addListener (this);

    setChangedFlag (false);
    modificationTime = getFile().getLastModificationTime();
}

Project::~Project()
{
    projectRoot.removeListener (this);
    ProjucerApplication::getApp().openDocumentManager.closeAllDocumentsUsingProject (*this, false);
}

const char* Project::projectFileExtension = ".jucer";

//==============================================================================
void Project::setTitle (const String& newTitle)
{
    projectRoot.setProperty (Ids::name, newTitle, getUndoManager());
    getMainGroup().getNameValue() = newTitle;

    bundleIdentifierValue.setDefault (getDefaultBundleIdentifierString());
    pluginAAXIdentifierValue.setDefault (getDefaultAAXIdentifierString());
}

String Project::getDocumentTitle()
{
    return getProjectNameString();
}

void Project::updateProjectSettings()
{
    projectRoot.setProperty (Ids::jucerVersion, ProjectInfo::versionString, nullptr);
    projectRoot.setProperty (Ids::name, getDocumentTitle(), nullptr);
}

bool Project::setCppVersionFromOldExporterSettings()
{
    auto highestLanguageStandard = -1;

    for (Project::ExporterIterator exporter (*this); exporter.next();)
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
                    cppStandardValue = "latest";
                    return true;
                }

                auto versionNum = cppLanguageStandard.toString().getLastCharacters (2).getIntValue();

                if (versionNum > highestLanguageStandard)
                    highestLanguageStandard = versionNum;
            }
        }
    }

    if (highestLanguageStandard != -1 && highestLanguageStandard >= 11)
    {
        cppStandardValue = highestLanguageStandard;
        return true;
    }

    return false;
}

void Project::updateDeprecatedProjectSettingsInteractively()
{
    jassert (! ProjucerApplication::getApp().isRunningCommandLine);

    for (Project::ExporterIterator exporter (*this); exporter.next();)
        exporter->updateDeprecatedProjectSettingsInteractively();
}

void Project::initialiseMainGroup()
{
    // Create main file group if missing
    if (! projectRoot.getChildWithName (Ids::MAINGROUP).isValid())
    {
        Item mainGroup (*this, ValueTree (Ids::MAINGROUP), false);
        projectRoot.addChild (mainGroup.state, 0, 0);
    }

    getMainGroup().initialiseMissingProperties();
}

void Project::initialiseProjectValues()
{
    projectNameValue.referTo         (projectRoot, Ids::name,             getUndoManager(), "JUCE Project");
    projectUIDValue.referTo          (projectRoot, Ids::ID,               getUndoManager(), createAlphaNumericUID());
    projectTypeValue.referTo         (projectRoot, Ids::projectType,      getUndoManager(), ProjectType_GUIApp::getTypeName());
    versionValue.referTo             (projectRoot, Ids::version,          getUndoManager(), "1.0.0");
    bundleIdentifierValue.referTo    (projectRoot, Ids::bundleIdentifier, getUndoManager(), getDefaultBundleIdentifierString());

    companyNameValue.referTo         (projectRoot, Ids::companyName,      getUndoManager());
    companyCopyrightValue.referTo    (projectRoot, Ids::companyCopyright, getUndoManager());
    companyWebsiteValue.referTo      (projectRoot, Ids::companyWebsite,   getUndoManager());
    companyEmailValue.referTo        (projectRoot, Ids::companyEmail,     getUndoManager());

    displaySplashScreenValue.referTo (projectRoot, Ids::displaySplashScreen, getUndoManager(), ! ProjucerApplication::getApp().isPaidOrGPL());
    splashScreenColourValue.referTo  (projectRoot, Ids::splashScreenColour,  getUndoManager(), "Dark");
    reportAppUsageValue.referTo      (projectRoot, Ids::reportAppUsage,      getUndoManager());

    if (ProjucerApplication::getApp().isPaidOrGPL())
    {
        reportAppUsageValue.setDefault (ProjucerApplication::getApp().licenseController->getState().applicationUsageDataState
                                        == LicenseState::ApplicationUsageData::enabled);
    }
    else
    {
        reportAppUsageValue.setDefault (true);
    }

    cppStandardValue.referTo                   (projectRoot, Ids::cppLanguageStandard, getUndoManager(), "14");

    headerSearchPathsValue.referTo             (projectRoot, Ids::headerPath, getUndoManager());
    preprocessorDefsValue.referTo              (projectRoot, Ids::defines,    getUndoManager());
    userNotesValue.referTo                     (projectRoot, Ids::userNotes,  getUndoManager());

    maxBinaryFileSizeValue.referTo             (projectRoot, Ids::maxBinaryFileSize,        getUndoManager(), 10240 * 1024);
    includeBinaryDataInAppConfigValue.referTo  (projectRoot, Ids::includeBinaryInAppConfig, getUndoManager(), true);
    binaryDataNamespaceValue.referTo           (projectRoot, Ids::binaryDataNamespace,      getUndoManager(), "BinaryData");
}

void Project::initialiseAudioPluginValues()
{
    buildVSTValue.referTo                    (projectRoot, Ids::buildVST,        getUndoManager(), true);
    buildVST3Value.referTo                   (projectRoot, Ids::buildVST3,       getUndoManager(), false);
    buildAUValue.referTo                     (projectRoot, Ids::buildAU,         getUndoManager(), true);
    buildAUv3Value.referTo                   (projectRoot, Ids::buildAUv3,       getUndoManager(), false);
    buildRTASValue.referTo                   (projectRoot, Ids::buildRTAS,       getUndoManager(), false);
    buildAAXValue.referTo                    (projectRoot, Ids::buildAAX,        getUndoManager(), false);
    buildStandaloneValue.referTo             (projectRoot, Ids::buildStandalone, getUndoManager(), false);
    enableIAAValue.referTo                   (projectRoot, Ids::enableIAA,       getUndoManager(), false);

    pluginNameValue.referTo                  (projectRoot, Ids::pluginName,             getUndoManager(), getProjectNameString());
    pluginDescriptionValue.referTo           (projectRoot, Ids::pluginDesc,             getUndoManager(), getProjectNameString());
    pluginManufacturerValue.referTo          (projectRoot, Ids::pluginManufacturer,     getUndoManager(), "yourcompany");
    pluginManufacturerCodeValue.referTo      (projectRoot, Ids::pluginManufacturerCode, getUndoManager(), "Manu");
    pluginCodeValue.referTo                  (projectRoot, Ids::pluginCode,             getUndoManager(), makeValid4CC (getProjectUIDString() + getProjectUIDString()));
    pluginChannelConfigsValue.referTo        (projectRoot, Ids::pluginChannelConfigs,   getUndoManager());

    pluginIsSynthValue.referTo               (projectRoot, Ids::pluginIsSynth,            getUndoManager(), false);
    pluginWantsMidiInputValue.referTo        (projectRoot, Ids::pluginWantsMidiIn,        getUndoManager(), false);
    pluginProducesMidiOutValue.referTo       (projectRoot, Ids::pluginProducesMidiOut,    getUndoManager(), false);
    pluginIsMidiEffectPluginValue.referTo    (projectRoot, Ids::pluginIsMidiEffectPlugin, getUndoManager(), false);
    pluginEditorNeedsKeyFocusValue.referTo   (projectRoot, Ids::pluginEditorRequiresKeys, getUndoManager(), false);

    pluginVSTCategoryValue.referTo           (projectRoot, Ids::pluginVSTCategory,          getUndoManager());
    pluginAUExportPrefixValue.referTo        (projectRoot, Ids::pluginAUExportPrefix,       getUndoManager(),
                                              CodeHelpers::makeValidIdentifier (getProjectNameString(), false, true, false) + "AU");
    pluginAUMainTypeValue.referTo            (projectRoot, Ids::pluginAUMainType,           getUndoManager());
    pluginRTASCategoryValue.referTo          (projectRoot, Ids::pluginRTASCategory,         getUndoManager());
    pluginRTASBypassDisabledValue.referTo    (projectRoot, Ids::pluginRTASDisableBypass,    getUndoManager());
    pluginRTASMultiMonoDisabledValue.referTo (projectRoot, Ids::pluginRTASDisableMultiMono, getUndoManager());
    pluginAAXIdentifierValue.referTo         (projectRoot, Ids::aaxIdentifier,              getUndoManager(), getDefaultAAXIdentifierString());
    pluginAAXCategoryValue.referTo           (projectRoot, Ids::pluginAAXCategory,          getUndoManager(), "AAX_ePlugInCategory_Dynamics");
    pluginAAXBypassDisabledValue.referTo     (projectRoot, Ids::pluginAAXDisableBypass,     getUndoManager());
    pluginAAXMultiMonoDisabledValue.referTo  (projectRoot, Ids::pluginAAXDisableMultiMono,  getUndoManager());
}

void Project::updateOldStyleConfigList()
{
    auto deprecatedConfigsList = projectRoot.getChildWithName (Ids::CONFIGURATIONS);

    if (deprecatedConfigsList.isValid())
    {
        projectRoot.removeChild (deprecatedConfigsList, nullptr);

        for (Project::ExporterIterator exporter (*this); exporter.next();)
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
        for (Project::ExporterIterator exporter (*this); exporter.next();)
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

    for (auto& key : oldExporters.getAllKeys())
    {
        auto oldExporter = exporters.getChildWithName (key);

        if (oldExporter.isValid())
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS (oldExporters[key]),
                                         TRANS ("The " + oldExporters[key]  + " Exporter is deprecated. The exporter will be removed from this project."));
            exporters.removeChild (oldExporter, nullptr);
        }
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
    StringArray parts = StringArray::fromTokens (v, "., ", {});

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

static bool isAnyModuleNewerThanProjucer (const OwnedArray<ModuleDescription>& modules)
{
    for (auto i = modules.size(); --i >= 0;)
    {
        auto* m = modules.getUnchecked(i);

        if (m->getID().startsWith ("juce_")
              && getJuceVersion (m->getVersion()) > getBuiltJuceVersion())
            return true;
    }

    return false;
}

void Project::warnAboutOldProjucerVersion()
{
    ModuleList available;

    available.scanGlobalJuceModulePath();

    if (! isAnyModuleNewerThanProjucer (available.modules))
        available.scanGlobalUserModulePath();

    if (! isAnyModuleNewerThanProjucer (available.modules))
        available.scanProjectExporterModulePaths (*this);

    if (! isAnyModuleNewerThanProjucer (available.modules))
        return;

    // Projucer is out of date!
    if (ProjucerApplication::getApp().isRunningCommandLine)
        std::cout <<  "WARNING! This version of the Projucer is out-of-date!" << std::endl;
    else
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Projucer",
                                          "This version of the Projucer is out-of-date!"
                                          "\n\n"
                                          "Always make sure that you're running the very latest version, "
                                          "preferably compiled directly from the JUCE repository that you're working with!");
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

static void forgetRecentFile (const File& file)
{
    RecentlyOpenedFilesList::forgetRecentFileNatively (file);
    getAppSettings().recentFiles.removeFile (file);
    getAppSettings().flush();
}

//==============================================================================
Result Project::loadDocument (const File& file)
{
    ScopedPointer<XmlElement> xml (XmlDocument::parse (file));

    if (xml == nullptr || ! xml->hasTagName (Ids::JUCERPROJECT.toString()))
        return Result::fail ("Not a valid Jucer project!");

    auto newTree = ValueTree::fromXml (*xml);

    if (! newTree.hasType (Ids::JUCERPROJECT))
        return Result::fail ("The document contains errors and couldn't be parsed!");

    registerRecentFile (file);

    enabledModulesList.reset();
    projectRoot = newTree;

    initialiseProjectValues();
    initialiseMainGroup();
    initialiseAudioPluginValues();

    parsedPreprocessorDefs = parsePreprocessorDefs (preprocessorDefsValue.get());

    removeDefunctExporters();
    updateOldModulePaths();
    setChangedFlag (false);

    if (! ProjucerApplication::getApp().isRunningCommandLine)
        warnAboutOldProjucerVersion();

    return Result::ok();
}

Result Project::saveDocument (const File& file)
{
    return saveProject (file, false);
}

Result Project::saveProject (const File& file, bool isCommandLineApp)
{
    if (isSaving)
        return Result::ok();

    if (isTemporaryProject())
    {
        askUserWhereToSaveProject();
        return Result::ok();
    }

    updateProjectSettings();

    if (! isCommandLineApp && ! isTemporaryProject())
        registerRecentFile (file);

    const ScopedValueSetter<bool> vs (isSaving, true, false);

    ProjectSaver saver (*this, file);
    return saver.save (! isCommandLineApp, shouldWaitAfterSaving, specifiedExporterToSave);
}

Result Project::saveResourcesOnly (const File& file)
{
    ProjectSaver saver (*this, file);
    return saver.saveResourcesOnly();
}

//==============================================================================
void Project::setTemporaryDirectory (const File& dir) noexcept
{
    tempDirectory = dir;

     // remove this file from the recent documents list as it is a temporary project
    forgetRecentFile (getFile());
}

void Project::askUserWhereToSaveProject()
{
    FileChooser fc ("Save Project");
    fc.browseForDirectory();

    if (fc.getResult().exists())
        moveTemporaryDirectory (fc.getResult());
}

void Project::moveTemporaryDirectory (const File& newParentDirectory)
{
    auto newDirectory = newParentDirectory.getChildFile (tempDirectory.getFileName());
    auto oldJucerFileName = getFile().getFileName();

    saveProjectRootToFile();

    tempDirectory.copyDirectoryTo (newDirectory);
    tempDirectory.deleteRecursively();
    tempDirectory = File();

    // reload project from new location
    if (auto* window = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (getFile()))
    {
        Component::SafePointer<MainWindow> safeWindow (window);

        MessageManager::callAsync ([safeWindow, newDirectory, oldJucerFileName]
        {
            if (safeWindow != nullptr)
                safeWindow.getComponent()->moveProject (newDirectory.getChildFile (oldJucerFileName));
        });
    }
}

bool Project::saveProjectRootToFile()
{
    ScopedPointer<XmlElement> xml (projectRoot.createXml());

    if (xml == nullptr)
    {
        jassertfalse;
        return false;
    }

    MemoryOutputStream mo;
    xml->writeToStream (mo, {});

    return FileHelpers::overwriteFileWithNewDataIfDifferent (getFile(), mo);
}

//==============================================================================
static void sendProjectSettingAnalyticsEvent (StringRef label)
{
    StringPairArray data;
    data.set ("label", label);

    Analytics::getInstance()->logEvent ("Project Setting",  data, ProjucerAnalyticsEvent::projectEvent);
}

void Project::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (tree.getRoot() == tree)
    {
        if (property == Ids::projectType)
        {
            sendChangeMessage();

            sendProjectSettingAnalyticsEvent ("Project Type = " + projectTypeValue.get().toString());
        }
        else if (property == Ids::name)
        {
            setTitle (projectRoot [Ids::name]);
        }
        else if (property == Ids::defines)
        {
            parsedPreprocessorDefs = parsePreprocessorDefs (preprocessorDefsValue.get());
        }
        else if (property == Ids::cppLanguageStandard)
        {
            sendProjectSettingAnalyticsEvent ("C++ Standard = " + cppStandardValue.get().toString());
        }

        changed();
    }
}

void Project::valueTreeChildAdded (ValueTree&, ValueTree&)          { changed(); }
void Project::valueTreeChildRemoved (ValueTree&, ValueTree&, int)   { changed(); }
void Project::valueTreeChildOrderChanged (ValueTree&, int, int)     { changed(); }
void Project::valueTreeParentChanged (ValueTree&)                   {}

//==============================================================================
bool Project::hasProjectBeenModified()
{
    auto oldModificationTime = modificationTime;
    modificationTime = getFile().getLastModificationTime();

    return (modificationTime.toMilliseconds() > (oldModificationTime.toMilliseconds() + 1000LL));
}

//==============================================================================
File Project::resolveFilename (String filename) const
{
    if (filename.isEmpty())
        return {};

    filename = replacePreprocessorDefs (getPreprocessorDefs(), filename);

   #if ! JUCE_WINDOWS
    if (filename.startsWith ("~"))
        return File::getSpecialLocation (File::userHomeDirectory).getChildFile (filename.trimCharactersAtStart ("~/"));
   #endif

    if (FileHelpers::isAbsolutePath (filename))
        return File::createFileWithoutCheckingPath (FileHelpers::currentOSStylePath (filename)); // (avoid assertions for windows-style paths)

    return getFile().getSiblingFile (FileHelpers::currentOSStylePath (filename));
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
        filename = FileHelpers::getRelativePathFrom (file, relativePathBase);
    }

    return filename;
}

//==============================================================================
const ProjectType& Project::getProjectType() const
{
    if (auto* type = ProjectType::findType (getProjectTypeString()))
        return *type;

    auto* guiType = ProjectType::findType (ProjectType_GUIApp::getTypeName());
    jassert (guiType != nullptr);
    return *guiType;
}

bool Project::shouldBuildTargetType (ProjectType::Target::Type targetType) const noexcept
{
    auto& projectType = getProjectType();

    if (! projectType.supportsTargetType (targetType))
        return false;

    switch (targetType)
    {
        case ProjectType::Target::VSTPlugIn:
            return shouldBuildVST();
        case ProjectType::Target::VST3PlugIn:
            return shouldBuildVST3();
        case ProjectType::Target::AAXPlugIn:
            return shouldBuildAAX();
        case ProjectType::Target::RTASPlugIn:
            return shouldBuildRTAS();
        case ProjectType::Target::AudioUnitPlugIn:
            return shouldBuildAU();
        case ProjectType::Target::AudioUnitv3PlugIn:
            return shouldBuildAUv3();
        case ProjectType::Target::StandalonePlugIn:
            return shouldBuildStandalonePlugin();
        case ProjectType::Target::AggregateTarget:
        case ProjectType::Target::SharedCodeTarget:
            return projectType.isAudioPlugin();
        case ProjectType::Target::unspecified:
            return false;
        default:
            break;
    }

    return true;
}

ProjectType::Target::Type Project::getTargetTypeFromFilePath (const File& file, bool returnSharedTargetIfNoValidSuffix)
{
    if      (LibraryModule::CompileUnit::hasSuffix (file, "_AU"))         return ProjectType::Target::AudioUnitPlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_AUv3"))       return ProjectType::Target::AudioUnitv3PlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_AAX"))        return ProjectType::Target::AAXPlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_RTAS"))       return ProjectType::Target::RTASPlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_VST2"))       return ProjectType::Target::VSTPlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_VST3"))       return ProjectType::Target::VST3PlugIn;
    else if (LibraryModule::CompileUnit::hasSuffix (file, "_Standalone")) return ProjectType::Target::StandalonePlugIn;

    return (returnSharedTargetIfNoValidSuffix ? ProjectType::Target::SharedCodeTarget : ProjectType::Target::unspecified);
}

const char* ProjectType::Target::getName() const noexcept
{
    switch (type)
    {
        case GUIApp:            return "App";
        case ConsoleApp:        return "ConsoleApp";
        case StaticLibrary:     return "Static Library";
        case DynamicLibrary:    return "Dynamic Library";
        case VSTPlugIn:         return "VST";
        case VST3PlugIn:        return "VST3";
        case AudioUnitPlugIn:   return "AU";
        case StandalonePlugIn:  return "Standalone Plugin";
        case AudioUnitv3PlugIn: return "AUv3 AppExtension";
        case AAXPlugIn:         return "AAX";
        case RTASPlugIn:        return "RTAS";
        case SharedCodeTarget:  return "Shared Code";
        case AggregateTarget:   return "All";
        default:                return "undefined";
    }
}

ProjectType::Target::TargetFileType ProjectType::Target::getTargetFileType() const noexcept
{
    switch (type)
    {
        case GUIApp:            return executable;
        case ConsoleApp:        return executable;
        case StaticLibrary:     return staticLibrary;
        case DynamicLibrary:    return sharedLibraryOrDLL;
        case VSTPlugIn:         return pluginBundle;
        case VST3PlugIn:        return pluginBundle;
        case AudioUnitPlugIn:   return pluginBundle;
        case StandalonePlugIn:  return executable;
        case AudioUnitv3PlugIn: return macOSAppex;
        case AAXPlugIn:         return pluginBundle;
        case RTASPlugIn:        return pluginBundle;
        case SharedCodeTarget:  return staticLibrary;
        default:
            break;
    }

    return unknown;
}

//==============================================================================
void Project::createPropertyEditors (PropertyListBuilder& props)
{
    props.add (new TextPropertyComponent (projectNameValue, "Project Name", 256, false),
               "The name of the project.");

    props.add (new TextPropertyComponent (versionValue, "Project Version", 16, false),
               "The project's version number, This should be in the format major.minor.point[.point]");

    props.add (new TextPropertyComponent (companyNameValue, "Company Name", 256, false),
               "Your company name, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyCopyrightValue, "Company Copyright", 256, false),
               "Your company copyright, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyWebsiteValue, "Company Website", 256, false),
               "Your company website, which will be added to the properties of the binary where possible");

    props.add (new TextPropertyComponent (companyEmailValue, "Company E-mail", 256, false),
               "Your company e-mail, which will be added to the properties of the binary where possible");

    {
        String licenseRequiredTagline ("Required for closed source applications without an Indie or Pro JUCE license");
        String licenseRequiredInfo ("In accordance with the terms of the JUCE 5 End-Use License Agreement (www.juce.com/juce-5-licence), "
                                    "this option can only be disabled for closed source applications if you have a JUCE Indie or Pro "
                                    "license, or are using JUCE under the GPL v3 license.");

        StringPairArray description;
        description.set ("Report JUCE app usage", "This option controls the collection of usage data from users of this JUCE application.");
        description.set ("Display the JUCE splash screen", "This option controls the display of the standard JUCE splash screen.");

        if (ProjucerApplication::getApp().isPaidOrGPL())
        {
            props.add (new ChoicePropertyComponent (reportAppUsageValue, String ("Report JUCE App Usage") + " (" + licenseRequiredTagline + ")"),
                       description["Report JUCE app usage"] + " " + licenseRequiredInfo);

            props.add (new ChoicePropertyComponent (displaySplashScreenValue, String ("Display the JUCE Splash Screen") + " (" + licenseRequiredTagline + ")"),
                       description["Display the JUCE splash screen"] + " " + licenseRequiredInfo);
        }
        else
        {
            StringArray options;
            Array<var> vars;

            options.add (licenseRequiredTagline);
            vars.add (var());

            props.add (new ChoicePropertyComponent (Value(), "Report JUCE App Usage", options, vars),
                       description["Report JUCE app usage"] + " " + licenseRequiredInfo);

            props.add (new ChoicePropertyComponent (Value(), "Display the JUCE Splash Screen", options, vars),
                       description["Display the JUCE splash screen"] + " " + licenseRequiredInfo);
        }
    }

    props.add (new ChoicePropertyComponent (splashScreenColourValue, "Splash Screen Colour",
                                            { "Dark", "Light" },
                                            { "Dark", "Light" }),
               "Choose the colour of the JUCE splash screen.");

    {
        StringArray projectTypeNames;
        Array<var> projectTypeCodes;

        auto types = ProjectType::getAllTypes();

        for (int i = 0; i < types.size(); ++i)
        {
            projectTypeNames.add (types.getUnchecked(i)->getDescription());
            projectTypeCodes.add (types.getUnchecked(i)->getType());
        }

        props.add (new ChoicePropertyComponent (projectTypeValue, "Project Type", projectTypeNames, projectTypeCodes),
                   "The project type for which settings should be shown.");
    }

    props.add (new TextPropertyComponent (bundleIdentifierValue, "Bundle Identifier", 256, false),
               "A unique identifier for this product, mainly for use in OSX/iOS builds. It should be something like 'com.yourcompanyname.yourproductname'");

    if (getProjectType().isAudioPlugin())
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

    props.add (new ChoicePropertyComponent (includeBinaryDataInAppConfigValue, "Include BinaryData in AppConfig"),
                                             "Include BinaryData.h in the AppConfig.h file");

    props.add (new TextPropertyComponent (binaryDataNamespaceValue, "BinaryData Namespace", 256, false),
                                          "The namespace containing the binary assests.");

    props.add (new ChoicePropertyComponent (cppStandardValue, "C++ Language Standard",
                                            { "C++11", "C++14", "C++17", "Use Latest" },
                                            { "11",    "14",    "17",    "latest" }),
               "The standard of the C++ language that will be used for compilation.");

    props.add (new TextPropertyComponent (preprocessorDefsValue, "Preprocessor Definitions", 32768, true),
               "Global preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace, commas, or "
               "new-lines to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.addSearchPathProperty (headerSearchPathsValue, "Header Search Paths", "Global header search paths.");

    props.add (new TextPropertyComponent (userNotesValue, "Notes", 32768, true),
               "Extra comments: This field is not used for code or project generation, it's just a space where you can express your thoughts.");
}

void Project::createAudioPluginPropertyEditors (PropertyListBuilder& props)
{
    props.add (new ChoicePropertyComponent (buildVSTValue, "Build VST"),
               "Whether the project should produce a VST plugin.");
    props.add (new ChoicePropertyComponent (buildVST3Value, "Build VST3"),
               "Whether the project should produce a VST3 plugin.");
    props.add (new ChoicePropertyComponent (buildAUValue, "Build AudioUnit"),
               "Whether the project should produce an AudioUnit plugin.");
    props.add (new ChoicePropertyComponent (buildAUv3Value, "Build AudioUnit v3"),
               "Whether the project should produce an AudioUnit version 3 plugin.");
    props.add (new ChoicePropertyComponent (buildRTASValue, "Build RTAS"),
               "Whether the project should produce an RTAS plugin.");
    props.add (new ChoicePropertyComponent (buildAAXValue, "Build AAX"),
               "Whether the project should produce an AAX plugin.");
    props.add (new ChoicePropertyComponent (buildStandaloneValue, "Build Standalone Plug-In"),
               "Whether the project should produce a standalone version of your plugin.");
    props.add (new ChoicePropertyComponent (enableIAAValue, "Enable Inter-App Audio"),
               "Whether a standalone plug-in should be an Inter-App Audio app. You should also enable the audio "
               "background capability in the iOS exporter.");

    props.add (new TextPropertyComponent (pluginNameValue, "Plugin Name", 128, false),
               "The name of your plugin (keep it short!)");
    props.add (new TextPropertyComponent (pluginDescriptionValue, "Plugin Description", 256, false),
               "A short description of your plugin.");

    props.add (new TextPropertyComponent (pluginManufacturerValue, "Plugin Manufacturer", 256, false),
               "The name of your company (cannot be blank).");
    props.add (new TextPropertyComponent (pluginManufacturerCodeValue, "Plugin Manufacturer Code", 4, false),
               "A four-character unique ID for your company. Note that for AU compatibility, this must contain at least one upper-case letter!");
    props.add (new TextPropertyComponent (pluginCodeValue, "Plugin Code", 4, false),
               "A four-character unique ID for your plugin. Note that for AU compatibility, this must contain at least one upper-case letter!");

    props.add (new TextPropertyComponent (pluginChannelConfigsValue, "Plugin Channel Configurations", 1024, false),
               "This list is a comma-separated set list in the form {numIns, numOuts} and each pair indicates a valid plug-in "
               "configuration. For example {1, 1}, {2, 2} means that the plugin can be used either with 1 input and 1 output, "
               "or with 2 inputs and 2 outputs. If your plug-in requires side-chains, aux output buses etc., then you must leave "
               "this field empty and override the isBusesLayoutSupported callback in your AudioProcessor.");

    props.add (new ChoicePropertyComponent (pluginIsSynthValue, "Plugin is a Synth"),
               "Enable this if you want your plugin to be treated as a synth or generator. It doesn't make much difference to the plugin itself, but some hosts treat synths differently to other plugins.");

    props.add (new ChoicePropertyComponent (pluginWantsMidiInputValue, "Plugin Midi Input"),
               "Enable this if you want your plugin to accept midi messages.");

    props.add (new ChoicePropertyComponent (pluginProducesMidiOutValue, "Plugin Midi Output"),
               "Enable this if your plugin is going to produce midi messages.");

    props.add (new ChoicePropertyComponent (pluginIsMidiEffectPluginValue, "Midi Effect Plugin"),
               "Enable this if your plugin only processes midi and no audio.");

    props.add (new ChoicePropertyComponent (pluginEditorNeedsKeyFocusValue, "Plugin Editor Requires Keyboard Focus"),
               "Enable this if your plugin needs keyboard input - some hosts can be a bit funny about keyboard focus..");

    props.add (new TextPropertyComponent (pluginAUExportPrefixValue, "Plugin AU Export Prefix", 128, false),
               "A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");

    props.add (new TextPropertyComponent (pluginAUMainTypeValue, "Plugin AU Main Type", 128, false),
               "In an AU, this is the value that is set as JucePlugin_AUMainType. Leave it blank unless you want to use a custom value.");

    props.add (new TextPropertyComponent (pluginVSTCategoryValue, "VST Category", 128, false),
               "In a VST, this is the value that is set as JucePlugin_VSTCategory. Leave it blank unless you want to use a custom value.");

    props.add (new TextPropertyComponent (pluginRTASCategoryValue, "Plugin RTAS Category", 128, false),
               "(Leave this blank if your plugin is a synth). This is one of the RTAS categories from FicPluginEnums.h, such as: ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics, "
               "ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay, "
               "ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction, "
               "ePlugInCategory_Dither, ePlugInCategory_SoundField");

    props.add (new TextPropertyComponent (pluginAAXCategoryValue, "Plugin AAX Category", 128, false),
               "This is one of the categories from the AAX_EPlugInCategory enum");

    props.add (new TextPropertyComponent (pluginAAXIdentifierValue, "Plugin AAX Identifier", 256, false),
               "The value to use for the JucePlugin_AAXIdentifier setting");
}

//==============================================================================
static StringArray getVersionSegments (const Project& p)
{
    auto segments = StringArray::fromTokens (p.getVersionString(), ",.", "");
    segments.trim();
    segments.removeEmptyStrings();
    return segments;
}

int Project::getVersionAsHexInteger() const
{
    auto segments = getVersionSegments (*this);

    auto value = (segments[0].getIntValue() << 16)
               + (segments[1].getIntValue() << 8)
               +  segments[2].getIntValue();

    if (segments.size() >= 4)
        value = (value << 8) + segments[3].getIntValue();

    return value;
}

String Project::getVersionAsHex() const
{
    return "0x" + String::toHexString (getVersionAsHexInteger());
}

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

Drawable* Project::Item::loadAsImageFile() const
{
    const MessageManagerLock mml (ThreadPoolJob::getCurrentThreadPoolJob());

    if (! mml.lockWasGained())
        return nullptr;

    return isValid() ? Drawable::createFromImageFile (getFile())
                     : nullptr;
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

Project::Item Project::Item::findItemWithID (const String& targetId) const
{
    if (state [Ids::ID] == targetId)
        return *this;

    if (isGroup())
    {
        for (auto i = getNumChildren(); --i >= 0;)
        {
            auto found = getChild(i).findItemWithID (targetId);

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

Value Project::Item::getShouldCompileValue()                { return state.getPropertyAsValue (Ids::compile, getUndoManager()); }
bool Project::Item::shouldBeCompiled() const                { return state [Ids::compile]; }

Value Project::Item::getShouldAddToBinaryResourcesValue()   { return state.getPropertyAsValue (Ids::resource, getUndoManager()); }
bool Project::Item::shouldBeAddedToBinaryResources() const  { return state [Ids::resource]; }

Value Project::Item::getShouldAddToXcodeResourcesValue()    { return state.getPropertyAsValue (Ids::xcodeResource, getUndoManager()); }
bool Project::Item::shouldBeAddedToXcodeResources() const   { return state [Ids::xcodeResource]; }

Value Project::Item::getShouldInhibitWarningsValue()        { return state.getPropertyAsValue (Ids::noWarnings, getUndoManager()); }
bool Project::Item::shouldInhibitWarnings() const           { return state [Ids::noWarnings]; }

bool Project::Item::isModuleCode() const                    { return belongsToModule; }

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
        for (auto i = getNumChildren(); --i >= 0;)
        {
            auto found = getChild(i).findItemForFile (file);

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
        f = getChild(i).getFile();

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
            getChild(i).sortAlphabetically (keepGroupsAtStart, true);
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

        for (DirectoryIterator iter (file, false, "*", File::findFilesAndDirectories); iter.next();)
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

bool Project::Item::addRelativeFile (const RelativePath& file, int insertIndex, bool shouldCompile)
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

    if (isMainGroup())
        return { icons.juceLogo, Colours::orange };

    return { isOpen ? icons.openFolder : icons.closedFolder, Colours::transparentBlack };
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

ValueWithDefault Project::getConfigFlag (const String& name)
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
String Project::getPluginRTASCategoryCode()
{
    if (static_cast<bool> (isPluginSynth()))
        return "ePlugInCategory_SWGenerators";

    auto s = getPluginRTASCategoryString();
    if (s.isEmpty())
        s = "ePlugInCategory_None";

    return s;
}

String Project::getAUMainTypeString()
{
    auto s = getPluginAUMainTypeString();

    if (s.isEmpty())
    {
        // Unfortunately, Rez uses a header where kAudioUnitType_MIDIProcessor is undefined
        // Use aumi instead.
        if      (isPluginMidiEffect())      s = "'aumi'";
        else if (isPluginSynth())           s = "kAudioUnitType_MusicDevice";
        else if (pluginWantsMidiInput())    s = "kAudioUnitType_MusicEffect";
        else                                s = "kAudioUnitType_Effect";
    }

    return s;
}

String Project::getAUMainTypeCode()
{
    auto s = getPluginAUMainTypeString();

    if (s.isEmpty())
    {
        if      (isPluginMidiEffect())      s = "aumi";
        else if (isPluginSynth())           s = "aumu";
        else if (pluginWantsMidiInput())    s = "aumf";
        else                                s = "aufx";
    }

    return s;
}

String Project::getIAATypeCode()
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

String Project::getIAAPluginName()
{
    auto s = getPluginManufacturerString();
    s << ": ";
    s << getPluginNameString();
    return s;
}

String Project::getPluginVSTCategoryString()
{
    auto s = pluginVSTCategoryValue.get().toString().trim();

    if (s.isEmpty())
        s = isPluginSynth() ? "kPlugCategSynth" : "kPlugCategEffect";
    return s;
}

bool Project::isAUPluginHost()
{
    return getModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_AU");
}

bool Project::isVSTPluginHost()
{
    return getModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_VST");
}

bool Project::isVST3PluginHost()
{
    return getModules().isModuleEnabled ("juce_audio_processors") && isConfigFlagEnabled ("JUCE_PLUGINHOST_VST3");
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

    exp->getTargetLocationValue() = exp->getTargetLocationString()
                                       + getUniqueTargetFolderSuffixForExporter (exp->getName(), exp->getTargetLocationString());

    auto exportersTree = getExporters();
    exportersTree.appendChild (exp->settings, getUndoManagerFor (exportersTree));
}

void Project::createExporterForCurrentPlatform()
{
    addNewExporter (ProjectExporter::getCurrentPlatformExporterName());
}

String Project::getUniqueTargetFolderSuffixForExporter (const String& exporterName, const String& base)
{
    StringArray buildFolders;

    auto exportersTree = getExporters();
    auto type = ProjectExporter::getValueTreeNameForExporter (exporterName);

    for (int i = 0; i < exportersTree.getNumChildren(); ++i)
    {
        auto exporterNode = exportersTree.getChild (i);

        if (exporterNode.getType() == Identifier (type))
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
bool Project::shouldSendGUIBuilderAnalyticsEvent() noexcept
{
    if (! hasSentGUIBuilderAnalyticsEvent)
    {
        hasSentGUIBuilderAnalyticsEvent = true;
        return true;
    }

    return false;
}

//==============================================================================
String Project::getFileTemplate (const String& templateName)
{
    int dataSize;

    if (auto* data = BinaryData::getNamedResource (templateName.toUTF8(), dataSize))
        return String::fromUTF8 (data, dataSize);

    jassertfalse;
    return {};

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
