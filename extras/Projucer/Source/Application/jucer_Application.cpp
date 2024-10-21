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

//==============================================================================
struct ProjucerApplication::MainMenuModel final : public MenuBarModel
{
    MainMenuModel()
    {
        setApplicationCommandManagerToWatch (&getCommandManager());
    }

    StringArray getMenuBarNames() override
    {
        return getApp().getMenuNames();
    }

    PopupMenu getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName) override
    {
        return getApp().createMenu (menuName);
    }

    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override
    {
        getApp().handleMainMenuCommand (menuItemID);
    }
};

//==============================================================================
void ProjucerApplication::initialise (const String& commandLine)
{
    initialiseLogger ("IDE_Log_");
    Logger::writeToLog (SystemStats::getOperatingSystemName());
    Logger::writeToLog ("CPU: " + String (SystemStats::getCpuSpeedInMegahertz())
                          + "MHz  Cores: " + String (SystemStats::getNumCpus())
                          + "  " + String (SystemStats::getMemorySizeInMegabytes()) + "MB");

    isRunningCommandLine = commandLine.isNotEmpty()
                            && ! commandLine.startsWith ("-NSDocumentRevisionsDebugMode");

    settings = std::make_unique<StoredSettings>();

    if (isRunningCommandLine)
    {
        auto appReturnCode = performCommandLine (ArgumentList ("Projucer", commandLine));

        if (appReturnCode != commandLineNotPerformed)
        {
            setApplicationReturnValue (appReturnCode);
            quit();
            return;
        }

        isRunningCommandLine = false;
    }

    if (sendCommandLineToPreexistingInstance())
    {
        DBG ("Another instance is running - quitting...");
        quit();
        return;
    }

    doBasicApplicationSetup();

    // do further initialisation in a moment when the message loop has started
    triggerAsyncUpdate();
}

bool ProjucerApplication::initialiseLogger (const char* filePrefix)
{
    if (logger == nullptr)
    {
       #if JUCE_LINUX || JUCE_BSD
        String folder = "~/.config/Projucer/Logs";
       #else
        String folder = "com.juce.projucer";
       #endif

        logger.reset (FileLogger::createDateStampedLogger (folder, filePrefix, ".txt",
                                                           getApplicationName() + " " + getApplicationVersion()
                                                               + "  ---  Build date: " __DATE__));
        Logger::setCurrentLogger (logger.get());
    }

    return logger != nullptr;
}

void ProjucerApplication::initialiseWindows (const String& commandLine)
{
    const String commandLineWithoutNSDebug (commandLine.replace ("-NSDocumentRevisionsDebugMode YES", StringRef()));

    if (commandLineWithoutNSDebug.trim().isNotEmpty() && ! commandLineWithoutNSDebug.trim().startsWithChar ('-'))
        anotherInstanceStarted (commandLine);
    else if (mainWindowList.windows.size() == 0)
        mainWindowList.reopenLastProjects();

    mainWindowList.createWindowIfNoneAreOpen();
}

void ProjucerApplication::handleAsyncUpdate()
{
    rescanJUCEPathModules();
    rescanUserPathModules();

    menuModel.reset (new MainMenuModel());

   #if JUCE_MAC
    rebuildAppleMenu();
    appleMenuRebuildListener = std::make_unique<AppleMenuRebuildListener>();
   #endif

    settings->appearance.refreshPresetSchemeList();
    setColourScheme (getGlobalProperties().getIntValue ("COLOUR SCHEME"), false);
    setEditorColourScheme (getGlobalProperties().getIntValue ("EDITOR COLOUR SCHEME"), false);
    updateEditorColourSchemeIfNeeded();

    ImageCache::setCacheTimeout (30 * 1000);
    tooltipWindow = std::make_unique<TooltipWindow> (nullptr, 1200);

    if (isAutomaticVersionCheckingEnabled())
        LatestVersionCheckerAndUpdater::getInstance()->checkForNewVersion (true);

    initialiseWindows (getCommandLineParameters());
}

void ProjucerApplication::doBasicApplicationSetup()
{
    LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
    initCommandManager();
    icons = std::make_unique<Icons>();
}

static void deleteTemporaryFiles()
{
    auto tempDirectory = File::getSpecialLocation (File::SpecialLocationType::tempDirectory).getChildFile ("PIPs");

    if (tempDirectory.exists())
        tempDirectory.deleteRecursively();
}

void ProjucerApplication::shutdown()
{
    utf8Window.reset();
    svgPathWindow.reset();
    aboutWindow.reset();
    pathsWindow.reset();
    editorColourSchemeWindow.reset();
    pipCreatorWindow.reset();

    mainWindowList.forceCloseAllWindows();
    openDocumentManager.clear();

   #if JUCE_MAC
    MenuBarModel::setMacMainMenu (nullptr);
   #endif

    menuModel.reset();
    commandManager.reset();
    settings.reset();

    if (! isRunningCommandLine)
        LookAndFeel::setDefaultLookAndFeel (nullptr);

    // clean up after ourselves and delete any temp project files that may have
    // been created from PIPs
    deleteTemporaryFiles();

    if (! isRunningCommandLine)
        Logger::writeToLog ("Shutdown");

    deleteLogger();
}

struct AsyncQuitRetrier final : private Timer
{
    AsyncQuitRetrier()   { startTimer (500); }

    void timerCallback() override
    {
        stopTimer();
        delete this;

        if (auto* app = JUCEApplicationBase::getInstance())
            app->systemRequestedQuit();
    }

    JUCE_DECLARE_NON_COPYABLE (AsyncQuitRetrier)
};

void ProjucerApplication::systemRequestedQuit()
{
    if (ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        new AsyncQuitRetrier();
    }
    else
    {
        closeAllMainWindows ([] (bool closedSuccessfully)
        {
            if (closedSuccessfully)
                ProjucerApplication::quit();
        });
    }
}

