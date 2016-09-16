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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainHostWindow.h"
#include "InternalFilters.h"


//==============================================================================
class MainHostWindow::PluginListWindow  : public DocumentWindow
{
public:
    PluginListWindow (MainHostWindow& owner_, AudioPluginFormatManager& pluginFormatManager)
        : DocumentWindow ("Available Plugins", Colours::white,
                          DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          owner (owner_)
    {
        const File deadMansPedalFile (getAppProperties().getUserSettings()
                                        ->getFile().getSiblingFile ("RecentlyCrashedPluginsList"));

        setContentOwned (new PluginListComponent (pluginFormatManager,
                                                  owner.knownPluginList,
                                                  deadMansPedalFile,
                                                  getAppProperties().getUserSettings(), true), true);

        setResizable (true, false);
        setResizeLimits (300, 400, 800, 1500);
        setTopLeftPosition (60, 60);

        restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("listWindowPos"));
        setVisible (true);
    }

    ~PluginListWindow()
    {
        getAppProperties().getUserSettings()->setValue ("listWindowPos", getWindowStateAsString());

        clearContentComponent();
    }

    void closeButtonPressed()
    {
        owner.pluginListWindow = nullptr;
    }

private:
    MainHostWindow& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListWindow)
};

//==============================================================================
MainHostWindow::MainHostWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(), Colours::lightgrey,
                      DocumentWindow::allButtons)
{
    formatManager.addDefaultFormats();
    formatManager.addFormat (new InternalPluginFormat());

    ScopedPointer<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                   ->getXmlValue ("audioDeviceState"));

    deviceManager.initialise (256, 256, savedAudioState, true);

    setResizable (true, false);
    setResizeLimits (500, 400, 10000, 10000);
    centreWithSize (800, 600);

    setContentOwned (new GraphDocumentComponent (formatManager, &deviceManager), false);

    restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("mainWindowPos"));

    setVisible (true);

    InternalPluginFormat internalFormat;
    internalFormat.getAllTypes (internalTypes);

    ScopedPointer<XmlElement> savedPluginList (getAppProperties().getUserSettings()->getXmlValue ("pluginList"));

    if (savedPluginList != nullptr)
        knownPluginList.recreateFromXml (*savedPluginList);

    pluginSortMethod = (KnownPluginList::SortMethod) getAppProperties().getUserSettings()
                            ->getIntValue ("pluginSortMethod", KnownPluginList::sortByManufacturer);

    knownPluginList.addChangeListener (this);

    if (FilterGraph* filterGraph = getGraphEditor()->graph.get())
        filterGraph->addChangeListener (this);

    addKeyListener (getCommandManager().getKeyMappings());

    Process::setPriority (Process::HighPriority);

   #if JUCE_MAC
    setMacMainMenu (this);
   #else
    setMenuBar (this);
   #endif

    getCommandManager().setFirstCommandTarget (this);
}

MainHostWindow::~MainHostWindow()
{
    pluginListWindow = nullptr;
    knownPluginList.removeChangeListener (this);

    if (FilterGraph* filterGraph = getGraphEditor()->graph.get())
        filterGraph->removeChangeListener (this);

    getAppProperties().getUserSettings()->setValue ("mainWindowPos", getWindowStateAsString());
    clearContentComponent();

   #if JUCE_MAC
    setMacMainMenu (nullptr);
   #else
    setMenuBar (nullptr);
   #endif
}

void MainHostWindow::closeButtonPressed()
{
    tryToQuitApplication();
}

