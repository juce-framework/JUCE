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

void createGUIEditorMenu (PopupMenu&);
void handleGUIEditorMenuCommand (int);
void registerGUIEditorCommands();

//==============================================================================
struct ProjucerApplication::MainMenuModel  : public MenuBarModel
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
        PopupMenu menu;
        getApp().createMenu (menu, menuName);
        return menu;
    }

    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override
    {
        getApp().handleMainMenuCommand (menuItemID);
    }
};

//==============================================================================
ProjucerApplication::ProjucerApplication() :  isRunningCommandLine (false)
{
}

void ProjucerApplication::initialise (const String& commandLine)
{
    if (commandLine.trimStart().startsWith ("--server"))
    {
        initialiseLogger ("Compiler_Log_");
        LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

       #if JUCE_MAC
        Process::setDockIconVisible (false);
       #endif

        server = createClangServer (commandLine);
    }
    else
    {
        initialiseLogger ("IDE_Log_");
        Logger::writeToLog (SystemStats::getOperatingSystemName());
        Logger::writeToLog ("CPU: " + String (SystemStats::getCpuSpeedInMegaherz())
                              + "MHz  Cores: " + String (SystemStats::getNumCpus())
                              + "  " + String (SystemStats::getMemorySizeInMegabytes()) + "MB");

        initialiseBasics();

        isRunningCommandLine = commandLine.isNotEmpty();

        licenseController = new LicenseController;
        licenseController->addLicenseStatusChangedCallback (this);

        if (isRunningCommandLine)
        {
            const int appReturnCode = performCommandLine (commandLine);

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

        openDocumentManager.registerType (new ProjucerAppClasses::LiveBuildCodeEditorDocument::Type(), 2);

        childProcessCache = new ChildProcessCache();

        initCommandManager();
        menuModel = new MainMenuModel();

        settings->appearance.refreshPresetSchemeList();

        setColourScheme (settings->getGlobalProperties().getIntValue ("COLOUR SCHEME"), false);
        setEditorColourScheme (settings->getGlobalProperties().getIntValue ("EDITOR COLOUR SCHEME"), false);
        updateEditorColourSchemeIfNeeded();

        // do further initialisation in a moment when the message loop has started
        triggerAsyncUpdate();
    }
}

void ProjucerApplication::initialiseBasics()
{
    LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    settings = new StoredSettings();
    ImageCache::setCacheTimeout (30 * 1000);
    icons = new Icons();
    tooltipWindow.setMillisecondsBeforeTipAppears (1200);
}

bool ProjucerApplication::initialiseLogger (const char* filePrefix)
{
    if (logger == nullptr)
    {
       #if JUCE_LINUX
        String folder = "~/.config/Projucer/Logs";
       #else
        String folder = "com.juce.projucer";
       #endif

        logger = FileLogger::createDateStampedLogger (folder, filePrefix, ".txt",
                                                      getApplicationName() + " " + getApplicationVersion()
                                                        + "  ---  Build date: " __DATE__);
        Logger::setCurrentLogger (logger);
    }

    return logger != nullptr;
}

void ProjucerApplication::handleAsyncUpdate()
{
    if (licenseController != nullptr)
        licenseController->startWebviewIfNeeded();

   #if JUCE_MAC
    PopupMenu extraAppleMenuItems;
    createExtraAppleMenuItems (extraAppleMenuItems);

    // workaround broken "Open Recent" submenu: not passing the
    // submenu's title here avoids the defect in JuceMainMenuHandler::addMenuItem
    MenuBarModel::setMacMainMenu (menuModel, &extraAppleMenuItems); //, "Open Recent");
   #endif

    versionChecker = new LatestVersionChecker();
}

void ProjucerApplication::initialiseWindows (const String& commandLine)
{
    const String commandLineWithoutNSDebug (commandLine.replace ("-NSDocumentRevisionsDebugMode YES", StringRef()));

    if (commandLineWithoutNSDebug.trim().isNotEmpty() && ! commandLineWithoutNSDebug.trim().startsWithChar ('-'))
        anotherInstanceStarted (commandLine);
    else
        mainWindowList.reopenLastProjects();

    mainWindowList.createWindowIfNoneAreOpen();

    if (licenseController->getState().applicationUsageDataState == LicenseState::ApplicationUsageData::notChosenYet)
        showApplicationUsageDataAgreementPopup();
}

void ProjucerApplication::shutdown()
{
    if (server != nullptr)
    {
        destroyClangServer (server);
        Logger::writeToLog ("Server shutdown cleanly");
    }

    versionChecker = nullptr;
    utf8Window = nullptr;
    svgPathWindow = nullptr;
    aboutWindow = nullptr;
    pathsWindow = nullptr;
    editorColourSchemeWindow = nullptr;

    if (licenseController != nullptr)
    {
        licenseController->removeLicenseStatusChangedCallback (this);
        licenseController = nullptr;
    }

    mainWindowList.forceCloseAllWindows();
    openDocumentManager.clear();

    childProcessCache = nullptr;

   #if JUCE_MAC
    MenuBarModel::setMacMainMenu (nullptr);
   #endif

    menuModel = nullptr;
    commandManager = nullptr;
    settings = nullptr;

    LookAndFeel::setDefaultLookAndFeel (nullptr);

    if (! isRunningCommandLine)
        Logger::writeToLog ("Shutdown");

    deleteLogger();
}

struct AsyncQuitRetrier  : private Timer
{
    AsyncQuitRetrier()   { startTimer (500); }

    void timerCallback() override
    {
        stopTimer();
        delete this;

        if (JUCEApplicationBase* app = JUCEApplicationBase::getInstance())
            app->systemRequestedQuit();
    }

    JUCE_DECLARE_NON_COPYABLE (AsyncQuitRetrier)
};

void ProjucerApplication::systemRequestedQuit()
{
    if (server != nullptr)
    {
        sendQuitMessageToIDE (server);
    }
    else if (ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        new AsyncQuitRetrier();
    }
    else
    {
        if (closeAllMainWindows())
            quit();
    }
}

//==============================================================================
void ProjucerApplication::licenseStateChanged (const LicenseState& state)
{
   #if ! JUCER_ENABLE_GPL_MODE
    if (state.type != LicenseState::Type::notLoggedIn
     && state.type != LicenseState::Type::noLicenseChosenYet)
   #else
    ignoreUnused (state);
   #endif
    {
        initialiseWindows (getCommandLineParameters());
    }
}

void ProjucerApplication::doLogout()
{
    if (licenseController != nullptr)
    {
        const LicenseState& state = licenseController->getState();

        if (state.type != LicenseState::Type::notLoggedIn && closeAllMainWindows())
            licenseController->logout();
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
    if (server == nullptr && ! commandLine.trim().startsWithChar ('-'))
        openFile (File (commandLine.unquoted()));
}

ProjucerApplication& ProjucerApplication::getApp()
{
    ProjucerApplication* const app = dynamic_cast<ProjucerApplication*> (JUCEApplication::getInstance());
    jassert (app != nullptr);
    return *app;
}

ApplicationCommandManager& ProjucerApplication::getCommandManager()
{
    ApplicationCommandManager* cm = ProjucerApplication::getApp().commandManager;
    jassert (cm != nullptr);
    return *cm;
}


//==============================================================================
enum
{
    recentProjectsBaseID = 100,
    activeDocumentsBaseID = 300,
    colourSchemeBaseID = 1000,
    codeEditorColourSchemeBaseID = 2000,
};

MenuBarModel* ProjucerApplication::getMenuModel()
{
    return menuModel.get();
}

StringArray ProjucerApplication::getMenuNames()
{
    const char* const names[] = { "File", "Edit", "View", "Build", "Window", "GUI Editor", "Tools", nullptr };
    return StringArray (names);
}

void ProjucerApplication::createMenu (PopupMenu& menu, const String& menuName)
{
    if (menuName == "File")             createFileMenu   (menu);
    else if (menuName == "Edit")        createEditMenu   (menu);
    else if (menuName == "View")        createViewMenu   (menu);
    else if (menuName == "Build")       createBuildMenu  (menu);
    else if (menuName == "Window")      createWindowMenu (menu);
    else if (menuName == "Tools")       createToolsMenu  (menu);
    else if (menuName == "GUI Editor")  createGUIEditorMenu (menu);
    else                                jassertfalse; // names have changed?
}

void ProjucerApplication::createFileMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::newProject);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::open);

    PopupMenu recentFiles;
    settings->recentFiles.createPopupMenuItems (recentFiles, recentProjectsBaseID, true, true);
    menu.addSubMenu ("Open Recent", recentFiles);

    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::closeDocument);
    menu.addCommandItem (commandManager, CommandIDs::saveDocument);
    menu.addCommandItem (commandManager, CommandIDs::saveDocumentAs);
    menu.addCommandItem (commandManager, CommandIDs::saveAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::closeProject);
    menu.addCommandItem (commandManager, CommandIDs::saveProject);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::openInIDE);
    menu.addCommandItem (commandManager, CommandIDs::saveAndOpenInIDE);
    menu.addSeparator();

   #if ! JUCER_ENABLE_GPL_MODE
    menu.addCommandItem (commandManager, CommandIDs::loginLogout);
   #endif

    #if ! JUCE_MAC
      menu.addCommandItem (commandManager, CommandIDs::showAboutWindow);
      menu.addCommandItem (commandManager, CommandIDs::showAppUsageWindow);
      menu.addCommandItem (commandManager, CommandIDs::showGlobalPathsWindow);
      menu.addSeparator();
      menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
    #endif
}