//==============================================================================
String ProjucerApplication::getVersionDescription() const
{
    String s;

    const Time buildDate (Time::getCompilationDate());

    s << "Projucer " << ProjectInfo::versionString
      << newLine
      << "Build date: " << buildDate.getDayOfMonth()
      << " " << Time::getMonthName (buildDate.getMonth(), true)
      << " " << buildDate.getYear();

    return s;
}

void ProjucerApplication::anotherInstanceStarted (const String& commandLine)
{
    if (! commandLine.trim().startsWithChar ('-'))
    {
        ArgumentList list ({}, commandLine);

        for (auto& arg : list.arguments)
            openFile (arg.resolveAsFile(), nullptr);
    }
}

ProjucerApplication& ProjucerApplication::getApp()
{
    ProjucerApplication* const app = dynamic_cast<ProjucerApplication*> (JUCEApplication::getInstance());
    jassert (app != nullptr);
    return *app;
}

ApplicationCommandManager& ProjucerApplication::getCommandManager()
{
    auto* cm = ProjucerApplication::getApp().commandManager.get();
    jassert (cm != nullptr);
    return *cm;
}


//==============================================================================
enum
{
    recentProjectsBaseID = 100,
    openWindowsBaseID = 300,
    activeDocumentsBaseID = 400,
    showPathsID = 1999,
    examplesBaseID = 2000
};

MenuBarModel* ProjucerApplication::getMenuModel()
{
    return menuModel.get();
}

StringArray ProjucerApplication::getMenuNames()
{
    return { "File", "Edit", "View", "Window", "Document", "Tools", "Help" };
}

PopupMenu ProjucerApplication::createMenu (const String& menuName)
{
    if (menuName == "File")
        return createFileMenu();

    if (menuName == "Edit")
        return createEditMenu();

    if (menuName == "View")
        return createViewMenu();

    if (menuName == "Window")
        return createWindowMenu();

    if (menuName == "Document")
        return createDocumentMenu();

    if (menuName == "Tools")
        return createToolsMenu();

    if (menuName == "Help")
        return createHelpMenu();

    jassertfalse; // names have changed?
    return {};
}

PopupMenu ProjucerApplication::createFileMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::newProject);
    menu.addCommandItem (commandManager.get(), CommandIDs::newProjectFromClipboard);
    menu.addCommandItem (commandManager.get(), CommandIDs::newPIP);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::open);

    {
        PopupMenu recentFiles;

        settings->recentFiles.createPopupMenuItems (recentFiles, recentProjectsBaseID, true, true);

        if (recentFiles.getNumItems() > 0)
        {
            recentFiles.addSeparator();
            recentFiles.addCommandItem (commandManager.get(), CommandIDs::clearRecentFiles);
        }

        menu.addSubMenu ("Open Recent", recentFiles);
    }

    menu.addSubMenu ("Open Example", createExamplesPopupMenu());

    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::closeDocument);
    menu.addCommandItem (commandManager.get(), CommandIDs::saveDocument);
    menu.addCommandItem (commandManager.get(), CommandIDs::saveDocumentAs);
    menu.addCommandItem (commandManager.get(), CommandIDs::saveAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::closeProject);
    menu.addCommandItem (commandManager.get(), CommandIDs::saveProject);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::openInIDE);
    menu.addCommandItem (commandManager.get(), CommandIDs::saveAndOpenInIDE);

   #if ! JUCE_MAC
    menu.addCommandItem (commandManager.get(), CommandIDs::showAboutWindow);
    menu.addCommandItem (commandManager.get(), CommandIDs::checkForNewVersion);
    menu.addCommandItem (commandManager.get(), CommandIDs::enableNewVersionCheck);
    menu.addCommandItem (commandManager.get(), CommandIDs::showGlobalPathsWindow);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::quit);
   #endif

    return menu;
}

PopupMenu ProjucerApplication::createEditMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::undo);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::redo);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::cut);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::copy);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::paste);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::del);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::selectAll);
    menu.addCommandItem (commandManager.get(), StandardApplicationCommandIDs::deselectAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::showFindPanel);
    menu.addCommandItem (commandManager.get(), CommandIDs::findSelection);
    menu.addCommandItem (commandManager.get(), CommandIDs::findNext);
    menu.addCommandItem (commandManager.get(), CommandIDs::findPrevious);
    return menu;
}

PopupMenu ProjucerApplication::createViewMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::showProjectSettings);
    menu.addCommandItem (commandManager.get(), CommandIDs::showFileExplorerPanel);
    menu.addCommandItem (commandManager.get(), CommandIDs::showModulesPanel);
    menu.addCommandItem (commandManager.get(), CommandIDs::showExportersPanel);
    menu.addCommandItem (commandManager.get(), CommandIDs::showExporterSettings);

    menu.addSeparator();
    createColourSchemeItems (menu);

    return menu;
}

