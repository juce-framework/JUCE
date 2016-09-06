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

#include "../jucer_Headers.h"
#include "jucer_Application.h"
#include "jucer_MainWindow.h"
#include "jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Wizards/jucer_NewProjectWizardClasses.h"
#include "../Utility/jucer_JucerTreeViewBase.h"


//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (ProjucerApplication::getApp().getApplicationName(),
                      Colour::greyLevel (0.6f),
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
    currentProject = nullptr;
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

    if (ProjectContentComponent* const pcc = getProjectContentComponent())
    {
        pcc->saveTreeViewState();
        pcc->saveOpenDocumentList();
        pcc->hideEditor();
    }

    if (! ProjucerApplication::getApp().openDocumentManager.closeAllDocumentsUsingProject (*project, true))
        return false;

    FileBasedDocument::SaveResult r = project->saveIfNeededAndUserAgrees();

    if (r == FileBasedDocument::savedOk)
    {
        setProject (nullptr);
        return true;
    }

    return false;
}

bool MainWindow::closeCurrentProject()
{
    return currentProject == nullptr || closeProject (currentProject);
}

void MainWindow::setProject (Project* newProject)
{
    createProjectContentCompIfNeeded();
    getProjectContentComponent()->setProject (newProject);
    currentProject = newProject;
    getProjectContentComponent()->updateMainWindowTitle();
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

        Result result (newDoc->loadFrom (file, true));

        if (result.wasOk() && closeCurrentProject())
        {
            setProject (newDoc);
            newDoc.release()->setChangedFlag (false);

            jassert (getProjectContentComponent() != nullptr);
            getProjectContentComponent()->reloadLastOpenDocuments();

            if (Project* p = getProject())
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
    for (int i = filenames.size(); --i >= 0;)
        if (canOpenFile (File (filenames[i])))
            return true;

    return false;
}

void MainWindow::filesDropped (const StringArray& filenames, int /*mouseX*/, int /*mouseY*/)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (canOpenFile (f) && openFile (f))
            break;
    }
}

bool MainWindow::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles)
{
    if (TreeView* tv = dynamic_cast<TreeView*> (sourceDetails.sourceComponent.get()))
    {
        Array<JucerTreeViewBase*> selected;

        for (int i = tv->getNumSelectedItems(); --i >= 0;)
            if (JucerTreeViewBase* b = dynamic_cast<JucerTreeViewBase*> (tv->getSelectedItem(i)))
                selected.add (b);

        if (selected.size() > 0)
        {
            for (int i = selected.size(); --i >= 0;)
            {
                if (JucerTreeViewBase* jtvb = selected.getUnchecked(i))
                {
                    const File f (jtvb->getDraggableFile());

                    if (f.existsAsFile())
                        files.add (f.getFullPathName());
                }
            }

            canMoveFiles = false;
            return files.size() > 0;
        }
    }

    return false;
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (ProjectContentComponent* const pcc = getProjectContentComponent())
        pcc->updateMissingFileStatuses();

    ProjucerApplication::getApp().openDocumentManager.reloadModifiedFiles();

    if (Project* p = getProject())
    {
        if (p->hasProjectBeenModified())
        {
            const int r = AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                                           TRANS ("The .jucer file has been modified since the last save."),
                                                           TRANS ("Do you want to keep the current project or re-load from disk?"),
                                                           TRANS ("Keep"),
                                                           TRANS ("Re-load from disk"));

            if (r == 0)
            {
                File projectFile = p->getFile();
                setProject (nullptr);
                openFile (projectFile);
            }
            else if (r == 1)
            {
                ProjucerApplication::getApp().getCommandManager().invokeDirectly (CommandIDs::saveProject, true);
            }
        }
    }
}

void MainWindow::updateTitle (const String& documentName)
{
    String name (ProjucerApplication::getApp().getApplicationName());

    if (currentProject != nullptr)
    {
        String projectName (currentProject->getDocumentTitle());

        if (currentProject->getFile().getFileNameWithoutExtension() != projectName)
            projectName = currentProject->getFile().getFileName();

        name << "  -  " << projectName;
    }

    if (documentName.isNotEmpty())
        name << "  -  " << documentName;

    setName (name);
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
    const CommandID ids[] = { CommandIDs::closeWindow };

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

        default:
            return false;
    }

    return true;
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

