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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainHostWindow.h"
#include "../Filters/InternalFilters.h"


//==============================================================================
class MainHostWindow::PluginListWindow  : public DocumentWindow
{
public:
    PluginListWindow (MainHostWindow& mw, AudioPluginFormatManager& pluginFormatManager)
        : DocumentWindow ("Available Plugins",
                          LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                          DocumentWindow::minimiseButton | DocumentWindow::closeButton),
          owner (mw)
    {
        auto deadMansPedalFile = getAppProperties().getUserSettings()
                                   ->getFile().getSiblingFile ("RecentlyCrashedPluginsList");

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

    ~PluginListWindow() override
    {
        getAppProperties().getUserSettings()->setValue ("listWindowPos", getWindowStateAsString());
        clearContentComponent();
    }

    void closeButtonPressed() override
    {
        owner.pluginListWindow = nullptr;
    }

private:
    MainHostWindow& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListWindow)
};

//==============================================================================
MainHostWindow::MainHostWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    formatManager.addDefaultFormats();
    formatManager.addFormat (new InternalPluginFormat());

    auto safeThis = SafePointer<MainHostWindow> (this);
    RuntimePermissions::request (RuntimePermissions::recordAudio,
                                 [safeThis] (bool granted) mutable
                                 {
                                     std::unique_ptr<XmlElement> savedAudioState (getAppProperties().getUserSettings()
                                                                                  ->getXmlValue ("audioDeviceState"));

                                     safeThis->deviceManager.initialise (granted ? 256 : 0, 256, savedAudioState.get(), true);
                                 });

   #if JUCE_IOS || JUCE_ANDROID
    setFullScreen (true);
   #else
    setResizable (true, false);
    setResizeLimits (500, 400, 10000, 10000);
    centreWithSize (800, 600);
   #endif

    graphHolder.reset (new GraphDocumentComponent (formatManager, deviceManager, knownPluginList));

    setContentNonOwned (graphHolder.get(), false);

    restoreWindowStateFromString (getAppProperties().getUserSettings()->getValue ("mainWindowPos"));

    setVisible (true);

    InternalPluginFormat internalFormat;
    internalFormat.getAllTypes (internalTypes);

    std::unique_ptr<XmlElement> savedPluginList (getAppProperties().getUserSettings()->getXmlValue ("pluginList"));

    if (savedPluginList != nullptr)
        knownPluginList.recreateFromXml (*savedPluginList);

    for (auto* t : internalTypes)
        knownPluginList.addType (*t);

    pluginSortMethod = (KnownPluginList::SortMethod) getAppProperties().getUserSettings()
                            ->getIntValue ("pluginSortMethod", KnownPluginList::sortByManufacturer);

    knownPluginList.addChangeListener (this);

    if (auto* filterGraph = graphHolder->graph.get())
        filterGraph->addChangeListener (this);

    addKeyListener (getCommandManager().getKeyMappings());

    Process::setPriority (Process::HighPriority);

  #if JUCE_IOS || JUCE_ANDROID
    graphHolder->burgerMenu.setModel (this);
  #else
   #if JUCE_MAC
    setMacMainMenu (this);
   #else
    setMenuBar (this);
   #endif
  #endif

    getCommandManager().setFirstCommandTarget (this);
}

MainHostWindow::~MainHostWindow()
{
    pluginListWindow = nullptr;
    knownPluginList.removeChangeListener (this);

    if (auto* filterGraph = graphHolder->graph.get())
        filterGraph->removeChangeListener (this);

    getAppProperties().getUserSettings()->setValue ("mainWindowPos", getWindowStateAsString());
    clearContentComponent();

  #if ! (JUCE_ANDROID || JUCE_IOS)
   #if JUCE_MAC
    setMacMainMenu (nullptr);
   #else
    setMenuBar (nullptr);
   #endif
  #endif

    graphHolder = nullptr;
}

void MainHostWindow::closeButtonPressed()
{
    tryToQuitApplication();
}

struct AsyncQuitRetrier  : private Timer
{
    AsyncQuitRetrier()   { startTimer (500); }

    void timerCallback() override
    {
        stopTimer();
        delete this;

        if (auto app = JUCEApplicationBase::getInstance())
            app->systemRequestedQuit();
    }
};