void ProjucerApplication::createColourSchemeItems (PopupMenu& menu)
{
    {
        PopupMenu colourSchemeMenu;

        colourSchemeMenu.addItem (PopupMenu::Item ("Dark")
                                    .setTicked (selectedColourSchemeIndex == 0)
                                    .setAction ([this] { setColourScheme (0, true); updateEditorColourSchemeIfNeeded(); }));

        colourSchemeMenu.addItem (PopupMenu::Item ("Grey")
                                    .setTicked (selectedColourSchemeIndex == 1)
                                    .setAction ([this] { setColourScheme (1, true); updateEditorColourSchemeIfNeeded(); }));

        colourSchemeMenu.addItem (PopupMenu::Item ("Light")
                                    .setTicked (selectedColourSchemeIndex == 2)
                                    .setAction ([this] { setColourScheme (2, true); updateEditorColourSchemeIfNeeded(); }));

        menu.addSubMenu ("Colour Scheme", colourSchemeMenu);
    }

    {
        PopupMenu editorColourSchemeMenu;

        auto& appearanceSettings = getAppSettings().appearance;

        appearanceSettings.refreshPresetSchemeList();
        auto schemes = appearanceSettings.getPresetSchemes();

        auto i = 0;

        for (auto& s : schemes)
        {
            editorColourSchemeMenu.addItem (PopupMenu::Item (s)
                                               .setEnabled (editorColourSchemeWindow == nullptr)
                                               .setTicked (selectedEditorColourSchemeIndex == i)
                                               .setAction ([this, i] { setEditorColourScheme (i, true); }));
            ++i;
        }

        editorColourSchemeMenu.addSeparator();
        editorColourSchemeMenu.addItem (PopupMenu::Item ("Create...")
                                           .setEnabled (editorColourSchemeWindow == nullptr)
                                           .setAction ([this] { showEditorColourSchemeWindow(); }));

        menu.addSubMenu ("Editor Colour Scheme", editorColourSchemeMenu);
    }
}

PopupMenu ProjucerApplication::createWindowMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::goToPreviousWindow);
    menu.addCommandItem (commandManager.get(), CommandIDs::goToNextWindow);
    menu.addCommandItem (commandManager.get(), CommandIDs::closeWindow);
    menu.addSeparator();

    int counter = 0;

    for (auto* window : mainWindowList.windows)
    {
        if (window != nullptr)
        {
            if (auto* project = window->getProject())
                menu.addItem (openWindowsBaseID + counter++, project->getProjectNameString());
        }
    }

    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::closeAllWindows);
    return menu;
}

PopupMenu ProjucerApplication::createDocumentMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::goToPreviousDoc);
    menu.addCommandItem (commandManager.get(), CommandIDs::goToNextDoc);
    menu.addCommandItem (commandManager.get(), CommandIDs::goToCounterpart);
    menu.addSeparator();

    auto numDocs = jmin (50, openDocumentManager.getNumOpenDocuments());

    for (int i = 0; i < numDocs; ++i)
    {
        OpenDocumentManager::Document* doc = openDocumentManager.getOpenDocument (i);
        menu.addItem (activeDocumentsBaseID + i, doc->getName());
    }

    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::closeAllDocuments);
    return menu;
}

PopupMenu ProjucerApplication::createToolsMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::showUTF8Tool);
    menu.addCommandItem (commandManager.get(), CommandIDs::showSVGPathTool);
    menu.addCommandItem (commandManager.get(), CommandIDs::showTranslationTool);
    return menu;
}

PopupMenu ProjucerApplication::createHelpMenu()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::showForum);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::showAPIModules);
    menu.addCommandItem (commandManager.get(), CommandIDs::showAPIClasses);
    menu.addCommandItem (commandManager.get(), CommandIDs::showTutorials);
    return menu;
}

PopupMenu ProjucerApplication::createExtraAppleMenuItems()
{
    PopupMenu menu;
    menu.addCommandItem (commandManager.get(), CommandIDs::showAboutWindow);
    menu.addCommandItem (commandManager.get(), CommandIDs::checkForNewVersion);
    menu.addCommandItem (commandManager.get(), CommandIDs::enableNewVersionCheck);
    menu.addSeparator();
    menu.addCommandItem (commandManager.get(), CommandIDs::showGlobalPathsWindow);
    return menu;
}

PopupMenu ProjucerApplication::createExamplesPopupMenu() noexcept
{
    PopupMenu menu;
    numExamples = 0;
    for (auto& dir : getSortedExampleDirectories())
    {
        PopupMenu m;
        for (auto& f : getSortedExampleFilesInDirectory (dir))
        {
            m.addItem (examplesBaseID + numExamples, f.getFileNameWithoutExtension());
            ++numExamples;
        }

        menu.addSubMenu (dir.getFileName(), m);
    }

    if (numExamples == 0)
    {
        menu.addItem (showPathsID, "Set path to JUCE...");
    }
    else
    {
        menu.addSeparator();
        menu.addCommandItem (commandManager.get(), CommandIDs::launchDemoRunner);
    }

    return menu;
}

#if JUCE_MAC
 void ProjucerApplication::rebuildAppleMenu()
 {
     auto extraAppleMenuItems = createExtraAppleMenuItems();

     // workaround broken "Open Recent" submenu: not passing the
     // submenu's title here avoids the defect in JuceMainMenuHandler::addMenuItem
     MenuBarModel::setMacMainMenu (menuModel.get(), &extraAppleMenuItems); //, "Open Recent");
 }
#endif

//==============================================================================
File ProjucerApplication::getJUCEExamplesDirectoryPathFromGlobal() noexcept
{
    auto globalPath = File::createFileWithoutCheckingPath (getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString()
                                                                           .replace ("~", File::getSpecialLocation (File::userHomeDirectory).getFullPathName()));

    if (globalPath.exists())
        return File (globalPath).getChildFile ("examples");

    return {};
}

Array<File> ProjucerApplication::getSortedExampleDirectories() noexcept
{
    Array<File> exampleDirectories;

    auto examplesPath = getJUCEExamplesDirectoryPathFromGlobal();

    if (! isValidJUCEExamplesDirectory (examplesPath))
        return {};

    for (const auto& iter : RangedDirectoryIterator (examplesPath, false, "*", File::findDirectories))
    {
        auto exampleDirectory = iter.getFile();

        if (exampleDirectory.getNumberOfChildFiles (File::findFiles | File::ignoreHiddenFiles) > 0
            && exampleDirectory.getFileName() != "DemoRunner"
            && exampleDirectory.getFileName() != "Assets"
            && exampleDirectory.getFileName() != "CMake")
        {
            exampleDirectories.add (exampleDirectory);
        }
    }

    exampleDirectories.sort();

    return exampleDirectories;
}

