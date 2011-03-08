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

#ifndef __JUCER_APPLICATION_H_6595C2A8__
#define __JUCER_APPLICATION_H_6595C2A8__

#include "../jucer_Headers.h"
#include "jucer_MainWindow.h"
#include "jucer_JuceUpdater.h"
#include "../Project/jucer_NewProjectWizard.h"


//==============================================================================
class JucerApplication   : public JUCEApplication
{
public:
    //==============================================================================
    JucerApplication()  {}
    ~JucerApplication() {}

    //==============================================================================
    void initialise (const String& commandLine)
    {
        /* Running a command-line of the form "Jucer --resave foobar.jucer" will try to load that
           jucer file and re-export all of its projects.
        */
        if (commandLine.startsWithIgnoreCase ("-resave ") || commandLine.startsWithIgnoreCase ("--resave "))
        {
            Project::resaveJucerFile (File::getCurrentWorkingDirectory()
                                        .getChildFile (commandLine.fromFirstOccurrenceOf (" ", false, false).unquoted()));
            quit();
            return;
        }

        commandManager = new ApplicationCommandManager();
        commandManager->registerAllCommandsForTarget (this);

        menuModel = new MainMenuModel();

        MainWindow* main = createNewMainWindow (false);
        doExtraInitialisation();

        ImageCache::setCacheTimeout (30 * 1000);

        if (commandLine.trim().isNotEmpty() && ! commandLine.trim().startsWithChar ('-'))
        {
            anotherInstanceStarted (commandLine);
        }
        else
        {
            Array<File> projects (StoredSettings::getInstance()->getLastProjects());

            for (int i = 0; i < projects.size(); ++ i)
                openFile (projects.getReference(i));
        }

      #if JUCE_MAC
        MenuBarModel::setMacMainMenu (menuModel);
      #endif

        main->setVisible (true);
    }

    void shutdown()
    {
      #if JUCE_MAC
        MenuBarModel::setMacMainMenu (0);
      #endif
        menuModel = 0;

        StoredSettings::deleteInstance();
        mainWindows.clear();

        OpenDocumentManager::deleteInstance();
        deleteAndZero (commandManager);
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        while (mainWindows.size() > 0)
        {
            if (! mainWindows[0]->closeCurrentProject())
                return;

            mainWindows.remove (0);
        }

        quit();
    }

    void closeWindow (MainWindow* w)
    {
        jassert (mainWindows.contains (w));
        mainWindows.removeObject (w);

      #if ! JUCE_MAC
        if (mainWindows.size() == 0)
            systemRequestedQuit();
      #endif

        updateRecentProjectList();
    }

    //==============================================================================
    const String getApplicationName()
    {
        return "The Jucer V" + getApplicationVersion();
    }

    const String getApplicationVersion()
    {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
      #ifndef JUCE_LINUX
        return false;
      #else
        return true; //xxx should be false but doesn't work on linux..
      #endif
    }

    void anotherInstanceStarted (const String& commandLine)
    {
        openFile (commandLine.unquoted());
    }

    virtual void doExtraInitialisation() {}

    //==============================================================================
    class MainMenuModel  : public MenuBarModel
    {
    public:
        MainMenuModel()
        {
            setApplicationCommandManagerToWatch (commandManager);
        }

        const StringArray getMenuBarNames()
        {
            const char* const names[] = { "File", "Edit", "View", "Window", "Update", 0 };
            return StringArray ((const char**) names);
        }