void MainHostWindow::tryToQuitApplication()
{
    if (graphHolder->closeAnyOpenPluginWindows())
    {
        // Really important thing to note here: if the last call just deleted any plugin windows,
        // we won't exit immediately - instead we'll use our AsyncQuitRetrier to let the message
        // loop run for another brief moment, then try again. This will give any plugins a chance
        // to flush any GUI events that may have been in transit before the app forces them to
        // be unloaded
        new AsyncQuitRetrier();
    }
    else if (ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        new AsyncQuitRetrier();
    }
   #if JUCE_ANDROID || JUCE_IOS
    else if (graphHolder == nullptr || graphHolder->graph->saveDocument (FilterGraph::getDefaultGraphDocumentOnMobile()))
   #else
    else if (graphHolder == nullptr || graphHolder->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
   #endif
    {
        // Some plug-ins do not want [NSApp stop] to be called
        // before the plug-ins are not deallocated.
        graphHolder->releaseGraph();

        JUCEApplication::quit();
    }
}

void MainHostWindow::changeListenerCallback (ChangeBroadcaster* changed)
{
    if (changed == &knownPluginList)
    {
        menuItemsChanged();

        // save the plugin list every time it gets changed, so that if we're scanning
        // and it crashes, we've still saved the previous ones
        std::unique_ptr<XmlElement> savedPluginList (knownPluginList.createXml());

        if (savedPluginList != nullptr)
        {
            getAppProperties().getUserSettings()->setValue ("pluginList", savedPluginList.get());
            getAppProperties().saveIfNeeded();
        }
    }
    else if (graphHolder != nullptr && changed == graphHolder->graph.get())
    {
        auto title = JUCEApplication::getInstance()->getApplicationName();
        auto f = graphHolder->graph->getFile();

        if (f.existsAsFile())
            title = f.getFileName() + " - " + title;

        setName (title);
    }
}

StringArray MainHostWindow::getMenuBarNames()
{
    StringArray names;
    names.add ("File");
    names.add ("Plugins");
    names.add ("Options");
    names.add ("Windows");
    return names;
}

PopupMenu MainHostWindow::getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/)
{
    PopupMenu menu;

    if (topLevelMenuIndex == 0)
    {
        // "File" menu
       #if ! (JUCE_IOS || JUCE_ANDROID)
        menu.addCommandItem (&getCommandManager(), CommandIDs::newFile);
        menu.addCommandItem (&getCommandManager(), CommandIDs::open);
       #endif

        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                            ->getValue ("recentFilterGraphFiles"));

        PopupMenu recentFilesMenu;
        recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);
        menu.addSubMenu ("Open recent file", recentFilesMenu);

       #if ! (JUCE_IOS || JUCE_ANDROID)
        menu.addCommandItem (&getCommandManager(), CommandIDs::save);
        menu.addCommandItem (&getCommandManager(), CommandIDs::saveAs);
       #endif

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
    if (menuItemID == 250)
    {
        if (graphHolder != nullptr)
            if (auto* graph = graphHolder->graph.get())
                graph->clear();
    }
   #if ! (JUCE_ANDROID || JUCE_IOS)
    else if (menuItemID >= 100 && menuItemID < 200)
    {
        RecentlyOpenedFilesList recentFiles;
        recentFiles.restoreFromString (getAppProperties().getUserSettings()
                                            ->getValue ("recentFilterGraphFiles"));

        if (graphHolder != nullptr)
            if (auto* graph = graphHolder->graph.get())
                if (graph != nullptr && graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                    graph->loadFrom (recentFiles.getFile (menuItemID - 100), true);
    }
   #endif
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
        if (auto* desc = getChosenType (menuItemID))
            createPlugin (*desc,
                          { proportionOfWidth  (0.3f + Random::getSystemRandom().nextFloat() * 0.6f),
                            proportionOfHeight (0.3f + Random::getSystemRandom().nextFloat() * 0.6f) });
    }
}

void MainHostWindow::menuBarActivated (bool isActivated)
{
    if (isActivated && graphHolder != nullptr)
        graphHolder->unfocusKeyboardComponent();
}

void MainHostWindow::createPlugin (const PluginDescription& desc, Point<int> pos)
{
    if (graphHolder != nullptr)
        graphHolder->createNewPlugin (desc, pos);
}