void ProjucerApplication::createEditMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::undo);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::redo);
    menu.addSeparator();
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::selectAll);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::deselectAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::showFindPanel);
    menu.addCommandItem (commandManager, CommandIDs::findSelection);
    menu.addCommandItem (commandManager, CommandIDs::findNext);
    menu.addCommandItem (commandManager, CommandIDs::findPrevious);
}

void ProjucerApplication::createViewMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::showProjectSettings);
    menu.addCommandItem (commandManager, CommandIDs::showProjectTab);
    menu.addCommandItem (commandManager, CommandIDs::showBuildTab);
    menu.addCommandItem (commandManager, CommandIDs::showFileExplorerPanel);
    menu.addCommandItem (commandManager, CommandIDs::showModulesPanel);
    menu.addCommandItem (commandManager, CommandIDs::showExportersPanel);
    menu.addCommandItem (commandManager, CommandIDs::showExporterSettings);

    menu.addSeparator();
    createColourSchemeItems (menu);
}

void ProjucerApplication::createBuildMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::toggleBuildEnabled);
    menu.addCommandItem (commandManager, CommandIDs::buildNow);
    menu.addCommandItem (commandManager, CommandIDs::toggleContinuousBuild);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::launchApp);
    menu.addCommandItem (commandManager, CommandIDs::killApp);
    menu.addCommandItem (commandManager, CommandIDs::cleanAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::reinstantiateComp);
    menu.addCommandItem (commandManager, CommandIDs::showWarnings);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::nextError);
    menu.addCommandItem (commandManager, CommandIDs::prevError);
}

