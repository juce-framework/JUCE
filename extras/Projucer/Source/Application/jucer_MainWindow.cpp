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
#include "jucer_Application.h"
#include "jucer_MainWindow.h"
#include "../Wizards/jucer_NewProjectWizardClasses.h"
#include "../Utility/UI/jucer_JucerTreeViewBase.h"
#include "../ProjectSaving/jucer_ProjectSaver.h"

//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (ProjucerApplication::getApp().getApplicationName(),
                      ProjucerApplication::getApp().lookAndFeel.getCurrentColourScheme()
                                                   .getUIColour (LookAndFeel_V4::ColourScheme::UIColour::windowBackground),
                      DocumentWindow::allButtons,
                      false)
{
    setUsingNativeTitleBar (true);

   #if ! JUCE_MAC
    setMenuBar (ProjucerApplication::getApp().getMenuModel());
   #endif

    createProjectContentCompIfNeeded();

    setResizable (true, false);
    centreWithSize (800, 600);

    auto& commandManager = ProjucerApplication::getCommandManager();

    auto registerAllAppCommands = [&]
    {
        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (getProjectContentComponent());
    };

    auto updateAppKeyMappings = [&]
    {
        commandManager.getKeyMappings()->resetToDefaultMappings();

        if (auto keys = getGlobalProperties().getXmlValue ("keyMappings"))
            commandManager.getKeyMappings()->restoreFromXml (*keys);

        addKeyListener (commandManager.getKeyMappings());
    };

    registerAllAppCommands();
    updateAppKeyMappings();

    setWantsKeyboardFocus (false);
    getLookAndFeel().setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
    projectNameValue.addListener (this);

    setResizeLimits (600, 500, 32000, 32000);
}

MainWindow::~MainWindow()
{
   #if ! JUCE_MAC
    setMenuBar (nullptr);
   #endif

    removeKeyListener (ProjucerApplication::getCommandManager().getKeyMappings());
    getGlobalProperties().setValue ("lastMainWindowPos", getWindowStateAsString());

    clearContentComponent();
    currentProject.reset();
}

void MainWindow::createProjectContentCompIfNeeded()
{
    if (getProjectContentComponent() == nullptr)
    {
        clearContentComponent();
        setContentOwned (new ProjectContentComponent(), false);
    }
}