void MainHostWindow::addPluginsToMenu (PopupMenu& m) const
{
    if (graphHolder != nullptr)
    {
        int i = 0;

        for (auto* t : internalTypes)
            m.addItem (++i, t->name + " (" + t->pluginFormatName + ")",
                       graphHolder->graph->getNodeForName (t->name) == nullptr);
    }

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

void MainHostWindow::getAllCommands (Array<CommandID>& commands)
{
    // this returns the set of all commands that this target can perform..
    const CommandID ids[] = {
                             #if ! (JUCE_IOS || JUCE_ANDROID)
                              CommandIDs::newFile,
                              CommandIDs::open,
                              CommandIDs::save,
                              CommandIDs::saveAs,
                             #endif
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
   #if ! (JUCE_IOS || JUCE_ANDROID)
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
   #endif

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
    switch (info.commandID)
    {
   #if ! (JUCE_IOS || JUCE_ANDROID)
    case CommandIDs::newFile:
        if (graphHolder != nullptr && graphHolder->graph != nullptr && graphHolder->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
            graphHolder->graph->newDocument();
        break;

    case CommandIDs::open:
        if (graphHolder != nullptr && graphHolder->graph != nullptr && graphHolder->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
            graphHolder->graph->loadFromUserSpecifiedFile (true);
        break;

    case CommandIDs::save:
        if (graphHolder != nullptr && graphHolder->graph != nullptr)
            graphHolder->graph->save (true, true);
        break;

    case CommandIDs::saveAs:
        if (graphHolder != nullptr && graphHolder->graph != nullptr)
            graphHolder->graph->saveAs (File(), true, true, true);
        break;
   #endif

    case CommandIDs::showPluginListEditor:
        if (pluginListWindow == nullptr)
            pluginListWindow.reset (new PluginListWindow (*this, formatManager));

        pluginListWindow->toFront (true);
        break;

    case CommandIDs::showAudioSettings:
        showAudioSettings();
        break;

    case CommandIDs::toggleDoublePrecision:
        if (auto* props = getAppProperties().getUserSettings())
        {
            bool newIsDoublePrecision = ! isDoublePrecisionProcessing();
            props->setValue ("doublePrecisionProcessing", var (newIsDoublePrecision));

            {
                ApplicationCommandInfo cmdInfo (info.commandID);
                updatePrecisionMenuItem (cmdInfo);
                menuItemsChanged();
            }

            if (graphHolder != nullptr)
                graphHolder->setDoublePrecision (newIsDoublePrecision);
        }
        break;

    case CommandIDs::aboutBox:
        // TODO
        break;

    case CommandIDs::allWindowsForward:
    {
        auto& desktop = Desktop::getInstance();

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
    auto* audioSettingsComp = new AudioDeviceSelectorComponent (deviceManager,
                                                                0, 256,
                                                                0, 256,
                                                                true, true,
                                                                true, false);

    audioSettingsComp->setSize (500, 450);

    DialogWindow::LaunchOptions o;
    o.content.setOwned (audioSettingsComp);
    o.dialogTitle                   = "Audio Settings";
    o.componentToCentreAround       = this;
    o.dialogBackgroundColour        = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    o.escapeKeyTriggersCloseButton  = true;
    o.useNativeTitleBar             = false;
    o.resizable                     = false;

     auto* w = o.create();
     auto safeThis = SafePointer<MainHostWindow> (this);

     w->enterModalState (true,
                         ModalCallbackFunction::create
                         ([safeThis] (int)
                         {
                             std::unique_ptr<XmlElement> audioState (safeThis->deviceManager.createStateXml());

                             getAppProperties().getUserSettings()->setValue ("audioDeviceState", audioState.get());
                             getAppProperties().getUserSettings()->saveIfNeeded();

                             if (safeThis->graphHolder != nullptr)
                                 if (safeThis->graphHolder->graph != nullptr)
                                     safeThis->graphHolder->graph->graph.removeIllegalConnections();
                         }), true);
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
    if (graphHolder != nullptr)
    {
       #if ! (JUCE_ANDROID || JUCE_IOS)
        if (files.size() == 1 && File (files[0]).hasFileExtension (FilterGraph::getFilenameSuffix()))
        {
            if (auto* filterGraph = graphHolder->graph.get())
                if (filterGraph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
                    filterGraph->loadFrom (File (files[0]), true);
        }
        else
       #endif
        {
            OwnedArray<PluginDescription> typesFound;
            knownPluginList.scanAndAddDragAndDroppedFiles (formatManager, files, typesFound);

            auto pos = graphHolder->getLocalPoint (this, Point<int> (x, y));

            for (int i = 0; i < jmin (5, typesFound.size()); ++i)
                if (auto* desc = typesFound.getUnchecked(i))
                    createPlugin (*desc, pos);
        }
    }
}

bool MainHostWindow::isDoublePrecisionProcessing()
{
    if (auto* props = getAppProperties().getUserSettings())
        return props->getBoolValue ("doublePrecisionProcessing", false);

    return false;
}

void MainHostWindow::updatePrecisionMenuItem (ApplicationCommandInfo& info)
{
    info.setInfo ("Double floating point precision rendering", String(), "General", 0);
    info.setTicked (isDoublePrecisionProcessing());
}
