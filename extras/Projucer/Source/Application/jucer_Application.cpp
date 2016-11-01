/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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

        if (commandLine.isNotEmpty())
        {
            isRunningCommandLine = true;
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

        if (! checkEULA())
        {
            quit();
            return;
        }

        childProcessCache = new ChildProcessCache();

        initCommandManager();
        menuModel = new MainMenuModel();

        settings->appearance.refreshPresetSchemeList();

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
    initialiseWindows (getCommandLineParameters());

   #if JUCE_MAC
    PopupMenu extraAppleMenuItems;
    createExtraAppleMenuItems (extraAppleMenuItems);

    // workaround broken "Open Recent" submenu: not passing the
    // submenu's title here avoids the defect in JuceMainMenuHandler::addMenuItem
    MenuBarModel::setMacMainMenu (menuModel, &extraAppleMenuItems); //, "Open Recent");
   #endif

    versionChecker = new LatestVersionChecker();

    showLoginFormAsyncIfNotTriedRecently();
}

void ProjucerApplication::initialiseWindows (const String& commandLine)
{
    const String commandLineWithoutNSDebug (commandLine.replace ("-NSDocumentRevisionsDebugMode YES", StringRef()));

    if (commandLineWithoutNSDebug.trim().isNotEmpty() && ! commandLineWithoutNSDebug.trim().startsWithChar ('-'))
        anotherInstanceStarted (commandLine);
    else
        mainWindowList.reopenLastProjects();

    mainWindowList.createWindowIfNoneAreOpen();
}

void ProjucerApplication::shutdown()
{
    if (server != nullptr)
    {
        destroyClangServer (server);
        Logger::writeToLog ("Server shutdown cleanly");
    }

    versionChecker = nullptr;
    appearanceEditorWindow = nullptr;
    globalPreferencesWindow = nullptr;
    utf8Window = nullptr;
    svgPathWindow = nullptr;

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
    colourSchemeBaseID = 1000
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
    menu.addCommandItem (commandManager, CommandIDs::loginLogout);

    #if ! JUCE_MAC
      menu.addCommandItem (commandManager, CommandIDs::showGlobalPreferences);
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
    menu.addCommandItem (commandManager, CommandIDs::showFilePanel);
    menu.addCommandItem (commandManager, CommandIDs::showConfigPanel);
    menu.addCommandItem (commandManager, CommandIDs::showBuildTab);
    menu.addCommandItem (commandManager, CommandIDs::showProjectSettings);
    menu.addCommandItem (commandManager, CommandIDs::showProjectModules);
    menu.addSeparator();
    createColourSchemeItems (menu);
}

void ProjucerApplication::createBuildMenu (PopupMenu& menu)
{
    menu.addCommandItem (commandManager, CommandIDs::enableBuild);
    menu.addCommandItem (commandManager, CommandIDs::toggleContinuousBuild);
    menu.addCommandItem (commandManager, CommandIDs::buildNow);

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
    const StringArray presetSchemes (settings->appearance.getPresetSchemes());

    if (presetSchemes.size() > 0)
    {
        PopupMenu schemes;

        for (int i = 0; i < presetSchemes.size(); ++i)
            schemes.addItem (colourSchemeBaseID + i, presetSchemes[i]);

        menu.addSubMenu ("Colour Scheme", schemes);
    }
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
    menu.addCommandItem (commandManager, CommandIDs::showGlobalPreferences);
}

void ProjucerApplication::handleMainMenuCommand (int menuItemID)
{
    if (menuItemID >= recentProjectsBaseID && menuItemID < recentProjectsBaseID + 100)
    {
        // open a file from the "recent files" menu
        openFile (settings->recentFiles.getFile (menuItemID - recentProjectsBaseID));
    }
    else if (menuItemID >= activeDocumentsBaseID && menuItemID < activeDocumentsBaseID + 200)
    {
        if (OpenDocumentManager::Document* doc = openDocumentManager.getOpenDocument (menuItemID - activeDocumentsBaseID))
            mainWindowList.openDocument (doc, true);
        else
            jassertfalse;
    }
    else if (menuItemID >= colourSchemeBaseID && menuItemID < colourSchemeBaseID + 200)
    {
        settings->appearance.selectPresetScheme (menuItemID - colourSchemeBaseID);
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
                              CommandIDs::showGlobalPreferences,
                              CommandIDs::showUTF8Tool,
                              CommandIDs::showSVGPathTool,
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

    case CommandIDs::showGlobalPreferences:
        result.setInfo ("Preferences...", "Shows the preferences window.", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (',', ModifierKeys::commandModifier, 0));
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
        result.setInfo ("SVG Path Helper", "Shows the SVG->Path data conversion utility", CommandCategories::general, 0);
        break;

    case CommandIDs::loginLogout:
        result.setInfo (ProjucerLicenses::getInstance()->isLoggedIn()
                           ? String ("Sign out ") + ProjucerLicenses::getInstance()->getLoginName()
                           : String ("Sign in..."),
                        "Log out of your JUCE account", CommandCategories::general, 0);
        result.setActive (ProjucerLicenses::getInstance()->isDLLPresent());
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
        case CommandIDs::showGlobalPreferences:     AppearanceSettings::showGlobalPreferences (globalPreferencesWindow); break;
        case CommandIDs::loginLogout:               loginOrLogout(); break;
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
                                new UTF8Component(), utf8Window,
                                500, 500, 300, 300, 1000, 1000);
}

