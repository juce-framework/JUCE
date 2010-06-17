/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_MainWindow.h"
#include "jucer_OpenDocumentManager.h"
#include "Code Editor/jucer_SourceCodeEditor.h"
#include "../model/Project/jucer_ProjectWizard.h"


//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colour::greyLevel (0.6f),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentComponent (new ProjectContentComponent());

    setApplicationCommandManagerToWatch (commandManager);

#if JUCE_MAC
    setMacMainMenu (this);
#else
    setMenuBar (this);
#endif

    setResizable (true, false);

    centreWithSize (700, 600);

    // restore the last size and position from our settings file..
    restoreWindowStateFromString (StoredSettings::getInstance()->getProps()
                                    .getValue ("lastMainWindowPos"));

    // Register all the app commands..
    {
        commandManager->registerAllCommandsForTarget (this);

        // use a temporary one of these to harvest its commands..
        ProjectContentComponent pcc;
        commandManager->registerAllCommandsForTarget (&pcc);

        DocumentEditorComponent dec (0);
        commandManager->registerAllCommandsForTarget (&dec);
    }

    commandManager->getKeyMappings()->resetToDefaultMappings();

    ScopedPointer <XmlElement> keys (StoredSettings::getInstance()->getProps().getXmlValue ("keyMappings"));

    if (keys != 0)
        commandManager->getKeyMappings()->restoreFromXml (*keys);

    addKeyListener (commandManager->getKeyMappings());

    // don't want the window to take focus when the title-bar is clicked..
    setWantsKeyboardFocus (false);

    //getPeer()->setCurrentRenderingEngine (0);

    setVisible (true);
}

MainWindow::~MainWindow()
{
#if JUCE_MAC
    setMacMainMenu (0);
#else
    setMenuBar (0);
#endif

    removeKeyListener (commandManager->getKeyMappings());

    // save the current size and position to our settings file..
    StoredSettings::getInstance()->getProps()
        .setValue ("lastMainWindowPos", getWindowStateAsString());

    setContentComponent (0);
    currentProject = 0;
}

ProjectContentComponent* MainWindow::getProjectContentComponent() const
{
    return dynamic_cast <ProjectContentComponent*> (getContentComponent());
}

void MainWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}

bool MainWindow::closeProject (Project* project)
{
    jassert (project == currentProject && project != 0);

    if (project == 0)
        return true;

    if (! OpenDocumentManager::getInstance()->closeAllDocumentsUsingProject (*project, true))
        return false;

    FileBasedDocument::SaveResult r = project->saveIfNeededAndUserAgrees();

    if (r == FileBasedDocument::savedOk)
    {
        setProject (0);
        return true;
    }

    return false;
}

bool MainWindow::closeCurrentProject()
{
    return currentProject == 0 || closeProject (currentProject);
}

bool MainWindow::closeAllDocuments (bool askUserToSave)
{
    for (int i = OpenDocumentManager::getInstance()->getNumOpenDocuments(); --i >= 0;)
    {
        OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument (i);
        getProjectContentComponent()->hideDocument (doc);

        if (! OpenDocumentManager::getInstance()->closeDocument (i, askUserToSave))
            return false;
    }

    return true;
}

void MainWindow::setProject (Project* newProject)
{
    if (newProject != 0)
        StoredSettings::getInstance()->setLastProject (newProject->getFile());

    getProjectContentComponent()->setProject (newProject);
    currentProject = newProject;
    commandManager->commandStatusChanged();
}

void MainWindow::reloadLastProject()
{
    openFile (StoredSettings::getInstance()->getLastProject());
}

void MainWindow::askUserToOpenFile()
{
    FileChooser fc ("Open File");

    if (fc.browseForFileToOpen())
        openFile (fc.getResult());
}

bool MainWindow::canOpenFile (const File& file) const
{
    return file.hasFileExtension (Project::projectFileExtension)
             || OpenDocumentManager::getInstance()->canOpenFile (file);
}

bool MainWindow::openFile (const File& file)
{
    if (file.hasFileExtension (Project::projectFileExtension))
    {
        ScopedPointer <Project> newDoc (new Project (file));

        if (file == File::nonexistent ? newDoc->loadFromUserSpecifiedFile (true)
                                      : newDoc->loadFrom (file, true))
        {
            if (closeCurrentProject())
            {
                setProject (newDoc.release());
                return true;
            }
        }
    }
    else if (file.exists())
    {
        return getProjectContentComponent()->showEditorForFile (file);
    }

    return false;
}

void MainWindow::createNewProject()
{
    ScopedPointer <Project> newProj (ProjectWizard::runNewProjectWizard (this));

    if (newProj != 0 && closeCurrentProject())
        setProject (newProj.release());
}

bool MainWindow::isInterestedInFileDrag (const StringArray& filenames)
{
    for (int i = filenames.size(); --i >= 0;)
        if (canOpenFile (filenames[i]))
            return true;

    return false;
}

void MainWindow::filesDropped (const StringArray& filenames, int mouseX, int mouseY)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (canOpenFile (f) && openFile (f))
            break;
    }
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (getProjectContentComponent() != 0)
        getProjectContentComponent()->updateMissingFileStatuses();

    OpenDocumentManager::getInstance()->reloadModifiedFiles();
}