Array<File> ProjucerApplication::getSortedExampleFilesInDirectory (const File& directory) noexcept
{
    Array<File> exampleFiles;

    for (const auto& iter : RangedDirectoryIterator (directory, false, "*.h", File::findFiles))
        exampleFiles.add (iter.getFile());

    exampleFiles.sort();

    return exampleFiles;
}

void ProjucerApplication::findAndLaunchExample (int selectedIndex)
{
    File example;

    for (auto& dir : getSortedExampleDirectories())
    {
        auto exampleFiles = getSortedExampleFilesInDirectory (dir);

        if (selectedIndex < exampleFiles.size())
        {
            example = exampleFiles.getUnchecked (selectedIndex);
            break;
        }

        selectedIndex -= exampleFiles.size();
    }

    // example doesn't exist?
    jassert (example != File());

    openFile (example, nullptr);
}

//==============================================================================
static String getPlatformSpecificFileExtension()
{
   #if JUCE_MAC
    return ".app";
   #elif JUCE_WINDOWS
    return ".exe";
   #elif JUCE_LINUX || JUCE_BSD
    return {};
   #else
    jassertfalse;
    return {};
   #endif
}

static File getPlatformSpecificProjectFolder()
{
    auto examplesDir = ProjucerApplication::getJUCEExamplesDirectoryPathFromGlobal();

    if (examplesDir == File())
        return {};

    auto buildsFolder = examplesDir.getChildFile ("DemoRunner").getChildFile ("Builds");

   #if JUCE_MAC
    return buildsFolder.getChildFile ("MacOSX");
   #elif JUCE_WINDOWS
    return buildsFolder.getChildFile ("VisualStudio2022");
   #elif JUCE_LINUX || JUCE_BSD
    return buildsFolder.getChildFile ("LinuxMakefile");
   #else
    jassertfalse;
    return {};
   #endif
}

static File tryToFindDemoRunnerExecutableInBuilds()
{
    auto projectFolder = getPlatformSpecificProjectFolder();

    if (projectFolder == File())
        return {};

   #if JUCE_MAC
    projectFolder = projectFolder.getChildFile ("build");
    auto demoRunnerExecutable = projectFolder.getChildFile ("Release").getChildFile ("DemoRunner.app");

    if (demoRunnerExecutable.exists())
        return demoRunnerExecutable;

    demoRunnerExecutable = projectFolder.getChildFile ("Debug").getChildFile ("DemoRunner.app");

    if (demoRunnerExecutable.exists())
        return demoRunnerExecutable;
   #elif JUCE_WINDOWS
    projectFolder = projectFolder.getChildFile ("x64");
    auto demoRunnerExecutable = projectFolder.getChildFile ("Release").getChildFile ("App").getChildFile ("DemoRunner.exe");

    if (demoRunnerExecutable.existsAsFile())
        return demoRunnerExecutable;

    demoRunnerExecutable = projectFolder.getChildFile ("Debug").getChildFile ("App").getChildFile ("DemoRunner.exe");

    if (demoRunnerExecutable.existsAsFile())
        return demoRunnerExecutable;
   #elif JUCE_LINUX || JUCE_BSD
    projectFolder = projectFolder.getChildFile ("build");
    auto demoRunnerExecutable = projectFolder.getChildFile ("DemoRunner");

    if (demoRunnerExecutable.existsAsFile())
        return demoRunnerExecutable;
   #endif

    return {};
}

static File tryToFindPrebuiltDemoRunnerExecutable()
{
    auto prebuiltFile = File (getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString())
                               .getChildFile ("DemoRunner" + getPlatformSpecificFileExtension());

   #if JUCE_MAC
    if (prebuiltFile.exists())
   #else
    if (prebuiltFile.existsAsFile())
   #endif
        return prebuiltFile;

    return {};
}

void ProjucerApplication::checkIfGlobalJUCEPathHasChanged()
{
    auto globalJUCEPath = File (getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get());

    if (lastJUCEPath != globalJUCEPath)
    {
        hasScannedForDemoRunnerProject = false;
        hasScannedForDemoRunnerExecutable = false;

        lastJUCEPath = globalJUCEPath;
    }
}

File ProjucerApplication::tryToFindDemoRunnerExecutable()
{
    checkIfGlobalJUCEPathHasChanged();

    if (hasScannedForDemoRunnerExecutable)
        return lastDemoRunnerExectuableFile;

    hasScannedForDemoRunnerExecutable = true;

    auto demoRunnerExecutable = tryToFindDemoRunnerExecutableInBuilds();

    if (demoRunnerExecutable == File())
        demoRunnerExecutable = tryToFindPrebuiltDemoRunnerExecutable();

    lastDemoRunnerExectuableFile = demoRunnerExecutable;

    return demoRunnerExecutable;
}

File ProjucerApplication::tryToFindDemoRunnerProject()
{
    checkIfGlobalJUCEPathHasChanged();

    if (hasScannedForDemoRunnerProject)
        return lastDemoRunnerProjectFile;

    hasScannedForDemoRunnerProject = true;

    auto projectFolder = getPlatformSpecificProjectFolder();

    if (projectFolder == File())
    {
        lastDemoRunnerProjectFile = File();
        return {};
    }

   #if JUCE_MAC
    auto demoRunnerProjectFile = projectFolder.getChildFile ("DemoRunner.xcodeproj");
   #elif JUCE_WINDOWS
    auto demoRunnerProjectFile = projectFolder.getChildFile ("DemoRunner.sln");
   #elif JUCE_LINUX || JUCE_BSD
    auto demoRunnerProjectFile = projectFolder.getChildFile ("Makefile");
   #endif

   #if JUCE_MAC
    if (! demoRunnerProjectFile.exists())
   #else
    if (! demoRunnerProjectFile.existsAsFile())
   #endif
        demoRunnerProjectFile = File();

    lastDemoRunnerProjectFile = demoRunnerProjectFile;

    return demoRunnerProjectFile;
}

