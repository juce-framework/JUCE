/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#include "jucer_ComponentLayoutEditor.h"
#include "jucer_JucerDocumentHolder.h"
#include "jucer_PrefsPanel.h"
#include "jucer_TestComponent.h"
#include "../model/jucer_ObjectTypes.h"
#include "../properties/jucer_FontPropertyComponent.h"


static OldSchoolLookAndFeel* oldLook = 0;
static const int snapSizes[] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32 };

//==============================================================================
class MultiDocHolder  : public MultiDocumentPanel
{
public:
    MultiDocHolder()
    {
        setBackgroundColour (Colour (0xffe6f0ff));
    }

    ~MultiDocHolder()
    {
    }

    bool tryToCloseDocument (Component* component)
    {
        JucerDocumentHolder* const holder = dynamic_cast <JucerDocumentHolder*> (component);

        return holder == 0
            || holder->getDocument() == 0
            || holder->getDocument()->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk;
    }
};

//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (T("The Jucer"),
                      Colours::azure,
                      DocumentWindow::allButtons)
{
    if (oldLook == 0)
        oldLook = new OldSchoolLookAndFeel();

    setContentComponent (multiDocHolder = new MultiDocHolder());

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
                                    .getValue (T("lastMainWindowPos")));

    // Register all the app commands..
    {
        commandManager->registerAllCommandsForTarget (JUCEApplication::getInstance());
        commandManager->registerAllCommandsForTarget (this);

        // use a temporary one of these to harvest its commands..
        JucerDocumentHolder tempDesignHolder (ObjectTypes::createNewDocument (0));
        commandManager->registerAllCommandsForTarget (&tempDesignHolder);
    }

    commandManager->getKeyMappings()->resetToDefaultMappings();

    XmlElement* const keys = StoredSettings::getInstance()->getProps().getXmlValue (T("keyMappings"));

    if (keys != 0)
    {
        commandManager->getKeyMappings()->restoreFromXml (*keys);
        delete keys;
    }

    addKeyListener (commandManager->getKeyMappings());

    // don't want the window to take focus when the title-bar is clicked..
    setWantsKeyboardFocus (false);

#ifndef JUCE_DEBUG
    // scan for fonts before the app gets started rather than glitching later
    FontPropertyComponent::preloadAllFonts();
#endif
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
        .setValue (T("lastMainWindowPos"), getWindowStateAsString());

    setContentComponent (0);

    LookAndFeel::setDefaultLookAndFeel (0);
    deleteAndZero (oldLook);
}

void MainWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}

JucerDocument* MainWindow::getActiveDocument() const throw()
{
    JucerDocumentHolder* holder = dynamic_cast <JucerDocumentHolder*> (multiDocHolder->getActiveDocument());

    if (holder == 0)
        return 0;

    return holder->getDocument();
}

bool MainWindow::closeAllDocuments()
{
    return multiDocHolder->closeAllDocuments (true);
}

bool MainWindow::closeDocument (JucerDocumentHolder* designHolder)
{
    return multiDocHolder->closeDocument (designHolder, true);
}

void MainWindow::openDocument (JucerDocument* const newDoc)
{
    const File f (newDoc->getFile());

    // check it's not already open..
    if (f != File::nonexistent)
    {
        for (int i = multiDocHolder->getNumDocuments(); --i >= 0;)
        {
            JucerDocumentHolder* holder = dynamic_cast <JucerDocumentHolder*> (multiDocHolder->getDocument (i));

            if (holder != 0 && holder->getDocument()->getFile() == f)
            {
                multiDocHolder->setActiveDocument (holder);
                delete newDoc;
                return;
            }
        }
    }

    multiDocHolder->addDocument (new JucerDocumentHolder (newDoc), Colour (0xffc4cdcd), true);
}

bool MainWindow::openFile (const File& file)
{
    JucerDocument* newDoc = ObjectTypes::loadDocumentFromFile (file, true);

    if (newDoc != 0)
        openDocument (newDoc);

    return newDoc != 0;
}