void MainWindow::setTitleBarIcon()
{
    if (auto* peer = getPeer())
    {
        if (currentProject != nullptr)
        {
            peer->setRepresentedFile (currentProject->getFile());
            peer->setIcon (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        }
        else
        {
            peer->setRepresentedFile ({});
        }
    }
}

void MainWindow::makeVisible()
{
    setVisible (true);
    addToDesktop();
    restoreWindowPosition();
    setTitleBarIcon();
    getContentComponent()->grabKeyboardFocus();
}

ProjectContentComponent* MainWindow::getProjectContentComponent() const
{
    return dynamic_cast<ProjectContentComponent*> (getContentComponent());
}

void MainWindow::closeButtonPressed()
{
    ProjucerApplication::getApp().mainWindowList.closeWindow (this);
}

bool MainWindow::closeCurrentProject (bool askUserToSave)
{
    if (currentProject == nullptr)
        return true;

    currentProject->getStoredProperties().setValue (getProjectWindowPosName(), getWindowStateAsString());

    if (auto* pcc = getProjectContentComponent())
    {
        pcc->saveTreeViewState();
        pcc->saveOpenDocumentList();
        pcc->hideEditor();
    }

    if (ProjucerApplication::getApp().openDocumentManager
         .closeAllDocumentsUsingProject (*currentProject, askUserToSave))
    {
        if (! askUserToSave || (currentProject->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk))
        {
            setProject (nullptr);
            return true;
        }
    }

    return false;
}

void MainWindow::moveProject (File newProjectFileToOpen)
{
    auto openInIDE = currentProject->shouldOpenInIDEAfterSaving();

    closeCurrentProject (false);
    openFile (newProjectFileToOpen);

    if (currentProject != nullptr)
    {
        ProjucerApplication::getApp().getCommandManager().invokeDirectly (openInIDE ? CommandIDs::saveAndOpenInIDE
                                                                                    : CommandIDs::saveProject,
                                                                          false);
    }
}

void MainWindow::setProject (std::unique_ptr<Project> newProject)
{
    if (newProject == nullptr)
    {
        getProjectContentComponent()->setProject (nullptr);
        projectNameValue.referTo (Value());

        currentProject.reset();
    }
    else
    {
        currentProject = std::move (newProject);

        createProjectContentCompIfNeeded();
        getProjectContentComponent()->setProject (currentProject.get());
        projectNameValue.referTo (currentProject->getProjectValue (Ids::name));

        if (auto* peer = getPeer())
            peer->setRepresentedFile (currentProject->getFile());
    }

    ProjucerApplication::getCommandManager().commandStatusChanged();
}

void MainWindow::restoreWindowPosition()
{
    String windowState;

    if (currentProject != nullptr)
        windowState = currentProject->getStoredProperties().getValue (getProjectWindowPosName());

    if (windowState.isEmpty())
        windowState = getGlobalProperties().getValue ("lastMainWindowPos");

    restoreWindowStateFromString (windowState);
}

bool MainWindow::canOpenFile (const File& file) const
{
    return (! file.isDirectory())
             && (file.hasFileExtension (Project::projectFileExtension)
                  || ProjucerApplication::getApp().openDocumentManager.canOpenFile (file));
}

bool MainWindow::openFile (const File& file)
{
    createProjectContentCompIfNeeded();

    if (file.hasFileExtension (Project::projectFileExtension))
    {
        auto newDoc = std::make_unique<Project> (file);
        auto result = newDoc->loadFrom (file, true);

        if (result.wasOk() && closeCurrentProject (true))
        {
            setProject (std::move (newDoc));
            currentProject->setChangedFlag (false);

            getProjectContentComponent()->reloadLastOpenDocuments();
            currentProject->updateDeprecatedProjectSettingsInteractively();

            return true;
        }
    }
    else if (file.exists())
    {
        return getProjectContentComponent()->showEditorForFile (file, true);
    }

    return false;
}

bool MainWindow::tryToOpenPIP (const File& pipFile)
{
    PIPGenerator generator (pipFile);

    if (! generator.hasValidPIP())
        return false;

    auto generatorResult = generator.createJucerFile();

    if (generatorResult != Result::ok())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "PIP Error.",
                                          generatorResult.getErrorMessage());

        return false;
    }


    if (! generator.createMainCpp())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "PIP Error.",
                                          "Failed to create Main.cpp.");

        return false;
    }

    if (! ProjucerApplication::getApp().mainWindowList.openFile (generator.getJucerFile()))
        return false;

    openPIP (generator);
    return true;
}

static bool isDivider (const String& line)
{
    auto afterIndent = line.trim();

    if (afterIndent.startsWith ("//") && afterIndent.length() > 20)
    {
        afterIndent = afterIndent.substring (2);

        if (afterIndent.containsOnly ("=")
            || afterIndent.containsOnly ("/")
            || afterIndent.containsOnly ("-"))
        {
            return true;
        }
    }

    return false;
}

static bool isEndOfCommentBlock (const String& line)
{
    if (line.contains ("*/"))
        return true;

    return false;
}

static int getIndexOfCommentBlockStart (const StringArray& lines, int blockEndIndex)
{
    for (int i = blockEndIndex; i >= 0; --i)
    {
        if (lines[i].contains ("/*"))
            return i;
    }

    return  0;
}

static int findBestLineToScrollTo (StringArray lines, StringRef className)
{
    for (auto line : lines)
    {
        if (line.contains ("struct " + className) || line.contains ("class " + className))
        {
            auto index = lines.indexOf (line);

            if (isDivider (lines[index - 1]))
                return index - 1;

            if (isEndOfCommentBlock (lines[index - 1]))
            {
                auto blockStartIndex = getIndexOfCommentBlockStart (lines, index - 1);

                if (blockStartIndex > 0 && isDivider (lines [blockStartIndex - 1]))
                    return blockStartIndex - 1;

                return blockStartIndex;
            }

            return lines.indexOf (line);
        }
    }

    return 0;
}