void ProjucerApplication::createColourSchemeItems (PopupMenu& menu)
{
    PopupMenu colourSchemes;

    colourSchemes.addItem (colourSchemeBaseID + 0, "Dark", true, selectedColourSchemeIndex == 0);
    colourSchemes.addItem (colourSchemeBaseID + 1, "Grey", true, selectedColourSchemeIndex == 1);
    colourSchemes.addItem (colourSchemeBaseID + 2, "Light", true, selectedColourSchemeIndex == 2);

    menu.addSubMenu ("Colour Scheme", colourSchemes);

    //==========================================================================
    PopupMenu editorColourSchemes;

    auto& appearanceSettings = getAppSettings().appearance;

    appearanceSettings.refreshPresetSchemeList();
    auto schemes = appearanceSettings.getPresetSchemes();

    auto i = 0;
    for (auto s : schemes)
    {
        editorColourSchemes.addItem (codeEditorColourSchemeBaseID + i, s,
                                     editorColourSchemeWindow == nullptr,
                                     selectedEditorColourSchemeIndex == i);
        ++i;
    }

    numEditorColourSchemes = i;

    editorColourSchemes.addSeparator();
    editorColourSchemes.addItem (codeEditorColourSchemeBaseID + numEditorColourSchemes,
                                 "Create...", editorColourSchemeWindow == nullptr);

    menu.addSubMenu ("Editor Colour Scheme", editorColourSchemes);
}

