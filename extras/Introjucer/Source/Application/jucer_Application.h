/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_APPLICATION_JUCEHEADER__
#define __JUCER_APPLICATION_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_MainWindow.h"
#include "jucer_CommandLine.h"
#include "../Project/jucer_Module.h"
#include "jucer_AutoUpdater.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"

void createGUIEditorMenu (PopupMenu&);
void handleGUIEditorMenuCommand (int);
void registerGUIEditorCommands();

//==============================================================================
class IntrojucerApp   : public JUCEApplication
{
public:
    //==============================================================================
    IntrojucerApp() :  isRunningCommandLine (false) {}

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
        settings = new StoredSettings();

        if (commandLine.isNotEmpty())
        {
            const int appReturnCode = performCommandLine (commandLine);

            if (appReturnCode != commandLineNotPerformed)
            {
                isRunningCommandLine = true;
                setApplicationReturnValue (appReturnCode);
                quit();
                return;
            }
        }

        if (sendCommandLineToPreexistingInstance())
        {
            DBG ("Another instance is running - quitting...");
            quit();
            return;
        }

        initialiseLogger ("log_");

        icons = new Icons();

        initCommandManager();

        menuModel = new MainMenuModel();

        doExtraInitialisation();

        settings->appearance.refreshPresetSchemeList();

        ImageCache::setCacheTimeout (30 * 1000);

        if (commandLine.trim().isNotEmpty() && ! commandLine.trim().startsWithChar ('-'))
            anotherInstanceStarted (commandLine);
        else
            mainWindowList.reopenLastProjects();

        mainWindowList.createWindowIfNoneAreOpen();

       #if JUCE_MAC
        MenuBarModel::setMacMainMenu (menuModel, nullptr, "Open Recent");
       #endif