void MainWindow::openPIP (PIPGenerator& generator)
{
    if (auto* window = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (generator.getJucerFile()))
    {
        if (auto* project = window->getProject())
        {
            project->setTemporaryDirectory (generator.getOutputDirectory());

            ProjectSaver liveBuildSaver (*project, project->getFile());
            liveBuildSaver.saveContentNeededForLiveBuild();

            if (auto* pcc = window->getProjectContentComponent())
            {
                pcc->invokeDirectly (CommandIDs::toggleBuildEnabled, true);
                pcc->invokeDirectly (CommandIDs::buildNow, true);
                pcc->invokeDirectly (CommandIDs::toggleContinuousBuild, true);

                auto fileToDisplay = generator.getPIPFile();

                if (fileToDisplay != File())
                {
                    pcc->showEditorForFile (fileToDisplay, true);

                    if (auto* sourceCodeEditor = dynamic_cast <SourceCodeEditor*> (pcc->getEditorComponent()))
                    {
                        sourceCodeEditor->editor->scrollToLine (findBestLineToScrollTo (StringArray::fromLines (fileToDisplay.loadFileAsString()),
                                                                                        generator.getMainClassName()));
                    }
                }

            }
        }
    }
}

bool MainWindow::isInterestedInFileDrag (const StringArray& filenames)
{
    for (auto& filename : filenames)
        if (canOpenFile (File (filename)))
            return true;

    return false;
}

void MainWindow::filesDropped (const StringArray& filenames, int /*mouseX*/, int /*mouseY*/)
{
    for (auto& filename : filenames)
    {
        const File f (filename);

        if (tryToOpenPIP (f))
            continue;

        if (! isPIPFile (f) && (canOpenFile (f) && openFile (f)))
            break;
    }
}

bool MainWindow::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles)
{
    if (auto* tv = dynamic_cast<TreeView*> (sourceDetails.sourceComponent.get()))
    {
        Array<JucerTreeViewBase*> selected;

        for (int i = tv->getNumSelectedItems(); --i >= 0;)
            if (auto* b = dynamic_cast<JucerTreeViewBase*> (tv->getSelectedItem(i)))
                selected.add (b);

        if (! selected.isEmpty())
        {
            for (int i = selected.size(); --i >= 0;)
            {
                if (auto* jtvb = selected.getUnchecked(i))
                {
                    auto f = jtvb->getDraggableFile();

                    if (f.existsAsFile())
                        files.add (f.getFullPathName());
                }
            }

            canMoveFiles = false;
            return ! files.isEmpty();
        }
    }

    return false;
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (auto* pcc = getProjectContentComponent())
        pcc->updateMissingFileStatuses();

    ProjucerApplication::getApp().openDocumentManager.reloadModifiedFiles();

    if (auto* p = getProject())
    {
        if (p->hasProjectBeenModified())
        {
            Component::SafePointer<Component> safePointer (this);

            MessageManager::callAsync ([=] ()
            {
                if (safePointer == nullptr)
                    return; // bail out if the window has been deleted

                auto result = AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                                            TRANS ("The .jucer file has been modified since the last save."),
                                                            TRANS ("Do you want to keep the current project or re-load from disk?"),
                                                            TRANS ("Keep"),
                                                            TRANS ("Re-load from disk"));

                if (safePointer == nullptr)
                    return;

                if (result == 0)
                {
                    if (auto* project = getProject())
                    {
                        auto oldTemporaryDirectory = project->getTemporaryDirectory();

                        auto projectFile = project->getFile();
                        setProject (nullptr);
                        openFile (projectFile);

                        if (oldTemporaryDirectory != File())
                            if (auto* newProject = getProject())
                                newProject->setTemporaryDirectory (oldTemporaryDirectory);
                    }
                }
                else
                {
                    ProjucerApplication::getApp().getCommandManager().invokeDirectly (CommandIDs::saveProject, true);
                }
            });
        }
    }
}

void MainWindow::showStartPage()
{
    jassert (currentProject == nullptr);

    setContentOwned (createNewProjectWizardComponent(), true);

    centreWithSize (900, 630);
    setVisible (true);
    addToDesktop();

    getContentComponent()->grabKeyboardFocus();
}

//==============================================================================
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return nullptr;
}