void MainWindowList::openDocument (OpenDocumentManager::Document* doc, bool grabFocus)
{
    Desktop& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
    {
        if (MainWindow* const mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
        {
            if (ProjectContentComponent* pcc = mw->getProjectContentComponent())
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

    getOrCreateFrontmostWindow()->getProjectContentComponent()->showDocument (doc, grabFocus);
}

bool MainWindowList::openFile (const File& file)
{
    for (int i = windows.size(); --i >= 0;)
    {
        MainWindow* const w = windows.getUnchecked(i);

        if (w->getProject() != nullptr && w->getProject()->getFile() == file)
        {
            w->toFront (true);
            return true;
        }
    }

    if (file.hasFileExtension (Project::projectFileExtension))
    {
        MainWindow* const w = getOrCreateEmptyWindow();
        bool ok = w->openFile (file);

        w->makeVisible();
        avoidSuperimposedWindows (w);

        return ok;
    }

    if (file.exists())
        return getOrCreateFrontmostWindow()->openFile (file);

    return false;
}

MainWindow* MainWindowList::createNewMainWindow()
{
    MainWindow* const w = new MainWindow();
    windows.add (w);
    w->restoreWindowPosition();
    avoidSuperimposedWindows (w);
    return w;
}

MainWindow* MainWindowList::getOrCreateFrontmostWindow()
{
    if (windows.size() == 0)
    {
        MainWindow* w = createNewMainWindow();
        avoidSuperimposedWindows (w);
        w->makeVisible();
        return w;
    }

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        MainWindow* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));
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
        MainWindow* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));
        if (windows.contains (mw) && mw->getProject() == nullptr)
            return mw;
    }

    return createNewMainWindow();
}

void MainWindowList::updateAllWindowTitles()
{
    for (int i = 0; i < windows.size(); ++i)
        if (ProjectContentComponent* pc = windows.getUnchecked(i)->getProjectContentComponent())
            pc->updateMainWindowTitle();
}

void MainWindowList::avoidSuperimposedWindows (MainWindow* const mw)
{
    for (int i = windows.size(); --i >= 0;)
    {
        MainWindow* const other = windows.getUnchecked(i);

        const Rectangle<int> b1 (mw->getBounds());
        const Rectangle<int> b2 (other->getBounds());

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
    Desktop& desktop = Desktop::getInstance();

    for (int i = 0; i < desktop.getNumComponents(); ++i)
    {
        if (MainWindow* const mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
            if (Project* p = mw->getProject())
                projects.add (p->getFile());
    }

    getAppSettings().setLastProjects (projects);
}

void MainWindowList::reopenLastProjects()
{
    Array<File> projects (getAppSettings().getLastProjects());

    for (int i = 0; i < projects.size(); ++ i)
        openFile (projects.getReference(i));
}

void MainWindowList::sendLookAndFeelChange()
{
    for (int i = windows.size(); --i >= 0;)
        windows.getUnchecked(i)->sendLookAndFeelChange();
}

Project* MainWindowList::getFrontmostProject()
{
    Desktop& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
        if (MainWindow* const mw = dynamic_cast<MainWindow*> (desktop.getComponent(i)))
            if (Project* p = mw->getProject())
                return p;

    return nullptr;
}

File findDefaultModulesFolder (bool mustContainJuceCoreModule)
{
    const MainWindowList& windows = ProjucerApplication::getApp().mainWindowList;

    for (int i = windows.windows.size(); --i >= 0;)
    {
        if (Project* p = windows.windows.getUnchecked (i)->getProject())
        {
            const File f (EnabledModuleList::findDefaultModulesFolder (*p));

            if (isJuceModulesFolder (f) || (f.isDirectory() && ! mustContainJuceCoreModule))
                return f;
        }
    }

    if (mustContainJuceCoreModule)
        return findDefaultModulesFolder (false);

    File f (File::getSpecialLocation (File::currentApplicationFile));

    for (;;)
    {
        File parent (f.getParentDirectory());

        if (parent == f || ! parent.isDirectory())
            break;

        if (isJuceFolder (parent))
            return parent.getChildFile ("modules");

        f = parent;
    }

    return File();
}