void ProjucerApplication::launchDemoRunner()
{
    auto demoRunnerFile = tryToFindDemoRunnerExecutable();

    if (demoRunnerFile != File() && demoRunnerFile.startAsProcess())
        return;

    demoRunnerFile = tryToFindDemoRunnerProject();

    if (demoRunnerFile != File())
    {
        auto& lf = Desktop::getInstance().getDefaultLookAndFeel();

       #if JUCE_LINUX || JUCE_BSD
        demoRunnerAlert.reset (lf.createAlertWindow ("Open Project",
                                                     "Couldn't find a compiled version of the Demo Runner."
                                                     " Please compile the Demo Runner project in the JUCE examples directory.",
                                                     "OK", {}, {},
                                                     MessageBoxIconType::WarningIcon, 1,
                                                     mainWindowList.getFrontmostWindow (false)));
        demoRunnerAlert->enterModalState (true, ModalCallbackFunction::create ([this] (int)
                                                {
                                                    demoRunnerAlert.reset (nullptr);
                                                }), false);

       #else
        demoRunnerAlert.reset (lf.createAlertWindow ("Open Project",
                                                     "Couldn't find a compiled version of the Demo Runner."
                                                     " Do you want to open the project?",
                                                     "Open project", "Cancel", {},
                                                     MessageBoxIconType::QuestionIcon, 2,
                                                     mainWindowList.getFrontmostWindow (false)));
        demoRunnerAlert->enterModalState (true, ModalCallbackFunction::create ([this, demoRunnerFile] (int retVal)
                                                {
                                                    demoRunnerAlert.reset (nullptr);

                                                    if (retVal == 1)
                                                        demoRunnerFile.startAsProcess();
                                                }), false);
       #endif
    }
}

//==============================================================================
void ProjucerApplication::handleMainMenuCommand (int menuItemID)
{
    if (menuItemID >= recentProjectsBaseID && menuItemID < (recentProjectsBaseID + 100))
    {
        // open a file from the "recent files" menu
        openFile (settings->recentFiles.getFile (menuItemID - recentProjectsBaseID), nullptr);
    }
    else if (menuItemID >= openWindowsBaseID && menuItemID < (openWindowsBaseID + 100))
    {
        if (auto* window = mainWindowList.windows.getUnchecked (menuItemID - openWindowsBaseID))
            window->toFront (true);
    }
    else if (menuItemID >= activeDocumentsBaseID && menuItemID < (activeDocumentsBaseID + 200))
    {
        if (auto* doc = openDocumentManager.getOpenDocument (menuItemID - activeDocumentsBaseID))
            mainWindowList.openDocument (doc, true);
        else
            jassertfalse;
    }
    else if (menuItemID == showPathsID)
    {
        showPathsWindow (true);
    }
    else if (menuItemID >= examplesBaseID && menuItemID < (examplesBaseID + numExamples))
    {
        findAndLaunchExample (menuItemID - examplesBaseID);
    }
}

//==============================================================================
void ProjucerApplication::getAllCommands (Array <CommandID>& commands)
{
    JUCEApplication::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::newProject,
                              CommandIDs::newProjectFromClipboard,
                              CommandIDs::newPIP,
                              CommandIDs::open,
                              CommandIDs::launchDemoRunner,
                              CommandIDs::closeAllWindows,
                              CommandIDs::closeAllDocuments,
                              CommandIDs::clearRecentFiles,
                              CommandIDs::saveAll,
                              CommandIDs::showGlobalPathsWindow,
                              CommandIDs::showUTF8Tool,
                              CommandIDs::showSVGPathTool,
                              CommandIDs::showAboutWindow,
                              CommandIDs::checkForNewVersion,
                              CommandIDs::enableNewVersionCheck,
                              CommandIDs::showForum,
                              CommandIDs::showAPIModules,
                              CommandIDs::showAPIClasses,
                              CommandIDs::showTutorials };

    commands.addArray (ids, numElementsInArray (ids));
}

