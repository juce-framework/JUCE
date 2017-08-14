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
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Utility/jucer_ProjucerLookAndFeel.h"
#include "../Licenses/jucer_LicenseController.h"

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
    void createToolsMenu (PopupMenu&);
    void createExtraAppleMenuItems (PopupMenu&);
    void handleMainMenuCommand (int menuItemID);

    //==============================================================================
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    //==============================================================================
    void createNewProject();
    void updateNewlyOpenedProject (Project&);
    void askUserToOpenFile();
    bool openFile (const File&);
    bool closeAllDocuments (bool askUserToSave);
    bool closeAllMainWindows();

    PropertiesFile::Options getPropertyFileOptionsFor (const String& filename, bool isProjectSettings);

    //==============================================================================
    void showUTF8ToolWindow();
    void showSVGPathDataToolWindow();

    void showAboutWindow();
    void showApplicationUsageDataAgreementPopup();
    void dismissApplicationUsageDataAgreementPopup();

    void showPathsWindow();
    void showEditorColourSchemeWindow();

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
    ProjucerLookAndFeel lookAndFeel;

    ScopedPointer<StoredSettings> settings;
    ScopedPointer<Icons> icons;

    struct MainMenuModel;
    ScopedPointer<MainMenuModel> menuModel;

    MainWindowList mainWindowList;
    OpenDocumentManager openDocumentManager;
    ScopedPointer<ApplicationCommandManager> commandManager;

    ScopedPointer<Component> utf8Window, svgPathWindow, aboutWindow, applicationUsageDataWindow,
                             pathsWindow, editorColourSchemeWindow;

    ScopedPointer<FileLogger> logger;

    bool isRunningCommandLine;
    ScopedPointer<ChildProcessCache> childProcessCache;
    ScopedPointer<LicenseController> licenseController;

private:
    void* server = nullptr;

    ScopedPointer<LatestVersionChecker> versionChecker;
    TooltipWindow tooltipWindow;

    void loginOrLogout();

    bool checkEULA();
    bool currentEULAHasBeenAcceptedPreviously() const;
    String getEULAChecksumProperty() const;
    void setCurrentEULAAccepted (bool hasBeenAccepted) const;

    void handleAsyncUpdate() override;
    void initCommandManager();

    //==============================================================================
    void setColourScheme (int index, bool saveSetting);

    void setEditorColourScheme (int index, bool saveSetting);
    void updateEditorColourSchemeIfNeeded();

    int selectedColourSchemeIndex = 0;

    int selectedEditorColourSchemeIndex = 0;
    int numEditorColourSchemes = 0;
};