        const PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName)
        {
            PopupMenu menu;

            if (topLevelMenuIndex == 0)    // "File" menu
            {
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
                menu.addCommandItem (commandManager, CommandIDs::openInIDE);
                menu.addCommandItem (commandManager, CommandIDs::saveAndOpenInIDE);

                #if ! JUCE_MAC
                  menu.addSeparator();
                  menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
                #endif
            }
            else if (topLevelMenuIndex == 1)    // "Edit" menu
            {
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
            else if (topLevelMenuIndex == 2)    // "View" menu
            {
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

                menu.addSeparator();
                menu.addCommandItem (commandManager, CommandIDs::useTabbedWindows);
            }
            else if (topLevelMenuIndex == 3)   // "Window" menu
            {
                menu.addCommandItem (commandManager, CommandIDs::closeWindow);
                menu.addSeparator();

                const int numDocs = jmin (50, OpenDocumentManager::getInstance()->getNumOpenDocuments());

                for (int i = 0; i < numDocs; ++i)
                {
                    OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument(i);

                    menu.addItem (300 + i, doc->getName());
                }

                menu.addSeparator();
                menu.addCommandItem (commandManager, CommandIDs::closeAllDocuments);
            }
            else if (topLevelMenuIndex == 4)  // "Juce" menu
            {
                menu.addCommandItem (commandManager, CommandIDs::showJuceVersion);
            }

            return menu;
        }

        void menuItemSelected (int menuItemID, int topLevelMenuIndex)
        {
            if (menuItemID >= 100 && menuItemID < 200)
            {
                // open a file from the "recent files" menu
                const File file (StoredSettings::getInstance()->recentFiles.getFile (menuItemID - 100));

                getApp()->openFile (file);
            }
            else if (menuItemID >= 300 && menuItemID < 400)
            {
                OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument (menuItemID - 300);
                getApp()->getOrCreateFrontmostWindow (true)->getProjectContentComponent()->showDocument (doc);
            }
        }

    private:
        JucerApplication* getApp() const
        {
            return static_cast<JucerApplication*> (JUCEApplication::getInstance());
        }
    };