void ProjucerApplication::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::newProject:
        result.setInfo ("New Project...", "Creates a new JUCE project", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('n', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::newProjectFromClipboard:
        result.setInfo ("New Project From Clipboard...", "Creates a new JUCE project from the clipboard contents", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('n', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::newPIP:
        result.setInfo ("New PIP...", "Opens the PIP Creator utility for creating a new PIP", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('p', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::launchDemoRunner:
        result.setInfo ("Launch Demo Runner", "Launches the JUCE demo runner application, or the project if it can't be found", CommandCategories::general, 0);
        result.setActive (tryToFindDemoRunnerExecutable() != File() || tryToFindDemoRunnerProject() != File());
        break;

    case CommandIDs::open:
        result.setInfo ("Open...", "Opens a JUCE project", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showGlobalPathsWindow:
        result.setInfo ("Global Paths...",
                        "Shows the window to change the stored global paths.",
                        CommandCategories::general, 0);
        break;

    case CommandIDs::closeAllWindows:
        result.setInfo ("Close All Windows", "Closes all open windows", CommandCategories::general, 0);
        result.setActive (mainWindowList.windows.size() > 0);
        break;

    case CommandIDs::closeAllDocuments:
        result.setInfo ("Close All Documents", "Closes all open documents", CommandCategories::general, 0);
        result.setActive (openDocumentManager.getNumOpenDocuments() > 0);
        break;

    case CommandIDs::clearRecentFiles:
        result.setInfo ("Clear Recent Files", "Clears all recent files from the menu", CommandCategories::general, 0);
        result.setActive (settings->recentFiles.getNumFiles() > 0);
        break;

    case CommandIDs::saveAll:
        result.setInfo ("Save All", "Saves all open documents", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier | ModifierKeys::altModifier, 0));
        break;

    case CommandIDs::showUTF8Tool:
        result.setInfo ("UTF-8 String-Literal Helper", "Shows the UTF-8 string literal utility", CommandCategories::general, 0);
        break;

    case CommandIDs::showSVGPathTool:
        result.setInfo ("SVG Path Converter", "Shows the SVG->Path data conversion utility", CommandCategories::general, 0);
        break;

    case CommandIDs::showAboutWindow:
        result.setInfo ("About Projucer", "Shows the Projucer's 'About' page.", CommandCategories::general, 0);
        break;

    case CommandIDs::checkForNewVersion:
        result.setInfo ("Check for New Version...", "Checks the web server for a new version of JUCE", CommandCategories::general, 0);
        break;

    case CommandIDs::enableNewVersionCheck:
        result.setInfo ("Automatically Check for New Versions",
                        "Enables automatic background checking for new versions of JUCE.",
                        CommandCategories::general,
                        (isAutomaticVersionCheckingEnabled() ? ApplicationCommandInfo::isTicked : 0));
        break;

    case CommandIDs::showForum:
        result.setInfo ("JUCE Community Forum", "Shows the JUCE community forum in a browser", CommandCategories::general, 0);
        break;

    case CommandIDs::showAPIModules:
        result.setInfo ("API Modules", "Shows the API modules documentation in a browser", CommandCategories::general, 0);
        break;

    case CommandIDs::showAPIClasses:
        result.setInfo ("API Classes", "Shows the API classes documentation in a browser", CommandCategories::general, 0);
        break;

    case CommandIDs::showTutorials:
        result.setInfo ("JUCE Tutorials", "Shows the JUCE tutorials in a browser", CommandCategories::general, 0);
        break;

    default:
        JUCEApplication::getCommandInfo (commandID, result);
        break;
    }
}

bool ProjucerApplication::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::newProject:                createNewProject(); break;
        case CommandIDs::newProjectFromClipboard:   createNewProjectFromClipboard(); break;
        case CommandIDs::newPIP:                    createNewPIP(); break;
        case CommandIDs::open:                      askUserToOpenFile(); break;
        case CommandIDs::launchDemoRunner:          launchDemoRunner(); break;
        case CommandIDs::saveAll:                   saveAllDocuments(); break;
        case CommandIDs::closeAllWindows:           closeAllMainWindowsAndQuitIfNeeded(); break;
        case CommandIDs::closeAllDocuments:         closeAllDocuments (OpenDocumentManager::SaveIfNeeded::yes); break;
        case CommandIDs::clearRecentFiles:          clearRecentFiles(); break;
        case CommandIDs::showUTF8Tool:              showUTF8ToolWindow(); break;
        case CommandIDs::showSVGPathTool:           showSVGPathDataToolWindow(); break;
        case CommandIDs::showGlobalPathsWindow:     showPathsWindow (false); break;
        case CommandIDs::showAboutWindow:           showAboutWindow(); break;
        case CommandIDs::checkForNewVersion:        LatestVersionCheckerAndUpdater::getInstance()->checkForNewVersion (false); break;
        case CommandIDs::enableNewVersionCheck:     setAutomaticVersionCheckingEnabled (! isAutomaticVersionCheckingEnabled()); break;
        case CommandIDs::showForum:                 launchForumBrowser(); break;
        case CommandIDs::showAPIModules:            launchModulesBrowser(); break;
        case CommandIDs::showAPIClasses:            launchClassesBrowser(); break;
        case CommandIDs::showTutorials:             launchTutorialsBrowser(); break;
        default:                                    return JUCEApplication::perform (info);
    }

    return true;
}

//==============================================================================
void ProjucerApplication::createNewProject()
{
    auto* mw = mainWindowList.getOrCreateEmptyWindow();
    jassert (mw != nullptr);

    mw->showStartPage();

    mainWindowList.checkWindowBounds (*mw);
}

void ProjucerApplication::createNewProjectFromClipboard()
{
    auto tempFile = File::getSpecialLocation (File::SpecialLocationType::tempDirectory).getChildFile ("PIPs").getChildFile ("Clipboard")
                                                                                       .getChildFile ("PIPFile_" + String (std::abs (Random::getSystemRandom().nextInt())) + ".h")
                                                                                       .getNonexistentSibling();

    if (tempFile.existsAsFile())
        tempFile.deleteFile();

    tempFile.create();
    tempFile.appendText (SystemClipboard::getTextFromClipboard());

    auto cleanup = [parent = WeakReference { this }, tempFile] (String errorString)
    {
        if (parent == nullptr || errorString.isEmpty())
            return;

        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon, "Error", errorString);
        parent->messageBox = AlertWindow::showScopedAsync (options, nullptr);
        tempFile.deleteFile();
    };

    if (! isPIPFile (tempFile))
    {
        cleanup ("Clipboard does not contain a valid PIP.");
        return;
    }

    openFile (tempFile, [parent = WeakReference { this }, cleanup] (bool openedSuccessfully)
    {
        if (parent == nullptr)
            return;

        if (! openedSuccessfully)
        {
            cleanup ("Couldn't create project from clipboard contents.");
            parent->mainWindowList.closeWindow (parent->mainWindowList.windows.getLast());
        }
    });
}

void ProjucerApplication::createNewPIP()
{
    showPIPCreatorWindow();
}

void ProjucerApplication::askUserToOpenFile()
{
    chooser = std::make_unique<FileChooser> ("Open File");
    auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (flags, [this] (const FileChooser& fc)
    {
        const auto result = fc.getResult();

        if (result != File{})
            openFile (result, nullptr);
    });
}

void ProjucerApplication::openFile (const File& file, std::function<void (bool)> callback)
{
    mainWindowList.openFile (file, std::move (callback));
}

void ProjucerApplication::saveAllDocuments()
{
    openDocumentManager.saveAllSyncWithoutAsking();

    for (int i = 0; i < mainWindowList.windows.size(); ++i)
        if (auto* pcc = mainWindowList.windows.getUnchecked (i)->getProjectContentComponent())
            pcc->refreshProjectTreeFileStatuses();
}

void ProjucerApplication::closeAllDocuments (OpenDocumentManager::SaveIfNeeded askUserToSave)
{
    openDocumentManager.closeAllAsync (askUserToSave, nullptr);
}

void ProjucerApplication::closeAllMainWindows (std::function<void (bool)> callback)
{
    mainWindowList.askAllWindowsToClose (std::move (callback));
}

void ProjucerApplication::closeAllMainWindowsAndQuitIfNeeded()
{
    closeAllMainWindows ([parent = WeakReference<ProjucerApplication> { this }] (bool closedSuccessfully)
    {
       #if JUCE_MAC
        ignoreUnused (parent, closedSuccessfully);
       #else
        if (parent == nullptr)
            return;

        if (closedSuccessfully && parent->mainWindowList.windows.size() == 0)
            parent->systemRequestedQuit();
       #endif
    });
}

void ProjucerApplication::clearRecentFiles()
{
    settings->recentFiles.clear();
    settings->recentFiles.clearRecentFilesNatively();
    settings->flush();
    menuModel->menuItemsChanged();
}

//==============================================================================
void ProjucerApplication::showUTF8ToolWindow()
{
    if (utf8Window != nullptr)
        utf8Window->toFront (true);
    else
        new FloatingToolWindow ("UTF-8 String Literal Converter", "utf8WindowPos",
                                new UTF8Component(), utf8Window, true,
                                500, 500, 300, 300, 1000, 1000);
}

void ProjucerApplication::showSVGPathDataToolWindow()
{
    if (svgPathWindow != nullptr)
        svgPathWindow->toFront (true);
    else
        new FloatingToolWindow ("SVG Path Converter", "svgPathWindowPos",
                                new SVGPathDataComponent(), svgPathWindow, true,
                                500, 500, 300, 300, 1000, 1000);
}

void ProjucerApplication::showAboutWindow()
{
    if (aboutWindow != nullptr)
        aboutWindow->toFront (true);
    else
        new FloatingToolWindow ({}, {}, new AboutWindowComponent(),
                                aboutWindow, false,
                                500, 300, 500, 300, 500, 300);
}

void ProjucerApplication::showPathsWindow (bool highlightJUCEPath)
{
    if (pathsWindow != nullptr)
        pathsWindow->toFront (true);
    else
        new FloatingToolWindow ("Global Paths", "pathsWindowPos",
                                new GlobalPathsWindowComponent(), pathsWindow, false,
                                600, 700, 600, 700, 600, 700);

    if (highlightJUCEPath)
        if (auto* pathsComp = dynamic_cast<GlobalPathsWindowComponent*> (pathsWindow->getChildComponent (0)))
            pathsComp->highlightJUCEPath();
}

void ProjucerApplication::showEditorColourSchemeWindow()
{
    if (editorColourSchemeWindow != nullptr)
        editorColourSchemeWindow->toFront (true);
    else
        new FloatingToolWindow ("Editor Colour Scheme", "editorColourSchemeWindowPos",
                                new EditorColourSchemeWindowComponent(), editorColourSchemeWindow, false,
                                500, 500, 500, 500, 500, 500);
}

void ProjucerApplication::showPIPCreatorWindow()
{
    if (pipCreatorWindow != nullptr)
        pipCreatorWindow->toFront (true);
    else
        new FloatingToolWindow ("PIP Creator", "pipCreatorWindowPos",
                                new PIPCreatorWindowComponent(), pipCreatorWindow, false,
                                600, 750, 600, 750, 600, 750);
}

void ProjucerApplication::launchForumBrowser()
{
    URL forumLink ("https://forum.juce.com/");

    if (forumLink.isWellFormed())
        forumLink.launchInDefaultBrowser();
}

void ProjucerApplication::launchModulesBrowser()
{
    URL modulesLink ("https://docs.juce.com/master/modules.html");

    if (modulesLink.isWellFormed())
        modulesLink.launchInDefaultBrowser();
}

void ProjucerApplication::launchClassesBrowser()
{
    URL classesLink ("https://docs.juce.com/master/classes.html");

    if (classesLink.isWellFormed())
        classesLink.launchInDefaultBrowser();
}

void ProjucerApplication::launchTutorialsBrowser()
{
    URL tutorialsLink ("https://juce.com/learn/tutorials");

    if (tutorialsLink.isWellFormed())
        tutorialsLink.launchInDefaultBrowser();
}

//==============================================================================
struct FileWithTime
{
    FileWithTime (const File& f) : file (f), time (f.getLastModificationTime()) {}
    FileWithTime() {}

    bool operator<  (const FileWithTime& other) const    { return time <  other.time; }
    bool operator== (const FileWithTime& other) const    { return time == other.time; }

    File file;
    Time time;
};

void ProjucerApplication::deleteLogger()
{
    const int maxNumLogFilesToKeep = 50;

    Logger::setCurrentLogger (nullptr);

    if (logger != nullptr)
    {
        auto logFiles = logger->getLogFile().getParentDirectory().findChildFiles (File::findFiles, false);

        if (logFiles.size() > maxNumLogFilesToKeep)
        {
            Array<FileWithTime> files;

            for (auto& f : logFiles)
                files.addUsingDefaultSort (f);

            for (int i = 0; i < files.size() - maxNumLogFilesToKeep; ++i)
                files.getReference (i).file.deleteFile();
        }
    }

    logger.reset();
}

PropertiesFile::Options ProjucerApplication::getPropertyFileOptionsFor (const String& filename, bool isProjectSettings)
{
    PropertiesFile::Options options;
    options.applicationName     = filename;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Application Support";
   #if JUCE_LINUX || JUCE_BSD
    options.folderName          = "~/.config/Projucer";
   #else
    options.folderName          = "Projucer";
   #endif

    if (isProjectSettings)
        options.folderName += "/ProjectSettings";

    return options;
}

void ProjucerApplication::initCommandManager()
{
    commandManager.reset (new ApplicationCommandManager());
    commandManager->registerAllCommandsForTarget (this);

    {
        CodeDocument doc;
        CppCodeEditorComponent ed (File(), doc);
        commandManager->registerAllCommandsForTarget (&ed);
    }
}

static void rescanModules (AvailableModulesList& list, const Array<File>& paths, bool async)
{
    if (async)
        list.scanPathsAsync (paths);
    else
        list.scanPaths (paths);
}

void ProjucerApplication::rescanJUCEPathModules()
{
    rescanModules (jucePathModulesList, { getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()).get().toString() }, ! isRunningCommandLine);
}