bool MainHostWindow::tryToQuitApplication()
{
    PluginWindow::closeAllCurrentlyOpenWindows();

    if (getGraphEditor() == nullptr
         || getGraphEditor()->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
    {
        // Some plug-ins do not want [NSApp stop] to be called
        // before the plug-ins are not deallocated.
        getGraphEditor()->releaseGraph();

        JUCEApplication::quit();
        return true;
    }

    return false;
}

void MainHostWindow::changeListenerCallback (ChangeBroadcaster* changed)
{
    if (changed == &knownPluginList)
    {
        menuItemsChanged();

        // save the plugin list every time it gets chnaged, so that if we're scanning
        // and it crashes, we've still saved the previous ones
        ScopedPointer<XmlElement> savedPluginList (knownPluginList.createXml());

        if (savedPluginList != nullptr)
        {
            getAppProperties().getUserSettings()->setValue ("pluginList", savedPluginList);
            getAppProperties().saveIfNeeded();
        }
    }
    else if (changed == getGraphEditor()->graph)
    {
        String title = JUCEApplication::getInstance()->getApplicationName();

        File f = getGraphEditor()->graph->getFile();

        if (f.existsAsFile())
            title = f.getFileName() + " - " + title;

        setName (title);
    }
}

StringArray MainHostWindow::getMenuBarNames()
{
    const char* const names[] = { "File", "Plugins", "Options", "Windows", nullptr };

    return StringArray (names);
}

PopupMenu MainHostWindow::getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        // "File" menu
        menu.addCommandItem (&getCommandManager(), CommandIDs::newFile);
        menu.addCommandItem (&getCommandManager(), CommandIDs::open);

        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                            ->getValue ("recentFilterGraphFiles"));

        PopupMenu recentFilesMenu;
        recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);
        menu.addSubMenu ("Open recent file", recentFilesMenu);

        menu.addCommandItem (&getCommandManager(), CommandIDs::save);
        menu.addCommandItem (&getCommandManager(), CommandIDs::saveAs);
        menu.addSeparator();
        menu.addCommandItem (&getCommandManager(), StandardApplicationCommandIDs::quit);
    }
    else if (topLevelMenuIndex == 1)
    {
        // "Plugins" menu
        PopupMenu pluginsMenu;
        addPluginsToMenu (pluginsMenu);
        menu.addSubMenu ("Create plugin", pluginsMenu);
        menu.addSeparator();
        menu.addItem (250, "Delete all plugins");
    }
    else if (topLevelMenuIndex == 2)
    {
        // "Options" menu

        menu.addCommandItem (&getCommandManager(), CommandIDs::showPluginListEditor);

        PopupMenu sortTypeMenu;
        sortTypeMenu.addItem (200, "List plugins in default order",      true, pluginSortMethod == KnownPluginList::defaultOrder);
        sortTypeMenu.addItem (201, "List plugins in alphabetical order", true, pluginSortMethod == KnownPluginList::sortAlphabetically);
        sortTypeMenu.addItem (202, "List plugins by category",           true, pluginSortMethod == KnownPluginList::sortByCategory);
        sortTypeMenu.addItem (203, "List plugins by manufacturer",       true, pluginSortMethod == KnownPluginList::sortByManufacturer);
        sortTypeMenu.addItem (204, "List plugins based on the directory structure", true, pluginSortMethod == KnownPluginList::sortByFileSystemLocation);
        menu.addSubMenu ("Plugin menu type", sortTypeMenu);

        menu.addSeparator();
        menu.addCommandItem (&getCommandManager(), CommandIDs::showAudioSettings);
        menu.addCommandItem (&getCommandManager(), CommandIDs::toggleDoublePrecision);

        menu.addSeparator();
        menu.addCommandItem (&getCommandManager(), CommandIDs::aboutBox);
    }
    else if (topLevelMenuIndex == 3)
    {
        menu.addCommandItem (&getCommandManager(), CommandIDs::allWindowsForward);
    }

    return menu;
}

void MainHostWindow::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
    GraphDocumentComponent* const graphEditor = getGraphEditor();

    if (menuItemID == 250)
    {
        if (graphEditor != nullptr)
            if (FilterGraph* filterGraph = getGraphEditor()->graph.get())
                filterGraph->clear();
    }
    else if (menuItemID >= 100 && menuItemID < 200)
    {
        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                            ->getValue ("recentFilterGraphFiles"));

        if (graphEditor != nullptr
              && getGraphEditor()->graph != nullptr
              && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
            graphEditor->graph->loadFrom (recentFiles.getFile (menuItemID - 100), true);
    }
    else if (menuItemID >= 200 && menuItemID < 210)
    {
             if (menuItemID == 200)     pluginSortMethod = KnownPluginList::defaultOrder;
        else if (menuItemID == 201)     pluginSortMethod = KnownPluginList::sortAlphabetically;
        else if (menuItemID == 202)     pluginSortMethod = KnownPluginList::sortByCategory;
        else if (menuItemID == 203)     pluginSortMethod = KnownPluginList::sortByManufacturer;
        else if (menuItemID == 204)     pluginSortMethod = KnownPluginList::sortByFileSystemLocation;

        getAppProperties().getUserSettings()->setValue ("pluginSortMethod", (int) pluginSortMethod);

        menuItemsChanged();
    }
    else
    {
        createPlugin (getChosenType (menuItemID),
                      proportionOfWidth  (0.3f + Random::getSystemRandom().nextFloat() * 0.6f),
                      proportionOfHeight (0.3f + Random::getSystemRandom().nextFloat() * 0.6f));
    }
}