        versionChecker = new LatestVersionChecker();
    }

    void shutdown() override
    {
        versionChecker = nullptr;
        appearanceEditorWindow = nullptr;
        utf8Window = nullptr;
        svgPathWindow = nullptr;

        mainWindowList.forceCloseAllWindows();
        openDocumentManager.clear();

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

    //==============================================================================
    void systemRequestedQuit() override
    {
        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
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
    const String getApplicationName() override       { return "Introjucer"; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }

    bool moreThanOneInstanceAllowed() override
    {
        return true; // this is handled manually in initialise()
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        openFile (File (commandLine.unquoted()));
    }

    static IntrojucerApp& getApp()
    {
        IntrojucerApp* const app = dynamic_cast<IntrojucerApp*> (JUCEApplication::getInstance());
        jassert (app != nullptr);
        return *app;
    }

    static ApplicationCommandManager& getCommandManager()
    {
        ApplicationCommandManager* cm = IntrojucerApp::getApp().commandManager;
        jassert (cm != nullptr);
        return *cm;
    }

    //==============================================================================
    class MainMenuModel  : public MenuBarModel
    {
    public:
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

    enum
    {
        recentProjectsBaseID = 100,
        activeDocumentsBaseID = 300,
        colourSchemeBaseID = 1000
    };

    virtual StringArray getMenuNames()
    {
        const char* const names[] = { "File", "Edit", "View", "Window", "GUI Editor", "Tools", nullptr };
        return StringArray (names);
    }

    virtual void createMenu (PopupMenu& menu, const String& menuName)
    {
        if (menuName == "File")             createFileMenu   (menu);
        else if (menuName == "Edit")        createEditMenu   (menu);
        else if (menuName == "View")        createViewMenu   (menu);
        else if (menuName == "Window")      createWindowMenu (menu);
        else if (menuName == "Tools")       createToolsMenu  (menu);
        else if (menuName == "GUI Editor")  createGUIEditorMenu (menu);
        else                                jassertfalse; // names have changed?
    }

    virtual void createFileMenu (PopupMenu& menu)
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

        #if ! JUCE_MAC
          menu.addSeparator();
          menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
        #endif
    }

    virtual void createEditMenu (PopupMenu& menu)
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

    virtual void createViewMenu (PopupMenu& menu)
    {
        menu.addCommandItem (commandManager, CommandIDs::showFilePanel);
        menu.addCommandItem (commandManager, CommandIDs::showConfigPanel);
        menu.addCommandItem (commandManager, CommandIDs::showProjectSettings);
        menu.addCommandItem (commandManager, CommandIDs::showProjectModules);
        menu.addSeparator();
        createColourSchemeItems (menu);
    }

    void createColourSchemeItems (PopupMenu& menu)
    {
        menu.addCommandItem (commandManager, CommandIDs::showAppearanceSettings);

        const StringArray presetSchemes (settings->appearance.getPresetSchemes());

        if (presetSchemes.size() > 0)
        {
            PopupMenu schemes;

            for (int i = 0; i < presetSchemes.size(); ++i)
                schemes.addItem (colourSchemeBaseID + i, presetSchemes[i]);

            menu.addSubMenu ("Colour Scheme", schemes);
        }
    }

    virtual void createWindowMenu (PopupMenu& menu)
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

    virtual void createToolsMenu (PopupMenu& menu)
    {
        menu.addCommandItem (commandManager, CommandIDs::showUTF8Tool);
        menu.addCommandItem (commandManager, CommandIDs::showSVGPathTool);
        menu.addCommandItem (commandManager, CommandIDs::showTranslationTool);
    }

    virtual void handleMainMenuCommand (int menuItemID)
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
    void getAllCommands (Array <CommandID>& commands) override
    {
        JUCEApplication::getAllCommands (commands);

        const CommandID ids[] = { CommandIDs::newProject,
                                  CommandIDs::open,
                                  CommandIDs::closeAllDocuments,
                                  CommandIDs::saveAll,
                                  CommandIDs::showAppearanceSettings,
                                  CommandIDs::showUTF8Tool,
                                  CommandIDs::showSVGPathTool };

        commands.addArray (ids, numElementsInArray (ids));
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
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

        case CommandIDs::showAppearanceSettings:
            result.setInfo ("Fonts and Colours...", "Shows the appearance settings window.", CommandCategories::general, 0);
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

        default:
            JUCEApplication::getCommandInfo (commandID, result);
            break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::newProject:                createNewProject(); break;
            case CommandIDs::open:                      askUserToOpenFile(); break;
            case CommandIDs::saveAll:                   openDocumentManager.saveAll(); break;
            case CommandIDs::closeAllDocuments:         closeAllDocuments (true); break;
            case CommandIDs::showUTF8Tool:              showUTF8ToolWindow (utf8Window); break;
            case CommandIDs::showSVGPathTool:           showSVGPathDataToolWindow (svgPathWindow); break;

            case CommandIDs::showAppearanceSettings:    AppearanceSettings::showEditorWindow (appearanceEditorWindow); break;
            default:                                    return JUCEApplication::perform (info);
        }

        return true;
    }

    //==============================================================================
    void createNewProject()
    {
        MainWindow* mw = mainWindowList.getOrCreateEmptyWindow();
        mw->showNewProjectWizard();
        mainWindowList.avoidSuperimposedWindows (mw);
    }

    virtual void updateNewlyOpenedProject (Project&) {}

    void askUserToOpenFile()
    {
        FileChooser fc ("Open File");

        if (fc.browseForFileToOpen())
            openFile (fc.getResult());
    }

    bool openFile (const File& file)
    {
        return mainWindowList.openFile (file);
    }

    bool closeAllDocuments (bool askUserToSave)
    {
        return openDocumentManager.closeAll (askUserToSave);
    }

    virtual bool closeAllMainWindows()
    {
        return mainWindowList.askAllWindowsToClose();
    }

    //==============================================================================
    void initialiseLogger (const char* filePrefix)
    {
        if (logger == nullptr)
        {
            logger = FileLogger::createDateStampedLogger (getLogFolderName(), filePrefix, ".txt",
                                                          getApplicationName() + " " + getApplicationVersion()
                                                            + "  ---  Build date: " __DATE__);
            Logger::setCurrentLogger (logger);
        }
    }

    struct FileWithTime
    {
        FileWithTime (const File& f) : file (f), time (f.getLastModificationTime()) {}
        FileWithTime() {}

        bool operator<  (const FileWithTime& other) const    { return time <  other.time; }
        bool operator== (const FileWithTime& other) const    { return time == other.time; }

        File file;
        Time time;
    };

    void deleteLogger()
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

    virtual void doExtraInitialisation() {}
    virtual void addExtraConfigItems (Project&, TreeViewItem&) {}

   #if JUCE_LINUX
    virtual String getLogFolderName() const    { return "~/.config/Introjucer/Logs"; }
   #else
    virtual String getLogFolderName() const    { return "com.juce.introjucer"; }
   #endif

    virtual PropertiesFile::Options getPropertyFileOptionsFor (const String& filename)
    {
        PropertiesFile::Options options;
        options.applicationName     = filename;
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX
        options.folderName          = "~/.config/Introjucer";
       #else
        options.folderName          = "Introjucer";
       #endif

        return options;
    }

    virtual Component* createProjectContentComponent() const
    {
        return new ProjectContentComponent();
    }

    //==============================================================================
    IntrojucerLookAndFeel lookAndFeel;

    ScopedPointer<StoredSettings> settings;
    ScopedPointer<Icons> icons;

    ScopedPointer<MainMenuModel> menuModel;

    MainWindowList mainWindowList;
    OpenDocumentManager openDocumentManager;
    ScopedPointer<ApplicationCommandManager> commandManager;

    ScopedPointer<Component> appearanceEditorWindow, utf8Window, svgPathWindow;

    ScopedPointer<FileLogger> logger;

    bool isRunningCommandLine;

private:
    ScopedPointer<LatestVersionChecker> versionChecker;

    class AsyncQuitRetrier  : private Timer
    {
    public:
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

    void initCommandManager()
    {
        commandManager = new ApplicationCommandManager();
        commandManager->registerAllCommandsForTarget (this);

        {
            CodeDocument doc;
            CppCodeEditorComponent ed (File::nonexistent, doc);
            commandManager->registerAllCommandsForTarget (&ed);
        }

        registerGUIEditorCommands();
    }
};


#endif   // __JUCER_APPLICATION_JUCEHEADER__