bool MainWindow::isInterestedInFileDrag (const StringArray& filenames)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (f.hasFileExtension (T(".cpp")))
            return true;
    }

    return false;
}

void MainWindow::filesDropped (const StringArray& filenames, int mouseX, int mouseY)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (f.hasFileExtension (T(".cpp")) && openFile (f))
            break;
    }
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (isActiveWindow())
        TestComponent::reloadAll();
}

//==============================================================================
const StringArray MainWindow::getMenuBarNames()
{
    const tchar* const names[] = { T("File"), T("Edit"), T("View"), 0 };

    return StringArray ((const tchar**) names);
}

const PopupMenu MainWindow::getMenuForIndex (int topLevelMenuIndex,
                                             const String& menuName)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        // "File" menu

        for (int i = 0; i < ObjectTypes::numDocumentTypes; ++i)
            menu.addCommandItem (commandManager, CommandIDs::newDocumentBase + i);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::open);

        PopupMenu recentFiles;
        StoredSettings::getInstance()->recentFiles.createPopupMenuItems (recentFiles, 100, true, true);
        menu.addSubMenu (T("Open recent file"), recentFiles);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::close);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::save);
        menu.addCommandItem (commandManager, CommandIDs::saveAs);
        menu.addSeparator();

        menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
    }
    else if (topLevelMenuIndex == 1)
    {
        // "Edit" menu

        menu.addCommandItem (commandManager, CommandIDs::undo);
        menu.addCommandItem (commandManager, CommandIDs::redo);
        menu.addSeparator();

        menu.addCommandItem (commandManager, CommandIDs::editCompLayout);
        menu.addCommandItem (commandManager, CommandIDs::editCompGraphics);
        menu.addSeparator();

        PopupMenu newComps;
        int i;
        for (i = 0; i < ObjectTypes::numComponentTypes; ++i)
            newComps.addCommandItem (commandManager, CommandIDs::newComponentBase + i);

        menu.addSubMenu (T("Add new component"), newComps);

        PopupMenu newElements;
        for (i = 0; i < ObjectTypes::numElementTypes; ++i)
            newElements.addCommandItem (commandManager, CommandIDs::newElementBase + i);

        menu.addSubMenu (T("Add new graphic element"), newElements);

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

        menu.addCommandItem (commandManager, CommandIDs::test);

        PopupMenu lookAndFeels;
        lookAndFeels.addItem (201, T("Default"), true, (typeid (LookAndFeel) == typeid (LookAndFeel::getDefaultLookAndFeel())) != 0);
        lookAndFeels.addItem (200, T("Old School"), true, (typeid (OldSchoolLookAndFeel) == typeid (LookAndFeel::getDefaultLookAndFeel())) != 0);

        menu.addSeparator();
        menu.addSubMenu (T("Look and Feel"), lookAndFeels);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::showGrid);
        menu.addCommandItem (commandManager, CommandIDs::enableSnapToGrid);

        const int currentSnapSize = getActiveDocument() != 0 ? getActiveDocument()->getSnappingGridSize() : 0;

        PopupMenu m;
        for (int i = 0; i < numElementsInArray (snapSizes); ++i)
            m.addItem (300 + i, String (snapSizes[i]) + T(" pixels"), true, snapSizes[i] == currentSnapSize);

        menu.addSubMenu (T("Grid size"), m, getActiveDocument() != 0);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::zoomIn);
        menu.addCommandItem (commandManager, CommandIDs::zoomOut);
        menu.addCommandItem (commandManager, CommandIDs::zoomNormal);

        menu.addSeparator();
        PopupMenu overlays;
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay0);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay33);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay66);
        overlays.addCommandItem (commandManager, CommandIDs::compOverlay100);
        menu.addSubMenu (T("Component Overlay"), overlays,
                         getActiveDocument() != 0 && getActiveDocument()->getComponentLayout() != 0);

        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::useTabbedWindows);
        menu.addSeparator();
        menu.addCommandItem (commandManager, CommandIDs::showPrefs);
    }

    return menu;
}