void ProjucerApplication::showSVGPathDataToolWindow()
{
    if (svgPathWindow != nullptr)
        svgPathWindow->toFront (true);
    else
        new FloatingToolWindow ("SVG Path Converter",
                                "svgPathWindowPos",
                                new SVGPathDataComponent(), svgPathWindow,
                                500, 500, 300, 300, 1000, 1000);
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

struct LiveBuildConfigItem   : public ConfigTreeItemTypes::ConfigTreeItemBase
{
    LiveBuildConfigItem (Project& p)  : project (p) {}

    bool isMissing() override                        { return false; }
    bool canBeSelected() const override              { return true; }
    bool mightContainSubItems() override             { return false; }
    String getUniqueName() const override            { return "live_build_settings"; }
    String getRenamingName() const override          { return getDisplayName(); }
    String getDisplayName() const override           { return "Live Build Settings"; }
    void setName (const String&) override            {}
    Icon getIcon() const override                    { return Icon (getIcons().config, getContrastingColour (Colours::green, 0.5f)); }

    void showDocument() override                     { showSettingsPage (new SettingsComp (project)); }
    void itemOpennessChanged (bool) override         {}

    Project& project;

    //==============================================================================
    struct SettingsComp  : public Component
    {
        SettingsComp (Project& p)
        {
            addAndMakeVisible (&group);

            PropertyListBuilder props;
            LiveBuildProjectSettings::getLiveSettings (p, props);

            group.setProperties (props);
            group.setName ("Live Build Settings");
            parentSizeChanged();
        }

        void parentSizeChanged() override  { updateSize (*this, group); }

        ConfigTreeItemTypes::PropertyGroupComponent group;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveBuildConfigItem)
};

void ProjucerApplication::addLiveBuildConfigItem (Project& project, TreeViewItem& parent)
{
    parent.addSubItem (new LiveBuildConfigItem (project));
}

PropertiesFile::Options ProjucerApplication::getPropertyFileOptionsFor (const String& filename)
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

    return options;
}

void ProjucerApplication::hideLoginForm()
{
    jassert (MessageManager::getInstance()->isThisTheMessageThread());
    loginForm = nullptr;
}

void ProjucerApplication::showLoginForm()
{
    if (ProjucerLicenses::getInstance()->isDLLPresent())
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        if (loginForm != nullptr)
            return;

        DialogWindow::LaunchOptions lo;

        lo.dialogTitle = "Log-in to Projucer";
        lo.dialogBackgroundColour = Colour (0xffdddddd);
        lo.content.setOwned (loginForm = new LoginForm());
        lo.escapeKeyTriggersCloseButton = true;
        lo.componentToCentreAround = nullptr;
        lo.escapeKeyTriggersCloseButton = true;
        lo.resizable = false;
        lo.useBottomRightCornerResizer = false;
        lo.useNativeTitleBar = true;

        lo.launchAsync();

        getGlobalProperties().setValue ("lastLoginAttemptTime",
                                        (int) (Time::getCurrentTime().toMilliseconds() / 1000));
    }
}

void ProjucerApplication::showLoginFormAsyncIfNotTriedRecently()
{
    if (ProjucerLicenses::getInstance()->isDLLPresent())
    {
        Time lastLoginAttempt (getGlobalProperties().getValue ("lastLoginAttemptTime").getIntValue() * (int64) 1000);

        if (Time::getCurrentTime().getDayOfMonth() != lastLoginAttempt.getDayOfMonth())
            startTimer (1000);
    }
    else
    {
        getGlobalProperties().removeValue ("lastLoginAttemptTime");
    }
}

void ProjucerApplication::timerCallback()
{
    stopTimer();

    if (! ProjucerLicenses::getInstance()->isLoggedIn())
        showLoginForm();
}

void ProjucerApplication::updateAllBuildTabs()
{
    for (int i = 0; i < mainWindowList.windows.size(); ++i)
        if (ProjectContentComponent* p = mainWindowList.windows.getUnchecked(i)->getProjectContentComponent())
            p->rebuildProjectTabs();
}

//==============================================================================
void ProjucerApplication::loginOrLogout()
{
    ProjucerLicenses& status = *ProjucerLicenses::getInstance();

    if (status.isLoggedIn())
        status.logout();
    else
        showLoginForm();

    updateAllBuildTabs();
}

bool ProjucerApplication::checkEULA()
{
    if (currentEULAHasBeenAcceptedPreviously()
          || ! ProjucerLicenses::getInstance()->isDLLPresent())
        return true;

    ScopedPointer<AlertWindow> eulaDialogue (new EULADialogue());
    bool hasBeenAccepted = (eulaDialogue->runModalLoop() == EULADialogue::accepted);
    setCurrentEULAAccepted (hasBeenAccepted);
    return hasBeenAccepted;
}

bool ProjucerApplication::currentEULAHasBeenAcceptedPreviously() const
{
    return getGlobalProperties().getValue (getEULAChecksumProperty()).getIntValue() != 0;
}

String ProjucerApplication::getEULAChecksumProperty() const
{
    return "eulaChecksum_" + MD5 (BinaryData::projucer_EULA_txt,
                                  BinaryData::projucer_EULA_txtSize).toHexString();
}

void ProjucerApplication::setCurrentEULAAccepted (bool hasBeenAccepted) const
{
    const String checksum (getEULAChecksumProperty());
    auto& globals = getGlobalProperties();

    if (hasBeenAccepted)
        globals.setValue (checksum, 1);
    else
        globals.removeValue (checksum);

    globals.saveIfNeeded();
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
