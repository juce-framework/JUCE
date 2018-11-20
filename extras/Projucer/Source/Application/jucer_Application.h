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

#include "jucer_MainWindow.h"
#include "../Project/jucer_Module.h"
#include "jucer_AutoUpdater.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "../Utility/UI/jucer_ProjucerLookAndFeel.h"
#include "../Licenses/jucer_LicenseController.h"

#if JUCE_MODULE_AVAILABLE_juce_analytics
 #include "jucer_ProjucerAnalytics.h"
#endif

struct ChildProcessCache;

//==============================================================================
class ProjucerApplication   : public JUCEApplication,
                              private AsyncUpdater,
                              private LicenseController::StateChangedCallback
{
public:
    ProjucerApplication();

    static ProjucerApplication& getApp();
    static ApplicationCommandManager& getCommandManager();

    //==============================================================================
    void initialise (const String& commandLine) override;
    void initialiseBasics();
    bool initialiseLogger (const char* filePrefix);
    void initialiseWindows (const String& commandLine);

    void shutdown() override;
    void systemRequestedQuit() override;
    void deleteLogger();

    //==============================================================================
    const String getApplicationName() override       { return "Projucer"; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }

    String getVersionDescription() const;
    bool moreThanOneInstanceAllowed() override       { return true; } // this is handled manually in initialise()

    void anotherInstanceStarted (const String& commandLine) override;

    //==============================================================================
    MenuBarModel* getMenuModel();
    StringArray getMenuNames();
    void createMenu (PopupMenu&, const String& menuName);
    void createFileMenu (PopupMenu&);
    void createEditMenu (PopupMenu&);
    void createViewMenu (PopupMenu&);
    void createBuildMenu (PopupMenu&);
    void createColourSchemeItems (PopupMenu&);
    void createWindowMenu (PopupMenu&);
    void createDocumentMenu (PopupMenu&);
    void createToolsMenu (PopupMenu&);
    void createHelpMenu (PopupMenu&);
    void createExtraAppleMenuItems (PopupMenu&);
    void handleMainMenuCommand (int menuItemID);

    //==============================================================================
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    //==============================================================================
    void createNewProject();
    void createNewProjectFromClipboard();
    void createNewPIP();
    void askUserToOpenFile();
    bool openFile (const File&);
    bool closeAllDocuments (bool askUserToSave);
    bool closeAllMainWindows();
    void closeAllMainWindowsAndQuitIfNeeded();
    void clearRecentFiles();

    PropertiesFile::Options getPropertyFileOptionsFor (const String& filename, bool isProjectSettings);

    //==============================================================================
    void showUTF8ToolWindow();
    void showSVGPathDataToolWindow();

    void showAboutWindow();
    void showApplicationUsageDataAgreementPopup();
    void dismissApplicationUsageDataAgreementPopup();

    void showPathsWindow (bool highlightJUCEPath = false);
    void showEditorColourSchemeWindow();

    void showPIPCreatorWindow();

    void launchForumBrowser();
    void launchModulesBrowser();
    void launchClassesBrowser();
    void launchTutorialsBrowser();

    void updateAllBuildTabs();
    LatestVersionChecker* createVersionChecker() const;

    //==============================================================================
    void licenseStateChanged (const LicenseState&) override;
    void doLogout();

    bool isPaidOrGPL() const              { return licenseController == nullptr || licenseController->getState().isPaidOrGPL(); }

    //==============================================================================
    void selectEditorColourSchemeWithName (const String& schemeName);
    static bool isEditorColourSchemeADefaultScheme (const StringArray& schemes, int editorColourSchemeIndex);
    static int getEditorColourSchemeForGUIColourScheme (const StringArray& schemes, int guiColourSchemeIndex);

    //==============================================================================
    void setAnalyticsEnabled (bool);

    //==============================================================================
    ProjucerLookAndFeel lookAndFeel;

    std::unique_ptr<StoredSettings> settings;
    std::unique_ptr<Icons> icons;

    struct MainMenuModel;
    std::unique_ptr<MainMenuModel> menuModel;

    MainWindowList mainWindowList;
    OpenDocumentManager openDocumentManager;
    std::unique_ptr<ApplicationCommandManager> commandManager;

    std::unique_ptr<Component> utf8Window, svgPathWindow, aboutWindow, applicationUsageDataWindow,
                             pathsWindow, editorColourSchemeWindow, pipCreatorWindow;

    std::unique_ptr<FileLogger> logger;

    bool isRunningCommandLine;
    std::unique_ptr<ChildProcessCache> childProcessCache;
    std::unique_ptr<LicenseController> licenseController;

private:
    void* server = nullptr;

    std::unique_ptr<LatestVersionChecker> versionChecker;
    TooltipWindow tooltipWindow;

    void loginOrLogout();

    bool checkEULA();
    bool currentEULAHasBeenAcceptedPreviously() const;
    String getEULAChecksumProperty() const;
    void setCurrentEULAAccepted (bool hasBeenAccepted) const;

    void handleAsyncUpdate() override;
    void initCommandManager();

    void deleteTemporaryFiles() const noexcept;

    void createExamplesPopupMenu (PopupMenu&) noexcept;
    Array<File> getSortedExampleDirectories() noexcept;
    Array<File> getSortedExampleFilesInDirectory (const File&) const noexcept;

    bool findWindowAndOpenPIP (const File&);

    File getJUCEExamplesDirectoryPathFromGlobal() noexcept;
    void findAndLaunchExample (int);
    File findDemoRunnerExecutable() noexcept;
    File findDemoRunnerProject() noexcept;
    void launchDemoRunner();

    int numExamples = 0;
    std::unique_ptr<AlertWindow> demoRunnerAlert;

   #if JUCE_LINUX
    ChildProcess makeProcess;
   #endif

    void resetAnalytics() noexcept;
    void setupAnalytics();

    void showSetJUCEPathAlert();
    std::unique_ptr<AlertWindow> pathAlert;

    //==============================================================================
    void setColourScheme (int index, bool saveSetting);

    void setEditorColourScheme (int index, bool saveSetting);
    void updateEditorColourSchemeIfNeeded();

    int selectedColourSchemeIndex = 0;

    int selectedEditorColourSchemeIndex = 0;
    int numEditorColourSchemes = 0;
};