void MainWindow::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] =
    {
        CommandIDs::closeWindow,
        CommandIDs::goToPreviousWindow,
        CommandIDs::goToNextWindow
    };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::closeWindow:
            result.setInfo ("Close Window", "Closes the current window", CommandCategories::general, 0);
            result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::goToPreviousWindow:
            result.setInfo ("Previous Window", "Activates the previous window", CommandCategories::general, 0);
            result.setActive (ProjucerApplication::getApp().mainWindowList.windows.size() > 1);
            result.defaultKeypresses.add (KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier | ModifierKeys::ctrlModifier, 0));
            break;

        case CommandIDs::goToNextWindow:
            result.setInfo ("Next Window", "Activates the next window", CommandCategories::general, 0);
            result.setActive (ProjucerApplication::getApp().mainWindowList.windows.size() > 1);
            result.defaultKeypresses.add (KeyPress (KeyPress::tabKey, ModifierKeys::ctrlModifier, 0));
            break;

        default:
            break;
    }
}

bool MainWindow::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::closeWindow:
            closeButtonPressed();
            break;

        case CommandIDs::goToPreviousWindow:
            ProjucerApplication::getApp().mainWindowList.goToSiblingWindow (this, -1);
            break;

        case CommandIDs::goToNextWindow:
            ProjucerApplication::getApp().mainWindowList.goToSiblingWindow (this, 1);
            break;

        default:
            return false;
    }

    return true;
}

void MainWindow::valueChanged (Value&)
{
    if (currentProject != nullptr)
        setName (currentProject->getProjectNameString() + " - Projucer");
    else
        setName ("Projucer");
}

//==============================================================================
MainWindowList::MainWindowList()
{
}

void MainWindowList::forceCloseAllWindows()
{
    windows.clear();
}

bool MainWindowList::askAllWindowsToClose()
{
    saveCurrentlyOpenProjectList();

    while (windows.size() > 0)
    {
        if (! windows[0]->closeCurrentProject (true))
            return false;

        windows.remove (0);
    }

    return true;
}

void MainWindowList::createWindowIfNoneAreOpen()
{
    if (windows.isEmpty())
        createNewMainWindow()->showStartPage();
}

void MainWindowList::closeWindow (MainWindow* w)
{
    jassert (windows.contains (w));

   #if ! JUCE_MAC
    if (windows.size() == 1 && ! isInReopenLastProjects)
    {
        JUCEApplicationBase::getInstance()->systemRequestedQuit();
    }
    else
   #endif
    {
        if (w->closeCurrentProject (true))
        {
            windows.removeObject (w);
            saveCurrentlyOpenProjectList();
        }
    }
}

void MainWindowList::goToSiblingWindow (MainWindow* w, int delta)
{
    auto index = windows.indexOf (w);

    if (index >= 0)
        if (auto* next = windows[(index + delta + windows.size()) % windows.size()])
            next->toFront (true);
}

void MainWindowList::openDocument (OpenDocumentManager::Document* doc, bool grabFocus)
{
    auto& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
    {
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
        {
            if (auto* pcc = mw->getProjectContentComponent())
            {
                if (pcc->hasFileInRecentList (doc->getFile()))
                {
                    mw->toFront (true);
                    mw->getProjectContentComponent()->showDocument (doc, grabFocus);
                    return;
                }
            }
        }
    }

    getFrontmostWindow()->getProjectContentComponent()->showDocument (doc, grabFocus);
}

bool MainWindowList::openFile (const File& file, bool openInBackground)
{
    for (auto* w : windows)
    {
        if (w->getProject() != nullptr && w->getProject()->getFile() == file)
        {
            w->toFront (true);
            return true;
        }
    }

    if (file.hasFileExtension (Project::projectFileExtension))
    {
        WeakReference<Component> previousFrontWindow (getFrontmostWindow());

        auto* w = getOrCreateEmptyWindow();
        jassert (w != nullptr);

        if (w->openFile (file))
        {
            w->makeVisible();
            checkWindowBounds (*w);

            if (openInBackground && previousFrontWindow != nullptr)
                previousFrontWindow->toFront (true);

            return true;
        }

        closeWindow (w);
        return false;
    }

    if (getFrontmostWindow()->tryToOpenPIP (file))
        return true;

    if (! isPIPFile (file) && file.exists())
        return getFrontmostWindow()->openFile (file);

    return false;
}