void MainWindow::updateTitle (const String& documentName)
{
    String name (JUCEApplication::getInstance()->getApplicationName());
    if (documentName.isNotEmpty())
        name = documentName + " - " + name;

    setName (name);
}

//==============================================================================
const StringArray MainWindow::getMenuBarNames()
{
    const char* const names[] = { "File", "Edit", "View", "Window", 0 };
    return StringArray ((const char**) names);
}

const PopupMenu MainWindow::getMenuForIndex (int topLevelMenuIndex,
                                             const String& menuName)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        // "File" menu

        menu.addCommandItem (commandManager, CommandIDs::newProject);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::open);

        PopupMenu recentFiles;
        StoredSettings::getInstance()->recentFiles.createPopupMenuItems (recentFiles, 100, true, true);
        menu.addSubMenu ("Open recent file", recentFiles);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::closeDocument);
        menu.addCommandItem (commandManager, CommandIDs::saveDocument);
        menu.addCommandItem (commandManager, CommandIDs::saveDocumentAs);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::closeProject);
        menu.addCommandItem (commandManager, CommandIDs::saveProject);
        menu.addCommandItem (commandManager, CommandIDs::saveProjectAs);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::openProjectInIDE);

#if ! JUCE_MAC
        menu.addSeparator();
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
#endif
    }
    else if (topLevelMenuIndex == 1)
    {
        // "Edit" menu

        menu.addCommandItem (commandManager, CommandIDs::undo);
        menu.addCommandItem (commandManager, CommandIDs::redo);
        menu.addSeparator();
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::selectAll);
        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::deselectAll);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::toFront);
        menu.addCommandItem (commandManager, CommandIDs::toBack);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::group);
        menu.addCommandItem (commandManager, CommandIDs::ungroup);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::bringBackLostItems);
    }
    else if (topLevelMenuIndex == 2)
    {
        // "View" menu

        menu.addCommandItem (commandManager, CommandIDs::showProjectSettings);
        menu.addSeparator();

        menu.addCommandItem (commandManager, CommandIDs::test);
        menu.addSeparator();

        menu.addCommandItem (commandManager, CommandIDs::showGrid);
        menu.addCommandItem (commandManager, CommandIDs::enableSnapToGrid);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::zoomIn);
        menu.addCommandItem (commandManager, CommandIDs::zoomOut);
        menu.addCommandItem (commandManager, CommandIDs::zoomNormal);

/*        menu.addSeparator();
        PopupMenu overlays;
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay0);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay33);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay66);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay100);
        menu.addSubMenu ("Component Overlay", overlays,
                         getActiveDocument() != 0 && getActiveDocument()->getComponentLayout() != 0);*/

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::useTabbedWindows);
        //menu.addSeparator();
        //menu.addCommandItem (commandManager, CommandIDs::showPrefs);
    }
    else if (topLevelMenuIndex == 3)
    {
        // "Window" menu

        const int numDocs = jmin (50, OpenDocumentManager::getInstance()->getNumOpenDocuments());

        for (int i = 0; i < numDocs; ++i)
        {
            OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument(i);

            menu.addItem (300 + i, doc->getName());
        }

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::closeAllDocuments);
    }

    return menu;
}

void MainWindow::menuItemSelected (int menuItemID,
                                   int topLevelMenuIndex)
{
    if (menuItemID >= 100 && menuItemID < 200)
    {
        // open a file from the "recent files" menu
        const File file (StoredSettings::getInstance()->recentFiles.getFile (menuItemID - 100));

        openFile (file);
    }
    else if (menuItemID == 201)
    {
        LookAndFeel::setDefaultLookAndFeel (0);
    }
    else if (menuItemID >= 300 && menuItemID < 400)
    {
        OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument (menuItemID - 300);
        getProjectContentComponent()->showDocument (doc);
    }
}

//==============================================================================
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return 0;
}

void MainWindow::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::newProject,
                              CommandIDs::open,
                              CommandIDs::showPrefs,
                              CommandIDs::closeAllDocuments,
                              CommandIDs::saveAll };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::newProject:
        result.setInfo ("New Project...",
                        "Creates a new Jucer project",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::open:
        result.setInfo ("Open...",
                        "Opens a Jucer project",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showPrefs:
        result.setInfo ("Preferences...",
                        "Shows the preferences panel.",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (',', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::closeAllDocuments:
        result.setInfo ("Close All Documents",
                        "Closes all open documents",
                        CommandCategories::general, 0);
        result.setActive (OpenDocumentManager::getInstance()->getNumOpenDocuments() > 0);
        break;

    case CommandIDs::saveAll:
        result.setInfo ("Save All",
                        "Saves all open documents",
                        CommandCategories::general, 0);
        result.setActive (OpenDocumentManager::getInstance()->anyFilesNeedSaving());
        break;

    default:
        break;
    }
}

bool MainWindow::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::newProject:
        createNewProject();
        break;

    case CommandIDs::open:
        askUserToOpenFile();
        break;

    case CommandIDs::showPrefs:
   //     PrefsPanel::show();
        break;

    case CommandIDs::saveAll:
        OpenDocumentManager::getInstance()->saveAll();
        break;

    case CommandIDs::closeAllDocuments:
        closeAllDocuments (true);
        break;

    default:
        return false;
    }

    return true;
}
