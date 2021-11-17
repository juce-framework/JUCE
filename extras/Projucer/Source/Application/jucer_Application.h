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

#include "UserAccount/jucer_LicenseController.h"
#include "jucer_MainWindow.h"
#include "../Project/Modules/jucer_Modules.h"
#include "jucer_AutoUpdater.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "../Utility/UI/jucer_ProjucerLookAndFeel.h"

//==============================================================================
class ProjucerApplication   : public JUCEApplication,
                              private AsyncUpdater
{
public:
    ProjucerApplication() = default;

    static ProjucerApplication& getApp();
    static ApplicationCommandManager& getCommandManager();

    //==============================================================================
    void initialise (const String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void deleteLogger();

    const String getApplicationName() override       { return "Projucer"; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }

    String getVersionDescription() const;
    bool moreThanOneInstanceAllowed() override       { return true; } // this is handled manually in initialise()

    void anotherInstanceStarted (const String& commandLine) override;

    //==============================================================================
    MenuBarModel* getMenuModel();

    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    bool isGUIEditorEnabled() const;

    //==============================================================================
    void openFile (const File&, std::function<void (bool)>);
    void showPathsWindow (bool highlightJUCEPath = false);
    PropertiesFile::Options getPropertyFileOptionsFor (const String& filename, bool isProjectSettings);
    void selectEditorColourSchemeWithName (const String& schemeName);

    //==============================================================================
    void rescanJUCEPathModules();
    void rescanUserPathModules();

    AvailableModulesList& getJUCEPathModulesList()     { return jucePathModulesList; }
    AvailableModulesList& getUserPathsModulesList()    { return userPathsModulesList; }

    LicenseController& getLicenseController()          { return *licenseController; }

    bool isAutomaticVersionCheckingEnabled() const;
    void setAutomaticVersionCheckingEnabled (bool shouldBeEnabled);

    bool shouldPromptUserAboutIncorrectJUCEPath() const;
    void setShouldPromptUserAboutIncorrectJUCEPath (bool shouldPrompt);

    static File getJUCEExamplesDirectoryPathFromGlobal() noexcept;
    static Array<File> getSortedExampleDirectories() noexcept;
    static Array<File> getSortedExampleFilesInDirectory (const File&) noexcept;

    //==============================================================================
    ProjucerLookAndFeel lookAndFeel;

    std::unique_ptr<StoredSettings> settings;
    std::unique_ptr<Icons> icons;

    struct MainMenuModel;
    std::unique_ptr<MainMenuModel> menuModel;

    MainWindowList mainWindowList;
    OpenDocumentManager openDocumentManager;
    std::unique_ptr<ApplicationCommandManager> commandManager;

    bool isRunningCommandLine = false;

private:
    //==============================================================================
    void handleAsyncUpdate() override;
    void doBasicApplicationSetup();

    void initCommandManager();
    bool initialiseLogger (const char* filePrefix);
    void initialiseWindows (const String& commandLine);

    void createNewProject();
    void createNewProjectFromClipboard();
    void createNewPIP();
    void askUserToOpenFile();
    void saveAllDocuments();
    void closeAllDocuments (OpenDocumentManager::SaveIfNeeded askUserToSave);
    void closeAllMainWindows (std::function<void (bool)>);
    void closeAllMainWindowsAndQuitIfNeeded();
    void clearRecentFiles();

    StringArray getMenuNames();
    PopupMenu createMenu (const String& menuName);
    PopupMenu createFileMenu();
    PopupMenu createEditMenu();
    PopupMenu createViewMenu();
    void createColourSchemeItems (PopupMenu&);
    PopupMenu createWindowMenu();
    PopupMenu createDocumentMenu();
    PopupMenu createToolsMenu();
    PopupMenu createHelpMenu();
    PopupMenu createExtraAppleMenuItems();
    void handleMainMenuCommand (int menuItemID);
    PopupMenu createExamplesPopupMenu() noexcept;

    void findAndLaunchExample (int);

    void checkIfGlobalJUCEPathHasChanged();
    File tryToFindDemoRunnerExecutable();
    File tryToFindDemoRunnerProject();
    void launchDemoRunner();

    void setColourScheme (int index, bool saveSetting);
    void setEditorColourScheme (int index, bool saveSetting);
    void updateEditorColourSchemeIfNeeded();

    void showUTF8ToolWindow();
    void showSVGPathDataToolWindow();
    void showAboutWindow();
    void showEditorColourSchemeWindow();
    void showPIPCreatorWindow();

    void launchForumBrowser();
    void launchModulesBrowser();
    void launchClassesBrowser();
    void launchTutorialsBrowser();

    void doLoginOrLogout();
    void showLoginForm();

    void enableOrDisableGUIEditor();

    //==============================================================================
   #if JUCE_MAC
    class AppleMenuRebuildListener  : private MenuBarModel::Listener
    {
    public:
        AppleMenuRebuildListener()
        {
            if (auto* model = ProjucerApplication::getApp().getMenuModel())
                model->addListener (this);
        }

        ~AppleMenuRebuildListener() override
        {
            if (auto* model = ProjucerApplication::getApp().getMenuModel())
                model->removeListener (this);
        }

    private:
        void menuBarItemsChanged (MenuBarModel*) override  {}

        void menuCommandInvoked (MenuBarModel*,
                                 const ApplicationCommandTarget::InvocationInfo& info) override
        {
            if (info.commandID == CommandIDs::enableNewVersionCheck)
                Timer::callAfterDelay (50, [] { ProjucerApplication::getApp().rebuildAppleMenu(); });
        }
    };

    void rebuildAppleMenu();

    std::unique_ptr<AppleMenuRebuildListener> appleMenuRebuildListener;
   #endif

    //==============================================================================
    std::unique_ptr<LicenseController> licenseController;

    std::unique_ptr<TooltipWindow> tooltipWindow;
    AvailableModulesList jucePathModulesList, userPathsModulesList;

    std::unique_ptr<Component> utf8Window, svgPathWindow, aboutWindow, pathsWindow,
                               editorColourSchemeWindow, pipCreatorWindow;

    std::unique_ptr<FileLogger> logger;

    int numExamples = 0;
    std::unique_ptr<AlertWindow> demoRunnerAlert;
    bool hasScannedForDemoRunnerExecutable = false, hasScannedForDemoRunnerProject = false;
    File lastJUCEPath, lastDemoRunnerExectuableFile, lastDemoRunnerProjectFile;

    int selectedColourSchemeIndex = 0, selectedEditorColourSchemeIndex = 0;

    std::unique_ptr<FileChooser> chooser;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerApplication)
    JUCE_DECLARE_WEAK_REFERENCEABLE (ProjucerApplication)
};