void ProjucerApplication::createWindowMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::closeWindow);
    menu.addSeparator();

    menu.addCommandItem (commandManager, CommandIDs::goToPreviousDoc);
    menu.addCommandItem (commandManager, CommandIDs::goToNextDoc);
    menu.addCommandItem (commandManager, CommandIDs::goToCounterpart);
    menu.addSeparator();

    const int numDocs = jmin (50, openDocumentManager.getNumOpenDocuments());

    for (int i = 0; i < numDocs; ++i)
    {
        OpenDocumentManager::Document* doc = openDocumentManager.getOpenDocument(i);
        menu.addItem (activeDocumentsBaseID + i, doc->getName());
    }

    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::closeAllDocuments);
}

void ProjucerApplication::createToolsMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::showUTF8Tool);
    menu.addCommandItem (commandManager, CommandIDs::showSVGPathTool);
    menu.addCommandItem (commandManager, CommandIDs::showTranslationTool);
}

void ProjucerApplication::createExtraAppleMenuItems (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::showAboutWindow);
    menu.addCommandItem (commandManager, CommandIDs::showAppUsageWindow);
    menu.addSeparator();
    menu.addCommandItem (commandManager, CommandIDs::showGlobalPathsWindow);
}

void ProjucerApplication::handleMainMenuCommand (int menuItemID)
{
    if (menuItemID >= recentProjectsBaseID && menuItemID < (recentProjectsBaseID + 100))
    {
        // open a file from the "recent files" menu
        openFile (settings->recentFiles.getFile (menuItemID - recentProjectsBaseID));
    }
    else if (menuItemID >= activeDocumentsBaseID && menuItemID < (activeDocumentsBaseID + 200))
    {
        if (OpenDocumentManager::Document* doc = openDocumentManager.getOpenDocument (menuItemID - activeDocumentsBaseID))
            mainWindowList.openDocument (doc, true);
        else
            jassertfalse;
    }
    else if (menuItemID >= colourSchemeBaseID && menuItemID < (colourSchemeBaseID + 3))
    {
        setColourScheme (menuItemID - colourSchemeBaseID, true);
        updateEditorColourSchemeIfNeeded();
    }
    else if (menuItemID >= codeEditorColourSchemeBaseID && menuItemID < (codeEditorColourSchemeBaseID + numEditorColourSchemes))
    {
        setEditorColourScheme (menuItemID - codeEditorColourSchemeBaseID, true);
    }
    else if (menuItemID == (codeEditorColourSchemeBaseID + numEditorColourSchemes))
    {
        showEditorColourSchemeWindow();
    }
    else
    {
        handleGUIEditorMenuCommand (menuItemID);
    }
}

//==============================================================================
void ProjucerApplication::getAllCommands (Array <CommandID>& commands)
{
    JUCEApplication::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::newProject,
                              CommandIDs::open,
                              CommandIDs::closeAllDocuments,
                              CommandIDs::saveAll,
                              CommandIDs::showGlobalPathsWindow,
                              CommandIDs::showUTF8Tool,
                              CommandIDs::showSVGPathTool,
                              CommandIDs::showAboutWindow,
                              CommandIDs::showAppUsageWindow,
                              CommandIDs::loginLogout };

    commands.addArray (ids, numElementsInArray (ids));
}