    //==============================================================================
    void getAllCommands (Array <CommandID>& commands)
    {
        JUCEApplication::getAllCommands (commands);

        const CommandID ids[] = { CommandIDs::newProject,
                                  CommandIDs::open,
                                  CommandIDs::showPrefs,
                                  CommandIDs::closeAllDocuments,
                                  CommandIDs::saveAll,
                                  CommandIDs::showJuceVersion };

        commands.addArray (ids, numElementsInArray (ids));
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
    {
        switch (commandID)
        {
        case CommandIDs::newProject:
            result.setInfo ("New Project...", "Creates a new Jucer project", CommandCategories::general, 0);
            result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::open:
            result.setInfo ("Open...", "Opens a Jucer project", CommandCategories::general, 0);
            result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::showPrefs:
            result.setInfo ("Preferences...", "Shows the preferences panel.", CommandCategories::general, 0);
            result.defaultKeypresses.add (KeyPress (',', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::closeAllDocuments:
            result.setInfo ("Close All Documents", "Closes all open documents", CommandCategories::general, 0);
            result.setActive (OpenDocumentManager::getInstance()->getNumOpenDocuments() > 0);
            break;

        case CommandIDs::saveAll:
            result.setInfo ("Save All", "Saves all open documents", CommandCategories::general, 0);
            result.setActive (OpenDocumentManager::getInstance()->anyFilesNeedSaving());
            break;

        case CommandIDs::showJuceVersion:
            result.setInfo ("Download the latest JUCE version", "Checks online for any Juce updates", CommandCategories::general, 0);
            break;

        default:
            JUCEApplication::getCommandInfo (commandID, result);
            break;
        }
    }

    bool perform (const InvocationInfo& info)
    {
        switch (info.commandID)
        {
            case CommandIDs::newProject:        createNewProject(); break;
            case CommandIDs::open:              askUserToOpenFile(); break;
            case CommandIDs::showPrefs:         showPrefsPanel(); break;
            case CommandIDs::saveAll:           OpenDocumentManager::getInstance()->saveAll(); break;
            case CommandIDs::closeAllDocuments: closeAllDocuments (true); break;
            case CommandIDs::showJuceVersion:   JuceUpdater::show (mainWindows[0]); break;
            default:                            return JUCEApplication::perform (info);
        }

        return true;
    }

    //==============================================================================
    void showPrefsPanel()
    {
        jassertfalse;
    }

    void createNewProject()
    {
        MainWindow* mw = createNewMainWindow (false);
        ScopedPointer <Project> newProj (NewProjectWizard::runNewProjectWizard (mw));

        if (newProj != 0)
        {
            mw->setProject (newProj.release());
            mw->setVisible (true);
        }
        else
        {
            closeWindow (mw);
        }
    }

    void askUserToOpenFile()
    {
        FileChooser fc ("Open File");

        if (fc.browseForFileToOpen())
            openFile (fc.getResult());
    }

    bool openFile (const File& file)
    {
        for (int j = mainWindows.size(); --j >= 0;)
        {
            if (mainWindows.getUnchecked(j)->getProject() != 0
                 && mainWindows.getUnchecked(j)->getProject()->getFile() == file)
            {
                mainWindows.getUnchecked(j)->toFront (true);
                return true;
            }
        }

        if (file.hasFileExtension (Project::projectFileExtension))
        {
            ScopedPointer <Project> newDoc (new Project (file));

            if (file == File::nonexistent ? newDoc->loadFromUserSpecifiedFile (true)
                                          : newDoc->loadFrom (file, true))
            {
                MainWindow* w = getOrCreateEmptyWindow (false);
                w->setProject (newDoc.release());
                w->restoreWindowPosition();
                w->setVisible (true);
                return true;
            }
        }
        else if (file.exists())
        {
            return getOrCreateFrontmostWindow (true)->openFile (file);
        }

        return false;
    }

    bool closeAllDocuments (bool askUserToSave)
    {
        for (int i = OpenDocumentManager::getInstance()->getNumOpenDocuments(); --i >= 0;)
        {
            OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getOpenDocument (i);

            for (int j = mainWindows.size(); --j >= 0;)
                mainWindows.getUnchecked(j)->getProjectContentComponent()->hideDocument (doc);

            if (! OpenDocumentManager::getInstance()->closeDocument (i, askUserToSave))
                return false;
        }

        return true;
    }

    void updateRecentProjectList()
    {
        Array<File> projects;

        for (int i = 0; i < mainWindows.size(); ++i)
        {
            MainWindow* mw = mainWindows[i];

            if (mw != 0 && mw->getProject() != 0)
                projects.add (mw->getProject()->getFile());
        }

        StoredSettings::getInstance()->setLastProjects (projects);
    }

    ScopedPointer<MainMenuModel> menuModel;

private:
    OwnedArray <MainWindow> mainWindows;

    MainWindow* createNewMainWindow (bool makeVisible)
    {
        MainWindow* mw = new MainWindow();

        for (int i = mainWindows.size(); --i >= 0;)
            if (mw->getBounds() == mainWindows.getUnchecked(i)->getBounds())
                mw->setBounds (mw->getBounds().translated (20, 20));

        mainWindows.add (mw);

        if (makeVisible)
            mw->setVisible (true);

        mw->restoreWindowPosition();
        return mw;
    }

    MainWindow* getOrCreateFrontmostWindow (bool makeVisible)
    {
        if (mainWindows.size() == 0)
            return createNewMainWindow (makeVisible);

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            MainWindow* mw = dynamic_cast <MainWindow*> (Desktop::getInstance().getComponent (i));
            if (mainWindows.contains (mw))
                return mw;
        }

        return mainWindows.getLast();
    }

    MainWindow* getOrCreateEmptyWindow (bool makeVisible)
    {
        if (mainWindows.size() == 0)
            return createNewMainWindow (makeVisible);

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            MainWindow* mw = dynamic_cast <MainWindow*> (Desktop::getInstance().getComponent (i));
            if (mainWindows.contains (mw) && mw->getProject() == 0)
                return mw;
        }

        return createNewMainWindow (makeVisible);
    }
};


#endif  // __JUCER_APPLICATION_H_6595C2A8__
