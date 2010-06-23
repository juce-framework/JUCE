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
#include "jucer_Application.h"
#include "jucer_MainWindow.h"
#include "jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Project/jucer_NewProjectWizard.h"

ApplicationCommandManager* commandManager = 0;


//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colour::greyLevel (0.6f),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentComponent (new ProjectContentComponent());

#if ! JUCE_MAC
    JucerApplication* app = static_cast<JucerApplication*> (JUCEApplication::getInstance());
    setMenuBar (app->menuModel);
#endif

    setResizable (true, false);

    centreWithSize (700, 600);

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
    getLookAndFeel().setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
}

MainWindow::~MainWindow()
{
#if ! JUCE_MAC
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
    if (! closeCurrentProject())
        return;

    JucerApplication* jucer = static_cast<JucerApplication*> (JUCEApplication::getInstance());
    jucer->closeWindow (this);
}

bool MainWindow::closeProject (Project* project)
{
    jassert (project == currentProject && project != 0);

    if (project == 0)
        return true;

    StoredSettings::getInstance()->getProps()
        .setValue (getProjectWindowPosName(), getWindowStateAsString());

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

void MainWindow::setProject (Project* newProject)
{
    getProjectContentComponent()->setProject (newProject);
    currentProject = newProject;
    commandManager->commandStatusChanged();

    // (mustn't do this when the project is 0, because that'll happen on shutdown,
    // which will erase the list of recent projects)
    if (newProject != 0)
        static_cast<JucerApplication*> (JUCEApplication::getInstance())->updateRecentProjectList();
}

void MainWindow::restoreWindowPosition()
{
    String windowState;

    if (currentProject != 0)
        windowState = StoredSettings::getInstance()->getProps().getValue (getProjectWindowPosName());

    if (windowState.isEmpty())
        windowState = StoredSettings::getInstance()->getProps().getValue ("lastMainWindowPos");

    restoreWindowStateFromString (windowState);
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
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return 0;
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
