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

    ApplicationCommandManager& commandManager = ProjucerApplication::getCommandManager();

    // Register all the app commands..
    commandManager.registerAllCommandsForTarget (this);
    commandManager.registerAllCommandsForTarget (getProjectContentComponent());

    // update key mappings..
    {
        commandManager.getKeyMappings()->resetToDefaultMappings();

        ScopedPointer<XmlElement> keys (getGlobalProperties().getXmlValue ("keyMappings"));

        if (keys != nullptr)
            commandManager.getKeyMappings()->restoreFromXml (*keys);

        addKeyListener (commandManager.getKeyMappings());
    }

    // don't want the window to take focus when the title-bar is clicked..
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

    // save the current size and position to our settings file..
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
        jassert (getProjectContentComponent() != nullptr);
    }
}

void MainWindow::makeVisible()
{
    restoreWindowPosition();
    setVisible (true);
    addToDesktop();  // (must add before restoring size so that fullscreen will work)
    restoreWindowPosition();

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

bool MainWindow::closeProject (Project* project)
{
    jassert (project == currentProject && project != nullptr);

    if (project == nullptr)
        return true;

    project->getStoredProperties().setValue (getProjectWindowPosName(), getWindowStateAsString());

    if (auto* pcc = getProjectContentComponent())
    {
        pcc->saveTreeViewState();
        pcc->saveOpenDocumentList();
        pcc->hideEditor();
    }

    if (! ProjucerApplication::getApp().openDocumentManager.closeAllDocumentsUsingProject (*project, true))
        return false;

    auto r = project->saveIfNeededAndUserAgrees();

    if (r == FileBasedDocument::savedOk)
    {
        setProject (nullptr);
        return true;
    }

    return false;
}

bool MainWindow::closeCurrentProject()
{
    return currentProject == nullptr || closeProject (currentProject.get());
}

void MainWindow::setProject (Project* newProject)
{
    createProjectContentCompIfNeeded();
    getProjectContentComponent()->setProject (newProject);
    currentProject.reset (newProject);

    if (currentProject != nullptr)
        projectNameValue.referTo (currentProject->getProjectValue (Ids::name));
    else
        projectNameValue.referTo (Value());

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
        ScopedPointer<Project> newDoc (new Project (file));

        auto result = newDoc->loadFrom (file, true);

        if (result.wasOk() && closeCurrentProject())
        {
            setProject (newDoc.get());
            newDoc.release()->setChangedFlag (false);

            jassert (getProjectContentComponent() != nullptr);
            getProjectContentComponent()->reloadLastOpenDocuments();

            if (auto* p = getProject())
                p->updateDeprecatedProjectSettingsInteractively();

            return true;
        }
    }
    else if (file.exists())
    {
        return getProjectContentComponent()->showEditorForFile (file, true);
    }

    return false;
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

        if (canOpenFile (f) && openFile (f))
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
                        auto projectFile = project->getFile();
                        setProject (nullptr);
                        openFile (projectFile);
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

void MainWindow::showNewProjectWizard()
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

void MainWindow::valueChanged (Value& v)
{
    if (v == Value())
        setName ("Projucer");
    else
        setName (projectNameValue.toString() + " - Projucer");
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
        if (! windows[0]->closeCurrentProject())
            return false;

        windows.remove (0);
    }

    return true;
}

void MainWindowList::createWindowIfNoneAreOpen()
{
    if (windows.size() == 0)
        createNewMainWindow()->showNewProjectWizard();
}

void MainWindowList::closeWindow (MainWindow* w)
{
    jassert (windows.contains (w));

   #if ! JUCE_MAC
    if (windows.size() == 1)
    {
        JUCEApplicationBase::getInstance()->systemRequestedQuit();
    }
    else
   #endif
    {
        if (w->closeCurrentProject())
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
        auto previousFrontWindow = getFrontmostWindow();

        auto* w = getOrCreateEmptyWindow();
        bool ok = w->openFile (file);

        w->makeVisible();
        avoidSuperimposedWindows (w);

        if (openInBackground && (previousFrontWindow != nullptr))
            previousFrontWindow->toFront (true);

        return ok;
    }

    if (file.exists())
        return getFrontmostWindow()->openFile (file);

    return false;
}

MainWindow* MainWindowList::createNewMainWindow()
{
    auto w = new MainWindow();
    windows.add (w);
    w->restoreWindowPosition();
    avoidSuperimposedWindows (w);
    return w;
}

MainWindow* MainWindowList::getFrontmostWindow (bool createIfNotFound)
{
    if (windows.isEmpty())
    {
        if (createIfNotFound)
        {
            auto* w = createNewMainWindow();
            avoidSuperimposedWindows (w);
            w->makeVisible();
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

void MainWindowList::avoidSuperimposedWindows (MainWindow* const mw)
{
    for (int i = windows.size(); --i >= 0;)
    {
        auto* other = windows.getUnchecked(i);

        auto b1 = mw->getBounds();
        auto b2 = other->getBounds();

        if (mw != other
             && std::abs (b1.getX() - b2.getX()) < 3
             && std::abs (b1.getY() - b2.getY()) < 3
             && std::abs (b1.getRight()  - b2.getRight()) < 3
             && std::abs (b1.getBottom() - b2.getBottom()) < 3)
        {
            int dx = 40, dy = 30;

            if (b1.getCentreX() >= mw->getScreenBounds().getCentreX())   dx = -dx;
            if (b1.getCentreY() >= mw->getScreenBounds().getCentreY())   dy = -dy;

            mw->setBounds (b1.translated (dx, dy));
        }
    }
}

void MainWindowList::saveCurrentlyOpenProjectList()
{
    Array<File> projects;
    auto& desktop = Desktop::getInstance();

    for (int i = 0; i < desktop.getNumComponents(); ++i)
    {
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
            if (auto* p = mw->getProject())
                projects.add (p->getFile());
    }

    getAppSettings().setLastProjects (projects);
}

void MainWindowList::reopenLastProjects()
{
    for (auto& p : getAppSettings().getLastProjects())
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