void ProjucerApplication::rescanUserPathModules()
{
    rescanModules (userPathsModulesList, { getAppSettings().getStoredPath (Ids::defaultUserModulePath, TargetOS::getThisOS()).get().toString() }, ! isRunningCommandLine);
}

bool ProjucerApplication::isAutomaticVersionCheckingEnabled() const
{
    return ! getGlobalProperties().getBoolValue (Ids::dontQueryForUpdate);
}

void ProjucerApplication::setAutomaticVersionCheckingEnabled (bool enabled)
{
    getGlobalProperties().setValue (Ids::dontQueryForUpdate, ! enabled);
}

bool ProjucerApplication::shouldPromptUserAboutIncorrectJUCEPath() const
{
    return ! getGlobalProperties().getBoolValue (Ids::dontAskAboutJUCEPath);
}

void ProjucerApplication::setShouldPromptUserAboutIncorrectJUCEPath (bool shouldPrompt)
{
    getGlobalProperties().setValue (Ids::dontAskAboutJUCEPath, ! shouldPrompt);
}

void ProjucerApplication::selectEditorColourSchemeWithName (const String& schemeName)
{
    auto& appearanceSettings = getAppSettings().appearance;
    auto schemes = appearanceSettings.getPresetSchemes();

    auto schemeIndex = schemes.indexOf (schemeName);

    if (schemeIndex >= 0)
        setEditorColourScheme (schemeIndex, true);
}