void MainWindow::menuItemSelected (int menuItemID,
                                   int topLevelMenuIndex)
{
    if (menuItemID >= 100 && menuItemID < 200)
    {
        // open a file from the "recent files" menu
        JucerDocument* const newDoc
            = ObjectTypes::loadDocumentFromFile (StoredSettings::getInstance()->recentFiles.getFile (menuItemID - 100),
                                                 true);

        if (newDoc != 0)
            openDocument (newDoc);
    }
    else if (menuItemID == 200)
    {
        LookAndFeel::setDefaultLookAndFeel (oldLook);
    }
    else if (menuItemID == 201)
    {
        LookAndFeel::setDefaultLookAndFeel (0);
    }
    else if (menuItemID >= 300 && menuItemID < 400)
    {
        if (getActiveDocument() != 0)
        {
            getActiveDocument()->setSnappingGrid (snapSizes [menuItemID - 300],
                                                  getActiveDocument()->isSnapActive (false),
                                                  getActiveDocument()->isSnapShown());
        }
    }
}

//==============================================================================
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return 0;
}

void MainWindow::getAllCommands (Array <CommandID>& commands)
{
    for (int i = 0; i < ObjectTypes::numDocumentTypes; ++i)
        commands.add (CommandIDs::newDocumentBase + i);

    const CommandID ids[] = { CommandIDs::open,
                              CommandIDs::showPrefs,
                              CommandIDs::useTabbedWindows };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    if (commandID >= CommandIDs::newDocumentBase
         && commandID < CommandIDs::newDocumentBase + ObjectTypes::numDocumentTypes)
    {
        const int index = commandID - CommandIDs::newDocumentBase;

        result.setInfo (T("New ") + String (ObjectTypes::documentTypeNames [index]),
                        T("Creates a new ") + String (ObjectTypes::documentTypeNames[index]),
                        CommandCategories::general, 0);

        return;
    }

    const int cmd = ModifierKeys::commandModifier;

    switch (commandID)
    {
    case CommandIDs::open:
        result.setInfo (T("Open..."),
                        T("Opens a Jucer .cpp component file for editing."),
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (T('o'), cmd, 0));
        break;

    case CommandIDs::showPrefs:
        result.setInfo (T("Preferences..."),
                        T("Shows the preferences panel."),
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (T(','), cmd, 0));
        break;

    case CommandIDs::useTabbedWindows:
        result.setInfo (T("Use tabs to show windows"),
                        T("Flips between a tabbed component and separate windows"),
                        CommandCategories::general, 0);
        result.setTicked (multiDocHolder->getLayoutMode() == MultiDocumentPanel::MaximisedWindowsWithTabs);
        break;

    default:
        break;
    }
}

bool MainWindow::isCommandActive (const CommandID commandID)
{
    return true;
}

bool MainWindow::perform (const InvocationInfo& info)
{
    if (info.commandID >= CommandIDs::newDocumentBase
         && info.commandID < CommandIDs::newDocumentBase + ObjectTypes::numDocumentTypes)
    {
        const int index = info.commandID - CommandIDs::newDocumentBase;
        openDocument (ObjectTypes::createNewDocument (index));

        return true;
    }

    switch (info.commandID)
    {
    case CommandIDs::open:
        openFile (File::nonexistent);
        break;

    case CommandIDs::showPrefs:
        PrefsPanel::show();
        break;

    case CommandIDs::useTabbedWindows:
        if (multiDocHolder->getLayoutMode() == MultiDocumentPanel::MaximisedWindowsWithTabs)
            multiDocHolder->setLayoutMode (MultiDocumentPanel::FloatingWindows);
        else
            multiDocHolder->setLayoutMode (MultiDocumentPanel::MaximisedWindowsWithTabs);

        break;

    default:
        return false;
    }

    return true;
}