void ProjucerApplication::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::newProject:
        result.setInfo ("New Project...", "Creates a new Jucer project", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('n', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::open:
        result.setInfo ("Open...", "Opens a Jucer project", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showGlobalPathsWindow:
        result.setInfo ("Global Search Paths...",
                        "Shows the window to change the global search paths.",
                        CommandCategories::general, 0);
        break;

    case CommandIDs::closeAllDocuments:
        result.setInfo ("Close All Documents", "Closes all open documents", CommandCategories::general, 0);
        result.setActive (openDocumentManager.getNumOpenDocuments() > 0);
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

    case CommandIDs::showAppUsageWindow:
        result.setInfo ("Application Usage Data", "Shows the application usage data agreement window", CommandCategories::general, 0);
        break;

    case CommandIDs::loginLogout:
        {
            bool isLoggedIn = false;
            String username;

            if (licenseController != nullptr)
            {
                const LicenseState state = licenseController->getState();
                isLoggedIn = (state.type != LicenseState::Type::notLoggedIn && state.type != LicenseState::Type::GPL);
                username = state.username;
            }

            result.setInfo (isLoggedIn
                               ? String ("Sign out ") + username + "..."
                               : String ("Sign in..."),
                            "Log out of your JUCE account", CommandCategories::general, 0);
        }
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
        case CommandIDs::open:                      askUserToOpenFile(); break;
        case CommandIDs::saveAll:                   openDocumentManager.saveAll(); break;
        case CommandIDs::closeAllDocuments:         closeAllDocuments (true); break;
        case CommandIDs::showUTF8Tool:              showUTF8ToolWindow(); break;
        case CommandIDs::showSVGPathTool:           showSVGPathDataToolWindow(); break;
        case CommandIDs::showGlobalPathsWindow:     showPathsWindow(); break;
        case CommandIDs::showAboutWindow:           showAboutWindow(); break;
        case CommandIDs::showAppUsageWindow:        showApplicationUsageDataAgreementPopup(); break;
        case CommandIDs::loginLogout:               doLogout(); break;
        default:                                    return JUCEApplication::perform (info);
    }

    return true;
}

//==============================================================================
void ProjucerApplication::createNewProject()
{
    MainWindow* mw = mainWindowList.getOrCreateEmptyWindow();
    mw->showNewProjectWizard();
    mainWindowList.avoidSuperimposedWindows (mw);
}

void ProjucerApplication::updateNewlyOpenedProject (Project& p)
{
    LiveBuildProjectSettings::updateNewlyOpenedProject (p);
}

void ProjucerApplication::askUserToOpenFile()
{
    FileChooser fc ("Open File");

    if (fc.browseForFileToOpen())
        openFile (fc.getResult());
}

bool ProjucerApplication::openFile (const File& file)
{
    return mainWindowList.openFile (file);
}

bool ProjucerApplication::closeAllDocuments (bool askUserToSave)
{
    return openDocumentManager.closeAll (askUserToSave);
}

bool ProjucerApplication::closeAllMainWindows()
{
    return server != nullptr || mainWindowList.askAllWindowsToClose();
}

//==============================================================================
void ProjucerApplication::showUTF8ToolWindow()
{
    if (utf8Window != nullptr)
        utf8Window->toFront (true);
    else
        new FloatingToolWindow ("UTF-8 String Literal Converter",
                                "utf8WindowPos",
                                new UTF8Component(), utf8Window, true,
                                500, 500, 300, 300, 1000, 1000);
}

void ProjucerApplication::showSVGPathDataToolWindow()
{
    if (svgPathWindow != nullptr)
        svgPathWindow->toFront (true);
    else
        new FloatingToolWindow ("SVG Path Converter",
                                "svgPathWindowPos",
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

void ProjucerApplication::showApplicationUsageDataAgreementPopup()
{
    if (applicationUsageDataWindow != nullptr)
        applicationUsageDataWindow->toFront (true);
    else
        new FloatingToolWindow ("Application Usage Analytics",
                                {}, new ApplicationUsageDataWindowComponent (isPaidOrGPL()),
                                applicationUsageDataWindow, false,
                                400, 300, 400, 300, 400, 300);
}

void ProjucerApplication::dismissApplicationUsageDataAgreementPopup()
{
    if (applicationUsageDataWindow != nullptr)
        applicationUsageDataWindow = nullptr;
}

void ProjucerApplication::showPathsWindow()
{
    if (pathsWindow != nullptr)
        pathsWindow->toFront (true);
    else
        new FloatingToolWindow ("Global Search Paths",
                                "pathsWindowPos",
                                new GlobalSearchPathsWindowComponent(), pathsWindow, false,
                                600, 500, 600, 500, 600, 500);
}

void ProjucerApplication::showEditorColourSchemeWindow()
{
    if (editorColourSchemeWindow != nullptr)
        editorColourSchemeWindow->toFront (true);
    else
    {
        new FloatingToolWindow ("Editor Colour Scheme",
                                "editorColourSchemeWindowPos",
                                new EditorColourSchemeWindowComponent(),
                                editorColourSchemeWindow,
                                false,
                                500, 500, 500, 500, 500, 500);
    }
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
        Array<File> logFiles;
        logger->getLogFile().getParentDirectory().findChildFiles (logFiles, File::findFiles, false);

        if (logFiles.size() > maxNumLogFilesToKeep)
        {
            Array <FileWithTime> files;

            for (int i = 0; i < logFiles.size(); ++i)
                files.addUsingDefaultSort (logFiles.getReference(i));

            for (int i = 0; i < files.size() - maxNumLogFilesToKeep; ++i)
                files.getReference(i).file.deleteFile();
        }
    }

    logger = nullptr;
}

PropertiesFile::Options ProjucerApplication::getPropertyFileOptionsFor (const String& filename, bool isProjectSettings)
{
    PropertiesFile::Options options;
    options.applicationName     = filename;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Application Support";
   #if JUCE_LINUX
    options.folderName          = "~/.config/Projucer";
   #else
    options.folderName          = "Projucer";
   #endif

    if (isProjectSettings)
        options.folderName += "/ProjectSettings";

    return options;
}

void ProjucerApplication::updateAllBuildTabs()
{
    for (int i = 0; i < mainWindowList.windows.size(); ++i)
        if (ProjectContentComponent* p = mainWindowList.windows.getUnchecked(i)->getProjectContentComponent())
            p->rebuildProjectTabs();
}

void ProjucerApplication::initCommandManager()
{
    commandManager = new ApplicationCommandManager();
    commandManager->registerAllCommandsForTarget (this);

    {
        CodeDocument doc;
        CppCodeEditorComponent ed (File(), doc);
        commandManager->registerAllCommandsForTarget (&ed);
    }

    registerGUIEditorCommands();
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
    if (applicationUsageDataWindow != nullptr)  applicationUsageDataWindow->sendLookAndFeelChange();
    if (pathsWindow != nullptr)                 pathsWindow->sendLookAndFeelChange();
    if (editorColourSchemeWindow != nullptr)    editorColourSchemeWindow->sendLookAndFeelChange();

    auto* mcm = ModalComponentManager::getInstance();
    for (auto i = 0; i < mcm->getNumModalComponents(); ++i)
        mcm->getModalComponent (i)->sendLookAndFeelChange();

    if (saveSetting)
    {
        auto& properties = settings->getGlobalProperties();
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
        auto& properties = settings->getGlobalProperties();
        properties.setValue ("EDITOR COLOUR SCHEME", index);
    }

    selectedEditorColourSchemeIndex = index;

    getCommandManager().commandStatusChanged();
}

bool ProjucerApplication::isEditorColourSchemeADefaultScheme (const StringArray& schemes, int editorColourSchemeIndex)
{
    auto& schemeName = schemes[editorColourSchemeIndex];
    return (schemeName == "Default (Dark)" || schemeName == "Default (Light)");
}

int ProjucerApplication::getEditorColourSchemeForGUIColourScheme (const StringArray& schemes, int guiColourSchemeIndex)
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