void ProjucerApplication::setColourScheme (int index, bool saveSetting)
{
    switch (index)
    {
        case 0: lookAndFeel.setColourScheme (LookAndFeel_V4::getDarkColourScheme());  break;
        case 1: lookAndFeel.setColourScheme (LookAndFeel_V4::getGreyColourScheme());  break;
        case 2: lookAndFeel.setColourScheme (LookAndFeel_V4::getLightColourScheme()); break;
        default: break;
    }

    lookAndFeel.setupColours();
    mainWindowList.sendLookAndFeelChange();

    if (utf8Window != nullptr)                  utf8Window->sendLookAndFeelChange();
    if (svgPathWindow != nullptr)               svgPathWindow->sendLookAndFeelChange();
    if (aboutWindow != nullptr)                 aboutWindow->sendLookAndFeelChange();
    if (pathsWindow != nullptr)                 pathsWindow->sendLookAndFeelChange();
    if (editorColourSchemeWindow != nullptr)    editorColourSchemeWindow->sendLookAndFeelChange();
    if (pipCreatorWindow != nullptr)            pipCreatorWindow->sendLookAndFeelChange();

    auto* mcm = ModalComponentManager::getInstance();
    for (auto i = 0; i < mcm->getNumModalComponents(); ++i)
        mcm->getModalComponent (i)->sendLookAndFeelChange();

    if (saveSetting)
    {
        auto& properties = getGlobalProperties();
        properties.setValue ("COLOUR SCHEME", index);
    }

    selectedColourSchemeIndex = index;

    getCommandManager().commandStatusChanged();
}

void ProjucerApplication::setEditorColourScheme (int index, bool saveSetting)
{
    auto& appearanceSettings = getAppSettings().appearance;
    auto schemes = appearanceSettings.getPresetSchemes();

    index = jmin (index, schemes.size() - 1);

    appearanceSettings.selectPresetScheme (index);

    if (saveSetting)
    {
        auto& properties = getGlobalProperties();
        properties.setValue ("EDITOR COLOUR SCHEME", index);
    }

    selectedEditorColourSchemeIndex = index;

    getCommandManager().commandStatusChanged();
}

static bool isEditorColourSchemeADefaultScheme (const StringArray& schemes, int editorColourSchemeIndex)
{
    auto& schemeName = schemes[editorColourSchemeIndex];
    return (schemeName == "Default (Dark)" || schemeName == "Default (Light)");
}

static int getEditorColourSchemeForGUIColourScheme (const StringArray& schemes, int guiColourSchemeIndex)
{
    auto defaultDarkEditorIndex  = schemes.indexOf ("Default (Dark)");
    auto defaultLightEditorIndex = schemes.indexOf ("Default (Light)");

    // Can't find default code editor colour schemes!
    jassert (defaultDarkEditorIndex != -1 && defaultLightEditorIndex != -1);

    return (guiColourSchemeIndex == 2 ? defaultLightEditorIndex : defaultDarkEditorIndex);
}

void ProjucerApplication::updateEditorColourSchemeIfNeeded()
{
    auto& appearanceSettings = getAppSettings().appearance;
    auto schemes = appearanceSettings.getPresetSchemes();

    if (isEditorColourSchemeADefaultScheme (schemes, selectedEditorColourSchemeIndex))
        setEditorColourScheme (getEditorColourSchemeForGUIColourScheme (schemes, selectedColourSchemeIndex), true);
}
