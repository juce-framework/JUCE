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

#if ! (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU)
 #error "If you're building the audio plugin host, you probably want to enable VST and/or AU support"
#endif


//==============================================================================
class PluginHostApp  : public JUCEApplication,
                       private AsyncUpdater
{
public:
    PluginHostApp() {}

    void initialise (const String&) override
    {
        // initialise our settings file..

        PropertiesFile::Options options;
        options.applicationName     = "Juce Audio Plugin Host";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Preferences";

        appProperties = new ApplicationProperties();
        appProperties->setStorageParameters (options);

        LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

        mainWindow = new MainHostWindow();
        mainWindow->setUsingNativeTitleBar (true);

        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (mainWindow);

        mainWindow->menuItemsChanged();

        // Important note! We're going to use an async update here so that if we need
        // to re-open a file and instantiate some plugins, it will happen AFTER this
        // initialisation method has returned.
        // On Windows this probably won't make a difference, but on OSX there's a subtle event loop
        // issue that can happen if a plugin runs one of those irritating modal dialogs while it's
        // being loaded. If that happens inside this method, the OSX event loop seems to be in some
        // kind of special "initialisation" mode and things get confused. But if we load the plugin
        // later when the normal event loop is running, everything's fine.
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        File fileToOpen;

        for (int i = 0; i < getCommandLineParameterArray().size(); ++i)
        {
            fileToOpen = File::getCurrentWorkingDirectory().getChildFile (getCommandLineParameterArray()[i]);

            if (fileToOpen.existsAsFile())
                break;
        }

        if (! fileToOpen.existsAsFile())
        {
            RecentlyOpenedFilesList recentFiles;
            recentFiles.restoreFromString (getAppProperties().getUserSettings()->getValue ("recentFilterGraphFiles"));

            if (recentFiles.getNumFiles() > 0)
                fileToOpen = recentFiles.getFile (0);
        }

        if (fileToOpen.existsAsFile())
            if (GraphDocumentComponent* graph = mainWindow->getGraphEditor())
                if (FilterGraph* ioGraph = graph->graph.get())
                    ioGraph->loadFrom (fileToOpen, true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        appProperties = nullptr;
        LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
            mainWindow->tryToQuitApplication();
        else
            JUCEApplicationBase::quit();
    }

    const String getApplicationName() override       { return "Juce Plug-In Host"; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    ApplicationCommandManager commandManager;
    ScopedPointer<ApplicationProperties> appProperties;
    LookAndFeel_V3 lookAndFeel;

private:
    ScopedPointer<MainHostWindow> mainWindow;
};

static PluginHostApp& getApp()                      { return *dynamic_cast<PluginHostApp*>(JUCEApplication::getInstance()); }
ApplicationCommandManager& getCommandManager()      { return getApp().commandManager; }
ApplicationProperties& getAppProperties()           { return *getApp().appProperties; }


// This kicks the whole thing off..
START_JUCE_APPLICATION (PluginHostApp)