MainWindow* MainWindowList::createNewMainWindow()
{
    windows.add (new MainWindow());
    return windows.getLast();
}

MainWindow* MainWindowList::getFrontmostWindow (bool createIfNotFound)
{
    if (windows.isEmpty())
    {
        if (createIfNotFound)
        {
            auto* w = createNewMainWindow();
            jassert (w != nullptr);

            w->makeVisible();
            checkWindowBounds (*w);

            return w;
        }

        return nullptr;
    }

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        auto* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));

        if (windows.contains (mw))
            return mw;
    }

    return windows.getLast();
}

MainWindow* MainWindowList::getOrCreateEmptyWindow()
{
    if (windows.size() == 0)
        return createNewMainWindow();

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        auto* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));

        if (windows.contains (mw) && mw->getProject() == nullptr)
            return mw;
    }

    return createNewMainWindow();
}

MainWindow* MainWindowList::getMainWindowForFile (const File& file)
{
    if (windows.size() > 0)
    {
        for (auto* window : windows)
        {
            if (auto* project = window->getProject())
            {
                if (project->getFile() == file)
                    return window;
            }
        }
    }

    return nullptr;
}

void MainWindowList::checkWindowBounds (MainWindow& windowToCheck)
{
    auto avoidSuperimposedWindows = [&]
    {
        for (auto* otherWindow : windows)
        {
            if (otherWindow == nullptr || otherWindow == &windowToCheck)
                continue;

            auto boundsToCheck = windowToCheck.getScreenBounds();
            auto otherBounds = otherWindow->getScreenBounds();

            if (std::abs (boundsToCheck.getX() - otherBounds.getX()) < 3
                 && std::abs (boundsToCheck.getY() - otherBounds.getY()) < 3
                 && std::abs (boundsToCheck.getRight()  - otherBounds.getRight()) < 3
                 && std::abs (boundsToCheck.getBottom() - otherBounds.getBottom()) < 3)
            {
                int dx = 40, dy = 30;

                if (otherBounds.getCentreX() >= boundsToCheck.getCentreX())  dx = -dx;
                if (otherBounds.getCentreY() >= boundsToCheck.getCentreY())  dy = -dy;

                windowToCheck.setBounds (boundsToCheck.translated (dx, dy));
            }
        }
    };

    auto ensureWindowIsFullyOnscreen = [&]
    {
        auto windowBounds = windowToCheck.getScreenBounds();
        auto screenLimits = Desktop::getInstance().getDisplays().findDisplayForRect (windowBounds).userArea;

        if (auto* peer = windowToCheck.getPeer())
            peer->getFrameSize().subtractFrom (screenLimits);

        auto constrainedX = jlimit (screenLimits.getX(), screenLimits.getRight()  - windowBounds.getWidth(),  windowBounds.getX());
        auto constrainedY = jlimit (screenLimits.getY(), screenLimits.getBottom() - windowBounds.getHeight(), windowBounds.getY());

        Point<int> constrainedTopLeft (constrainedX, constrainedY);

        if (windowBounds.getPosition() != constrainedTopLeft)
            windowToCheck.setTopLeftPosition (constrainedTopLeft);
    };

    avoidSuperimposedWindows();
    ensureWindowIsFullyOnscreen();
}

void MainWindowList::saveCurrentlyOpenProjectList()
{
    Array<File> projects;
    auto& desktop = Desktop::getInstance();

    for (int i = 0; i < desktop.getNumComponents(); ++i)
    {
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
            if (auto* p = mw->getProject())
                if (! p->isTemporaryProject())
                    projects.add (p->getFile());
    }

    getAppSettings().setLastProjects (projects);
}

void MainWindowList::reopenLastProjects()
{
    const ScopedValueSetter<bool> setter (isInReopenLastProjects, true);

    for (auto& p : getAppSettings().getLastProjects())
        if (p.existsAsFile())
            openFile (p, true);
}

void MainWindowList::sendLookAndFeelChange()
{
    for (auto* w : windows)
        w->sendLookAndFeelChange();
}

Project* MainWindowList::getFrontmostProject()
{
    auto& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
            if (auto* p = mw->getProject())
                return p;

    return nullptr;
}
