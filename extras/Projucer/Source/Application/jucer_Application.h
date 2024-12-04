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

#include "jucer_MainWindow.h"
#include "../Project/Modules/jucer_Modules.h"
#include "jucer_AutoUpdater.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "../Utility/UI/jucer_ProjucerLookAndFeel.h"

//==============================================================================
class ProjucerApplication final : public JUCEApplication,
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

    //==============================================================================
   #if JUCE_MAC
    class AppleMenuRebuildListener final : private MenuBarModel::Listener
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
    ScopedMessageBox messageBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerApplication)
    JUCE_DECLARE_WEAK_REFERENCEABLE (ProjucerApplication)
};