void MainHostWindow::menuBarActivated (bool isActivated)
{
    GraphDocumentComponent* const graphEditor = getGraphEditor();

    if (graphEditor != nullptr && isActivated)
        graphEditor->unfocusKeyboardComponent();
}

void MainHostWindow::createPlugin (const PluginDescription* desc, int x, int y)
{
    GraphDocumentComponent* const graphEditor = getGraphEditor();

    if (graphEditor != nullptr)
        graphEditor->createNewPlugin (desc, x, y);
}

void MainHostWindow::addPluginsToMenu (PopupMenu& m) const
{
    for (int i = 0; i < internalTypes.size(); ++i)
        m.addItem (i + 1, internalTypes.getUnchecked(i)->name);

    m.addSeparator();

    knownPluginList.addToMenu (m, pluginSortMethod);
}

const PluginDescription* MainHostWindow::getChosenType (const int menuID) const
{
    if (menuID >= 1 && menuID < 1 + internalTypes.size())
        return internalTypes [menuID - 1];

    return knownPluginList.getType (knownPluginList.getIndexChosenByMenu (menuID));
}

//==============================================================================
ApplicationCommandTarget* MainHostWindow::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void MainHostWindow::getAllCommands (Array <CommandID>& commands)
{
    // this returns the set of all commands that this target can perform..
    const CommandID ids[] = { CommandIDs::newFile,
                              CommandIDs::open,
                              CommandIDs::save,
                              CommandIDs::saveAs,
                              CommandIDs::showPluginListEditor,
                              CommandIDs::showAudioSettings,
                              CommandIDs::toggleDoublePrecision,
                              CommandIDs::aboutBox,
                              CommandIDs::allWindowsForward
                            };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainHostWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const String category ("General");

    switch (commandID)
    {
    case CommandIDs::newFile:
        result.setInfo ("New", "Creates a new filter graph file", category, 0);
        result.defaultKeypresses.add(KeyPress('n', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::open:
        result.setInfo ("Open...", "Opens a filter graph file", category, 0);
        result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::save:
        result.setInfo ("Save", "Saves the current graph to a file", category, 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::saveAs:
        result.setInfo ("Save As...",
                        "Saves a copy of the current graph to a file",
                        category, 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showPluginListEditor:
        result.setInfo ("Edit the list of available plug-Ins...", String(), category, 0);
        result.addDefaultKeypress ('p', ModifierKeys::commandModifier);
        break;

    case CommandIDs::showAudioSettings:
        result.setInfo ("Change the audio device settings", String(), category, 0);
        result.addDefaultKeypress ('a', ModifierKeys::commandModifier);
        break;

    case CommandIDs::toggleDoublePrecision:
        updatePrecisionMenuItem (result);
        break;

    case CommandIDs::aboutBox:
        result.setInfo ("About...", String(), category, 0);
        break;

    case CommandIDs::allWindowsForward:
        result.setInfo ("All Windows Forward", "Bring all plug-in windows forward", category, 0);
        result.addDefaultKeypress ('w', ModifierKeys::commandModifier);
        break;

    default:
        break;
    }
}

bool MainHostWindow::perform (const InvocationInfo& info)
{
    GraphDocumentComponent* const graphEditor = getGraphEditor();

    switch (info.commandID)
    {
    case CommandIDs::newFile:
        if (graphEditor != nullptr && graphEditor->graph != nullptr && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
            graphEditor->graph->newDocument();
        break;

    case CommandIDs::open:
        if (graphEditor != nullptr && graphEditor->graph != nullptr && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
            graphEditor->graph->loadFromUserSpecifiedFile (true);
        break;

    case CommandIDs::save:
        if (graphEditor != nullptr && graphEditor->graph != nullptr)
            graphEditor->graph->save (true, true);
        break;

    case CommandIDs::saveAs:
        if (graphEditor != nullptr && graphEditor->graph != nullptr)
            graphEditor->graph->saveAs (File(), true, true, true);
        break;

    case CommandIDs::showPluginListEditor:
        if (pluginListWindow == nullptr)
            pluginListWindow = new PluginListWindow (*this, formatManager);

        pluginListWindow->toFront (true);
        break;

    case CommandIDs::showAudioSettings:
        showAudioSettings();
        break;

    case CommandIDs::toggleDoublePrecision:
        if (PropertiesFile* props = getAppProperties().getUserSettings())
        {
            bool newIsDoublePrecision = ! isDoublePrecisionProcessing();
            props->setValue ("doublePrecisionProcessing", var (newIsDoublePrecision));

            {
                ApplicationCommandInfo cmdInfo (info.commandID);
                updatePrecisionMenuItem (cmdInfo);
                menuItemsChanged();
            }

            if (graphEditor != nullptr)
                graphEditor->setDoublePrecision (newIsDoublePrecision);
        }
        break;

    case CommandIDs::aboutBox:
        // TODO
        break;

    case CommandIDs::allWindowsForward:
    {
        Desktop& desktop = Desktop::getInstance();

        for (int i = 0; i < desktop.getNumComponents(); ++i)
            desktop.getComponent (i)->toBehind (this);

        break;
    }

    default:
        return false;
    }

    return true;
}

void MainHostWindow::showAudioSettings()
{
    AudioDeviceSelectorComponent audioSettingsComp (deviceManager,
                                                    0, 256,
                                                    0, 256,
                                                    true, true, true, false);

    audioSettingsComp.setSize (500, 450);

    DialogWindow::LaunchOptions o;
    o.content.setNonOwned (&audioSettingsComp);
    o.dialogTitle                   = "Audio Settings";
    o.componentToCentreAround       = this;
    o.dialogBackgroundColour        = Colours::azure;
    o.escapeKeyTriggersCloseButton  = true;
    o.useNativeTitleBar             = false;
    o.resizable                     = false;

    o.runModal();

    ScopedPointer<XmlElement> audioState (deviceManager.createStateXml());

    getAppProperties().getUserSettings()->setValue ("audioDeviceState", audioState);
    getAppProperties().getUserSettings()->saveIfNeeded();

    GraphDocumentComponent* const graphEditor = getGraphEditor();

    if (graphEditor != nullptr && graphEditor->graph != nullptr)
        graphEditor->graph->removeIllegalConnections();
}

bool MainHostWindow::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

void MainHostWindow::fileDragEnter (const StringArray&, int, int)
{
}

void MainHostWindow::fileDragMove (const StringArray&, int, int)
{
}

void MainHostWindow::fileDragExit (const StringArray&)
{
}

void MainHostWindow::filesDropped (const StringArray& files, int x, int y)
{
    GraphDocumentComponent* const graphEditor = getGraphEditor();

    if (graphEditor != nullptr)
    {
        if (files.size() == 1 && File (files[0]).hasFileExtension (filenameSuffix))
        {
            if (FilterGraph* filterGraph = graphEditor->graph.get())
            if (filterGraph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                filterGraph->loadFrom (File (files[0]), true);
        }
        else
        {
            OwnedArray <PluginDescription> typesFound;
            knownPluginList.scanAndAddDragAndDroppedFiles (formatManager, files, typesFound);

            Point<int> pos (graphEditor->getLocalPoint (this, Point<int> (x, y)));

            for (int i = 0; i < jmin (5, typesFound.size()); ++i)
                createPlugin (typesFound.getUnchecked(i), pos.getX(), pos.getY());
        }
    }
}

GraphDocumentComponent* MainHostWindow::getGraphEditor() const
{
    return dynamic_cast<GraphDocumentComponent*> (getContentComponent());
}

bool MainHostWindow::isDoublePrecisionProcessing()
{
    if (PropertiesFile* props = getAppProperties().getUserSettings())
        return props->getBoolValue ("doublePrecisionProcessing", false);

    return false;
}

void MainHostWindow::updatePrecisionMenuItem (ApplicationCommandInfo& info)
{
    info.setInfo ("Double floating point precision rendering", String(), "General", 0);
    info.setTicked (isDoublePrecisionProcessing());
}
